// Platform setup ------------------------------------------------------------------------------------

// This platform doesn't support true double precision math
#define HAL_NO_DOUBLE_PRECISION

// This is for ~16MHz AVR processors or similar.
#define HAL_SLOW_PROCESSOR

// Set default timer mode unless specified
#ifndef STEP_WAVE_FORM
  #define STEP_WAVE_FORM PULSE
#endif

// Lower limit (fastest) step rate in uS for this platform (in SQW mode)
#define HAL_MAXRATE_LOWER_LIMIT 76.8

// Width of step pulse
#define HAL_PULSE_WIDTH 10000

// New symbols for the Serial ports so they can be remapped if necessary -----------------------------
#ifndef MEGA2560_ARDUINO_SERIAL_ON
  // SerialA is always enabled, SerialB and SerialC are optional
  #define HAL_SERIAL_B_ENABLED       // Enable support for RX1/TX1

  // don't enable serial C on a Classic board since pins are used
  #if PINMAP != Classic
    #define HAL_SERIAL_C_ENABLED
    #define HAL_SERIAL_C_SERIAL2     // Use RX2/TX2 for channel C (defaults to RX3/TX3 otherwise.)
  #endif

  // this tells OnStep that a .transmit() method needs to be called to send data
  #define HAL_SERIAL_TRANSMIT

  // Use low overhead serial
  #include "HAL_Serial.h"
#else
  // New symbols for the Serial ports so they can be remapped if necessary -----------------------------
  #define SerialA Serial
  // SerialA is always enabled, SerialB and SerialC are optional
  #define HAL_SERIAL_B_ENABLED
  #define SerialB Serial1
  // don't enable serial C on a Classic board since pins are used
  #if PINMAP != Classic
    #define HAL_SERIAL_C_ENABLED
    #define SerialC Serial2
  #endif
#endif

// New symbol for the default I2C port -------------------------------------------------------------
#define HAL_Wire Wire

// Non-volatile storage ------------------------------------------------------------------------------
#if defined(NV_AT24C32)
  #ifdef E2END
    #undef E2END
  #endif
  #include "../drivers/NV_I2C_EEPROM_AT24C32.h"
#elif defined(NV_MB85RC256V)
  #ifdef E2END
    #undef E2END
  #endif
  #include "../drivers/NV_I2C_FRAM_MB85RC256V.h"
#else
  #include "../drivers/NV_EEPROM.h"
#endif

//--------------------------------------------------------------------------------------------------
// General purpose initialize for HAL
void HAL_Init(void) {
}

//--------------------------------------------------------------------------------------------------
// Internal MCU temperature (in degrees C)
float HAL_MCU_Temperature(void) {
  return -999;
}

//--------------------------------------------------------------------------------------------------
// Initialize timers

// Enable a pseudo low priority level for Timer1 (sidereal clock) so the
// critical motor ISR's don't get delayed by the big slow sidereal clock ISR
#define HAL_USE_NOBLOCK_FOR_TIMER1

extern long int siderealInterval;
extern void SiderealClockSetInterval (long int);

// Init sidereal clock timer
void HAL_Init_Timer_Sidereal() {
  SiderealClockSetInterval(siderealInterval);
}

// Init Axis1 and Axis2 motor timers and set their priorities

// Note the granularity below, these run at 0.5uS per tick with 16bit depth
#define HAL_FIXED_PRESCALE_16BIT_MOTOR_TIMERS

void HAL_Init_Timers_Motor() {
  // ~0 to 0.032 seconds (31 steps per second minimum, granularity of timer is 0.5uS) /8  pre-scaler
  TCCR3B = (1 << WGM12) | (1 << CS11);
  TCCR3A = 0;
  TIMSK3 = (1 << OCIE3A);

  // ~0 to 0.032 seconds (31 steps per second minimum, granularity of timer is 0.5uS) /8  pre-scaler
  TCCR4B = (1 << WGM12) | (1 << CS11);
  TCCR4A = 0;
  TIMSK4 = (1 << OCIE4A);
}

//--------------------------------------------------------------------------------------------------
// Set timer1 to interval (in microseconds*16), for the 1/100 second sidereal timer

void Timer1SetInterval(long iv, double rateRatio) {
  iv=round(((double)iv)/rateRatio);

  TCCR1B = 0; TCCR1A = 0;
  TIMSK1 = 0;

  // set compare match register to desired timer count:
  if (iv<65536) { TCCR1B |= (1 << CS10); } else {
  iv=iv/8;
  if (iv<65536) { TCCR1B |= (1 << CS11); } else {
  iv=iv/8;
  if (iv<65536) { TCCR1B |= (1 << CS10); TCCR1B |= (1 << CS11); } else {
  iv=iv/4;  
  if (iv<65536) { TCCR1B |= (1 << CS12); } else {
  iv=iv/4;
  if (iv<65536) { TCCR1B |= (1 << CS10); TCCR1B |= (1 << CS12); 
  }}}}}
  
  OCR1A = iv-1;
  // CTC mode
  TCCR1B |= (1 << WGM12);
  // timer compare interrupt enable
  TIMSK1 |= (1 << OCIE1A);
}

//--------------------------------------------------------------------------------------------------
// Re-program interval for the motor timers

// interval (r) is in (in microseconds*2) due to using HAL_FIXED_PRESCALE_16BIT_MOTOR_TIMERS 
// for a maximum period of 0.032 seconds (16 bit, 65535 maximum interval)

// prepare to set Axis1/2 hw timers to interval (in microseconds*16), maximum time is about 134 seconds
void PresetTimerInterval(long iv, float TPSM, volatile uint32_t *nextRate, volatile uint16_t *nextRep) {
  iv=iv/8L;
  // 0.0327 * 4096 = 134.21s
  uint32_t i=iv; uint16_t t=1; while (iv>65536L) { t*=2; iv=i/t; if (t==4096) { iv=65535L; break; } }
  cli(); *nextRate=(iv * TPSM) - 1L; *nextRep=t; sei();
}

// Must work from within the motor ISR timers, in microseconds*(F_COMP/1000000.0) units
#define QuickSetIntervalAxis1(r) (OCR3A=r)
#define QuickSetIntervalAxis2(r) (OCR4A=r)

// --------------------------------------------------------------------------------------------------
// Fast port writing help

#define CLR(x,y) (x&=(~(1<<y)))
#define SET(x,y) (x|=(1<<y))
#define TGL(x,y) (x^=(1<<y))

// We use standard #define's to do **fast** digitalWrite's to the step and dir pins for the Axis1/2 stepper drivers
#define Axis1StepPinHIGH SET(Axis1StepPORT, Axis1StepBit)
#define Axis1StepPinLOW CLR(Axis1StepPORT, Axis1StepBit)
#define Axis1DirPinHIGH SET(Axis1DirPORT, Axis1DirBit)
#define Axis1DirPinLOW CLR(Axis1DirPORT, Axis1DirBit)

#define Axis2StepPinHIGH SET(Axis2StepPORT, Axis2StepBit)
#define Axis2StepPinLOW CLR(Axis2StepPORT, Axis2StepBit)
#define Axis2DirPinHIGH SET(Axis2DirPORT, Axis2DirBit)
#define Axis2DirPinLOW CLR(Axis2DirPORT, Axis2DirBit)
