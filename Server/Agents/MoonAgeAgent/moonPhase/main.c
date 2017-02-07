/****************************************************/
/***  MoonAgeAgent                                ***/
/***  Calculate Moon phase and save it into       ***/
/***  bee_logger database                         ***/
/****************************************************/
#include <stdio.h>
#include <mysql.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../Includes/databaseCredentials.h"

#define OK 0
#define TROUBLE 1
#define REJECTED 2

static char *description[] = {
    "new",                  /* totally dark                         */
    "waxing crescent",      /* increasing to full & quarter light   */
    "in its first quarter", /* increasing to full & half light      */
    "waxing gibbous",       /* increasing to full & > than half     */
    "full",                 /* fully lighted                        */
    "waning gibbous",       /* decreasing from full & > than half   */
    "in its last quarter",  /* decreasing from full & half light    */
    "waning crescent"       /* decreasing from full & quarter light */
};

MYSQL* Connection;
MYSQL Mysql;

int main() {
    int day, month, year, phase, status, queryState;
    time_t now;
    struct tm *lcltime;
    char age[50];
    MYSQL_RES *result;
    char buffersql[500];
  
    printf("/*************************************/ \n");
    printf("/*** BeeMonitor - MoonAgeAgent ***/ \n");
    printf("/*************************************/ \n\n");
    now = time ( NULL );
    lcltime = localtime ( &now );
    day = lcltime->tm_mday;
    month = lcltime->tm_mon+1;
    year = lcltime->tm_year-100+2000;
    phase = moon_age(month, day, year);
    strcpy(age, description[(int)((phase + 2) * 16L / 59L)]);
    printf("-> Moonage = %s\n", age);
    strcpy(buffersql,"");
    sprintf(buffersql,"INSERT INTO moon_age (id, value, time_stamp) VALUES (0,'%s', now());", age);
    status = databaseConnect();
    if(status != OK){
        printf("---> Database connect error!\n");
        return TROUBLE;
    }
    queryState = mysql_query(Connection, buffersql);
    if (queryState != 0) {
        printf("---> SQL statement error: %s\n", mysql_error(&Mysql));
        return TROUBLE;
    }
    result = mysql_store_result(Connection);
    mysql_free_result(result);
    mysql_close(Connection);
    printf("---> OK\n");
    printf("Done.\n");
    return OK;

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


/*****************************************************************************/
/*** Moon age calculation                                                  ***/
/*** Function adopted from http://www8.cs.umu.se/~isak/snippets/moon_age.c ***/
/*****************************************************************************/
int moon_age(int month, int day, int year)
{
    static short int ages[] =
    {18, 0, 11, 22, 3, 14, 25, 6, 17,
        28, 9, 20, 1, 12, 23, 4, 15, 26, 7};
    static short int offsets[] =
    {-1, 1, 0, 1, 2, 3, 4, 5, 7, 7, 9, 9};
    
    if (day == 31)
        day = 1;
    return ((ages[(year + 1) % 19] + ((day + offsets[month-1]) % 30) +
             (year < 1900)) % 30);
}