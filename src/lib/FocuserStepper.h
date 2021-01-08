// -----------------------------------------------------------------------------------
// Focusers

#pragma once

// time to write position to nv after last movement of Focuser 1/2, default = 5 minutes
#define FOCUSER_WRITE_DELAY 1000L*60L*5L

#include "Focuser.h"

class focuserStepper : public focuser {
  public:
    void init(int stepPin, int dirPin, int enPin, int nvAddress, int nvTcfCoef, int nvTcfEn, float maxRate, double stepsPerMicro, double min, double max, double minRate) {
      this->stepPin=stepPin;
      this->dirPin=dirPin;
      this->enPin=enPin;
      this->nvAddress=nvAddress;
      this->nvTcfCoef=nvTcfCoef;
      this->nvTcfEn=nvTcfEn;
      this->minRate=minRate;
      this->maxRate=maxRate;
      this->spm=stepsPerMicro;

      if (stepPin != -1) pinMode(stepPin,OUTPUT);
      if (dirPin != -1) pinMode(dirPin,OUTPUT);

      // get the temperature compensated focusing settings
      setTcfCoef(nv.readFloat(nvTcfCoef));
      setTcfEnable(nv.read(nvTcfEn));
      
      // get step position
      spos=readPos();
      long lmin=(long)(min*spm); if (spos < lmin) spos=lmin;
      long lmax=(long)(max*spm); if (spos > lmax) spos=lmax;
      target.part.m=spos; target.part.f=0;
      lastPos=spos;
      delta.fixed=0;

      // set min/max
      setMin(lmin);
      setMax(lmax);

      // steps per second, maximum
      spsMax=(1.0/maxRate)*1000.0;
      // microns per second default
      setMoveRate(100);

      nextPhysicalMove=micros()+(unsigned long)(maxRate*1000.0);
      lastPhysicalMove=nextPhysicalMove;
    }

    // temperature compensation
    void setTcfCoef(double coef) { if (abs(coef) >= 10000.0) coef = 0.0; tcf_coef = coef; nv.writeFloat(nvTcfCoef,tcf_coef); }
    double getTcfCoef() { return tcf_coef; }
    void setTcfEnable(boolean enabled) { tcf = enabled; nv.write(nvTcfEn,tcf); }
    boolean getTcfEnable() { return tcf; }

    // sets logic state for reverse motion
    void setReverseState(int reverseState) {
      this->reverseState=reverseState;
      if (reverseState == LOW) forwardState=HIGH; else forwardState=LOW;
    }

    // sets logic state for disabling stepper driver
    void setDisableState(boolean disableState) {
      this->disableState=disableState;
      if (disableState == LOW) enableState=HIGH; else enableState=LOW;
      if (enPin != -1) { pinMode(enPin,OUTPUT); enableDriver(); currentlyDisabled=false; }
    }

    // allows enabling/disabling stepper driver
    void powerDownActive(boolean active) {
      if (enPin == -1) { pda=false; return; }
      pda=active;
      if (pda) { pinMode(enPin,OUTPUT); disableDriver(); currentlyDisabled=true; }
    }

    // set movement rate in microns/second, from minRate to 1000
    void setMoveRate(double rate) {
      constrain(rate,minRate,1000);
      moveRate=rate*spm;                            // in steps per second
      if (moveRate > spsMax) moveRate=spsMax;       // limit to maxRate
    }

    // move in
    void startMoveIn() {
      delta.fixed=doubleToFixed(+moveRate/100.0);   // in steps per centi-second
    }

    // move out
    void startMoveOut() {
      delta.fixed=doubleToFixed(-moveRate/100.0);   // in steps per centi-second
    }

    // sets target position in steps
    void setTarget(long pos) {
      target.part.m=pos; target.part.f=0;
      if ((long)target.part.m < smin) target.part.m=smin; if ((long)target.part.m > smax) target.part.m=smax;
    }

    // sets target relative position in steps
    void relativeTarget(long pos) {
      target.part.m+=pos; target.part.f=0;
      if ((long)target.part.m < smin) target.part.m=smin; if ((long)target.part.m > smax) target.part.m=smax;
    }
    
    // do automatic movement
    void move() {
      target.fixed+=delta.fixed;
      // stop at limits
      if (((long)target.part.m < smin) || ((long)target.part.m > smax)) delta.fixed=0;
    }

    void follow(boolean slewing) {

      // if enabled and the timeout has elapsed, disable the stepper driver
      if (pda && !currentlyDisabled && ((micros()-lastPhysicalMove) > 10000000L)) { disableDriver(); currentlyDisabled=true; }
    
      // write position to non-volatile storage if not moving for FOCUSER_WRITE_DELAY milliseconds
      if ((spos != lastPos)) { lastMove=millis(); lastPos=spos; }
      if (!slewing && (spos != readPos())) {
        // needs updating and enough time has passed?
        if ((long)(millis()-lastMove) > FOCUSER_WRITE_DELAY) writePos(spos);
      }

      // temperature compensation
      long tcfSteps;
      if (tcf) {
        tcfSteps = -round((tcf_coef * (ambient.getTelescopeTemperature() - 10.0)) * spm);
      } else {
        tcfSteps = 0;
      }

      unsigned long microsNow=micros();
      if ((long)(microsNow-nextPhysicalMove) > 0) {
        nextPhysicalMove=microsNow+(unsigned long)(maxRate*1000.0);
    
        if ((spos < (long)target.part.m + tcfSteps) && (spos < smax)) {
          if (pda && currentlyDisabled) { enableDriver(); currentlyDisabled=false; delayMicroseconds(5); }
          digitalWrite(stepPin,LOW); delayMicroseconds(5);
          digitalWrite(dirPin,forwardState); delayMicroseconds(5);
          digitalWrite(stepPin,HIGH); spos++;
          lastPhysicalMove=micros();
        } else
        if ((spos > (long)target.part.m + tcfSteps) && (spos > smin)) {
          if (pda && currentlyDisabled) { enableDriver(); currentlyDisabled=false; delayMicroseconds(5); }
          digitalWrite(stepPin,LOW); delayMicroseconds(5);
          digitalWrite(dirPin,reverseState); delayMicroseconds(5);
          digitalWrite(stepPin,HIGH); spos--;
          lastPhysicalMove=micros();
        }
      }
    }

  private:

    void enableDriver() {
      if (enPin == -1) return;
      // for Aux5/Aux6 (DAC) support for stepper driver EN control on MaxPCB Aux5=A21=66 Aux6=A22=67
      if ((enPin == 66) || (enPin == 67)) { if (enableState == HIGH) analogWrite(enPin,255); else analogWrite(enPin,0); delayMicroseconds(30); } else 
      { digitalWrite(enPin,enableState); delayMicroseconds(5); }
    }

    void disableDriver() {
      if (enPin == -1) return;
      if ((enPin == 66) || (enPin == 67)) { if (disableState == HIGH) analogWrite(enPin,255); else analogWrite(enPin,0); delayMicroseconds(30); } else 
      { digitalWrite(enPin,disableState); delayMicroseconds(5); }
    }
};
