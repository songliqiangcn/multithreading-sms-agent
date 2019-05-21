#!/bin/sh

live=`env | grep "HOME=/home" | awk -F '=' '{print $2}'`

if [ $live = '/home/sms' ]; then
    cd /home/sms/send_sms_v2
fi

if [ $live = '/home/johnson' ]; then
    cd /home/johnson/send_sms_v2
fi


#kill sendsms.sh
num=`ps -ef|grep "./shell/sendsms.sh" |grep -v "defunct" |grep -v grep |wc -l`
if [ $num -ge 1 ]; then    
    killall -9 sendsms.sh > /dev/null 2>&1    
fi

#kill SENDSMS
num=`ps -ef|grep "SENDSMS_V2"|grep -v "defunct"|grep -v grep|wc -l`
if [ $num -ge 1 ]; then
    ./SENDSMS_V2 stop > /dev/null 2>&1        
fi

sleep 1

#Kill again if failed to stop
killall -9 sendsms.sh
killall -9 SENDSMS_V2


#./shell/sendsms.sh "start -l 1 -r 1 -n 2" &

#nohup ./shell/sendsms.sh "start -l 1 -r 2 -n 1" > /dev/null 2>&1 &
#nohup ./shell/sendsms.sh "start -l 2 -r 2 -n 1" > /dev/null 2>&1 &
#nohup ./shell/sendsms.sh "start -l 3 -r 5 -n 3" > /dev/null 2>&1 &
#nohup ./shell/sendsms.sh "start -l 4 -r 3 -n 2" > /dev/null 2>&1 &
nohup ./shell/sendsms.sh "start -l 5 -r 3 -n 2" > /dev/null 2>&1 &
#nohup ./shell/sendsms.sh "start -l 6 -r 5 -n 2" > /dev/null 2>&1 &
nohup ./shell/sendsms.sh "start -l 7 -r 10 -n 2" > /dev/null 2>&1 &
#nohup ./shell/sendsms.sh "start -l 8 -r 10 -n 2" > /dev/null 2>&1 &
nohup ./shell/sendsms.sh "start -l 9 -r 10 -n 5" > /dev/null 2>&1 &
nohup ./shell/sendsms.sh "start -l 10 -r 5 -n 2" > /dev/null 2>&1 &



