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


    printf("please input ipv4 or ipv6 to select mode\n");
    while(1)
    {
        bzero(mode, 100);
        ioctl(0, FIONREAD, &nread); //取得从键盘输入字符的个数，包括回车。
        if(nread == 0)
        {
            continue;

        }
        else if(nread == 1)
        {
            nread = read(0, mode, nread);
            continue;
        }
        else
        {
            nread = read(0, mode, nread);
            mode[nread - 1] = '\0';
            if(!(strcmp(mode, "ipv4")) || !(strcmp(mode, "ipv6")))
            {
                break;
            }
            else
            {
                printf("please input ipv4 or ipv6 to select mode\n");

            }
        }
    }


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
    printf("youinput:%s\n", host_name );
    if(!strcmp(mode, "ipv4")) //ipv4
    {
        he = gethostbyname(host_name);
        if(NULL == he)
        {
            printf("no information for &s\n", host_name);
            exit(1);
        }
        else
        {
            struct in_addr in;
            printf("[Formal Name]:%s\n", he->h_name);
            printf("[Address Type]:%d\n", he->h_addrtype);
            printf("\n[IP Address]:");
            for(p = he->h_addr_list; 0 != *p; p++)
            {

                memcpy(&in.s_addr, *p, sizeof(in.s_addr));
                printf("\t%s", inet_ntoa(in));
            }
            printf("\n[Aliases Name]:");
            for(p = (he->h_aliases) ; 0 != *p; p++)
            {
                printf("\t%s", *p);
            }
            printf("\n");

        }
    }
    else    //ipv6
    {
        res_init();
        _res.options |= RES_USE_INET6;
        he = gethostbyname(host_name);
        if(NULL == he)
        {
            printf("no information for &s\n", host_name);
            exit(1);
        }
        else
        {
            char in[100];
            printf("[Formal Name]:%s\n", he->h_name);
            printf("[Address Type]:%d\n", he->h_addrtype);
            printf("\n[IP Address]:");
            for(p = he->h_addr_list; 0 != *p; p++)
            {

                
                printf("\t%s\n", inet_ntop(AF_INET6, *p, in, 100));

            }
            printf("\n[Aliases Name]:");
            for(p = (he->h_aliases) ; 0 != *p; p++)
            {
                printf("\t%s", *p);
            }
            printf("\n");
        }

    }


}
