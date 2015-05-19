/*
Copyright (c) 2015, Broadcom Europe Ltd
Copyright (c) 2015, Jan Toodre
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * \file RaspiMSql.c
 * Handling SQL queries
 *
 * \date 18th April 2015
 * \Author: Jan Toodre
 *
 *
 */
 
 
#include "RaspiMSql.h"

MYSQL *con;
char query[SQL_QUERY_SIZE];
unsigned int video_id[2];
int total_motion_duration = 0, nof_motions = 0, video_length = 0;
int purgeAge = 0, unix_timestamp_now = 0,purge_level = 0;
time_t currentTime;

char *sql_key[SQL_CFG_SIZE + 1] ={
   "node_number",
   "sql_host","sql_user","sql_psw","sql_db",
   "table_motion","table_video","table_notifications",
   "purge_mode","purge_level","video_path","image_path"
};
char *sql_stru[SQL_CFG_SIZE + 1];
long int sql_val[SQL_CFG_SIZE + 1];

extern bool sqlQuery(QUERY_ID id, void *queryResult, char *str, size_t size){
	MYSQL_RES *result;
	MYSQL_ROW row;
	switch(id){
		case SQL_INIT:
			//get sql data
			read_sql_config(str);
			con = mysql_init(NULL);
			if(con == NULL) return 0;
			//read_sql_cfg
			if((mysql_real_connect(con, sql_stru[c_sql_host], sql_stru[c_sql_user], sql_stru[c_sql_psw], 
				sql_stru[c_sql_db], 0, NULL, 0)) == NULL){
				return 0;
			}
			snprintf(query,SQL_QUERY_SIZE,"SELECT MAX(id) FROM %s WHERE cam_id = '%d'",
				"Videos",(int)sql_val[c_node_number]);
			if(mysql_query(con, query)){
				//error
				return 0;
			}else{
				result = mysql_store_result(con);
				row = mysql_fetch_row(result);
				video_id[0] = atoi(row[0]);
				mysql_free_result(result);
			}
			memset(query,0,SQL_QUERY_SIZE);
			return 1;
		case SQL_IMAGE_NEW_IMAGE:
			snprintf(query, SQL_QUERY_SIZE, "INSERT INTO Images (cam_id, filename) VALUES ('%d', '%s')", (int)sql_val[c_node_number], str);
			if (mysql_query(con, query)){
				//error("Could not add new entry for videos",1);
				memset(query,0,SQL_QUERY_SIZE);
				exit(1);
			}
			memset(query, 0, SQL_QUERY_SIZE);
			return 1;
		case SQL_VIDEO_NEW_VIDEO: 
			snprintf(query, SQL_QUERY_SIZE, "INSERT INTO Videos (cam_id, filename) VALUES ('%d', '%s')", (int)sql_val[c_node_number], str);
			if (mysql_query(con, query)){
				//error("Could not add new entry for videos",1);
				memset(query,0,SQL_QUERY_SIZE);
				exit(1);
			}
			memset(query, 0, SQL_QUERY_SIZE);
			return 1;
		case SQL_VIDEO_GET_NEW_ID:
			snprintf(query, SQL_QUERY_SIZE, "SELECT MAX(id) FROM Videos WHERE cam_id='%d'",(int)sql_val[c_node_number]);
			if(mysql_query(con, query)){
				//error("Query to database failed(Unable to update video id for new video)",1);
				memset(query,0,SQL_QUERY_SIZE);
				exit(1);
			}else{
				video_id[1] = video_id[0];
				result = mysql_store_result(con);
				row = mysql_fetch_row(result);
				video_id[0] = atoi(row[0]);
				mysql_free_result(result);
				//Send new video ID to Motion
				setVideoID(video_id[0]);
			}
			memset(query,0,SQL_QUERY_SIZE);
			return 1;
		case SQL_MOTION_LOG:
			if(str == NULL){
				printLog(ERROR, "Error: Could not add motion");
				exit(1);//error("Empty notification string!\n",1);
			}
			if(mysql_query(con, str)){
				//error("Inserting notification to database failed!",1);
				exit(1);
			}
			return 1;
		case SQL_MOTION_GET_MOTIONS: 
			snprintf(query,SQL_QUERY_SIZE,"SELECT COUNT(*) FROM Motion WHERE video_id='%d'", video_id[0]);
			if(mysql_query(con, query)){
				memset(query,0,SQL_QUERY_SIZE);
				return 0;
				//error("Update getting number of motions failed",0);
			}else{
				result = mysql_store_result(con);
				row = mysql_fetch_row(result);
				if (row[0] == NULL){
					nof_motions = 0;
				}else{
					nof_motions = atoi(row[0]);
				}
			}
			memset(query,0,SQL_QUERY_SIZE);
			return 1;
		case SQL_MOTION_GET_TOTAL_DURATION: 
			snprintf(query,SQL_QUERY_SIZE,"SELECT SUM(motion_duration) FROM Motion WHERE video_id='%d'", video_id[0]);
			if(mysql_query(con, query)){
				//error("Getting total motion duration failed",1);
				memset(query,0,SQL_QUERY_SIZE);
				exit(1);
			}else{
				result = mysql_store_result(con);
				row = mysql_fetch_row(result);
				if (row[0] == NULL){
					total_motion_duration = 0;
				}else{
					total_motion_duration = atoi(row[0]);
				}
				mysql_free_result(result);
			}
			memset(query,0,SQL_QUERY_SIZE);
			return 1;
		case SQL_UPDATE_MOTION_DATA: 
			video_length = atoi(str);
			snprintf(query,SQL_QUERY_SIZE,"UPDATE Videos SET video_length='%d',nof_motions='%d',total_motion_duration='%d',ready='1' WHERE id='%d'"
			, video_length, nof_motions, total_motion_duration, video_id[0]);
			video_length = 0;
			if(mysql_query(con, query)){
				//error("Update query to database failed",0);
				memset(query,0,SQL_QUERY_SIZE);
				return 0;
			}
			memset(query,0,SQL_QUERY_SIZE);
			return 1;
		case SQL_UPDATE_CAMERA_STATUS:
			snprintf(query, SQL_QUERY_SIZE, "UPDATE Camera SET stream_status='%s' WHERE id='%d'",str,(int)sql_val[c_node_number]);
			if(mysql_query(con, query)){
				//error("Query to database failed(Unable to update video id for new video)",1);
				memset(query,0,SQL_QUERY_SIZE);
				exit(1);
			}
			memset(query,0,SQL_QUERY_SIZE);
			return 1;
		case SQL_PURGE:
			purgeAge = atoi(str);
			if(purgeAge){
				time(&currentTime);
				unix_timestamp_now = (int)mktime(localtime(&currentTime));
				switch(sql_val[c_purge_mode]){
					//case 0: break;//in seconds | disabled not sure if anyone would use seconds for this..
					case 1: purgeAge = 60 * purgeAge; break;//in minutes
					case 2: purgeAge = 3600 * purgeAge; break;//in hours
					case 3: purgeAge = 86400 * purgeAge; break;//in days
					default: printLog(WARNING, "Invalid purge mode/mode not set!\n"); return 0;				
				}
				switch(sql_val[c_purge_level]){//Maybe some day do this with masking
					case 0: purge_level = c_video_path_sql; break;
					case 1: purge_level = c_image_path_sql; break;
					default: printLog(WARNING, "Invalid purge level/level not set!\n"); return 0;	
				}
				if(purge(unix_timestamp_now-purgeAge,purge_level) == 0){
					printLog(WARNING,"Purge failed!\n");
				}
			}
			break;
		case SQL_READ_CONFIG: //To be implemented
			break;
		case SQL_SAVE_CONFIG: //To be implemented
			break;
		case SQL_CUSTOM: 
			if((str == NULL) || (size == 0)) return 0;

		case SQL_CLEAN:
			mysql_close(con);
			return 1;	
		default: /*error("Invalid query!\n",0);*/ return 0;//fail
	}
	return 0;
}

