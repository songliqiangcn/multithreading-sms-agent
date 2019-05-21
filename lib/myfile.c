//myfile.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "myfile.h"

#define FIFO_HEAD_LEN 10

int fifowrite(const char* file_name, const char* pbuf, unsigned long size)
{
        int fd;
        int bw;
        char head_buf[FIFO_HEAD_LEN] = "0";

        if (size == 0) return 0;

        fd = open(file_name, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IROTH|S_IRGRP);
        if (fd < 0)
        {
                fprintf(stderr, "filewrite(): failed open [%s]\n", file_name);
                return -1;
        }
        if (flock(fd, LOCK_EX) == -1)
        {
                fprintf(stderr, "filewrite(): failed lock [%s]\n", file_name);
                close(fd);
                return -1;
        }
        lseek(fd, 0, 0);
        if (read(fd, head_buf, FIFO_HEAD_LEN) < FIFO_HEAD_LEN)
        {
                write(fd, head_buf, FIFO_HEAD_LEN);
        }
        lseek(fd, 0, 2);
        bw = write(fd, pbuf, size);
	flock(fd, LOCK_UN);
        close(fd);
        return bw;
}

/*
int fiforead(const char* file_name, const char* pbuf, unsigned long size)
{
        int fd;
        int br;
        char head_buf[FIFO_HEAD_LEN] = "0";
        int readpos = 0;

        fd = open(file_name, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IROTH|S_IRGRP);
        if (fd < 0)
        {
                fprintf(stderr, "fileread(): failed open [%s]\n", file_name);
                return -1;
        }
        if (flock(fd, LOCK_EX) == -1)
        {
                fprintf(stderr, "fileread(): failed lock [%s]\n", file_name);
                close(fd);
                return -1;
        }
        lseek(fd, 0, 0);
        if (read(fd, head_buf, FIFO_HEAD_LEN) < FIFO_HEAD_LEN)
        {
                write(fd, head_buf, FIFO_HEAD_LEN);
        }
        else
        {
                readpos = FIFO_HEAD_LEN + atoi(head_buf);
                if (lseek(fd, readpos, 0) == -1) br = -1;
                else br = read(fd, pbuf, size);
                readpos += br;
                if (br < size)
                {
                        readpos = FIFO_HEAD_LEN;
                        ftruncate(fd, readpos);
                }
                lseek(fd, 0, 0);
                snprintf(head_buf, FIFO_HEAD_LEN, "%d", readpos-FIFO_HEAD_LEN);
                write(fd, head_buf, FIFO_HEAD_LEN);
        }
        flock(fd, LOCK_UN);
        close(fd);
        return br;
}
*/

int fiforead(const char* file_name, char* pbuf, unsigned long size)
{
        int fd;
        int br=0;
        char head_buf[FIFO_HEAD_LEN] = "0";
//        int readpos = 0;

        fd = open(file_name, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IROTH|S_IRGRP);
        if (fd < 0)
        {
                fprintf(stderr, "fileread(): failed open [%s]\n", file_name);
                return -1;
        }
        if (flock(fd, LOCK_EX) == -1)
        {
                fprintf(stderr, "fileread(): failed lock [%s]\n", file_name);
                close(fd);
                return -1;
        }
        
	lseek(fd, 0, 0);
        if (read(fd, head_buf, FIFO_HEAD_LEN) < FIFO_HEAD_LEN)
        {
                write(fd, head_buf, FIFO_HEAD_LEN);
        }
        else
        {
		br = lseek(fd, 0 - size, SEEK_END);
	        if (br >= FIFO_HEAD_LEN)
		{
			int rb;
			rb = read(fd, pbuf, size);
        		ftruncate(fd, br);
			lseek(fd, 0, 0);
	        	snprintf(head_buf, FIFO_HEAD_LEN, "%d", br);
        		write(fd, head_buf, FIFO_HEAD_LEN);
        		br = rb;
        	}
	}
	flock(fd, LOCK_UN);
        close(fd);
        return br;
}

