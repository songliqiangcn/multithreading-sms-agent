#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

#include <db.h>
#include <sys/types.h>


#include "mysock.h"
#include "struct.h"
#include "myfile.h"
#include "md5.h"

unsigned int	SV_PORT;
char SV_ADDR[64] = ""; //server bind IP address
char ALLOW_IP_LIST[1024] = ""; //ALLOW ip list
char SMS_DATA_PATH[512] = ""; //data path
char SMS_DUP_DB_PATH[512] = ""; //duplicate check DB path
char SMS_USR_NET_DB_PATH[512] = ""; //user phone number and network map DB path
char SMS_LOG_PATH[1024] = ""; //log path
char SMSAGENT_NAME[128] = ""; //engine command name
char SMS_ENGINE_PATH[512] = ""; //sms engine path

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //thread lock for accept the connection for socket.

pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER; //thread lock for cleaning up duplicate DB 
pthread_cond_t nodne_cond = PTHREAD_COND_INITIALIZER; //conditional-variable for cleaning up duplicate DB

DB_ENV *gbl_dbenv; //Berkeley DB environment 

long DupDbTimestamp = 0; //the last timestamp we did clean up duplicate DB.
int cleanup_db_flag = 0;//clean up duplicate DB. remove the items which is overtime.
int cStopFlag = 0; //Signal flag for stop. 1 stop 0 normal

/**
 * Help message.
 * @param sProgram command name
 * @return null 
 **/
void Usage(char* sProgram){
	fprintf(stdout, "Usage: %s start|stop|help [-n threads' number] [-a Allow IP list] [-t Hours to keep duplicate key DB]\n", sProgram);
	fprintf(stdout, "   start     Start this service.\n"); 
	fprintf(stdout, "   stop      Stop this service.\n"); 	
	fprintf(stdout, "   help      Print this help message.\n");  	
	fprintf(stdout, "   -n        How many threads to generate [ 1-30 ], default is 20\n");	
    fprintf(stdout, "   -a        Allowed ip list. e.g. 192.168.20.1,192.168.20.2. Keep empty for all.\n");    
    fprintf(stdout, "   -t        How many hours the duplicate key DB (Berkeley DB) willl be stored. Default is 1 week.\n");    
	exit(1);
}

/**
 * Initialize Berkeley DB environment
 * @param dbenv Berkeley DB environment handle
 * @param db_home DB data path
 * @return return the status of function 
 **/
int init_db_env(DB_ENV **dbenv, char* db_home){
    int ret;
    //char tmp_file[512], log_file[512];
    
    if ((ret = db_env_create(dbenv, 0)) != 0) {
        fprintf(stderr, "DB environment create error: %s\n", db_strerror(ret));
        return ret; 
    }
    
    //snprintf(tmp_file, sizeof(tmp_file), "%s/tmp", db_home);
    //snprintf(log_file, sizeof(log_file), "%s/log", db_home);
    
    (*dbenv)->set_errpfx(*dbenv, "SMS ENGINE");
    (*dbenv)->set_errfile(*dbenv, stderr);    
    //(*dbenv)->set_thread_count(*dbenv, 30);
    (*dbenv)->set_data_dir(*dbenv, db_home);
    //(*dbenv)->set_tmp_dir(*dbenv, tmp_file);
    //(*dbenv)->set_lg_dir(*dbenv, log_file);
    (*dbenv)->set_cachesize(*dbenv, 0, 64*1024, 1);
    (*dbenv)->set_alloc(*dbenv, malloc , realloc , free);
    //(*dbenv)->set_encrypt(*dbenv, PASSWORD, DB_ENCRYPT_AES);    
    
    if ((ret = (*dbenv)->open(*dbenv, db_home, DB_CREATE | DB_THREAD | DB_INIT_MPOOL | DB_INIT_CDB, 0)) != 0){
        (*dbenv)->close((*dbenv), 0);
        fprintf(stderr, "DB environment open error: %s\n", db_strerror(ret));
    }
    
    return ret;
}

/**
 * Initialize all global variables and folder.
 * @param cmd command name
 * @return null 
 **/
