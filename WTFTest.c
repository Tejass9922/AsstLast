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


int main(int argc, char **argv)
{

    system("mkdir clientTest"); //creates a folder for the client
    system("mkdir serverTest"); //creates a folder for the server
    system("mv wtfClient.c clientTest"); //moves client to a folder
    system("mv wtfServer.c serverTest"); //movues server to a folder
    system("cd clientTest && gcc -g wtfClient.c -o wtfClient -lcrypto"); //compiles client
    system("cd serverTest && gcc -g -o wtfServer -pthread wtfServer.c"); //compiles server

    system("cd clientTest && ./wtfClient configure 127.0.0.1 9020"); //runs configure for client
    system("cd serverTest && ./wtfServer 9020 &"); //starts up server

    system("cd clientTest && ./wtfClient create testFolder"); //runs create command for client

    system("cd clientTest && echo \"Some random text to make sure it is sending over properly.\" > testFolder/file1.txt");

    system("cd clientTest && ./wtfClient add testFolder file1.txt");

    system("cd clientTest && ./wtfClient commit testFolder");

    system("cd clientTest && ./wtfClient push testFolder");

    system("killall wtfServer");

    


    //system("cd clientFolder && gcc -g wtfClient.c -o wtfClient -lcrypto");
}
