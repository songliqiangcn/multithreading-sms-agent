// mysock.c
#include "mysock.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int _my_inet_ntoa(struct in_addr in, char* ip_addr)
{
	unsigned char* bytes=(unsigned char *)&in;
	sprintf(ip_addr, "%d.%d.%d.%d",bytes[0],bytes[1],bytes[2],bytes[3]);
	return 0;
}

int listen_socket(const char* bind_ip, unsigned long port)
{
	//struct	hostent *hp;
    struct	sockaddr_in local;
    //char	hostname[64];
    int		i_socket;
	int		optval=1;

	local.sin_family = AF_INET;
    /*gethostname(hostname, sizeof(hostname));
    fprintf(stdout, "HOSTNAME:[%s]\n", hostname);
    if((hp=gethostbyname((char*)hostname))==NULL)
    {
        fprintf(stdout, "ERR:[%s]:local host unknown.\n",hostname);
        return -1;
    }

    bcopy(hp->h_addr,&local.sin_addr,hp->h_length);
    */
    local.sin_port = htons(port);
    local.sin_addr.s_addr=(strlen(bind_ip))?inet_addr(bind_ip):INADDR_ANY;

    i_socket = socket(AF_INET,SOCK_STREAM,0);

    if (i_socket == -1)
    {
        perror("socket() failed with error");
        return -1;
    }
	if (setsockopt(i_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
	{
		perror("setsockopt");
		return -1;
	}
    if (bind(i_socket,(struct sockaddr*)&local,sizeof(local) ) == -1)
    {
        perror("bind() failed with error");
        return -1;
    }

    if (listen(i_socket, 5) == -1)
    {
        perror("listen() failed with error \n");
        return -1;
    }
    return i_socket;
}

int safe_recv(int sd, char* pBuf, int size, int timeout)
{
	fd_set  fds;
	struct  timeval tv;

	int br = 0;
	int byte_rcv = 0;

	if (sd < 0) return -1;
	while(byte_rcv < size)
	{
		FD_ZERO(&fds);
       		FD_SET(sd, &fds);
		tv.tv_sec=timeout;
		tv.tv_usec=0;

		if (select(sd+1, &fds, 0, 0, &tv) < 0)
                return MYSOCK_CLOSED_ERR;  /* error in socket */
		else if(!FD_ISSET(sd, &fds))
                return MYSOCK_TIMEOUT_ERR;   /* time out */

		br = recv(sd, pBuf + byte_rcv, size - byte_rcv, 0);
		if (br == -1 || br == 0) {return MYSOCK_CLOSED_ERR;}
		byte_rcv += br;
	}
	return size;
}

int safe_send(int sd, const char* pBuf, int size, int timeout)
{
	fd_set  fds;
        struct  timeval tv;

	int bs = 0;
	unsigned int bcount = 0;

	if (sd < 0) return -1;
	while(bcount < size)
	{
		FD_ZERO(&fds);
        	FD_SET(sd, &fds);
                tv.tv_sec=timeout;
                tv.tv_usec=0;

                if (select(sd+1, 0, &fds, 0, &tv) < 0)
                	return MYSOCK_CLOSED_ERR;  /* error in socket */
                else if(!FD_ISSET(sd, &fds))
                	return MYSOCK_TIMEOUT_ERR;   /* time out */
	
		bs = send(sd, pBuf+bcount, size-bcount,0);
		if (bs == -1 || bs == 0) {return MYSOCK_CLOSED_ERR;}
		bcount += bs;
	}
	return size;
}

int connect_to(const char* ip, unsigned int port)
{
	int sd;
	struct sockaddr_in local;
	struct hostent *hp;

	local.sin_family = AF_INET;
	if((hp=gethostbyname(ip)) == NULL) 
	{
		perror("gethostbyname()");
		return -1;
	}

	memcpy(&local.sin_addr, hp->h_addr, hp->h_length);
    	local.sin_port = htons(port);
    	sd = socket(AF_INET,SOCK_STREAM,0);

    	if (sd == -1) 
	{
		close(sd);
		return -1;
	}
	if (connect(sd, (struct sockaddr *)&local, sizeof(local)) == -1) 
	{
		perror("connect()");
		close(sd);
		return -1;
	}	
	return sd;
}

int safe_close(int conn)
{
	if (conn < 0) return 0;
	shutdown(conn, 2);
	close(conn);
	return 0;
}