void initGblParams(const char* cmd){    
    int ret;
    
    //sms engine path
    if (strcmp(getenv("HOME"), HOME_PATH_LIVE) == 0){
        snprintf(SMS_ENGINE_PATH, sizeof(SMS_ENGINE_PATH), "%s", SMS_ENGINE_PATH_LIVE);
    }
    else{
        snprintf(SMS_ENGINE_PATH, sizeof(SMS_ENGINE_PATH), "%s", SMS_ENGINE_PATH_LOCAL);
    }
    
    //data path
    snprintf(SMS_DATA_PATH, sizeof(SMS_DATA_PATH), "%s/data", SMS_ENGINE_PATH);
    ProcessFolder(SMS_DATA_PATH, 0);
    
    //duplicate check DB path
    snprintf(SMS_DUP_DB_PATH, sizeof(SMS_DUP_DB_PATH), "%s/data/dup_db", SMS_ENGINE_PATH);
    RebuildDupDB(SMS_DUP_DB_PATH, 0);
    
    //user phonenumber and network map DB path
    snprintf(SMS_USR_NET_DB_PATH, sizeof(SMS_USR_NET_DB_PATH), "%s/data/user_network_db", SMS_ENGINE_PATH);
    RebuildDupDB(SMS_USR_NET_DB_PATH, 0);
    
    //log path
    snprintf(SMS_LOG_PATH, sizeof(SMS_LOG_PATH), "%s/log", SMS_ENGINE_PATH);
    ProcessFolder(SMS_LOG_PATH, 1);
                                             
    //command name
    snprintf(SMSAGENT_NAME, sizeof(SMSAGENT_NAME), "%s", cmd);
    
        
    //default
    snprintf(SV_ADDR, sizeof(SV_ADDR), "%s", SMSAGENT_BIND_IP);    
    SV_PORT = SMSAGENT_BIND_PORT;
    
    //create Berkeley DB env
    if ((ret = init_db_env(&gbl_dbenv, SMS_DUP_DB_PATH)) != 0){
        exit(1);
    };
    
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
    
    snprintf(cmd, sizeof(cmd), "killall -2 %s >/dev/null 2>&1", sProgram);
    system(cmd);
    sleep(1);
    
    snprintf(cmd, sizeof(cmd), "killall -9 %s >/dev/null 2>&1", sProgram);
    system(cmd);
    //printf("cmd %s \n",cmd);
    //exit(1);    
}

/**
 * Generate unique binu message ID.
 * @param biNuMsgID messageID
 * @param slen string length
 * @return null 
 **/
void getBinuUniMsgID(char* biNuMsgID, int slen){    
    struct timeval tms;
    struct timezone temp;
    char usec[7], id[20];
                    
    timerclear(&tms);
    gettimeofday(&tms, &temp );    
    memset(&id, 0, sizeof(id));
    memset(&usec, 0, sizeof(usec));
    
    snprintf(usec, sizeof(usec), "%06d", tms.tv_usec);
    snprintf(id, sizeof(id), "%010d", tms.tv_sec);    
    strcat(id, usec);
                                                              
    snprintf(biNuMsgID, slen, "%-s", id);    
}

/**
 * Check whether we need start the service to clean up duplicate DB, send signal to thread if we need.
 * @param tid thread id 
 * @return NULL
 **/

void clean_up_DB_check(unsigned int tid){
    long current_time;
                                                                                                
    pthread_mutex_lock(&db_mutex);
    //printf("[%u]----------DUPLICATE CLEAN UP CHECK----------\n", (unsigned int)tid);
    current_time = time((time_t *)NULL);
    if ((cleanup_db_flag == 0) && (current_time - DupDbTimestamp) > DUPLICATE_CHECK_TIME){                        
        DupDbTimestamp = current_time;
        cleanup_db_flag = 1;
        //send single
        pthread_cond_signal(&nodne_cond);
        pthread_cond_broadcast(&nodne_cond);
        printf("[%u]----------Send clean up signal to main thread \n", (unsigned int)tid);   
    }
    //printf("[%u]----------END DUPLICATE CLEAN UP CHECK----------\n\n", (unsigned int)tid);
    pthread_mutex_unlock(&db_mutex);
    
}

/**
 * Open the Berkeyley DB table
 * @param dbp DB handle
 * @param env DB environment
 * @param db_file_nm table name
 * @param db_nm
 * @return the status of function
 **/