//function to read sql cfg
bool purge(int timestamp, int key){
	if(timestamp <= 0) return 0;
	MYSQL_RES *presult;
	MYSQL_ROW prow;
	int counter = 0, elnum = 0;
	char *tableName;
	switch(key){//later on replace this.. read from cfg
		case c_video_path_sql: asprintf(&tableName,"Videos"); break;
		case c_image_path_sql: asprintf(&tableName,"Images"); break;
		default: break;
	}
	snprintf(query,SQL_QUERY_SIZE,"SELECT COUNT(*) FROM %s WHERE deleted='0' AND date <= FROM_UNIXTIME(%d)",tableName,timestamp);
	if(mysql_query(con, query)){
		//error("Query to database failed(Unable to update video id for new video)",1);
		memset(query,0,SQL_QUERY_SIZE);
		printLog(WARNING, "Purging failed!(Query failed)\n");
		return 0;
	}
	presult = mysql_store_result(con);
	prow = mysql_fetch_row(presult);
	counter = atoi(prow[0]);//get number of results	
	snprintf(query,SQL_QUERY_SIZE,"SELECT filename FROM %s WHERE deleted='0' AND date <= FROM_UNIXTIME(%d)",tableName,timestamp);
	if(mysql_query(con, query)){
		//error("Query to database failed(Unable to update video id for new video)",1);
		memset(query,0,SQL_QUERY_SIZE);
		printLog(ERROR,"Purging failed!(Query failed)\n");
		exit(1);
	}else{
		presult = mysql_store_result(con);
		//prow = mysql_fetch_row(presult);
		char videoArray[counter][64];
		for(elnum = 0; elnum < counter && (prow = mysql_fetch_row(presult)); elnum++){//populate table with filenames
			//prow = mysql_fetch_row(presult);
				snprintf(videoArray[elnum],64,"%s",prow[0]);
		}
		//update database and deleted
		for(elnum = 0; elnum < counter; elnum++){//populate table with filenames
			printf("%s\n",videoArray[elnum]);//for testing
			snprintf(query,SQL_QUERY_SIZE,"UPDATE %s SET deleted='1' WHERE filename='%s'",tableName,videoArray[elnum]);
			if(mysql_query(con, query)){
				memset(query,0,SQL_QUERY_SIZE);
				return 0;
			}
			memset(query,0,SQL_QUERY_SIZE);
			snprintf(query,SQL_QUERY_SIZE, "sudo rm %s%s*",sql_stru[key],videoArray[elnum]);//should remove .th aswell
			system(query);
			memset(query,0,SQL_QUERY_SIZE);
		}
	}	
	mysql_free_result(presult);
	if(counter)
		snprintf(query,SQL_QUERY_SIZE,"Purge successful! Deleted %d items from %s\n", counter, sql_stru[key]);
	else
		snprintf(query,SQL_QUERY_SIZE,"No files to purge from %s!\n", sql_stru[key]);
	printLog(INFO, query);
	memset(query,0,SQL_QUERY_SIZE);
	free(tableName);
	return 1;
}

