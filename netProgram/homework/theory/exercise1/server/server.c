#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>//import open method 
#include "link.h"

#define PORT 2223
#define BACKLOG 5
#define MAXDATASIZE 4096
#define FILENAME_MAX_SIZE 512
#define MAXFD 64
extern int daemon_proc; /*defined in error.c*/
char path[FILENAME_MAX_SIZE];
void daemon_init(const char *pname, int facility)
{
    int i;
    pid_t pid;
    if ((pid = fork()) != 0) exit(0);
    setsid();
    signal(SIGHUP, SIG_IGN);
    if ((pid = fork()) != 0) exit(0);
    getcwd(path, FILENAME_MAX_SIZE);
    strcat(path,"/");
    chdir("/");
    umask(0);
    for (i = 0; i < MAXFD; i++) close(i);
    openlog(pname, LOG_PID, facility);
}
void *sig_chld(int a);

static mlink *header;
main(int argc, char **argv)
{
    char buf[MAXDATASIZE], filename[FILENAME_MAX_SIZE];
    int num; // length of rec
    int fileid;//file id
    char temppath[FILENAME_MAX_SIZE];
    /*init var*/
    int listenfd, connectfd;
    struct sockaddr_in server, client;
    socklen_t addrlen;
    pid_t pid;
    header = malloc(sizeof(link));
    header->pid = 0;
    header->next = NULL;


    daemon_init(argv[0], 0);
    /*creat a tcp socket*/
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /*handle exception*/
        syslog(LOG_NOTICE | LOG_LOCAL0, "socket()error.");
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
        syslog(LOG_NOTICE | LOG_LOCAL0, "Bind()error");
        exit(1);
    }
    if(listen(listenfd, BACKLOG) == -1)
    {
        syslog(LOG_NOTICE | LOG_LOCAL0, "listen()error");
        exit(1);


    }
    signal(SIGCHLD, sig_chld);
    while(1)
    {

        addrlen = sizeof(client);
        if((connectfd = accept(listenfd,
                               (struct sockaddr *)&client, &addrlen)) == -1)
        {

            syslog(LOG_NOTICE | LOG_LOCAL0, "accept()error");
            exit(1);
        }
        // if((pid = waitpid(-1, NULL, WNOHANG)) > 0)   //-1:no dead child; positive num : first dead child
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
            syslog(LOG_NOTICE | LOG_LOCAL0, "[PROCESS ID %d]", getpid());
            syslog(LOG_NOTICE | LOG_LOCAL0, "a connetion form ip: %s, port: %d\n", inet_ntoa(client.sin_addr), htons(client.sin_port));


            if(read(connectfd, filename, sizeof(filename)) < 0)
            {
                syslog(LOG_NOTICE | LOG_LOCAL0, "readline()error");
                exit(1);

            }
            bzero(temppath,FILENAME_MAX_SIZE);
            strcpy(temppath, path);
            strcat(temppath, filename);
            syslog(LOG_NOTICE | LOG_LOCAL0, "[PROCESS ID %d]", getpid());
            syslog(LOG_NOTICE | LOG_LOCAL0, "Recieving a file named: %s\n", temppath);

            fileid = open(temppath, O_WRONLY | O_CREAT | O_TRUNC, 00777);

            if(fileid < 0)
            {
                //write only | if there is no that file then creat it  |
                //if open that file and write only successfully then clear that file
                //0644 other users can read | file owner can read and wirte | user group can read

                syslog(LOG_NOTICE | LOG_LOCAL0, "Open file %s failed", temppath);
                exit(1);
            }
            syslog(LOG_NOTICE | LOG_LOCAL0, "[PROCESS ID %d]", getpid());
            syslog(LOG_NOTICE | LOG_LOCAL0, "Recieving...\n");
            bzero(buf, MAXDATASIZE);

            while(num = read(connectfd, buf, sizeof(buf)))
            {
                if(num < 0)
                {
                    syslog(LOG_NOTICE | LOG_LOCAL0, "read()error");
                    exit(1);
                }


                if(write(fileid, buf, num) < 0)
                {
                    syslog(LOG_NOTICE | LOG_LOCAL0, "write to file error");
                    exit(1);
                }



                //printf("%d\n",num );
                bzero(buf, MAXDATASIZE);


            }

            syslog(LOG_NOTICE | LOG_LOCAL0, "[PROCESS ID %d]", getpid());
            syslog(LOG_NOTICE | LOG_LOCAL0, "Recieve Successful\n");

            close(fileid);
            close(connectfd);
            syslog(LOG_NOTICE | LOG_LOCAL0, "[PROCESS ID %d]", getpid());
            syslog(LOG_NOTICE | LOG_LOCAL0, "connection break\n");
            fflush(stdin);


            exit(0);

        }
        //error
        else
        {
            syslog(LOG_NOTICE | LOG_LOCAL0, "fork error\n");
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
            syslog(LOG_NOTICE | LOG_LOCAL0, "Child PROCESS %d dead\n", cur->pid);
            delete(header, cur->pid);
            cur = header;
        }
        cur = cur->next;
    }
}
