/****************************************************/
/***  InitialStateAgent                           ***/
/***  Send data from the bee_logger database to   ***/
/***  InitialState site                           ***/
/***                                              ***/
/*** (c) 2016, Zoongo Ltd., Tomas Ivansky         ***/
/****************************************************/

#include <stdio.h>
#include <mysql.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <time.h>
#include "../../Includes/databaseCredentials.h"
#include "../../Includes/emojiDefinition.h"
#include "../../Includes/sqlStatements.h"
#include "../../Includes/InitialStateAgent.h"

// Global variables
BeeSensors beeSensors;
MyMeteoSensors myMeteo;
MeteoProvider providerMeteo;
char* moonPhase;
MYSQL* Connection;
MYSQL Mysql;
weightPoints weightPoint[MAX_WEIGHT_POINTS];
int numWeightPoints;


int main() {
    int state;
   
    printf("/*********************************/ \n");
    printf("/*** BeeMonitor - InitialState agent \n");
    printf("/*** (c) 2016 Zongo Ltd., Tomas Ivansky \n");
    printf("/*** \n");
    printf("/*** Communication with InitialState.com is based on David G.Simmons Initial State C-Library\n");
    printf("/*** https://github.com/davidgs/intiial-state-C \n");
    printf("/*********************************/ \n\n");
    state = readPositionsAndNodes();
   if(state != OK) {
        printf("readpositionAndNodes error\n");
        return(TROUBLE);
   }
    
  // Send data for Productivity bucket
  state = productivityPayload();
  if(state != OK) {
        printf("productivityPayload error\n");
        return(TROUBLE);
  }
  
  // Send data for detailed node info buckets
  state = nodesDetailedInfoPayload();
  if(state != OK) {
        printf("nodesDetailedInfoPayload error\n");
        return(TROUBLE);
  }
    
  // Send data for node telemetry info
  state = telemetryPayload();
  if(state != OK) {
        printf("telemetryPayload error\n");
        return(TROUBLE);
  }
    
  printf("Done.\n");
  return(OK);
}


/*************************************/
/*** Read and send data for        ***/
/*** telemetry buckets             ***/
/*************************************/
int telemetryPayload() {
    int i, retval;
    char signalname[200];
    
    
    for(i=0;i<=numWeightPoints-1;i++){
        sprintf(signalname,"%s %s-%s (node nr. %u) [V]", EMOJI_BATTERY, "Battery voltage", weightPoint[i].name_location, weightPoint[i].node);
        retval = sensorDetailPayload(weightPoint[i].node, -99, TELEMETRY_NAME, TELEMETRY_BUCKET_KEY, TELEMETRY_ACCESS_KEY, signalname, SQL_TLM_VOLTAGE_FRST, SQL_TLM_VOLTAGE_NXT, "battery_voltage", "value");
        if(retval == OK){
            printf(SIGNAL_OK, signalname);
        } else {
            printf(SIGNAL_ERR, signalname);
            
        }
    }
    return retval;
}


/*************************************/
/*** Read and send data for        ***/
/*** Productivity bucket           ***/
/*************************************/
int productivityPayload(){
    int i, retval;
    char signalname[200];
    
    for(i=0;i<=numWeightPoints-1;i++){
        sprintf(signalname,"%s %s-%u-%u", EMOJI_SCALE, weightPoint[i].name_location, weightPoint[i].node, weightPoint[i].position);
        retval = sensorDetailPayload(weightPoint[i].node, weightPoint[i].position, PRODUCTION_NAME, PRODUCTION_BUCKET_KEY, PRODUCTION_ACCESS_KEY, signalname, SQL_WGHT_DET_FRST, SQL_WGHT_DET_NXT, "hive_weight", "value");
        if(retval == OK){
            printf(SIGNAL_OK, signalname);
        } else {
            printf(SIGNAL_ERR, signalname);

        }
    }
    return retval;
}


