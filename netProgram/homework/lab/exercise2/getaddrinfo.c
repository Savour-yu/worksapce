#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <resolv.h>
#include <sys/types.h>
#include <sys/socket.h>
int main()
{
    struct hostent *he;
    char mode[100];
    char host_name[100];
    int i;
    int nread;
    char **p;
    struct addrinfo hints;
    struct addrinfo *res, *cur;
    char ipstr[100];
    struct sockaddr_in *sa;
    struct sockaddr_in6 *sa6;
    printf("please input host_name\n");
    nread = 0;
    while(nread <= 1)
    {
        bzero(host_name, 100);
        ioctl(0, FIONREAD, &nread); //取得从键盘输入字符的个数，包括回车。
        if(nread == 0)
        {
            continue;

        }
        else if(nread == 1)
        {
            nread = read(0, host_name, nread);
            continue;
        }
        else
        {
            nread = read(0, host_name, nread);
            host_name[nread - 1] = '\0';
            break;
        }
    }


    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_socktype = SOCK_STREAM;

    i = getaddrinfo(host_name, NULL, &hints, &res);

    if(i == -1)
    {
        perror("getaddrinfo()error");
        exit(1);
    }
    else
    {
        printf("[Formal Name]\t%s\n", res->ai_canonname);
        for(cur = res; NULL != cur; cur = cur->ai_next)
        {
            
            if(cur->ai_family == AF_INET6)
            {
                sa6 = cur->ai_addr;
                printf("[IPV6]\t%s\n", inet_ntop(AF_INET6, &(sa6->sin6_addr), ipstr, 100));

                
            }
            else
            {
                sa = cur->ai_addr;
                printf("[IPV4]\t%s\n", inet_ntoa((sa->sin_addr)));
            }
        }

    }



}
