# Makefile
#

SH = bash
CFLAGS = -g
LDFLAGS = -lm


main: sender reciever
	
sender: udpclient.cpp 
	g++ $^ -o $@
	
reciever: udpserver.cpp
	g++ $^ -o $@
