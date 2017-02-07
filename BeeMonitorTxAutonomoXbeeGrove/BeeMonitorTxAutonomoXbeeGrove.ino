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
//--- Presumed hardware components:
//--- 1. Sodaq Autonomo
//--- 2. Sodaq Autonomo Grove shield
//--- 3. HX711 for the load scale sensor instrumentation
//--- 4. Dallas temperature sensors
//--- 5. XBee Series1 or GPRSBee Sim900
//--- 6. LiPo 
//--- 7. Solar panel 5V, power depends on you bat capacity and communication period
//--------------------------------------------
//--- Compiler parameters
//--------------------------------------------
//#define DEBUG  //--- Uncomment in the case of debug via serial monitor
//#define XBEE  //--- Uncomment in the case of XBee Series1 communication
#define GPRSBEE  //--- Uncomment in the case of GPRSBee Sim900 communication


#include <Wire.h>
#include <Q2HX711.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Printers.h>
#include <RTCZero.h>
#ifdef XBEE
   #include <XBee.h>
#endif
#ifdef GPRSBEE
   #include <GPRSbee.h>
#endif 
#include <RunningMedian.h>  // http://playground.arduino.cc/Main/RunningMedian

#include "BeeMessages.h"
#include "BeeStructures.h"
#include "BeeParameters.h"


String error[MAX_ERRORS];                                    // Array of strings for remote error reporting
int num_err = 0;                                             // Init number of errs

//--- Varibles for the RTC setup
uint8_t seconds;
uint8_t minutes;
uint8_t hours ;

//--- Payload variables
TelemetryRecord telemetry;
SensorRecord sensor;


#ifdef XBEE
   XBee xbee = XBee();                                     // XBee Series 1 
   TxStatusResponse txStatus = TxStatusResponse();         // Transmittion response
   Rx64Response rx64 = Rx64Response();                     // Receiving response 64bit packet
   Rx16Response rx16 = Rx16Response();                     // Receiving response 16bit packet
#endif
   RTCZero rtc;                                            // Real-time clock
   OneWire oneWire(ONE_WIRE_BUS_PIN);                      // One wire for Dallas sensors
   DallasTemperature sensors(&oneWire);                    // Dallas sensor object
   DeviceAddress dallasProbe;                              // Dallas sensor address
   Q2HX711 scale(HX_CLOCK_PIN, HX_DATA_PIN);               // HX711 - scale sensor 
   RunningMedian weightSamples = RunningMedian(WEIGHT_SAMPLES_NUMBER);  // create RunningMedian object
  
void setup()
{
  SerialUSB.begin(9600);                                 // Start USB print output
  Serial1.begin(9600);                                   // Start serial dedicated for XBee or GPRSBee
  rtc.begin(24);                                         // Start RTC
  resetTimer(HOUR_TX, MINUTE_TX, SECOND_TX);             // Tx timer reset
  Wire.begin();
  pinMode(LED_PIN, OUTPUT);                              // Set LED pin as output
  pinMode(BEEDTR, OUTPUT);                               // Set XBee turnon pin as output
  pinMode(BEE_VCC, OUTPUT);                              // Set XBee Vcc pin as output                                 
  groveOn();
  startDallas();                                         // Start Dallas temperature sensor
#ifdef XBEE  
  xBeeOn();                                              // Switch xBee ON
  xbee.setSerial(Serial1);                               // Serial1 assigned for XBEE
#endif
#ifdef GPRSBEE
  digitalWrite(BEE_VCC, HIGH);
  while ((!SerialUSB) && (millis()<10000));
  gprsbee.initAutonomoSIM800(Serial1, BEE_VCC, BEEDTR, BEECTS);
#ifdef DEBUG
  gprsbee.setDiag(SerialUSB);
#endif
  gprsbee.on();
#endif

  initErrors();                                          // Init of Errors array
  setSleepMode();                                        // Set of sleep mode
  delay(5000);
}

