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
#include <netdb.h> 


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

// Returns hostname for the local computer 
void checkHostName(int hostname) 
{ 
    if (hostname == -1) 
    { 
        perror("gethostname"); 
        exit(1); 
    } 
} 
  
// Returns host information corresponding to host name 
void checkHostEntry(struct hostent * hostentry) 
{ 
    if (hostentry == NULL) 
    { 
        perror("gethostbyname"); 
        exit(1); 
    } 
} 
  
// Converts space-delimited IPv4 addresses 
// to dotted-decimal format 
void checkIPbuffer(char *IPbuffer) 
{ 
    if (NULL == IPbuffer) 
    { 
        perror("inet_ntoa"); 
        exit(1); 
    } 
} 
void insertFileNode(File **head, File *newNode)
{
    newNode->next = *head;
    *head = newNode;
}
void deleteNode(File **head_ref, char* key) 
{ 
    printf("Key: %s\n",key);
    // Store head node 
    File* temp = *head_ref, *prev; 
  
    // If head node itself holds the key to be deleted 
    if (temp != NULL &&(strcmp(key,temp->filePath)==0)) 
    { 
        *head_ref = temp->next;   // Changed head 
        free(temp);               // free old head 
        return; 
    } 
  
    // Search for the key to be deleted, keep track of the 
    // previous node as we need to change 'prev->next' 
    while (temp != NULL && (strcmp(key,temp->filePath)!=0)) 
    { 
        prev = temp; 
        temp = temp->next; 
    } 
  
    // If key was not present in linked list 
    if (temp == NULL) {
        printf("Specified File does not Exist!\n");
    return; 
    }
  
    // Unlink the node from linked list 
    prev->next = temp->next; 
  
    free(temp);  // Free memory 
} 
 void reverse(File** head_ref) 
{ 
    File* prev = NULL; 
    File* current = *head_ref; 
    File* next = NULL; 
    while (current != NULL) { 
        // Store next 
        next = current->next; 
  
        // Reverse current node's pointer 
        current->next = prev; 
  
        // Move pointers one position ahead. 
        prev = current; 
        current = next; 
    } 
    *head_ref = prev; 
} 
File* createFileNode(int version, char* filePath, char* hash)
{
    struct File* temp = (struct File*)malloc(sizeof(File));
    temp->filePath = filePath;
    temp->version = version;
    temp->hash = hash;
    return temp;
}


char* computeHash(char* path)
{
    char hash[SHA_DIGEST_LENGTH];
    char* buffer = readInFile(path);
    SHA1(buffer, strlen(buffer), hash);
    char* test = malloc(sizeof(char) * 40);
    int j = 0;
    while(j < 20)
    {
        sprintf((char*)&(test[j * 2]), "%02x", hash[j]);
        j++;
    }
    return test;

}

void writeCommit(int fd,int version,char*filePath, char*hash,char command){
    char sp = ' ';
    char nL = '\n';
   char versionBuff[20];
    write(fd,&command,1);
    write(fd,&sp,1);
    sprintf(versionBuff,"%d",version);
    write(fd,versionBuff,strlen(versionBuff));
    write(fd,&sp,1);
    write(fd,filePath,strlen(filePath));
    write(fd,&sp,1);
    write(fd,hash,strlen(hash));
    write(fd,&sp,1);
    write(fd,&nL,1);

}
void writeUpdate(int fd,int version,char*filePath, char*hash,char command){
    char sp = ' ';
    char nL = '\n';
   char versionBuff[20];
   write(fd,&command,1);
   write(fd,&sp,1);
    sprintf(versionBuff,"%d",version);
   write(fd,versionBuff,strlen(versionBuff));
   write(fd,&sp,1);
   write(fd,filePath,strlen(filePath));
   write(fd,&sp,1);
   write(fd,hash,strlen(hash));
   write(fd,&sp,1);
   write(fd,&nL,1);

}

