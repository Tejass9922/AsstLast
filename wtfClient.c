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
#include <unistd.h>


int main(int argc, char **argv)
{
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
   //shrav
   int port = atoi(argv[1]);
   servaddr.sin_port = htons(port);
   servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
   //inet_pton(AF_INET, argv[1],&servaddr.sin_addr);
   
   connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr));
   //while (recv_line(sockfd,buffer ) > 0)
   while (1)
   {
            
            printf("File Name: ");
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
            char c;
          int status=  read(atoi(server_reply), &c, 1); 
          printf("%d\n",status);
         /*int fd1 = atoi(server_reply);
            
            do{
        
            status =  read(fd1, &c, 1); 
            
            if (status<=0){
                break;
            }
            printf("%c",c); 
    }while(status>0);*/
   
  
   
   }

    close(sockfd);
	return 0;
}