void loop()
{ 
    if(readBeeSensors(&sensor) == OK){                    // Reads sensor data
       sendBeeSensors(&sensor);                           // Sends sensor data via XBee
       //delay(500); 
     }
     if(readTelemetry(&telemetry) == OK){                 // Reads board telemetry
        sendTelemetry(&telemetry);                        // Sends telemetry data via XBee
        //delay(500); 
     }
     sendErrors();                                        // Send errors
#ifdef XBEE
     xBeeOff();
#endif
#ifdef GPRSBEE
     gprsBeeOff();
#endif
     groveOff();
#ifdef DEBUG
     debug("Pred sleepem");                               // System goes sleep
#endif
     systemSleep();      
#ifdef DEBUG                               
     debug("Za sleepem");
     blinkAwake();                                        // Awake indication
#endif
#ifdef XBEE
     xBeeOn();                                            // Switch xBee ON
#endif
#ifdef GPRSBEE
     gprsBeeOn();
#endif
     groveOn();                                           // Switch Grove ON
     startDallas();
     //delay(500);
}


//----------------------------------------
//--- Reads scale sensor and return weight in grams
//----------------------------------------
double readWeightScale(double temperature, int calcMethod, int statMethod) {
  int i;
  long weightSensorValue;
  float weightKg;
  float correction;
  long loadCellZeroOffset;
  long loadCellKgDivider;
  double finalWeightKg;

  loadCellZeroOffset = LOAD_CELL_ZERO_OFFSET;
  loadCellKgDivider = LOAD_CELL_KG_DIVIDER; 
  weightSamples.clear();
  do { // read x times weight and take median

    for (int i=0; i<WEIGHT_TIME_BETWEEN_MEASUREMENTS; i++) {
      delay(1000);
    }
    weightSensorValue = scale.read();
    if(calcMethod == KG_DIVIDER) {
       weightKg = ((float)weightSensorValue - (float)loadCellZeroOffset) / (float)loadCellKgDivider;
    } else {
       weightKg = WGHT_E_A * pow(weightSensorValue,3) + WGHT_E_B * pow(weightSensorValue,2) + WGHT_E_C * weightSensorValue + WGHT_E_D; // Raw weight calculation based on calibration parameters
       correction = COMP_TEMP_PAR * (temperature - COMP_TEMP);                                                                         // Temperature compensation
       weightKg = weightKg - correction;    
    }
    if(weightKg < 0){
       weightKg = 0;
    }
    weightSamples.add(weightKg);
  } while (weightSamples.getCount() < weightSamples.getSize());
  if(statMethod == MEDIAN){
    finalWeightKg = weightSamples.getMedian();
  } else {
    finalWeightKg = weightSamples.getAverage();   
  }
   return(finalWeightKg);
}


//----------------------------------------
//--- Sends error messages
//----------------------------------------
int sendErrors(){
  int i;
  
  for(i=0;i<=num_err-1;i++){
#ifdef XBEE
      sendPayloadXbee("error",error[i]);
      delay(PAUSE_BETWEEN_PACKETS);
#endif
#ifdef GPRSBEE
   sendPayloadGPRSbee("error",error[i]);
#endif
   }
   return(OK);
}

#ifdef XBEE
//----------------------------------------
//--- Switch xBee ON
//----------------------------------------
void xBeeOn (){
  digitalWrite(BEE_VCC, HIGH);                               // Turn on BEE_VCC
  digitalWrite(BEEDTR, LOW);                                 // Turn on the XBee
}


//----------------------------------------
//--- Switch xBee OFF
//----------------------------------------
void xBeeOff (){
  digitalWrite(BEE_VCC, LOW);                               // Turn off BEE_VCC
  digitalWrite(BEEDTR, HIGH);                               // Turn off the XBee
}


