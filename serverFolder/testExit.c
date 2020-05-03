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
#include <signal.h>



static volatile int keepRunning = 1;

void intHandler(int dummy) {
   printf("\nCaught signal\n");
   exit(0);
}

// ...

int main(void) {

   signal(SIGINT, intHandler);
   while(1){

   }

}