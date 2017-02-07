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
//--- Error, warning messages and status parameters
//--------------------------------------------

//--- Error messagess
#define ERR_READING_DALLAS_TEMP       "1000"
#define ERR_READING_ONBOARD_TEMP      "1010"
#define ERR_RESUSCITATION_ONEWIRE_BUS "1015"
#define ERR_OVERHEATING_ONBOARD_TEMP  "1020"
#define ERR_LOW_VOLTAGE               "1030"
#define ERR_HUMIDITY_SENSOR           "1040"
#define ERR_ATMPRESSURE_SENSOR        "1050"
#define ERR_TEMP_CORR_INT             "1060"
#define ERR_TEMP_CONST                "1070"

//--- Warning messages
#define WARN_LOW_VOLTAGE              "3050"

//--- Status definition
#define OK                              0
#define PACKET_NOT_SENT                 -1
#define PACKET_SENT_OK                  0
#define RECORD_SENT_OK                  -99
#define PAYLOAD_TYPE_NOT_EXISTS        -2
