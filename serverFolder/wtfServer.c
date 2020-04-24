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

//added return statments at the end of each function to make sure locks work properly 
//check to make sure this did not F anything up lmfaoooo

static pthread_mutex_t projectMutexes[1000];

typedef struct Project{
    char* projectName;
    struct Project* next;
}Project;


Project* head = NULL;

typedef struct File{
    int version;
    char* filePath;
    char* hash;
    struct File* next;
}File;
typedef struct CommitFile{
    char command;
    int version;
    char* filePath;
    char* hash;
    struct CommitFile* next;
}CommitFile;

typedef struct Manifest{
    int ProjectVersion;
    struct File* fileHead; 
    struct Manifest* ptr;
}Manifest;

void addProject(Project** head_ref, char* projectName) 
{ 
    /* 1. allocate node */
    Project* new_node = (struct Project*) malloc(sizeof(Project)); 
  
    Project *last = *head_ref;  /* used in step 5*/
   
    /* 2. put in the data  */
    new_node->projectName  = projectName; 
  
    /* 3. This new node is going to be the last node, so make next  
          of it as NULL*/
    new_node->next = NULL; 
  
    /* 4. If the Linked List is empty, then make the new node as head */
    if (*head_ref == NULL) 
    { 
       *head_ref = new_node; 
       return; 
    }   
       
    /* 5. Else traverse till the last node */
    while (last->next != NULL) 
        last = last->next; 
    last->next = new_node; 
    return;     
} 

int searchProject(Project* head, char* projectName)
{
    int count = 0;
    while (head!= NULL)
    {
        if (strcmp(head->projectName, projectName) == 0)
        {
            return count;
        }
        else{
            head = head->next;
            count++;
        }
    }
    return -1;
}



char* readInFile(char* fileName);
int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}


void listFilesRecursively(char *basePath, int socket)
{
    
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
            if (is_regular_file(path) == 1) //check to see if it is a file or directory 
            {
                char* file = "FILE";
                send(socket,file,strlen(file),0); //sends file message 
                char* confirmation = malloc(sizeof(char) * 9);
                recv(socket, confirmation, 100, 0); //gets cofirmation from client 
                int fileNameLength = strlen(path)+1; //gets the lenghth of the file path
                char size[10];
                //printf("length: %d\n", fileNameLength); 
                sprintf(size,"%d",fileNameLength); //changes the integer into a char array to be sent over to the client
                send(socket,size,10,0); //sends the size of the path name 
                char nameSizeConfirm[8];
                recv(socket,nameSizeConfirm,8,0); //recieves a confirmation that the client got the size of the path name 
                //printf("path: %s + Size: %d\n", path, fileNameLength);
                char* tPath = (char*)(malloc(sizeof(char)*(strlen(path))));
                tPath = path;
                send(socket,tPath,fileNameLength,0); //sends actual file path name
                char* confirmName = malloc(sizeof(char) * 9);
                recv(socket,nameSizeConfirm,8,0); //client confirms it got the namE

                
                int Fsize = 1000;//strlen(readInFile(tPath)); //gets size of file
                char* fileBuffer = (char*) malloc(sizeof(char) * Fsize); //mallocs a buffer for the file 
                fileBuffer = readInFile(tPath); //puts file into a buffer
                if (fileBuffer[0] != '\0')
                {
                    send(socket,"FF",3,0); //send a signal to client saying file is not empty 
                    char*emptyCheck = (char*)(malloc(sizeof(char)*4));
                    recv(socket,emptyCheck,4,0); //recieved confimration from client that file is not empty 

                    char fileSizeArr[10]; 
                    sprintf(fileSizeArr,"%d", Fsize); //changes the integer into a char array to be sent over to the client
                    send(socket,fileSizeArr,10,0); //sends the size of buffer
                    char FileSizeConfirm[8];
                    recv(socket,FileSizeConfirm,8,0); //client confirms it got the size of the file
                    send(socket,fileBuffer, Fsize, 0); //sends the actual file buffer
                    recv(socket,FileSizeConfirm,8,0); //client confirms it got the namE
                    //printf("Buffer: %s\n", fileBuffer);

                }
                else
                {
                    char* isNotEmpty = malloc( sizeof(char) * 11);
                    send(socket, "EE", 3, 0);
                    char*emptyCheck = (char*)(malloc(sizeof(char)*4));
                    recv(socket,emptyCheck,4,0); //recieved confimration from client that file is empty 
                  
                    printf("File Empty\n");
                }
             
               
            

            }
            else
            {
                char* file = "DIRE";
                send(socket,file,strlen(file),0); //sends file message 
                char* confirmation = malloc(sizeof(char) * 9);
                recv(socket, confirmation, 100, 0); //gets cofirmation from client 
                int direNameLength = strlen(path); //gets the lenghth of the file path
                char size[10];
                //printf("length: %d\n", fileNameLength); 
                sprintf(size,"%d",direNameLength); //changes the integer into a char array to be sent over to the client
                send(socket,size,10,0); //sends the size of the path name 
                char direSizeConfirm[8];
                recv(socket, direSizeConfirm,8,0); //recieves a confirmation that the client got the size of the path name 
                //printf("path: %s + Size: %d\n", path, direNameLength);
                send(socket,path,direNameLength,0); //sends actual file path name
                char* confirmName = malloc(sizeof(char) * 9);
                recv(socket, direSizeConfirm, 8,0); //client confirms it got the name
            }
            
            //printf("%s\t", path);
            //printf("%d\n",is_regular_file(path));
            
            listFilesRecursively(path,socket);
        }
    }

    closedir(dir);

    
   
    
  
    

}
    
     
  

