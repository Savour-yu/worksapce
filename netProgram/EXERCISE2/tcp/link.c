#include "link.h"

int main()
{
    link *header = malloc(sizeof(link));
    int i = 1;
    for(; i < 9; i++)insert(header, (int)i);

    delete(header, 8);
    link *cur = header->next;
    while(cur)
    {
        printf("%d\n", cur->pid);

        if(cur->pid == 5)
        {
            delete(header, 5);
            cur = header;
        }
        cur = cur->next;
    }
    while(cur)
    {
        printf("%d\n", cur->pid);

        cur = cur->next;
    }
    return 0;
}