int open_db(DB **dbp, DB_ENV *env, char *db_file_nm, char *db_nm){    
    int ret = -1;    
    
    if (db_file_nm == NULL){
        return ret;
    }
    
    
    //init_dead_file(rcrd_tm); 
    if ((ret = db_create(dbp, env, 0)) != 0){
        fprintf(stderr, "db_create error: %s \n", db_strerror(ret));
        return ret;
    }
    
    if ((ret = (*dbp)->set_flags(*dbp, DB_RECNUM)) != 0){//set records have line number.
        fprintf(stderr, "set_flags error: %s \n", db_strerror(ret));
        return ret;
    }
    
    if ((ret = (*dbp)->open(*dbp, NULL, db_file_nm, db_nm, DB_BTREE, DB_CREATE | DB_THREAD, 0664)) != 0){
        fprintf(stderr, "DB open failed(%d): %s \n", ret, db_strerror(ret));
        return ret;
    }
    
    return ret;
}


/**
 * Main logic function.
 * @param sd socket handle
 * @param clientIP remote client IP address
 * @param dbp BerkeleyDB handle
 * @param msgid unique binu message ID
 * @return null 
 **/
void ProcessRequest(int sd, const char* clientIP, DB **dbp, const char* msgid){    
    RequestBody rBody, nBody;
    SMSResponseDS smsrepds;
    char log_text[1500], now_time[25], log_file[250], queue_file[512], md5code[35], tmpCode[3];    
    long current_time, dupkey_val, current_time2;
    int binu_level = 0;
    int resend = 0;    
    pthread_t tid;    
    unsigned char dup_key[16];
    
    DBT tKey, tData;    
    int ret;
    
    tid = pthread_self();
    
    memset((char*)&rBody, 0, sizeof(RequestBody));
    memset((char*)&log_text, 0, sizeof(log_text));
    memset((char*)&now_time, 0, sizeof(now_time));
    memset((char*)&log_file, 0, sizeof(log_file));
    memset((char*)&queue_file, 0, sizeof(queue_file));
    memset((char*)&dup_key, 0, sizeof(dup_key));
    
    
    if(safe_recv(sd, (char*)&rBody, sizeof(RequestBody), 10) == sizeof(RequestBody)){
        //check the stop flag 
        //if (cStopFlag != 0){
        //    fprintf(stderr,"Received the stop signal, close socket.\n");
        //    break;
        //}
        
        bzero (&smsrepds, sizeof(SMSResponseDS));
        current_time = time((time_t *)NULL);
        
        memset((char*)&nBody, 0, sizeof(RequestBody));    
        memcpy((char*)&nBody,&rBody,sizeof(RequestBody));    
        
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
        
        
        //duplication message check
        //printf("\n\n[%u]----------NEW REQUEST----------\n", (unsigned int)tid);
        if (strlen(rBody.dup_key) > 0){//we need do duplicate check.            
            memset(&tKey, 0, sizeof(tKey));
            memset(&tData, 0, sizeof(tData));
            
            tKey.data = (char *)rBody.dup_key;
            tKey.size = sizeof(rBody.dup_key);
            
            //If Berkeley DB needs to return a value and DB_THREAD is set, the Dbt must have an allocation policy for management of the returned value.
            tKey.flags = DB_DBT_USERMEM;            
            tData.flags = DB_DBT_MALLOC;
            
            //search by duplicate key
            if ((ret = (*dbp)->get(*dbp, NULL, &tKey, &tData, 0)) != 0){
                //tData.data is malloc memory by bdb, so we need to free it .
                free(tData.data);
                
                if (ret == DB_NOTFOUND){//not found                    
                    memset(&tData, 0, sizeof(tData));                    
                    tData.data = &current_time;
                    tData.size = sizeof(long);
                    tData.flags = DB_DBT_USERMEM;
                                        
                    if ((ret = (*dbp)->put(*dbp, NULL, &tKey, &tData, 0)) != 0){//add new one
                        fprintf(stderr, "Insert new duplicate key error: %s\n", db_strerror(ret));
                    }
                    else{
                        //fprintf(stdout, "[%u] Success to add new duplicate key[%s] with data [%d]\n", (unsigned int)tid, (char*)tKey.data, *(long*)tData.data);
                    }
                }
                else{
                    fprintf(stderr, "Search item by duplicate key error: %s\n", db_strerror(ret));
                }
            }
            else{//found
                //fprintf(stderr, "found key [%s] with value[%d] and check it value now\n", (char*)tKey.data, *(long*)tData.data);
                
                dupkey_val = *(long*)tData.data;
                //tData.data is malloc memory by bdb, so we need to free it .
                free(tData.data);
                
                //do we need to update it?
                if ((current_time - dupkey_val) <= DUPLICATE_TIME){//can not send SMS
                    //fprintf(stdout, "[%u] duplicate item, within the check period, can not send SMS, now[%d] val[%d], sub[%d]\n", (unsigned int)tid, current_time, dupkey_val, (current_time-dupkey_val));
                    
                
                    //send response to socket handle
                    snprintf(smsrepds.Reason, sizeof(smsrepds.Reason), "%-50s", "3:duplicate data:MSGID:0\n");
                    ret = safe_send(sd, (char*)&smsrepds, sizeof(SMSResponseDS), 2);
                    //printf("[%u]----------END NEW REQUEST----------\n\n", (unsigned int)tid);
                    //continue;
                    
		    //write log
		    strftime(now_time, sizeof(now_time),"%Y-%m-%d %H:%M:%S",localtime(&current_time));
		    snprintf(log_text, sizeof(log_text), "[%s] SMSAGENT[pthread_id:%u] ClientIP[%s] MsgID[%s] MsgType[%s] MsgCount[%s] CreditID[%s] From[%s] FromName[%s] FromAccountID[%s] To[%s] ToAccountID[%s] GivenProvider[%s] ProviderType[%s] DeviceIP[%s] DeviceID[%s] AppID[%s] BinuLevel[%s] PostScript[%s] SentProvider[%s] Error[Duplicate Data] Message[%s]",now_time, (unsigned int)tid, clientIP, rBody.msgid, rBody.msg_type, rBody.sms_count, rBody.credit_id, rBody.from, rBody.from_name, rBody.from_account_id, rBody.to, rBody.to_account_id, rBody.provider, rBody.provider_type, rBody.device_ip, rBody.device_id, rBody.app_id, rBody.binulevel, rBody.post_script, rBody.sent_providers, rBody.message);
		    strftime(now_time,25,"%Y%m%d",localtime(&current_time));
		    snprintf(log_file, sizeof(log_file), "%s/rejectSmsagent_%s.log", SMS_LOG_PATH, now_time);
		    mylog(&log_file, log_text);
		    
		    return;
                }
                else{//can send SMS and update the timestamp with current one.
                    memset(&tData, 0, sizeof(tData));                    
                    tData.data = &current_time;
                    tData.size = sizeof(long);
                    tData.flags = DB_DBT_USERMEM;
                    if ((ret = (*dbp)->put(*dbp, NULL, &tKey, &tData, 0)) != 0){
                        fprintf(stderr, "Update duplicate key value to currect timestamp error: %s\n", db_strerror(ret));
                    }
                    else{
                        //fprintf(stdout, "[%u] Success to update duplicate key with value %d\n", (unsigned int)tid, *(long*)tData.data);
                    }                    
                }
            }            
            current_time2 = time((time_t *)NULL);
            //printf("[%u]----------Key[%s] TakesSeconds[%ld] ----------\n", (unsigned int)tid, rBody.dup_key, (current_time2 - current_time));
        }
        
        
        
        //update msgid and update to selected provider
        resend = 0;
        if (strlen(rBody.msgid) == 0){//new one
            //fprintf(stderr, "msgid is null, generate new one [%u]\n", (unsigned int)tid);
            memset((char*)&nBody.msgid, 0, sizeof(nBody.msgid));
            snprintf(nBody.msgid, sizeof(nBody.msgid), "%s", msgid);
            
            memset((char*)&rBody.msgid, 0, sizeof(rBody.msgid));
            snprintf(rBody.msgid, sizeof(rBody.msgid), "%s", msgid);
            
            //selecte a provider we want by user's network then set field sent_provider's value to 0
            //testing
            if (strlen(rBody.provider) == 0){//not hard code provider
                //testing
                //here we can make some rules to change the providers by some conditions.
                /*
                if (strcmp(rBody.to, "61428032601") == 0){
                    snprintf(nBody.provider, sizeof(nBody.provider), "clickatell");
                    snprintf(rBody.provider, sizeof(rBody.provider), "clickatell");
                
                    snprintf(nBody.provider_type, sizeof(nBody.provider_type), "3");
                    snprintf(rBody.provider_type, sizeof(rBody.provider_type), "3");
                }
                */
            }
        }
        else{//resend
            resend = 1;
            //fprintf(stdout, "Resend message with msgid [%s] sent_provider[%s] level[%s]\n", rBody.msgid, rBody.sent_providers, rBody.binulevel);
        }
        
        
        //write data into cache file
        binu_level = atoi(rBody.binulevel);        
        snprintf(queue_file, sizeof(queue_file), "%s/.SMS_REQUEST_QUEUE_%d.swp",SMS_DATA_PATH, binu_level);        
        fifowrite(queue_file, (char*)&nBody, sizeof(RequestBody));
        
        //send response to socket        
        snprintf(smsrepds.Reason, sizeof(smsrepds.Reason), "1:Success:MSGID:%-s%s", rBody.msgid, "\n");
        safe_send(sd, (char*)&smsrepds, sizeof(SMSResponseDS), 2);        
        
        //write log        
        strftime(now_time, sizeof(now_time),"%Y-%m-%d %H:%M:%S",localtime(&current_time));        
        snprintf(log_text, sizeof(log_text), "[%s] SMSAGENT[pthread_id:%u] ClientIP[%s] MsgID[%s] MsgType[%s] MsgCount[%s] CreditID[%s] From[%s] FromName[%s] FromAccountID[%s] To[%s] ToAccountID[%s] GivenProvider[%s] ProviderType[%s] DeviceIP[%s] DeviceID[%s] AppID[%s] BinuLevel[%s] PostScript[%s] SentProvider[%s] Message[%s]",now_time, (unsigned int)tid, clientIP, rBody.msgid, rBody.msg_type, rBody.sms_count, rBody.credit_id, rBody.from, rBody.from_name, rBody.from_account_id, rBody.to, rBody.to_account_id, rBody.provider, rBody.provider_type, rBody.device_ip, rBody.device_id, rBody.app_id, rBody.binulevel, rBody.post_script, rBody.sent_providers, rBody.message);
        strftime(now_time,25,"%Y%m%d",localtime(&current_time));        
        if (resend == 1){
            snprintf(log_file, sizeof(log_file), "%s/resendSmsagent_%s.log", SMS_LOG_PATH, now_time);    
        }
        else{
            snprintf(log_file, sizeof(log_file), "%s/smsagent_%s.log", SMS_LOG_PATH, now_time);
        }                
        mylog(&log_file, log_text);
               
        
        //print out 
        //fprintf(stdout, "msgid[%s] msgtype[%s] credit_id[%s] from[%s] from_name[%s] from_account_id[%s] to[%s] to_account_id[%s] provider[%s] device_ip[%s] device_id[%s] post_script[%s] app_id[%s] binulevel[%s] dup_key[%s] sms_count[%s] provider_type[%s] sent_providers[%s] message[%s]\n", rBody.msgid, rBody.msg_type, rBody.credit_id, rBody.from, rBody.from_name, rBody.from_account_id, rBody.to, rBody.to_account_id, rBody.provider, rBody.device_ip, rBody.device_id, rBody.post_script, rBody.app_id, rBody.binulevel, rBody.dup_key, rBody.sms_count, rBody.provider_type, rBody.sent_providers, rBody.message);
        
        //printf("[%u]----------END NEW REQUEST----------\n\n", (unsigned int)tid);
    }
   
}