//----------------------------------------
//--- Xbee sends payload to defined node
//----------------------------------------
int sendPayloadXbee(String desc, String payload){
   char tempChar[80];
   byte tempBytes[81];
   String tempStr = "";
   Tx16Request tx; 
   
   tempStr = desc + "|" + payload; 
   tempStr.toCharArray(tempChar,tempStr.length());    
   tempStr.getBytes(tempBytes, sizeof(tempBytes));
   tx = Tx16Request(GATEWAY, tempBytes, tempStr.length());
   xbee.send(tx);  
#ifdef DEBUG
   debug("Payload:"); 
   debug(tempStr);
#endif
   if (xbee.readPacket(5000)) {    
       if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {   
         xbee.getResponse().getTxStatusResponse(txStatus);   
           if (txStatus.getStatus() == SUCCESS) {
#ifdef DEBUG       
              debug("PACKET_SENT_OK");  
#endif 
              return(PACKET_SENT_OK);
           } 
           else {
#ifdef DEBUG
              debug("PACKET_NOT_SENT");  
#endif
              return(PACKET_NOT_SENT);           
           }
        }
   }
#ifdef DEBUG
   debug("PACKET_NOT_SENT");  
#endif
   return(PACKET_NOT_SENT);
}
#endif

//----------------------------------------
//--- Reads battery voltage
//----------------------------------------
float getVoltage(){
  uint16_t batteryVoltage = analogRead(BAT_VOLT);
  return (ADC_AREF / 1.023) * (BATVOLT_R1 + BATVOLT_R2) / BATVOLT_R2 * batteryVoltage;
}


//----------------------------------------
//--- Reads internal telemetry, saves values into TelemetryRecord
//----------------------------------------
int readTelemetry(TelemetryRecord *telemetry) {
    String internalTemperature;
    float batteryVoltage;

  telemetry->batteryVoltage = "";
  telemetry->tempInt = "";
  batteryVoltage = getVoltage();
  if(batteryVoltage < 3.7) {
    addError(ERR_LOW_VOLTAGE);
  }
  telemetry->batteryVoltage += batteryVoltage;
  return(OK);
}