void commitFile(Manifest client, int cNodeLength ,Manifest server, int sNodeLength, char* projectName,int socket)
{
        char hostbuffer[256]; 
        char *IPbuffer = malloc(16); 
        struct hostent *host_entry; 
        int hostname; 
    
        // To retrieve hostname 
        hostname = gethostname(hostbuffer, sizeof(hostbuffer)); 
        checkHostName(hostname); 
    
        // To retrieve host information 
        host_entry = gethostbyname(hostbuffer); 
        checkHostEntry(host_entry); 
    
        // To convert an Internet network 
        // address into ASCII string 
        IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])); 
    
        printf("Hostname: %s\n", hostbuffer); 
        printf("Host IP: %s\n", IPbuffer);

     
   
        
        char* commitFileName = malloc((strlen(projectName) + 10 + strlen(IPbuffer)) * sizeof(char));
        strcpy(commitFileName, projectName);
        char commiteExt[10] = "/.Commit_";
        strcat(commitFileName, commiteExt);
        strcat(commitFileName, IPbuffer);
        printf("%s\n",commitFileName);   
        if (client.ProjectVersion != server.ProjectVersion)
        {
            printf("Update Local Project\n");
            send(socket,"Stop\0",5,0);
            return;
        }
        int commitFD = open(commitFileName,O_RDWR|O_APPEND);
    if (commitFD!=-1){
        printf("*Overwriting Commit File**\n");
    }
     commitFD = open(commitFileName,O_RDWR|O_APPEND|O_CREAT|O_TRUNC,0777);    

        File* cheadTemp = client.fileHead;
        File* sheadTemp = server.fileHead;
        File* cheadTemp2 = client.fileHead;
        File* sheadTemp2 = server.fileHead;
    
     //printf("commitFd: %d\n",commitFD);
        if ((sheadTemp==NULL) &&(cheadTemp==NULL)){
            printf("perfect case, No commits to be made!");
        }
        else if ((sheadTemp==NULL) &&(cheadTemp!=NULL)){
            while (cheadTemp!=NULL){
                printf("reached!*!\n");
                printf("A %d %s %s\n",cheadTemp->version,cheadTemp->filePath,cheadTemp->hash);
                writeCommit(commitFD,cheadTemp->version,cheadTemp->filePath,cheadTemp->hash,'A');
                cheadTemp = cheadTemp->next;
            }
        }
        else if ((sheadTemp!=NULL) &&(cheadTemp==NULL))
        {
            printf("reached!*!\n");
           while (sheadTemp!=NULL){
             printf("D %d %s %s\n",sheadTemp->version,sheadTemp->filePath,sheadTemp->hash);
            writeCommit(commitFD,sheadTemp->version,sheadTemp->filePath,sheadTemp->hash,'D');
            sheadTemp = sheadTemp->next;
           }
        }

        else
        {
           bool checkTest = false;
        int addCheck = 0;
        int count1 = 0;
        int loopCheck = 0;
        //printf("%d\t%d\n",cNodeLength,sNodeLength);
        while(cheadTemp != NULL)//iterates thrr client nodes 
        {
           checkTest = false;

            sheadTemp = server.fileHead;
            while (sheadTemp != NULL)//iterates thru sever nodes 
            {
               
                loopCheck++;
                if (strcmp(cheadTemp->filePath, sheadTemp->filePath) == 0)//compares client file name server file name
                {   
                    
                    if (strcmp(cheadTemp->hash, sheadTemp->hash) == 0)//compares client hash and server hash if file names are equal 
                    {
                        char* liveHash;
                        liveHash = computeHash(cheadTemp->filePath);

                        if (strcmp(liveHash, cheadTemp->hash) == 0)//if file and hash are equal, checks live hash vs client hash 
                        {
                            printf("equal match for: %s\n",cheadTemp->filePath);
                           //if everything is the same
                        }         
                        else
                        {
                          //modify code
                          
                          printf("M %d %s %s\n",++cheadTemp->version,cheadTemp->filePath,cheadTemp->hash); 
                         
                          writeCommit(commitFD,cheadTemp->version,cheadTemp->filePath,liveHash,'M');//if livehash is not the same as client hash 
                        }
                    }  
                    else if  (! (sheadTemp->version < cheadTemp->version )) //if file paths are the same, hashes are different
                    {
                        printf("**Synch projects first!**\n");
                        remove(commitFileName);
                       return; 
                        //delete .commit file if it exists
                       
                    }
                     checkTest = true;
                }  
               
                else if ((sheadTemp->next==NULL)&&(checkTest==false))
                {
                // printf("%s\n%s\n",cheadTemp->filePath,sheadTemp->filePath);
                
                   printf("A %d %s %s\n",cheadTemp->version,cheadTemp->filePath,cheadTemp->hash);
                   writeCommit(commitFD,cheadTemp->version,cheadTemp->filePath,cheadTemp->hash,'A');
                   checkTest = true;
                   
                 }
                
                sheadTemp = sheadTemp->next;
            }
           
            cheadTemp = cheadTemp->next;
            

            count1++;
        }
      
        addCheck = 0;
        count1 = 0;
       checkTest = false;
        while (sheadTemp2 != NULL)
        {
            int count2 = 0;
            cheadTemp2 = client.fileHead;
             checkTest = false;
            while (cheadTemp2 != NULL && count2 < cNodeLength)//iterates thru client nodes 
            {
               
                if (strcmp(cheadTemp2->filePath, sheadTemp2->filePath) == 0) //check server name and client name 
                {
                   checkTest=true;
                }
            
               else if  ((cheadTemp2->next==NULL)&&(checkTest==false)){
                    printf("D %d %s %s\n",sheadTemp2->version,sheadTemp2->filePath,sheadTemp2->hash);
                    writeCommit(commitFD,sheadTemp2->version,sheadTemp2->filePath,sheadTemp2->hash,'D');
                 }
               
                cheadTemp2 = cheadTemp2->next;
            }
            count1++;
            sheadTemp2 = sheadTemp2->next;
        }

        }


        close(commitFD);

        char temp[8];

        int Namelength = strlen(commitFileName); //gets length of file name 
        char NameSize[10];
        sprintf(NameSize,"%d",Namelength); //converts int to char*
        printf("client Size: %d\n", Namelength);
        send(socket, NameSize ,strlen(NameSize), 0); //sends size of file name

        recv(socket,temp,8,0); //recieved confirmation



        send(socket,commitFileName ,strlen(commitFileName)+1, 0); //sends file name 

        recv(socket,temp,8,0); //recieves confirmation

        printf("\n");

       int fd = open(commitFileName,O_RDONLY);
       char c;
       int status = 0;
       char buffer[1000000];
      
       int counter = 0;
       do{
           status = read(fd,&c,1);
           if (status<=0)
            break;
          buffer[counter] = c;
          counter++;

       }while (status>0);
            
  

    char*commitBuffer = &buffer[0];
   
   close(fd);
       
        printf("reached\n");
      

     
        int commitSize = strlen(commitBuffer);
      

        int length = commitSize;
        char size[10];
        sprintf(size,"%d",commitSize);
             //converts the size into a char* to send over to the server
       
        send(socket, size ,strlen(size), 0); //sends size of file

       
        recv(socket,temp,8,0);//gets confirmation from server that it got the size 
       
        send(socket,commitBuffer ,commitSize, 0); //sends the commit buffer using the size of it stores in size */

       
}
void updateFile(Manifest client, Manifest server, char* projectName,int socket)
{
        
            char* updateFileName = malloc((strlen(projectName) + 9) * sizeof(char));
            strcpy(updateFileName, projectName);
            char updateExt[9] = "/.Update";
            strcat(updateFileName, updateExt);
           
   
         int updateFD = open(updateFileName,O_RDWR|O_APPEND);
        if (client.ProjectVersion == server.ProjectVersion)
        {
            printf("Full Success: NO Updates\n");
            return;
        }
       
        File* cheadTemp = client.fileHead;
        File* sheadTemp = server.fileHead;
        File* cheadTemp2 = client.fileHead;
        File* sheadTemp2 = server.fileHead;
        
       
       
    if (updateFD!=-1){
        printf("*Overwriting Update File**\n");
    }
     updateFD = open(updateFileName,O_RDWR|O_APPEND|O_CREAT|O_TRUNC,0777);
    
     //printf("commitFd: %d\n",commitFD);
        if ((sheadTemp==NULL) &&(cheadTemp==NULL)){
            printf("No updates need to be made\n");
           
        }
        else if ((sheadTemp==NULL) &&(cheadTemp!=NULL)){
            while (cheadTemp!=NULL){
                printf("D %d %s %s\n",cheadTemp->version,cheadTemp->filePath,cheadTemp->hash);
                writeCommit(updateFD,cheadTemp->version,cheadTemp->filePath,cheadTemp->hash,'D');
                cheadTemp = cheadTemp->next;
            }
        }
        else if ((sheadTemp!=NULL) &&(cheadTemp==NULL))
        {
           while (sheadTemp!=NULL){
             printf("A %d %s %s\n",sheadTemp->version,sheadTemp->filePath,sheadTemp->hash);
            writeCommit(updateFD,sheadTemp->version,sheadTemp->filePath,sheadTemp->hash,'A');
            sheadTemp = sheadTemp->next;
           }
        }

        else
        {
          
           
        int addCheck = 0;
        int count1 = 0;
        bool checkTest = false;
        //printf("%d\t%d\n",cNodeLength,sNodeLength);
        while(cheadTemp != NULL)//iterates thrr client nodes 
        {
           checkTest = false;
            int count2 = 0;
            sheadTemp = server.fileHead;
            while (sheadTemp != NULL)//iterates thru sever nodes 
            {

                if (strcmp(cheadTemp->filePath, sheadTemp->filePath) == 0)//compares client file name server file name
                {   
                    
                  if ((strcmp(cheadTemp->hash, sheadTemp->hash) != 0)&&(cheadTemp->version!=sheadTemp->version)) //check to see if stored hash is the same but file version is different
                  {
                        char* liveHash = malloc(sizeof(char) * 40);
                        liveHash = computeHash(cheadTemp->filePath);
                        if (strcmp(cheadTemp->hash,liveHash)==0){ //check to see if live hash of client file is different than stored hash
                         //modify code
                          printf("M %d %s %s\n",sheadTemp->version,sheadTemp->filePath,sheadTemp->hash); //if live hash of client file is different than stored hash adds a modify line 
                         
                          writeUpdate(updateFD,sheadTemp->version,sheadTemp->filePath,sheadTemp->hash,'M');
                         //write Update file
                        }
                        else
                        {
                              //conflict case
                              printf("C %s\n",cheadTemp->filePath); //if live hash of client file is different than stored hash adds a modify line 
                              //write conflict file
                        }

                  }       
                 checkTest = true;
                }  
               
                 if ((sheadTemp->next==NULL)&&(checkTest==false)){
                   printf("D %d %s %s\n",cheadTemp->version,cheadTemp->filePath,cheadTemp->hash);
                   writeUpdate(updateFD,cheadTemp->version,cheadTemp->filePath,cheadTemp->hash,'D');
                   checkTest = true;
                 }
                 addCheck = 0;
                sheadTemp = sheadTemp->next;
            }
           
            cheadTemp = cheadTemp->next;
            
        }
        checkTest = false;
        addCheck = 0;
        count1 = 0;
        while (sheadTemp2 != NULL)
        {
            int count2 = 0;
            cheadTemp2 = client.fileHead;
            checkTest = false;
            while (cheadTemp2 != NULL)//iterates thru client nodes 
            {
               
                if (strcmp(cheadTemp2->filePath, sheadTemp2->filePath) == 0) //check server name and client name 
                {
                  
                    checkTest = true;
                }
                
               // printf("%d\n",addCheck);
                if ((cheadTemp2->next==NULL)&&(addCheck==0)){
                    printf("A %d %s %s\n",sheadTemp2->version,sheadTemp2->filePath,sheadTemp2->hash);
                    writeUpdate(updateFD,sheadTemp2->version,sheadTemp2->filePath,sheadTemp2->hash,'A');

                 }
                 addCheck = 0;
                cheadTemp2 = cheadTemp2->next;
            }
            sheadTemp2 = sheadTemp2->next;
        }

        }


        close(updateFD);

      /*  char* updateBuffer = (char*)(malloc(sizeof(char)* strlen(readInFile(updateFileName))));
        
        updateBuffer = readInFile(updateFileName); //gets commit file size
        int updateSize = strlen(updateBuffer);
        //printf("%s\n",commitBuffer);

        int length = updateSize;
        char size[10];
        sprintf(size,"%d",updateSize);
             //converts the size into a char* to send over to the server
       
        send(socket, size ,strlen(size), 0); //sends size of file

        char temp[8];
        recv(socket,temp,8,0);//gets confirmation from server that it got the size 
       
        send(socket,updateBuffer ,updateSize, 0); //sends the commit buffer using the size of it stores in size 
        */

        
    
        
}

