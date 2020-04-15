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
#include <openssl/sha.h>
#include <dirent.h>

char* readInFile(char* fileName);
typedef struct File{
    int version;
    char* filePath;
    char* hash;
    struct File* next;
}File;

typedef struct Manifest{
    int ProjectVersion;
    struct File* fileHead; 
}Manifest;

typedef struct ConfigureInfo{

    char*IP;
    int portNumber;
}ConfigureInfo;

ConfigureInfo info;
void insertFileNode(File **head, File *newNode)
{
    newNode->next = *head;
    *head = newNode;
}

File* createFileNode(int version, char* filePath, char* hash)
{
    struct File* temp = (struct File*)malloc(sizeof(File));
    temp->filePath = filePath;
    temp->version = version;
    temp->hash = hash;
    return temp;
}



void commit(char* projectName, int socket){

    int len = strlen(projectName)+1;
    send(socket,projectName,len,0);

    char* recieveSize = malloc (sizeof(char) * 10);

    recv(socket, recieveSize, 10, 0);
    int size = atoi(recieveSize);
    printf("Recieve size: %s\n", recieveSize);
    printf("Size of Server Manifest: %d\n",size);
    send(socket,"Got Size", 8 ,0);
    char*serverManifest =(char*)(malloc(sizeof(char)*size));
    recv(socket,serverManifest,size,0);
    printf("check: %s\n",serverManifest);

   
    Manifest* server;
    File* sHead;

   int i=0;
    char*buffer = (char*)malloc(sizeof(char)*1);
    while (serverManifest[i]!='\n'){
        int len = strlen(buffer);
        buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
        buffer[len] = serverManifest[i];
        buffer[len+1] = '\0';
        i++;
    }
    i++;
    
    //server->ProjectVersion = atoi(buffer);
   // printf("version: %d\n", server->ProjectVersion);
    int temp = atoi(buffer);
    printf("version check: %d\n", temp);
    int count = 0;
    int version;
    char*filePath;
    char*hash;
    buffer = (char*)malloc(sizeof(char)*1);
    while (i<strlen(serverManifest))
    {
        //printf("Char Check: %c\n", serverManifest[i]);
        if (serverManifest[i]==' '){
            
            if (count==0)
            {
                 version = atoi(buffer);
                 //printf("version check: %d\n", version);
                 buffer = malloc(sizeof(char) *1);
                 count++;
            }
            else if (count==1)
            {
               filePath = malloc(strlen(buffer));
               strcpy(filePath,buffer);
               memmove(filePath, filePath+1, strlen(filePath));
               //mem move gets ride of extra space at the beginning 
               //printf("FilePath: %s\n", filePath);
               buffer = malloc(sizeof(char) *1);
               count++;
            }
            else if (count==2)
            {
               hash = malloc(strlen(buffer));
               strcpy(hash,buffer);
               memmove(hash, hash+1, strlen(hash));
               //printf("hash: %s\n", hash);
               buffer = malloc(sizeof(char) *1);
               count++;
            }
            //printf("Count: %d\n", count);
            
            //buffer = (char*)malloc(sizeof(char)*1);
            
        }
        if (serverManifest[i]=='\n')
        {
            File* tempNode = createFileNode(version, filePath, hash);     
            insertFileNode(&sHead, tempNode);
            count = 0;
            i++;
        }
        else
        {
            int len = strlen(buffer);
            buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
            buffer[len] = serverManifest[i];
            buffer[len+1] = '\0'; 
            
            i++;
        }
        
    }
    //server->fileHead = sHead;
    count = 0;
    while (sHead != NULL && count < 3)
    {
        printf("Count: %d\n", count);
        count++;
        printf("%d\t",sHead->version);
        printf("%s\t",sHead->filePath);
        printf("%s\n",sHead->hash);
        sHead = sHead->next;
    }








    Manifest* client;
    File* cHead;

    char path[strlen(projectName)+5+strlen(".Manifest")];
        strcpy(path,projectName);
        strcat(path,"/");
        strcat(path,".Manifest");

    char* clientBuffer = readInFile(path);
    printf("ClientBuffer: \n%s", clientBuffer);
    
    i=0;
    buffer = (char*)malloc(sizeof(char)*1);
    while (serverManifest[i]!='\n'){
        int len = strlen(buffer);
        buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
        buffer[len] = serverManifest[i];
        buffer[len+1] = '\0';
        i++;
    }
    i++;
    
    //server->ProjectVersion = atoi(buffer);
   // printf("version: %d\n", server->ProjectVersion);
    printf("i: %d\n", i);
    count = 0;
    version;
    //char*filePath;
    //char*hash;
    buffer = (char*)malloc(sizeof(char)*1);
    while (i<strlen(clientBuffer))
    {
        //printf("Char Check: %c\n", serverManifest[i]);
        if (clientBuffer[i]==' '){
            
            if (count==0)
            {
                 version = atoi(buffer);
                 //printf("version check: %d\n", version);
                 buffer = malloc(sizeof(char) *1);
                 count++;
            }
            else if (count==1)
            {
               filePath = malloc(strlen(buffer));
               strcpy(filePath,buffer);
               memmove(filePath, filePath+1, strlen(filePath));
               //mem move gets ride of extra space at the beginning 
               //printf("FilePath: %s\n", filePath);
               buffer = malloc(sizeof(char) *1);
               count++;
            }
            else if (count==2)
            {
               hash = malloc(strlen(buffer));
               strcpy(hash,buffer);
               memmove(hash, hash+1, strlen(hash));
               //printf("hash: %s\n", hash);
               buffer = malloc(sizeof(char) *1);
               count++;
            }
            //printf("Count: %d\n", count);
            
            //buffer = (char*)malloc(sizeof(char)*1);
            
        }
        if (clientBuffer[i]=='\n')
        {
            File* tempNode = createFileNode(version, filePath, hash);     
            insertFileNode(&cHead, tempNode);
            count = 0;
            i++;
        }
        else
        {
            int len = strlen(buffer);
            buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
            buffer[len] = clientBuffer[i];
            buffer[len+1] = '\0'; 
            
            i++;
        }
        
    }

    count = 0;
    while (cHead != NULL && count < 3)
    {
        printf("Count: %d\n", count);
        count++;
        printf("%d\t",cHead->version);
        printf("%s\t",cHead->filePath);
        printf("%s\n",cHead->hash);
        cHead = cHead->next;
    }
    
    close(socket);
    
    return;
}


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
/*
void add(char*projectName, char*fileName)
{
       
        char nL = '\n';
        char x =  1+'0';
        int i = -1;
        struct dirent *de; 
        DIR *dr = opendir(projectName); 
        if (dr == NULL)  
        { 
            printf("Project does not Exist" ); 
            
        } 
        else{
         
            char path[strlen(projectName)+5+strlen(fileName)];
            strcpy(path,projectName);
            strcat(path,"/");
            strcat(path,fileName);
           
            int fd1 = open(path,O_RDONLY);
           
             char hash[SHA_DIGEST_LENGTH];
            if (fd1!=-1){
                int status;
                char c;
                char*buffer = (char*)malloc(sizeof(char)*1);
                do{
                     status = read(fd1,&c,1);
                    if (status<=0)
                        break;
                    int len = strlen(buffer);
                    buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
                    buffer[len] = c;
                    buffer[len+1] = '\0'; 
                }while(status>0); 
                SHA1(buffer, strlen(buffer), hash);
               
            }
            int len = strlen(projectName);
            char sp  = ' '; //subject to change if we have files and/dirs with spaces in them
            
             while ((de = readdir(dr)) != NULL) {
                 if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0){
                     if (strcmp(de->d_name,".Manifest")==0){
                         char manifestExtension[10];
                         strcpy(manifestExtension,".Manifest");
                        char manifestPath[strlen(projectName)+5+strlen(fileName)];
                        strcpy(manifestPath,projectName);
                        strcat(manifestPath,"/");
                        strcat(manifestPath,manifestExtension);
                         int fd = open(manifestPath,O_RDWR|O_APPEND);
                     
                         if (fd!=-1){
                             write(fd,&nL,1);
                             write(fd,&x,1);
                             write(fd,&sp,1);
                             write(fd,projectName,strlen(projectName));
                             write(fd,&sp,1);
                             write(fd,fileName,strlen(fileName));
                             write(fd,&sp,1);
                             write(fd,hash,strlen(hash));
                         }
                     }
                }
             }
               
        }

}
*/
void destroy(int socket, char* projectName)
{
    char* returnMessage = malloc (sizeof(char) * 50);
    int len = strlen(projectName)+1;
    send(socket,projectName,len,0);
    recv(socket, returnMessage, 50, 0);
    printf("%s\n", returnMessage);
}

