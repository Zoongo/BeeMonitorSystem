//--------------------------------------------
//--- BeeMonitor receiver
<<<<<<< HEAD
//--- Board: Arduino Mega 
//--- Communication: XBee Series1 and Arduino Wifi shield
=======
//--- Board: Arduino Mega
//--- Communication: XBee Series1 + WiFi
>>>>>>> origin/master
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
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <XBee.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"
#include <stdlib.h>

#define PL_RECEIVED 0
#define PL_NOT_RECEIVED -1
#define _OK 0
#define ERROR -1
#define NONAME -99
#define NORMAL 1
#define BIG 2

#define DEBUG 1


char ssid[] = "RXMRESEARCH";        // network SSID (name)
char pass[] = "jmkvktjkm";          // network password
IPAddress server(172,18,0,35);      // Apache server address:
int status = WL_IDLE_STATUS;


WiFiClient client;                   // Initialize the Wifi client library
SoftwareSerial Sserial(11, 10);      // Initializr rx, tx
XBee xbee = XBee();                  // Initialize xBee and important communication objects
Rx16Response rx16 = Rx16Response();
Rx64Response rx64 = Rx64Response();
XBeeAddress64 node1 = XBeeAddress64(0x00000000, 0x00005678);

//--------------------------------------------------------
//--- Setup function
//--------------------------------------------------------
void setup() {
    Serial.begin(38400);
    Sserial.begin(9600);
    xbee.setSerial(Sserial);
#ifdef DEBUG    
    Serial.println("Wifi Initializing...");
#endif    
  // Check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
  // Check for the firmware version
  String fv = WiFi.firmwareVersion();
  if (fv != "1.1.0") {
    Serial.println("Please upgrade the firmware");
  }
  // Attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
#ifdef DEBUG     
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
#endif
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // Wait 10 seconds for connection:
    delay(10000);
  }
  // You're connected now, so print out the status:
#ifdef DEBUG   
  printWifiStatus();
#endif
}


//--------------------------------------------------------
//--- Main loop function
//--------------------------------------------------------
void loop() {
  int rssi;
  float rssi_percent;
  uint8_t id_node;
  char paylstring[99];
  char type[30];
  char value[30];
  char tabname[30];
  char position[30];
  char id_node_char[10];
  int status;
  String str;



    if(receivePacket(paylstring, &rssi, &id_node) == PL_RECEIVED){
#ifdef DEBUG       
        Serial.print("=============>>>>>");
        Serial.print("Packet"); 
        Serial.println("=============<<<<<");
#endif

        giveDataContext(paylstring, type, value);
        status = getTablePosition(type, tabname, position);
        if(status == _OK){
            str=String(id_node);
            str.toCharArray(id_node_char,3);
#ifdef DEBUG 
            Serial.print("RSSI: "); Serial.println(rssi);
            rssi_percent = 100 * rssi/64;           
            Serial.print("RSSI%: "); Serial.println(rssi_percent);
            Serial.print("Calling sendPOST with params: ");
            Serial.print("tabname>");Serial.print(tabname);Serial.print("< "); 
            Serial.print("value>");Serial.print(value);Serial.print("< ");             
            Serial.print("id_node_char>");Serial.print(id_node_char);Serial.print("< ");                
            Serial.print("position>");Serial.print(position);Serial.println("< ");                
#endif 
            sendPOST(tabname, value, id_node_char, position);            
        } else {
#ifdef DEBUG 
            Serial.print("Unidentified payload type >");
            Serial.print(type); 
            Serial.println("<");
#endif     
        }
  }
} 


//--------------------------------------------------------
//--- Send data to application server
//--------------------------------------------------------
int getTablePosition(char *type,  char *tabname, char *position){  
  if (!strcmp(type,"meteoTemp1")){
      strcpy(position,"0");
      strcpy(tabname,"local_meteo_temp1");
      return(_OK);
  } else if (!strcmp(type,"meteoTemp2")){
      strcpy(position,"0");
      strcpy(tabname,"local_meteo_temp2");     
      return(_OK);  
  } else if (!strcmp(type,"pressure")){
      strcpy(position,"0");
      strcpy(tabname,"local_meteo_pressure");       
      return(_OK);
  } else if (!strcmp(type,"humidity")){
      strcpy(position,"0");
      strcpy(tabname,"local_meteo_hum");  
      return(_OK);
  } else if (!strcmp(type,"tempHive")){
      strcpy(position,"1");
      strcpy(tabname,"hive_temp");  
      return(_OK);
  }  else if (!strcmp(type,"wghtHive")){
      strcpy(position,"1");
      strcpy(tabname,"hive_weight");  
      return(_OK);
  } else if (!strcmp(type,"batteryVoltage")){
      strcpy(position,"0");
      strcpy(tabname,"tlm_voltage");  
      return(_OK);
  }
  else if (!strcmp(type,"error")){
      strcpy(position,"");
      strcpy(tabname,"err_log"); 
      return(_OK);
  } else {
    return(ERROR);
  }
}


