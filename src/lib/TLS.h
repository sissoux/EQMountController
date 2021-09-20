#pragma once
#include "Julian.h"
#include <TinyGPS++.h>
#include <TimeLib.h>

#define MIN_DETECTED_SATELLITE 5

class timeLocationSource
{
    public:
    bool active = false;
    
    timeLocationSource(Stream *SerialCom, double timeZone)
    {
        SerialGPS = SerialCom;
        setSyncProvider(getTeensy3Time);
        _timeZone = timeZone;
    }

    void setTimeZone(double timeZone)
    {
      _timeZone = timeZone;
    }

    bool init()
    {
      active = true;
      return true;
    }

    bool isGPSReady()  //GPS Time and location validation method
    { 
        return (gps.location.isValid() && gps.satellites.isValid() && gps.satellites.value()>=MIN_DETECTED_SATELLITE && gps.date.isValid() && gps.time.isValid());
    }

    bool poll() // needs to be called often. Will check GPS info and returns true is correct data is available. 
    {
        if (CorrectLocationCaptured && SingleAcquisition) return false;   //Get GPS data only until valid data has been detected to avoid changing location during usage.
        while (SerialGPS->available() > 0) gps.encode(SerialGPS->read());
        if (isGPSReady()) 
        {
            CorrectLocationCaptured = true;
            latitude = gps.location.lat();
            longitude = gps.location.lng();
            setDateFromGPS();
            return true;
        }
        return false;
    }
    
    void get(double &JD, double &LMT) //Get time from GPS if synced or from RTC (teensy) if not.
    {
        if (!isGPSReady())
        {
            int y1=year();
            JD=julian(y1,month(),day());
            LMT=(hour()+(minute()/60.0)+(second()/3600.0));
        }
        else
        {
            JD=julian(gps.date.year(),gps.date.month(),gps.date.day());
            LMT=(gps.time.hour()+(gps.time.minute()/60.0)+(gps.time.second()/3600.0))-_timeZone;
            if (LMT < 0)   { LMT+=24.0; JD-=1; }
            if (LMT >= 24) { LMT-=24.0; JD+=1; }
        }
    }

    void set(double &JD, double &LMT)
    {
      int y,mo,d,h;
      double m,s;
      
      greg(JD,&y,&mo,&d); y-=2000; if (y >= 100) y-=100;

      double f1=fabs(LMT)+0.000139;
      h=floor(f1);
      m=(f1-h)*60.0;
      s=(m-floor(m))*60.0;

      setTime(h, floor(m), floor(s), d, mo, y);   //set current system time
      Teensy3Clock.set(now());               //set Teensy time
    }

    void setDateFromGPS()
    {
      double JD=julian(gps.date.year(),gps.date.month(),gps.date.day());
      double LMT=(gps.time.hour()+(gps.time.minute()/60.0)+(gps.time.second()/3600.0))-_timeZone;
      if (LMT < 0)   { LMT+=24.0; JD-=1; }
      if (LMT >= 24) { LMT-=24.0; JD+=1; }
      set(JD, LMT);
    }

    bool getLocation(double *systemLatitude, double *systemLongitude)
    {
        if (!isGPSReady()) return false;
        *systemLatitude = latitude;
        *systemLongitude = longitude;
        return true;
    }


    
    private:
    TinyGPSPlus gps;
    bool CorrectLocationCaptured = false;
    bool SingleAcquisition = true;
    double _timeZone = 0.0;
    Stream *SerialGPS;
    double latitude = 0;
    double longitude = 0;
    double JD = 0; // Julian date
    double LMT = 0; // Local mean time
    
    static time_t getTeensy3Time()
    {
        return Teensy3Clock.get();
    }
};

