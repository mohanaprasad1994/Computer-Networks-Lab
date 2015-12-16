/* udpserver.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include<time.h>
#include<iostream>
using namespace std;
#include <vector>
#include <sys/time.h>
#include<fcntl.h>
#include<sys/select.h>
#define MAX_WINDOW_SIZE 2050
#define MAX_MESSAGE_LEN 2050


struct ACK{
  long long int curr_window;
  char ack[MAX_WINDOW_SIZE];
  
};
struct buffer_slot{
  int seq_no;
  int present;
  long long int time_recieved;
};

long long int get_time(){
  struct timeval tv;
    gettimeofday(&tv,NULL);
    long long int time1=tv.tv_sec*1000000L+tv.tv_usec;
    return time1;
  
}

double PACKET_ERROR_RATE=0;
int MAX_PACKETS=1000;
int DEBUG=0;
int num_bits=3;
int WINDOW_SIZE=3;
int BUFFER_SIZE=100;
int num_present_in_buffer=0;
int num_present=0;
int curr_window_no=0;
int seq_no=0;
int ptr=0;
long long int MAX_SEQ_NO= (1<<(num_bits));
vector<buffer_slot> buffer;
long long int num_packets=0;
long long int start_time;

int sock;
unsigned int addr_len;
int  bytes_read;
char recv_data[1024];
struct sockaddr_in server_addr, client_addr;

void generate_buffer(){
  //cout<<"hi";
  buffer.clear();
  int i;
  for(i=0;i<WINDOW_SIZE;i++){
    buffer_slot * a=(buffer_slot *)malloc(sizeof(buffer_slot));
    a->seq_no=(seq_no+i)%MAX_SEQ_NO;
    a->present=0;
    buffer.push_back(*a);
  }
}

void recv_and_send_ack(){
  ACK to_send;
  to_send.curr_window=curr_window_no;
  //cerr<<"hello";
  to_send.ack[WINDOW_SIZE] = '\0';
  char recv_data[MAX_MESSAGE_LEN];
  ACK ac;
  long long int time1=get_time();
  bytes_read = recvfrom(sock, &ac, MAX_MESSAGE_LEN, 0,(struct sockaddr *) &client_addr, &addr_len);
  int num=rand()%100000000;
  //cerr<<"blah "<<num<<" "<<PACKET_ERROR_RATE<<endl;
  if(num>=PACKET_ERROR_RATE*100000000){
  
  long long int * seq= new long long int;
  *seq = ac.curr_window;
    //cerr<<"recieved "<<*seq<<" "<<num_present_in_buffer<<" "<<buffer.size()<<endl;

  int i;
  if(num_present<BUFFER_SIZE-1 || (num_present==BUFFER_SIZE-1)&&(*seq==buffer[ptr].seq_no)){
  int FLAG=0;
  for(i=0;i<WINDOW_SIZE;i++){
    //cout<<"buffer = "<<buffer[i].seq_no<<" ";
    if(buffer[i].present==1)
      to_send.ack[i]='1';
    else
      to_send.ack[i]='0';
    if(buffer[i].seq_no==*seq && buffer[i].present!=1){
      buffer[i].present=1;
      buffer[i].time_recieved=time1;
      to_send.ack[i]='1';
      num_present_in_buffer++;
      num_present++;
      num_packets++;
      FLAG=1;
      
    }
    if(buffer[i].seq_no==*seq && buffer[i].present==1){
      buffer[i].present=1;
      to_send.ack[i]='1';
      FLAG=1;
      
    }
  }
  //cout<<"current window "<<curr_window_no<<endl;
  if(FLAG==1){
    sendto(sock, &(to_send), sizeof(ACK), 0,(struct sockaddr *) &client_addr, sizeof (struct sockaddr));
  
    //cerr<<"sent ack"<<to_send.ack<<" "<<to_send.curr_window<<" time "<<time1-start_time<<endl;
    
  }
  else{
    
    sendto(sock, &(to_send), sizeof(ACK), 0,(struct sockaddr *) &client_addr, sizeof (struct sockaddr));
  
    //cerr<<"sent ack"<<to_send.ack<<" "<<to_send.curr_window<<endl;
    
  }
  if(DEBUG==1)
    while(buffer[ptr].present==1){
      cerr<<"Seq No: "<<buffer[ptr].seq_no<<" Time Recieved : "<<(buffer[ptr].time_recieved-start_time)/1000<<" : "<<(buffer[ptr].time_recieved-start_time)%1000<<endl;
      //cerr<<"current window "<<curr_window_no<<endl;
      ptr++;
      num_present--;
      
    }
  }
  if(num_present_in_buffer==WINDOW_SIZE){
    num_present_in_buffer=0;
    num_present=0;
    curr_window_no++;
    ptr=0;
    seq_no=(seq_no+WINDOW_SIZE)%MAX_SEQ_NO;
    generate_buffer();
  }
  
    
  }
}

/*
Port No has to be given as command line argument 
 */

int main(int argc, char *argv[]) {
      start_time=get_time();

    srand(time(NULL));
    int i,port_flag=0;
    for(i=1;i<argc;i++){
      if(argv[i][0]=='-'){
	switch(argv[i][1]){
	  case 'd':DEBUG=1;break;
	  
	  case 'p':port_flag=i+1;i++;break;
	  case 'n':num_bits=atoi(argv[i+1]);MAX_SEQ_NO= (1<<(num_bits));i++;break;
	  case 'e':PACKET_ERROR_RATE=atof(argv[i+1]);i++;break;
	  case 'N':MAX_PACKETS=atoi(argv[i+1]);i++;break;
	  case 'W':WINDOW_SIZE=atoi(argv[i+1]);i++;break;
 	  case 'B':BUFFER_SIZE=atoi(argv[i+1]);i++;break;
	  
	  
	}
      }
    }
    
    if (port_flag==0) {
        printf("PortNo Missing");
        exit(0);
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[port_flag]));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);

    if (bind(sock, (struct sockaddr *) &server_addr,
            sizeof (struct sockaddr)) == -1) {
        perror("Bind");
        exit(1);
    }

    addr_len = sizeof (struct sockaddr);

    printf("\nUDPServer Waiting for client on port %s\n", argv[port_flag]);
    fflush(stdout);
    generate_buffer();
    fflush(stdout);
    //cerr<<"kk";
    while (num_packets<MAX_PACKETS) {
        recv_and_send_ack();
    }
    //cout<<PACKET_ERROR_RATE<<endl;
    return 0;
}