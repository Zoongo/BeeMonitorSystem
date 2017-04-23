//--------------------------------------------
//--- BeeMonitor transmitter
//--- Board: SODAQ Autonomo
//--- Communication: XBee Series1
//--------------------------------------------
// Copyright (C) 2015-2017 by Tomas Ivansky
//--------------------------------------------
// GNU GPL v3 License
// ------------------
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// The license can be viewed at
// http://www.gnu.org/licenses/gpl-3.0.txt
//--------------------------------------------
//--- Important parameters setup
//--------------------------------------------

//--- Communication parameters
#ifdef XBEE
   //--- XBee communication parameters
   #define PAUSE_BETWEEN_PACKETS 4000       // Period between packets in miliseconds
   #define NUMBER_ATTEMPTS 3                // Number of unsuccesfull sending attepts
   #define GATEWAY 0x1234                   // 16bit XBEEs address of Gateway
#endif
#ifdef GPRSBEE
   //--- GPRSBee communication parameters
   #define APN "internet.t-mobile.cz"                               // APN of your provider
   #define GET_URL "http://geek4bee.com/insert_mysql.php"           // URL of the PHP 
   #define LENGTHHTTP 120                                           // HTTP GET string length
   #define LENGTHSTRING 20                                          // Payload component max. length
   #define URL_TEMPLATE "%s?node=%s&value=%s&position=%s&table=%s"  // Structure of URL for the GET
   #define NODE_ID "1"                                              // It corresponds with ID record in the MySQL table positions on the data-server
#endif

//--- Communication period setup 
#define HOUR_TX 0                        // Hour fragment of tx period
#define MINUTE_TX 1                     // Minute fragment of tx period
#define SECOND_TX 0                      // Second fragment of tx period

//--- Battery status parameters
#define ADC_AREF 3.3
#define BATVOLTPIN A0
#define BATVOLT_R1 4.7
#define BATVOLT_R2 10

//--- Weight statistics
#define WEIGHT_SAMPLES_NUMBER  10          // Number of samples for median calculation
#define WEIGHT_TIME_BETWEEN_MEASUREMENTS 3 // Wait between weight samples in second                          

//--- Weight measurement calculation methods
#define KG_DIVIDER 1                       // Method for good linearity based on KG divider
#define REGRESSION 2                       // Method based on polynomic regression

//--- Weight statistic calculation methods
#define MEDIAN 1                           // Median is reported
#define AVERAGE 2                          // Average is reported

//--- Scale sensor calibration and temperature correction
//--- Parameters for REGRESSIONS method
#define  WGHT_E_A  2.79888E-18             // Regression coeficient 
#define  WGHT_E_B -6.417E-11               // Regression coeficient 
#define  WGHT_E_C  0.000447765             // Regression coeficient 
#define  WGHT_E_D  -897.6871728            // Regression coeficient 
#define  COMP_TEMP  15.0                   // Calibration temperature (degrees of Celsius)
#define  COMP_TEMP_PAR  -0.0279            // Temperature calibration coeficient
//--- Parameters for KG_DIVIDER method
#define LOAD_CELL_ZERO_OFFSET 45169;       // Raw sensor data for zero kg load 
#define LOAD_CELL_KG_DIVIDER  458770;      // Value of calculated kg divider

#define  TEMPERATURE_PRECISION 11          // Dallas temperature sensor precision

//--- Pins assignment
#define ONE_WIRE_BUS_PIN A12               // One wire
#define HX_DATA_PIN A4                     // HX711
#define HX_CLOCK_PIN A5                    // HX711
#define LED_PIN 13                         // Debug blinking LED

// Error reporting
#define MAX_ERRORS 10                      // Max. number of errors collected during one session
