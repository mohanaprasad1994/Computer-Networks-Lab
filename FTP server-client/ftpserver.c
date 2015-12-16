/* tcpserver.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include<dirent.h>

#define SIZE 1
#define NUMELEM 1000
int
main(int argc, char *argv[]) {
    chdir("FTP-SERVER");
    
    char root[1024];
    getcwd(root,1024);
    puts(root);
    int root_len=strlen(root);
    int sock, connected, bytes_received, true = 1;
    char send_data[1024], recv_data[1024];

    struct sockaddr_in server_addr, client_addr;
    int sin_size;
    if (argc < 2) {
        printf("Enter PortNo");
        exit(0);
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);

    if (bind(sock, (struct sockaddr *) &server_addr, sizeof (struct sockaddr)) == -1) {
        perror("Unable to bind");
        exit(1);
    }

    if (listen(sock, 5) == -1) {
        perror("Listen");
        exit(1);
    }

    printf("\nTCPServer Waiting for client on port %s\n", argv[1]);
    fflush(stdout);

    
    while (1) {

        sin_size = sizeof (struct sockaddr_in);

        connected = accept(sock, (struct sockaddr *) &client_addr, &sin_size);

        printf("\n I got a connection from (%s , %d)",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        fflush(stdout);        
        chdir(root);
        if(fork()==0) // fork a new child
        while(1){
        
            bytes_received = recv(connected, recv_data, 1024, 0);
            recv_data[bytes_received] = '\0';
            printf("bytes recieved=%d\n",bytes_received);
            if(bytes_received<=0){
                printf("Connection lost\n");
                fflush(stdout);
                break;
            }
            printf("\nRecieved data = %s ", recv_data);
            fflush(stdout);
            if(strcmp(recv_data,"QUIT")==0){
                printf("recieved QUIT from (%s , %d)\n",inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                fflush(stdout);
                break;
             }
             
            if(recv_data[0]=='C' && recv_data[1]=='W' && recv_data[2]=='D' && recv_data[3]==' '){ // change working directory
                send(connected,"OK",2,0);
                char buff[1024];
                getcwd(buff,1024);
                char temp[1024];
                strcpy(temp,&recv_data[4]);
                if(recv_data[4]=='~' || recv_data[4]=='/'){ // handle cd/.../.../... and cd ~/.../..
                    strcpy(temp,root);
                    temp[strlen(root)]='/';
                    strcpy(&temp[strlen(root)+1],&recv_data[5]);
                    printf("%s\n",temp);
                }
                if(chdir(temp)==-1)
                    send(connected,"Illegal CWD Operation",21,0);
                else{
                    char buff2[1024];
                    getcwd(buff2,1024);
                    printf("%s\n",buff2);
                    int flag=0;int i;
                    for(i=0;i<strlen(root) && flag==0;i++)
                        if(root[i]!=buff2[i]){
                            flag=1;
                            chdir(buff);
                            send(connected,"Illegal CWD Operation",21,0);
                        }
                        if (flag==0 && strcmp(buff2,root)==0)
                            send(connected,"/",1,0);
                        else if(flag==0){
                            send(connected,&buff2[strlen(root)],strlen(&buff2[strlen(root)]),0);
                        }
                  }
                }
                if(recv_data[0]=='P' && recv_data[1]=='W' && recv_data[2]=='D' && recv_data[3]=='\0'){ //send PWD
                    //printf("I am here");
                    send(connected,"OK",2,0);
                    char buff[1024];
                    getcwd(buff,1024);
                    if(strlen(buff)==root_len)
                        send(connected,"/",1,0);
                    else
                        send(connected,&buff[root_len],strlen(buff)-root_len,0);
                }
                
                if(recv_data[0]=='R' && recv_data[1]=='E' && recv_data[2]=='T' && recv_data[3]=='R' && recv_data[4]==' '){ //RETR handling 
                    printf("RETR\n");
                    fflush(stdout);
                    FILE * fil;int read;
                    char temp[1024];
                    strcpy(temp,&recv_data[5]);
                    if(recv_data[5]=='~' || recv_data[5]=='/'){ // handling path names
                        strcpy(temp,root);
                        temp[strlen(root)]='/';
                        strcpy(&temp[strlen(root)+1],&recv_data[6]);
                        printf("%s\n",temp);
                    }
                    if(!(fil=fopen(temp,"r")))
                        send(connected,"FILE DOES'T EXIST",17,0);
                    else{
                        send(connected,"File Name OK",12,0);
                        printf("HEHEHE");
                        fflush(stdout);
                        fseek(fil, 0L, SEEK_END);
                        int sz = ftell(fil);
                        fseek(fil, 0L, SEEK_SET);
                        char temp[1024];
                        snprintf (temp, sizeof(temp), "%d",sz);
                        //itoa(sz,temp,10);
                        //send(connected,temp,strlen(temp),0);
                        int flag=0;
                        while(flag==0){
                            read=fread(send_data,SIZE,NUMELEM,fil); // send the requested file and append @$ at the end
                            //printf("%d",read);
                            fflush(stdout);
                            if(read<1000){
                                send_data[read]='@';
                                send_data[read+1]='$';
                                flag=1;
                                read+=2;
                            }
                            send(connected,send_data,read,0);
                        }
                        fclose(fil);
                    }
                }
                
                if(recv_data[0]=='S' && recv_data[1]=='T' && recv_data[2]=='O' && recv_data[3]=='R' && recv_data[4]==' '){ // STOR handling
                    send(connected,"OK to SEND File",15,0);
                    FILE *fil;
                    int i;
                    int is_path=0;
                    for(i=strlen(recv_data);i>=5;i--)                // if /SD1/a.txt if given, this is the code for finding "a.txt"
                        if(recv_data[i]=='/'){
                           i++;
                           is_path=1;
                           break;
                        }
                    if(is_path==1)
                        fil=fopen(&recv_data[i],"w");
                    else
                        fil=fopen(&recv_data[5],"w");
                    int flag=0; // flag to see if end marker has been reached
                   //printf("HEHEHE");
                   
                    while(flag==0){
                       bytes_received=recv(connected,recv_data,1002,0);
                       //printf("%d ",bytes_received);
                       int i;
                       //if(sz<=10000)
                           for (i=0;i<bytes_received-1 && flag==0;i++)             // Check for end marker
                               if(recv_data[i]=='@' && recv_data[i+1]=='$'){
                                   flag=1;
                                   bytes_received-=2;
                               }
                       fwrite(recv_data,SIZE,bytes_received,fil);
                    }
                    fclose(fil);
                    send(connected,"File Received",13,0);                   // Send file received message
                }
                if(recv_data[0]=='N' && recv_data[1]=='L' && recv_data[2]=='S' && recv_data[3]=='T'){
                   //printf("NLST\n");
                   fflush(stdout);
                   send(connected,"OK",2,0);
                   //printf("HEHEHE\n");
                   fflush(stdout);
                   char buff[1024];
                   getcwd(buff,1024);
                   printf("%s",buff);
                   DIR *dirp = opendir(buff);
                   struct dirent *d;
                   while((d=readdir(dirp))!=NULL){
                       if(strcmp(d->d_name,".")==0 || strcmp(d->d_name,"..")==0);
                       else{
                       send(connected,d->d_name,strlen(d->d_name)+1,0);
                       bytes_received=recv(connected,recv_data,1024,0);
                       }
                       //printf("recv_data\n");
                   }
                   closedir(dirp);
                   send(connected,"@$",3,0);
                }
                fflush(stdout);
                
            
           }
           
           
        /*printf("\n SEND A WORD(q to quit): ");
        scanf("%s", send_data);
        send(connected, send_data, strlen(send_data), 0);
        if (strcmp(send_data, "q") == 0 || strcmp(send_data, "Q") == 0) {
            break;

        }*/
        
        close(connected);
    }

    close(sock);
    return 0;
}
