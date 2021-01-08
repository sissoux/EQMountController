/*
 * Title       OnStep
 * by          Howard Dutton
 *
 * Copyright (C) 2012 to 2020 Howard Dutton
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Description:
 *   Full featured stepper motor telescope microcontroller for Equatorial and
 *   Alt-Azimuth mounts, with the LX200 derived command set.
 *
 * Author: Howard Dutton
 *   http://www.stellarjourney.com
 *   hjd1964@gmail.com
 *
 * Revision history, and newer versions:
 *   See GitHub: https://github.com/hjd1964/OnStep
 *
 * Documentation:
 *   https://groups.io/g/onstep/wiki/home
 *
 * Discussion, Questions, ...etc
 *   https://groups.io/g/onstep
 */

// Use Config.xxx.h to configure OnStep to your requirements

// firmware info, these are returned by the ":GV?#" commands
#define FirmwareDate          __DATE__
#define FirmwareVersionMajor  3
#define FirmwareVersionMinor  16      // minor version 0 to 99
#define FirmwareVersionPatch  "q"     // for example major.minor patch: 1.3c
#define FirmwareVersionConfig 3       // internal, for tracking configuration file changes
#define FirmwareName          "On-Step"
#define FirmwareTime          __TIME__

// On first upload OnStep automatically initializes a host of settings in nv memory (EEPROM.)
// This option forces that initialization again.
// Change to true, upload OnStep and nv will be reset to default. Then immediately set to false and upload again.
// *** IMPORTANT: This option must not be left set to true or it will cause excessive wear of EEPROM or FLASH ***
#define NV_INIT_KEY_RESET false

#include <errno.h>
#include <math.h>

#include "Constants.h"
#include "src/sd_drivers/Models.h"
#include "Config.h"
#include "src/pinmaps/Models.h"
#include "src/HAL/HAL.h"
#include "Validate.h"

// Enable debugging messages on DebugSer -------------------------------------------------------------
#define DEBUG_OFF             // default=_OFF, use "DEBUG_ON" to activate
#define DebugSer SerialA      // default=SerialA, or SerialB for example (always 9600 baud)

// Helper macros for debugging, with less typing
#if defined(DEBUG_ON)
  #define D(x)       DebugSer.print(x)
  #define DH(x)      DebugSer.print(x,HEX)
  #define DL(x)      DebugSer.println(x)
  #define DHL(x,y)   DebugSer.println(x,HEX)
#else
  #define D(x)
  #define DH(x,y)
  #define DL(x)
  #define DHL(x,y)
#endif

// ---------------------------------------------------------------------------------------------------

#include "src/lib/St4SerialMaster.h"
#include "src/lib/FPoint.h"
#include "Globals.h"
#include "src/lib/Julian.h"
#include "src/lib/Misc.h"
#include "src/lib/Sound.h"
#include "src/lib/Coord.h"
#include "Align.h"
#include "src/lib/Library.h"
#include "src/lib/Command.h"
#include "src/lib/TLS.h"
#include "src/lib/Weather.h"
weather ambient;

#if ROTATOR == ON
  #include "src/lib/Rotator.h"
  rotator rot;
#endif

#if FOCUSER1 == ON || FOCUSER2 == ON
  #include "src/lib/Focuser.h"
  #if FOCUSER1 == ON
    #if AXIS4_DRIVER_DC_MODE != OFF
      #include "src/lib/FocuserDC.h"
      focuserDC foc1;
    #else
      #include "src/lib/FocuserStepper.h"
      focuserStepper foc1;
    #endif
  #endif
  #if FOCUSER2 == ON
    #if AXIS5_DRIVER_DC_MODE != OFF
      #include "src/lib/FocuserDC.h"
      focuserDC foc2;
    #else
      #include "src/lib/FocuserStepper.h"
      focuserStepper foc2;
    #endif
  #endif
#endif

