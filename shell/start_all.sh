#!/bin/sh

trap "" SIGTERM SIGSTOP SIGPIPE

live=`env | grep "HOME=/home" | awk -F '=' '{print $2}'`
if [ $live = '/home/sms' ]; then
    cd /home/sms/send_sms_v2
fi

if [ $live = '/home/johnson' ]; then
    cd /home/johnson/send_sms_v2
fi

# SMS engine agent service
./shell/start_smsagent.sh
sleep 1
echo "SMS Engine agent service started"
###############################################################

# SMS engine send sms service
./shell/start_sendsms.sh
sleep 1
echo "SMS Engine send sms service started"

# SMS engine report match service
./shell/start_report_match.sh
sleep 1
echo "SMS Engine report match service started"
###############################################################


# SMS engine credit transaction commit service
./shell/start_sms_scan_and_commit.sh
sleep 1
echo "SMS Engine credit transaction commit service started"

# biNu Email incoming service
./shell/start_incoming_email.sh
sleep 1
echo "biNu Email incoming service started"
###############################################################


# biNu messenger deleted messages clean up service
./shell/start_messenger_cleanup.sh
sleep 1
echo "biNu messenger deleted messages clean up service started"

# biNu Group email service
./shell/start_group_email.sh
sleep 1
echo "biNu Group email service started"
###############################################################


#biNu Chat delete all conversations daemon service
if [ $live = '/home/sms' ]; then
    cd /home/sms/engines/binu_chat/public_html/binu_chat
fi

if [ $live = '/home/johnson' ]; then
    cd /home/johnson/engines/binu_chat/public_html/binu_chat
fi

./shell/start_delete_all_cons.sh
sleep 1
echo "biNu Chat delete all conversations daemon service started"
###############################################################


#biNu Survey Group Messages service
if [ $live = '/home/sms' ]; then
    cd /home/sms/engines/binu_email/group_messages/
fi

if [ $live = '/home/johnson' ]; then
    cd /home/johnson/engines/binu_email/group_messages/shell
fi

./shell/start_group_messages.sh
sleep 1
echo "biNu Survey Group Messages daemon service started"
###############################################################

echo "-----------------------------------"
echo "All done"