//----------------------------------------
//--- Sends internal telemetry information
//----------------------------------------
int sendTelemetry(TelemetryRecord *telemetry){
  

#ifdef XBEE
   sendPayloadXbee("batteryVoltage",telemetry->batteryVoltage);
   delay(PAUSE_BETWEEN_PACKETS);
#endif
#ifdef GPRSBEE
   sendPayloadGPRSbee("batteryVoltage",telemetry->batteryVoltage);
#endif
   return(OK);
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


//----------------------------------------
//--- Read Dallas temperature sensor 
//----------------------------------------
float getTemp(DeviceAddress deviceAddress){
  float tempC;
  
      tempC = sensors.getTempC(deviceAddress);
  return (tempC);
}


//----------------------------------------
//--- Reads bee sensors, save values into SensorRecord structure
//----------------------------------------
int readBeeSensors(SensorRecord *sensor) {
   double probeTemperature;
   double correctionTemperature;
                    
   sensor->wghtHive = "";                                                    
   sensor->tempHive = "";  
   sensors.requestTemperatures();
   probeTemperature = getTemp(dallasProbe);                                
   sensor->tempHive += probeTemperature;           
   if( (probeTemperature < -70) || (probeTemperature > 90) ){
      correctionTemperature = 15.0;                 
    } else {
    correctionTemperature = probeTemperature;
   }
   sensor->wghtHive += readWeightScale(correctionTemperature, KG_DIVIDER, MEDIAN);              
   return(OK);
}


//----------------------------------------
//--- Sends sensor data
//----------------------------------------
int sendBeeSensors(SensorRecord *sensor){
#ifdef XBEE
   sendPayloadXbee("tempHive",sensor->tempHive);
   delay(PAUSE_BETWEEN_PACKETS);
   sendPayloadXbee("wghtHive",sensor->wghtHive);
   delay(PAUSE_BETWEEN_PACKETS);
#endif
#ifdef GPRSBEE
   sendPayloadGPRSbee("tempHive",sensor->tempHive);
   sendPayloadGPRSbee("wghtHive",sensor->wghtHive);
#endif
   return(OK);
}


//----------------------------------------
//--- Start Dallas temperature sensor
//----------------------------------------
void startDallas(){

 oneWire.reset();                        
 digitalWrite(ONE_WIRE_BUS_PIN, LOW);   
 delay(200);
 digitalWrite(ONE_WIRE_BUS_PIN, HIGH);            
 sensors.begin();
  if (!sensors.getAddress(dallasProbe, 0)){
         addError(ERR_READING_DALLAS_TEMP);
  }
  sensors.setResolution(dallasProbe, TEMPERATURE_PRECISION); 
  return;
}


//----------------------------------------
//--- Reset timer
//----------------------------------------
void resetTimer(uint8_t hours, uint8_t minutes, uint8_t seconds){
    rtc.disableAlarm();
    rtc.detachInterrupt();
    rtc.setHours(0);
    rtc.setMinutes(0);
    rtc.setSeconds(0);
    rtc.setAlarmSeconds(seconds);
    rtc.setAlarmMinutes(minutes);
    rtc.setAlarmHours(hours);
    rtc.enableAlarm(RTCZero::MATCH_HHMMSS);
    rtc.attachInterrupt(RTC_ISR);
}


//----------------------------------------
//--- Set the sleepmode
//----------------------------------------
void setSleepMode(){
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
}


//----------------------------------------
//--- System sleep
//----------------------------------------
void systemSleep()
{
  if (USB->DEVICE.FSMSTATUS.bit.FSMSTATE == USB_FSMSTATUS_FSMSTATE_SUSPEND_Val) {
    USBDevice.detach();
    __WFI();
    USBDevice.attach();
    USB->DEVICE.CTRLB.bit.UPRSM = 0x01u;
    //while (USB->DEVICE.CTRLB.bit.UPRSM);
  }
}


//----------------------------------------
//--- RTC interrupt handler
//----------------------------------------
void RTC_ISR()
{
#ifdef DEBUG
  printTimeStamp("RTC_ISR alarm");
#endif
  resetTimer(HOUR_TX, MINUTE_TX, SECOND_TX);
}


//----------------------------------------
//--- Set switchable positions of Grove shield ON
//----------------------------------------
void groveOn() {
  pinMode(VCC_SW, OUTPUT);                               
  digitalWrite(VCC_SW, HIGH);
  return;
}


//----------------------------------------
//--- Set switchable positions of Grove shield OFF
//----------------------------------------
void groveOff() {
  pinMode(VCC_SW, OUTPUT);                               
  digitalWrite(VCC_SW, LOW);
  return;
}

#ifdef GPRSBEE
//----------------------------------------
//--- Set GPRSBee ON
//----------------------------------------
void gprsBeeOn(){
    digitalWrite(BEE_VCC, HIGH);
    gprsbee.on();
    return;
}


//----------------------------------------
//--- Set GPRSBee OFF
//----------------------------------------
void gprsBeeOff(){
      digitalWrite(BEE_VCC, LOW);
      gprsbee.off();
    return;
}


   //--------------------------------------------------------
   //--- Sends http GET
   //--------------------------------------------------------
   int sendGET(char* table, char* value, char* node, char* position)
   {
     boolean retval;
     char buffer[LENGTHHTTP];
     char phpData[LENGTHHTTP];

     memset(buffer,0,sizeof(buffer));
     memset(phpData,0,sizeof(phpData));
     snprintf(phpData,sizeof(phpData),URL_TEMPLATE,GET_URL, node, value, position, table);
#ifdef DEBUG  
     SerialUSB.print("phpData >");   SerialUSB.print(phpData);   SerialUSB.println("<");
#endif
     retval = gprsbee.doHTTPGET(APN,phpData, buffer, sizeof(buffer));
#ifdef DEBUG  
     SerialUSB.print("GET result: >"); SerialUSB.print(buffer); SerialUSB.println("<");
#endif
     if(!strcmp(buffer,"OK")){
        return(PACKET_SENT_OK);
     } else {
        return(PACKET_NOT_SENT);
     }
   }

//--------------------------------------------------------
//--- Send payload via GPRSBee
//--------------------------------------------------------
int sendPayloadGPRSbee(String desc, String payload){
   char table[LENGTHSTRING];
   char value[LENGTHSTRING];
   char type[LENGTHSTRING];
   char pos[LENGTHSTRING];

   desc.toCharArray(type, LENGTHSTRING + 1);
   payload.toCharArray(value, LENGTHSTRING + 1);
   getTablePosition(type, table, pos);
#ifdef DEBUG
   debug("desc:");
   debug(desc);
   debug("payload:");
   debug(payload);
   debug("type:");
   debug(type);
   debug("value:");
   debug(value);
   debug("table:");
   debug(table);
#endif
   sendGET(table, value, NODE_ID, pos);
   return(OK);
}


//--------------------------------------------------------
//--- Gives table names and positions
//--------------------------------------------------------
int getTablePosition(char *type,  char *tabname, char *position){  
  if (!strcmp(type,"meteoTemp1")){
      strcpy(position,"0");
      strcpy(tabname,"local_meteo_temp1");
      return(OK);
  } else if (!strcmp(type,"meteoTemp2")){
      strcpy(position,"0");
      strcpy(tabname,"local_meteo_temp2");     
      return(OK);  
  } else if (!strcmp(type,"pressure")){
      strcpy(position,"0");
      strcpy(tabname,"local_meteo_pressure");       
      return(OK);
  } else if (!strcmp(type,"humidity")){
      strcpy(position,"0");
      strcpy(tabname,"local_meteo_hum");  
      return(OK);
  } else if (!strcmp(type,"tempHive")){
      strcpy(position,"1");
      strcpy(tabname,"hive_temp");  
      return(OK);
  }  else if (!strcmp(type,"wghtHive")){
      strcpy(position,"1");
      strcpy(tabname,"hive_weight");  
      return(OK);
  } else if (!strcmp(type,"batteryVoltage")){
      strcpy(position,"0");
      strcpy(tabname,"tlm_voltage");  
      return(OK);
  }
  else if (!strcmp(type,"error")){
      strcpy(position,"");
      strcpy(tabname,"err_log"); 
      return(OK);
  } else {
     return(PAYLOAD_TYPE_NOT_EXISTS);
  }
}
#endif


#ifdef DEBUG
//----------------------------------------
//--- Function to print a device address (on the OneWire bus
//----------------------------------------
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) SerialUSB.print("0");
    SerialUSB.print(deviceAddress[i], HEX);
  }
}


//----------------------------------------
//--- Print time-stamp
//----------------------------------------
void printTimeStamp(char* label){
  SerialUSB.print(label);
  SerialUSB.print(" :");
  SerialUSB.print(rtc.getHours());
  SerialUSB.print(":");
  SerialUSB.print(rtc.getMinutes());
  SerialUSB.print(":");
  SerialUSB.println(rtc.getSeconds());
}


//----------------------------------------
//--- Debug messagess
//----------------------------------------
void debug(char* message) {
    SerialUSB.println(message);
}
void debug(String message) {
    SerialUSB.println(message);
}
void debug(float message) {
    SerialUSB.println(message);
}


//----------------------------------------
//--- Awake blink indication 
//----------------------------------------
void blinkAwake(){
  int i;

  for (i=0;i<=10;i++){
      digitalWrite(13, HIGH);
      delay(200);
      digitalWrite(13, LOW);
      delay(200);
  }
}
#endif