/**
 * Individual thread process. Handle the network communication things.
 * @param thread_arg threadID 
 * @return null 
 **/
void* ServiceThread(int* thread_arg){
    int l_socket = *thread_arg;
    int srv_socket;
    struct  sockaddr_in from;
    int fromlen = sizeof(from);
    char clientIP[18];
    int securityCheck = 0;
    SMSResponseDS smsrepds;
    long LocalDupDbTimestamp = 0;
    pthread_t tid;
    char msgid[20];
    
    tid = pthread_self();
    
    //process BerkeleyDB
    
    DB *dbp;
    char BerkeleyDB_Table[250];
    int ret;
    u_int32_t countp;
    
    
    memset((char*)&BerkeleyDB_Table, 0, sizeof(BerkeleyDB_Table));
    snprintf(BerkeleyDB_Table, sizeof(BerkeleyDB_Table), "%s/SMS_DUP_DB.db",SMS_DUP_DB_PATH);
        
    //open duplicate DB
    while((ret = open_db(&dbp, gbl_dbenv, BerkeleyDB_Table, NULL)) != 0){
        fprintf(stderr, "Failed to open duplicate DB handle, error: %s\n", db_strerror(ret));
        sleep(1);
    }
    
    
    while (1){
        //check the stop flag 
        if (cStopFlag != 0){
            fprintf(stderr,"[%u] Received the stop signal.\n", (unsigned int)tid);
            break;
        }
                
        // enter critical space
        pthread_mutex_lock(&mutex);                        
        srv_socket = accept(l_socket, (struct sockaddr*)&from, (socklen_t *)&fromlen);
        //printf("[%u]----------receive new request from socket----------\n",(unsigned int)tid);
        //get client IP
        bzero (&clientIP, sizeof(clientIP));
        snprintf(clientIP, sizeof(clientIP), "%s", inet_ntoa(from.sin_addr));        
        
        //get unique msgid
        bzero (&msgid, sizeof(msgid));        
        getBinuUniMsgID(msgid, sizeof(msgid));
                
        // leave criticla space
        pthread_mutex_unlock(&mutex);
                                                                
        if (srv_socket == -1) break; //accept error, exit thread
                
        securityCheck = SecurityCheck(ALLOW_IP_LIST, clientIP);
        bzero (&smsrepds, sizeof(SMSResponseDS));
        
        if (securityCheck == 1){//security check success            
            snprintf(smsrepds.Reason, sizeof(smsrepds.Reason), "%-50s", "1:Success:MSGID:0\n");
            safe_send(srv_socket, (char*)&smsrepds, sizeof(SMSResponseDS), 2);        
           
            //start the service
            ProcessRequest(srv_socket, clientIP, &dbp, msgid);
        }
        else{//security check fail        
            snprintf(smsrepds.Reason, sizeof(smsrepds.Reason), "%-50s", "2:Ip access deny:MSGID:0\n");
            safe_send(srv_socket, (char*)&smsrepds, sizeof(SMSResponseDS), 2);        
        }
        
        //close the socket
        shutdown(srv_socket, 2);
        close(srv_socket);
        
        //do we need start to clean up duplicate DB?
        //(more info refer to function clean_up_dup_db())
        //solution 1:
        //Yes, we can send signal to the main thread to clean up duplicate DB data for every 24 hours.
        //But we have to use a lock for every thread. I worry about it will take more time to handle it.
        //My solution is using function pthread_cond_timedwait() in main thread for itself, set a timestamp for the timeout if no signal received. So same like a sleep() will wake up automatically by itself. So only main thread need to think about the cleaning up DB things. Other threads will focus on the business things.
        
        //solution 2:        
        //clean_up_DB_check(tid);        
    }
    
    //close BerkeleyDB
    if (dbp != NULL){
        if ((ret = dbp->close(dbp, 0)) != 0){
            fprintf(stderr, "Close DB handle failed: %s\n", db_strerror(ret));
        }
    }
}