// support for TMC2130, TMC5160, etc. stepper drivers in SPI mode
#if MODE_SWITCH_BEFORE_SLEW == TMC_SPI
  #include "src/lib/SoftSPI.h"
  #include "src/lib/TMC_SPI.h"
  #if AXIS1_DRIVER_STATUS == TMC_SPI
//                        SS      ,SCK     ,MISO    ,MOSI
    tmcSpiDriver tmcAxis1(Axis1_M2,Axis1_M1,Axis1_M3,Axis1_M0);
  #else
    tmcSpiDriver tmcAxis1(Axis1_M2,Axis1_M1,      -1,Axis1_M0);
  #endif
  #if AXIS2_DRIVER_STATUS == TMC_SPI
    tmcSpiDriver tmcAxis2(Axis2_M2,Axis2_M1,Axis2_M3,Axis2_M0);
  #else
    tmcSpiDriver tmcAxis2(Axis2_M2,Axis2_M1,      -1,Axis2_M0);
  #endif
  #if ROTATOR == ON && AXIS3_DRIVER_MODEL == TMC_SPI
    tmcSpiDriver tmcAxis3(Axis3_M2,Axis3_M1,      -1,Axis3_M0);
  #endif
  #if FOCUSER1 == ON && AXIS4_DRIVER_MODEL == TMC_SPI
    tmcSpiDriver tmcAxis4(Axis4_M2,Axis4_M1,      -1,Axis4_M0);
  #endif
  #if FOCUSER2 == ON && AXIS5_DRIVER_MODEL == TMC_SPI
    tmcSpiDriver tmcAxis5(Axis5_M2,Axis5_M1,      -1,Axis5_M0);
  #endif
#endif

