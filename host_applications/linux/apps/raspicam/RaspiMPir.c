/*
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
 * \file RaspiMPir.c
 * Motion detection
 *
 * \date 18th April 2015
 * \Author: Jan Toodre
 *
 */
 
#include "RaspiMPir.h"

time_t motionTime, timeArray[2];
int motionCounter = 0, motionModifier = 3;
char notification_query[NOTIFICATION_SIZE];
int pir_pin;
unsigned int motion_video_id, motion_unix_timestamp;

PI_THREAD (pir_motion){
	/*Interrupt handler initialization*/
	if (wiringPiISR (pir_pin, INT_EDGE_RISING, &motion) < 0)
	{
		printf("Unable to set up ISR for motion!\n");
		exit(1);
	}
	while(1){
		delay(100);
	}
}

void start_pir_motion(int pin){
/* Initialize pin (input, rising edge, pull-up). NB! pin is BCM*/
	pir_pin = pin;
	char pinInit[32];
	snprintf(pinInit, 32, "gpio export %d in", pin);
	system(pinInit);//Make pin program usable
	delay(100);
	memset(pinInit,0,32);
	snprintf(pinInit, 32, "gpio edge %d rising", pin);
	system(pinInit);//set up BCM pin2 trigger on rising edge
	delay(100);
	memset(pinInit,0,32);
	wiringPiSetupSys();
	delay(100);
	snprintf(pinInit, 32, "gpio -g mode %d up", pin);
	system(pinInit);
	delay(100);
	memset(pinInit,0,32);

	int thread = piThreadCreate(pir_motion);
	if(thread != 0) {
		printf("Could not start PIR motion\n");
		exit(1);
	}
}

void motion(void){
	time(&motionTime);
	motion_unix_timestamp = (int)mktime(localtime(&motionTime));				
	while(1){
		while(digitalRead(pir_pin)){
			time(&timeArray[0]);
			delay(500);
			motionCounter=0;
		}
		motionCounter++;
		if(motionCounter >= motionModifier){
		/*Write to database : timestamp, video id, motion duration*/
			snprintf(notification_query, NOTIFICATION_SIZE, "INSERT INTO Motion (video_id, motion_start, motion_duration) VALUES (%d, FROM_UNIXTIME(%d),%d)", motion_video_id, motion_unix_timestamp, 
				(((int)timeArray[0])-(motion_unix_timestamp)));
			sqlQuery(SQL_MOTION_LOG,NULL,notification_query,0);
			/*if (mysql_query(con, notification_query)){
				printf("Could not add motion timestamp\n");
				exit(1);
			}*/
			memset(notification_query, 0, NOTIFICATION_SIZE);
			motionCounter = 0;
			break;
		}
		delay(500);
	}
}

void setVideoID(int id){
	motion_video_id = id;
}

