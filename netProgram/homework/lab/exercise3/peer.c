/*
1.everyone can send measage to net
2.everyone can receive the measage in the net
*/
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>

#define MAXDATASIZE 1024
#define BROAD_PORT 1234

int main(int argc, char **argv)
{
    struct sockaddr_in peer, client;
    int sockfd, on = 1;
    char buf[MAXDATASIZE];
    int nread, num;
    fd_set allset;
    int addrlen = sizeof(struct sockaddr_in);
    if(argc != 2)
    {
        printf("Usage:%s <ip address>\n", argv[0] );
        exit(1);
    }
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0) ) < 0)
    {
        perror("socket error\n");
        exit(1);
    }


    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(int));
    memset(&peer, 0, addrlen);
    peer.sin_family = AF_INET;
    peer.sin_port = htons(BROAD_PORT);
    if(inet_pton(AF_INET, argv[1], &(peer.sin_addr)) < 0)
    {
        printf("Wrong IP address\n");
        exit(1);
    }

    memset(&client, 0, addrlen);
    int opt = SO_REUSEADDR;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
    client.sin_family = AF_INET;
    client.sin_port = htons(BROAD_PORT);
    client.sin_addr.s_addr=htonl(INADDR_ANY);
    if((bind(sockfd, (struct sockaddr *)&client, addrlen) ) == -1)
    {
        perror("Bind()error");
        exit(1);
    }


    while(1)
    {
        FD_ZERO(&allset);
        FD_SET(sockfd, &allset);
        FD_SET(STDIN_FILENO, &allset);//0:keyborad
        bzero(buf, MAXDATASIZE);

        if((select((sockfd > STDIN_FILENO ? sockfd + 1 : STDIN_FILENO + 1),
                   &allset, NULL, NULL, NULL)) < 1)
        {
            //socket error
            perror("select() error");
            exit(1);
        }
        else
        {
            if(FD_ISSET(sockfd, &allset))	//sockfd ready to read
            {
                bzero(buf, MAXDATASIZE);
                //num = read(sockfd, buf, MAXDATASIZE);
                num = recvfrom(sockfd, buf, MAXDATASIZE,0,(struct  sockaddr *)&client,&addrlen);

                if(num < 0)
                {
                    perror("read()error");
                    exit(1);
                }
                else
                {
                    buf[num] = '\0';
                    printf("[%s:%d]",
						inet_ntoa(client.sin_addr),htons(client.sin_port));
                    printf("%s\n", buf);

                }

            }
            if(FD_ISSET(STDIN_FILENO, &allset))	//keyborad have some input
            {


                bzero(buf, MAXDATASIZE);
                ioctl(STDIN_FILENO, FIONREAD, &nread); //取得从键盘输入字符的个数，包括回车。
                if(nread == 0)
                {
                    continue;

                }
                else if(nread == 1)
                {
                    nread = read(STDIN_FILENO, buf, nread);
                    continue;
                }
                else
                {
                    nread = read(STDIN_FILENO, buf, nread);
                    buf[nread - 1] = '\0';
                    sendto(sockfd, buf, MAXDATASIZE, 0, (struct sockaddr *)&peer, addrlen);
                    
                    if(!strcmp(buf, "exit"))	//equal return 0
                    {
                        break;
                    }
                }
            }
        }

    }
    fflush(stdout);
    close(sockfd);

}