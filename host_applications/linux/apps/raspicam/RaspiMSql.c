/*
Copyright (c) 2015, Broadcom Europe Ltd
Copyright (c) 2015, Silvan Melchior
Copyright (c) 2015, Robert Tidey
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
 * \date 18th Aprl 2015
 * \Author: Robert Tidey / Jan Toodre
 *
 *
 */
 
 
#include "RaspiMSql.h"

MYSQL *con;
char query[SQL_QUERY_SIZE];
unsigned int video_id[2];
int total_motion_duration = 0, nof_motions = 0;

char *sql_key[SQL_CFG_SIZE + 1] ={
   "node_number",
   "sql_host","sql_user","sql_psw","sql_db",
   "table_motion","table_video","table_notifications"
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
				printf("Video ID: %d\n",video_id[0]);
				mysql_free_result(result);
			}
			memset(query,0,SQL_QUERY_SIZE);
			return 1;				
		case SQL_VIDEO_NEW_VIDEO: 
			//make sure that str 0 at the end doesnt FSU
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
			}
			memset(query,0,SQL_QUERY_SIZE);
			return 1;
		case SQL_MOTION_LOG:
			if(str == NULL) exit(1);//error("Empty notification string!\n",1);
			if(mysql_query(con, str)){
				//error("Inserting notification to database failed!",1);
				exit(1);
			}
			return 1;
		case SQL_MOTION_GET_MOTIONS: 
			snprintf(query,SQL_QUERY_SIZE,"SELECT COUNT(*) FROM Motion WHERE video_id='%d'", video_id[1]);
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
			snprintf(query,SQL_QUERY_SIZE,"SELECT SUM(motion_duration) FROM Motion WHERE video_id='%d'", video_id[1]);
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
			snprintf(query,SQL_QUERY_SIZE,"UPDATE Videos SET nof_motions='%d',total_motion_duration='%d',ready='1' WHERE id='%d'"
			, nof_motions, total_motion_duration, video_id[0]);
			if(mysql_query(con, query)){
				//error("Update query to database failed",0);
				memset(query,0,SQL_QUERY_SIZE);
				return 0;
			}
			memset(query,0,SQL_QUERY_SIZE);
			return 1;
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
