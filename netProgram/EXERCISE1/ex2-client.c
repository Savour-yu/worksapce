#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <fcntl.h>//import open method 

#define PORT 2223

#define MAXDATASIZE 4096
#define FILENAME_MAX_SIZE 512

int main(int argc, char *argv[]){
	int sockfd, num, connectfd ;//num: size of readbuf ;
	struct hostent *he;
	struct sockaddr_in server;



	if(argc != 3){
		printf("Wrong input style\n please input like that: ./ex2-client 127.0.0.1 a.txt\n");
		exit(1);
	}
	


	if((he = gethostbyname(argv[1])) == NULL){
		perror("gethostbyname()error\n");
		exit(1);
	}

	

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket()error\n");
		exit(1);

	}
	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr = *((struct in_addr *)he ->h_addr);
	if((connectfd = connect(sockfd, (struct sockaddr *)&server,sizeof(server))) == -1){
		perror("connect()error\n");
		exit(1);

	}


	int fileId;//fileid: open file descirb
	char buf[MAXDATASIZE];

	char filename[FILENAME_MAX_SIZE];
	memcpy(filename,argv[2],strlen(argv[2]));
	if((fileId = open(filename, O_RDONLY)) < 0){
		perror("open()error\n");
		exit(1);
	}

	
	printf("start sending the file:%s\n",filename);
	write(sockfd, filename, sizeof(filename));/*send the filename*/
	printf("sending %s...\n",filename);
	
	while(num=read(fileId, buf,MAXDATASIZE)){
		if(num < 0){
			perror("read error");
			exit(1);
		}
		
		if(write(sockfd, buf, num) < 0){
			perror("write to socket error");
			exit(1);
		}	
			
		bzero(buf,MAXDATASIZE);
	}



	printf("sending file over\n");
	close(fileId);
	//close(connectfd);
	close(sockfd);
	printf("socket closed\n");
}