/*************************************/
/*** Read and send data for        ***/
/*** node detailed info            ***/
/*************************************/
int nodesDetailedInfoPayload(){
    int i, retval, lastnode;
    char signalname[200];
    
    for(i=0;i<=numWeightPoints-1;i++){
        //Weight ----------------------
        sprintf(signalname,"%s %s %u-%u", EMOJI_SCALE, "Weight [kg]", weightPoint[i].node, weightPoint[i].position);
        retval = sensorDetailPayload(weightPoint[i].node, weightPoint[i].position, weightPoint[i].bucketname, weightPoint[i].bucketkey, weightPoint[i].accesskey, signalname, SQL_WGHT_DET_FRST, SQL_WGHT_DET_NXT, "hive_weight","value");
        if(retval == OK){
            printf(SIGNAL_OK, signalname);
        } else {
            printf(SIGNAL_ERR, signalname);
            
        }
        //Send sensor temeprature ----------------------
        if (weightPoint[i].node != 12){ //Due to problem of node 12 with external temp sensor
           sprintf(signalname,"%s %s %u-%u", EMOJI_TEMPERATURE, "Sensor AVERAGE DAILY temp [Celsius]", weightPoint[i].node, weightPoint[i].position);
            retval = sensorDetailPayload(weightPoint[i].node, weightPoint[i].position, weightPoint[i].bucketname, weightPoint[i].bucketkey, weightPoint[i].accesskey, signalname, SQL_TEMPSENS_DET_FRST, SQL_TEMPSENS_DET_NXT, "whtr_temp","avg_temp");
            if(retval == OK){
                printf(SIGNAL_OK, signalname);
            } else {
                printf(SIGNAL_ERR, signalname);
                
            }
        } else {
            //--- Average temperature
            sprintf(signalname,"%s %s %u", EMOJI_TEMPERATURE, "Sensor AVERAGE DAILY temp [Celsius]", weightPoint[i].position);
            retval = sensorDetailPayload(weightPoint[i].node, -99, weightPoint[i].bucketname, weightPoint[i].bucketkey, weightPoint[i].accesskey, signalname, SQL_TEMPTLM_DET_AVG_FRST, SQL_TEMPTLM_DET_AVG_NXT, "tlm_temp","avg_temp");
            if(retval == OK){
                printf(SIGNAL_OK, signalname);
            } else {
                printf(SIGNAL_ERR, signalname);
                
            }
            
            //--- Minimum temperature
            sprintf(signalname,"%s %s %u", EMOJI_TEMPERATURE, "Sensor MINIMUM DAILY temp [Celsius]", weightPoint[i].position);
            retval = sensorDetailPayload(weightPoint[i].node, -99, weightPoint[i].bucketname, weightPoint[i].bucketkey, weightPoint[i].accesskey, signalname, SQL_TEMPTLM_DET_MIN_FRST, SQL_TEMPTLM_DET_MIN_NXT, "tlm_temp","min_temp");
            if(retval == OK){
                printf(SIGNAL_OK, signalname);
            } else {
                printf(SIGNAL_ERR, signalname);
                
            }
            
            //--- Maximum temperature
            sprintf(signalname,"%s %s %u", EMOJI_TEMPERATURE, "Sensor MAXIMUM DAILY temp [Celsius]", weightPoint[i].position);
            retval = sensorDetailPayload(weightPoint[i].node, -99, weightPoint[i].bucketname, weightPoint[i].bucketkey, weightPoint[i].accesskey, signalname, SQL_TEMPTLM_DET_MAX_FRST, SQL_TEMPTLM_DET_MAX_NXT, "tlm_temp","max_temp");
            if(retval == OK){
                printf(SIGNAL_OK, signalname);
            } else {
                printf(SIGNAL_ERR, signalname);
                
            }
        }
        if(retval == OK){
            printf(SIGNAL_OK, signalname);
        } else {
            printf(SIGNAL_ERR, signalname);
            
        }
        if(lastnode != weightPoint[i].node) {
          //External meteo cloudiness
          sprintf(signalname,"%s", "Cloudiness [%]");
          retval = sensorDetailPayload(weightPoint[i].node, -99, weightPoint[i].bucketname, weightPoint[i].bucketkey, weightPoint[i].accesskey, signalname, SQL_METEO_CLOUDINESS_FRST, SQL_METEO_CLOUDINESS_NXT, "external_meteo","cloudiness");
          if(retval == OK){
              printf(SIGNAL_OK, signalname);
          } else {
              printf(SIGNAL_ERR, signalname);
            
          }
        
          //External meteo pressure
          sprintf(signalname,"%s", "Pressure [HPa]");
          retval = sensorDetailPayload(weightPoint[i].node, -99, weightPoint[i].bucketname, weightPoint[i].bucketkey, weightPoint[i].accesskey, signalname, SQL_METEO_PRESSURE_FRST, SQL_METEO_PRESSURE_NXT, "external_meteo","pressure");
          if(retval == OK){
              printf(SIGNAL_OK, signalname);
          } else {
              printf(SIGNAL_ERR, signalname);
            
          }
        
          //External meteo wind-speed
          sprintf(signalname,"%s", "Wind speed [m/s]");
          retval = sensorDetailPayload(weightPoint[i].node, -99, weightPoint[i].bucketname, weightPoint[i].bucketkey, weightPoint[i].accesskey, signalname, SQL_METEO_WINDSPEED_FRST, SQL_METEO_WINDSPEED_NXT, "external_meteo","wind_speed");
          if(retval == OK){
              printf(SIGNAL_OK, signalname);
          } else {
              printf(SIGNAL_ERR, signalname);
            
          }
        
          //External meteo wind-direction
          sprintf(signalname,"%s", "Wind direction [grad]");
          retval = sensorDetailPayload(weightPoint[i].node, -99, weightPoint[i].bucketname, weightPoint[i].bucketkey, weightPoint[i].accesskey, signalname, SQL_METEO_WINDDIRECTION_FRST, SQL_METEO_WINDDIRECTION_NXT, "external_meteo","wind_direction");
          if(retval == OK){
              printf(SIGNAL_OK, signalname);
          } else {
              printf(SIGNAL_ERR, signalname);
            
          }
            
            //External meteo wind-direction with emojis
            sprintf(signalname,"%s", "Wind direction emoji");
            retval = sensorDetailPayload(weightPoint[i].node, -99, weightPoint[i].bucketname, weightPoint[i].bucketkey, weightPoint[i].accesskey, signalname, SQL_METEO_WINDDIRECTION_FRST, SQL_METEO_WINDDIRECTION_NXT, "external_meteo","wind_direction");
            if(retval == OK){
                printf(SIGNAL_OK, signalname);
            } else {
                printf(SIGNAL_ERR, signalname);
                
            }
            
          //External meteo weather-type as text
          sprintf(signalname,"%s", "Weather");
          retval = sensorDetailPayload(weightPoint[i].node, -99, weightPoint[i].bucketname, weightPoint[i].bucketkey, weightPoint[i].accesskey, signalname, SQL_METEO_WEATHER_FRST, SQL_METEO_WEATHER_NXT, "external_meteo","weather");
          if(retval == OK){
              printf(SIGNAL_OK, signalname);
          } else {
              printf(SIGNAL_ERR, signalname);
            
          }
 
          //External meteo weather-type as emoji
          sprintf(signalname,"%s", "Weather emoji");
          retval = sensorDetailPayload(weightPoint[i].node, -99, weightPoint[i].bucketname, weightPoint[i].bucketkey, weightPoint[i].accesskey, signalname, SQL_METEO_WEATHER_FRST, SQL_METEO_WEATHER_NXT, "external_meteo","weather");
            
          if(retval == OK){
              printf(SIGNAL_OK, signalname);
          } else {
              printf(SIGNAL_ERR, signalname);
          }
            
          //Moon age as emoji
          sprintf(signalname,"%s", "Moon age emoji");
          retval = sensorDetailPayload(-99, -99, weightPoint[i].bucketname, weightPoint[i].bucketkey, weightPoint[i].accesskey, signalname, SQL_MOONAGE_FRST, SQL_MOONAGE_NXT, "moon_age","value");
          if(retval == OK){
              printf(SIGNAL_OK, signalname);
          } else {
            printf(SIGNAL_ERR, signalname);
          }
        }
        lastnode = weightPoint[i].node;
    }
    return retval;
}