void checkout(int sock)
{
    char*projectName = (char*)(malloc(sizeof(char)*100));
    int readSize = recv(sock, projectName, 100, 0);//gets the name of the project
    if (strcmp("!**SlATtCanT!DiRExiSTS!!:)**",projectName ) == 0)
    {
       printf("Client already has the project!\n");
       return;
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
       // listDirectoryRecursively(projectName);

        char* startMessage = malloc (sizeof(char) * 14);
        recv(sock, startMessage, 100, 0); //gets start process message
        printf("%s\n", startMessage);

        listFilesRecursively(projectName, sock);
        printf("reached here\n");
        char* STOP = "STOP";
        send(sock,STOP,strlen(STOP),0);
    }

    return;
    

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
            send(socket,"DNE",4,0);
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


            char commitNameSize[10];
            recv(socket, commitNameSize, 10, 0); //gets size of file name 

            if (strcmp(commitNameSize, "Stop") == 0)
            {
                printf("Project versions are different\n");
                return;
            }

            int NameSize = atoi(commitNameSize); //converts char* to int 
            printf("Name Size: %d\n", NameSize);
            send(socket,"Got Size", 8 ,0); //sends confirmation

         

            char* commitPath = malloc(NameSize); //mallocs size for filename 

            recv(socket, commitPath, 100, 0); //gets file name 
            send(socket,"Got Size", 8 ,0); //sends confirmation

            //strcpy(commitPath,projectName);
            //strcat(commitPath,"/");
            //strcat(commitPath,".Commit");


            char commitFileSize[10];
            recv(socket, commitFileSize, 10, 0); //gets size of file as a char*
            
            int commitSize = atoi(commitFileSize); //converts char* into an integer 

            char* clientCommitFile = (char*)(malloc(sizeof(char)*commitSize));

            send(socket,"Got Size", 8 ,0); //sends confirmation that it got the size

            recv(socket,clientCommitFile,commitSize,0);//saves the commit file inside clientCommitFile

            printf("Client Commit:\n%s", clientCommitFile);

            
            int commitFD = open(commitPath,O_RDWR|O_APPEND);
            if (commitFD!=-1){
                 printf("*Overwriting Commit File**\n");
            }
             commitFD = open(commitPath,O_RDWR|O_APPEND|O_CREAT|O_TRUNC,0777);   
            printf("%s\n",commitPath);
            printf("%d\n",commitSize);
            write(commitFD,clientCommitFile,commitSize);

            close(commitFD);



        }
        
        return;

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

    return;
}

