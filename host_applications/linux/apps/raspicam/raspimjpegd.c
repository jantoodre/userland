#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>//umask
#include <sys/stat.h>//umask
#include <unistd.h>
#include <syslog.h>


int getPIDbyName(char *pName);
int startProcess(char *pName);

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
	
	FILE *daemonPath = fopen("/tmp/raspimjpegd","w");
	fprintf(daemonPath,"%d",(int)getpid());
	fclose(daemonPath);
	
    int procId = getPIDbyName(argv[1]);
    while(1){//Keep it alive

        procId = getPIDbyName(argv[1]);
        if(procId < 0){
			syslog(LOG_NOTICE, "Raspimjpeg is not running!starting raspimjpeg..\n"); 
            //printf("Could not get PID of '%s' (process not running)\n",argv[1]);
            //printf("Starting process '%s'\n",argv[1]);
            startProcess(argv[1]);
            procId = getPIDbyName(argv[1]);
            if(procId < 0) syslog(LOG_NOTICE, "Raspimjpeg started, PID: %d\n",procId); // printf("Could not start '%s'\n", argv[1]);
            else syslog(LOG_NOTICE, "Could not start raspimjpeg.\n"); //printf("'%s' successfully started! PID : %d\n",argv[1], procId);
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
        asprintf(&systemCall, "%s&", pName);//starts process in the background
        system(systemCall);
        free(systemCall);
        return 0;
}
