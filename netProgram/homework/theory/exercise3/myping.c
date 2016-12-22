//myping.c
#include <stdio.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <setjmp.h>
#include <errno.h>

#define PACKET_SIZE 4096
#define MAX_WAIT_TIME 5     //seconds
#define MAX_NO_PACKETS 3

char sendPacket[PACKET_SIZE];
char recvPacket[PACKET_SIZE];
int sockfd, datalen = 64;
int nsend = 0, nrecv = 0;
struct sockaddr_in dest_addr;
pid_t pid;
int n = 0;
struct sockaddr_in from;
struct timeval tvrecv;
void statistics(int signo);
unsigned short cal_chksum(unsigned short *addr, int len);
int pack(int pack_no);
void send_packet(void);
void recv_packet(void);
int unpack(char *buf, int len);
void tv_sub(struct timeval *out, struct timeval *in);

void statistics(int signo)
{
    printf("\n-------------------PING statistics-------------------\n");
    float a=nsend,b=nrecv;
    printf("%d packets transmitted, %d recevied, %.2f%% lost\n", nsend, nrecv, (a - b) / a * 100);
    close(sockfd);
    exit(0);
}

unsigned short cal_chksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;
    while(nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }
    if(nleft == 1)
    {
        *(unsigned char *) (&answer) = *(unsigned char *)w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    answer = ~sum;
    return answer;
}
int pack(int pack_no)
{
    int packsize;
    struct icmp *icmp;
    struct timeval *tval;
    icmp = (struct icmp *)sendPacket;
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_cksum = 0;
    icmp->icmp_seq = pack_no;
    icmp->icmp_id = pid;

    packsize = datalen;
    tval = (struct timeval *)icmp->icmp_data;
    gettimeofday(tval, NULL);
    icmp->icmp_cksum = cal_chksum((unsigned short *)icmp, packsize);
    return packsize;
}

void send_packet()
{
    int packsize;
    nsend++;
    packsize = pack(nsend);
    if(sendto(sockfd, sendPacket, packsize, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
    {
        perror("sendto error");
        return;
    }

}

void recv_packet()
{
    int n, fromlen;
    extern int errno;
    fromlen = sizeof(from);
    if((n = recvfrom(sockfd, recvPacket, sizeof(recvPacket), 0, (struct sockaddr *)&from, &fromlen)) < 0)
    {
        if(errno == EINTR)return;
        perror("recvfrom error");
        return;
    }
    gettimeofday(&tvrecv, NULL);
    if(unpack(recvPacket, n) == -1)return;
    nrecv++;
}


int unpack(char *buf, int len)
{
    int i, iphdrlen;
    struct ip *ip;
    struct icmp *icmp;
    struct timeval *tvsend;
    double rtt;

    ip = (struct ip *)buf;
    iphdrlen = (ip->ip_hl) * 4;
    icmp = (struct icmp *)(buf + iphdrlen);
    len -= iphdrlen;
    if(len < 8)
    {
        printf("ICMP packets \'s length is less than 9\n");
        return -1;
    }

    if(icmp->icmp_id == pid)
    {

        switch(icmp->icmp_type)
        {
        case ICMP_ECHOREPLY:
            tvsend = (struct timeval *)icmp->icmp_data;
            tv_sub(&tvrecv, tvsend);
            rtt = tvrecv.tv_sec * 1000 + (double)tvrecv.tv_usec / 1000;
            printf("%d byte form %s: icmp_seq=%u ttl=%d rtt=%.3f ms\n",
                   len, inet_ntoa(from.sin_addr), icmp->icmp_seq, ip->ip_ttl, rtt);
            break;
        case ICMP_DEST_UNREACH:
            switch(icmp->icmp_code)
            {
            case ICMP_NET_UNREACH:
                printf("Destination Net Unreachable\n");
                break;
            case ICMP_HOST_UNREACH:
                printf("Destination Host Unreachable\n");
                break;
            case ICMP_PROT_UNREACH:
                printf("Destination Protocol Unreachable\n");
                break;
            case ICMP_PORT_UNREACH:
                printf("Destination Port Unreachable\n");
                break;
            case ICMP_FRAG_NEEDED:
                printf("Frag needed and DF set \n");
                break;
            case ICMP_SR_FAILED:
                printf("Source Route Failed\n");
                break;
            case ICMP_NET_UNKNOWN:
                printf("Destination Net Unknown\n");
                break;
            case ICMP_HOST_UNKNOWN:
                printf("Destination Host Unknown\n");
                break;
            case ICMP_HOST_ISOLATED:
                printf("Source Host Isolated\n");
                break;
            case ICMP_NET_ANO:
                printf("Destination Net Prohibited\n");
                break;
            case ICMP_HOST_ANO:
                printf("Destination Host Prohibited\n");
                break;
            case ICMP_NET_UNR_TOS:
                printf("Destination Net Unreachable for Type of Service\n");
                break;
            case ICMP_HOST_UNR_TOS:
                printf("Destination Host Unreachable for Type of Service\n");
                break;
            case ICMP_PKT_FILTERED:
                printf("Packet filtered\n");
                break;
            case ICMP_PREC_VIOLATION:
                printf("Precedence Violation\n");
                break;
            case ICMP_PREC_CUTOFF:
                printf("Precedence Cutoff\n");
                break;
            default:
                printf("Dest Unreachable, Bad Code: %d\n", icmp->icmp_code);
                break;
            }
        default:
            return -1;
        }
    }
    else return -1;
}

void tv_sub(struct timeval *out, struct timeval *in)
{
    if((out->tv_usec -= in->tv_usec) < 0)
    {
        out->tv_sec--;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}

main(int argc, char **argv)
{
    struct hostent *host;
    struct protoent *protocol;
    unsigned long inaddr = 01;
    int waittime = MAX_WAIT_TIME;
    int size = 50 * 1024;
    fd_set rset;
    int nRet;
    struct timeval timeout;
    

    if(argc < 2)
    {
        printf("usage:%s hostname/IP address\n", argv[0]);
        exit(1);
    }
    if((protocol = getprotobyname("icmp")) == NULL)
    {
        perror("getprotobyname error");
        exit(1);
    }
    if((sockfd = socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0)
    {
        perror("socket() error");
        exit(1);
    }
    setuid(getuid());
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    if((host = gethostbyname(argv[1])) == NULL)
    {
        perror("gethostbyname() error");
        exit(1);

    }
    dest_addr.sin_addr = *((struct in_addr *)host->h_addr);
    pid = getpid();
    printf("PING %s(%s): %d bytes data in ICMP packets.\n",
           argv[1], inet_ntoa(dest_addr.sin_addr), datalen);
    while(1)
    {
        FD_ZERO(&rset);
        FD_SET(sockfd, &rset);

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        nRet = select(sockfd + 1, &rset, NULL, NULL, &timeout);
        if(nRet < 0)
        {
            //socket error
            perror("select() error");
            exit(1);
        }
        else if(nRet == 0 )
        {
            if(nsend < MAX_NO_PACKETS)send_packet();
            else statistics(SIGALRM);

        }
        else if(FD_ISSET(sockfd, &rset))
        {
            recv_packet();
        }
        fflush(0);
    }

    return 0;
}


