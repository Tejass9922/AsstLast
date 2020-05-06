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


    DIR *sf = opendir("serverTest");
    DIR* cf = opendir("clientTest") ;

    if ((sf)||(cf)){
         printf("run make clean!\n");
         return;
    }


    system("mkdir clientTest"); //creates a folder for the client
    system("mkdir serverTest"); //creates a folder for the server
    system("cp WTF.c clientTest"); //moves client to a folder
    system("cp WTFServer.c serverTest"); //movues server to a folder


    pid_t pid = fork();

    if (pid==0){

        system("cd clientTest && gcc -g WTF.c -o WTF -lcrypto"); //compiles client

        system("cd clientTest && ./WTF configure NotValid 9020");  //prints error statement because hostname is not valid
     
        system("cd clientTest && ./WTF configure 127.0.0.1 9020"); //runs configure for client 

        system("cd clientTest && ./WTF NotACommand 127.0.0.1 9020"); //prints an error statement because command is not valid

        system("cd clientTest && ./WTF configure 127.0.0.1 9020 Extra"); //prints an error statement because of invalid number of arguments

        system("cd clientTest && ./WTF create testFolder"); //runs create command for client and creates a project called testFolder

        system("cd clientTest && echo \"Some random text to make sure it is sending over properly.\" > testFolder/file1.txt"); //puts a file inside of testFolder

        system("cd clientTest && ./WTF add testFolder file1.txt"); //adds a file to .manifest of testFolder

        system("cd clientTest && ./WTF commit testFolder"); //commits testFolder

        system("cd clientTest && ./WTF push testFolder"); //Pushes testFolder

        system("cd clientTest && ./WTF create testRoll"); //runs create command for client and creates a project called testRoll 

        system("cd clientTest && echo \"Roll back check.\" > testRoll/file2.txt"); //puts a file inside of testRoll

        system("cd clientTest && ./WTF add testRoll file2.txt"); //adds a file to .manifest of testRoll

        system("cd clientTest && ./WTF commit testRoll"); //commits testRoll

        system("cd clientTest && ./WTF push testRoll"); //Pushes testRoll

        system("cd clientTest && ./WTF update testRoll"); //Pushes testRoll

        system("cd clientTest && ./WTF upgrade testRoll"); //Pushes testRoll

        system("cd clientTest && echo \"Roll back check2.\" > testRoll/file3.txt"); //puts another file inside of testRoll

        system("cd clientTest && ./WTF add testRoll file3.txt"); //adds another file to .manifest of testRoll

        system("cd clientTest && ./WTF commit testRoll"); //commits testRoll again

        system("cd clientTest && ./WTF push testRoll"); //Pushes testRoll again

        system("cd clientTest && ./WTF rollback testRoll 1"); //Rollsback to version 1 of testRoll //this does not work properly

        system("cd clientTest && ./WTF history testRoll"); //Pushes testRoll

        system("cd clientTest && ./WTF currentversion testRoll"); //Pushes testRoll

        system("cd clientTest && ./WTF checkout testFolder"); //prints error message, project already exists on client side
        
        system("cd clientTest && rm -rf testFolder"); //manually deletes folder so checkout can be completed

        system("cd clientTest && ./WTF checkout testFolder"); //checkouts out project from server

        system("cd clientTest && ./WTF destroy testFolder"); //destroys project on server side

        system("cd clientTest && ./WTF commit testRoll"); //commits testRoll

        system("cd clientTest && ./WTF remove testRoll fileThatDoesNotExistOnManifest"); //Error message, file does not exist on manifest

        system("cd clientTest && ./WTF remove testRoll file2.txt"); //removes file2.txt from testRoll's manifest

  
        kill(pid,SIGINT); //sends signal to end process and kill threads, will output a message for each thread 
    }
    else 
    {
            system("cd serverTest && gcc -g -o WTFServer -pthread WTFServer.c");//compiles server
            system("cd serverTest && ./WTFServer 9020"); //starts up server
           
          
             
    }

    
    
   
     
   

    //system("cd clientFolder && gcc -g wtfClient.c -o wtfClient -lcrypto");

}

