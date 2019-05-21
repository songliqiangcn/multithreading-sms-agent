// myfile.h
#ifndef __MYFILE_H__
#define __MYFILE_H__

#include <stdio.h>
#include <sys/file.h>
#include <sys/stat.h>

int fifowrite(const char* file_name, const char* pbuf, unsigned long size);
int fiforead(const char* file_name, char* pbuf, unsigned long size);
void trimstring(char* line);
char* addslashes(char *str);
void ProcessFolder(char* folder, const int f);
int SecurityCheck(const char* ipAllow, const char* ip);

#endif

