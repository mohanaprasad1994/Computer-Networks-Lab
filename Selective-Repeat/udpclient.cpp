/* udpclient.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
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


#define MAX_MESSAGE_LEN 2040
#define MAX_WINDOW_SIZE 2050
/*
Command Line Arguments
Port No :argv[1]
IP address :argv[2]
 */
struct packet{
    
    long long int sequence_no;
    char message[MAX_MESSAGE_LEN];
    long long int packet_no;
    long long int window_no;
    int message_len;
    int num_of_trans;
    long long int first_sent_time;
    long long int last_sent_time;
    long long int time_out_time;
    
};
struct data{
  long long int seq_no;
  char message[MAX_MESSAGE_LEN];
};
struct ACK{
  long long int curr_window;
  char ack[MAX_WINDOW_SIZE];
  
};

int MAX_PACKET_LENGTH=100;
int PACKET_GEN_RATE=200;
int BUFFER_SIZE=100;
int WINDOW_SIZE=3;
int MAX_PACKETS=1000;
int DEBUG=0;
int num_bits=3;
int size_of_buffer=0;
int sock;
unsigned int addrlen;
int rv,nbytes;
long long int packet_len_sum=0;
long long int num_packets=0;
long long int num_packets_sent=0;
long long int sender_start_time;
long long int last_add_buffer;
long long int MAX_SEQ_NO= (1<<(num_bits));
long long int RTT_avg=300000;
long long int curr_window=0;
int window_start=0;
int window_end=WINDOW_SIZE;
int retransmission_FLAG=0;
long long int num_transmissions=0;
int num_ack_recved_window=0;
vector <packet> buffer;
vector <data> send_buffer;
int pack_rate;

struct sockaddr_in server_addr;
struct hostent *host;

long long int get_time(){
  struct timeval tv;
    gettimeofday(&tv,NULL);
    long long int time1=tv.tv_sec*1000000L+tv.tv_usec;
    return time1;
  
}

void generate_packets(){
    
    long long int time1=get_time();
    //cout<<time1-sender_start_time<<endl;
    
    int num_packets_to_add=((time1-last_add_buffer)*1.0/1000000)*PACKET_GEN_RATE;
    last_add_buffer= last_add_buffer + (long long int)(num_packets_to_add*1.0/PACKET_GEN_RATE*1000000);
    //cout<<num_packets_to_add<<" ";
    int i,j;
    for (i=0;i<num_packets_to_add && num_packets_sent<MAX_PACKETS;i++){
        
        
        if(size_of_buffer<BUFFER_SIZE){
	    struct packet *pac=(packet *)malloc(sizeof(packet));
	    pac->message_len=(rand()%(MAX_PACKET_LENGTH-40) )+ 40;
	    packet_len_sum+=pac->message_len;
	    //cerr<<"hello";
	    //for (j=0;j<pac->message_len;j++)
	    //  pac->message[j]=rand()%256;
	    pac->num_of_trans=0;
            pac->packet_no=num_packets;
            num_packets++;
            pac->window_no=pac->packet_no/WINDOW_SIZE;
            pac->sequence_no=pac->packet_no%MAX_SEQ_NO;
            buffer.push_back((*pac));
	    
 
	    data * send_data=(data*)malloc(sizeof(data));
	    send_data->seq_no=buffer[i].sequence_no;
	    int j;
	    //for(j=0;j<buffer[i].message_len;j++)
	    //  send_data->message[j]=buffer[i].message[j];
	    
	    send_buffer.push_back((*send_data));
	    
            size_of_buffer++;
	    //cout<<pac->packet_no<<" "<<pac->sequence_no<<" "<<pac->window_no<<" "<<size_of_buffer;
	    
	    
        }
    }
    
}

