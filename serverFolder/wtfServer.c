#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <fcntl.h>
#include <dirent.h>

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

void createProject(int sock){


    char*projectName = (char*)(malloc(sizeof(char)*100));
    read(sock, projectName, 100);
    printf("recieved project name: %s",projectName);
    DIR *dr = opendir(projectName); 
    if (dr == NULL)  
    { 
        int check = mkdir(projectName,0777);
        char*filePath = (char*)(malloc(sizeof(char)*100));
        strcpy(filePath,projectName);
        strcat(filePath,"/");
        strcat(filePath,projectName);
        strcat(filePath,".Manifest");
        printf("file Path: %s\n",filePath);
        int filedescriptor = open(filePath, O_RDWR | O_APPEND | O_CREAT,0777); 
        printf("fD %d\n",filedescriptor);
            
    } 
        else{
            printf("\n**Project already Exists**\n");
        }
    //Now that we made a physical copy of a directory with the given project name on the server with a manifest
    //we are supposed to send that over to the client. How do we send it over? In what format?
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

    /*
    if (recv(sock,command,2000,0) > 0)
    {
        printf("Recieved\n");
    }

    if (strcmp(command, "getFiles\0") != 0)
    {
        printf("Rec: %s\n", command);
    }
    */


    read(sock, command, 100);
    //recv(sock,command,2000,0);
    printf("recieved: %s\n", command);
    if (strcmp(command,"create")==0)
    {
        printf("got Command\n");
        char* replyCommand = "Got The Command";
        write(sock, replyCommand, strlen(replyCommand) + 1);
        createProject(sock);
    }
    if (strcmp(command, "getFiles") == 0)
    {
        printf("got Command\n");
        returnFiles(sock);
    }
    command = malloc (100 * sizeof(char));
   
    //returnFiles(sock);
    

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
