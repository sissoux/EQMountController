// ---------------------------------------------------------------------------------------------------------------------------------
// Configuration for OnStep Ethernet Add-on

/*
 *          For more information on setting OnStep up see http://www.stellarjourney.com/index.php?r=site/equipment_onstep 
 *                      and join the OnStep Groups.io at https://groups.io/g/onstep
 * 
 *           *** Read the compiler warnings and errors, they are there to help guard against invalid configurations ***
*/

// ---------------------------------------------------------------------------------------------------------------------------------
// ADJUST THE FOLLOWING TO CONFIGURE YOUR ADD-ON'S FEATURES ------------------------------------------------------------------------
// <-Req'd = always must set, <-Often = usually must set, Option = optional, Adjust = adjust as req'd, Infreq = infrequently changed

//      Parameter Name              Value   Default  Notes                                                                      Hint
// ETHERNET HARDWARE ---------------------------------------------------------------------------------------------------------------
#define W5500                         OFF //    OFF, ON If using W5500 Ethernet Adapter: RST ctrl Pin9, SPI default pins,     Option
                                          //         CS on Pin10. OFF for W5100 adapter. Uses standard Arduino libraries.

// SERIAL PORTS --------------------------------------------------------------------------------------------------------------------
#define SERIAL_BAUD_DEFAULT          9600 //   9600, Common baud rates for these parameters are 9600,19200,57600,115200.      Infreq
#define SERIAL_BAUD                 57600 //  57600, Use 19200 if talking to a Mega2560 OnStep                               <-Req'd
                                          //         At startup this firmware will attempt to switch OnStep's baud rate to this
                                          //         faster speed (vs. SERIAL_BAUD_DEFAULT) and AFTER success, start WiFi, etc.

// DISPLAY -------------------------------------------------------------------------------------------------------------------------
#define DISPLAY_LANGUAGE             L_en //   L_en, English. Specify language with two letter country code, if supported.    Adjust
#define DISPLAY_WEATHER               OFF //    OFF, ON Shows weather/ambient conditions (from OnStep) on status page.        Option
#define DISPLAY_INTERNAL_TEMPERATURE  OFF //    OFF, ON for internal MCU temperature display.                                 Option
#define DISPLAY_HIGH_PRECISION_COORDS OFF //    OFF, ON for pull high precision coordinates from OnStep.                      Option
#define DISPLAY_SPECIAL_CHARS          ON //     ON, For standard ASCII special symbols (compatibility.)                      Infreq
#define DISPLAY_ADVANCED_CHARS         ON //     ON, For standard "RA/Dec" instead of symbols.                                Infreq
#define DISPLAY_HIGH_PRECISION_COORDS OFF //    OFF, ON for high precision coordinate display on status page.                 Infreq

// COMMAND CHANNELS ----------------------------------------------------------------------------------------------------------------
#define MONITOR_GUIDE_COMMANDS        OFF //    OFF, Allow error reporting to also monitor guide commands.                    Infreq

// ENCODER SUPPORT -----------------------------------------------------------------------------------------------------------------
#define AXIS1_ENC                     OFF //    OFF, CWCCW, AB. RA/Azm Axis on Pin 5 (A or CW) and Pin 6 (B or CCW,)          Option
#define AXIS1_ENC_REVERSE             OFF //    OFF, ON to reverse the count direction.                                       Adjust
#define AXIS1_ENC_TICKS_DEG     555.55555 // 555.55, n, (ticks/degree.) Encoder ticks per degree.                             Adjust
#define AXIS1_ENC_DIFF_LIMIT          900 //    900, n, (arcsec.) Maximum difference between encoder and OnStep before sync.  Adjust

#define AXIS2_ENC                     OFF //    OFF, CWCCW, AB. Dec/Alt Axis on Pin 7 (A or CW) and Pin 8 (B or CCW)          Option
#define AXIS2_ENC_REVERSE             OFF //    OFF, ON to reverse the count direction.                                       Option
#define AXIS2_ENC_TICKS_DEG      13.33333 // 13.333, n, (ticks/degree.) Encoder ticks per degree.                             Adjust
#define AXIS2_ENC_DIFF_LIMIT          900 //    900, n, (arcsec.) Maximum difference between encoder and OnStep before sync.  Adjust
                                                     