/*************************************/
/*** Prepare payload and send to   ***/
/*** Initialstate detailed sensor  ***/
/*** info                          ***/
/*************************************/
int sensorDetailPayload(int node, int position, char *bucketname, char *bucketkey, char *accesskey, char *signalname, char *initquery, char *normalquery, char *tabname, char *attrname){
    int status;
    int numProcessed;
    int lastId;
    int readedLastId;
    int retval;
    int numRecord = 0;
    int queryState;
    char queryString[255];
    char value[100];
    char *buffer, buffer1[100];
    char *time_stamp;
    char emoji[50];
    MYSQL_RES *result;
    MYSQL_ROW row;
    Payload payload;
    
    strcpy(value,"");
    strcpy(buffer1,"");
    readedLastId = readLastID(bucketname, tabname, node, position, attrname, signalname);
    printf("-> Detailed sensor information: %s, bucket: %s signal: %s\n", tabname, bucketname, signalname);
    status = databaseConnect();
    if(status != OK){
        printf("-->sensorDetailPayload: Database connect error!\n");
        return TROUBLE;
    }
    if (readedLastId > 0) {
        if(node >= 0){
           if(position >= 0){
              sprintf(queryString, normalquery, INITIALSTATE_TIME_FORMAT, node, position, readedLastId);
           } else {
               sprintf(queryString, normalquery, INITIALSTATE_TIME_FORMAT, node, readedLastId);
           }
        } else {
            sprintf(queryString, normalquery, INITIALSTATE_TIME_FORMAT, readedLastId);
        }
    } else {
        if (node >= 0){
           if(position >= 0){
               sprintf(queryString, initquery, INITIALSTATE_TIME_FORMAT, node, position);
           } else {
               sprintf(queryString, initquery, INITIALSTATE_TIME_FORMAT, node);
           }
        } else {
           sprintf(queryString, initquery, INITIALSTATE_TIME_FORMAT);
        }
    }
    queryState = mysql_query(Connection, queryString);
    if (queryState != 0) {
        printf("-->sensorDetailPayload: SQL query error: %s", mysql_error(&Mysql));
        return TROUBLE;
    }
    result = mysql_store_result(Connection);
    numProcessed = 0;
    while ( ( row = mysql_fetch_row(result)) != NULL ) {
        if(numRecord == 0){
            retval = create_bucket(accesskey, bucketkey, bucketname);
            if(retval ){
                printf("-->sensorDetailPayload: create_bucket error\n");
                return(TROUBLE);
            }
        }
        buffer = (row[0] ? row[0] : "NULL");
        
        /*** Mapping on text and emois ***/
        if(!strcmp(signalname,"Weather")){
            //mappingWeather2emoi(buffer, value);
            weatherDecode (buffer, value, emoji);
        } else if (!strcmp(signalname,"Weather emoji")){
             weatherDecode (buffer, buffer1, emoji);
            strcpy(value,emoji);
        } else if (!strcmp(signalname,"Wind direction emoji")) {
            windDirectionEmoji(atoi(buffer), emoji);
            strcpy(value,emoji);
        } else if (!strcmp(signalname,"Moon age emoji")) {
            moonAgeEmoji(buffer, emoji);
            strcpy(value,emoji);
        } else {
            strcpy(value,buffer);
        }
        time_stamp = (row[1] ? row[1] : "NULL");
        lastId = atoi((row[2] ? row[2] : "NULL"));
        createPayload(&payload, accesskey, bucketkey, bucketname, time_stamp, value, signalname);
        retval = stream_event(accesskey, bucketkey, payload.json_text);
        printf(".");
        if(retval ){
            printf("-->sensorDetailPayload: stream_event error\n");
            return(TROUBLE);
        }
        numProcessed++;
    }
    mysql_free_result(result);
    printf("--> %u signals %s were sent to Initialstate.com\n", numProcessed, signalname);
    mysql_close(Connection);
    if (numProcessed > 0) {
        updateLastID(bucketname, tabname, lastId, "OK", node, position, attrname, signalname);
    }
    printf("\n\n");
    return OK;
}



