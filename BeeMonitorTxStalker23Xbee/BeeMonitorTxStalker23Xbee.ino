//--------------------------------------------
//--- BeeMonitor transmitter
//--- Version for: 
//--- Seeeduino Stalker 2.3
//--- XBee Series1
//--- Two temperature sensors + two load scale sensors
//--------------------------------------------

#include <avr/sleep.h>
#include <avr/power.h>
#include <Wire.h>
#include <DS3231.h>
#include <XBee.h>
#include <HX711.h>
#include <Battery.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//--- Error messagess used for remote logging
#define ERR_READING_DALLAS_TEMP         "'Ext temp sensor dead'"
#define ERR_FOUND_DALLAS_TEMP           "'Ext temp sensor not found'"
#define ERR_RESUSCITATION_ONEWIRE_BUS   "'One wire bus restart'"
#define ERR_READING_ONBOARD_TEMP        "'Reading onboard temperature'"
#define ERR_OVERHEATING_ONBOARD_TEMP    "'Onboard temperature too high'"
#define ERR_LOW_VOLTAGE                 "'Low voltage'"
#define ERR_HUMIDITY_SENSOR             "'Humidity sensor'"
#define ERR_ATMPRESSURE_SENSOR          "'Atmo pressure sensor'"
#define ERR_TEMP_CORR_INT               "'Int sensor used for corr'"
#define ERR_TEMP_CONST                  "'Const 15C used for corr'"

//--- Communication, sleeping and energy parameters
#define SLEEP_PERIOD 1800          // Sleeping period in second. It defines period between transmissions
#define WAIT_ON_THE_START 60       // Waiting period on the start in seconds
#define PAUSE_BETWEEN_PACKETS 4000 // Period between packets in miliseconds
#define NUMBER_ATTEMPTS 3          // Number of unsuccesfull sending attepts 
#define GATEWAY 0x1234             // 16bit XBEEs address of Gateway
#define BATTERY_UPPER 4.15         // Upper battery voltager for the battery percentage calculation
#define BATTERY_LOWER 3.6          // Upper battery voltager for the battery percentage calculation
#define TEMPERATURE_PRECISION 9    // Dallas temperature sensor precision

#define OK 0
#define PACKET_NOT_SENT -1
#define PACKET_SENT_OK 0
#define RECORD_SENT_OK -99

//--- Pins assignment
#define ONE_WIRE_BUS_PIN 7
#define XBEE_SLEEP_PIN 3
#define INTERRUPT_PIN 2
#define EXTERN_POWER_PIN 4
#define SCALE1_LOW_PIN A0
#define SCALE1_HIGH_PIN A1
#define SCALE2_LOW_PIN A2
#define SCALE2_HIGH_PIN A3

//--- Scale sensor calibration and temperature correction
//--- parameters
#define  WGHT_E_A1  2.79888E-18
#define  WGHT_E_B1 -6.417E-11
#define  WGHT_E_C1  0.000447765
#define  WGHT_E_D1  -897.6871728
#define  COMP_TEMP1  15.0
#define  COMP_TEMP_PAR1  -0.0279
#define  WGHT_E_A2  3.44083E-19
#define  WGHT_E_B2 -7.60712E-12
#define  WGHT_E_C2  2.2515E-5
#define  WGHT_E_D2  140.7635036
#define  COMP_TEMP2  15.0
#define  COMP_TEMP_PAR2  0.0047

//The following code is taken from sleep.h as Arduino Software v22 (avrgcc) 
#define sleep_bod_disable() \
{ \
  uint8_t tempreg; \
  __asm__ __volatile__("in %[tempreg], %[mcucr]" "\n\t" \
                       "ori %[tempreg], %[bods_bodse]" "\n\t" \
                       "out %[mcucr], %[tempreg]" "\n\t" \
                       "andi %[tempreg], %[not_bodse]" "\n\t" \
                       "out %[mcucr], %[tempreg]" \
                       : [tempreg] "=&d" (tempreg) \
                       : [mcucr] "I" _SFR_IO_ADDR(MCUCR), \
                         [bods_bodse] "i" (_BV(BODS) | _BV(BODSE)), \
                         [not_bodse] "i" (~_BV(BODSE))); \
}