void clean_up_dup_data(){
    DB *dbp;
    DBC *cur;
    char BerkeleyDB_Table[250], now_time[25], now_time2[25], log_text[1500], log_file[250];
    DBT tKey, tData,tKeydel;    
    int ret, tal_index, del_index;
    pthread_t tid;
    long dup_val, dup_past, current_time, current_time2;
    
    memset((char*)&log_text, 0, sizeof(log_text));
    memset((char*)&log_file, 0, sizeof(log_file));
    memset((char*)&now_time, 0, sizeof(now_time));
    memset((char*)&now_time2, 0, sizeof(now_time2));
    memset((char*)&BerkeleyDB_Table, 0, sizeof(BerkeleyDB_Table));
    
    current_time = time((time_t *)NULL);
    strftime(now_time, sizeof(now_time),"%Y-%m-%d %H:%M:%S",localtime(&current_time));
    
    tid = pthread_self();
    snprintf(BerkeleyDB_Table, sizeof(BerkeleyDB_Table), "%s/SMS_DUP_DB.db",SMS_DUP_DB_PATH);
    //printf("[%u]-----------Open DB [%ld]----------\n", (unsigned int)tid, current_time);
    while((ret = open_db(&dbp, gbl_dbenv, BerkeleyDB_Table, NULL)) != 0){
        fprintf(stderr, "Failed to open duplicate DB handle to clean up DB, error: %s\n", db_strerror(ret));
        sleep(1);
    }
    current_time = time((time_t *)NULL);
    //printf("[%u]-----------1.Start Clean Up DB [%ld]----------\n", (unsigned int)tid, current_time);
    if ((ret = dbp->cursor(dbp, NULL, &cur, DB_WRITECURSOR)) != 0){
        fprintf(stderr, "Failed to pen cursor to read DB %s\n", db_strerror(ret));
        return;
    }
    current_time = time((time_t *)NULL);
    //printf("[%u]-----------2. Start Clean Up DB [%ld]----------\n", (unsigned int)tid, current_time);
    memset(&tKey, 0, sizeof(tKey));
    memset(&tData, 0, sizeof(tData));
    tKey.flags = DB_DBT_MALLOC;
    tData.flags = DB_DBT_MALLOC;
            
    tal_index = 0;
    del_index = 0;
    
    while((ret = cur->c_get(cur, &tKey, &tData, DB_NEXT)) == 0){
        tal_index++;
        dup_val = *(long*)tData.data;
        dup_past = current_time - dup_val; 
        //printf("[%d]The KEY = %s,\t The val = %d [%d]\n", index, (char *)(tKey.data), dup_val, dup_past);        
                
        if (dup_past > DUPLICATE_TIME){
            del_index++;
            //memset(&tKeydel, 0, sizeof(tKeydel));
                    
            //tKeydel.data = (char*)tKey.data;
            //tKeydel.size = sizeof(tKey.data);
            //tKeydel.flags = DB_DBT_USERMEM;
            ret = cur->c_del(cur, 0);            
            //printf(">>>Delete item with key[%s]\n", (char*)tKey.data );
        }
        free(tData.data);
        free(tKey.data);
    }
    ret = cur->c_close(cur);    
    
    current_time2 = time((time_t *)NULL);
    dup_past = (current_time2 - current_time);
    strftime(now_time2, sizeof(now_time2),"%Y-%m-%d %H:%M:%S",localtime(&current_time2));
    //printf("[%u]----------Total [%d] Del[%d] Active[%d] StartTime[%s] Endtime[%s] TakeSeconds[%d]----------\n", (unsigned int)tid, tal_index, del_index, (tal_index - del_index), now_time, now_time2, dup_past);
    
    snprintf(log_text, sizeof(log_text), "[%s] DuplicateDB_CleanUp Total[%d] Del[%d] Active[%d] EndTime[%s] TakeSeconds[%d]",now_time, tal_index, del_index, (tal_index - del_index), now_time2, dup_past);        
    strftime(now_time,25,"%Y%m%d",localtime(&current_time2));        
    snprintf(log_file, sizeof(log_file), "%s/duplicate_cleanup_%s.log", SMS_LOG_PATH, now_time);
    mylog(&log_file, log_text);
    
    
    //close BerkeleyDB
    if (dbp != NULL){
        if ((ret = dbp->close(dbp, 0)) != 0){
            fprintf(stderr, "Close DB handle failed, error: %s\n", db_strerror(ret));
        }
    }
    
}