void history(char * projectName, int socket)
{
    int len = strlen(projectName)+1; //gets size projectName string length 
    send(socket,projectName,len,0); //sends over project name to server 

    char* recieveSize = malloc (sizeof(char) * 10);

    recv(socket, recieveSize, 10, 0); //gets size of the buffer of the history file 
    if (strcmp(recieveSize, "DNE") == 0)
    {
        printf("Folder / File does not exist\n"); //check to see if folder exists on server side, if not it stops
        return;
    }
    int size = atoi(recieveSize);
    send(socket,"Got Size", 8 ,0); //sends a confirmation that it got the size 
    char* historyBuffer =(char*)(malloc(sizeof(char)*size));
    recv(socket, historyBuffer ,size,0); //gets buffer containing the file

    printf("History: \n%s\n", historyBuffer);



}

void currentVersion(char* projectName, int socket)
{
    int len = strlen(projectName)+1; //gets size projectName string length 
    send(socket,projectName,len,0); //sends over project name to server 

    char* recieveSize = malloc (sizeof(char) * 10);

    recv(socket, recieveSize, 10, 0); //gets size of the buffer of the manifest file, which holds the current version info  
    if (strcmp(recieveSize, "DNE") == 0)
    {
        printf("Folder does not exist\n"); //check to see if folder exists on server side, if not it stops
        return;
    }
    int size = atoi(recieveSize);
    send(socket,"Got Size", 8 ,0); //sends a confirmation that it got the size 
    char* manifestVersion =(char*)(malloc(sizeof(char)*size));
    recv(socket, manifestVersion ,size,0); //gets buffer containing the file

    printf("Current Version: \n%s\n", manifestVersion);
}