void create(int socket, char* projectName){
   int len = strlen(projectName)+1;
   int check = mkdir(projectName,0777);

   send(socket,projectName,len,0);

   char* fileName = malloc(sizeof(char) * 100);

   int recieve = recv(socket,fileName,100,0);
   if (strcmp("PROJ_EXISTS", fileName) == 0)
   {
       printf("Project exists\n");
       return;
   }
   if (recieve > 0)
   {
       printf("Got File Name: %s\n", fileName);
       send(socket,"Got File Name",13 ,0);
   }
   else
   {
       printf("Did not get fileName\n");
       send(socket,"Did Not Recieve File Name", 26 ,0);
   }
   recieve = 0;
   
   int filedescriptor = open(fileName, O_RDWR | O_APPEND | O_CREAT,0777); 

   char* fileContents = malloc(sizeof(char) * 100);

   recieve = recv(socket,fileContents,100,0);

   if (recieve > 0)
   {
       printf("Got Contents: %s\n", fileContents);  
   }
   else
   {
       printf("Did not get fileName\n");   
   }

   write(filedescriptor, fileContents, strlen(fileContents));
   
close(filedescriptor);
}

void checkout(char*projectName){
   
  int socket =  connectToServer();


}
int getConfigureDetails(){

    char name[17] = "server.configure"; 
    int fd = open(name,O_RDONLY);
    
    if (fd==-1){
        
        return -1;
    }
  
    else{
        
       
        int status;
        char c;
        char*buffer = (char*)malloc(sizeof(char)*1);
        int i = 0;
        do{
            status = read(fd,&c,1);
            if (status<=0)
                break;
            if (c==' ')
            { 
                if (i==0){
                    info.IP = (char*)malloc( strlen(buffer)+1 );
                    strcpy(info.IP,buffer);
                   
                }
                i++;
                 buffer = (char*)(malloc(sizeof(char)*1));
                
            }
            else
            {
                int len = strlen(buffer);
                buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
                buffer[len] = c;
                buffer[len+1] = '\0'; 
            }
        }while(status>0);
       info.portNumber = atoi(buffer);
    }
    
    return 0;
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

}

