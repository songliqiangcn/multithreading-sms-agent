#!/bin/sh

trap "" SIGTERM SIGSTOP SIGPIPE

live=`env | grep "HOME=/home" | awk -F '=' '{print $2}'`
if [ $live = '/home/sms' ]; then
    cd /home/sms/send_sms_v2
fi

if [ $live = '/home/johnson' ]; then
    cd /home/johnson/send_sms_v2
fi

#kill messenger_cleanup.sh
num=`ps -ef|grep "./shell/messenger_cleanup.sh" |grep -v "defunct" |grep -v grep |wc -l`
if [ $num -ge 1 ]; then    
    killall -9 messenger_cleanup.sh > /dev/null
fi

#kill  messenger_cleanup.php
pid=`ps -ef | grep "php messenger_cleanup.php" |grep -v "defunct"|grep -v grep | awk -F ' ' '{print $2}'`
if [ -n "$pid" ]; then    
    kill -9 $pid > /dev/null   
fi

sleep 1

nohup ./shell/messenger_cleanup.sh > /dev/null 2>&1 &




 