/*************************************/
/*** Read all nodes and positions  ***/
/*** for Productivity bucket       ***/
/*************************************/
int readPositionsAndNodes(){
    int queryState;
    int status;
    MYSQL_RES *result;
    MYSQL_ROW row;
    char buffer[80];
    
    status = databaseConnect();
    if(status != OK){
        printf("readPositionsAndNodes: Database connect error!\n");
        return TROUBLE;
    }
    queryState = mysql_query(Connection, SQL_POSITIONS_NODES);
    if (queryState != 0) {
        printf("readPositionsAndNodes: SQL query error: %s", mysql_error(&Mysql));
        return TROUBLE;
    }
    result = mysql_store_result(Connection);
    numWeightPoints = 0;
    while ( ( row = mysql_fetch_row(result)) != NULL ) {
        strcpy(buffer,"");
        sprintf(buffer,"%s-%s-%s",(row[0] ? row[0] : "NULL"), (row[3] ? row[3] : "NULL"), (row[4] ? row[4] : "NULL"));
        strcpy(weightPoint[numWeightPoints].nameSignal, buffer);
        weightPoint[numWeightPoints].node = atoi((row[3] ? row[3] : "NULL"));
        weightPoint[numWeightPoints].position = atoi((row[4] ? row[4] : "NULL"));
        strcpy(weightPoint[numWeightPoints].latitude, (row[1] ? row[1] : "NULL"));
        strcpy(weightPoint[numWeightPoints].longitude, (row[2] ? row[2] : "NULL"));
        strcpy(weightPoint[numWeightPoints].bucketname, (row[5] ? row[5] : "NULL"));
        strcpy(weightPoint[numWeightPoints].bucketkey, (row[6] ? row[6] : "NULL"));
        strcpy(weightPoint[numWeightPoints].accesskey, (row[7] ? row[7] : "NULL"));
        strcpy(weightPoint[numWeightPoints].name_location, (row[8] ? row[8] : "NULL"));
        numWeightPoints++;
    }
    mysql_free_result(result);
    mysql_close(Connection);
    return(OK);
}


