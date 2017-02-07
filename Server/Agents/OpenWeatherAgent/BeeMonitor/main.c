/****************************************************/
/***  OpenWeatherAgent                            ***/
/***  Load data from openWeather.com and save it  ***/
/***  into bee_logger database                    ***/
/****************************************************/
#include <stdio.h>
#include <mysql.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <time.h>
#include "../../Includes/databaseCredentials.h"

#define OPENWEATHER_API_STRING "http://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&APPID=%s"
#define OPENWEATHER_APIKEY "6f4cf766dcc7765036e962eee7f20527"
#define KELVIN -272.15

#define OK 0
#define TROUBLE 1
#define REJECTED 2

#define MAXARRAY 1000
#define EXTMETEO_ATTEMPTS 3
#define EXTMETEO_DELAY

typedef struct {
    char windDirection[10];
    char windSpeed[10];
    char cloudiness[10];
    char pressure[10];
    char humidity[10];
    char temperature[10];
    char visibility[10];
    char weather[10];
} MeteoProvider;


typedef struct {
    char nameSignal[80];
    int node;
    int position;
    char latitude[60];
    char longitude[60];
    char bucketname[60];
    char bucketkey[60];
    char accesskey[60];
    char name_location[60];
} weightPoints;


typedef struct
{
    char json_text[500];
    char* access_key [100];
    char* bucket_key [100];
    int something_to_update;
} Payload;

struct string {
    char *ptr;
    size_t len;
};


int create_bucket(char *access_key, char *bucket_key, char *bucket_name);
int stream_event(char *access_key, char *bucket_key, char *json);
int readPositionsAndNodes();
void delay_sec (int seconds);
void getAllExternalMeteo();
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s);
void init_string(struct string *s);


/*** Global variables ***/
MeteoProvider providerMeteo;
MYSQL* Connection;
MYSQL Mysql;
weightPoints weightPoint[10];
int numWeightPoints;



int main() {
    int state;
   
    printf("/*************************************/ \n");
    printf("/*** BeeMonitor - OpenWeatherAgent ***/ \n");
    printf("/*************************************/ \n\n");
    state = readPositionsAndNodes();
   if(state != OK) {
        printf("readpositionAndNodes error\n");
        return(TROUBLE);
   }
   
   // Download external meteo information
   getAllExternalMeteo();
    
  printf("Done.\n");
  return(OK);
}




int testCashForOpenMeteo(char *latitude, char *longitude){
        int queryState;
        int status;
        MYSQL_RES *result;
        MYSQL_ROW row;
        char sqlbuffer[500];
        int retval;
        
        status = databaseConnect();
        if(status != OK){
            printf("testCashForOpenMeteo: Database connect error!\n");
            return -99;
        }
        sprintf(sqlbuffer,"SELECT ROUND(IFNULL(TIME_TO_SEC(now()) - TIME_TO_SEC(max(external_meteo.time_stamp))/60,0),0) FROM bee_logger.external_meteo, bee_logger.locations WHERE bee_logger.locations.node = bee_logger.external_meteo.node AND bee_logger.locations.latitude = '%s' AND bee_logger.locations.longitude = '%s';", latitude, longitude);
        queryState = mysql_query(Connection, "SELECT DISTINCT locations.name_location, locations.latitude, locations.longitude, hive_weight.node, hive_weight.position, locations.bucketname, locations.bucketkey, locations.accesskey, locations.name_location FROM locations, hive_weight WHERE locations.node =  hive_weight.node;");
        if (queryState != 0) {
            printf("testCashForOpenMeteo: SQL query error: %s", mysql_error(&Mysql));
            return -99;
        }
        result = mysql_store_result(Connection);
        while ( ( row = mysql_fetch_row(result)) != NULL ) {
            retval = atoi((row[0] ? row[0] : "NULL"));
        }
        if (retval == 0){
            retval = 10000;
        }
        mysql_free_result(result);
        mysql_close(Connection);
        return(retval);
}


/*************************************/
/*** Collect external meteo        ***/
/*** for all nodes                 ***/
/*************************************/
void getAllExternalMeteo() {
    int i;
    int attempts;
    int state;
    char last_latitude[60];
    char last_longitude[60];
    
    strcpy(last_latitude, "");
    strcpy(last_longitude, "");
    for (i=0;i<=numWeightPoints-1;i++){
        if(testCashForOpenMeteo(weightPoint[i].latitude, weightPoint[i].longitude) > 60){
          if(strcmp(last_latitude,weightPoint[i].latitude) && strcmp(last_longitude,weightPoint[i].longitude)) {
            strcpy(last_latitude, weightPoint[i].latitude);
            strcpy(last_longitude, weightPoint[i].longitude);
            attempts = 0;
            do {
                state = saveExternalMeteo(i);
                attempts++;
                delay_sec(3);
            } while(attempts < EXTMETEO_ATTEMPTS && state == REJECTED);
            if(state == REJECTED){
                printf("---> Request for extenal meteo information for %s %s was rejected by the provider\n\n\n", weightPoint[i].latitude, weightPoint[i].longitude);
            }
          }
        }
    }
}


