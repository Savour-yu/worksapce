#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>



main(){
	char str[100];
	struct in_addr inp;
	printf("Pease input a IPv4 address:\n");
	scanf("%s",str);
  int a = inet_pton(AF_INET,str,&inp);
  	if(a == 1)
    {
      uint32_t netlong = (inp).s_addr;
    	printf("Your input IP adress is:%x\n", ntohl(netlong));
  	}
  	else if(a == 0){
    	printf("WRONG INPUT\n");
  	}
    else{
      printf("ERROR\n");
    }
}