/*************************************/
/*** Create payload for sending ot ***/
/*** Initial state - json          ***/
/*************************************/
int createPayload(Payload *payload, char *accesKey, char *bucketKey, char *bucketName, char *timeStamp, char *value, char *nameSignal){
    strcpy(payload->access_key, accesKey);
    strcpy(payload->bucket_key, bucketKey);
    payload->something_to_update = 1;
    strcpy(payload->json_text, "[\n");
    strcat(payload->json_text, "{\n");
    strcat(payload->json_text,"\"");
    strcat(payload->json_text,"iso8601");
    strcat(payload->json_text,"\"");
    strcat(payload->json_text,": \"");
    strcat(payload->json_text,timeStamp);
    strcat(payload->json_text,"\"");
    strcat(payload->json_text,",\n");
    strcat(payload->json_text,"\"");
    strcat(payload->json_text,"key");
    strcat(payload->json_text,"\"");
    strcat(payload->json_text,": \"");
    strcat(payload->json_text,nameSignal);
    strcat(payload->json_text,"\"");
    strcat(payload->json_text,",\n");
    strcat(payload->json_text,"\"");
    strcat(payload->json_text,"value");
    strcat(payload->json_text,"\"");
    strcat(payload->json_text,": \"");
    strcat(payload->json_text,value);
    strcat(payload->json_text,"\"");
    strcat(payload->json_text,"\n}");
    strcat(payload->json_text,"\n");
    strcat(payload->json_text,"] \n");
    return OK;
}


/*************************************/
/*** Create InitialState bucket    ***/
/*************************************/
/*** Code adopted from https://github.com/davidgs/intiial-state-C ***/
/*************************************/
int create_bucket(char *access_key, char *bucket_key, char *bucket_name) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *chunk = NULL;
    char json[1024];
    char tmp[256];
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    if (access_key == NULL || bucket_key == NULL) {
        return 1; // MUST have an access key and bucket key
    }
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, BUCKET_API);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        /* Add a custom header */
        sprintf(tmp, "X-IS-AccessKey: %s", access_key);
        chunk = curl_slist_append(chunk, tmp);
        chunk = curl_slist_append(chunk, "Content-Type: application/json");
        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        sprintf(json, "{\"bucketKey\" : \"%s\", \"bucketName\" : \"%s\"}",
                bucket_key, bucket_name);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
        
#ifdef SKIP_PEER_VERIFICATION
        /*
         * If you want to connect to a site who isn't using a certificate that is
         * signed by one of the certs in the CA bundle you have, you can skip the
         * verification of the server's certificate. This makes the connection
         * A LOT LESS SECURE.
         *
         * If you have a CA cert for the server stored someplace else than in the
         * default bundle, then the CURLOPT_CAPATH option might come handy for
         * you.
         */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
        
#ifdef SKIP_HOSTNAME_VERIFICATION
        /*
         * If the site you're connecting to uses a different host name that what
         * they have mentioned in their server certificate's commonName (or
         * subjectAltName) fields, libcurl will refuse to connect. You can skip
         * this check, but this will make the connection less secure.
         */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
        
        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();
    return 0;
}