//--- Structure for sensor data processing
typedef struct  {
  String tempHive2;
  String wghtHive1;
  String wghtHive2;
} SensorRecord;

//--- Structure for internal telemetry processing
typedef struct {
  String batteryVoltage;
  String batteryPercentage;
  String tempInt;
} TelemetryRecord;

//--- Function prototypes
  int readSensors(SensorRecord *sensor);
  int readTelemetry(TelemetryRecord *telemetry);
  int sendSensors(SensorRecord *sensor);
  int sendTelemetry(TelemetryRecord *telemetry);

  DS3231 RTC;                                             //Create RTC object for DS3231
  static DateTime interruptTime;                          // Interrupt time
  Battery battery;                                        // Battery
  XBee xbee = XBee();                                     // XBee Series 1 
  TxStatusResponse txStatus = TxStatusResponse();         // Transmittion response
  Rx64Response rx64 = Rx64Response();                     // Receiving response 64bit packet
  Rx16Response rx16 = Rx16Response();                     // Receiving response 16bit packet
  byte Int = INTERRUPT_PIN;                               // Define Interrupt pin
  boolean clockInterrupt= false;                          //Flag if a clock interrupt occurred
  byte XBEE_sleep = XBEE_SLEEP_PIN;                       //XBEE sleep pin

  HX711 scale1(SCALE1_HIGH_PIN, SCALE1_LOW_PIN);          // Weight scale1
  HX711 scale2(SCALE2_HIGH_PIN, SCALE2_LOW_PIN);          // Weight scale2
  OneWire oneWire(ONE_WIRE_BUS_PIN);                      // One wire for Dallas sensors
  DallasTemperature sensors(&oneWire);                    // Dallas sensors object

  // Addresses of Dallas sensors
  DeviceAddress dallasProbe;

  //Array of strings for remote error reporting
  String error[10];
  int num_err = 0;
  #define error(s) error_P(PSTR(s))                         // Store error strings in flash to save RAM

//--------------------------------------------
//--- Mandatory setup method
//--------------------------------------------
void setup () 
{
     Serial.begin(9600);
     sensors.begin();
     Wire.begin();
     //Initialize INT0 pin for accepting interrupts
     PORTD |= 0x04; 
     DDRD &=~ 0x04;
     pinMode(EXTERN_POWER_PIN, INPUT);  
     if (!sensors.getAddress(dallasProbe, 0)){
         addError(ERR_FOUND_DALLAS_TEMP);
     }
     sensors.setResolution(dallasProbe, TEMPERATURE_PRECISION);
     pinMode(XBEE_SLEEP_PIN, OUTPUT);                            // Set XBEE sleep pin as output
     xbee.setSerial(Serial);                                     // Serial assigned for XBEE
     digitalWrite(XBEE_SLEEP_PIN,LOW);                           // Wake up XBEE Module  
     scale1.set_scale();                                         // Scale1 init
     scale2.set_scale();                                         // Scale2 init
     RTC.begin();                                           
     attachInterrupt(0, INT0_ISR, LOW);                          // Only LOW level interrupt can wake up from PWR_DOWN
     set_sleep_mode(SLEEP_MODE_PWR_DOWN);                        // Sleep mode setup
     DateTime  start = RTC.now();                                // Real-time clock start
     interruptTime = DateTime(start.get() + WAIT_ON_THE_START);  // Interrupt time calculation
}


