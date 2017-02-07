/****************************************************/
/***   sqlStatements.h                            ***/
/***  Send data from the bee_logger database to   ***/
/***  InitialState site                           ***/
/***                                              ***/
/*** (c) 2016, Zoongo Ltd., Tomas Ivansky         ***/
/****************************************************/

#define SQL_TEMPSENS_DET_FRST "SELECT value, DATE_FORMAT(time_stamp, %s), id FROM whtr_temp WHERE node = %u AND position = %u ORDER BY id"
#define SQL_TEMPSENS_DET_NXT "SELECT value, DATE_FORMAT(time_stamp, %s), id FROM whtr_temp WHERE node = %u AND position = %u AND id > %u ORDER BY id"
#define SQL_TEMPTLM_DET_FRST "SELECT value, DATE_FORMAT(time_stamp, %s), id FROM tlm_temp WHERE node = %u ORDER BY id"
#define SQL_TEMPTLM_DET_NXT "SELECT value, DATE_FORMAT(time_stamp, %s), id FROM tlm_temp WHERE node = %u AND  id > %u ORDER BY id"
#define SQL_TEMPTLM_DET_AVG_FRST "SELECT AVG(value), DATE_FORMAT(DATE(time_stamp), %s), (SELECT max(id) FROM bee_logger.tlm_temp) FROM bee_logger.tlm_temp WHERE node = %u GROUP BY 2 ORDER BY 2;"
#define SQL_TEMPTLM_DET_AVG_NXT "SELECT AVG(value), DATE_FORMAT(DATE(time_stamp), %s), (SELECT max(id) FROM bee_logger.tlm_temp) FROM bee_logger.tlm_temp WHERE node = %u AND id > %u GROUP BY 2 ORDER BY 2;"
#define SQL_TEMPTLM_DET_MIN_FRST "SELECT MIN(value), DATE_FORMAT(DATE(time_stamp), %s), (SELECT max(id) FROM bee_logger.tlm_temp) FROM bee_logger.tlm_temp WHERE node = %u GROUP BY 2 ORDER BY 2;"
#define SQL_TEMPTLM_DET_MIN_NXT "SELECT MIN(value), DATE_FORMAT(DATE(time_stamp), %s), (SELECT max(id) FROM bee_logger.tlm_temp) FROM bee_logger.tlm_temp WHERE node = %u AND id > %u GROUP BY 2 ORDER BY 2;"
#define SQL_TEMPTLM_DET_MAX_FRST "SELECT MAX(value), DATE_FORMAT(DATE(time_stamp), %s), (SELECT max(id) FROM bee_logger.tlm_temp) FROM bee_logger.tlm_temp WHERE node = %u GROUP BY 2 ORDER BY 2;"
#define SQL_TEMPTLM_DET_MAX_NXT "SELECT MAX(value), DATE_FORMAT(DATE(time_stamp), %s), (SELECT max(id) FROM bee_logger.tlm_temp) FROM bee_logger.tlm_temp WHERE node = %u AND id > %u GROUP BY 2 ORDER BY 2;"
#define SQL_WGHT_DET_FRST "SELECT value, DATE_FORMAT(time_stamp, %s), id FROM hive_weight WHERE node = %u AND position = %u ORDER BY id"
#define SQL_WGHT_DET_NXT "SELECT value, DATE_FORMAT(time_stamp, %s), id FROM hive_weight WHERE node = %u AND position = %u AND id > %u ORDER BY id"
#define SQL_METEO_CLOUDINESS_FRST "SELECT cloudiness, DATE_FORMAT(time_stamp, %s), id FROM external_meteo WHERE node = %u ORDER BY id"
#define SQL_METEO_CLOUDINESS_NXT "SELECT cloudiness, DATE_FORMAT(time_stamp, %s), id FROM external_meteo WHERE node = %u AND id > %u ORDER BY id"
#define SQL_METEO_PRESSURE_FRST "SELECT pressure, DATE_FORMAT(time_stamp, %s), id FROM external_meteo WHERE node = %u ORDER BY id"
#define SQL_METEO_PRESSURE_NXT "SELECT pressure, DATE_FORMAT(time_stamp, %s), id FROM external_meteo WHERE node = %u AND id > %u ORDER BY id"
#define SQL_METEO_WINDSPEED_FRST "SELECT wind_speed, DATE_FORMAT(time_stamp, %s), id FROM external_meteo WHERE node = %u ORDER BY id"
#define SQL_METEO_WINDSPEED_NXT "SELECT wind_speed, DATE_FORMAT(time_stamp, %s), id FROM external_meteo WHERE node = %u AND id > %u ORDER BY id"
#define SQL_METEO_WINDDIRECTION_FRST "SELECT wind_direction, DATE_FORMAT(time_stamp, %s), id FROM external_meteo WHERE node = %u ORDER BY id"
#define SQL_METEO_WINDDIRECTION_NXT "SELECT wind_direction, DATE_FORMAT(time_stamp, %s), id FROM external_meteo WHERE node = %u AND id > %u ORDER BY id"
#define SQL_METEO_WEATHER_FRST "SELECT weather, DATE_FORMAT(time_stamp, %s), id FROM external_meteo WHERE node = %u ORDER BY id"
#define SQL_METEO_WEATHER_NXT "SELECT weather, DATE_FORMAT(time_stamp, %s), id FROM external_meteo WHERE node = %u AND id > %u ORDER BY id"
#define SQL_MOONAGE_FRST "SELECT value, DATE_FORMAT(time_stamp, %s), id FROM moon_age ORDER BY id"
#define SQL_MOONAGE_NXT "SELECT value, DATE_FORMAT(time_stamp, %s), id FROM moon_age WHERE id > %u ORDER BY id"
#define SQL_POSITIONS_NODES "SELECT DISTINCT locations.name_location, locations.latitude, locations.longitude, hive_weight.node, hive_weight.position, locations.bucketname, locations.bucketkey, locations.accesskey, locations.name_location FROM locations, hive_weight WHERE locations.node =  hive_weight.node;"
#define SQL_READ_LAST_ID "SELECT max(id_record) FROM istate_log WHERE tabname = '%s' AND bucketname = '%s' AND node = %d AND position = %d AND attribute = '%s' AND signalname = '%s';"
#define SQL_WRITE_LAST_ID "INSERT INTO istate_log (id, tabname, bucketname, id_record, status, node, position, attribute, signalname) VALUES (0,'%s','%s',%u,'%s', %u, %d, '%s', '%s');"
#define SQL_TLM_VOLTAGE_FRST "SELECT value, DATE_FORMAT(time_stamp, %s), id FROM tlm_voltage WHERE node = %u ORDER BY id"
#define SQL_TLM_VOLTAGE_NXT "SELECT value, DATE_FORMAT(time_stamp, %s), id FROM tlm_voltage WHERE node = %u AND id > %u ORDER BY id"
#define INITIALSTATE_TIME_FORMAT "'%Y-%m-%dT%H:%i+01:00'"