/*************************************/
/*** Create InitialState stream    ***/
/*************************************/
/*** Code adopted from https://github.com/davidgs/intiial-state-C ***/
/*************************************/
int stream_event(char *access_key, char *bucket_key, char *json) {
    
    CURL *curl;
    CURLcode res;
    struct curl_slist *chunk = NULL;
    char tmp[1024];
    if (access_key == NULL || bucket_key == NULL) {
        return 1; // MUST have an access key and bucket key
    }
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, EVENT_API);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        /* Add a custom header */
        sprintf(tmp, "X-IS-AccessKey: %s", access_key);
        chunk = curl_slist_append(chunk, tmp);
        sprintf(tmp, "X-IS-BucketKey: %s", bucket_key);
        chunk = curl_slist_append(chunk, tmp);
        chunk = curl_slist_append(chunk, "Content-Type: application/json");
        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        
#ifdef SKIP_PEER_VERIFICATION
        /*
         * If you want to connect to a site who isn't using a certificate that is
         * signed by one of the certs in the CA bundle you have, you can skip the
         * verification of the server's certificate. This makes the connection
         * A LOT LESS SECURE.
         *
         * If you have a CA cert for the server stored someplace else than in the
         * default bundle, then the CURLOPT_CAPATH option might come handy for
         * you.
         */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
        
#ifdef SKIP_HOSTNAME_VERIFICATION
        /*
         * If the site you're connecting to uses a different host name that what
         * they have mentioned in their server certificate's commonName (or
         * subjectAltName) fields, libcurl will refuse to connect. You can skip
         * this check, but this will make the connection less secure.
         */
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
        
        /* Perform the request, res will get the return code */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        
        /* Check for errors */
        
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();
    return 0;
}


/*************************************/
/*** Delay for given period        ***/
/*** in second                    ***/
/*************************************/
void delay_sec (int seconds)
{
    time_t start = time ((time_t *)0);
    double elapsed = 0;
    while (elapsed < seconds) {
        time_t finish = time ((time_t *)0);
        elapsed = difftime (finish, start);
    }
    return;
}


/*************************************/
/*** Connect to the MySQL database ***/
/*************************************/
int databaseConnect(){
    
    mysql_init(&Mysql);
    Connection = mysql_real_connect(&Mysql, DBHOST, USERNAME, PASSWORD , DBNAME, DBPORT, 0, 0);
    if (Connection == NULL) {
        printf("databaseConnect: Connection to MySql database error: %s", mysql_error(&Mysql));
        return TROUBLE;
    } else {
        return OK;
    }
}


/*************************************/
/*** Get ID of the last succesfull ***/
/*** processed record              ***/
/*************************************/
int readLastID(char *bucketname, char *tabname, int node, int position, char *attrname, char *signalname){
    int queryState;
    int status;
    MYSQL_RES *result;
    MYSQL_ROW row;
    char buffer[500];
    int returnedId;
    
    status = databaseConnect();
    if(status != OK){
        printf("readLastID: Database connect error!\n");
        return TROUBLE;
    }
    strcpy(buffer,"");
    sprintf(buffer,SQL_READ_LAST_ID, tabname, bucketname, node, position, attrname, signalname);
    queryState = mysql_query(Connection, buffer);
    if (queryState != 0) {
        printf("readLastID: SQL query error: %s", mysql_error(&Mysql));
        return TROUBLE;
    }
    result = mysql_store_result(Connection);
    while ( ( row = mysql_fetch_row(result)) != NULL ) {
        returnedId = atoi((row[0] ? row[0] : "NULL"));
    }
    mysql_free_result(result);
    mysql_close(Connection);
    return(returnedId);
}


/*************************************/
/*** Insert ID of the last         ***/
/*** succesfull processed record   ***/
/*************************************/
int updateLastID(char *bucketname, char *tabname, int idRecord, char* statement, int node, int position, char *attrname, char *signalname){
    int queryState;
    int status;
    MYSQL_RES *result;
    char buffer[250];
    int returnedId;
    
    status = databaseConnect();
    if(status != OK){
        printf("updateLastID: Database connect error!\n");
        return TROUBLE;
    }
    strcpy(buffer,"");
    sprintf(buffer,SQL_WRITE_LAST_ID, tabname, bucketname, idRecord, statement, node, position, attrname, signalname);
    queryState = mysql_query(Connection, buffer);
    if (queryState != 0) {
        printf("updateID: SQL query error: %s", mysql_error(&Mysql));
        return TROUBLE;
    }
    result = mysql_store_result(Connection);
    mysql_free_result(result);
    mysql_close(Connection);
    return(returnedId);
}


