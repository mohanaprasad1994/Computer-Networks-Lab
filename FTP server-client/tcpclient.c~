/* tcpclient.c */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


#define SIZE 1
#define NUMELEM 1000

/////// Directory names are not more than 1024 characters
/////// CWD only one space

/////// files are byte oriented

/*
argv[2]: ipaddress of server
argv[1]: port no of server
 */

int
main(int argc, char *argv[]) {

    int sock, bytes_received;
    
    char send_data[1024], recv_data[1024];
    struct hostent *host;
    struct sockaddr_in server_addr;
    if (argc < 3) {
        printf("Enter PortNo");
        exit(0);
    }

    host = gethostbyname(argv[1]);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr = *((struct in_addr *) host->h_addr);
    bzero(&(server_addr.sin_zero), 8);

    if (connect(sock, (struct sockaddr *) &server_addr,
            sizeof (struct sockaddr)) == -1) {
        perror("Connect");
        exit(1);
    }
    
    while(1){
        gets(send_data);
        //puts(send_data);
        if (strcmp(send_data,"quit")==0){
            char *data="QUIT";
            send(sock, data, strlen(data), 0);
            break;
        }
        if (send_data[0]=='l' && send_data[1]=='c' && send_data[2]=='d' && send_data[3]==' '){
            if(chdir(&send_data[4])==-1)
                printf(" %s directory doesn't exist",&send_data[4]);
            else{
                    char buff[1024];
                    getcwd(buff,1024);
                    printf("%s\n",buff);
                }
        }        
        if (send_data[0]=='c' && send_data[1]=='d' && send_data[2]==' '){ // handling cd
           char temp[1024];
           strcpy(temp,"CWD ");
           strcpy(&temp[4],&send_data[3]);
           send(sock,temp,strlen(temp),0);
           bytes_received=recv(sock,recv_data,1024,0); // recieve confirmation
           recv_data[bytes_received]='\0';
           if(bytes_received==0 ){
               printf("Connection failed\n");
               break;  
            }
           else if(strcmp(recv_data,"OK")!=0){
               printf("server didn't send OK\n");
               break;
               }
           else{
              bytes_received=recv(sock,recv_data,1024,0);
              recv_data[bytes_received]='\0';
              if(strcmp(recv_data,"Illegal CWD Operation")==0) // check for illegal operation
                   printf("Illegal CWD Operation\n");
              else
                printf(" changed directory to %s\n",recv_data);
           }
        } 
        
        if(send_data[0]=='p' && send_data[1]=='w' && send_data[2]=='d' ){ // prints current working directory of server and client
            char buff[1024];
            getcwd(buff,1024);
            printf(" present workin directory of client is %s\n",buff);
            send(sock,"PWD",3,0);
            bytes_received=recv(sock,recv_data,1024,0);
            recv_data[bytes_received]='\0';
            if(bytes_received==0 ){
               printf("Connection failed\n");
               break;  
            }
           else if(strcmp(recv_data,"OK")!=0){ // check for confirmation
               printf("server didn't send OK\n");
               break;
               }
           else{
               bytes_received=recv(sock,recv_data,1024,0);
               recv_data[bytes_received]='\0';
               printf("present working directory of server is %s\n",recv_data);
           }
        } 
        
        if ( send_data[0]=='g' && send_data[1]=='e' && send_data[2]=='t' && send_data[3]==' '){ // get a file
           char temp[1024];
           strcpy(temp,"RETR ");
           strcpy(&temp[5],&send_data[4]);
           send(sock,temp,strlen(temp),0);
           bytes_received=recv(sock,recv_data,1024,0); // confirmation
           recv_data[bytes_received]='\0';
           if(bytes_received==0 ){
               printf("Connection failed\n");
               break;  
            }
           else if(strcmp(recv_data,"File Name OK")!=0){
               //printf("%s\n",recv_data);
               printf("server didn't send \"File Name OK\" or file doesn't exist\n");
               
            }
           else{
               //printf("HEHEHE\n");
               //bytes_received=recv(sock,recv_data,1024,0);
               recv_data[bytes_received]='\0';
               int sz=atoi(recv_data);
               //printf("size of file = %d bytes\n",sz);
               int i;
               int is_path=0;
               for(i=strlen(send_data)-1;i>=4;i--)                // if /SD1/a.txt if given, this is the code for finding "a.txt"
                   if(send_data[i]=='/'){
                       i++;
                       is_path=1;
                       break;
                   }
               FILE *fil;
               if(is_path==1)
                   fil=fopen(&send_data[i],"w");
               else 
                   fil=fopen(&send_data[4],"w");
               int flag=0; // flag to see if end marker has been reached
               //printf("HEHEHE");
               
               while(flag==0){
                   bytes_received=recv(sock,recv_data,1002,0);
                   //printf("%d ",bytes_received);
                   int i;
                   //if(sz<=10000)
                       for (i=0;i<bytes_received-1 && flag==0;i++)
                           if(recv_data[i]=='@' && recv_data[i+1]=='$'){
                               flag=1;
                               bytes_received-=2;
                           }
                   fwrite(recv_data,SIZE,bytes_received,fil);
                   sz-=bytes_received;
                   //printf("%d\n",sz);
                   
               }
               
               fclose(fil);
               printf("DONE\n");
           }    
        }       
            
        if(send_data[0]=='p' && send_data[1]=='u' && send_data[2]=='t' && send_data[3]==' '){ // put a file
                    FILE * fil;
                    if(!(fil=fopen(&send_data[4],"r")))
                        printf("No such file\n");
                    else{
                       char temp[1024];int read;
                       strcpy(temp,"STOR ");
                       strcpy(&temp[5],&send_data[4]);
                       send(sock,temp,strlen(temp),0);
                       bytes_received=recv(sock,recv_data,1024,0);
                       recv_data[bytes_received]='\0';
                       if(bytes_received==0 ){
                           printf("Connection failed\n");
                           break;  
                        }
                       else if(strcmp(recv_data,"OK to SEND File")!=0){ // confirmation
                           //printf("%s\n",recv_data);
                           printf("server didn't send \"OK to SEND File\" or file doesn't exist\n");
                           
                        }
                        else{
                            // loop to send the data
                            int flag=0;
                            while(flag==0){
                                read=fread(send_data,SIZE,NUMELEM,fil);
                                //printf("%d",read);
                                fflush(stdout);
                                if(read<1000){// append @$ at the end
                                    send_data[read]='@';
                                    send_data[read+1]='$';
                                    flag=1;
                                    read+=2;
                                }
                                send(sock,send_data,read,0);
                            }
                            fclose(fil);
                            bytes_received=recv(sock,recv_data,1024,0);
                            recv_data[bytes_received]='\0';
                            if(strcmp(recv_data,"File Received")==0)
                                printf("File sent\n");
                            else
                                printf("Error in sending file\n");
                        }
                        
                        
                    }
        }  
        
        if (send_data[0]=='l' && send_data[1]=='s' && (send_data[2]=='\0' || send_data[2]==' ')){
            send(sock,"NLST",4,0);
            //printf("HELLO");
            fflush(stdout);
            bytes_received=recv(sock,recv_data,1024,0);
            recv_data[bytes_received]='\0';
            if(bytes_received==0 ){
               printf("Connection failed\n");
               break;  
             }
            else if(strcmp(recv_data,"OK")!=0){
               //printf("%s\n",recv_data);
               printf("server didn't send \"OK\"\n");
               
             }
            else{
                
                int flag=0; // flag to see if end marker has been reached
               //printf("HEHEHE");
               
               while(flag==0){
                   bytes_received=recv(sock,recv_data,1002,0);
                   send(sock,"OK",3,0);
                   //printf("%s\n",recv_data);
                   int i;
                   //if(sz<=10000)
                       for (i=0;i<bytes_received-1 && flag==0;i++)
                           if(recv_data[i]=='@' && recv_data[i+1]=='$'){
                               flag=1;
                               bytes_received-=2;
                           }
                   if(flag!=1)
                       printf("%s\n",recv_data);
                   else
                       printf("----------\n");
                   
                   
               }
               
               
               
            }
            
        
        }
        
    
    }
    /*bytes_received = recv(sock, recv_data, 1024, 0);
    recv_data[bytes_received] = '\0';
    printf("\nRecieved data = %s ", recv_data);*/

    close(sock);
    
    return 0;
}
