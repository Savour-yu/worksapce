#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <poll.h>
#include <signal.h>

#define INFTIM -1
#define PORT 2223
#define MAXDATASIZE 1024


main()
{
    char buf[MAXDATASIZE];
    int num; // length of rec
    int fileid;//file id

    /*init var*/
    int sockfd;
    struct sockaddr_in server, client;
    socklen_t addrlen;
    int nread;
    struct pollfd fdarray[2];

    int nRet;	//select()return value

    int flag = 0;
    // creat a tcp socket
    
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        // handle exception
        perror("socket()error.");
        exit(1);
    }


    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);	//listen any ip in server
    if(bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("Bind()error");
        exit(1);
    }
    addrlen = sizeof(client);

    fdarray[1].fd = sockfd;
    fdarray[0].fd = STDIN_FILENO;
    fdarray[0].events = fdarray[1].events = POLLIN;
    while(1)
    {
        bzero(buf, MAXDATASIZE);
        if((nRet = poll(fdarray, 2, INFTIM)) < 1)
        {
            //socket error
            perror("poll() error");
            exit(1);
        }
        else
        {
            if(fdarray[1].revents)	//sockfd ready to read
            {
                bzero(buf, MAXDATASIZE);
                num = recvfrom(sockfd, buf, MAXDATASIZE, 0, (struct  sockaddr *)&client, &addrlen);
                flag = 1;
                if(num < 0)
                {
                    perror("recvfrom()error");
                    exit(1);
                }
                else
                {
                    buf[num] = '\0';
                    printf("[%s:%d]",
                           inet_ntoa(client.sin_addr), htons(client.sin_port));
                    printf("%s\n", buf);

                    if(!strcmp(buf, "exit"))
                    {
                        printf("client %s 's is offline\n", inet_ntoa(client.sin_addr));
                    }
                }

            }
            if(fdarray[0].revents)	//keyborad have some input
            {
                if(flag != 1)
                {
                    continue;
                }
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
                    sendto(sockfd, buf, MAXDATASIZE, 0, (struct sockaddr *)&client, addrlen);
                    if(!strcmp(buf, "exit"))	//equal return 0
                    {
                        flag = 0;
                        break;
                    }
                }
            }
        }
        
    }
    fflush(stdin);
    close(sockfd);
}