/*************************************/
/*** Decode OpenWeather weather    ***/
/*** codes                         ***/
/*************************************/
void weatherDecode (char *input, char *retval, char *emoji){
    int code;
    
    code = atoi(input);
    switch (code){
        case 200 : strcpy(retval, "thunderstorm with light rain"); break;
        case 201 : strcpy(retval, "thunderstorm with rain"); break;
        case 202 : strcpy(retval,"thunderstorm with heavy rain"); break;
        case 210 : strcpy(retval,"light thunderstorm"); break;
        case 211 : strcpy(retval, "thunderstorm"); break;
        case 212 : strcpy(retval, "heavy thunderstorm"); break;
        case 221 : strcpy(retval, "ragged thunderstorm"); break;
        case 230 : strcpy(retval, "thunderstorm with light drizzle"); break;
        case 231 : strcpy(retval, "thunderstorm with drizzle"); break;
        case 232 : strcpy(retval, "thunderstorm with heavy drizzle"); break;
        case 300 : strcpy(retval, "light intensity drizzle"); break;
        case 301 : strcpy(retval, "drizzle"); break;
        case 302 : strcpy(retval, "heavy intensity drizzle"); break;
        case 310 : strcpy(retval, "light intensity drizzle rain"); break;
        case 311 : strcpy(retval, "drizzle rain"); break;
        case 312 : strcpy(retval, "heavy intensity drizzle rain"); break;
        case 313 : strcpy(retval, "shower rain and drizzle"); break;
        case 314 : strcpy(retval, "heavy shower rain and drizzle"); break;
        case 321 : strcpy(retval, "shower drizzle"); break;
        case 500 : strcpy(retval, "light rain"); break;
        case 501 : strcpy(retval, "moderate rain"); break;
        case 502 : strcpy(retval, "heavy intensity rain"); break;
        case 503 : strcpy(retval, "very heavy rain"); break;
        case 504 : strcpy(retval, "extreme rain"); break;
        case 511 : strcpy(retval, "freezing rain"); break;
        case 520 : strcpy(retval, "light intensity shower rain"); break;
        case 521 : strcpy(retval, "shower rain"); break;
        case 522 : strcpy(retval, "heavy intensity shower rain"); break;
        case 531 : strcpy(retval, "ragged shower rain"); break;
        case 600 : strcpy(retval, "light snow"); break;
        case 601 : strcpy(retval, "snow"); break;
        case 602 : strcpy(retval, "heavy snow"); break;
        case 611 : strcpy(retval, "sleet"); break;
        case 612 : strcpy(retval, "shower sleet"); break;
        case 615 : strcpy(retval, "light rain and snow"); break;
        case 616 : strcpy(retval, "rain and snow"); break;
        case 620 : strcpy(retval, "light shower snow"); break;
        case 621 : strcpy(retval, "shower snow"); break;
        case 622 : strcpy(retval, "heavy shower snow"); break;
        case 701 : strcpy(retval, "mist"); break;
        case 711 : strcpy(retval, "smoke"); break;
        case 721 : strcpy(retval, "haze"); break;
        case 731 : strcpy(retval, "sand, dust whirls"); break;
        case 741 : strcpy(retval, "fog"); break;
        case 751 : strcpy(retval, "sand"); break;
        case 761 : strcpy(retval, "dust"); break;
        case 762 : strcpy(retval, "volcanic ash"); break;
        case 771 : strcpy(retval, "squalls"); break;
        case 781 : strcpy(retval, "tornado"); break;
        case 800 : strcpy(retval, "clear sky"); break;
        case 801 : strcpy(retval, "few clouds"); break;
        case 802 : strcpy(retval, "scattered clouds"); break;
        case 803 : strcpy(retval, "broken clouds"); break;
        case 804 : strcpy(retval, "overcast clouds"); break;
        case 900 : strcpy(retval, "tornado"); break;
        case 901 : strcpy(retval, "tropical storm"); break;
        case 902 : strcpy(retval, "hurricane"); break;
        case 903 : strcpy(retval, "cold"); break;
        case 904 : strcpy(retval, "hot"); break;
        case 905 : strcpy(retval, "windy"); break;
        case 906 : strcpy(retval, "hail"); break;
        case 951 : strcpy(retval, "calm"); break;
        case 952 : strcpy(retval, "light breeze"); break;
        case 953 : strcpy(retval, "gentle breeze"); break;
        case 954 : strcpy(retval, "moderate breeze"); break;
        case 955 : strcpy(retval, "fresh breeze"); break;
        case 956 : strcpy(retval, "strong breeze"); break;
        case 957 : strcpy(retval, "high wind, near gale"); break;
        case 958 : strcpy(retval, "gale"); break;
        case 959 : strcpy(retval, "severe gale"); break;
        case 960 : strcpy(retval, "storm"); break;
        case 961 : strcpy(retval, "violent storm"); break;
        case 962 : strcpy(retval, "hurricane"); break;
    }
    if(code >=200 && code <= 232){
        // Thunderstorm
        strcpy(emoji,EMOJI_STORM);
    } else if (code >= 300 && code <= 321){
        // Drizzle
        strcpy(emoji,EMOJI_DRIZZLE);
    } else if (code >= 500 && code <= 531){
        // Rain
        strcpy(emoji,EMOJI_RAIN);
    } else if (code >= 600 && code <= 622){
        // Snow
        strcpy(emoji,EMOJI_SNOW);
    } else if (code >= 701 && code <= 781){
        // Atmosphere
        strcpy(emoji,EMOJI_ATMOSPHERE);
    } else if (code == 800){
        // Clear
        strcpy(emoji,EMOJI_SUNNY);
    } else if (code >= 801 && code <= 803){
        // few - broken clouds
        strcpy(emoji,EMOJI_SUNNYCLOUD);
    } else if (code == 804){
        // Overcast clouds
        strcpy(emoji,EMOJI_CLOUD);
    } else if (code >= 900 && code <= 902){
        // Extreme - tornado etc.
        strcpy(emoji,EMOJI_WINDSPEED);
    } else {
        // Other
        strcpy(emoji, EMOJI_BOOM);
    }
}