/*************************************/
/*** Save acquired external meteo  ***/
/*** information for given node    ***/
/*** into database                 ***/
/*************************************/
int saveExternalMeteo(int index){
    MeteoProvider meteo;
    char meteoBuffer[1000];
    char buffer[10];
    int resultMeteo;
    double temperature;
    int queryState;
    int status;
    MYSQL_RES *result;
    char buffersql[500];

    printf("-> Download OpenWeather data into database ...\n");
        printf("--> Download position %s latitude: %s longitude: %s\n",weightPoint[index].nameSignal, weightPoint[index].latitude, weightPoint[index].longitude);
       resultMeteo = getExternalMeteo(meteoBuffer, weightPoint[index].latitude, weightPoint[index].longitude);
       if(strcmp(meteoBuffer, "{\"cod\":\"404\",\"message\":\"Error: Not found city\"}")){
           findToken(meteoBuffer,"weather", meteo.weather);
           findToken(meteoBuffer,"temp", buffer);
           temperature = atof(buffer);
           temperature = temperature + KELVIN;
           if(temperature < -200){
               printf("--->saveExternalMeteo: Wrong data!\n\n");
               return TROUBLE;
           }
           sprintf(meteo.temperature,"%.2f",temperature);
           findToken(meteoBuffer,"pressure", meteo.pressure);
           findToken(meteoBuffer,"humidity", meteo.humidity);
           findToken(meteoBuffer,"visibility", meteo.visibility);
           findToken(meteoBuffer,"clouds", meteo.cloudiness);
           findToken(meteoBuffer,"speed", meteo.windSpeed);
           findToken(meteoBuffer,"deg", meteo.windDirection);
           status = databaseConnect();
           if(status != OK){
               printf("--->saveExternalMeteo: Database connect error!\n");
               return TROUBLE;
           }
           strcpy(buffersql,"");
           sprintf(buffersql,"INSERT INTO external_meteo (id, wind_direction, wind_speed, cloudiness, pressure, humidity, temperature, visibility, weather, node, time_stamp) VALUES (0,'%s','%s','%s','%s','%s','%s','%s','%s','%u',now());", meteo.windDirection, meteo.windSpeed, meteo.cloudiness, meteo.pressure, meteo.humidity, meteo.temperature, meteo.visibility, meteo.weather, weightPoint[index].node);
           queryState = mysql_query(Connection, buffersql);
           if (queryState != 0) {
               printf("---> saveExternalMeteo: SQL statement error: %s\n", mysql_error(&Mysql));
               return TROUBLE;
           }
           result = mysql_store_result(Connection);
           mysql_free_result(result);
           mysql_close(Connection);
           printf("---> OK\n");
           return OK;
       }
    printf("--> Rejected by the meteo-provider\n\n");
    return REJECTED;
}



/*************************************/
/*** Read external meteo-data for  ***/
/*** GPS co-ordinates              ***/
/*************************************/
int getExternalMeteo(char *meteobuffer, char *latitude, char *longitude) {
    char urlBuffer[255];
    int i;
    
    sprintf(urlBuffer, OPENWEATHER_API_STRING,latitude,longitude, OPENWEATHER_APIKEY);
    CURL *curl = curl_easy_init();
    if(curl) {
        struct string s;
        init_string(&s);
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, urlBuffer);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
           curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        res = curl_easy_perform(curl);
        //sprintf(meteobuffer,"%s", s.ptr);
        for(i=0;i<=s.len-1;i++){
            meteobuffer[i] = s.ptr[i];
        }
        free(s.ptr);
        
        curl_easy_cleanup(curl);
    }
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
    queryState = mysql_query(Connection, "SELECT DISTINCT locations.name_location, locations.latitude, locations.longitude, hive_weight.node, hive_weight.position, locations.bucketname, locations.bucketkey, locations.accesskey, locations.name_location FROM locations, hive_weight WHERE locations.node =  hive_weight.node;");
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
/*** Find token of given meteo     ***/
/*** category in string/json       ***/
/*** returned from OpenWeather     ***/
/*************************************/
int findToken(char *source, char *token, char *retval){
    char *position;
    int index;
    int offset;
    int delta;
    int i;
    int result;
    
    position = strstr (source, token);
    index = position-source;
    if (index > 0) {
        if(!strcmp(token,"weather")){
            delta = 8;
        } else if (!strcmp(token,"temp")){
            delta = 1;
        } else if (!strcmp(token,"pressure")){
            delta = 1;
        } else if (!strcmp(token,"humidity")){
            delta = 1;
        } else if (!strcmp(token,"visibility")){
            delta = 1;
        } else if (!strcmp(token,"clouds")){
            delta = 8;
        }
        else if (!strcmp(token,"speed")){
            delta = 1;
        }
        else if (!strcmp(token,"deg")){
            delta = 1;
        }
        offset = delta + strlen(token);
        i = 0;
        do {
            retval[i] = source[ index + offset + i + 1];
            i++;
        } while(retval[i-1] != ',' && retval[i-1] != '}' && retval[i-1] != '"');
        retval[i-1] = '\0';
        result = OK;
    } else {
        strcpy(retval,"");
        result = TROUBLE;
    }
    return result;
}

/*************************************/
/*** Write into                    ***/
/*** string structure              ***/
/*************************************/
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
    size_t new_len = s->len + size*nmemb;
    s->ptr = realloc(s->ptr, new_len+1);
    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr+s->len, ptr, size*nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;
    
    return size*nmemb;
}


/*************************************/
/*** Initialisation of the         ***/
/*** string structure              ***/
/*************************************/
void init_string(struct string *s) {
    s->len = 0;
    s->ptr = malloc(s->len+1);
    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
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