#define ENCODERS_AUTO_SYNC            OFF //    OFF, ON, Enable support for auto sync of OnStep to encoder values.            Adjust

// ENCODER RATE CONTROL
#define AXIS1_ENC_RATE_CONTROL        OFF //    OFF, ON Rate control for RA high resolution encoder. EQ mounts only.          Infreq
#define AXIS1_ENC_INTPOL_COS          OFF //    OFF, ON enables cosine compensation feature.                                  Infreq
#define AXIS1_ENC_RATE_AUTO           OFF //    OFF, n, (Worm period in seconds.) Adjusts avg encoder pulse rate to account   Option
                                          //         for skew in the average guide rate over the last worm period.            Option
#define AXIS1_ENC_BIN_AVG             OFF //    OFF, n, (Number of bins.)  Enables binned rolling average feature.            Option

// AUXILLARY SWITCH/FEATURE CONTROL ------------------------------------------------------------------------------------------------
// *** Warning: only OnStep Aux pins that are unused for other purposes should be assigned! ***
#define SW0_OFF                           //   _OFF, "Name" for Aux0 feature on Control webpage, provides On/Off control.     Adjust
#define SW1_OFF                           //   _OFF, "Name" for Aux1 feature on Control webpage, provides On/Off control.     Adjust
#define SW2_OFF                           //   _OFF, "Name" for Aux2 feature on Control webpage, provides On/Off control.     Adjust
#define SW3_OFF                           //   _OFF, "Name" for Aux3 feature on Control webpage, provides On/Off control.     Adjust
#define SW4_OFF                           //   _OFF, "Name" for Aux4 feature on Control webpage, provides On/Off control.     Adjust
#define SW5_OFF                           //   _OFF, "Name" for Aux5 feature on Control webpage, provides On/Off control.     Adjust
#define SW6_OFF                           //   _OFF, "Name" for Aux6 feature on Control webpage, provides On/Off control.     Adjust
#define SW7_OFF                           //   _OFF, "Name" for Aux7 feature on Control webpage, provides On/Off control.     Adjust
#define SW8_OFF                           //   _OFF, "Name" for Aux8 feature on Control webpage, provides On/Off control.     Adjust

// AUXILLARY ANALOG/FEATURE CONTROL ------------------------------------------------------------------------------------------------
// *** Warning: only OnStep Aux pins that are unused for other purposes should be assigned! ***
#define AN3_OFF                           //   _OFF, "Name" for Aux3 feature on Control webpage, provides 0..100% PWM.        Adjust
#define AN4_OFF                           //   _OFF, "Name" for Aux4 feature on Control webpage, provides 0..100% PWM.        Adjust
#define AN5_OFF                           //   _OFF, "Name" for Aux5 feature on Control webpage, provides 0..100% PWM.        Adjust
#define AN6_OFF                           //   _OFF, "Name" for Aux6 feature on Control webpage, provides 0..100% PWM.        Adjust
#define AN7_OFF                           //   _OFF, "Name" for Aux7 feature on Control webpage, provides 0..100% PWM.        Adjust
#define AN8_OFF                           //   _OFF, "Name" for Aux8 feature on Control webpage, provides 0..100% PWM.        Adjust

// ETHERNET SETTINGS ---------------------------------------------------------------------------------------------------------------
// Enter a unique MAC address for your controller if you like:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// The IP addresses below will be dependent on your local network:
IPAddress ip(192, 168, 1, 55);
IPAddress myDns(192,168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// THAT'S IT FOR USER CONFIGURATION!

// -------------------------------------------------------------------------------------------------------------------------

#define Ser Serial1 // Default=Serial1, This is the hardware serial port where OnStep is attached

// ---------------------------------------------------------------------------------------------------------------------------------