char* readInFile(char* fileName)
{
    char  buffer [100000];
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
                  
                    
                    /*int len = strlen(buffer);
                    char* new_buffer = (char *)malloc((strlen(buffer)+2));
                    memcpy((void *)new_buffer,(void *)buffer,strlen(buffer));
                    free(buffer);
                    buffer = new_buffer;*/
                    buffer[counter] = c;
                    counter++;
                  
                }
               
            }while(status >0);
       
            close(fd);

            char*nB  = buffer;
         
           
        return nB; 
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
        system(pathToDelete);
        printf("Successfully Destroyed\n");
        send(sock, "Successfully Deleted", 20, 0);
        return;
    }
    else
    {
        closedir(dr);
        printf("Project Does not Exist\n");
        send(sock,"Project Does not Exist", 22, 0);
        return;
    }

    return;
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

    return;
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
CommitFile* createCommitFileNode(char command, int version, char* filePath, char* hash)
{
    struct CommitFile* temp = (struct CommitFile*)malloc(sizeof(CommitFile));
    temp->command = command;
    temp->filePath = filePath;
    temp->version = version;
    temp->hash = hash;
    return temp;
}
File* tokenizeManifest(char* clientBuffer)
{
    
   
   
   char projectVersionBuff[10];
   int counter = 0;
    File* cHead = NULL;//= server->fileHead;
    int i=0;
    char* buffer = malloc(sizeof(char) * 2);

    
   buffer[0] = '\0';
     int len = strlen(buffer);
   
    while (clientBuffer[i]!='\n'){
        buffer = realloc(buffer,(len+ 2)*sizeof(char));
       
        buffer[len] = clientBuffer[i];
        buffer[len+1] = '\0';
        i++;
    }
    i++;
    int projVersion = atoi(buffer);
    projVersion++;
  
   
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
                // printf("version check: %d\n", version);
                 buffer = malloc(sizeof(char) *1);
                buffer[0] = '\0';
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
                buffer[0] = '\0';
               count++;
            }
            else if (count==2)
            {
               hash = malloc(strlen(buffer));
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
            buffer[++len] = '\0'; 
        }
        i++;
        
    }

    File*temp = cHead;
    printf("reached\n");
      

    return cHead;
     
   
  
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
    int SNodeLength = 0;
   CommitFile*head = NULL;
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
                 count++;
            }
            else if (count==2)
            {
               filePath = malloc(strlen(buffer));
               strcpy(hash,buffer);
               memmove(filePath, filePath+1, strlen(filePath));
               buffer = malloc(sizeof(char) *1);
               count++;
            }
             else if (count==3)
            {
               hash = malloc(strlen(buffer));
               strcpy(hash,buffer);
               memmove(hash, hash+1, strlen(hash));
               buffer = malloc(sizeof(char) *1);
               count++;
            }
           
        }
        if (cBuffer[i]=='\n')
        {
         //  printf("%d\t%s\t%s",version,filePath,hash);
            CommitFile* tempNode = createCommitFileNode(command,version, filePath, hash);     
            insertCommitFileNode(&head, tempNode);
            SNodeLength++;
            count = 0;
            i++;
        }
        else
        {
            int len = strlen(buffer);
            buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
            buffer[len] = cBuffer[i];
            buffer[len+1] = '\0'; 
            
            i++;
        }
        
    }
    return head;
}

