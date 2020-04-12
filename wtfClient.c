#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include<time.h>

typedef struct File{
    int version;
    char*filePath;
    char*hash;
}File;

typedef struct ConfigureInfo{

    char*IP;
    int portNumber;
}ConfigureInfo;

ConfigureInfo info;


void setTimeout(int milliseconds)
{
    // a current time of milliseconds
    int milliseconds_since = clock() * 1000 / CLOCKS_PER_SEC;

    // needed count milliseconds of return from this timeout
    int end = milliseconds_since + milliseconds;

    // wait while until needed time comes
    do {
        milliseconds_since = clock() * 1000 / CLOCKS_PER_SEC;
    } while (milliseconds_since <= end);
}

void add(char*projectName, char*fileName){
//find filePath
connectToServer(fileName);
//try to make the connection variables ussed global so we can call them here

}


void configure(char* host, char* port){

    char configureS [50] = "server.configure";
  int fd =  open(configureS,O_RDWR);
    if (fd!=-1){
        printf("*Overwriting Configure File**\n");
    }
    fd =open(configureS,O_RDWR|O_CREAT|O_TRUNC, S_IRUSR | S_IWUSR);
    write(fd,host,strlen(host));
    char sp = ' ';
    write(fd,&sp,1);
    write(fd,port, strlen(port));
    info.IP = (char*)(malloc(strlen(host)+1));
    strcpy(info.IP,host);
    info.portNumber = atoi(port);

}

int getFiles(int sockfd, char* fileName)
{
       char* server_reply = malloc(sizeof(char) * 2000);
       char* buffer = malloc(sizeof(char) *1);
       printf("File Name: %s\n",fileName);
        int status;
        char c;
        int fd = open(fileName,O_RDWR);

        char* command = "getFiles\0";

        
        /*
        if (send(sockfd,command,strlen(command),0) < 0)
        {
            printf("did not send\n");
        }
        */

        //write(sockfd, command, strlen(command));


        do{
   
            status =  read(fd, &c, 1); 
        
            if (status<=0){
                break;
            }
            if (c=='\n'){
                if (send(sockfd,buffer,strlen(buffer),0) < 0)
                {
                printf("Error \n");
                return 1;
                }
                if(recv(sockfd,server_reply,2000,0 ) < 0)
                {
                puts("Error");
                }
                printf("%s\n",server_reply);
                server_reply = malloc(sizeof(char) *2000);
                buffer = malloc(sizeof(char) *1);
            }

            else{
        
                int len = strlen(buffer);
                buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
                buffer[len] = c;
                buffer[len+1] = '\0';    
        
            }
        }while(status>0);
        if (buffer[0]!='\0'){
            if (send(sockfd,buffer,strlen(buffer),0) < 0)
                {
                printf("Error \n");
                return 1;
                }
                if(recv(sockfd,server_reply,2000,0 ) < 0)
                {
                puts("Error");
                }
                printf("%s\n",server_reply);
                server_reply = malloc(sizeof(char) *2000);
        } 
        close(fd);

        close(sockfd);
}

int connectToServer(char* portNumber, char* fileName){
      char cfp [100] = "server.configure";
      int configureFD = open(cfp,O_RDWR);
      if (configureFD!=-1){ 
        int sockfd;
        char* buffer = malloc(sizeof(char) *1);
        char server_reply[2000];
        ssize_t n;
  
        struct sockaddr_in servaddr; 

        sockfd = socket(AF_INET,SOCK_STREAM,0);
        if (sockfd == -1)
        {
            perror("Could not create socket");
        }
	    printf("Created Socket \n");
        bzero(&servaddr,sizeof (servaddr));
        servaddr.sin_family = AF_INET;
        int port1 = (info.portNumber);
        printf("portNumber: %d\n", port1);
        int port = atoi(portNumber);
        servaddr.sin_port = htons(port);
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        int cx  = connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr));
        while (cx==-1){
        printf("trying to reconnect\n");
        setTimeout(3000);
        cx = connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr));
        }

        return sockfd;
      
         
    }
    
    else
    {
          printf("**Cannot find a.Configure file to use to connect to server!**\n");
    }

    
    return 0;  
  }

int main(int argc, char **argv)
{
char*host;
char*port1;
   

     bool recur = false;
    bool compressC = false;
    bool decompressC = false;
    bool buildCode = false;
    int flagCounter = 0;
    int i;
   
        if (strcmp(argv[1],"configure")==0){
         host = argv[2];
         port1= argv[3];
         configure(host,port1);
        }
        if (strcmp(argv[1],"add")==0){
            flagCounter++;
            buildCode = true;
        }
        if (strcmp(argv[1],"destroy")==0){
            flagCounter++;
            decompressC = true;
        }
        if (strcmp(argv[1],"create")==0){
            flagCounter++;
           compressC = true;
        }
        if (strcmp(argv[1],"update")==0){
            flagCounter++;
           compressC = true;
        }
        if (strcmp(argv[1],"upgrade")==0){
            flagCounter++;
           compressC = true;
        }
        if (strcmp(argv[1],"commit")==0){
            flagCounter++;
           compressC = true;
        }
        if (strcmp(argv[1],"push")==0){
            flagCounter++;
           compressC = true;
        }
        if (strcmp(argv[1],"remove")==0){
            flagCounter++;
           compressC = true;
        }
        if (strcmp(argv[1],"currentversion")==0){
            flagCounter++;
           compressC = true;
        }
        if (strcmp(argv[1],"history")==0){
            flagCounter++;
           compressC = true;
        }
        if (strcmp(argv[1],"rollback")==0){
            flagCounter++;
           compressC = true;
        }
         if (strcmp(argv[1],"checkout")==0){
            flagCounter++;
           compressC = true;
        }
        if (strcmp(argv[1],"getFiles")==0){
           int sockfd = connectToServer(argv[2], argv[3]);
           getFiles(sockfd, argv[3]);
        }

       
        return 0;
}
    
  




    
  