void clean_up_dup_db(){    
    char BerkeleyDB_Table[250];
    int ret,tal_index,del_index;    
    u_int32_t countp;
    pthread_t tid;    
    struct timeval tv;
    struct timespec ts;
    
    tid = pthread_self();
    
    while(1){
        //check the stop flag 
        if (cStopFlag != 0){
            fprintf(stderr,"Received the stop signal\n");
            break;
        }
        
        //soluction 1:
        //Yes, we can send signal to the main thread to clean up duplicate DB data for every 24 hours.
        //But we have to use a lock for every thread. I worry about it will take more time to handle it.
        //My solution is using function pthread_cond_timedwait() in main thread for itself, set a timestamp for the timeout if no signal received. 
        //So it is same like a sleep() will wake up automatically by itself. 
        //So only main thread need to think about the cleaning up DB things. Other threads will focus on the business things.         
        if (gettimeofday(&tv, NULL) < 0){
            fprintf(stderr, "gettimeofday error\n");  
        } 
        ts.tv_sec = tv.tv_sec + DUPLICATE_CHECK_TIME;
        ts.tv_nsec = tv.tv_usec * 1000;        
        
        pthread_mutex_lock(&db_mutex);
        pthread_cond_timedwait(&nodne_cond,&db_mutex,&ts); //will timeout automatically after DUPLICATE_CHECK_TIME seconds.
        //printf("\n\n[%u]----------RECEIVE CLEAN UP SIGNAL[%d]----------\n", (unsigned int)tid, cleanup_db_flag);
        pthread_mutex_unlock(&db_mutex);
        
        clean_up_dup_data();
        
        
        //solution 2:
        //useing condition signal waitting for the signal sent from other threads.
        //shortcoming: every thread will check the timestamp using thread lock. 
        /*
        pthread_mutex_lock(&db_mutex);
        while (cleanup_db_flag == 0){
            pthread_cond_wait(&nodne_cond,&db_mutex);
            break;
        }
        pthread_mutex_unlock(&db_mutex);
        if (cleanup_db_flag != 0){
            clean_up_dup_data();
            
            pthread_mutex_lock(&db_mutex);
            cleanup_db_flag = 0;
            pthread_mutex_unlock(&db_mutex);
        }
        */        
    }
}