void trimstring(char* line){
	if (line == 0) return;
	if (strlen(line) == 0) return;
	while(line[strlen(line)-1] == ' '||line[strlen(line)-1] == '\t' ||line[strlen(line)-1] == '\r' ||line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';
	while(line[0] == '\t' || line[0] == '\r' ||line[0] == '\n'||line[0] == ' ') 
		strcpy(line, line+1);
	return;
}

void chopstring(char* line){
	if (line == 0) return;
	if (strlen(line) == 0) return;
	while(line[strlen(line)-1] == ' '||line[strlen(line)-1] == '\t' ||line[strlen(line)-1] == '\r' ||line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';
		line[strlen(line)] = 0;
	return;
}

int SecurityCheck(const char* ipAllow, const char* ip){    
    const char* split = ",";
    char * p;    
    char tempstr[strlen(ipAllow)];
    strcpy(tempstr, ipAllow);
    
    if (strlen(ipAllow) == 0){
        //printf("ipallow list is empty\n");
        return 1;//allow all
    }
    else{
        p = strtok(tempstr, split);
        while(p != NULL){
            //printf ("check remote ip[%s] with ip[%s]\n", ip, p);
            
            if (strcmp(p, ip) == 0){
                //printf("Yes, ip[%s] is allow \n", ip);
                return 1;
            }
            p = strtok(NULL,split);
        }    
    }
    return 0;
}

void mylog(const char* logfile, const char* context){
	FILE * FP;
	int i;
	if (context == NULL) return;
	if (strlen(context) == 0) return;
	else{
		char tempstr[strlen(context)+10];
		strcpy(tempstr, context);
		
		for (i = 0; i < (int)strlen(tempstr);i++){
			if (tempstr[i] == '\r' || tempstr[i] == '\n')
				tempstr[i] = ' ';
		}
		
		if ((FP = fopen(logfile, "a")) == NULL){
			fprintf(stderr, "logdump: failed open file %s\n", logfile);
			return;
		}
		if (flock(fileno(FP), LOCK_EX) == -1){
			fprintf(stderr, "Failed lock [%s]\n", logfile);
			fclose(FP);
			return;
		}	
		fprintf (FP,"%s\n",tempstr);
		if (flock(fileno(FP),LOCK_UN) != 0)	{
			fprintf(stderr, "logdump: failed unlock file %s\n", logfile);
		}
		fclose (FP);
	}
}


char* addslashes(char *str){
    char *new_str;
    char *source, *target;
    char *end;
    int new_length;
    int length = strlen(str);
    
    if (length == 0){
        char *p = (char *) malloc(1);
        if (p == NULL){
            return p;
        }
        p[0] = 0;
        return p;
    }
    
    new_str = (char *) malloc(2 * length + 1);
    if (new_str == NULL){
        return new_str;
    }
    source = str;
    end = source + length;
    target = new_str;
    
    while(source < end){
        switch (*source){
            case '\0':
                *target++ = '\\';
                *target++ = '0';
                break;
            //case '\'':
            //case '\"':
            //case '\\':
              case '"':
                *target++ = '\\';
                *target++ = '"';                
                break;
            case '`':
                *target++ = '\\';
                *target++ = '`';
                break;
            case '$':
                *target++ = '\\';
                *target++ = '$';
                break;
            case '\\':
                *target++ = '\\';
                *target++ = '\\';
                break;
            default:
                *target++ = *source;
                break;
        }
        source++;
    }
    
    *target = 0;
    new_length = target - new_str;
    new_str = (char *) realloc(new_str, new_length + 1);
    return new_str;
}

void ProcessFolder(char* folder, const int f){
    struct stat buf;
    int is;
    
    is = stat(folder, &buf);
     
    if ((is < 0) || !S_ISDIR(buf.st_mode)){
        if (mkdir(folder, 0755) == -1){
            fprintf(stderr, "Can not create folder %s\n", folder);            
            exit(0);
        }
        if (f == 1){
            if (chmod(folder, 0777) == -1){
                fprintf(stderr, "Can not change folder permission %s\n", folder);            
                exit(0);
            }
        }                        
    }        
}

void RebuildDupDB(char * folder, int flag){    
    char cmd[250];
    struct stat buf;
    int is;
    
    is = stat(folder, &buf);
    
    
    if ((is == 0) && S_ISDIR(buf.st_mode)){
        if (flag == 1){//rebuild
            snprintf(cmd, sizeof(cmd), "rm -rf %s", folder);
            if (system(cmd) != 0){
                fprintf(stderr, "Can not run the command %s\n", cmd);
                exit(0);
            }
        }
        else{
            return;
        }
    }
    
    if (mkdir(folder, 0755) == -1){
        fprintf(stderr, "Can not create folder %s\n", folder);
        exit(0);
    }    
}