/*************************************/
/*** Decode wind-directions on     ***/
/*** emojis                        ***/
/*************************************/
void windDirectionEmoji(int windDirection, char *emoji){
    if((windDirection >= 355 && windDirection <= 360) || (windDirection >= 0 && windDirection <= 5)){
        strcpy(emoji, EMOJI_NORD);
    } else if (windDirection >= 6 && windDirection <= 85){
        strcpy(emoji, EMOJI_NORD_EAST);
    } else if (windDirection >= 86 && windDirection <= 95){
        strcpy(emoji, EMOJI_EAST);
    } else if (windDirection >= 96 && windDirection <= 175){
        strcpy(emoji, EMOJI_SOUTH_EAST);
    } else if (windDirection >= 176 && windDirection <= 185){
        strcpy(emoji, EMOJI_SOUTH);
    } else if (windDirection >= 186 && windDirection <= 265){
        strcpy(emoji, EMOJI_SOUTH_WEST);
    } else if (windDirection >= 266 && windDirection <= 275){
        strcpy(emoji, EMOJI_WEST);
    } else if (windDirection >= 276 && windDirection <= 354){
        strcpy(emoji, EMOJI_WEST_NORD);
    } else {
        strcpy(emoji, EMOJI_UNDEFINED);
    }
}


/*************************************/
/*** Decode moon phases     on     ***/
/*** emojis                        ***/
/*************************************/
void moonAgeEmoji(char *moonAge, char *emoji){
    
    if(!strcmp(moonAge,"new")){
        strcpy(emoji, EMOJI_NEW_MOON);
    } else if (!strcmp(moonAge,"waxing crescent")){
        strcpy(emoji, EMOJI_WAXINGCRESCENT_MOON);
    } else if (!strcmp(moonAge,"in its first quarter")){
        strcpy(emoji, EMOJI_FIRSTQUARTER_MOON);
    } else if (!strcmp(moonAge,"waxing gibbous")){
        strcpy(emoji, EMOJI_WAXINGGIBBOUS_MOON);
    } else if (!strcmp(moonAge,"full")){
        strcpy(emoji, EMOJI_FULL_MOON);
    } else if (!strcmp(moonAge,"waning gibbous")){
        strcpy(emoji, EMOJI_WANINGGIBBOUS_MOON);
    } else if (!strcmp(moonAge,"in its last quarter")){
        strcpy(emoji, EMOJI_LASTQUARTER_MOON);
    } else if (!strcmp(moonAge,"waning crescent")){
        strcpy(emoji, EMOJI_WANINGCRESCENT_MOON);
    } else {
        strcpy(emoji, EMOJI_UNDEFINED);
    }
}