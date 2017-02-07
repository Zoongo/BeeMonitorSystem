/****************************************************/
/***  InitialStateAgent.h                         ***/
/***  Send data from the bee_logger database to   ***/
/***  InitialState site                           ***/
/***                                              ***/
/*** (c) 2016, Zoongo Ltd., Tomas Ivansky         ***/
/****************************************************/


//--- Status messagess
#define SIGNAL_OK "--> Signal %s OK\n\n"
#define SIGNAL_ERR "--> Problem! Sending of %s signal failure\n\n"

//--- InitialState API links
#define BUCKET_API "https://groker.initialstate.com/api/buckets"
#define EVENT_API "https://groker.initialstate.com/api/events"

//--- Execution statuses
#define OK 0
#define TROUBLE 1
#define REJECTED 2

//--- Maximal number of weighting sensors - points
#define MAX_WEIGHT_POINTS 10

//--- Production bucket params
#define PRODUCTION_NAME "Production"
#define PRODUCTION_BUCKET_KEY "PK3KDKQ36P8N"
#define PRODUCTION_ACCESS_KEY "gAkZfOLg4rMB0kVUSd1gdYbn2xeXLkVM"

//--- Telemetry bucket params
#define TELEMETRY_NAME "Telemetry"
#define TELEMETRY_BUCKET_KEY "J83YHV4HULLS"
#define TELEMETRY_ACCESS_KEY "gAkZfOLg4rMB0kVUSd1gdYbn2xeXLkVM"

typedef struct  {
    char *tempHive;
    char *wghtHive;
    char *position;
    char *node;
    char *appiary;
} BeeSensors;

typedef struct {
    char *temperature1;
    char *temperature2;
    char *humidity;
    char *pressure;
} MyMeteoSensors;

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

//--- Function prototypes
int productivityPayload();
int nodesDetailedInfoPayload();
int telemetryPayload();
int sensorDetailPayload(int node, int position, char *bucketname, char *bucketkey, char *accesskey, char *signalname, char *initquery, char *normalquery, char *tabname, char *attrname);
int create_bucket(char *access_key, char *bucket_key, char *bucket_name);
int stream_event(char *access_key, char *bucket_key, char *json);
int readPositionsAndNodes();
void delay_sec (int seconds);
void getAllExternalMeteo();
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s);
void init_string(struct string *s);
void mappingWeather2emoi(char *weatherCode, char *emojiCode);
void weatherDecode (char *input, char *retval, char *emoji);
void windDirectionEmoji(int windDirection, char *emoji);
void moonAgeEmoji(char *moonAge, char *emoji);
int readLastID(char *bucketname, char *tabname, int node, int position, char *attrname, char *signalname);
int databaseConnect();
int createPayload(Payload *payload, char *accesKey, char *bucketKey, char *bucketName, char *timeStamp, char *value, char *nameSignal);
int updateLastID(char *bucketname, char *tabname, int idRecord, char* statement, int node, int position, char *attrname, char *signalname);
