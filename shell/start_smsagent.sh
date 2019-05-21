#!/bin/sh

trap "" SIGTERM SIGSTOP SIGPIPE

live=`env | grep "HOME=/home" | awk -F '=' '{print $2}'`
if [ $live = '/home/sms' ]; then
    cd /home/sms/send_sms_v2
fi

if [ $live = '/home/johnson' ]; then
    cd /home/johnson/send_sms_v2
fi

#kill smsagent.sh
num=`ps -ef|grep "./shell/smsagent.sh" |grep -v "defunct" |grep -v grep |wc -l`
if [ $num -ge 1 ]; then    
    killall -9 smsagent.sh > /dev/null 2>&1    
fi

#kill SMSAGENT
num=`ps -ef|grep "SMSAGENT_V2"|grep -v "defunct"|grep -v grep|wc -l`
if [ $num -ge 1 ]; then
    ./SMSAGENT_V2 stop > /dev/null       
fi

#Kill again if failed to stop


pid=`ps -ef | grep "SMSAGENT_V2" |grep -v "defunct"|grep -v grep | awk -F ' ' '{print $2}'`
if [ -n "$pid" ]; then
    kill -9 $pid > /dev/null
fi

killall -9 SMSAGENT_V2

sleep 1
nohup ./shell/smsagent.sh "start -n 15"> /dev/null 2>&1 &