File* tokenizeClientManifest(File*cHead,char*clientBuffer){
    int i=0;
   char* buffer = (char*)malloc(sizeof(char)*1);
   buffer[0] = '\0';
    while (clientBuffer[i]!='\n'){
        int len = strlen(buffer);
        buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
        buffer[len] = clientBuffer[i];
        buffer[len+1] = '\0';
        i++;
    }
    i++;
    int projVersion = atoi(buffer);
   
   int count = 0;
   int version;
   char*hash;
   char*filePath;
    //char*filePath;
    //char*hash;
    int cNodeLength = 0;
    buffer = (char*)malloc(sizeof(char)*1);
    buffer[0] = '\0';

    while (i<strlen(clientBuffer))
    {
        if (clientBuffer[i]==' '){
            
            if (count==0)
            {
                 version = atoi(buffer);
                 //printf("version check: %d\n", version);
                 buffer = malloc(sizeof(char) *1);
                 buffer[0] = '\0';
                 count++;
            }
            else if (count==1)
            {
               filePath = malloc(strlen(buffer)+1);
               strcpy(filePath,buffer);
               memmove(filePath, filePath+1, strlen(filePath));
               //mem move gets ride of extra space at the beginning 
               //printf("FilePath: %s\n", filePath);
               buffer = malloc(sizeof(char) *1);
               buffer[0] = '\0';
               count++;
            }
            else if (count==2)
            {
               hash = malloc(strlen(buffer)+1);
               strcpy(hash,buffer);
               memmove(hash, hash+1, strlen(hash));
               //printf("hash: %s\n", hash);
               buffer = malloc(sizeof(char) *1);
               buffer[0] = '\0';
               count++;
            }
            //printf("Count: %d\n", count);
            
            //buffer = (char*)malloc(sizeof(char)*1);
            
        }
        if (clientBuffer[i]=='\n')
        {
          
            File* tempNode = createFileNode(version, filePath, hash);     
            insertFileNode(&cHead, tempNode);
            cNodeLength++;
            count = 0;
           buffer = malloc(sizeof(char) *1);
                buffer[0] = '\0';
           
        }
        else
        {
            int len = strlen(buffer);
            buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
            buffer[len] = clientBuffer[i];
            buffer[len+1] = '\0'; 
        }
        i++;
        
    }
    return cHead;
}


void update(char* projectName, int socket){

    int len = strlen(projectName)+1; //allocates mem for project name
    send(socket,projectName,len,0); //sends project name

    char* recieveSize = malloc (sizeof(char) * 10); //allocates size of "size" of project manifest buffer 

    recv(socket, recieveSize, 10, 0); //gets size of buffer 
    if (strcmp(recieveSize, "DNE") == 0)
    {
        printf("Server does not have project\n");
        return;
    }
    int size = atoi(recieveSize); //turns char* into an int
    send(socket,"Got Size", 8 ,0);
    char*serverManifest =(char*)(malloc(sizeof(char)*size)); //sends cofirmation it got the size 
    recv(socket,serverManifest,size,0); //gets full manifest buffer from server and stores it in serverManifest
   // printf("check:\n%s\n",serverManifest);

   
    Manifest server;
    Manifest client;
    File* sHead = NULL;
    File* cHead = NULL;
    

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


    
    server.ProjectVersion = atoi(buffer);
   // printf("version: %d\n", server->ProjectVersion);
    int temp = atoi(buffer);
    //printf("version check: %d\n", temp);
    int count = 0;
    int version;
    char*filePath;
    char*hash;
    buffer = (char*)malloc(sizeof(char)*1);
    int SNodeLength = 0;
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
               filePath = malloc(strlen(buffer)+1);
               strcpy(filePath,buffer);
               memmove(filePath, filePath+1, strlen(filePath));
               //mem move gets ride of extra space at the beginning 
               //printf("FilePath: %s\n", filePath);
               buffer = malloc(sizeof(char) *1);
               count++;
            }
            else if (count==2)
            {
               hash = malloc(strlen(buffer)+1);
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
         //  printf("%d\t%s\t%s",version,filePath,hash);
            File* tempNode = createFileNode(version, filePath, hash);     
            insertFileNode(&sHead, tempNode);
            SNodeLength++;
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
    File* tempShead = sHead;
   // printf("\n");
  /*  while (tempShead != NULL)
    {
        printf("%d\t",tempShead->version);
        printf("%s\t",tempShead->filePath);
        printf("%s\n",tempShead->hash);
        tempShead = tempShead->next;
    }*/
    


    char path[strlen(projectName)+5+strlen(".Manifest")];
        strcpy(path,projectName);
        strcat(path,"/");
        strcat(path,".Manifest");

    char* clientBuffer = readInFile(path);
    //printf("Client Length: %d\n", strlen(clientBuffer));
    //printf("ClientBuffer: \n%s", clientBuffer);
    
    i=0;
    buffer = (char*)malloc(sizeof(char)*1);
    while (clientBuffer[i]!='\n'){
        int len = strlen(buffer);
        buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
        buffer[len] = clientBuffer[i];
        buffer[len+1] = '\0';
        i++;
    }
    i++;
    
    client.ProjectVersion = atoi(buffer); //might have to allocate space for the Manifest struct
   // printf("version: %d\n", server->ProjectVersion);
    //printf("i: %d\n", i);
    count = 0;
    version;
    //char*filePath;
    //char*hash;
    int cNodeLength = 0;
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
               filePath = malloc(strlen(buffer)+1);
               strcpy(filePath,buffer);
               memmove(filePath, filePath+1, strlen(filePath));
               //mem move gets ride of extra space at the beginning 
               //printf("FilePath: %s\n", filePath);
               buffer = malloc(sizeof(char) *1);
               count++;
            }
            else if (count==2)
            {
               hash = malloc(strlen(buffer)+1);
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
            cNodeLength++;
            count = 0;
           
        }
        else
        {
            int len = strlen(buffer);
            buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
            buffer[len] = clientBuffer[i];
            buffer[len+1] = '\0'; 
            
           
        }
        i++;
        
    }

    count = 0;
    File* tempChead = cHead;
    
   /* while (tempChead != NULL)
    {
        //printf("Client\n");
        printf("%d\t",tempChead->version);
        printf("%s\t",tempChead->filePath);
        printf("%s\n",tempChead->hash);
        tempChead = tempChead->next;
        count++;
    }*/
    
    client.fileHead = cHead;
    server.fileHead = sHead;
    //printf("Project Version: %d\n", server.ProjectVersion);
    

    
    

    updateFile(client, server, projectName,socket);

    return;
   
}

