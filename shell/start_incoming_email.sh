#!/bin/sh

trap "" SIGTERM SIGSTOP SIGPIPE

live=`env | grep "HOME=/home" | awk -F '=' '{print $2}'`
if [ $live = '/home/sms' ]; then
    cd /home/sms/send_sms_v2
fi

if [ $live = '/home/johnson' ]; then
    cd /home/johnson/send_sms_v2
fi

#kill incoming_email.sh
num=`ps -ef|grep "./shell/incoming_email.sh" |grep -v "defunct" |grep -v grep |wc -l`
if [ $num -ge 1 ]; then    
    killall -9 incoming_email.sh > /dev/null
fi

#kill  incoming_email.php
pid=`ps -ef | grep "php incoming_email.php" |grep -v "defunct"|grep -v grep | awk -F ' ' '{print $2}'`
if [ -n "$pid" ]; then    
    kill -9 $pid > /dev/null   
fi

sleep 1

nohup ./shell/incoming_email.sh > /dev/null 2>&1 &




 