void applyChanges(File*manifestHead,CommitFile*commitHead)
{
    File*mHead = manifestHead;
    CommitFile*cHead = commitHead;
    while (cHead!=NULL){
        if ((cHead->command=='M') ||(cHead->command=='D'))
        {
            while (mHead!=NULL)
            {
                if (strcmp(mHead->filePath,cHead->filePath)==0){
                    if (cHead->command=='M')
                    {
                        mHead->hash = (char*)(malloc(sizeof(char)*(strlen(cHead->hash)+1)));
                        strcpy(mHead->hash,cHead->hash);
                        mHead->version = cHead->version;

                    }
                    else
                    {
                        deleteNode(&mHead,mHead->filePath);
                    if (mHead==NULL)
                        break;
                    }
                }
                mHead = mHead->next;
            }
        }
        cHead = cHead->next;
       
    }
    File*mhead2 = manifestHead;
    CommitFile* cHead2 = commitHead;

    while (cHead2!=NULL)
    {
        if (cHead2->command=='A')
        {
            File*temp = createFileNode(cHead2->version,cHead2->filePath,cHead2->hash);
            insertFileNode(&mhead2,temp);
        }
    }

}
void push(int sock)
{
    /*
     char FileNameSize[10];
     recv(sock, FileNameSize, 10, 0); //gets size of project name 
     int FNameSize = atoi(FileNameSize); //converts char* to int 
     send(sock,"Got Size", 8 ,0); //sends confirmation

     */
    
     char projectName[30];
     recv(sock, projectName, 30, 0); //gets name of project
     send(sock,"Got Name", 9 ,0); //sends confirmation it got name
     printf("projName: %s\n", projectName);

    //need to get project Name

     char commitNameSize[10];
     recv(sock, commitNameSize, 10, 0); //gets size of file name 
     int NameSize = atoi(commitNameSize); //converts char* to int 
     send(sock,"Got Size", 8 ,0); //sends confirmation

            

     char* commitPath = malloc(NameSize); //mallocs size for filename 
     

    
    int readSize = recv(sock, commitPath, NameSize, 0); //gets commit file name 
    if (readSize > 0)
    {
        printf("Got requested path\n");
    }

    send(sock,"Got Project", 12 ,0); //sends confirmation it got the commit file name 
    

   
    char commitFileSize[10];
    recv(sock, commitFileSize, 10, 0); //gets size of file as a char*
            
    int commitSize = atoi(commitFileSize); //converts char* into an integer 

    char* clientCommitFile = (char*)(malloc(sizeof(char)*commitSize));

    send(sock,"Got Size", 8 ,0); //sends confirmation that it got the size

    recv(sock,clientCommitFile,commitSize,0);//saves the commit file inside clientCommitFile

    //printf("Client Commit:\n%s", clientCommitFile);

    
    struct dirent *dp, *dx;
    DIR *dir = opendir(projectName); //opens project directory
    if (!dir){
        printf("Project Name does not exist!\n");
        return; //returns if DNE
    }
    bool same = false;
    while ((dp = readdir(dir)) != NULL)
    {
         char*commmitExtraction = &commitPath[strlen(projectName)+1];
      
        
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0 && strcmp(dp->d_name, commmitExtraction) == 0)
        {
            printf("Found a match between Commits!, starting to push!\n");   //loops through directory until we find a mathch between what was sent and what we have (.Commit files)
            same = true;

        }
    }
    closedir(dir);
       DIR *dir2 = opendir(projectName); //open the directory once more to delete any other potential .Commit files
    if (!same)
    {
        closedir(dir2);
        printf("Sent Commit file is not the same / does not exist on the server!\n");  //if we did not find a match origianlly, return
        return;
    }
  
    
    //otherwise loop through and delete all .Commit files that are not equal to the matching one.
        char commitExt[9];
        while ((dx = readdir(dir)) != NULL)
        {
            if (strcmp(dx->d_name, ".") != 0 && strcmp(dx->d_name, "..") != 0 && strcmp(dx->d_name, commitPath) != 0)
            {
               strncpy(commitExt,dx->d_name,8);
               commitExt[8] = '\0';
               if (strcmp(commitExt,".Commit")==0){
                   char path [strlen(projectName)+2+strlen(dx->d_name)];
                   strcpy(path,projectName);
                   strcat(path,"/");
                   strcat(path,dx->d_name);
                   remove(path);
               }
            }
        }

    //create a path to a new directory called Oldversions
        char olderVersionsPath[strlen(projectName)+15];
       
        strcpy(olderVersionsPath,"olderVersions");
   
    //open the directory : if it is null, create one, otherwise continue on
        DIR *dir3 = opendir(olderVersionsPath);
        if (!dir3)
        {
            printf("Making new Older Versions Directory!\n");
            mkdir(olderVersionsPath,0777);
        }

        char manifestPath[strlen(projectName)+11];
        strcpy(manifestPath,projectName);
        strcat(manifestPath,"/.Manifest");
        int fd = open(manifestPath,O_RDONLY);
        if (fd==-1)
            printf("bad path\n");
        
        int status; 
        char c;
        char*projVersion = (char*)(malloc(sizeof(char)*1));
        projVersion[0] = '\0';
    while (c!='\n'){
       read(fd,&c,1);
       int len = strlen(projVersion);
       projVersion = (char*)realloc(projVersion,(len+ 2)*sizeof(char));
       projVersion[len] = c;
       projVersion[len+1] = '\0';    
    }
    close(fd);

    
        char oldProjectsPath[14+(2*(strlen(projectName)))+strlen(projVersion)+4];
        strcpy(oldProjectsPath,olderVersionsPath);
        strcat(oldProjectsPath,"/");
        strcat(oldProjectsPath,projectName);
        strcat(oldProjectsPath,"/");
        strcat(oldProjectsPath,projectName);
        strcat(oldProjectsPath,"_");
        strcat(oldProjectsPath,projVersion);
        
        strcat(olderVersionsPath,"/");
        strcat(olderVersionsPath,projectName);

       

       int check2 = mkdir(olderVersionsPath,0777);
      
        char copy[200];
       strcpy(copy,"cp -R ");
       strcat(copy,projectName);
       strcat(copy," ");
      strcat(copy,oldProjectsPath);
     // printf("%s\n",copy);
        system(copy);

        closedir(dir2);
        closedir(dir3); 
         char  path[strlen(projectName)+11];
        strcpy(path,projectName);
        strcat(path,"/.Manifest");
        char*buff = readInFile(path); 
    
        int i=0;
                char*buffer = (char*)(malloc(sizeof(char)*1));
                buffer[0] = '\0';
                 while (buff[i]!='\n'){
                    int len = strlen(buffer);
                    buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
                    buffer[len] = buff[i];
                    buffer[len+1] = '\0';
                    i++;
                }
                 i++;
                int manifestVersion = atoi(buffer);
                manifestVersion++;
                 buffer = (char*)(malloc(sizeof(char)*1));
               
                File* cHead = NULL;

                cHead = tokenizeManifest(buff);
       
       File* temp = cHead;
         while (temp!= NULL)
        {
            printf("%d\t%s\t%s\n", temp->version, temp->filePath, temp->hash);
            temp = temp->next;
        }


        //Run through manifest and create Nodes
        //run through commit and look for all "delete files" and "modify" options
            //find those in the linked list and modify / delete from the linked list
        //next run through all the add file commands in the commit file
            // add new nodes to the linked list 
        //write back to the manifest file while incrementing the proj.version
        //write out a history file 
       
       // CommitFile*commitHead = NULL;
        //commitHead = tokenizeCommit(clientCommitFile);
       // applyChanges(manifestHead,commitHead);  //checks for M, A , D commands in commit linked list and applies changes to the LL of the Manifest

    
       
    
 return;
}

