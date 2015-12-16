# Makefile
#

SH = bash
CFLAGS = -g
LDFLAGS = -lm


main: ftpclient ftpserver
	
ftpclient: ftpclient.c 
	gcc $^ -o $@
	
ftpserver: ftpserver.c
	gcc $^ -o $@
