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
//--- Basic structures
//--------------------------------------------

//--- Structure for weight scale sensor data processing
typedef struct  {
    String tempHive;
    String wghtHive;
} SensorRecord;

//--- Structure for internal telemetry processing
typedef struct {
    String batteryVoltage;
    String tempInt;
} TelemetryRecord;

#ifdef BPMMETEO
//--- Structure for meteo sensor data processing
typedef struct  {
    String temp1;
    String temp2;
    String pressure;
    String humidity;
} MeteoRecord;
#endif
