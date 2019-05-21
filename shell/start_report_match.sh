#!/bin/sh

trap "" SIGTERM SIGSTOP SIGPIPE

live=`env | grep "HOME=/home" | awk -F '=' '{print $2}'`
if [ $live = '/home/sms' ]; then
    cd /home/sms/send_sms_v2
fi

if [ $live = '/home/johnson' ]; then
    cd /home/johnson/send_sms_v2
fi

#kill report_match.sh
num=`ps -ef|grep "./shell/report_match.sh" |grep -v "defunct" |grep -v grep |wc -l`
if [ $num -ge 1 ]; then    
    killall -9 report_match.sh > /dev/null
fi

#kill  sms_report_match.php
pid=`ps -ef | grep "php sms_report_match.php" |grep -v "defunct"|grep -v grep | awk -F ' ' '{print $2}'`
if [ -n "$pid" ]; then    
    kill -9 $pid > /dev/null   
fi

sleep 1

nohup ./shell/report_match.sh > /dev/null 2>&1 &




 


