#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#define PORT 2223

#define MAXDATASIZE 4096

int main(int argc, char *argv[]){
	int sockfd, num;
	struct hostent *he;
	struct sockaddr_in server;
	char buf[MAXDATASIZE];
	socklen_t addrlen;



	if(argc != 2){
		printf("Wrong input style\n please input like that: ./client 127.0.0.1 \n");
		exit(1);
	}
	


	if((he = gethostbyname(argv[1])) == NULL){
		perror("gethostbyname()error\n");
		exit(1);
	}

	

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("socket()error\n");
		exit(1);

	}
	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr = *((struct in_addr *)he ->h_addr);

	printf("you input:");
	scanf("%s",buf);
	sendto(sockfd,buf,MAXDATASIZE,0,(struct  sockaddr *)&server,sizeof(server));
	addrlen = sizeof(server);

	while(1){	
		bzero(buf,MAXDATASIZE);
		num = recvfrom(sockfd, buf, MAXDATASIZE,0,(struct  sockaddr *)&server,&addrlen);
		if(num < 0){
			perror("recvfrom()error");
			exit(1);
		}
		else{
			buf[num]='\0';
			printf("You got a message form server:%s\n",buf);
			if(!strcmp(buf,"bye")){
				bzero(buf,MAXDATASIZE);
				sendto(sockfd,buf,MAXDATASIZE,0,(struct  sockaddr *)&server,addrlen);
				break;
			}
			bzero(buf,MAXDATASIZE);
			printf("Input your message:");
			scanf("%s",buf);
			sendto(sockfd,buf,MAXDATASIZE,0,(struct  sockaddr *)&server,addrlen);
			if(!strcmp(buf,"bye")){
				bzero(buf,MAXDATASIZE);
				sendto(sockfd,buf,MAXDATASIZE,0,(struct  sockaddr *)&server,addrlen);
				break;
			}
			bzero(buf,MAXDATASIZE);
		}
	}		
	

	
	close(sockfd);
	printf("socket closed\n");
}