CC=gcc
CFLAGS=-Wall -g -lpthread -ldb
#MYSQL_INCLUDE=-I/usr/local/mysql32358/include/mysql -I/usr/local/mysql32358/include
#MYSQL_LIB=-L/usr/local/mysql32358/lib/mysql -lmysqlclient
INCLUDE=-I. -I./include 
LIB=-L./lib 

PUBLIC_OBJS = ./lib/mysock.o ./lib/myfile.o ./lib/md5.o

all:clean ./SMSAGENT_V2 ./SENDSMS_V2 

.c.o:
	$(CC) $(LIB) $(INCLUDE) -c $< -o $@
.cpp.o:
	$(CC) $(LIB) $(INCLUDE) -c $< -o $@


SMSAGENT_OBJS=./sms_agent.o
./SMSAGENT_V2: $(SMSAGENT_OBJS) $(PUBLIC_OBJS)
	$(CC) -o $@ $(SMSAGENT_OBJS) $(CFLAGS) $(PUBLIC_OBJS) $(LIB) $(INCLUDE)

SENDSMS_OBJS=./send_sms.o
./SENDSMS_V2: $(SENDSMS_OBJS) $(PUBLIC_OBJS)
	$(CC) -o $@ $(SENDSMS_OBJS) $(CFLAGS) $(PUBLIC_OBJS) $(LIB) $(INCLUDE)

clean:
	rm -rf *.o
	rm -rf $(PUBLIC_OBJS)
	
	