void commit(char* projectName, int socket){

    int len = strlen(projectName)+1; //allocates mem for project name
    send(socket,projectName,len,0); //sends project name

    char* recieveSize = malloc (sizeof(char) * 10); //allocates size of "size" of project manifest buffer 

    recv(socket, recieveSize, 10, 0); //gets size of buffer 
    if (strcmp(recieveSize, "DNE") == 0)
    {
        printf("Server does not have project\n");
        return;
    }
    int size = atoi(recieveSize); //turns char* into an int
    send(socket,"Got Size", 8 ,0);
    char*serverManifest =(char*)(malloc(sizeof(char)*size)); //sends cofirmation it got the size 
    recv(socket,serverManifest,size,0); //gets full manifest buffer from server and stores it in serverManifest
   // printf("check:\n%s\n",serverManifest);

   
    Manifest server;
    Manifest client;
    File* sHead = NULL;
    File* cHead = NULL;
    

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


    
    server.ProjectVersion = atoi(buffer);
   // printf("version: %d\n", server->ProjectVersion);
    int temp = atoi(buffer);
    //printf("version check: %d\n", temp);
    int count = 0;
    int version;
    char*filePath;
    char*hash;
    buffer = (char*)malloc(sizeof(char)*1);
    int SNodeLength = 0;
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
               filePath = malloc(strlen(buffer)+1);
               strcpy(filePath,buffer);
               memmove(filePath, filePath+1, strlen(filePath));
               //mem move gets ride of extra space at the beginning 
               //printf("FilePath: %s\n", filePath);
               buffer = malloc(sizeof(char) *1);
               count++;
            }
            else if (count==2)
            {
               hash = malloc(strlen(buffer)+1);
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
         //  printf("%d\t%s\t%s",version,filePath,hash);
            File* tempNode = createFileNode(version, filePath, hash);     
            insertFileNode(&sHead, tempNode);
            SNodeLength++;
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
    File* tempShead = sHead;
   // printf("\n");
  /*  while (tempShead != NULL)
    {
        printf("%d\t",tempShead->version);
        printf("%s\t",tempShead->filePath);
        printf("%s\n",tempShead->hash);
        tempShead = tempShead->next;
    }*/
    


    char path[strlen(projectName)+5+strlen(".Manifest")];
        strcpy(path,projectName);
        strcat(path,"/");
        strcat(path,".Manifest");

    char* clientBuffer = readInFile(path);
    //printf("Client Length: %d\n", strlen(clientBuffer));
    //printf("ClientBuffer: \n%s", clientBuffer);
    
    i=0;
    buffer = (char*)malloc(sizeof(char)*1);
    while (clientBuffer[i]!='\n'){
        int len = strlen(buffer);
        buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
        buffer[len] = clientBuffer[i];
        buffer[len+1] = '\0';
        i++;
    }
    i++;
    
    client.ProjectVersion = atoi(buffer); //might have to allocate space for the Manifest struct
   // printf("version: %d\n", server->ProjectVersion);
    //printf("i: %d\n", i);
    count = 0;
    version;
    //char*filePath;
    //char*hash;
    int cNodeLength = 0;
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
               filePath = malloc(strlen(buffer)+1);
               strcpy(filePath,buffer);
               memmove(filePath, filePath+1, strlen(filePath));
               //mem move gets ride of extra space at the beginning 
               //printf("FilePath: %s\n", filePath);
               buffer = malloc(sizeof(char) *1);
               count++;
            }
            else if (count==2)
            {
               hash = malloc(strlen(buffer)+1);
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
            cNodeLength++;
            count = 0;
           
        }
        else
        {
            int len = strlen(buffer);
            buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
            buffer[len] = clientBuffer[i];
            buffer[len+1] = '\0'; 
            
           
        }
        i++;
        
    }

    count = 0;
    File* tempChead = cHead;
    
   /* while (tempChead != NULL)
    {
        //printf("Client\n");
        printf("%d\t",tempChead->version);
        printf("%s\t",tempChead->filePath);
        printf("%s\n",tempChead->hash);
        tempChead = tempChead->next;
        count++;
    }*/
    
    client.fileHead = cHead;
    server.fileHead = sHead;
    //printf("Project Version: %d\n", server.ProjectVersion);
    

    
    

    commitFile(client, cNodeLength , server, SNodeLength , projectName,socket);

    return;
   
}