int getSqlKey(char *key) {
   int i;
   for(i=0; i < SQL_CFG_SIZE; i++) {
      if(strcmp(key, sql_key[i]) == 0) {
         break;
      }
   }
   return i;
}

void addSqlValue(int keyI, char *value){
   long int val=strtol(value, NULL, 10);

   if (sql_stru[keyI] != 0) free(sql_stru[keyI]);
      asprintf(&sql_stru[keyI],"%s", value);
   if (strcmp(value, "true") == 0)
      val = 1;
   else if (strcmp(value, "false") == 0)
      val = 0;
   sql_val[keyI] = val;
}

void read_sql_config(char *cfilename) {
   FILE *fp;
   int length;
   unsigned int len = 0;
   char *line = NULL;
   char *value = NULL;

   fp = fopen(cfilename, "r");
   if(fp != NULL) {
      while((length = getline(&line, &len, fp)) != -1) {
         line[length-1] = 0;
         value = strchr(line, ' ');
         if (value != NULL) {
            // split line into key, value
            *value = 0;
            value++;
            value = sqlTrim(value);
            if (strlen(line) > 0 && *line != '#' && strlen(value) > 0) {
               addSqlValue(getSqlKey(line), value);
            }
         }
      }
      if(line) free(line);
   }   
}

char *sqlTrim(char *s) {
   char *end = s + strlen(s)-1;
   while(*s && isspace(*s))
      *s++ = 0;
   while(isspace(*end))
      *end-- = 0;
   return s;
}
