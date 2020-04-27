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
#include<string.h> 
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
char *strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}
int testExt(char*path,int version){

     char*buffer = malloc(sizeof(char)*1);
    buffer[0]= '\0';
    int i = strlen(path)-1;
   
    while (i>=0)
    {
        if (path[i]=='_'){
            break;  
        }
       
            int len = strlen(buffer);
            buffer = (char*)realloc(buffer,(len+ 2)*sizeof(char));
            buffer[len] = path[i];
            buffer[len+1] = '\0';
            
    i--;
    }
    char* revBuf = strrev(buffer);
    if (version==atoi(buffer)){
        printf("found match!\n");
       return 0;
    }
    else if (version<atoi(buffer))
    {
       return -1;
    }
    
    return 1;
    

}
int main(int argc, char **argv){

    char*path = "olderVersions/projectTest2_2/project_t_1_4";
   
  //  testDirs(path);

    testExt(path,4);
}