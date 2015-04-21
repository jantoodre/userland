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
 * \file RaspiMSql.h
 **/
 
#ifndef _RASPIMSQL_H_
#define _RASPIMSQL_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <my_global.h>//
#include <mysql.h>//
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#ifndef _RASPIMJPEG_H_
#include "RaspiMJPEG.h"
#endif


/*Query ID
Classes: Video, Motion,Log,Update,Custom*/
typedef enum
	{
		SQL_INIT,
		SQL_VIDEO_NEW_VIDEO,
		SQL_VIDEO_GET_NEW_ID,
		SQL_MOTION_LOG,
		SQL_MOTION_GET_MOTIONS,
		SQL_MOTION_GET_TOTAL_DURATION,
		SQL_UPDATE_MOTION_DATA,
		SQL_UPDATE_CAMERA_STATUS,
		SQL_READ_CONFIG,
		SQL_SAVE_CONFIG,
		SQL_PURGE,
		SQL_CUSTOM,
		SQL_CLEAN		
	} QUERY_ID;
	
	typedef enum
	{
		c_node_number,
		c_sql_host,c_sql_user,c_sql_psw,c_sql_db,
		c_sql_table_motion,c_sql_table_video,c_sql_table_notifications,
		c_purge_mode,c_purge_level,c_video_path_sql,c_image_path_sql
	}sqlkey_type;
	
	#define SQL_QUERY_SIZE		512
	#define SQL_CFG_SIZE		11
	
extern MYSQL *con;
extern char query[SQL_QUERY_SIZE];
extern unsigned int video_id[2];
extern char *sql_key[SQL_CFG_SIZE + 1];
extern char *sql_stru[SQL_CFG_SIZE + 1];
extern long int sql_val[SQL_CFG_SIZE + 1];
extern int total_motion_duration, nof_motions, video_length;

//Not sure yet how to build the whole structure, since everyone might have their own
//thoughts about how database tables  must look like
/*
typedef enum
	{
		c_sql_host,c_sql_user,c_sql_psw,c_sql_db,
		c_sql_table_motion,c_sql_mtn_id,c_sql_mtn_vid,c_sql_mtn_start,c_sql_mtn_duration,
		c_sql_table_video,c_sql_vid_id,c_sql_vid_node,c_sql_vid_filename,c_sql_vid_mtn,c_sql_vid_mtn_dur,c_sql_vid_rdy,
		c_sql_table_image,c_sql_img_id,c_sql_img_node,c_sql_img_filename,
		c_sql_table_thumb,c_sql_th_id,c_sql_th_node,c_sql_th_vid,//Unused
		c_sql_table_config,c_sql_cfg_id,//must have columns as in program (e.g. 'br' for brightness)
		c_sql_table_notifications,c_sql_ntfy_id,c_sql_ntfy_class,c_sql_ntfy_type,c_sql_ntfy_msg,c_sql_ntfy_dismiss,
		c_sql_table_node,c_sql_node_id
	}sqlkey_type;
*/

/*SQL queries
returns 1 on failure, 0 on sucess
returns result if necessary
Sends string/custom query(without return)
size of buffer(string)*/
extern bool sqlQuery(QUERY_ID id, void *queryResult, char *str, size_t size);
extern bool purge(int timestamp, int key);
extern int getSqlKey(char *key);
extern void addSqlValue(int keyI, char *value);
extern void read_sql_config(char *cfilename);
extern char *sqlTrim(char *s);

#endif




