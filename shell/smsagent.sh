#!/bin/sh

trap "" SIGTERM SIGSTOP SIGPIPE

arg1="$1"

live=`env | grep "HOME=/home" | awk -F '=' '{print $2}'`

if [ $live = '/home/sms' ]; then
    cd /home/sms/send_sms_v2
fi

if [ $live = '/home/johnson' ]; then
    cd /home/johnson/send_sms_v2
fi

# remove Berkeley DB data
rm -rf ./dup_db
mkdir dup_db

while(true)
do
	num=`ps -ef|grep "SMSAGENT_V2"|grep -v "defunct"|grep -v grep|wc -l`
	if [ $num -lt 1 ]; then		
		echo "[`date`] SMSAGENT START" >>  ./log/start_smsagent.log		
		nohup ./SMSAGENT_V2 $arg1   >> ./log/error_smsagent.log 2>&1
	fi
	  sleep 10
done


