#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

#include "mysock.h"
#include "struct.h"
#include "myfile.h"

int nRecords = 3; //How many records once to read and send. 
int nProcess = 3; //How many process to process.
int nBinuLevel = 0; //BiNu level number.
char cFileName[1024]; //Cache file name.
int cStopFlag = 0; //Signal flag for stop. 1 stop 0 normal
char SMS_LOG_PATH[1024] = "";
char SMS_DATA_PATH[512] = "";
char SENDSMS_NAME[128] = "";
char SMS_ENGINE_PATH[512] = "";
char PHP_SMS_PROVIDER_PATH[512] = "";

/**
 * Help message.
 * @param sProgram command name
 * @return null 
 **/
void Usage(char* sProgram)
{
	fprintf(stdout, "Usage: %s start|stop|help -l Level ID [-r Records Number] [-n Process Number]\n", sProgram);	
	fprintf(stdout, "       start     Start this service.\n"); 
	fprintf(stdout, "       stop      Stop this service.\n"); 	
	fprintf(stdout, "       help      Print this help message.\n"); 
    fprintf(stdout, "       -l        BiNu level number.(1 - 10)\n"); 
	fprintf(stdout, "       -r        How many records once to read and send.(default 3, 1-20).\n"); 
    fprintf(stdout, "       -n        How many process to process.(default 3, 1-50)\n"); 
    
	exit(1);
}

/**
 * Set stop flag to true
 * @param 
 * @return null 
 **/
void set_stop_flag(){
    cStopFlag = 1;    
}

/**
 * Stop the service.
 * @param sProgram command name
 * @return null 
 **/
void service_stop(char* sProgram){
	char cmd[128];
	snprintf(cmd, sizeof(cmd), "killall -2 %s", SENDSMS_NAME);
	system(cmd);	    
    //printf("----------cmd %s \n",cmd);
    exit(1);
}

/**
 * Main logic function, fetch the data from cache file and send SMS
 * @param fifofile cache file name
 * @return null 
 **/
