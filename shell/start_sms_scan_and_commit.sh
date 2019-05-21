#!/bin/sh

trap "" SIGTERM SIGSTOP SIGPIPE

live=`env | grep "HOME=/home" | awk -F '=' '{print $2}'`
if [ $live = '/home/sms' ]; then
    cd /home/sms/send_sms_v2
fi

if [ $live = '/home/johnson' ]; then
    cd /home/johnson/send_sms_v2
fi

#kill sms_scan_and_commit.sh
num=`ps -ef|grep "./shell/sms_scan_and_commit.sh" |grep -v "defunct" |grep -v grep |wc -l`
if [ $num -ge 1 ]; then    
    killall -9 sms_scan_and_commit.sh > /dev/null
fi

#kill  sms_scan_and_commit.php
pid=`ps -ef | grep "php sms_scan_and_commit.php" |grep -v "defunct"|grep -v grep | awk -F ' ' '{print $2}'`
if [ -n "$pid" ]; then    
    kill -9 $pid > /dev/null   
fi

sleep 1

nohup ./shell/sms_scan_and_commit.sh > /dev/null 2>&1 &




 