/**
 * Main function.
 * @param argc the amount of command paramaters.
 * @param argv the array of command paramaters.
 * @return null 
 **/
int main(int argc, char** argv){
	char action[10];
	int	l_socket = 0;
	int loops = 0;
	int	succ_count = 0;
	int	n = 0, ret;
    int thread_num = 20;//by default generate 20 thread
	
    
	if ((argc < 2)||(strcmp(argv[1],"help")==0))
		Usage(argv[0]);
	
    
	
	//init paramaters.
	for (n = 1; n < argc; n++){
		if (strcmp(argv[n], "start") == 0){
			snprintf(action, 10, "%s", argv[n]);
		}
		else if (strcmp(argv[n], "stop") == 0){
			snprintf(action, 10, "%s", argv[n]);
		}
		else if (strcmp(argv[n], "-n") == 0){
			n++;
			thread_num = atoi(argv[n]);
			
			if (thread_num <= 0 || thread_num > MAX_THREAD_NUM){
				fprintf(stderr, "ERR:Invalid threads' number, should be between 1 and 30\n");
				exit(1);
			}
		}
		else if (strcmp(argv[n], "-a") == 0){
			n++;
            
            if (strlen(argv[n]) > sizeof(ALLOW_IP_LIST)){
                fprintf(stderr, "ERR:The length of string allow IP address over the max length of system defined. \n");
                exit(1);
            }
            
			snprintf(ALLOW_IP_LIST, 1024, "%s", argv[n]);
		}
        else if (strcmp(argv[n], "-t") == 0){
            n++;
            DUPLICATE_TIME = atoi(argv[n]) * 60 * 60;
        }	
		else{
			Usage(argv[0]);
		}		
	}
    //testing
	//DUPLICATE_TIME = 60; //keep dup for 7 seconds
    //DUPLICATE_CHECK_TIME = 5; //every 5 seconds to check.
   //printf("adter [%ld]\n", DUPLICATE_TIME / 3600);
    
    //stop when get signal.
    signal (SIGINT, set_stop_flag);
        
	if (strcmp(action, "stop") == 0){
		service_stop(argv[0]);
	}
	else if (strcmp(action, "start") == 0){
        //init everhting.
        initGblParams(argv[0]);
    
		//init thread
		pthread_t tid_arr[MAX_THREAD_NUM];
		if (pthread_mutex_init(&mutex, NULL) != 0){
			fprintf(stderr, "ERR:Failed init thread mutex.\n");
			exit(1);
		}
        
        if (pthread_mutex_init(&db_mutex, NULL) != 0){
            fprintf(stderr, "ERR:Failed init thread db_mutex.\n");
            exit(1);
        }
        
		//bind server
		if ((l_socket = listen_socket(SV_ADDR, SV_PORT)) == -1){
			pthread_mutex_destroy(&mutex);
            
			fprintf(stderr, "ERR:Failed start service.\n");
			exit(1);
		}
		//create thread to send SMS
		for (loops = 0; loops < thread_num; loops++){			
			if (pthread_create(&tid_arr[loops], (pthread_attr_t *)PTHREAD_CREATE_JOINABLE, (void *)ServiceThread, (void*)&l_socket) == 0)
				succ_count++;
		}
		//fprintf(stdout, "Starting SMS Agent:  [  OK  ]\n");
        
        //waitting for cleaning up duplicate DB for items which is overtime for one week
        sleep(2);
        clean_up_dup_db();
        
        //clect thread
		for (loops = 0; loops < MAX_THREAD_NUM; loops++)
			pthread_join(tid_arr[loops], NULL);
			
		shutdown(l_socket, 2);
		close(l_socket);
		pthread_mutex_destroy(&mutex);
        pthread_mutex_destroy(&db_mutex);
        
        //close Berkeley DB environment
        if (gbl_dbenv != NULL){
            if ((ret = gbl_dbenv->close(gbl_dbenv, 0)) != 0){
                fprintf(stderr, "Environment close failed: %s\n", db_strerror(ret));                
            }
        }
        
	}
	else{
		Usage(argv[0]);
	}
	
	return 0;	
	
}








