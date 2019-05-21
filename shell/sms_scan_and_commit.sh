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
  num=`ps -ef|grep "sms_scan_and_commit.php" |grep -v "defunct" |grep -v grep |wc -l`  
  if [ $num -eq 0 ]; then    
    echo "[`date`] SMS SCAN AND COMMIT START" >>  ./log/start_smsScanAndCommit.log
    nohup php sms_scan_and_commit.php >> ./log/error_smsScanAndCommit.log 2>&1
  fi
    sleep 10
done