void send_packets(){
    int len=buffer.size();
    int i;
    struct timeval tv1;
    
    for(i=0;i<len;i++){
	gettimeofday(&tv1,NULL);
        if(buffer[i].window_no==curr_window){
            if (buffer[i].num_of_trans==0){
                buffer[i].num_of_trans++;
                struct timeval tv;
		gettimeofday(&tv,NULL);
		long long int time1=get_time();
		
		
		buffer[i].first_sent_time=time1;
		buffer[i].last_sent_time=time1;
		if(buffer[i].packet_no<10)
		  buffer[i].time_out_time=time1+300000;
		else //buffer[i].time_out_time=time1+80;
 		  buffer[i].time_out_time=time1+2*RTT_avg;
		
		//buffer[i].time_out_time=time1+300000;
		
		
		num_transmissions++;
		//cout<<buffer[i].packet_no<<" "<<buffer[i].sequence_no<<" "<<buffer[i].window_no<<endl;
		
		sendto(sock, &(buffer[i]), sizeof(long long int)+buffer[i].message_len, 0,(struct sockaddr *) &server_addr, sizeof (struct sockaddr));

            }       
            
            else if (buffer[i].time_out_time<=tv1.tv_sec*1000000L+tv1.tv_usec){
                buffer[i].num_of_trans++;
                struct timeval tv;
		long long int time1=get_time();
		
                sendto(sock, &(buffer[i]), sizeof(long long int)+buffer[i].message_len, 0,(struct sockaddr *) &server_addr, sizeof (struct sockaddr));
		
		buffer[i].last_sent_time=time1;
		if(buffer[i].packet_no<10)
		  buffer[i].time_out_time=time1+300000;
		else //buffer[i].time_out_time=time1+80;
		  buffer[i].time_out_time=time1+2*RTT_avg;
		
		buffer[i].time_out_time=time1+300000;
		num_transmissions++;
		
		//cout<<buffer[i].packet_no<<" "<<buffer[i].sequence_no<<" "<<buffer[i].window_no<<" "<<buffer[i].num_of_trans<<endl;

		if (buffer[i].num_of_trans>10){
		   retransmission_FLAG=1;
		   break;
		}
            }       
        }
    
    }
    //cout<< num_transmissions<<" "<<size_of_buffer<<endl;
} 


void recieveACK(){
  
  int i;
  for (i=0;i<WINDOW_SIZE;i++){
	

        //if(rv == 1){ 
	    ACK recv;
            nbytes = recvfrom(sock, &recv, sizeof(ACK), MSG_DONTWAIT, (struct sockaddr *) &server_addr, &addrlen);
	    if(nbytes==-1)
	      break;
	    
	    
	    long long int time1=get_time();
	    //cerr<<"WTH "<< recv.ack<<" "<<recv.curr_window<<endl;
	    if(nbytes!=-1){
	    int j;
	    
	    if (recv.curr_window==curr_window){
	    
	    for(j=0;j<WINDOW_SIZE;j++)
	      if (recv.ack[j]=='1'){
		int len=buffer.size();
		int k;
		for(k=0;k<len;k++)
		  if(buffer[k].window_no==curr_window && buffer[k].sequence_no==(curr_window*WINDOW_SIZE+j)%MAX_SEQ_NO){
		   
		    num_ack_recved_window++;
		    
		    
		    RTT_avg=(RTT_avg*num_packets_sent+time1-buffer[k].first_sent_time)*1.0/(num_packets_sent+1);
		    //cerr<<"RTT AVG "<<RTT_avg<<endl;
		    if(DEBUG==1)
		      cerr<<"Seq "<<buffer[k].sequence_no<<": "<<"Time Generated: "<<(buffer[k].first_sent_time-sender_start_time)/1000<<" : "<<(buffer[k].first_sent_time-sender_start_time)%1000<<" RTT : "<<(time1-buffer[k].first_sent_time)<<" Number of Attempts: "<< buffer[k].num_of_trans<<endl;
		    //cerr<<"buffer size "<<buffer.size()<<endl;
		    buffer.erase(buffer.begin()+k);
		    send_buffer.erase(send_buffer.begin()+k);
		    size_of_buffer--;
		    num_packets_sent++;
		    break;
		  }
	      }
		    
	    }
	    else {
	      //cerr<<"CURR WINDOW != RECIEVED ACK WINDOW\n";
	      int len=buffer.size();
		int k;
	      for(k=0;k<len;k++)
		  if(buffer[k].window_no<recv.curr_window){
		    
		    num_ack_recved_window++;
		    
		    
		    RTT_avg=(RTT_avg*num_packets_sent+time1-buffer[k].first_sent_time)*1.0/(num_packets_sent+1);
		    //cerr<<"RTT AVG "<<RTT_avg<<endl;
		    if(DEBUG==1)
		      cerr<<"Seq "<<buffer[k].sequence_no<<": "<<"Time Generated: "<<(buffer[k].first_sent_time-sender_start_time)/1000<<" : "<<(buffer[k].first_sent_time-sender_start_time)%1000<<" RTT : "<<(time1-buffer[k].first_sent_time)<<" Number of Attempts: "<< buffer[k].num_of_trans<<endl;
		    //cerr<<"buffer size "<<buffer.size()<<endl;
		    buffer.erase(buffer.begin()+k);
		    send_buffer.erase(send_buffer.begin()+k);
		    size_of_buffer--;
		    num_packets_sent++;
		    break;
		  }
	    }
	    
	    }
	    
	    

        //} 
    
    
  }
  
  if (num_ack_recved_window==WINDOW_SIZE){
    curr_window++;
    num_ack_recved_window=0;
    //cerr<<"hmm";
    
  }
  
  
  
}



