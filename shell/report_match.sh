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
  num=`ps -ef|grep "sms_report_match.php" |grep -v "defunct" |grep -v grep |wc -l`  
  if [ $num -eq 0 ]; then    
    echo "[`date`] SMS REPORT MATCH START" >>  ./log/start_smsmatch.log
    nohup php sms_report_match.php >> ./log/error_smsmatch.log 2>&1
  fi
    sleep 10
done

