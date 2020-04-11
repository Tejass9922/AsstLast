#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h> 
#include <sys/stat.h>


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
int main(int argc, char **argv)
{
 char* host ;
   char* port1;
   int port;
if (strcmp(argv[1],"configure")==0){
      host = argv[2];
     port1= argv[3];
     configure(host,port1);
}

   else{ 
    int sockfd;
    char buffer[1000];
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
   servaddr.sin_port = htons(port);
   servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
   //inet_pton(AF_INET, argv[1],&servaddr.sin_addr);
   
   connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr));
   //while (recv_line(sockfd,buffer ) > 0)
   while (1)
   {
	   
    printf("Enter a Message: ");
    scanf("%s",buffer);

    if (send(sockfd,buffer,strlen(buffer),0) < 0)
    {
	   printf("Error \n");
	   return 1;
    }
    if(recv(sockfd,server_reply,2000,0 ) < 0)
    {
	   puts("Error");
	   break;
    }
  
   printf("Server Reply: %s \n",server_reply );
   
   }

    close(sockfd);
   }
	return 0;
}