void push(char*projectName,int socket)
{

        send(socket, projectName, strlen(projectName) + 1, 0); //sends project name
        char* projectReply = malloc (sizeof(char) * 9);
        recv(socket, projectReply, 9, 0);
        printf("%s\n", projectReply);



   
        char hostbuffer[256]; 
        char *IPbuffer = malloc(16); 
        struct hostent *host_entry; 
        int hostname; 
    
        // To retrieve hostname 
        hostname = gethostname(hostbuffer, sizeof(hostbuffer)); 
        checkHostName(hostname); 
    
        // To retrieve host information 
        host_entry = gethostbyname(hostbuffer); 
        checkHostEntry(host_entry); 
    
        // To convert an Internet network 
        // address into ASCII string 
        IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])); 
    
        printf("Hostname: %s\n", hostbuffer); 
        printf("Host IP: %s\n", IPbuffer);

     
   
        
        char* commitFileName = malloc((strlen(projectName) + 10 + strlen(IPbuffer)) * sizeof(char)); //mallocs size for file name 
        strcpy(commitFileName, projectName);
        char commiteExt[10] = "/.Commit_";
        strcat(commitFileName, commiteExt);
        strcat(commitFileName, IPbuffer);
        printf("%s\n",commitFileName);   //creates project name

        char temp[8];

        int Namelength = strlen(commitFileName); //gets length of file name 
        char NameSize[10];
        sprintf(NameSize,"%d",Namelength); //converts int to char*

        send(socket, NameSize ,strlen(NameSize), 0); //sends size of commit file name

        recv(socket,temp,8,0); //recieved confirmation server got commit file name size
   

    
    send(socket,commitFileName,strlen(commitFileName),0);//sends commit file name

    char* confirmation = malloc (sizeof(char) * 12);
    recv(socket, confirmation, 12 ,0); //gets confirmation it got the commit file name
    printf("%s\n", confirmation);

    
    /*
    char commitFileName[strlen(projectName)+10];
    strcpy(commitFileName,projectName);
    strcat(commitFileName,"/");
    strcat(commitFileName,".Commit");
    */
    

   int fd = open(commitFileName,O_RDONLY);
   if (fd!=-1){
      char c;
       int status = 0;
       char buffer[1000000];
      
       int counter = 0;
       do{
           status = read(fd,&c,1);
           if (status<=0)
            break;
          buffer[counter] = c;
          counter++;

       }while (status>0);
        char*commitBuffer = &buffer[0];
         close(fd);
        
        int length = strlen(commitBuffer);
        printf("%d\n",length);
        char size[10];
        sprintf(size,"%d",length);
             //converts the size into a char* to send over to the server
       
        send(socket, size ,strlen(size), 0); //sends size of file

        char temp[8];
        recv(socket,temp,8,0);//gets confirmation from server that it got the size 
       
        send(socket,commitBuffer ,length, 0); //sends the commit buffer using the size of it stores in size 
   }

  
      
            
  

   
  
  //active commit? does that mean commit file per project on the server? or just one total commit file at a time? 
   
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
            if (fd1!=-1){

            char hash[SHA_DIGEST_LENGTH];
            char hexHash[SHA_DIGEST_LENGTH];
            printf("shaLength: %d\n", SHA_DIGEST_LENGTH);
            /*
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
            }
            */

            char* buffer = readInFile(path);
            SHA1(buffer, strlen(buffer), hash);
            
            char test[40];
            int j = 0;
            while(j < 20)
            {
                sprintf((char*)&(test[j * 2]), "%02x", hash[j]);
                j++;
            }
            printf("testBuffer: %s\n", test);
            
            
            
           
            //sprintf(hexHash,"%x",hash);
           
               
            
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
                             write(fd,&x,1);
                             write(fd,&sp,1);
                             write(fd,path,strlen(path));
                             write(fd,&sp,1);
                             write(fd,test,strlen(test));
                             printf("length: %d\n", strlen(test));
                             write(fd,&sp,1); 
                            write(fd,&nL,1);
                         }
                     }
                }
             }
               
        }
        else
        {
            printf("**File is not in the Project**\n");
        }
        
        }

}
void removeFile(char*projectName,char*fileName)
{
     DIR *dr = opendir(projectName);   //open directory for projectName
        if (dr == NULL)  
        { 
            printf("Project does not Exist" ); 
            return;
        } 
       
            char manifest[strlen(projectName)+10];  //
            strcpy(manifest,projectName);
            strcat(manifest,"/");
            strcat(manifest,".Manifest");
            char targetFilePath[strlen(projectName)+strlen(fileName)+1];
            strcpy(targetFilePath,projectName);
            strcat(targetFilePath,"/");
            strcat(targetFilePath,fileName);
            int fd = open(manifest,O_RDWR);
            if (fd!=-1){
                
                bool dne =true;
                int projectVersion;
                char*clientManifest = readInFile(manifest);
                int i=0;
                char*buffer = (char*)(malloc(sizeof(char)*1));
                 while (clientManifest[i]!='\n'){
                    int len = strlen(buffer);
                    buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
                    buffer[len] = clientManifest[i];
                    buffer[len+1] = '\0';
                    i++;
                }
                 i++;
                int projVersion = atoi(buffer);
                 buffer = (char*)(malloc(sizeof(char)*1));
                File* cHead = NULL;

                cHead = tokenizeClientManifest(cHead,clientManifest);
            
                if (cHead==NULL){
                    printf("**Empty Manifest File!**\n");
                    return;
                }   

                reverse(&cHead);
                File* temp = cHead;
                
                deleteNode(&cHead,targetFilePath); 
               /* while(temp!=NULL){
                    printf("%d\t%s\t%s\n",temp->version,temp->filePath,temp->hash);
                    temp = temp->next;
                }*/
                close(fd);

                int newOpen = open(manifest,O_RDWR|O_TRUNC|O_APPEND);
                char sP = ' ';
                char nL = '\n';
                char v1c[20];
                sprintf(v1c,"%d",projVersion); //might have to change manifest Version

                write(newOpen,v1c,strlen(v1c));
                write(newOpen,&nL,1);
                while (cHead!=NULL){
                    char versionBuff[20];
                    sprintf(versionBuff,"%d",cHead->version);
                    write(fd,versionBuff,strlen(versionBuff));
                    write(fd,&sP,1);
                    write(fd,cHead->filePath,strlen(cHead->filePath));
                    write(fd,&sP,1);
                    write(fd,cHead->hash,strlen(cHead->hash));
                    write(fd,&sP,1);
                    write(fd,&nL,1);
                    cHead = cHead->next;
                }

                close(newOpen);

            }
            else
            {
                printf("**Manifest file does not Exist!**\n");
            }
            
            
        
        
}
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

   char* fileContents = malloc(sizeof(char) * 2);

   recieve = recv(socket,fileContents,2,0);

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

