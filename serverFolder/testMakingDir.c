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

void makeDirs(char* path){

    char*buffer = malloc(sizeof(char)*1);
    buffer[0]= '\0';
    int i = 0;
   
    while (i<strlen(path))
    {
        if (path[i]=='/'){
            printf("%s\n",buffer);
            DIR *dp;
            opendir(buffer);
            if (!dp)
            {
                mkdir(buffer,0777);
            }
            closedir(dp);
            
        }
       
            int len = strlen(buffer);
            buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
            buffer[len] = path[i];
            buffer[len+1] = '\0';
            
        
    i++;
    }


}

int main(int argc, char **argv){

    char*path = "Test69/test420/test4/abst/tejas.txt";
   
    testDirs(path);
}