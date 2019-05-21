#!/bin/sh

trap "" SIGTERM SIGSTOP SIGPIPE

live=`env | grep "HOME=/home" | awk -F '=' '{print $2}'`
if [ $live = '/home/sms' ]; then
    cd /home/sms/send_sms_v2
fi

if [ $live = '/home/johnson' ]; then
    cd /home/johnson/send_sms_v2
fi

#kill group_email.sh
num=`ps -ef|grep "./shell/group_email.sh" |grep -v "defunct" |grep -v grep |wc -l`
if [ $num -ge 1 ]; then    
    killall -9 group_email.sh > /dev/null
fi

#kill  send_group_email.php
num=`ps -ef | grep "groupEmail" | grep -v grep | wc -l`
if [ $num -ge 1 ]; then
    killall -9 groupEmail > /dev/null
fi


#pid=`ps -ef | grep "groupEmail" |grep -v "defunct"|grep -v grep | awk -F ' ' '{print $2}'`
#if [ -n "$pid" ]; then    
#    kill -9 $pid > /dev/null   
#fi

sleep 1

nohup ./shell/group_email.sh > /dev/null 2>&1 &




 


