#!/bin/sh

trap "" SIGHUP SIGPIPE SIGTERM

live=`env | grep "HOME=/home" | awk -F '=' '{print $2}'`

if [ $live = '/home/sms' ]; then
    cd /home/sms/send_sms_v2
fi

if [ $live = '/home/johnson' ]; then
    cd /home/johnson/send_sms_v2
fi

arg1="$1"

ulimit -c unlimited
while [ 1 ]
do
  echo "[`date`] SENDSMS START" >>  ./log/start_sendsms.log          
  nohup ./SENDSMS_V2 $arg1 >> ./log/error_sendsms.log  2>&1
  sleep 1
done


