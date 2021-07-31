// -------------------------------------------------------------------------------------------------
// Pin map for OnStep MiniPCB (Teensy3.0, 3.1, 3.2, and 4.0)

#if defined(__MK20DX256__) || defined(_mk20dx128_h_) || defined(__MK20DX128__) || defined(__IMXRT1052__) || defined(__IMXRT1062__)

// The multi-purpose pins (Aux3..Aux8 can be analog pwm/dac if supported)
#define Aux0               26     
#define Aux1               26  
#define Aux2               26   
#define Aux3               26
#define Aux4               26
#if !defined(_mk20dx128_h_) && !defined(__MK20DX128__) && !defined(__IMXRT1052__) && !defined(__IMXRT1062__)
  #define Aux5              A14     // true analog output
#endif
#define Aux5_Analog

// Misc. pins
#ifndef OneWirePin
  #define OneWirePin         24     // Default Pin for one wire bus
#endif


// The PEC index sense is a logic level input, resets the PEC index on rising edge then waits for 60 seconds before allowing another reset
#define PecPin               23
#define AnalogPecPin         23     // PEC Sense, analog or digital

// The status LED is a two wire jumper with a 10k resistor in series to limit the current to the LED
#define LEDnegPin          22     // Drain
#define LEDpos2Pin         OFF    // Drain
#define ReticlePin         Aux4   // Drain

// For a piezo buzzer
#define TonePin              29     // Tone

// The PPS pin is a 3.3V logic input, OnStep measures time between rising edges and adjusts the internal sidereal clock frequency
#define PpsPin               28     // PPS time source, GPS for example

#define LimitPin           Aux3     // The limit switch sense is a logic level input normally pull high (2k resistor,) shorted to ground it stops gotos/tracking

// Axis1 RA/Azm step/dir driver

//#define Axis1DirPin          2     // Dir
//#define Axis1StepPin         3     // Step
//#define Axis1_EN             5     // Enable
//#define Axis1_M0             11    // Microstep Mode 0 or SPI MOSI
//#define Axis1_M1             13    // Microstep Mode 1 or SPI SCK
//#define Axis1_M2             4     // Microstep Mode 2 or SPI CS or Decay Mode
//#define Axis1_M3             12    // ESP8266 GPIO0 (option on MiniPCB) or SPI MISO/Fault
//#define Axis1ModePin   Axis1_M2    // Decay mode
//#define Axis1FaultPin        12    // SPI MISO/Fault
//#define Axis1HomePin       Aux3    // Sense home position


#define Axis1_EN             5      // Enable
#define Axis1_M0             11     // Microstep Mode 0 or SPI MOSI
#define Axis1_M1             13     // Microstep Mode 1 or SPI SCK
#define Axis1_M2             4     // Microstep Mode 2 or SPI CS or Decay Mode
#define Axis1_M3             12     // ESP8266 GPIO0 (option on MiniPCB) or SPI MISO/Fault
#define Axis1_STEP           3      // Step
#define Axis1_DIR            2      // Dir
#define Axis1_DECAY    Axis1_M2     // Decay mode
#define Axis1_FAULT          12     // SPI MISO/Fault
#define Axis1_HOME         Aux3     // Sense home position

// Axis2 Dec/Alt step/dir driver

//#define Axis2DirPin           6     // Dir
//#define Axis2StepPin          9     // Step
//#define Axis2_EN              10    // Enable
//#define Axis2_M0              11    // Microstep Mode 0 or SPI MOSI
//#define Axis2_M1              13    // Microstep Mode 1 or SPI SCK
//#define Axis2_M2              20    // Microstep Mode 2 or SPI CS or Decay Mode
//#define Axis2_M3              12    // SPI MISO/Fault or I2C SDA
//#define Axis2ModePin          Axis2_M2     // Decay mode
//#define Axis2FaultPin         12
//#define Axis2HomePin          Aux4     // Sense home position

#define Axis2_EN              10     // Enable
#define Axis2_M0              11     // Microstep Mode 0 or SPI MOSI
#define Axis2_M1              13     // Microstep Mode 1 or SPI SCK
#define Axis2_M2              20     // Microstep Mode 2 or SPI CS or Decay Mode
#define Axis2_M3              12     // SPI MISO/Fault or I2C SDA
#define Axis2_STEP            9      // Step
#define Axis2_DIR             6      // Dir (ESP8266 GPIO0 on MiniPCB13)
#define Axis2_DECAY    Axis2_M2      // Decay mode
#define Axis2_FAULT           12
#define Axis2_HOME         Aux4      // Sense home position


// For rotator stepper driver
#define Axis3_EN            OFF     // ENable
#define Axis3_STEP           30     // Step
#define Axis3_DIR            33     // Dir

// For focuser1 stepper driver
#define Axis4_EN             17     // ENable
#define Axis4_STEP           18     // Step
#define Axis4_DIR            19     // Dir

// For focuser2 stepper driver
#define Axis5_EN            OFF     // ENable
#define Axis5_STEP           30     // Step
#define Axis5_DIR            33     // Dir

// ST4 interface

// ST4 interface
#define ST4RAw               OFF     // ST4 RA- West
#define ST4DEs               OFF     // ST4 DE- South
#define ST4DEn               OFF     // ST4 DE+ North
#define ST4RAe               OFF     // ST4 RA+ East

#else
#error "Wrong processor for this configuration!"

#endif