//--------------------------------------------
//--- Mandatory loop method
//--------------------------------------------
void loop () 
{
    TelemetryRecord telemetry;
    SensorRecord sensor;
        
// *** EXECUTIVE ROUTINES ***  
    initErrors();
    RTC.clearINTStatus();                                                                       // This function call is  a must to bring /INT pin HIGH after an interrupt.
    RTC.enableInterrupts(interruptTime.hour(),interruptTime.minute(),interruptTime.second());   // Set the interrupt at (h,m,s)
    attachInterrupt(0, INT0_ISR, LOW);                                                          // Enable INT0 interrupt (as ISR disables interrupt). This strategy is required to handle LEVEL triggered interrupt
    digitalWrite(XBEE_sleep, LOW);                                                              // Wake up XBEE module
    scale1.power_up();                                                                          // Wake up scale1 sensor
    delay(1000);
    scale2.power_up();                                                                          // Wake up scale2 sensor
    delay(1000);
    if(readSensors(&sensor) == OK){                                                             // Reads sensor data
       sendSensors(&sensor);                                                                    // Sends sensor data via XBee
       delay(1000); 
    }
    if(readTelemetry(&telemetry) == OK){                                                        // Reads board telemetry
       sendTelemetry(&telemetry);                                                               // Sends telemetry data via XBee
        delay(1000); 
    }
    sendErrors();                                                                              // Sends identified errors
    
// *** POWER DOWN ROUTINES ***
    scale1.power_down();                                                                        // Scale1 power down
    scale2.power_down();                                                                        // Scale2 power down
    digitalWrite(XBEE_sleep, HIGH);                                                             //Put XBEE sleep pin high again
    cli(); 
    sleep_enable();                                                                             // Set sleep enable bit
    sleep_bod_disable();                                                                        // Disable brown out detection during sleep. Saves more power
    sei();      
    Serial.println("\nSleeping");
    delay(10);                                                                                  // This delay is required to allow print to complete
    //Shut down all peripherals like ADC before sleep. Refer Atmega328 manual
    power_all_disable();                                                                        // This shuts down ADC, TWI, SPI, Timers and USART
    sleep_cpu();                                                                                // Sleep the CPU as per the mode set earlier(power down)  
    sleep_disable();                                                                            // Wakes up sleep and clears enable bit. Before this ISR would have executed
    power_all_enable();                                                                         // This shuts enables ADC, TWI, SPI, Timers and USART
    delay(10);                                                                                  // This delay is required to allow CPU to stabilize
    Serial.println("Awake from sleep");   
    printTimeStamp(); 
    delay(1000);
} 


//--------------------------------------------
// Interrupt service routine for external interrupt on INT0 pin conntected to DS3231 /INT
// Keep this as short as possible. Possibly avoid using function calls
//--------------------------------------------
void INT0_ISR()
{
    detachInterrupt(0); 
    interruptTime = DateTime(interruptTime.get() + SLEEP_PERIOD);  //decide the time for next interrupt, configure next interrupt  
}


//--------------------------------------------
// Prints time-stamp and temperature
//--------------------------------------------
void printTimeStamp() {
    float temp;
    
    RTC.convertTemperature();    //convert current temperature into registers
    temp = RTC.getTemperature(); //Read temperature sensor value   
    DateTime now = RTC.now(); 
    Serial.print(now.date(), DEC);Serial.print(".");
    Serial.print(now.month(), DEC);Serial.print(".");
    Serial.print(now.year(), DEC);Serial.print("  ");
    Serial.print(now.hour(), DEC);Serial.print(":");
    Serial.print(now.minute(), DEC);Serial.print(":");
    Serial.print(now.second(), DEC);
    Serial.print("   ");
    Serial.print(temp);
    Serial.print(" deg C");
    Serial.println();
}


//--------------------------------------------
//--- Get internal temperature of the mainboard 
//--- as string
//--------------------------------------------
String getInternalTemperature(){
  char tempBuffer[8];
  char temperature[6];
  float intTemp;

  intTemp = RTC.getTemperature();
  if(intTemp > 45.0) {
    addError(ERR_OVERHEATING_ONBOARD_TEMP);
  }
  dtostrf(intTemp, 4, 2, temperature);
  sprintf(tempBuffer,"%s",temperature);
  return (tempBuffer);
}


