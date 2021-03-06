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
#include<regex.h>

//debug 
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
typedef struct CommitFile{
    char command;
    int version;
    char* filePath;
    char* hash;
    struct CommitFile* next;
}CommitFile;

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
void insertCommitFileNode(CommitFile **head, CommitFile *newNode)
{
    newNode->next = *head;
    *head = newNode;
}
void deleteNode(File **head_ref, char* key) 
{ 
    //printf("Key: %s\n",key);
    // Store head node 
    File* temp = *head_ref, *prev; 
  
    // If head node itself holds the key to be deleted 
    if (temp != NULL &&(strcmp(key,temp->filePath)==0)) 
    { 
        *head_ref = temp->next;   // Changed head 
        free(temp);   // free old head 
         printf("Successfully Removed File!\n");            
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
    printf("Successfully Removed File!\n");
    
  
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
CommitFile* createCommitFileNode(char command, int version, char* filePath, char* hash)
{
    struct CommitFile* temp = (struct CommitFile*)malloc(sizeof(CommitFile));
    temp->command = command;
    temp->filePath = filePath;
    temp->version = version;
    temp->hash = hash;
    return temp;
}

CommitFile* tokenizeCommit(char*cBuffer){
    int i=0;
    int count = 0;
    int version;
    char*filePath;
    char*hash;
    char command;
   char* buffer = (char*)malloc(sizeof(char)*1);
   buffer[0] = '\0';
   CommitFile* head = NULL;
    while (i<strlen(cBuffer))
    {
        //printf("Char Check: %c\n", serverManifest[i]);
        if (cBuffer[i]==' ')
        {
            
            if (count==0)
            {
                 command = buffer[0];
                 buffer = malloc(sizeof(char) *1);
                 buffer[0] = '\0';
                 count++;
            }
            else if (count==1)
            {
                version = atoi(buffer);
                 memmove(buffer, buffer+1, strlen(buffer));
                 buffer = malloc(sizeof(char) *1);
                 buffer[0] = '\0';
                 count++;
            }
            else if (count==2)
            {
               
               filePath = malloc(strlen(buffer)+1);
               
               strcpy(filePath,buffer);
              
               memmove(filePath, filePath+1, strlen(filePath)+1);
               buffer = malloc(sizeof(char) *1);
                buffer[0] = '\0';
               count++;
            }
             else if (count==3)
            {
               hash = malloc(strlen(buffer)+1);
               strcpy(hash,buffer);
               memmove(hash, hash+1, strlen(hash));
               buffer = malloc(sizeof(char) *1);
                buffer[0] = '\0';
               count++;
            }

           
        }
        if (cBuffer[i]=='\n')
        {
         //  printf("%d\t%s\t%s",version,filePath,hash);
           // printf("%c\t%d\t%s\t%s\n",command, version, filePath, hash);
            CommitFile* tempNode = createCommitFileNode(command,version, filePath, hash);     
            insertCommitFileNode(&head, tempNode);
            buffer = malloc(sizeof(char)*1);
             buffer[0] = '\0';
            count = 0;
           
        }
        else
        {
            int len = strlen(buffer);
            buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
            buffer[len] = cBuffer[i];
            buffer[len+1] = '\0'; 
            
           
        }
        i++;
        
    }
  
     CommitFile* temp = head;
        /*while (temp!= NULL)
        {
            printf("%c\t%d\t%s\t%s\n",temp->command, temp->version, temp->filePath, temp->hash);
            temp = temp->next;
        }
        */
    return head;
}


char* computeHash(char* path)
{
    char hash[SHA_DIGEST_LENGTH];
    char* buffer = readInFile(path);
    SHA1(buffer, strlen(buffer), hash);
    char* test = malloc(sizeof(char) * 41);
    int j = 0;
    while(j < 20)
    {
        sprintf((char*)&(test[j * 2]), "%02x", hash[j]);
        j++;
    }
    test[40] = '\0';
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

void writeConflict(int fd,char*filePath, char*hash,char command){
    char sp = ' ';
    char nL = '\n';
   write(fd,&command,1);
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
    
      //  printf("Hostname: %s\n", hostbuffer); 
       // printf("Host IP: %s\n", IPbuffer);

     
   
        
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
            printf("perfect case, No commits to be made!\n");
            remove(commitFileName);
      
        }
        else if ((sheadTemp==NULL) &&(cheadTemp!=NULL)){
            while (cheadTemp!=NULL){
              
               printf("%s\n",cheadTemp->filePath);
                writeCommit(commitFD,cheadTemp->version,cheadTemp->filePath,cheadTemp->hash,'A');
                cheadTemp = cheadTemp->next;
            }
        }
        else if ((sheadTemp!=NULL) &&(cheadTemp==NULL))
        {
           
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
                        send(socket, "Stop\0" ,5 , 0);
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
        //printf("client Size: %d\n", Namelength);
        send(socket, NameSize ,strlen(NameSize), 0); //sends size of file name

        recv(socket,temp,8,0); //recieved confirmation



        send(socket,commitFileName ,strlen(commitFileName)+1, 0); //sends file name 

        recv(socket,temp,8,0); //recieves confirmation

        printf("\n");

       /*int fd = open(commitFileName,O_RDONLY);
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

       }while (status>0);*/
            
  

    char*commitBuffer = readInFile(commitFileName);
   
  // close(fd);
       
     
      

     
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
      bool conflictCheck = false;
             char* conflictPath = malloc((strlen(projectName) + 11) * sizeof(char));
            strcpy(conflictPath, projectName);
            char conflictExt[11] = "/.Conflict";
            strcat(conflictPath, conflictExt);
        
            remove(conflictPath);
            char* updateFileName = malloc((strlen(projectName) + 9) * sizeof(char));
            strcpy(updateFileName, projectName);
            char updateExt[9] = "/.Update";
            strcat(updateFileName, updateExt);
           
   
         int updateFD = open(updateFileName,O_RDWR|O_APPEND);
        if (client.ProjectVersion == server.ProjectVersion)
        {
            printf("Full Success: NO Updates\n");
            remove(conflictPath);
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
                    
                  if ((strcmp(cheadTemp->hash, sheadTemp->hash) != 0)) //check to see if stored hash is the same but file version is different
                  {
                        char* liveHash = malloc(sizeof(char) * 41);
                        liveHash = computeHash(cheadTemp->filePath);
                        if (strcmp(cheadTemp->hash,liveHash)==0){ //check to see if live hash of client file is different than stored hash

                            if ((cheadTemp->version!=sheadTemp->version)){
                                //modify code
                                printf("M %d %s %s\n",sheadTemp->version,sheadTemp->filePath,sheadTemp->hash); //if live hash of client file is different than stored hash adds a modify line 
                                
                                writeUpdate(updateFD,sheadTemp->version,sheadTemp->filePath,sheadTemp->hash,'M');
                                //write Update file
                            }
                        }
                        else
                        {
                            int conflictFD = open(conflictPath,O_RDWR|O_CREAT|O_APPEND, 0777);
                              remove(updateFileName);
                              conflictCheck=true;
                              printf("C %s\t%s \n",cheadTemp->filePath,liveHash); //if live hash of client file is different than stored hash adds a modify line 

                              writeConflict(conflictFD,cheadTemp->filePath,liveHash,'C');
                              //write conflict file
                              close(conflictFD);
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
                if ((cheadTemp2->next==NULL)&&(checkTest==false)){
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

       /* char* updateBuffer = (char*)(malloc(sizeof(char)* strlen(readInFile(updateFileName))));
        
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
       
        send(socket,updateBuffer ,updateSize, 0); //sends the commit buffer using the size of it stores in size */
        
    if (!conflictCheck)
          printf("Successfully Updated!\n");
    else
        printf("There is a Conflict! Make sure to resolve it and try to Update again!\n");
    
        
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
     if (strcmp(recieveSize, "EMPTY") == 0)
    {
        printf("History is empty\n"); //check to see if folder exists on server side, if not it stops
        return;
    }
    int size = atoi(recieveSize);
    send(socket,"Got Size", 8 ,0); //sends a confirmation that it got the size 
    char* historyBuffer =(char*)(malloc(sizeof(char)*size));
    recv(socket, historyBuffer ,size,0); //gets buffer containing the file

    printf("History: \n%s\n", historyBuffer);

    printf("Successfully got History!\n");



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

    printf("Successfully got Current Version!\n");
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
File* tokenizeManifest( File* cHead, char* clientBuffer)
{
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
               filePath = malloc(strlen(buffer) + 1);
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
               hash = malloc(strlen(buffer) + 1);
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
    buffer[0] = '\0';
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
    buffer[0] = '\0';
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
        if (serverManifest[i]=='\n')
        {
         //  printf("%d\t%s\t%s",version,filePath,hash);
            File* tempNode = createFileNode(version, filePath, hash);     
            insertFileNode(&sHead, tempNode);
            SNodeLength++;
            count = 0;
            i++;
              buffer = malloc(sizeof(char) *1);
               buffer[0] = '\0';
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
    buffer[0] = '\0';
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
    buffer[0] = '\0';
    while (i<strlen(clientBuffer))
    {
        //printf("Char Check: %c\n", serverManifest[i]);
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
             buffer = (char*)malloc(sizeof(char)*1);
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
    buffer[0] = '\0';
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
    buffer[0] = '\0';
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
        if (serverManifest[i]=='\n')
        {
         //  printf("%d\t%s\t%s",version,filePath,hash);
            File* tempNode = createFileNode(version, filePath, hash);     
            insertFileNode(&sHead, tempNode);
            SNodeLength++;
            count = 0;
            i++;
             buffer = malloc(sizeof(char) *1);
               buffer[0] = '\0';
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
    buffer[0]='\0';

    while (i<strlen(clientBuffer))
    {
        //printf("Char Check: %c\n", serverManifest[i]);
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
    printf("Successfully Commited!\n");
    return;
   
}
void rollback(char* projectName,int version, int socket){

    int len = strlen(projectName)+1; //allocates mem for project name
    send(socket,projectName,len,0); //sends project name
    char*reply = malloc(sizeof(char)*4);
    recv(socket,reply,10,0);
    if (strcmp(reply,"DNE")==0){
        printf("project does not exist on server!\n");
        return;
    }
  
   char versionBuff[20];
    sprintf(versionBuff,"%d",version);
    send(socket,versionBuff,strlen(versionBuff)+1,0);

   char* rep = malloc(sizeof(char) * 5);
   recv(socket, rep, 5, 0);
   if (strcmp(rep, "SUCC") == 0)
   {
       printf("Succesfull Rollback to Version: %d\n",version);
   }
    else{
        printf("Failed to Rollback!\n");
    }

}
void writeManifest(int version,char*filePath, char*hash, int fd){
    char sp = ' ';
    char nL = '\n';
    char versionBuff[20];
    sprintf(versionBuff,"%d",version);
    write(fd,versionBuff,strlen(versionBuff));
    write(fd,&sp,1);
    write(fd,filePath,strlen(filePath));
    write(fd,&sp,1);
    write(fd,hash,strlen(hash));
    write(fd,&sp,1);
    write(fd,&nL,1);

}

void applyChanges(File*manifestHead,CommitFile* commitHead, int manFD)
{
   File* mHead1 = manifestHead;
   CommitFile* cHead1 = commitHead;
    int addCheck = 1;
     int loop_check = 0;
    while (mHead1 != NULL)
    {
        addCheck = 0;
        cHead1 = commitHead;
        while (cHead1 != NULL)
        {

            //printf("%s\n", cHead1->filePath);
            if (strcmp(mHead1->filePath, cHead1->filePath) == 0)
            {


                if(cHead1->command == 'M')
                {
                   //printf("Modify %d\t%s\t%s\n", cHead1->version, cHead1->filePath, cHead1->hash);
                   writeManifest(cHead1->version, cHead1->filePath, cHead1->hash, manFD);
                 //  printf("MOD\n");
                   
                }
                addCheck = 1;

            }
            else if((cHead1->next == NULL) && (addCheck == 0))
            {
               writeManifest(mHead1->version, mHead1->filePath, mHead1->hash, manFD);
               addCheck = 1;
            }
            cHead1 = cHead1->next;
        }
        mHead1 = mHead1 ->next;
    }

    File* mHead2 = manifestHead;
    CommitFile* cHead2 = commitHead;

   
    while (cHead2 !=NULL)
    {
        if (cHead2->command == 'A')
        {
            //printf("Add %d\t%s\t%s\n", cHead2->version, cHead2->filePath, cHead2->hash);
            writeManifest(cHead2->version, cHead2->filePath, cHead2->hash, manFD);
            //printf("ADD\n");
        }
        cHead2 = cHead2->next;
    }


}

void makeDirs(char* path){      //making directories on server side up till the path that is given. Used in push.

    char*buffer = malloc(sizeof(char)*1);
    buffer[0]= '\0';
    int i = 0;
  
    while (i<strlen(path))
    {
        if (path[i]=='/'){
            struct stat sb;
            if (!(stat(buffer, &sb) == 0 && S_ISDIR(sb.st_mode))) {
               mkdir(buffer,0777);
            }       
            
        }
       
            int len = strlen(buffer);
            buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
            buffer[len] = path[i];
            buffer[len+1] = '\0';
            
        
    i++;
    }
}

void writeNewFiles(int fd,char*fileBuffer,bool empty){
//check if empty buffer first
if (!empty)
    write(fd,fileBuffer,strlen(fileBuffer));
}

void upgrade(char* projectName, int socket)
{
    send(socket, projectName, strlen(projectName) + 1, 0); //sends project name to server 
    char* projectReply = malloc (sizeof(char) * 9);
    recv(socket, projectReply, 9, 0); //gets reply from server 
    if (strcmp(projectReply, "DNE") == 0)
    {
        printf("Server does not have project\n"); //if project does not exists on server 
        return;
    }
  //  printf("%s\n", projectReply);

    char updateFileName[strlen(projectName)+9];
    char updateExt[9] = "/.Update";
    strcpy(updateFileName,projectName);
    strcat(updateFileName,updateExt);
    char*updateBuffer = readInFile(updateFileName);

    char manifestFileName[strlen(projectName)+11];
    strcpy(manifestFileName,projectName);
    char manifestExt[11] = "/.Manifest";
    strcat(manifestFileName,manifestExt);
    char*manifestBuffer = readInFile(manifestFileName);

    int manFD = open(manifestFileName,O_RDWR,O_TRUNC);

      char versionBuff[20];
    
    send(socket, "GET", 4, 0); //sends command to get manifest version 
    recv(socket, versionBuff, 20, 0); //gets manifets version 

    char* confirm = malloc(sizeof(char) * 4);

    int bufferSize = strlen(updateBuffer);
    char* bufferSizeArr = malloc (sizeof(char) * 10);
    sprintf(bufferSizeArr, "%d", bufferSize);
    send(socket, bufferSizeArr, strlen(bufferSizeArr) + 1, 0); //send buffer size 
    recv(socket, confirm, 10, 0); //gets confirmation server got the buffer size 

    send(socket, updateBuffer, bufferSize + 1,0); //sends the update buffer 

    write(manFD,versionBuff,strlen(versionBuff));
    char nL = '\n';
    write(manFD,&nL,1);

    

   //(Update) 
   CommitFile* uHead = NULL;
   uHead = tokenizeCommit(updateBuffer);

   //printf("%s\n", updateBuffer);
   File*mHead = NULL;
   mHead =  tokenizeManifest(mHead,manifestBuffer);
    if (mHead==NULL)
        printf("null case\n");
    File*temp = mHead;
    while(temp!=NULL){
        printf("%d\t%s\t%s\n",temp->version,temp->filePath,temp->hash);
        temp = temp->next;
    }

     CommitFile*temp2 = uHead;
    
   /* while(temp2!=NULL){
        printf("%c\t%d\t%s\t%s\n",temp2->command,temp2->version,temp2->filePath,temp2->hash);
        temp2 = temp2->next;
    }*/
   
   //write the version number ( new one from server)

    applyChanges(mHead,uHead,manFD);

    close(manFD);

     CommitFile* cHead2 = uHead;
    while (cHead2 !=NULL)
    {
        if ((cHead2->command == 'A') || (cHead2->command == 'M'))
        {
            makeDirs(cHead2->filePath);
            char* fileSize = malloc (sizeof(char) * 10);
            fileSize[0] = '\0';
            recv(socket, fileSize, 10, 0); //gets size of file buffer 
            if (strcmp(fileSize, "NO") == 0)
            {
                int fd = open(cHead2->filePath,O_RDWR|O_CREAT|O_TRUNC,0777);    //empty file handling
                close(fd);
                send(socket, "OK", 3, 0);
            }
            else
            {          
                send(socket, "Confirm", 8, 0); //sends confirmation it got the file size 

                int size = atoi(fileSize);

               // printf("Length: %d\n", size);

                char* fileBuffer = malloc(sizeof(char) * size);
                recv(socket, fileBuffer, size+1, 0); //gets file buffer
                send(socket, "Confirm", 8, 0); //sends confirmation it got the file buffer 

              
                int fd = open(cHead2->filePath,O_RDWR|O_CREAT|O_TRUNC,0777);
                writeNewFiles(fd,fileBuffer,false);
                close(fd);
                
            }
            

        }

        cHead2 = cHead2->next;
    }

    CommitFile* cHead3 = uHead;
    while (cHead3!=NULL)
    {
       if ((cHead3->command)=='D'){
           remove(cHead3->filePath);
       }
      cHead3 =  cHead3->next;
    }
  remove(updateFileName);
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
    
       

        char commitFileNameDelete[strlen(IPbuffer)+3+strlen(projectName)+7];
        strcpy(commitFileNameDelete,projectName);
        strcat(commitFileNameDelete,"/.Commit_");
        strcat(commitFileNameDelete,IPbuffer);
        printf("%s\n",commitFileNameDelete);
        remove(commitFileNameDelete);

        printf("Successfully Upgraded!\n");
  return;

}

void push(char*projectName,int socket)
{

        send(socket, projectName, strlen(projectName) + 1, 0); //sends project name
        char* projectReply = malloc (sizeof(char) * 9);
        recv(socket, projectReply, 9, 0);
        if (strcmp(projectReply, "DNE") == 0)
        {
            printf("Server does not have project\n");
            return;
        }
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
    
     

     
   
        
        char* commitFileName = malloc((strlen(projectName) + 10 + strlen(IPbuffer)) * sizeof(char)); //mallocs size for file name 
        strcpy(commitFileName, projectName);
        char commiteExt[10] = "/.Commit_";
        strcat(commitFileName, commiteExt);
        strcat(commitFileName, IPbuffer);
        printf("%s\n",commitFileName);   //creates project name

        char temp[8];

        int Namelength = strlen(commitFileName) + 1; //gets length of file name 
        char NameSize[10];
        sprintf(NameSize,"%d",Namelength); //converts int to char*

        send(socket, NameSize , strlen(NameSize) + 1, 0); //sends size of commit file name

        recv(socket,temp,8,0); //recieved confirmation server got commit file name size
   

    
    send(socket,commitFileName, Namelength,0);//sends commit file name

    char* confirmation = malloc (sizeof(char) * 12);
    recv(socket, confirmation, 12 ,0); //gets confirmation it got the commit file name
    printf("%s\n", confirmation);

    
    
       int fdTest = open(commitFileName,O_RDWR);
       if (fdTest==-1){
           printf("No commit file on Client side, Commit once again!\n");
           send(socket, "DNE" , 4, 0); 
           return;
       }
       close(fdTest);
          char*commitBuffer = readInFile(commitFileName); 
        
        CommitFile* commitHead = tokenizeCommit(commitBuffer);      //tokenizes commit Buffer
        int length = strlen(commitBuffer);
        //printf("%d\n",length);
        char size[10];
        sprintf(size,"%d",length);
             //converts the size into a char* to send over to the server
       
        send(socket, size ,strlen(size), 0); //sends size of file

        char tempor[8];
        recv(socket,tempor,8,0);//gets confirmation from server that it got the size 
       
        send(socket,commitBuffer ,length, 0); //sends the commit buffer using the size of it stores in size 

      
    char* DNE = malloc (sizeof(char) * 4);
    recv(socket, DNE, 4, 0);

    if (strcmp(DNE, "DNE") == 0)
    {
        printf("No matching commit file\n");
        remove(commitFileName);
        return;
    }

    char* Check = malloc(5 * sizeof(char));

    if (commitHead==NULL){
        printf("Nothing to change1\n");
        return;
    }

    CommitFile* cHead2 = commitHead;
    while (cHead2 !=NULL)
    {
        if ((cHead2->command == 'A') || (cHead2->command == 'M'))
        {
            char* fileBuffer = readInFile(cHead2->filePath); //puts file into a buffer

            int fileSize = strlen(fileBuffer); //gets file buffer size

            if (fileSize != 0)
            {
              

                char* charFileSize = malloc(sizeof(char) * 10); 
                charFileSize[0] = '\0';
                
                sprintf(charFileSize, "%d", fileSize); //converts int to char* 
                send(socket, charFileSize, strlen(charFileSize)+1, 0); //sends size of file buffer 
            
                char* reply = malloc(sizeof(char) * 8);
                recv(socket, reply, 8, 0); //gets confirmation


                send(socket, fileBuffer, strlen(fileBuffer)+1, 0); //sends actual file buffer 
                reply = malloc(sizeof(char) * 8);
                recv(socket, reply, 8, 0); //gets confirmation

              
            }
            else{
                send(socket, "NO", 3, 0); //sends size of file buffer 
                char* ok = malloc(sizeof(char) * 3);
                recv(socket, ok, 3, 0); //gets confirmation

            }
            




        }

        cHead2 = cHead2->next;
    }
        

    remove(commitFileName);

    //Changing manifest version
    
  /*  char manifestFileName[strlen(projectName)+11];
    strcpy(manifestFileName,projectName);
    char manifestExt[11] = "/.Manifest";
    strcat(manifestFileName,manifestExt);
    char*manifestBuffer = readInFile(manifestFileName);

    int i=0;
   char* buffer = (char*)malloc(sizeof(char)*1);
   buffer[0] = '\0';
    while (manifestBuffer[i]!='\n'){
        int len = strlen(buffer);
        buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
        buffer[len] = manifestBuffer[i];
        buffer[len+1] = '\0';
        i++;
    }
    i++;
    int projVersion = atoi(buffer);
    projVersion++;
    char versionBuff2[20];
    sprintf(versionBuff2,"%d",projVersion);
    
    File* mHead = NULL;
    mHead =  tokenizeManifest(mHead,manifestBuffer);
    
    int manFD = open(manifestFileName,O_RDWR,O_TRUNC);

    write(manFD,versionBuff2,strlen(versionBuff2));
    char nL = '\n';
    write(manFD,&nL,1);

    
    while (mHead!=NULL){
       writeManifest(mHead->version,mHead->filePath,mHead->hash,manFD);
        mHead = mHead->next;
    }
    close(manFD);
    */
    printf("Successfully Pushed!\n");
   
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
           

             File* tempMan = NULL;
             char manifestPath[strlen(projectName)+11];
             strcpy(manifestPath,projectName);
             strcat(manifestPath,"/.Manifest");
             char*buffer = readInFile(manifestPath);
            tempMan = tokenizeManifest(tempMan, buffer);

            while (tempMan!=NULL){

                if(strcmp(tempMan->filePath,path)==0){
                    printf("File already Exists on the Manifest!\n");
                    return;
                }
                tempMan = tempMan->next;
            }



            int fd1 = open(path,O_RDONLY);

            if (fd1!=-1){

            char hash[SHA_DIGEST_LENGTH];
            char hexHash[SHA_DIGEST_LENGTH];
           
           

            char* buffer = readInFile(path);

            char* test  = (char*)(malloc(sizeof(char)*41));
          
            test = computeHash(path);
           
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
                             write(fd,&sp,1); 
                            write(fd,&nL,1);
                         }
                     }
                }
             }
               printf("Successfully Added!\n");
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
            if (fd!=-1)
            {
                
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
    printf("%s\n", returnMessage);  //Success / faulure message

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
      // printf("Got Contents: %s\n", fileContents);  
   }
   else
   {
       printf("Did not get fileName\n");   
   }

   write(filedescriptor, fileContents, strlen(fileContents));
   
    close(filedescriptor);

    printf("Successfully Created Project!\n");
   
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
        //printf("%s\n", getPrompt); 
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
          
            if (strcmp(isNotEmpty, "FF") == 0)
            {

                char* recieveFileSize = (char*)malloc (sizeof(char) * 10); 
                recv(socket, recieveFileSize, 10, 0); //gets size of the file 
                int Filesize = atoi(recieveFileSize); //turns char array of size into a usuable int
                send(socket,"Got Size", 8 ,0); //sends confirmation that it got the size of the file buffer 
                char* fileBuffer = (char*) malloc(sizeof(char) * Filesize); //allocates mem for the file buffer 
                recv(socket, fileBuffer, Filesize, 0); //gets the file buffer
                send(socket,"Got Size", 8 ,0); //sends confirmation that it got the file pat
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
            char* fileName = (char*) malloc(sizeof(char) * Diresize); //allocates mem for the file path 
            recv(socket, fileName, Diresize, 0); //gets the file path
            //printf("Dire: %s + Size: %d\n", fileName, Diresize); 
            char* gotName = "Got Name";
            send(socket,"Got Size", 8 ,0); //sends confirmation that it got the file path
            int check = mkdir(fileName,0777);
           
        }
        recv(socket, getPrompt, 5, 0);
    }
   closedir(dr);

   printf("Successfully Checked Out!\n");

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

struct hostent* result = gethostbyname(host);
int validHost = result != NULL;

if (validHost==0){
    printf("Invalid host/IP!\n");
    return;
}
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

    printf("Successfully Configured!\n");

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
    printf("Cannot open the file / File was deleted\n");
    char*dne = "DNE\0";
    return dne;
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
        struct hostent *server;
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
        servaddr.sin_port = htons(port1);
        server = gethostbyname(info.IP);
        bcopy((char *)server->h_addr_list[0], 
         (char *)&servaddr.sin_addr.s_addr,
         server->h_length);
        

        //servaddr.sin_addr.s_addr = inet_addr(info.IP);
        printf("%s\n",info.IP);
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
bool canCommit(char*projectName){
     struct dirent *de; 
    DIR *dr = opendir(projectName);     // do the stat thing
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
bool canUpgrade(char*projectName){
     struct dirent *de;         //do the stat thing instead
    DIR *dr = opendir(projectName);
    if (dr == NULL)
    {
        printf("Project Does not exist on Client\n");
        return false;
    }
    bool hasUpdate = false;
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
              hasUpdate = true;

            }
        }
     }

     if (!hasUpdate){
         printf("No .Update File found!!\n");
        return false;
     }
    
    return true;
    
}
int main(int argc, char **argv)
{
    char*host;
    char*port1; 

        char* args_list[] = {"add","create","destroy","remove","currentversion","history","commit","push","update","upgrade","configure","checkout","rollback"};
        int i;
        bool valid = false;
        for (i=0; i<13; i++)
        {
            if (strcmp(argv[1],args_list[i])==0){
               valid = true;
            }
        }

        if (!valid){
            printf("Invalid command!\n");
            return;
        }
        
        if (strcmp(argv[1],"configure")==0){
       if (argc!=4){
           printf("Invalid number of arguements!\n");
        return;
       }
         host = argv[2];
         port1= argv[3];
         
         configure(host,port1);
        }
       
       
        if (strcmp(argv[1],"create")==0){
            if (argc!=3){
             printf("Invalid number of arguements!\n");
            return;
            }
            int socket =  connectToServer();
            if (socket > 0){
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
                else {
                    printf("Reply: %s\n", reply);
                    create(socket, argv[2]);  
                }

            }
            else{
                 printf("Cannot Connect to Server!\n");
            }
        }
        if (strcmp(argv[1],"destroy")==0){
             if (argc!=3){
             printf("Invalid number of arguements!\n");
            return;
            }
            int socket =  connectToServer();
            if (socket > 0){
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
                    printf("Project does not exist on Server!\n");
                    return;
                }

                destroy(socket, argv[2]);  
            }
            else{
                  printf("Cannot Connect to Server!\n");
            }
        }
        
        if (strcmp(argv[1],"add")==0)
        {
             if (argc!=4){
             printf("Invalid number of arguements!\n");
            return;
            }
            add(argv[2],argv[3]);
          
        }
        if (strcmp(argv[1],"remove")==0){
             if (argc!=4){
             printf("Invalid number of arguements!\n");
            return;
            }
            removeFile(argv[2],argv[3]);
        }
        
        if (strcmp(argv[1],"commit")==0){
             if (argc!=3){
             printf("Invalid number of arguements!\n");
            return;
            }

             if (! canCommit(argv[2]))
                return;

            int socket =  connectToServer();

            if (socket>0){
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
                    printf("Project does not exist on Server!\n");
                    return;
                }

            
                commit(argv[2], socket);  

            }
            else
            {
                  printf("Cannot Connect to Server!\n");
            }
        }
        if (strcmp(argv[1],"push")==0){
             if (argc!=3){
             printf("Invalid number of arguements!\n");
            return;
            }
            int socket = connectToServer();
            if (socket>0){
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
                    printf("Project does not exist on Server!\n");
                    return;
                }

                push(argv[2],socket);
            }
            else
            {
                  printf("Cannot Connect to Server!\n");
            }
        }
        if (strcmp(argv[1],"history")==0){
             if (argc!=3){
             printf("Invalid number of arguements!\n");
            return;
            }
            int socket = connectToServer();
            if (socket>0)
            {

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
                    printf("Project does not exist on Server!\n");
                    return;
                }

                history(argv[2],socket);

            }

            else{
                  printf("Cannot Connect to Server!\n");
            }
        }
        if (strcmp(argv[1],"currentversion")==0){
             if (argc!=3){
             printf("Invalid number of arguements!\n");
            return;
            }
            int socket = connectToServer();
            if (socket>0)
            {
            char command[15] = "currentversion";
            send(socket,command,15,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket,reply,50,0);
            printf("Reply: %s\n", reply);

            send(socket, argv[2], strlen(argv[2]) + 1, 0); //sends name of the file to be created to check server list
            reply = malloc(50* sizeof(char));
            recv(socket, reply, 50, 0); //gets confirmation that server got the name of the project

            if (strcmp(reply ,"DNE") == 0)
            {
                 printf("Project does not exist on Server!\n");
                return;
            }

            currentVersion(argv[2],socket);

            }
            else{
                  printf("Cannot Connect to Server!\n");
            }
        }
        if (strcmp(argv[1],"checkout")==0){
             if (argc!=3){
             printf("Invalid number of arguements!\n");
            return;
            }
                int socket = connectToServer();
                if (socket>0){
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
                    printf("Project does not exist on Server!\n");
                    return;
                }

                checkout(argv[2],socket);

            }

            else{

                  printf("Cannot Connect to Server!\n");
            }
        }
        if (strcmp(argv[1],"update")==0){
             if (argc!=3){
             printf("Invalid number of arguements!\n");
            return;
            }
             int socket = connectToServer();
             if (socket>0){
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
                printf("Project does not exist on Server!\n");
                return;
            }
            
            update(argv[2],socket);

             }
             else{
                   printf("Cannot Connect to Server!\n");
             }
        }

        if (strcmp(argv[1],"upgrade")==0){
             if (argc!=3){
             printf("Invalid number of arguements!\n");
            return;
            }
            if (! canUpgrade(argv[2]))
                return;

            int socket = connectToServer();
            if (socket>0){
            char command[8] = "upgrade";
            send(socket,command,8,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket,reply,50,0);
            printf("Reply: %s\n", reply);

            send(socket, argv[2], strlen(argv[2]) + 1 , 0); //sends name of the file to be created to check server list
            reply = malloc(50* sizeof(char));
            recv(socket, reply, 50, 0); //gets confirmation that server got the name of the project

            if (strcmp(reply ,"DNE") == 0)
            {
                printf("Project Does not exist on Server!\n");
                return;
            }
            
              upgrade(argv[2],socket);

            }
            else
            {
                  printf("Cannot Connect to Server!\n");
            }
            
        }


        if (strcmp(argv[1],"rollback")==0){
         
            if (argc!=4){
             printf("Invalid number of arguements!\n");
            return;
            }
            int socket = connectToServer();
            if (socket>0){
            char command[9] = "rollback";
            send(socket,command,9,0);
            char* reply = malloc(50* sizeof(char));
            recv(socket,reply,50,0);
            printf("Reply: %s\n", reply);

            send(socket, argv[2], strlen(argv[2]) + 1 , 0); //sends name of the file to be created to check server list
            reply = malloc(50* sizeof(char));
            recv(socket, reply, 50, 0); //gets confirmation that server got the name of the project

            if (strcmp(reply ,"DNE") == 0)
            {
                printf("Project Does not exist on Server!\n");
                return;
            }
            int version = atoi(argv[3]);
            
             rollback(argv[2],version,socket);
            }
            else
            {
                  printf("Cannot Connect to Server!\n");
            }
        }
       
       

       
        return 0;
}

