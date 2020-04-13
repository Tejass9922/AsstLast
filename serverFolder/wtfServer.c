#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <fcntl.h>
#include <dirent.h>



int createDIR(int sock)
{
    int read_size, write_size;
    char *message;
	char* dirName = malloc(100 * sizeof(char));
    static char command[1000];

    printf("socketCheck: %d\n", sock);
    int readTemp = recv(sock,dirName,100,0);
    int check; 
  
    check = mkdir(dirName); 
    char* manifestName = malloc(((sizeof(char) * readTemp) * 2) + 10);
    strcpy(manifestName, "./");
    strcat(manifestName, dirName);
    strcat(manifestName, "/");
    strcat(manifestName, dirName);
    strcat(manifestName, ".manifest");
    printf("readSize: %d  message: %s\n", readTemp, dirName);
    printf("fileName: %s\n", manifestName);
    int newFile = open(manifestName, O_WRONLY | O_APPEND | O_CREAT, 0666);
    printf("newFileFD: %d\n", newFile);
}

int returnFiles(int sock)
{
    int read_size, write_size;
    char *message;
	char* client_message = malloc(sizeof(char) * 2000);
    static char command[1000];
    char* fileContents = malloc(sizeof(char) * 2000);
    
    message = " \nHello Server Handler \n";
    int status = 0;
    char c[1];

    //recv(sock,client_message,2000,0))
//int mStatus = recv(sock,client_message,2000,0);
    printf("Hello: %s\n",client_message);
    while(recv(sock,client_message,2000,0) > 0){
   
     printf("reachedHere");
     printf("%s\n",client_message);

     int file = open(client_message, O_RDONLY, 777);
     printf("filename: %s fd: %d\n", client_message, file);

     int count = 0;
     do{
        
        status =  read(file, &c, 1);     
        if (status <=0 ){
            break;
        }
        else
        {
            fileContents[count] = c[0];
            count++;
        }
            
     
    }while(status>0);
     
     close(file);
     char c = file + '0';

     write(sock,fileContents,strlen(fileContents));
     client_message = malloc( sizeof(char) * 2000);
     fileContents = malloc( sizeof(char) * 2000);

     //write(sock, &c, 1);
    }
}


void *server_handler (void *fd_pointer);

int main(int argc, char **argv)
{

    int listenfd, connfd, *new_sock;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr; 
   
   listenfd = socket(AF_INET,SOCK_STREAM,0);
   if (listenfd == -1)
   {
	  perror("Could not create Socket \n"); 
   }
	puts("Socket Created");
  
  
   bzero(&servaddr,sizeof (servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
   int port = atoi(argv[1]);
   servaddr.sin_port = htons(port);
   
   if (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
   {
	   perror("bind failed");
	   return 1;
   }
   puts("bind success");
   listen(listenfd, 5);
   

   puts("Waiting for connections");
   clilen = sizeof(cliaddr);
    while ((connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen)))
  
	{
		puts("Connection accepted");
		
		pthread_t server_thread;
        new_sock = malloc(1);
        *new_sock = connfd;
		pthread_create(&server_thread,NULL,server_handler,(void*) new_sock);
	}
	if (connfd < 0)
	{
		perror("Accecpt Failed");
		return 1;
	}
    
	return 0;
  
   //close(connfd);
}

void *server_handler (void *fd_pointer)
{
	printf("Hello Server Handler \n");
	int sock = *(int *)fd_pointer;
    //char client_message[2000];
	static int send_once = 0;
	if (send_once < 1)
	{
	//write(sock,message,strlen(message));
	send_once++;
	}
    char* command = malloc(100 * sizeof(char));


    //read(sock, command, 100);
    recv(sock,command,2000,0);
    printf("recieved: %s\n", command);
    if (strcmp(command, "getFiles") == 0)
    {
        printf("got Command\n");
        returnFiles(sock);
    }
    if (strcmp(command, "create") == 0)
    {
        printf("got Command\n");
        char* replyCommand = "Got The Command";
        write(sock, replyCommand, strlen(replyCommand) + 1);
        createDIR(sock);
    }
    command = malloc (2000 * sizeof(char));
    


    //returnFiles(sock);
    
    //createDIR(sock);

    /*
    puts("Client disconnected");
        fflush(stdout);
    
	else if(read_size == -1)
    {
        perror("recv failed");
    }
    free(fd_pointer);
     
    return 0;
    */
}