int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    sender_start_time=get_time();
    last_add_buffer=sender_start_time;
    
    int i,port_flag=0,host_flag=1;
    for(i=1;i<argc;i++){
      if(argv[i][0]=='-'){
	switch(argv[i][1]){
	  case 'd':DEBUG=1;break;
	  case 's':host_flag=i+1;i++;break;
	  case 'p':port_flag=i+1;i++;break;
	  case 'n':num_bits=atoi(argv[i+1]);MAX_SEQ_NO= (1<<(num_bits));i++;break;
	  case 'L':MAX_PACKET_LENGTH=atoi(argv[i+1]);i++;break;
	  case 'R':PACKET_GEN_RATE=atoi(argv[i+1]);i++;break;
	  case 'N':MAX_PACKETS=atoi(argv[i+1]);i++;break;
	  case 'W':WINDOW_SIZE=atoi(argv[i+1]);i++;break;
 	  case 'B':BUFFER_SIZE=atoi(argv[i+1]);i++;break;
	  
	  
	}
      }
    }
    pack_rate=PACKET_GEN_RATE;
    if (WINDOW_SIZE>(MAX_SEQ_NO-1)/2){
        cout<<"Window size is greater than (Max seq no)/2 \n";
    }
    else if (WINDOW_SIZE<=0){
        cout<<"Window size is <=0 \n";
    }
   
    
    
    if (port_flag==0) {
        printf("Enter PortNo");
        exit(0);
    }
    if (host_flag==1) {
        printf("Enter Reciever name/IP");
        exit(0);
    }

      host = (struct hostent *) gethostbyname((char *) argv[host_flag]);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[port_flag]));
    server_addr.sin_addr = *((struct in_addr *) host->h_addr);
    bzero(&(server_addr.sin_zero), 8);
    //fd_set readfds; 
    //fcntl(sock, F_SETFL, O_NONBLOCK); 
    while (num_packets_sent<MAX_PACKETS && retransmission_FLAG==0) {
        generate_packets();
	send_packets();
	recieveACK();
    }
    cout<< "PACKET GEN RATE: "<<pack_rate<<" per second"<<endl;
    cout<< "AVG PACKET LENGTH: "<<packet_len_sum/num_packets+4<<" Bytes"<<endl;
    cout<< "RETRANSMISSION RATIO: "<<1.0*num_transmissions/num_packets_sent<<endl;
    cout<< "AVG RTT: "<<RTT_avg<<endl;
    


}