void setup() {
  // early pin initialization
  initPre();
  
  // take a half-second to let any connected devices come up before we start setting up pins
  delay(500);

#ifdef DEBUG_ON
  // Initialize USB serial debugging early, so we can use DebugSer.print() for debugging, if needed
  DebugSer.begin(9600);
  delay(5000);
#endif
  
  // Call hardware specific initialization
  HAL_Init();

  SerialA.begin(SERIAL_A_BAUD_DEFAULT);
#ifdef HAL_SERIAL_B_ENABLED
  SerialB.begin(SERIAL_B_BAUD_DEFAULT);
#endif
#ifdef HAL_SERIAL_C_ENABLED
  SerialC.begin(SERIAL_C_BAUD_DEFAULT);
#endif
#if ST4_HAND_CONTROL == ON
  SerialST4.begin();
#endif

  // Take another two seconds to be sure Serial ports are online
  delay(2000);

  // initialize the Non-Volatile Memory
  if (!nv.init()) {
    while (true) {
      SerialA.print("NV (EEPROM) failure!#\r\n");
      for (int i=0; i<200; i++) { 
        #ifdef HAL_SERIAL_TRANSMIT
          SerialA.transmit();
        #endif
        delay(10);
      }
    }
  }

  // initialize the Object Library
  Lib.init();

  // prepare PEC buffer
#if MOUNT_TYPE != ALTAZM
  createPecBuffer();
#endif
  
  // set initial values for some variables
  initStartupValues();

  // set pins for input/output as specified in Config.h and PinMap.h
  initPins();

  // get guiding ready
  initGuide();

  // if this is the first startup set EEPROM to defaults
  initWriteNvValues();
  
  // get weather monitoring ready to go
  if (!ambient.init()) generalError=ERR_WEATHER_INIT;

  // get the TLS ready (if present)
  if (!tls.init()) generalError=ERR_SITE_INIT;
  
  // this sets up the sidereal timer and tracking rates
  siderealInterval=nv.readLong(EE_siderealInterval); // the number of 16MHz clocks in one sidereal second (this is scaled to actual processor speed)
  SiderealRate=siderealInterval/StepsPerSecondAxis1;
  timerRateAxis1=SiderealRate;
  timerRateAxis2=SiderealRate;

  // backlash takeup rates
  TakeupRate=SiderealRate/TRACK_BACKLASH_RATE;
  timerRateBacklashAxis1=SiderealRate/TRACK_BACKLASH_RATE;
  timerRateBacklashAxis2=(SiderealRate/TRACK_BACKLASH_RATE)*timerRateRatio;

  // now read any saved values from EEPROM into varaibles to restore our last state
  initReadNvValues();

  // starts the hardware timers that keep sidereal time, move the motors, etc.
  setTrackingRate(default_tracking_rate);
  setDeltaTrackingRate();
  initStartTimers();
 
  // tracking autostart
#if TRACK_AUTOSTART == ON
  #if MOUNT_TYPE != ALTAZM

    // tailor behaviour depending on TLS presence
    if (!tls.active) {
      setHome();
      safetyLimitsOn=false;
    } else {
      if (parkStatus == Parked) unPark(true); else setHome();
    }
    
    // start tracking
    trackingState=TrackingSidereal;
    enableStepperDrivers();
  #else
    #warning "Tracking autostart ignored for MOUNT_TYPE ALTAZM"
  #endif
#else
  // unpark without tracking, if parked
  if (parkStatus == Parked) unPark(false);
#endif

  // start rotator if present
#if ROTATOR == ON
  rot.init(Axis3StepPin,Axis3DirPin,Axis3_EN,AXIS3_STEP_RATE_MAX,AXIS3_STEPS_PER_DEGREE,AXIS3_LIMIT_MIN,AXIS3_LIMIT_MAX);
  #if AXIS3_DRIVER_REVERSE == ON
    rot.setReverseState(HIGH);
  #endif
  rot.setDisableState(AXIS3_DRIVER_DISABLE);
  
  #if MODE_SWITCH_BEFORE_SLEW == TMC_SPI && AXIS3_DRIVER_MODEL == TMC_SPI
    tmcAxis3.setup(AXIS3_DRIVER_INTPOL,AXIS3_DRIVER_DECAY_MODE,AXIS3_DRIVER_CODE&0b001111,AXIS3_DRIVER_IRUN,AXIS3_DRIVER_IRUN,AXIS3_DRIVER_RSENSE,AXIS3_DRIVER_SUBMODEL);
    delay(150);
    tmcAxis3.setup(AXIS3_DRIVER_INTPOL,AXIS3_DRIVER_DECAY_MODE,AXIS3_DRIVER_CODE&0b001111,AXIS3_DRIVER_IRUN,AXIS3_DRIVER_IHOLD,AXIS3_DRIVER_RSENSE,AXIS3_DRIVER_SUBMODEL);
  #endif
  
  #if AXIS3_DRIVER_POWER_DOWN == ON
    rot.powerDownActive(true);
  #else
    rot.powerDownActive(false);
  #endif
#endif

  // start focusers if present
#if FOCUSER1 == ON
  foc1.init(Axis4StepPin,Axis4DirPin,Axis4_EN,EE_posAxis4,EE_tcfCoefAxis4,EE_tcfEnAxis4,AXIS4_STEP_RATE_MAX,AXIS4_STEPS_PER_MICRON,AXIS4_LIMIT_MIN*1000.0,AXIS4_LIMIT_MAX*1000.0,AXIS4_LIMIT_MIN_RATE);
  #if AXIS4_DRIVER_DC_MODE != OFF
    foc1.initDcPower(EE_dcPwrAxis4);
    foc1.setPhase1();
  #endif
  #if AXIS4_DRIVER_REVERSE == ON
    foc1.setReverseState(HIGH);
  #endif
  foc1.setDisableState(AXIS4_DRIVER_DISABLE);

  #if MODE_SWITCH_BEFORE_SLEW == TMC_SPI && AXIS4_DRIVER_MODEL == TMC_SPI
    tmcAxis4.setup(AXIS4_DRIVER_INTPOL,AXIS4_DRIVER_DECAY_MODE,AXIS4_DRIVER_CODE&0b001111,AXIS4_DRIVER_IRUN,AXIS4_DRIVER_IRUN,AXIS4_DRIVER_RSENSE,AXIS4_DRIVER_SUBMODEL);
    delay(150);
    tmcAxis4.setup(AXIS4_DRIVER_INTPOL,AXIS4_DRIVER_DECAY_MODE,AXIS4_DRIVER_CODE&0b001111,AXIS4_DRIVER_IRUN,AXIS4_DRIVER_IHOLD,AXIS4_DRIVER_RSENSE,AXIS4_DRIVER_SUBMODEL);
  #endif

  #if AXIS4_DRIVER_POWER_DOWN == ON
    foc1.powerDownActive(true);
  #else
    foc1.powerDownActive(false);
  #endif
#endif

#if FOCUSER2 == ON
  foc2.init(Axis5StepPin,Axis5DirPin,Axis5_EN,EE_posAxis5,EE_tcfCoefAxis5,EE_tcfEnAxis5,AXIS5_STEP_RATE_MAX,AXIS5_STEPS_PER_MICRON,AXIS5_LIMIT_MIN*1000.0,AXIS5_LIMIT_MAX*1000.0,AXIS5_LIMIT_MIN_RATE);
  #if AXIS5_DRIVER_DC_MODE == DRV8825
    foc2.initDcPower(EE_dcPwrAxis5);
    foc2.setPhase2();
  #endif
  #if AXIS5_DRIVER_REVERSE == ON
    foc2.setReverseState(HIGH);
  #endif
  foc2.setDisableState(AXIS5_DRIVER_DISABLE);

  #if MODE_SWITCH_BEFORE_SLEW == TMC_SPI && AXIS5_DRIVER_MODEL == TMC_SPI
    tmcAxis5.setup(AXIS5_DRIVER_INTPOL,AXIS5_DRIVER_DECAY_MODE,AXIS5_DRIVER_CODE&0b001111,AXIS5_DRIVER_IRUN,AXIS5_DRIVER_IRUN,AXIS5_DRIVER_RSENSE,AXIS5_DRIVER_SUBMODEL);
    delay(150);
    tmcAxis5.setup(AXIS5_DRIVER_INTPOL,AXIS5_DRIVER_DECAY_MODE,AXIS5_DRIVER_CODE&0b001111,AXIS5_DRIVER_IRUN,AXIS5_DRIVER_IHOLD,AXIS5_DRIVER_RSENSE,AXIS5_DRIVER_SUBMODEL);
  #endif

  #if AXIS5_DRIVER_POWER_DOWN == ON
    foc2.powerDownActive(true);
  #else
    foc2.powerDownActive(false);
  #endif
#endif

  // finally clear the comms channels
  delay(500);
  SerialA.flush();
#ifdef HAL_SERIAL_B_ENABLED
  SerialB.flush();
#endif
#ifdef HAL_SERIAL_C_ENABLED
  SerialC.flush();
#endif
  delay(500);

  // prep counters (for keeping time in main loop)
  cli(); siderealTimer=lst; guideSiderealTimer=lst; PecSiderealTimer=lst; sei();
  last_loop_micros=micros();
}

