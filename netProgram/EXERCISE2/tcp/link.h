#ifndef _LINK_H_
#define _LINK_H_
#include <stdio.h>
#include <sys/types.h>


typedef struct mLink
{
    pid_t pid;
    struct mLink *next;
}mlink;

mlink *insert(const mlink *header, pid_t pid)
{
    mlink *current = header;
    while(current->next) current = current->next;
    mlink *i = (mlink *)malloc(sizeof(mlink));
    i->pid = pid;
    i->next = NULL;
    current->next = i;
    return i;
}

mlink *delete(const mlink *header, pid_t pid)
{
    mlink *current = header;
    mlink *temp = NULL;
    while(current->next)
    {
        if (current->next->pid == pid)
        {
            if(current->next->next)
            {
                temp = current->next;
                current->next = current->next->next;
                free(temp);
            }
            else
            {
                temp = current->next;
                current->next = NULL;
                free(temp);
                break;
            }
        }
        current = current->next;
    }
    return header;
}

#endif