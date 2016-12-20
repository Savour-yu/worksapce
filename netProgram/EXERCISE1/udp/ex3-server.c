#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>






#define PORT 2223
#define BACKLOG 1
#define MAXDATASIZE 1024
main(){
	char buf[MAXDATASIZE];
	int num; // length of rec
	int fileid;//file id

	/*init var*/
	int sockfd;
	struct sockaddr_in server,client;
	socklen_t addrlen;
	/*creat a tcp socket*/
	if((sockfd = socket(AF_INET, SOCK_DGRAM,0)) == -1){
		/*handle exception*/
		perror("socket()error.");
		exit(1);
	}

	
	bzero(&server,sizeof(server));

	server.sin_family=AF_INET;
	server.sin_port=htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1){
		perror("Bind()error");
		exit(1);
	}
	addrlen = sizeof(client);
	while(1){	

		bzero(buf,MAXDATASIZE);
		num = recvfrom(sockfd, buf, MAXDATASIZE,0,(struct  sockaddr *)&client,&addrlen);			
		if(num < 0){
			perror("recvfrom()error");
			exit(1);
		}
		else{
			buf[num]='\0';
			printf("You got a message form client's ip is %s, port is %d\n",inet_ntoa(client.sin_addr),htons(client.sin_port));
			printf("message:%s\n",buf);
			if(!strcmp(buf,"bye")){
				bzero(buf,MAXDATASIZE);
				sendto(sockfd,buf,MAXDATASIZE,0,(struct  sockaddr *)&client,addrlen);
				break;
			}
			bzero(buf,MAXDATASIZE);
			printf("send you message:");
			scanf("%s",buf);
			sendto(sockfd,buf,MAXDATASIZE,0,(struct  sockaddr *)&client,addrlen);
			if(!strcmp(buf,"bye")){
				bzero(buf,MAXDATASIZE);
				sendto(sockfd,buf,MAXDATASIZE,0,(struct  sockaddr *)&client,addrlen);
				break;
			}
			bzero(buf,MAXDATASIZE);
		}
	}			
	fflush(stdin);
	close(sockfd);
}