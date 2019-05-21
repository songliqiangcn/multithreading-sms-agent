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
  num=`ps -ef|grep "messenger_cleanup.php" |grep -v "defunct" |grep -v grep |wc -l`  
  if [ $num -eq 0 ]; then    
    echo "[`date`] biNu Messenger Clean Up START" >>  ./log/start_messenger_cleanup.log
    nohup php messenger_cleanup.php >> ./log/error_messenger_cleanup.log 2>&1
  fi
    sleep 10
done