void loop() {
  loop2();
  Align.model(0); // GTA compute pointing model, this will call loop2() during extended processing
}

void loop2() {
  // GUIDING -------------------------------------------------------------------------------------------
  ST4();
  if ((trackingState != TrackingMoveTo) && (parkStatus == NotParked)) guide();

#if MOUNT_TYPE != ALTAZM
  // PERIODIC ERROR CORRECTION -------------------------------------------------------------------------
  if ((trackingState == TrackingSidereal) && (parkStatus == NotParked) && (!((guideDirAxis1 || guideDirAxis2) && (activeGuideRate > GuideRate1x)))) { 
    // only active while sidereal tracking with a guide rate that makes sense
    pec();
  } else disablePec();
#endif

#if HOME_SENSE != OFF
  // AUTOMATIC HOMING ----------------------------------------------------------------------------------
  checkHome();
#endif

  // 1/100 SECOND TIMED --------------------------------------------------------------------------------
  cli(); long tempLst=lst; sei();
  if (tempLst != siderealTimer) {
    siderealTimer=tempLst;

#ifdef ESP32
    timerSupervisor(true);
#endif
    
#if MOUNT_TYPE != ALTAZM
    // WRITE PERIODIC ERROR CORRECTION TO EEPROM
    if (pecAutoRecord > 0) {
      // write PEC table to EEPROM, should do several hundred bytes/second
      pecAutoRecord--;
      nv.update(EE_pecTable+pecAutoRecord,pecBuffer[pecAutoRecord]);
    }
#endif

    // FLASH LED DURING SIDEREAL TRACKING
    if (trackingState == TrackingSidereal) {
#if LED_STATUS == ON
      if (siderealTimer%20L == 0L) { if (ledOn) { digitalWrite(LEDnegPin,HIGH); ledOn=false; } else { digitalWrite(LEDnegPin,LOW); ledOn=true; } }
#endif
    }

    // SIDEREAL TRACKING DURING GOTOS
    // keeps the target where it's supposed to be while doing gotos
    if (trackingState == TrackingMoveTo) {
      moveTo();
      if (lastTrackingState == TrackingSidereal) {
        // origTargetAxisn isn't used in Alt/Azm mode since meridian flips never happen
        origTargetAxis1.fixed+=fstepAxis1.fixed;
        // don't advance the target during meridian flips
        if ((getInstrPierSide() == PierSideEast) || (getInstrPierSide() == PierSideWest)) {
          cli();
          targetAxis1.fixed+=fstepAxis1.fixed;
          targetAxis2.fixed+=fstepAxis2.fixed;
          sei();
        }
      }
    }

    // ROTATOR/FOCUSERS, MOVE THE TARGET
#if ROTATOR == ON
    rot.move(trackingState == TrackingSidereal);
#endif
#if FOCUSER1 == ON
    foc1.move();
#endif
#if FOCUSER2 == ON
    foc2.move();
#endif

    // CALCULATE SOME TRACKING RATES, ETC.
    if (lst%3 == 0) doFastAltCalc(false);
#if MOUNT_TYPE == ALTAZM
    // figure out the current Alt/Azm tracking rates
    if (lst%3 != 0) doHorRateCalc();
#else
    // figure out the current refraction compensated tracking rate
    if ((rateCompensation != RC_NONE) && (lst%3 != 0)) doRefractionRateCalc();
#endif

    // SAFETY CHECKS
#if LIMIT_SENSE != OFF
    // support for limit switch(es)
    byte limit_1st = digitalRead(LimitPin);
    if (limit_1st == LIMIT_SENSE_STATE) {
      // Wait for a short while, then read again
      delayMicroseconds(50);
      byte limit_2nd = digitalRead(LimitPin);
      if (limit_2nd == LIMIT_SENSE_STATE) {
        // It is still low, there must be a problem
        generalError=ERR_LIMIT_SENSE;
        stopLimit();
      }
    }
#endif

    // check for fault signal, stop any slew or guide and turn tracking off
#if AXIS1_DRIVER_STATUS == LOW || AXIS1_DRIVER_STATUS == HIGH
    faultAxis1=(digitalRead(Axis1FaultPin) == AXIS1_DRIVER_STATUS);
#elif AXIS1_DRIVER_STATUS == TMC_SPI
    if (lst%2 == 0) faultAxis1=tmcAxis1.error();
#endif
#if AXIS2_DRIVER_STATUS == LOW || AXIS2_DRIVER_STATUS == HIGH
    faultAxis2=(digitalRead(Axis2FaultPin) == AXIS2_DRIVER_STATUS);
#elif AXIS2_DRIVER_STATUS == TMC_SPI
    if (lst%2 == 1) faultAxis2=tmcAxis2.error();
#endif

    if (faultAxis1 || faultAxis2) { 
      generalError=ERR_MOTOR_FAULT;
      if (trackingState == TrackingMoveTo) {
        if (!abortSlew) abortSlew=StartAbortSlew;
      } else {
        trackingState=TrackingNone;
        if (guideDirAxis1) guideDirAxis1='b';
        if (guideDirAxis2) guideDirAxis2='b';
      }
    }

    if (safetyLimitsOn) {
      // check altitude overhead limit and horizon limit
      if (currentAlt < minAlt) { generalError=ERR_ALT_MIN; stopLimit(); }
      if (currentAlt > maxAlt) { generalError=ERR_ALT_MAX; stopLimit(); }
    }

    // OPTION TO POWER DOWN AXIS2 IF NOT MOVING
#if AXIS2_DRIVER_POWER_DOWN == ON && MOUNT_TYPE != ALTAZM
    autoPowerDownAxis2();
#endif

    // UPDATE GPS INFO.
#if TIME_LOCATION_SOURCE == GPS
    if (!tls.active && tls.poll()) {
      tls.getSite(latitude,longitude);
      tls.get(JD,LMT);
      UT1=LMT+timeZone;
      updateLST(jd2last(JD,UT1,false));
      dateWasSet=true;
      timeWasSet=true;
      if (generalError == ERR_SITE_INIT) generalError=ERR_NONE;
    }
#endif

    // UPDATE THE UT1 CLOCK
    cli(); long cs=lst; sei();
    double t2=(double)((cs-lst_start)/100.0)/1.00273790935;
    // This just needs to be accurate to the nearest second, it's about 10x better
    UT1=UT1_start+(t2/3600.0);
  }

  // ROTATOR/DEROTATOR/FOCUSERS ------------------------------------------------------------------------
#if ROTATOR == ON
  rot.follow();
#endif
#if FOCUSER1 == ON
  foc1.follow(isSlewing());
#endif
#if FOCUSER2 == ON
  foc2.follow(isSlewing());
#endif

  // FASTEST POLLING -----------------------------------------------------------------------------------
  if (!isSlewing()) nv.poll();
  
  // WORKLOAD MONITORING -------------------------------------------------------------------------------
  long this_loop_micros=micros();
  loop_time=this_loop_micros-last_loop_micros;
  if (loop_time > worst_loop_time) worst_loop_time=loop_time;
  last_loop_micros=this_loop_micros;
  average_loop_time=(average_loop_time*49.0+loop_time)/50.0;

  unsigned long tempMs=millis();

  // 0.5 SECOND TIMED ----------------------------------------------------------------------------------
  static unsigned long debugTimer=0;
  if (((long)(tempMs-debugTimer) > 0) && (millis() < 1200)) {
    debugTimer=tempMs+500UL;
//    DL(((double)SiderealRate/(double)timerRateAxis1));
  }

  // 1 SECOND TIMED ------------------------------------------------------------------------------------
  static unsigned long housekeepingTimer=0;
  if (housekeepingTimer == 0) {
    housekeepingTimer = millis();
  }

  if ((long)(tempMs-housekeepingTimer) > 0) {
    housekeepingTimer=tempMs+1000UL;

#if ROTATOR == ON && MOUNT_TYPE == ALTAZM
    // calculate and set the derotation rate as required
    double h,d; getApproxEqu(&h,&d,true);
    if (trackingState == TrackingSidereal) rot.derotate(h,d);
#endif

    // adjust tracking rate for Alt/Azm mounts
    // adjust tracking rate for refraction
    setDeltaTrackingRate();

    // basic check to see if we're not at home
    if (trackingState != TrackingNone) atHome=false;

#if PEC_SENSE >= 0
    // analog mode, see if we're on the PEC index
    if (trackingState == TrackingSidereal) pecAnalogValue = analogRead(AnalogPecPin);
#endif
    
#if PPS_SENSE != OFF
    // update clock via PPS
    if (trackingState == TrackingSidereal) {
      cli();
      PPSrateRatio=((double)1000000.0/(double)(PPSavgMicroS));
      if ((long)(micros()-(PPSlastMicroS+2000000UL)) > 0) PPSsynced=false; // if more than two seconds has ellapsed without a pulse we've lost sync
      sei();
  #if LED_STATUS2 == ON
      if (PPSsynced) { if (led2On) { digitalWrite(LEDneg2Pin,HIGH); led2On=false; } else { digitalWrite(LEDneg2Pin,LOW); led2On=true; } } else { digitalWrite(LEDneg2Pin,HIGH); led2On=false; } // indicate PPS
  #endif
      if (LastPPSrateRatio != PPSrateRatio) { SiderealClockSetInterval(siderealInterval); LastPPSrateRatio=PPSrateRatio; }
    }
#endif

#if LED_STATUS == ON
    // LED indicate PWR on 
    if (trackingState != TrackingSidereal) if (!ledOn) { digitalWrite(LEDnegPin,LOW); ledOn=true; }
#endif
#if LED_STATUS2 == ON
    // LED indicate STOP and GOTO
    if (trackingState == TrackingMoveTo) if (!led2On) { digitalWrite(LEDneg2Pin,LOW); led2On=true; }
  #if PPS_SENSE != OFF
    if (trackingState == TrackingNone) if (led2On) { digitalWrite(LEDneg2Pin,HIGH); led2On=false; }
  #else
    if (trackingState != TrackingMoveTo) if (led2On) { digitalWrite(LEDneg2Pin,HIGH); led2On=false; }
  #endif
#endif

    // SAFETY CHECKS, keeps mount from tracking past the meridian limit, past the AXIS1_LIMIT_UNDER_POLE, or past the Dec limits
    if (safetyLimitsOn) {
      if (meridianFlip != MeridianFlipNever) {
        if (getInstrPierSide() == PierSideWest) {
          if (getInstrAxis1() > degreesPastMeridianW) {
            if (autoMeridianFlip) {
              if (goToHere(true) != CE_NONE) { generalError=ERR_MERIDIAN; trackingState=TrackingNone; }
            } else {
              generalError=ERR_MERIDIAN; stopLimit();
            }
          }
        } else
        if (getInstrPierSide() == PierSideEast) {
          if (getInstrAxis1() < -degreesPastMeridianE) { generalError=ERR_MERIDIAN; stopLimit(); }
          if (getInstrAxis1() > AXIS1_LIMIT_UNDER_POLE) { generalError=ERR_UNDER_POLE; stopLimit(); }
        }
      } else {
#if MOUNT_TYPE != ALTAZM
        // when Fork mounted, ignore pierSide and just stop the mount if it passes the UnderPoleLimit
        if (getInstrAxis1() > AXIS1_LIMIT_UNDER_POLE) { generalError=ERR_UNDER_POLE; stopLimit(); }
#else
        // when Alt/Azm mounted, just stop the mount if it passes AXIS1_LIMIT_MAXAZM
        if (getInstrAxis1() > AXIS1_LIMIT_MAXAZM) { generalError=ERR_AZM; stopLimit(); }
#endif
      }
    }
    // check for exceeding AXIS2_LIMIT_MIN or AXIS2_LIMIT_MAX
#if MOUNT_TYPE != ALTAZM
  #if AXIS2_TANGENT_ARM == ON
      if (posAxis2/AXIS2_STEPS_PER_DEGREE < AXIS2_LIMIT_MIN) { generalError=ERR_DEC; decMinLimit(); } else
      if (posAxis2/AXIS2_STEPS_PER_DEGREE > AXIS2_LIMIT_MAX) { generalError=ERR_DEC; decMaxLimit(); } else
      if (trackingState == TrackingSidereal && generalError == ERR_DEC) generalError=ERR_NONE;
  #else
      if (currentDec < AXIS2_LIMIT_MIN) { generalError=ERR_DEC; decMinLimit(); }
      if (currentDec > AXIS2_LIMIT_MAX) { generalError=ERR_DEC; decMaxLimit(); }
  #endif
#endif

    // update weather info
    if (!isSlewing()) ambient.poll();

  } else {
    // COMMAND PROCESSING --------------------------------------------------------------------------------
    processCommands();
  }
}