int LoopOnInMsg(const char *fifofile){    
    char* newMessage;
    char* newFname;
    int count_read = 1;
    
    RequestBody rBody;    
    char cmd[1600], log_file[250], now_time[25], log_text[1500];
    size_t readsize;
    RequestBody *readbuf, *tmpreadbuf;
    int loopcount = 0;
    int i = 0;
    int record_count = 0;
    long current_time;
    
    
    memset ((void *) &rBody, 0, sizeof (RequestBody));
    memset ((void *) &cmd, 0, sizeof (cmd));
    memset((char*) &log_file, 0, sizeof(log_file));
    memset((char*) &now_time, 0, sizeof(now_time));
    memset ((void *) &log_text, 0, sizeof (log_text));
    
    //fprintf(stdout, "read file [%s]\n", fifofile);
    
    //malloc the memory for records.
    readsize = sizeof (RequestBody) * nRecords;
    readbuf = (RequestBody *) malloc (readsize);
    if (!readbuf){
        perror ("malloc");
        exit (1);
    }
    
    while(1){
        //printf("-----------cStopFlag[%d]----------\n", cStopFlag);
        //sleep(1);
        if (cStopFlag != 0){
            //fprintf(stderr,"Received the stop signal\n");
            break;
        }
        
        if (fiforead (fifofile, (char *) readbuf, readsize) != readsize){
            if (fiforead (fifofile, (char *) readbuf, sizeof (RequestBody)) != sizeof (RequestBody)){
                //usleep(10000);
                sleep (1);
                continue;
            }
            else{
                loopcount = 1;
            }
        }
        else{
            loopcount = nRecords;
        }
        
        for (i = 0; i < loopcount; i++){
            tmpreadbuf = readbuf + i;    
            memcpy ((void *) &rBody, (void *) tmpreadbuf, sizeof (RequestBody));
            record_count++;    
            
            rBody.msglen[9] = 0;
            rBody.msgid[19] = 0;
            rBody.credit_id[19] = 0;
            rBody.from[19] = 0;
            rBody.from_name[19] = 0;
            rBody.from_account_id[19] = 0;
            rBody.to[19] = 0;
            rBody.to_account_id[19] = 0;
            rBody.provider[19] = 0;
            rBody.device_ip[19] = 0;        
            rBody.device_id[19] = 0;
            rBody.post_script[19] = 0;            
            rBody.app_id[9] = 0;        
            rBody.binulevel[4] = 0;
            rBody.dup_key[34] = 0;
            rBody.msg_type[19] = 0;
            rBody.sms_count[1] = 0;
            rBody.provider_type[1] = 0;
            rBody.message[499] = 0;
            rBody.sent_providers[199] = 0;
            rBody.reserved[459] = 0;
            
                
            trimstring(rBody.msglen);
            trimstring(rBody.msgid);
            trimstring(rBody.credit_id);//credit engine transaction ID        
            trimstring(rBody.from);
            trimstring(rBody.from_name);
            trimstring(rBody.from_account_id);
            trimstring(rBody.to);
            trimstring(rBody.to_account_id);
            trimstring(rBody.provider);
            trimstring(rBody.device_ip);
            trimstring(rBody.device_id);
            //trimstring(rBody.post_script);
            chopstring(rBody.post_script);
            trimstring(rBody.app_id);        
            trimstring(rBody.binulevel);
            trimstring(rBody.dup_key);
            trimstring(rBody.msg_type);
            trimstring(rBody.sms_count);
            trimstring(rBody.provider_type);
            //trimstring(rBody.message);
            chopstring(rBody.message);
            trimstring(rBody.sent_providers);
            trimstring(rBody.reserved);
            
            //print out 
            //fprintf(stdout, "msgid[%s] credit_id[%s] from[%s] from_account_id[%s] to[%s] to_account_id[%s] provider[%s] provider_type[%s] device_ip[%s] device_id[%s] post_script[%s] app_id[%s] binulevel[%s] dup_key[%s] sms_count[%s] sent_providers[%s] message[%s]\n", rBody.msgid, rBody.credit_id, rBody.from, rBody.from_account_id, rBody.to, rBody.to_account_id, rBody.provider, rBody.provider_type, rBody.device_ip, rBody.device_id, rBody.post_script, rBody.app_id, rBody.binulevel, rBody.dup_key, rBody.sms_count, rBody.sent_providers, rBody.message);
            
            //write log            
            current_time = time((time_t *)NULL);
            strftime(now_time, sizeof(now_time),"%Y-%m-%d %H:%M:%S",localtime(&current_time));                   
            snprintf(log_text, sizeof(log_text), "[%s] MsgID[%s] MsgType[%s] MsgCount[%s] CreditID[%s] From[%s] FromName[%s] FromAccountID[%s] To[%s] ToAccountID[%s] Provider[%s] ProviderType[%s] DeviceIP[%s] DeviceID[%s] AppID[%s] BinuLevel[%s] PostScript[%s] SentProvider[%s] Message[%s]",now_time, rBody.msgid, rBody.msg_type, rBody.sms_count, rBody.credit_id, rBody.from, rBody.from_name, rBody.from_account_id, rBody.to, rBody.to_account_id, rBody.provider, rBody.provider_type, rBody.device_ip, rBody.device_id, rBody.app_id, rBody.binulevel, rBody.post_script, rBody.sent_providers, rBody.message);
            
            strftime(now_time,25,"%Y%m%d",localtime(&current_time));
            if (atoi(rBody.provider_type) == 4){//resend
                snprintf(log_file, sizeof(log_file), "%s/resendSendsms_c_%s.log", SMS_LOG_PATH, now_time);
            }
            else{
                snprintf(log_file, sizeof(log_file), "%s/sendsms_c_%s.log", SMS_LOG_PATH, now_time);
            }
            
            mylog(&log_file, log_text);
            
            //run php script
            bzero (&newMessage, sizeof(newMessage));
            newMessage = addslashes(rBody.message);
            bzero(&newFname, sizeof(newFname));
            newFname = addslashes(rBody.from_name);

            bzero (&cmd, sizeof(cmd));
            snprintf(cmd,sizeof(cmd),"php %s -mid \"%s\" -mt \"%s\" -c \"%s\" -cid \"%s\" -f \"%s\" -fn \"%s\" -t \"%s\" -pro \"%s\" -pt \"%s\" -dip \"%s\" -did \"%s\" -fid \"%s\" -tid \"%s\" -aid \"%s\" -pst \"%s\" -stpro \"%s\" -msg \"%s\"", PHP_SMS_PROVIDER_PATH, rBody.msgid, rBody.msg_type, rBody.sms_count, rBody.credit_id, rBody.from, newFname, rBody.to, rBody.provider, rBody.provider_type, rBody.device_ip, rBody.device_id, rBody.from_account_id, rBody.to_account_id, rBody.app_id, rBody.post_script, rBody.sent_providers, newMessage);
            //fprintf(stdout, " \n%s\n\n\n", cmd);
            system(cmd);
            
        }
        if (record_count >= nRecords || record_count >= loopcount){
            record_count = 0;
            //fprintf(stdout, "\n");
            usleep (1000);
        }
    }    
}

