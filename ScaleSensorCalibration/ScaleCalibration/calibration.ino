//--------------------------------------------
//--- BeeMonitor transmitter
//--- Board: SODAQ Autonomo
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
#include <Wire.h>
#include <Q2HX711.h>
#include <RunningMedian.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//--- HX711 pins definition
#define HX_DATA_PIN A5        
#define HX_CLOCK_PIN A4   
//--- OneWire pin definition
#define ONE_WIRE_BUS_PIN A12               

#define  TEMPERATURE_PRECISION 11                       // Dallas temperature sensor precision
Q2HX711 scale(HX_DATA_PIN, HX_CLOCK_PIN);               // HX711 - scale sensor
OneWire oneWire(ONE_WIRE_BUS_PIN);                      // One wire for Dallas sensors
DallasTemperature sensors(&oneWire);                    // Dallas sensor object
DeviceAddress dallasProbe;                              // Dallas sensor address
   
long weightSensorValue;
const int waitTimeLoadSamples = 1;                      // Take 1 time wait sample
int weightSamplesNumber = 10;                           // Take at least 10 readings for adjusting
RunningMedian weightSamples = RunningMedian(weightSamplesNumber);


void setup() {
  pinMode(BEE_VCC, OUTPUT);
  groveOn();
}

void loop() {
  startDallas();
  weightSamples.clear();
  do {
    for (int i = 0; i < waitTimeLoadSamples; i++) {
      delay(1000);
    }
    weightSensorValue = scale.read();
    SerialUSB.print("Raw sensor reading: ");
    SerialUSB.print(weightSensorValue);
    weightSamples.add(weightSensorValue);
    SerialUSB.print("    Temperature: ");
    sensors.requestTemperatures();
    SerialUSB.println(getTemp(dallasProbe), DEC);
    delay(1000);
  } while (weightSamples.getCount() < weightSamples.getSize());
  SerialUSB.print("Average: "); SerialUSB.print(weightSamples.getAverage());
  SerialUSB.print(" Median: "); SerialUSB.println(weightSamples.getMedian());
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
//--- Read Dallas temperature sensor 
//----------------------------------------
float getTemp(DeviceAddress deviceAddress){
  float tempC;
  
      tempC = sensors.getTempC(deviceAddress);
  return (tempC);
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
         SerialUSB.println("Unknown address of Dallas sensor!");;
  } else {
      SerialUSB.print("Dallas senzor HEX address:");
      printAddress(dallasProbe);
      SerialUSB.println("");

  }
  sensors.setResolution(dallasProbe, TEMPERATURE_PRECISION); 
  return;
}

//----------------------------------------
//--- Print a Dallas device address
//----------------------------------------
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) SerialUSB.print("0");
    SerialUSB.print(deviceAddress[i], HEX);
  }
}