void checkout(char* projectName, int socket){

    DIR *dr = opendir(projectName);
    if (dr == NULL)
    {
        int check = mkdir(projectName,0777);
        
    }
    else
    {
        send(socket,"!**SlATtCanT!DiRExiSTS!!:)**",29,0);
        printf("Project Already Exists!\n");
        return;
    }
   
    char* returnMessage = malloc (sizeof(char) * 50);
    int len = strlen(projectName)+1;
    send(socket,projectName,len,0);
    recv(socket, returnMessage, 50, 0);
    if (strcmp(returnMessage, "DNE") == 0)
    {
        printf("Folder does not exist\n"); //check to see if folder exists on server side, if not it stops
        return;
    }
    printf("%s\n", returnMessage); 

   
    
    char* startSending = "Start Process";
    send(socket,startSending,strlen(startSending),0); //starts the loop for recieving files and directories
    char *getPrompt = malloc(sizeof(char) * 5);
    recv(socket, getPrompt, 5, 0);


    while (strcmp(getPrompt, "STOP") != 0)
    {
        if (strcmp(getPrompt, "FILE")==0)
        {
            char* gotType = "Got Type";
            send(socket,gotType,strlen(gotType),0); //sebd confirmation it got the type 
            char* recieveSize = (char*)malloc (sizeof(char) * 10);
            recv(socket, recieveSize, 10, 0); //gets size of the path name
            int Namesize = atoi(recieveSize); //turns char array of size into a usuable int
            send(socket,"Got Size", 8 ,0); //sends confirmation that it got the size of the file name 
            char* fileName = (char*) malloc(sizeof(char) * Namesize); //allocates mem for the file path 
            recv(socket, fileName, Namesize, 0); //gets the file path
            //printf("FILE: %s + Size: %d\n", fileName, Namesize); 
            char* gotName = "Got Name";
            send(socket,"Got Size", 8 ,0); //sends confirmation that it got the file path

            char* isNotEmpty = malloc( sizeof(char) * 11);
            recv(socket, isNotEmpty, 11, 0);
            send(socket, "OKK", 4, 0);
            int fd = open(fileName,O_RDWR|O_CREAT|O_APPEND,0777);
            printf("FD: %d",fd);
            if (strcmp(isNotEmpty, "FF") == 0)
            {

                char* recieveFileSize = (char*)malloc (sizeof(char) * 10); 
                recv(socket, recieveFileSize, 10, 0); //gets size of the file 
                int Filesize = atoi(recieveFileSize); //turns char array of size into a usuable int
                send(socket,"Got Size", 8 ,0); //sends confirmation that it got the size of the file buffer 
                char* fileBuffer = (char*) malloc(sizeof(char) * Filesize); //allocates mem for the file buffer 
                recv(socket, fileBuffer, Filesize, 0); //gets the file buffer
                send(socket,"Got Size", 8 ,0); //sends confirmation that it got the file pat
                printf("Buffer: %s\n", fileBuffer);
                write(fd,fileBuffer,strlen(fileBuffer));
            }
            else
            {  
                printf("Empty File\n");
            }
 
        }
        else if(strcmp(getPrompt, "DIRE") == 0)
        {
            char* gotType = "Got Type";
            send(socket,gotType,strlen(gotType),0); //send confirmation it got the type 
            char* recieveSize = malloc (sizeof(char) * 10);
            recv(socket, recieveSize, 10, 0); //gets size of the path name
            int Diresize = atoi(recieveSize); //turns char array of size into a usuable int
            send(socket,"Got Size", 8 ,0); //sends confirmation that it got the size of the file name 
            char* fileName = (char*) malloc(sizeof(char) * Diresize + 1); //allocates mem for the file path 
            recv(socket, fileName, Diresize, 0); //gets the file path
            //printf("Dire: %s + Size: %d\n", fileName, Diresize); 
            char* gotName = "Got Name";
            send(socket,"Got Size", 8 ,0); //sends confirmation that it got the file path
            int check = mkdir(fileName,0777);
           
        }
        recv(socket, getPrompt, 5, 0);
    }
   closedir(dr);

}