/**
 * Main function.
 * @param argc the amount of command paramaters.
 * @param argv the array of command paramaters.
 * @return null 
 **/
int main(int argc, char** argv){
	int	n;	
	char action[10];
    
    //sms engine path
    if (strcmp(getenv("HOME"), HOME_PATH_LIVE) == 0){
        snprintf(SMS_ENGINE_PATH, sizeof(SMS_ENGINE_PATH), "%s", SMS_ENGINE_PATH_LIVE);
    }
    else{
        snprintf(SMS_ENGINE_PATH, sizeof(SMS_ENGINE_PATH), "%s", SMS_ENGINE_PATH_LOCAL);
    }
        
    snprintf(SMS_LOG_PATH, sizeof(SMS_LOG_PATH), "%s/log", SMS_ENGINE_PATH);
    snprintf(SMS_DATA_PATH, sizeof(SMS_DATA_PATH), "%s/data", SMS_ENGINE_PATH);
    snprintf(SENDSMS_NAME, sizeof(SENDSMS_NAME), "%s", argv[0]);
    snprintf(PHP_SMS_PROVIDER_PATH, sizeof(PHP_SMS_PROVIDER_PATH), "%s/sms_provider.php", SMS_ENGINE_PATH);
    
	if (argc < 2){
        Usage(argv[0]);
    }
    if (strcmp(argv[1],"help") == 0){
        Usage(argv[0]);
    }
    else if (strcmp(argv[1],"stop") == 0){
        service_stop(argv[0]);    
    }
    else if (strcmp(argv[1],"start") == 0){    
        if (argc < 4){
            Usage(argv[0]);
        }
        else{
            for (n = 1; n < argc; n++){
                if (strcmp(argv[n], "start") == 0){
                    snprintf(action, sizeof(action), "%s", argv[n]);
                }
                else if (strcmp(argv[n], "stop") == 0){
                    snprintf(action, sizeof(action), "%s", argv[n]);
                }
                else if (strcmp(argv[n], "-l") == 0){
                    n++;
                    nBinuLevel = atoi(argv[n]);    
                }
                else if (strcmp(argv[n], "-r") == 0){
                    n++;
                    nRecords = atoi(argv[n]);
                    if (nRecords < 1 || nRecords > 20){
                        fprintf(stderr, "<%s> Error: Records number should between 1 and 20", argv[0]);
                        exit(1);
                    }            
                }
                else if (strcmp(argv[n], "-n") == 0){
                    n++;
                    nProcess = atoi(argv[n]);
                    if (nProcess < 1 || nProcess > 50){
                        fprintf(stderr, "<%s> Error: Process number should between 1 and 50", argv[0]);
                        exit(1);
                    }
                }
                else{
                    Usage(argv[0]);
                }        
            }
        }
    }    
    else{
        Usage(argv[0]);
    }
    
    
    //check paramaters
    if (nBinuLevel == 0){
        fprintf(stderr, "<%s> Error: biNu level is empty (1 - 10).", argv[0]);
        exit(1);
    }
	
    //stop when get signal.
    signal (SIGINT, set_stop_flag);
    
	if (strcmp(action, "stop") == 0){
		service_stop(argv[0]);
	}
	else if (strcmp(action, "start") == 0){        
        //check cache file folder.    
        ProcessFolder(SMS_DATA_PATH, 0);
        ProcessFolder(SMS_LOG_PATH, 1);
    
        snprintf(cFileName, sizeof(cFileName), "%s/.SMS_REQUEST_QUEUE_%d.swp",SMS_DATA_PATH, nBinuLevel);
        //fprintf(stdout, "Starting Send SMS:  [  OK  ]\n");
		for (n = 1; n < nProcess; n++){
			if (fork() == 0){
				LoopOnInMsg(cFileName);
				exit(0);
			}
		}
        usleep (10000);
		LoopOnInMsg(cFileName);
        //fprintf(stdout, "Stopping Send SMS:  [  OK  ]\n");
	}
	else{
		Usage(argv[0]);
	}
	
	return 0;	
}



