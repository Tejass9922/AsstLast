#include<pthread.h>
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
#include <time.h>
#include <openssl/sha.h>
#include <dirent.h>
#include <openssl/err.h>



char* readInFile(char* fileName);
int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void listDirectoryRecursively(char *basePath)
{
    if (is_regular_file(basePath)==0){
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
          
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            printf("%s\n",path);
            listDirectoryRecursively(path);
        }
    }

    closedir(dir);

    }
    
     
     
}

void listFilesRecursively(char *basePath)
{
    if (is_regular_file(basePath)==1){
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
          
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            printf("%s\n", path);
            listFilesRecursively(path);
        }
    }

    closedir(dir);

    }

}
    
     
  

void checkout(int sock)

{
    char*projectName = (char*)(malloc(sizeof(char)*100));
    int readSize = recv(sock, projectName, 100, 0);//gets the name of the project
    if (readSize > 0)
    {
        printf("Got requested path\n");
    }
    DIR *dr = opendir(projectName); 
    if (dr == NULL)  
    { 
        printf("Project does not Exist\n");
        char* DNE = "DNE";
        send(sock,DNE,strlen(DNE),0); //if folder DNE it sends an error message to client and stops 
        return;        
    }
    else
    {
        char* reply = "Got Path";
        send(sock,reply,strlen(reply),0); 
        listDirectoryRecursively(projectName);
        printf("\n");
       // listFilesRecursively(projectName);
    }
    

}

void history(int sock){

    char*projectName = (char*)(malloc(sizeof(char)*100));
    int readSize = recv(sock, projectName, 100, 0);//gets the name of the project
    if (readSize > 0)
    {
        printf("Got requested path\n");
    }

     char path[strlen(projectName)+5+strlen(".history")];
            strcpy(path,projectName);
            strcat(path,"/");
            strcat(path,".history");



    DIR *dr = opendir(projectName); 
        if (dr == NULL)  
        { 
            printf("Project does not Exist\n");
            char* DNE = "DNE";
            send(sock,DNE,strlen(DNE),0); //if folder DNE it sends an error message to client and stops 
            return;
            
        } 
        else
        {
            int fd = open(path,O_RDWR);
            if (fd!=-1){
                char* buffer = malloc(sizeof(char) * (strlen(readInFile(path))));
                buffer = readInFile(path); //opens and store the history file into a buffer
                printf("History Buffer: %s\n", buffer);
                int length = strlen(buffer);
                char size[10];
                printf("length: %d\n", length); 
                sprintf(size,"%d",length); //changes the integer into a char array to be sent over to the client
                send(sock,size,10,0); //sends the size of the buffer that it will send next 
                char temp[8];
                recv(sock,temp,8,0); //recieves a confirmation that the client got the size 
                send(sock,buffer,length,0); // sends over the actual buffer containing the history file.
            }
            else
            {
                 printf("File does not exist!\n");
                 char* DNE = "DNE";
                send(sock,DNE,strlen(DNE),0); //if folder DNE it sends an error message to client and stops 
                return;
            }
        }
    return;
}

void currentVersion(int sock){

    char*projectName = (char*)(malloc(sizeof(char)*100));
    int readSize = recv(sock, projectName, 100, 0);//gets the name of the project
    if (readSize > 0)
    {
        printf("Got requested path\n");
    }

     char path[strlen(projectName)+5+strlen(".Manifest")];
            strcpy(path,projectName);
            strcat(path,"/");
            strcat(path,".Manifest");



    DIR *dr = opendir(projectName); 
        if (dr == NULL)  
        { 
            printf("Project does not Exist\n");
            char* DNE = "DNE";
            send(sock,DNE,strlen(DNE),0); //if folder DNE it sends an error message to client and stops 
            return;
            
        } 
        else
        {
            char* buffer = malloc(sizeof(char) * (strlen(readInFile(path))));
            buffer = readInFile(path); //opens and store the current version file into a buffer
            printf("Current Version: \n%s\n", buffer);
            int length = strlen(buffer);
            char size[10];
            printf("length: %d\n", length); 
            sprintf(size,"%d",length); //changes the integer into a char array to be sent over to the client
            send(sock,size,10,0); //sends the size of the buffer that it will send next 
            char temp[8];
            recv(sock,temp,8,0); //recieves a confirmation that the client got the size 
            send(sock,buffer,length,0); // sends over the actual buffer containing the current version.

        }
    return;
}

