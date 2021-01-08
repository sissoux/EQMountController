// -------------------------------------------------------------------------------------------------
// Pin map for OnStep MiniPCB (Teensy3.0, 3.1, 3.2, and 4.0)

#if defined(__MK20DX256__) || defined(_mk20dx128_h_) || defined(__MK20DX128__) || defined(__IMXRT1052__) || defined(__IMXRT1062__)

// The multi-purpose pins (Aux3..Aux8 can be analog pwm/dac if supported)
#define Aux0                 19
#define Aux1                 18
#define Aux2                  5
#define Aux3                  4     // should be ok as pwm analog output (w/#define Aux3_Analog)
#define Aux4                 22     // should be ok as pwm analog output (w/#define Aux4_Analog)
#if !defined(_mk20dx128_h_) && !defined(__MK20DX128__) && !defined(__IMXRT1052__) && !defined(__IMXRT1062__)
  #define Aux5              A14     // true analog output
#endif
#define Aux5_Analog

// Misc. pins
#ifndef OneWirePin
  #define OneWirePin         24     // Default Pin for one wire bus
#endif
#if PINMAP == MiniPCB13
  #define ESP8266Gpio0Pin     2     // ESP8266 GPIO0 (Dir2)
  #define ESP8266RstPin    Aux2     // ESP8266 RST
#else
  #define ESP8266Gpio0Pin  Aux1     // ESP8266 GPIO0 or SPI MISO/Fault
  #define ESP8266RstPin    Aux2     // ESP8266 RST or SPI MISO/Fault
#endif

// The PEC index sense is a logic level input, resets the PEC index on rising edge then waits for 60 seconds before allowing another reset
#define PecPin               23
#define AnalogPecPin         23     // PEC Sense, analog or digital

// The status LED is a two wire jumper with a 10k resistor in series to limit the current to the LED
#define LEDnegPin          Aux0     // Drain
#define LEDneg2Pin         Aux4     // Drain
#define ReticlePin         Aux4     // Drain

// For a piezo buzzer
#define TonePin              29     // Tone

// The PPS pin is a 3.3V logic input, OnStep measures time between rising edges and adjusts the internal sidereal clock frequency
#define PpsPin               28     // PPS time source, GPS for example

#define LimitPin           Aux3     // The limit switch sense is a logic level input normally pull high (2k resistor,) shorted to ground it stops gotos/tracking

// Axis1 RA/Azm step/dir driver
#define Axis1DirPin          21     // Dir
#define Axis1StepPin         20     // Step

#define Axis1_EN             14     // Enable
#define Axis1_M0             15     // Microstep Mode 0 or SPI MOSI
#define Axis1_M1             16     // Microstep Mode 1 or SPI SCK
#define Axis1_M2             17     // Microstep Mode 2 or SPI CS or Decay Mode
#define Axis1_M3           Aux1     // ESP8266 GPIO0 (option on MiniPCB) or SPI MISO/Fault
#define Axis1ModePin   Axis1_M2     // Decay mode
#define Axis1FaultPin      Aux1     // SPI MISO/Fault
#define Axis1HomePin       Aux3     // Sense home position

#define Axis2DirPin           2     // Dir (ESP8266 GPIO0 on MiniPCB13)
#define Axis2StepPin          3     // Step

// Axis2 Dec/Alt step/dir driver
#define Axis2_EN              9     // Enable
#define Axis2_M0              8     // Microstep Mode 0 or SPI MOSI
#define Axis2_M1              7     // Microstep Mode 1 or SPI SCK
#define Axis2_M2              6     // Microstep Mode 2 or SPI CS or Decay Mode
#if PINMAP == MiniPCB13
  #define Axis2_M3         Aux1     // SPI MISO/Fault or I2C SDA
#else
  #define Axis2_M3         Aux2     // ESP8266 RST or SPI MISO/Fault
#endif
#define Axis2ModePin   Axis2_M2     // Decay mode
#define Axis2FaultPin  Axis2_M3
#define Axis2HomePin       Aux4     // Sense home position

// For rotator stepper driver
#define Axis3_EN             -1     // ENable
#define Axis3StepPin         30     // Step
#define Axis3DirPin          33     // Dir

// For focuser1 stepper driver
#define Axis4_EN             -1     // ENable
#define Axis4StepPin         31     // Step
#define Axis4DirPin          32     // Dir

// For focuser2 stepper driver
#define Axis5_EN             -1     // ENable
#define Axis5StepPin         30     // Step
#define Axis5DirPin          33     // Dir

// ST4 interface
#define ST4RAw               10     // ST4 RA- West
#define ST4DEs               11     // ST4 DE- South
#define ST4DEn               12     // ST4 DE+ North
#define ST4RAe               13     // ST4 RA+ East

#else
#error "Wrong processor for this configuration!"

#endif