//--------------------------------------------------------
//--- Sends http post
//--------------------------------------------------------
void sendPOST(char* table, char* value, char* node, char* position)
{
  char inChar;
  String phpData;

#ifdef DEBUG 
  Serial.println("...Connecting to web server");
#endif
  phpData = "table="+(String)table+"&"+"node="+(String)node+"&value="+(String)value+"&position="+(String)position;
#ifdef DEBUG 
  Serial.print("phpData:"); Serial.println(phpData);
#endif
  if(!client.connected()){ 
    client.connect(server, 8888);
  }
#ifdef DEBUG 
    Serial.println("connecting...");
#endif
    client.println("POST /insert_mysql.php HTTP/1.1");
    client.println("Host: 172.18.0.35:8888");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded;");
    client.print("Content-Length: ");
    client.println(phpData.length());
    client.println();
    client.println(phpData);   
    while(client.connected()){
      while(client.available()){
        inChar = client.read();
        Serial.write(inChar);
      }
    }
    client.stop();
    Serial.println();
    return;
}


//--------------------------------------------------------
//--- Receives packet and transform it to paylstring
//--------------------------------------------------------
int receivePacket(char *paylstring, int *rssi, uint8_t *id_node)
{
  int i;
  int retval;
  uint8_t incomingAddress;
  
  retval = PL_NOT_RECEIVED;
  for (i = 0; i < 100; i++) {
    paylstring[i] = '\0';
  }
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
        if (xbee.getResponse().getApiId() == RX_16_RESPONSE || 
            xbee.getResponse().getApiId() == RX_64_RESPONSE) {
           if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
                xbee.getResponse().getRx16Response(rx16);
                for (i = 0; i < rx16.getDataLength(); i++) {
                  paylstring[i] = char(rx16.getData(i));
                }
                *rssi =  rx16.getRssi();
                *id_node = rx16.getRemoteAddress16();
                Tx64Request tx = Tx64Request(node1, rx16.getData(), sizeof(rx16.getData()));
                xbee.send(tx);
                retval = PL_RECEIVED;               
           }
        } else {
                xbee.getResponse().getRx64Response(rx64);
                for (i = 0; i < rx64.getDataLength(); i++) {
                  paylstring[i] = char(rx64.getData(i));
                }
                *rssi =  rx64.getRssi();
                *id_node = rx64.getRemoteAddress64();
                retval = PL_RECEIVED;  
      }  
  } 
#ifdef DEBUG   
  if (retval == PL_RECEIVED) {
      Serial.print("Paylstring>>>"); 
      Serial.print(paylstring); 
      Serial.println("<<<"); 
  }
#endif
  if (paylstring[0] == '\0'){
    retval = PL_NOT_RECEIVED;
  }
  return (retval);
}


//--------------------------------------------------------
//--- Split received payload to type and value
//--------------------------------------------------------
 void giveDataContext(char *payldata, char *type, char *value) {
  int i = 0;
  int j = 0;
#ifdef DEBUG
   Serial.println("Pred giveDataContext");
#endif 
  strcpy(type,"");
  strcpy(value,"");
  while (payldata[i] != '|') {
    type[i] = payldata[i];
    i++;
  }
  type[i]='\0';
  i++;
  while (payldata[i] != '\0') {
    value[j] = payldata[i];
    i++;
    j++;
  }
  value[j]='\0';
#ifdef DEBUG
   Serial.println("Za giveDataContext");
#endif 
}


//--------------------------------------------------------
// Prints wifi connection status
//--------------------------------------------------------
#ifdef DEBUG 
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
#endif