//----------------------------------------
//--- Reads sensors, save values into SensorRecord structure
//----------------------------------------
int readSensors(SensorRecord *sensor) {
  double probeTemperature;
  double rtcTemperature;
  double correctionTemperature;

  rtcTemperature = RTC.getTemperature();
  sensor->wghtHive1 = "";                            
  sensor->wghtHive2 = "";                                                    
  sensor->tempHive2 = "";  
  sensors.requestTemperatures();
  probeTemperature = getTemp(dallasProbe);                                
  sensor->tempHive2 += probeTemperature;                              
  delay(1000);                           
  if((probeTemperature > -60) && (probeTemperature < 85)){
    correctionTemperature = probeTemperature;
  } else {  
      if(rtcTemperature > -60 && rtcTemperature < 85){
        correctionTemperature = rtcTemperature;
        addError(ERR_TEMP_CORR_INT);
      } else {
        correctionTemperature = 15;
        addError(ERR_TEMP_CONST);
      }
  }
  sensor->wghtHive1 += readWeightScale(1, correctionTemperature);        // Weight scale1 reading
  sensor->wghtHive2 += readWeightScale(2, correctionTemperature);        // Weight scale2 reading
  return(OK);
}


//----------------------------------------
//--- Reads scale sensor and return weight in grams
//----------------------------------------
double readWeightScale(int sensor, double temperature) {
double retval = 0;
  double reading1 = 0;
  double correction1 = 0;
  double weight1 = 0;
  double reading2 = 0;
  double weight2 = 0;
  double correction2 = 0;

  switch (sensor) {
    case 1 : {
       // Recalculation based on calibration equation
       reading1 = scale1.get_units(30);
       weight1 = WGHT_E_A1 * pow(reading1,3) + WGHT_E_B1 * pow(reading1,2) + WGHT_E_C1 * reading1 + WGHT_E_D1;
       if(weight1 < 0){
         weight1 = 0;
      }
      // Temperature compensation
      correction1 = COMP_TEMP_PAR1 * (temperature - COMP_TEMP1);
      weight1 = weight1 - correction1;
      if(weight1 < 0){
         weight1 = 0;
      }
      retval = weight1;
      break;
    }
    case 2 : {
       // Recalculation based on calibration equation
       reading2 = scale2.get_units(30);
       weight2 = WGHT_E_A2 * pow(reading2,3) + WGHT_E_B2 * pow(reading2,2) + WGHT_E_C2 * reading2 + WGHT_E_D2;
       if(weight2 < 0){
         weight2 = 0;
      }
      // Temperature compensation
      correction2 = COMP_TEMP_PAR2 * (temperature - COMP_TEMP2);

      weight2 = weight2 - correction2;
      if(weight2 < 0){
         weight2 = 0;
      }
      retval = weight2;
      break;
    }
    default : {
      retval = -9999;
    }
  }
  return(retval);
}


//----------------------------------------
//--- Reads internal telemetry, saves values into TelemetryRecord
//----------------------------------------
int readTelemetry(TelemetryRecord *telemetry) {
    String internalTemperature;
    float batteryVoltage;
    int   batteryPercentage;

  telemetry->batteryVoltage = "";
  telemetry->batteryPercentage = "";
  telemetry->tempInt = "";
  battery.update();
  batteryVoltage = battery.getVoltage();
  if(batteryVoltage < 3.7) {
    addError(ERR_LOW_VOLTAGE);
  }
  internalTemperature = getInternalTemperature();
  batteryPercentage = ((batteryVoltage - BATTERY_LOWER)*100)/(BATTERY_UPPER-BATTERY_LOWER);
  if(batteryPercentage < 0) {
    batteryPercentage = 0;
  }
  if(batteryPercentage > 100) {
    batteryPercentage = 100;
  }
  telemetry->batteryVoltage += batteryVoltage;
  telemetry->batteryPercentage += batteryPercentage;
  telemetry->tempInt += internalTemperature;
  return(OK);
}


