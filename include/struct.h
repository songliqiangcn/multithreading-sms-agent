//struct_misc.h
#include <time.h>

#ifndef __STRUCT_H__
#define __STRUCT_H__

char* SMSAGENT_BIND_IP = "127.0.0.1";
unsigned int SMSAGENT_BIND_PORT = 7001;

char* HOME_PATH_LIVE = "/home/sms";
char* SMS_ENGINE_PATH_LIVE = "/home/sms/send_sms_v2";
char* SMS_ENGINE_PATH_LOCAL = "/home/johnson/send_sms_v2";

//long DUPLICATE_TIME = 60 * 60 * 24 * 7; //Duplicate data db (Berkeley DB) will can be stored for one week.
long DUPLICATE_TIME = 60 * 5; //Duplicate data db (Berkeley DB) will can be stored for 5 minutes.
//long DUPLICATE_CHECK_TIME =  60 * 60 * 1; //Every hour will do the duplicate clean up check.
long DUPLICATE_CHECK_TIME =  60 * 2; //Do the duplicate clean up check for every 2 minutes.
const int MAX_THREAD_NUM = 100; //Max thread number.

char* VERSION = "2.0";

//message body structure       
typedef struct _RequestBody{
	char msglen[10];//It will tell us how many bytes for the package data this time.		
	char msgid[20];
    char credit_id[20];//credit engine transaction ID. we will need transfer it to credit engine after we received the status report
    char from[20];
    char from_name[20];
	char from_account_id[20];
    char to[20];
    char to_account_id[20];
	char provider[20];
	char device_ip[20];
	char device_id[20];	
	char post_script[20];
    char app_id[10];	
    char binulevel[5];//Every SMS has different level. the priority from high to low is 9 - 1
    char dup_key[35];
    char msg_type[20];//'verification', 'notification' .....
    char sms_count[2];
    char provider_type[2];//1: normal 2: hard code provider 3: selected provider 4: resend
    char message[500];
    char sent_providers[200];//Saved the providers we tried already, e.g. 1,2. If 0, means field provider is a selected provider by our logic and need try other providers if failed to delivery message.
    char reserved[460]; //Reserved for future.
}RequestBody;

typedef struct _SMSResponseDS
{
	//char	ErrCode;
	char	Reason[50];
} SMSResponseDS;


#endif

