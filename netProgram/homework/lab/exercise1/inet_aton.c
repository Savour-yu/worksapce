#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>



main(){
	char str[100];
	struct in_addr inp;
	printf("Pease input a IP address:\n");
	scanf("%s",str);
  	if(inet_aton(str,&inp) == 1){
    	uint32_t netlong = (inp).s_addr;
      printf("Your input IP adress is:%x\n", ntohl(netlong));
  	}
  	else {
    	printf("WRONG INPUT\n");
  	}
}
