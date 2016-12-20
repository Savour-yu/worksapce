#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <fcntl.h>//import open method 
#include "link.h"

#define PORT 2223
#define BACKLOG 5
#define MAXDATASIZE 4096
#define FILENAME_MAX_SIZE 512

void *sig_chld(int a);

static mlink *header;
main()
{
    char buf[MAXDATASIZE], filename[FILENAME_MAX_SIZE];
    int num; // length of rec
    int fileid;//file id

    /*init var*/
    int listenfd, connectfd;
    struct sockaddr_in server, client;
    socklen_t addrlen;
    pid_t pid;
    header = malloc(sizeof(link));
    header->pid = 0;
    header->next = NULL;
    /*creat a tcp socket*/
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /*handle exception*/
        perror("socket()error.");
        exit(1);
    }

    // set socket option
    int opt = SO_REUSEADDR;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("Bind()error");
        exit(1);
    }
    if(listen(listenfd, BACKLOG) == -1)
    {
        perror("listen()error");
        exit(1);


    }
    signal(SIGCHLD, sig_chld);
    while(1)
    {

        addrlen = sizeof(client);
        if((connectfd = accept(listenfd,
                               (struct sockaddr *)&client, &addrlen)) == -1)
        {

            perror("accept()error");
            exit(1);
        }
        // if((pid = waitpid(-1, NULL, WNOHANG)) > 0) 	//-1:no dead child; positive num : first dead child
        // {
        //     printf("Child PROCESS %d dead\n", pid);
        // }
        //parent process
        if((pid = fork()) > 0)
        {
            insert(header, pid);
            close(connectfd);
            continue;
        }
        //child process
        else if(pid == 0)
        {
            close(listenfd);
            printf("[PROCESS ID %d]", getpid());
            printf("a connetion form ip: %s, port: %d\n", inet_ntoa(client.sin_addr), htons(client.sin_port));


            if(read(connectfd, filename, sizeof(filename)) < 0)
            {
                perror("readline()error");
                exit(1);

            }

            printf("[PROCESS ID %d]", getpid());
            printf("Recieving a file named: %s\n", filename);
            fileid = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 00777);

            if(fileid < 0)
            {
                //write only | if there is no that file then creat it  |
                //if open that file and write only successfully then clear that file
                //0644 other users can read | file owner can read and wirte | user group can read

                perror("Open file failed");
                exit(1);
            }
            printf("[PROCESS ID %d]", getpid());
            printf("Recieving...\n");
            bzero(buf, MAXDATASIZE);

            while(num = read(connectfd, buf, sizeof(buf)))
            {
                if(num < 0)
                {
                    perror("read()error");
                    exit(1);
                }


                if(write(fileid, buf, num) < 0)
                {
                    perror("write to file error");
                    exit(1);
                }



                //printf("%d\n",num );
                bzero(buf, MAXDATASIZE);


            }

            printf("[PROCESS ID %d]", getpid());
            printf("Recieve Successful\n");

            close(fileid);
            close(connectfd);
            printf("[PROCESS ID %d]", getpid());
            printf("connection break\n");
            fflush(stdin);


            exit(0);

        }
        //error
        else
        {
            printf("fork error\n");
            exit(0);
        }


    }
    close(listenfd);
}


void *sig_chld(int a)
{
    mlink *cur = header->next;
    while(cur)
    {
        if((waitpid(cur->pid, NULL, WNOHANG)) > 0)
        {
            printf("Child PROCESS %d dead\n", cur->pid);
            delete(header, cur->pid);
            cur = header;
        }
        cur = cur->next;
    }
}