int getConfigureDetails(){

    char name[17] = ".Configure"; 
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

    char configureS [50] = ".Configure";
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
   char*buffer = (char*)(malloc(sizeof(char)*1));
   buffer[0] = '\0';
    char c;
    int fd = open(fileName,O_RDONLY);
    int status;
    int counter = 0;
    if (fd!=-1){
        do{
    
                status =  read(fd, &c, 1);
                if (status<=0){
                    break;
                }
                else{   
                    int len = strlen(buffer);
                    buffer =  realloc(buffer,(len+ 2)*sizeof(char));
                    buffer[len] = c;
                    buffer[len+1] = '\0';
                  
                }
               
            }while(status >0);
         
            close(fd);
        return buffer; 
    }
    printf("Cannot open the file");
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
bool canCommit(int socket,char*projectName){
     struct dirent *de; 
    DIR *dr = opendir(projectName);
    if (dr == NULL)
    {
        printf("Client does not have folder\n");
        return false;
    }
     while ((de = readdir(dr)) != NULL) 
     {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
        {
            if( strcmp(de->d_name,".Conflict")==0)
            {
                printf("**Found a .Conflict File! Cannot Commit!**");
                return false;
            }
            if (strcmp(de->d_name,".Update")==0)
            {
                char temp [7+strlen(projectName)+2];
                strcpy(temp,projectName);
                strcat(temp,"/");
                strcat(temp,".Update");
                int fd = open(temp,O_RDONLY);
                if (fd==-1){
                    return true;
                }
                char c;
                int status = read(fd,&c,1);
                if (!status<=0){
                    return false;
                }

            }
        }
     }
    return true;
    
}

int main(int argc, char **argv)
{
    char*host;
    char*port1; 

        //send a project name
        //recieve confirmation 

        
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
            char command[7] = "create";
            send(socket,command,7,0);//sends command to create Check
            char* reply = malloc(50* sizeof(char));
            recv(socket, reply, 50, 0); //recieves confirmation that server got the command

            send(socket, argv[2], strlen(argv[2]) + 1, 0); //sends name of the file to be created to check server list
            reply = malloc(50* sizeof(char));
            recv(socket, reply, 50, 0); //gets confirmation that server got the name of the project
            
            if (strcmp(reply, "Exists") == 0)
            {
                printf("Project already exists\n");
                return;
            }
            else{
                printf("Reply: %s\n", reply);
                create(socket, argv[2]);  
            }
        }
        if (strcmp(argv[1],"destroy")==0){
            int socket =  connectToServer();
            char command[8] = "destroy";
            send(socket,command,8,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket, reply, 50, 0);
            printf("Reply: %s\n", reply);

            send(socket, argv[2], strlen(argv[2]) + 1, 0); //sends name of the file to be created to check server list
            reply = malloc(50* sizeof(char));
            recv(socket, reply, 50, 0); //gets confirmation that server got the name of the project

            if (strcmp(reply ,"DNE") == 0)
            {
                return;
            }

            destroy(socket, argv[2]);  
        }
        
        if (strcmp(argv[1],"add")==0)
        {
            add(argv[2],argv[3]);
          
        }
        if (strcmp(argv[1],"remove")==0){
            removeFile(argv[2],argv[3]);
        }
        
        if (strcmp(argv[1],"commit")==0){
            int socket =  connectToServer();
            char command[7] = "commit";
            send(socket,command,7,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket, reply, 50, 0);
            printf("Reply: %s\n", reply);

            send(socket, argv[2], strlen(argv[2]) + 1, 0); //sends name of the file to be created to check server list
            reply = malloc(50* sizeof(char));
            recv(socket, reply, 50, 0); //gets confirmation that server got the name of the project

            if (strcmp(reply ,"DNE") == 0)
            {
                return;
            }

            if (canCommit(socket,argv[2]))
            commit(argv[2], socket);  
        }
        if (strcmp(argv[1],"push")==0){
            int socket = connectToServer();
            char command[5] = "push";
            send(socket,command,5,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket,reply,50,0);
            printf("Reply: %s\n", reply);

            send(socket, argv[2], strlen(argv[2]) + 1, 0); //sends name of the file to be created to check server list
            reply = malloc(50* sizeof(char));
            recv(socket, reply, 50, 0); //gets confirmation that server got the name of the project

            if (strcmp(reply ,"DNE") == 0)
            {
                return;
            }

            push(argv[2],socket);
        }
        if (strcmp(argv[1],"history")==0){
            int socket = connectToServer();
            char command[8] = "history";
            send(socket,command,8,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket,reply,50,0);
            printf("Reply: %s\n", reply);

            send(socket, argv[2], strlen(argv[2]) + 1, 0); //sends name of the file to be created to check server list
            reply = malloc(50* sizeof(char));
            recv(socket, reply, 50, 0); //gets confirmation that server got the name of the project

            if (strcmp(reply ,"DNE") == 0)
            {
                return;
            }

            history(argv[2],socket);
        }
        if (strcmp(argv[1],"currentVersion")==0){
            int socket = connectToServer();
            char command[15] = "currentVersion";
            send(socket,command,15,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket,reply,50,0);
            printf("Reply: %s\n", reply);

            send(socket, argv[2], strlen(argv[2]) + 1, 0); //sends name of the file to be created to check server list
            reply = malloc(50* sizeof(char));
            recv(socket, reply, 50, 0); //gets confirmation that server got the name of the project

            if (strcmp(reply ,"DNE") == 0)
            {
                return;
            }

            currentVersion(argv[2],socket);
        }
        if (strcmp(argv[1],"checkout")==0){
            int socket = connectToServer();
            char command[9] = "checkout";
            send(socket,command,9,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket,reply,50,0);
            printf("Reply: %s\n", reply);

            send(socket, argv[2], strlen(argv[2]) + 1, 0); //sends name of the file to be created to check server list
            reply = malloc(50* sizeof(char));
            recv(socket, reply, 50, 0); //gets confirmation that server got the name of the project

            if (strcmp(reply ,"DNE") == 0)
            {
                return;
            }

            checkout(argv[2],socket);
        }
        if (strcmp(argv[1],"update")==0){
             int socket = connectToServer();
            char command[7] = "update";
            send(socket,command,7,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket,reply,50,0);
            printf("Reply: %s\n", reply);

            send(socket, argv[2], strlen(argv[2]) + 1 , 0); //sends name of the file to be created to check server list
            reply = malloc(50* sizeof(char));
            recv(socket, reply, 50, 0); //gets confirmation that server got the name of the project

            if (strcmp(reply ,"DNE") == 0)
            {
                return;
            }
            
            update(argv[2],socket);
        }
        if (strcmp(argv[1],"lock")==0){
            int socket = connectToServer();
            char command[5] = "lock";
            send(socket,command,5,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket,reply,50,0);
            printf("Reply: %s\n", reply);
            send(socket,argv[2],strlen(argv[2]),0);

            
        }
        if (strcmp(argv[1],"lock1")==0){
             int socket = connectToServer();
            char command[6] = "lock1";
            send(socket,command,6,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket,reply,50,0);
            printf("Reply: %s\n", reply);
            send(socket,argv[2],strlen(argv[2]),0);

            
        }

       
        return 0;
}