char* readInFile(char* fileName)
{
    char* buffer = malloc(sizeof(char) *1);
    char c;
    int fd = open(fileName,O_RDWR);
    int status;

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
    return buffer; 
}

int getFiles(int sockfd, char* fileName)
{
       char* server_reply = malloc(sizeof(char) * 2000);
       char* buffer = malloc(sizeof(char) *1);
       printf("File Name: %s\n",fileName);
        int status;
        char c;
        int fd = open(fileName,O_RDWR);
        printf("fileDescriptor: %d\n", fd);

     

        do{
   
            status =  read(fd, &c, 1); 

            //printf("loop Check: %c\n", c);
        
            if (status<=0){
                //printf("status Check: %d\n", status);
                break;
            }
            if (c=='\n'){
                printf("buffer: %s\n", buffer);
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

        //close(sockfd);
}

int connectToServer(){
     getConfigureDetails();
      if (getConfigureDetails()!=-1){ 
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
        //printf("portNumber: %d\n", port1);
        servaddr.sin_port = htons(port1);
        servaddr.sin_addr.s_addr = inet_addr(info.IP);
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
   

   
    
        if (strcmp(argv[1],"configure")==0){
         host = argv[2];
         port1= argv[3];
         configure(host,port1);
        }
       
        if (strcmp(argv[1],"getFiles")==0){
            
           int sockfd = connectToServer(argv[1], argv[2]);
           char* getFile = "getFiles";
           write(sockfd, getFile, strlen(getFile) + 1);
           getFiles(sockfd, argv[2]);
           close(sockfd);
        }
        if (strcmp(argv[1],"create")==0){
            int socket =  connectToServer();
            char command[6] = "create";
            send(socket,command,6,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket, reply, 2000, 0);
            printf("Reply: %s\n", reply);
            create(socket, argv[2]);  
        }
        if (strcmp(argv[1],"destroy")==0){
            int socket =  connectToServer();
            char command[7] = "destroy";
            send(socket,command,7,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket, reply, 2000, 0);
            printf("Reply: %s\n", reply);
            destroy(socket, argv[2]);  
        }
        /*
        if (strcmp(argv[1],"add")==0)
        {
            add(argv[2],argv[3]);
        }
        */
        if (strcmp(argv[1],"commit")==0){
            int socket =  connectToServer();
            char command[6] = "commit";
            send(socket,command,6,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket, reply, 2000, 0);
            printf("Reply: %s\n", reply);
            commit(argv[2], socket);  
        }

       
        return 0;
}