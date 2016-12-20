#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <fcntl.h>//import open method 

#define PORT 2223

#define MAXDATASIZE 4096
#define FILENAME_MAX_SIZE 512


struct ARG
{
    char filename[FILENAME_MAX_SIZE];
    struct hostent *h;

};
void *process(void *arg)
{

    struct ARG *info;
    info = (struct ARG *)malloc(sizeof(struct ARG));
    memcpy(info, arg, sizeof(struct ARG));
    // info = (struct ARG*)arg;

    int sockfd;
    struct sockaddr_in server;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket()error\n");
        exit(1);

    }
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = *((struct in_addr *)(info->h)->h_addr);
    if(connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("connect()error\n");
        exit(1);

    }

    int fileId;//fileid: open file descirb
    char buf[MAXDATASIZE];
    int num;
    pthread_t tid = pthread_self();
    printf("[pthread ID %lu]", (unsigned int)tid );
    printf("start sending the file:%s\n", info->filename);
    if((fileId = open(info->filename, O_RDONLY)) < 0)
    {
        perror("open()error\n");
        exit(1);
    }

    write(sockfd, info->filename, sizeof(info->filename));/*send the filename*/
    printf("[pthread ID %lu]", (unsigned int)tid );
    printf("sending %s...\n", info->filename);

    while(num = read(fileId, buf, MAXDATASIZE))
    {
        if(num < 0)
        {
            perror("read error");
            exit(1);
        }

        if(write(sockfd, buf, num) < 0)
        {
            perror("write to socket error");
            exit(1);
        }

        bzero(buf, MAXDATASIZE);
    }
    printf("[pthread ID %lu]", tid );

    printf("sending file over\n");
    close(fileId);
    close(sockfd);
    printf("[pthread ID %lu]", (unsigned int)tid );
    printf("socket closed\n");
    free(arg);
    printf("[pthread ID %lu]exit\n", (unsigned int)tid );

    pthread_exit(NULL);

}
int main(int argc, char *argv[])
{
    struct hostent *he;
    he = (struct hostent *)malloc(sizeof(struct hostent));

    struct ARG *arg;

    if(argc < 3 || argc > 7)
    {
        printf("Wrong input style\nPlease input like that:\n./ex2-client 127.0.0.1 + some files split by blankspace\n");
        exit(1);
    }



    if((he = gethostbyname(argv[1])) == NULL)
    {
        perror("gethostbyname()error\n");
        exit(1);
    }


    int fileNum = argc - 2;
    int i;
    pthread_t tid[fileNum];




    for(i = 0; i < fileNum; ++i)
    {
        arg = (struct ARG *)malloc(sizeof(struct ARG));
        arg->h = (struct hostent *)malloc(sizeof(struct hostent));
        memcpy(arg->filename, argv[i + 2], strlen(argv[i + 2]));
        (arg->filename)[strlen(argv[i + 2])] = '\0';
        memcpy((arg->h), he, sizeof(struct hostent));
        if(pthread_create((tid + i), NULL, process, (void *)arg))
        {
            perror("pthread_create()error\n");
            exit(1);
        }
    }
    for(i = 0; i < fileNum; ++i)
    {
        pthread_join(*(tid + i), NULL);
    }
}