//----------------------------------------
//--- Sends sensor data
//----------------------------------------
int sendSensors(SensorRecord *sensor){
   sendPayload("tempHive2",sensor->tempHive2);
   delay(PAUSE_BETWEEN_PACKETS);
   sendPayload("wghtHive1",sensor->wghtHive1);
   delay(PAUSE_BETWEEN_PACKETS);
   sendPayload("wghtHive2",sensor->wghtHive2);
   delay(PAUSE_BETWEEN_PACKETS);
   return(OK);
}


//----------------------------------------
//--- Sends internal telemetry information
//----------------------------------------
int sendTelemetry(TelemetryRecord *telemetry){
  
   sendPayload("tempInt",telemetry->tempInt);
   delay(PAUSE_BETWEEN_PACKETS);
   sendPayload("batteryVoltage",telemetry->batteryVoltage);
   delay(PAUSE_BETWEEN_PACKETS);
   sendPayload("batteryPercentage",telemetry->batteryPercentage);
   delay(PAUSE_BETWEEN_PACKETS);
   return(OK);
}


//----------------------------------------
//--- Sends error messages
//----------------------------------------
int sendErrors(){
  int i;
  
  for(i=0;i<=num_err-1;i++){
      sendPayload("error",error[i]);
      delay(PAUSE_BETWEEN_PACKETS);
   }
   return(OK);
}


//----------------------------------------
//--- Xbee sends payload to defined node
//----------------------------------------
int sendPayload(String desc, String payload){
   char tempChar[80];
   byte tempBytes[81];
   String tempStr = "";
   Tx16Request tx; 

   tempStr = desc + "|" + payload; 
   tempStr.toCharArray(tempChar,tempStr.length());    
   tempStr.getBytes(tempBytes, sizeof(tempBytes));
   tx = Tx16Request(0x1234, tempBytes, tempStr.length());
   xbee.send(tx);  
   if (xbee.readPacket(1000)) {            
       if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
         xbee.getResponse().getTxStatusResponse(txStatus);   
           if (txStatus.getStatus() == SUCCESS) {
              return(PACKET_SENT_OK);
           } 
           else {
              return(PACKET_NOT_SENT);           
           }
        }
   }
   return(PACKET_NOT_SENT);
}


//----------------------------------------
//--- Read Dallas temperature sensor 
//----------------------------------------
float getTemp(DeviceAddress deviceAddress){
  float tempC = 85;
  int i = 0;
  int resuscit = 0;

  do {
      tempC = sensors.getTempC(deviceAddress);
      if ((tempC < -60) || (tempC > 80)){         // If reading failed then
           oneWire.reset();                        // try to resuscitate one-wire bus
           digitalWrite(ONE_WIRE_BUS_PIN, LOW);    
           sensors.begin();
           if (!sensors.getAddress(dallasProbe, 0)){
                addError(ERR_FOUND_DALLAS_TEMP);
           }
           sensors.setResolution(dallasProbe, TEMPERATURE_PRECISION);          
           sensors.requestTemperatures();
           resuscit++;
      }
      i++;
  } while(i < 4 && ((tempC == 85.00) || (tempC == -127.00)));
  if((tempC <= -127) || (tempC >= 85)){
     addError(ERR_READING_DALLAS_TEMP);
  }
  if(resuscit > 0) {
     addError(ERR_RESUSCITATION_ONEWIRE_BUS);
  }
  return (tempC);
}


//----------------------------------------
//--- Add error to error array if already not exists
//----------------------------------------
void addError(String errorType){
  bool errExists = false;
  int i;

  if(num_err < 9) {
     for(i=0;i<=num_err-1;i++){
       if(errorType == error[i]){
         errExists = true;
       }
     }
     if(errExists == false){
       error[num_err] += errorType;
       num_err++;
     }
  }
}


//----------------------------------------
//--- Init array for error reporting
//----------------------------------------
void initErrors(){
  int i;

    for(i=0; i<=9; i++){
       error[i] = String("");
    }
    num_err = 0;
}