void update(int socket){

    char*projectName = (char*)(malloc(sizeof(char)*100)); //allocates meme for the project name
    int readSize = recv(socket, projectName, 100, 0); //gets project name
    if (readSize > 0)
    {
        printf("Got requested path\n");
    }

     char path[strlen(projectName)+5+strlen(".Manifest")];
            strcpy(path,projectName);
            strcat(path,"/");
            strcat(path,".Manifest");



    DIR *dr = opendir(projectName); 
        if (dr == NULL)  //check to see if the project exists, and sends error messages
        { 
            send(socket,"DNE",4,0); 
            printf("Project does not Exist" );
            return;
            
        } 
        else
        {
            char* buffer = malloc(sizeof(char) * (strlen(readInFile(path)))); //allocates mem for the file
            buffer = readInFile(path); //stores file in buffer
            printf("Server Buffer: %s\n", buffer);
            int length = strlen(buffer);
            char size[10];
            printf("length: %d\n", length); //manifest 
            sprintf(size,"%d",length); //changes size from an integer to a char* 
            send(socket,size,10,0); //sends size as a char* 
            char temp[8];
            recv(socket,temp,8,0); //gets confirmation that client got size
            send(socket,buffer,length,0); //sends actula buffer 
        }
        //might have to recv the .Update file from the client

    return;
}

void *server_handler (void *fd_pointer);

