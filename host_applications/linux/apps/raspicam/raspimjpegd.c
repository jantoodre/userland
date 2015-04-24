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
 * \file raspimjpegd.c
 * \date 23rd April 2015
 * \Author: Jan Toodre
 *
 * Description
 * This is raspberrymjpeg daemon to keep programm running 24/7.
 * The program terminates itself after receiving a SIGINT or
 * SIGTERM.
 * 
 * Installation:
 * Program can be compiled using : sudo gcc raspimjpegd.c -o raspimjpegd
 * Copy this to /usr/bin/
 * NB! RaspiMJPEG.c should be updated aswell! (However if functionality needs testing then
 * daemon should be started with "sudo raspimjpegd raspimjpeg")
 **/


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>


int getPIDbyName(char *pName);
int startProcess(char *pName);
void term (int signum);

int running = 1;

int main(int argc, char **argv){
	if(argc < 2){
		return 1;
	}
	pid_t pid;
	
	/*Clone to child*/
	pid = fork();
	/*Forking failed*/
	if(pid < 0){
		exit(EXIT_FAILURE);
	}
    /*Fork success, parent*/
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}
	
	/*Goes to child*/
	
	/* Open a connection to the syslog server */
	openlog(argv[0],LOG_NOWAIT|LOG_PID,LOG_USER); 
	 
	/* Sends a message to the syslog daemon */
	syslog(LOG_NOTICE, "Successfully started raspimjpeg daemon\n"); 
	 
	umask(0);
	
	pid_t sid;
	/*Trying to create own process group*/
	sid = setsid();
	if (sid < 0){
		syslog(LOG_ERR, "Could not create process group\n");
		exit(EXIT_FAILURE);
	}
	/*change working directory*/
	if ((chdir("/")) < 0){
		syslog(LOG_ERR, "Could not change working directory to /\n");
		exit(EXIT_FAILURE);
	}
	
	/*Closing std streams*/
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
	/*Setting up signals*/
	struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
	
	FILE *daemonPath = fopen("/tmp/raspimjpegd","w");
	fprintf(daemonPath,"%d",(int)getpid());
	fclose(daemonPath);
	
    int procId;// = getPIDbyName(argv[1]);
    while(running){//Keep it alive

        procId = getPIDbyName(argv[1]);
        if(procId < 0){
			syslog(LOG_NOTICE, "Raspimjpeg is not running!starting raspimjpeg..\n"); 
            startProcess(argv[1]);
			usleep(100000);
            procId = getPIDbyName(argv[1]);
            if(procId > 0) syslog(LOG_NOTICE, "Raspimjpeg started, PID: %d\n",procId);
            else syslog(LOG_NOTICE, "Could not start raspimjpeg.\n"); 
        }else{
			usleep(1000000);
        }
    }
	daemonPath = fopen("/tmp/raspimjpegd","w");
	fprintf(daemonPath,"%d",0);
	fclose(daemonPath);
	closelog();
	/*end of child*/
    return 0;
}

int getPIDbyName(char *pName){
        FILE *fp;
        int processID = -1;
        char *systemCall;
        if(pName != 0){
                asprintf(&systemCall,"pidof %s > /tmp/pidof",pName);
                system(systemCall);
                fp = fopen("/tmp/pidof", "r");
                fscanf(fp , "%d", &processID);
                fclose(fp);
                free(systemCall);
        }
        return processID;
}

int startProcess(char *pName){
        char *systemCall;
        asprintf(&systemCall, "sudo %s&", pName);//starts process in the background
        system(systemCall);
        free(systemCall);
        return 0;
}

void term (int signum){
	running = 0;
}
