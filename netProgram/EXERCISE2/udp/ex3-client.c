#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>  

#define PORT 1024

#define MAXDATASIZE 4096

int main(int argc, char *argv[]){
	int sockfd, num;
	struct hostent *he;
	struct sockaddr_in server;
	char buf[MAXDATASIZE];
	socklen_t addrlen;

	fd_set rset;	//read set
	fd_set wset;	//write set
	int nRet;	//select()return value
	int nread;
	if(argc != 2)
	{
		printf("Wrong input style\n please input like that: ./client 127.0.0.1 \n");
		exit(1);
	}
	


	if((he = gethostbyname(argv[1])) == NULL)
	{
		perror("gethostbyname()error\n");
		exit(1);
	}

	

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket()error\n");
		exit(1);

	}
	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr = *((struct in_addr *)he ->h_addr);

	buf[0]='\0';
	sendto(sockfd,buf,MAXDATASIZE,0,(struct  sockaddr *)&server,sizeof(server));
	addrlen = sizeof(server);

	

	while(1){	
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		
		FD_SET(sockfd, &rset);
		FD_SET(STDIN_FILENO, &wset);//0:keyborad

		bzero(buf,MAXDATASIZE);
		if((nRet=select((sockfd>STDIN_FILENO?sockfd+1:STDIN_FILENO+1),
			&rset,&wset,NULL,NULL))<1)
		{	//socket error
			perror("select() error");
			exit(1);
		}
		else
		{
			if(FD_ISSET(sockfd, &rset))	//sockfd ready to read
			{
				bzero(buf,MAXDATASIZE);
				num = recvfrom(sockfd, buf, MAXDATASIZE,0,(struct  sockaddr *)&server,&addrlen);
				
				if(num < 0)
				{
					perror("recvfrom()error");
					exit(1);
				}
				else
				{
					buf[num]='\0';
					printf("[%s:%d]",
						inet_ntoa(server.sin_addr),htons(server.sin_port));
					printf("%s\n",buf);
					
					if(!strcmp(buf,"exit"))
					{
						printf("server %s 's is offline\n", inet_ntoa(server.sin_addr));
						break;
					}
				}
			}
			if(FD_ISSET(STDIN_FILENO, &wset))	//keyborad have some input
			{
				bzero(buf,MAXDATASIZE);
				ioctl(STDIN_FILENO,FIONREAD,&nread);//取得从键盘输入字符的个数，包括回车。   
               	if(nread ==0)   
               	{   
                  	continue;
                  	  
               	}else if(nread == 1)
               	{
               		nread = read(STDIN_FILENO,buf,nread);
               		continue;
               	}
               	else{
               		nread = read(STDIN_FILENO,buf,nread);   
               		buf[nread-1] = '\0';
					sendto(sockfd,buf,MAXDATASIZE,0,(struct sockaddr *)&server,addrlen);
					if(!strcmp(buf,"exit"))	//equal return 0
					{
						break;
					}
				}
			}
		}
					
	}
	close(sockfd);
	printf("socket closed\n");
	fflush(stdin);

}