void commit(int socket){
    
    char*projectName = (char*)(malloc(sizeof(char)*100));
    int readSize = recv(socket, projectName, 100, 0);
    if (readSize > 0)
    {
        printf("Got requested path\n");
    }

     char path[strlen(projectName)+5+strlen(".Manifest")];
            strcpy(path,projectName);
            strcat(path,"/");
            strcat(path,".Manifest");



    DIR *dr = opendir(projectName); 
        if (dr == NULL)  
        { 
            printf("Project does not Exist" );
            return;
            
        } 
        else
        {
            char* buffer = malloc(sizeof(char) * (strlen(readInFile(path))));
            buffer = readInFile(path);
            printf("Server Buffer: %s\n", buffer);
            int length = strlen(buffer);
            char size[10];
            printf("length: %d\n", length); //manifest 
            sprintf(size,"%d",length);
            send(socket,size,10,0);
            char temp[8];
            recv(socket,temp,8,0);
            send(socket,buffer,length,0);

            char commitFileSize[10];
            recv(socket, commitFileSize, 10, 0); //gets size of file as a char*
            
            int commitSize = atoi(commitFileSize); //converts char* into an integer 

            char* clientCommitFile = (char*)(malloc(sizeof(char)*commitSize));

            send(socket,"Got Size", 8 ,0); //sends confirmation that it got the size

            recv(socket,clientCommitFile,commitSize,0);//saves the commit file inside clientCommitFile

            printf("Client Commit:\n%s", clientCommitFile);

            char commitPath[strlen(projectName)+9];
            strcpy(commitPath,projectName);
            strcat(commitPath,"/");
            strcat(commitPath,".Commit");

            int fd = open(commitPath,O_RDWR|O_CREAT,0777);
            printf("%s\n",commitPath);
            printf("%d\n",commitSize);
            write(fd,clientCommitFile,commitSize);



        }
        
        

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

char* readInFile(char* fileName)
{
    char* buffer = malloc(sizeof(char) *1);
    char c;
    int fd = open(fileName,O_RDWR);
    int status;
    if (fd!=-1){
        do{
    
                status =  read(fd, &c, 1); 
                if (status<=0){
                    break;
                }
                else{   
                    int len = strlen(buffer);
                    buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
                    buffer[len] = c;
                    buffer[len+1] = '\0';    
                }
            }while(status >0);

            close(fd);
        return buffer; 
    }
    printf("Cannot open the file");
}


void destroyProject(int sock)
{
    
    char*projectName = (char*)(malloc(sizeof(char)*100));

    read(sock, projectName, 100);
    DIR *dr = opendir(projectName); 
    char* pathToDelete = malloc(sizeof(char) * 100);
    strcpy(pathToDelete, "rm -rf ");
    strcat(pathToDelete, projectName);
    //printf("Path to delete: %s\n", pathToDelete);   
    if (dr != NULL)
    {
        closedir(dr);
        printf("Successfully Destroyed\n");
        send(sock, "Successfully Deleted", 20, 0);
        system(pathToDelete);
        return;
    }
    else
    {
        closedir(dr);
        printf("Project Does not Exist\n");
        send(sock,"Project Does not Exist", 22, 0);
        return;
    }
}

void createProject(int sock){
    char*projectName = (char*)(malloc(sizeof(char)*100));
    read(sock, projectName, 100);
    printf("recieved project name: %s",projectName);
    char*filePath = (char*)(malloc(sizeof(char)*100));
    DIR *dr = opendir(projectName); 
    if (dr == NULL)  
    { 
        int check = mkdir(projectName,0777);     
        strcpy(filePath,projectName);
        strcat(filePath,"/");
        //strcat(filePath,projectName);
        strcat(filePath,".Manifest");
        printf("file Path: %s\n",filePath);
        int filedescriptor = open(filePath, O_RDWR | O_APPEND | O_CREAT,0777); 
        printf("fD %d\n",filedescriptor);
        char nL = '\n';
        char c = '1';
        write(filedescriptor,&c,1);
        write(filedescriptor,&nL,1);
      
        close(filedescriptor);

        char* response = malloc(sizeof(char) * 100);
        send(sock,filePath ,strlen(filePath),0);

        int recieve;
        recieve = recv(sock, response ,100,0);
        printf("Client Response: %s\n", response);

        printf("File Contents: %s\n", readInFile(filePath));
    //how do you know how much memory to allocate for this buffer 
        char* fileContents = (char*)malloc(sizeof(readInFile(filePath)));
        fileContents = readInFile(filePath);
        send(sock, fileContents, strlen(readInFile(filePath)), 0);
        closedir(dr);
        //close(check);
            
    } 
        else{
            write(sock,"PROJ_EXISTS",11);
            printf("\n**Project already Exists**\n");
        }


    //Now that we made a physical copy of a directory with the given project name on the server with a manifest
    //we are supposed to send that over to the client. How do we send it over? In what format?
}

void push(int sock)
{
    char*projectName = (char*)(malloc(sizeof(char)*100));
    int readSize = recv(sock, projectName, 100, 0);//gets project name from client 
    if (readSize > 0)
    {
        printf("Got requested path\n");
    }

    send(sock,"Got Project", 11 ,0);

     char path[strlen(projectName)+5+strlen(".Commit")];
            strcpy(path,projectName);
            strcat(path,"/");
            strcat(path,".Commit");

    char commitFileSize[10];
    recv(sock, commitFileSize, 10, 0); //gets size of file as a char*
            
    int commitSize = atoi(commitFileSize); //converts char* into an integer 

    char* clientCommitFile = (char*)(malloc(sizeof(char)*commitSize));

    send(sock,"Got Size", 8 ,0); //sends confirmation that it got the size

    recv(sock,clientCommitFile,commitSize,0);//saves the commit file inside clientCommitFile

    printf("Client Commit:\n%s", clientCommitFile);

    
    

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
        new_sock = malloc(sizeof(int));
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
	printf("In Server Handler\n");
	int sock = *(int *)fd_pointer;
    //char client_message[2000];
	static int send_once = 0;
	if (send_once < 1)
	{
	//write(sock,message,strlen(message));
	send_once++;
	}
    char* command = malloc(100 * sizeof(char));

    read(sock, command, 100);
    //recv(sock,command,2000,0);
    printf("recieved: %s\n", command);
    if (strcmp(command,"create")==0)
    {
        printf("got Command to create\n");
        char* replyCommand = "Got The Command to create";
        write(sock, replyCommand, strlen(replyCommand) + 1);
        createProject(sock);
    }
    if (strcmp(command, "destroy") == 0)
    {
        printf("got Command to destroy\n");
        char* replyCommand = "Got The Command to destroy";
        write(sock, replyCommand, strlen(replyCommand) + 1);
        destroyProject(sock);
    }
    if (strcmp(command, "getFiles") == 0)
    {
        printf("got Command to get files\n");
        char* replyCommand = "Got The Command to get files";
        write(sock, replyCommand, strlen(replyCommand) + 1);
        returnFiles(sock);
    }
    if (strcmp(command, "commit") == 0)
    {
        printf("got Command to commit\n");
        char* replyCommand = "Got The Command to commit";
        write(sock, replyCommand, strlen(replyCommand) + 1);
        
        commit(sock);
        
    }
    if (strcmp(command,"push")==0)
    {
        printf("got Command to push\n");
        char* replyCommand = "Got the Command to push";
        write(sock, replyCommand,strlen(replyCommand)+1);
        //lock
        //if (canPush(sock))
         push(sock);
         //unlock 
    }
    if (strcmp(command, "history") == 0)
    {
        printf("got Command to history\n");
        char* replyCommand = "Got The Command to get history";
        write(sock, replyCommand, strlen(replyCommand) + 1);  
        history(sock);
        
    }
    if (strcmp(command, "currentVersion") == 0)
    {
        printf("got Command for current version\n");
        char* replyCommand = "Got The Command for current version";
        write(sock, replyCommand, strlen(replyCommand) + 1);  
        currentVersion(sock);
        
    }
    if (strcmp(command, "checkout") == 0)
    {
        printf("got Command for checkout\n");
        char* replyCommand = "Got The Command for checkout";
        write(sock, replyCommand, strlen(replyCommand) + 1);  
        checkout(sock);
        
    }
    command = malloc (100 * sizeof(char));
   
}

