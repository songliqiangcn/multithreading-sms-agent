#!/bin/sh

trap "" SIGTERM SIGSTOP SIGPIPE

live=`env | grep "HOME=/home" | awk -F '=' '{print $2}'`
if [ $live = '/home/sms' ]; then
    cd /home/sms/send_sms_v2
fi

if [ $live = '/home/johnson' ]; then
    cd /home/johnson/send_sms_v2
fi


while(true)
do
  num=`ps -ef|grep "groupEmail" |grep -v "defunct" |grep -v grep |wc -l`  
  if [ $num -eq 0 ]; then    
    echo "[`date`] GROUP EMAIL START" >>  ./log/start_group_email.log
    ./groupEmail start >> ./log/error_group_email.log 2>&1
  fi
    sleep 10
done