int main(int argc, char **argv)
{
    int i = 0;
    while (i < 1000)
    {
        pthread_mutex_init(&projectMutexes[i], NULL); //initializes all mutexes
        i++;
    }

    printf("OKKKEr\n");


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

//void addNametoList(File* head )

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

    //run a function to add all existing project names to the linked list 
    //run it inside the main method first so that it does not add names to the list every time you want to create a thread

    read(sock, command, 100);
    //recv(sock,command,2000,0);
    printf("recieved: %s\n", command); //gets command from the client
    if (strcmp(command,"create")==0)
    {
        printf("got Command to create\n");
        char* replyCommand = "Got The Command to create";
        write(sock, replyCommand, strlen(replyCommand) + 1); //sends confirmation it got the command
        char* projectName = malloc(sizeof(char) * 50);
        recv(sock, projectName, 50, 0); //gets project name and stores it in projectName variable
        char* replyName = "reply Name";
        send(sock, replyName, strlen(replyName), 0);//sends confirmation it got the name

        int mutexPosition = searchProject(head, projectName); //checks to see if project already exists
        if (mutexPosition == -1) //if this is -1 it means the project does not exist
        {
            addProject(&head, projectName); //adds project
            mutexPosition = searchProject(head, projectName); //gets position of the project
            printf("Lock Check: %d\n", pthread_mutex_lock(&projectMutexes[mutexPosition])); //lock mutex for that project
            createProject(sock); //does function 
            printf("Unlock Check: %d\n", pthread_mutex_unlock(&projectMutexes[mutexPosition])); //unlocks mutex for that project

        }
        else
        {
            char* exists = "PROJ_EXISTS";
            write(sock, exists, strlen(exists) + 1);
            return;
        }
    }
    if (strcmp(command, "destroy") == 0)
    {
        printf("got Command to destroy\n");
        char* replyCommand = "Got The Command to destroy";
        write(sock, replyCommand, strlen(replyCommand) + 1);

        char* projectName = malloc(sizeof(char) * 50);
        recv(sock, projectName, 50, 0); //gets project name and stores it in projectName variable
        
       
        DIR *dp = opendir(projectName);

         printf("reached\n");
        if(!dp){
            char* replyName = "DNE";
            send(sock, replyName, strlen(replyName), 0); //if project DNE exists it tells the client
            printf("Project does not exist\n");
            return;
        }
        else{
            closedir(dp); //closes the directory check
            char* replyName = "Got Name";
            send(sock, replyName, strlen(replyName), 0);//sends confirmation it got the name (project exists)
            
            int mutexPosition = searchProject(head, projectName); 
            if (mutexPosition == -1) //if this is -1 it means the project does not exist
            {
                addProject(&head, projectName); //adds project to the linked list
                mutexPosition = searchProject(head, projectName); //updates the position of the mutex so that it can be initialized
            }

            printf("Lock: %d\n", pthread_mutex_lock(&projectMutexes[mutexPosition])); //locks the specified lock (using mutexPosition)
            printf("position: %d\n", mutexPosition);
            destroyProject(sock); 
            printf("Unlock: %d\n", pthread_mutex_unlock(&projectMutexes[mutexPosition])); //unlocks the specified lock (using mutexPosition)
            return;
        }
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

        char* projectName = malloc(sizeof(char) * 50);
        recv(sock, projectName, 50, 0); //gets project name and stores it in projectName variable
        

        DIR *dp = opendir(projectName);
        if(!dp){
            char* replyName = "DNE";
            send(sock, replyName, strlen(replyName), 0); //if project DNE exists it tells the client
            return;
        }
        else{
            closedir(dp); //closes the directory check
            char* replyName = "Got Name";
            send(sock, replyName, strlen(replyName), 0);//sends confirmation it got the name (project exists)

            int mutexPosition = searchProject(head, projectName); 
            if (mutexPosition == -1) //if this is -1 it means the project does not exist
            {
                addProject(&head, projectName); //adds project to the linked list
                mutexPosition = searchProject(head, projectName); //updates the position of the mutex so that it can be initialized
            }

            printf("Lock: %d\n", pthread_mutex_lock(&projectMutexes[mutexPosition])); //locks the specified lock (using mutexPosition)
            commit(sock); 
            printf("Unlock: %d\n", pthread_mutex_unlock(&projectMutexes[mutexPosition])); //unlocks the specified lock (using mutexPosition)
            return;
        }
        
    }
    if (strcmp(command,"push")==0)
    {
        printf("got Command to push\n");
        char* replyCommand = "Got the Command to push";
        write(sock, replyCommand,strlen(replyCommand)+1);

        char* projectName = malloc(sizeof(char) * 50);
        recv(sock, projectName, 50, 0); //gets project name and stores it in projectName variable
        

        DIR *dp = opendir(projectName);
        if(!dp){
            char* replyName = "DNE";
            send(sock, replyName, strlen(replyName), 0); //if project DNE exists it tells the client
            return;
        }
        else{
            closedir(dp); //closes the directory check
            char* replyName = "Got Name";
            send(sock, replyName, strlen(replyName), 0);//sends confirmation it got the name (project exists)
           
            int mutexPosition = searchProject(head, projectName); 
            if (mutexPosition == -1) //if this is -1 it means the project does not exist
            {
                addProject(&head, projectName); //adds project to the linked list
                mutexPosition = searchProject(head, projectName); //updates the position of the mutex so that it can be initialized
            }

            printf("Lock: %d\n", pthread_mutex_lock(&projectMutexes[mutexPosition])); //locks the specified lock (using mutexPosition)
            push(sock); 
            printf("Unlock: %d\n", pthread_mutex_unlock(&projectMutexes[mutexPosition])); //unlocks the specified lock (using mutexPosition)
            return;
        }
    }
    if (strcmp(command, "history") == 0)
    {
        printf("got Command to history\n");
        char* replyCommand = "Got The Command to get history";
        write(sock, replyCommand, strlen(replyCommand) + 1);  

        char* projectName = malloc(sizeof(char) * 50);
        recv(sock, projectName, 50, 0); //gets project name and stores it in projectName variable
        

        DIR *dp = opendir(projectName);
        if(!dp){
            char* replyName = "DNE";
            send(sock, replyName, strlen(replyName), 0); //if project DNE exists it tells the client
            return;
        }
        else{
            closedir(dp); //closes the directory check
            char* replyName = "Got Name";
            send(sock, replyName, strlen(replyName), 0);//sends confirmation it got the name (project exists)
          
            int mutexPosition = searchProject(head, projectName); 
            if (mutexPosition == -1) //if this is -1 it means the project does not exist
            {
                addProject(&head, projectName); //adds project to the linked list
                mutexPosition = searchProject(head, projectName); //updates the position of the mutex so that it can be initialized
            }

            printf("Lock: %d\n", pthread_mutex_lock(&projectMutexes[mutexPosition])); //locks the specified lock (using mutexPosition)
            history(sock); 
            printf("Unlock: %d\n", pthread_mutex_unlock(&projectMutexes[mutexPosition])); //unlocks the specified lock (using mutexPosition)
            return;
        }
        
    }
    if (strcmp(command, "currentVersion") == 0)
    {
        printf("got Command for current version\n");
        char* replyCommand = "Got The Command for current version";
        write(sock, replyCommand, strlen(replyCommand) + 1);  

        char* projectName = malloc(sizeof(char) * 50);
        recv(sock, projectName, 50, 0); //gets project name and stores it in projectName variable
        

        DIR *dp = opendir(projectName);
        if(!dp){
            char* replyName = "DNE";
            send(sock, replyName, strlen(replyName), 0); //if project DNE exists it tells the client
            return;
        }
        else{
            closedir(dp); //closes the directory check
            char* replyName = "Got Name";
            send(sock, replyName, strlen(replyName), 0);//sends confirmation it got the name (project exists)
            
            int mutexPosition = searchProject(head, projectName); 
            if (mutexPosition == -1) //if this is -1 it means the project does not exist
            {
                addProject(&head, projectName); //adds project to the linked list
                mutexPosition = searchProject(head, projectName); //updates the position of the mutex so that it can be initialized
            }

            printf("Lock: %d\n", pthread_mutex_lock(&projectMutexes[mutexPosition])); //locks the specified lock (using mutexPosition)
            currentVersion(sock); 
            printf("Unlock: %d\n", pthread_mutex_unlock(&projectMutexes[mutexPosition])); //unlocks the specified lock (using mutexPosition)
            return;
        }
        
    }
    if (strcmp(command, "checkout") == 0)
    {
        printf("got Command for checkout\n");
        char* replyCommand = "Got The Command for checkout";
        write(sock, replyCommand, strlen(replyCommand) + 1);  

        char* projectName = malloc(sizeof(char) * 50);
        recv(sock, projectName, 50, 0); //gets project name and stores it in projectName variable
        

        DIR *dp = opendir(projectName);
        if(!dp){
            char* replyName = "DNE";
            send(sock, replyName, strlen(replyName), 0); //if project DNE exists it tells the client
            return;
        }
        else{
            closedir(dp); //closes the directory check
            char* replyName = "Got Name";
            send(sock, replyName, strlen(replyName), 0);//sends confirmation it got the name (project exists)
           
            int mutexPosition = searchProject(head, projectName); 
            if (mutexPosition == -1) //if this is -1 it means the project does not exist
            {
                addProject(&head, projectName); //adds project to the linked list
                mutexPosition = searchProject(head, projectName); //updates the position of the mutex so that it can be initialized
            }

            printf("Lock: %d\n", pthread_mutex_lock(&projectMutexes[mutexPosition])); //locks the specified lock (using mutexPosition)
            printf("position: %d\n", mutexPosition);
            checkout(sock); 
            printf("Unlock: %d\n", pthread_mutex_unlock(&projectMutexes[mutexPosition])); //unlocks the specified lock (using mutexPosition)
            return;
        }
        
    }
    if (strcmp(command, "update") == 0)
    {
        printf("got Command for update\n");
        char* replyCommand = "Got The Command for update";
        write(sock, replyCommand, strlen(replyCommand) + 1);  

        char* projectName = malloc(sizeof(char) * 50);
        recv(sock, projectName, 50, 0); //gets project name and stores it in projectName variable
        

        DIR *dp = opendir(projectName);
        if(!dp){
            char* replyName = "DNE";
            send(sock, replyName, strlen(replyName), 0); //if project DNE exists it tells the client
            return;
        }
        else{
            closedir(dp); //closes the directory check
            char* replyName = "Got Name";
            send(sock, replyName, strlen(replyName), 0);//sends confirmation it got the name (project exists)
            
            int mutexPosition = searchProject(head, projectName); 
            if (mutexPosition == -1) //if this is -1 it means the project does not exist
            {
                addProject(&head, projectName); //adds project to the linked list
                mutexPosition = searchProject(head, projectName); //updates the position of the mutex so that it can be initialized
            }

            printf("Lock: %d\n", pthread_mutex_lock(&projectMutexes[mutexPosition])); //locks the specified lock (using mutexPosition)
            update(sock); 
            printf("Unlock: %d\n", pthread_mutex_unlock(&projectMutexes[mutexPosition])); //unlocks the specified lock (using mutexPosition)
            return;
        }
    }
    if (strcmp(command, "lock") == 0)
    {
        printf("got Command for update\n");
        char* replyCommand = "Got The Command for lock";
        write(sock, replyCommand, strlen(replyCommand) + 1);  
        char* projName = malloc (sizeof(char) * 30);
        recv(sock, projName, 30, 0);
        //printf("%s\n", projName);
        int mutexPosition = searchProject(head, projName); //check to see if the project exists and what 'place' in the linked list it is in
        printf("Project Position: %d\n", mutexPosition);
        if (mutexPosition == -1) //if this is -1 it means the project does not exist
        {
            addProject(&head, projName); //adds project to the linked list
            mutexPosition = searchProject(head, projName); //updates the position of the mutex so that it can be initialized
            printf("Updated Position: %d\n", mutexPosition); //prints new position of mutex 
            //all mutexes are initialized in the main method
            printf("Lock: %d\n", pthread_mutex_lock(&projectMutexes[mutexPosition]));
            //pthread_mutex_lock(&projectMutexes[mutexPosition]); //locks the mutex for that specific prjoject
            Project* temp = head;
            while (temp!= NULL)
            {
                printf("%s\n", temp->projectName);
                temp = temp->next;
            }

            //destroy

            printf("Unlock: %d\n", pthread_mutex_unlock(&projectMutexes[mutexPosition]));
            //pthread_mutex_unlock(&projectMutexes[mutexPosition]); //unlocks the mutex for that specific project
        }
        else
        {
            printf("Name exists\n");
        }
        
    }
    if (strcmp(command, "lock1") == 0)
    {
        printf("got Command for update\n");
        char* replyCommand = "Got The Command for lock1";
        write(sock, replyCommand, strlen(replyCommand) + 1);  
        char* projName = malloc (sizeof(char) * 30);
        recv(sock, projName, 30, 0);
        //printf("%s\n", projName);
        int mutexPosition = searchProject(head, projName); //check to see if the project exists and what 'place' in the linked list it is in
        printf("Project Position: %d\n", mutexPosition);
        if (mutexPosition == -1) //if this is -1 it means the project does not exist
        {
            addProject(&head, projName); //adds project to the linked list
            mutexPosition = searchProject(head, projName); //updates the position of the mutex so that it can be initialized
            printf("Updated Position: %d\n", mutexPosition); //prints new position of mutex 
            //all mutexes are initialized in the main method
            printf("Lock: %d\n", pthread_mutex_lock(&projectMutexes[mutexPosition]));
            //pthread_mutex_lock(&projectMutexes[mutexPosition]); //locks the mutex for that specific prjoject
            Project* temp = head;
            while (temp!= NULL)
            {
                printf("%s\n", temp->projectName);
                temp = temp->next;
            }
            printf("Unlock: %d\n", pthread_mutex_unlock(&projectMutexes[mutexPosition]));
            //pthread_mutex_unlock(&projectMutexes[mutexPosition]); //unlocks the mutex for that specific project
        }
        else
        {
            printf("Name exists\n");
        }
        
        
        
    }
    command = malloc (100 * sizeof(char));
   
}

