//test.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main(void)
{
	int fd, n, addrlen;
	struct sockaddr_in addr;
	struct hostent *h;
	struct in_addr *a;
	char buffer[128];

	/* ... */
	fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
	if(fd==-1)exit(1);//error

	memset((void*)&addr,(int)'\0',sizeof(addr));
	addr.sin_family=AF_INET;

	if((h=gethostbyname("tejo"))==NULL) exit(1); //error
	a=(struct in_addr*) h->h_addr_list[0];

	//memcpy(&(addr.sin_addr), a, sizeof(struct in_addr));
	addr.sin_addr=*a;
	addr.sin_port=htons(58000);

	n=sendto(fd,"Hello!\n",7,0,(struct sockaddr*)&addr,sizeof(addr));
	if(n==-1)exit(1);//error

	addrlen=sizeof(addr);
	n=recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
	if(n==-1) exit(1);//error

	write(1,"echo: ",6);//stdout
	write(1,buffer,n);

	close(fd);
	exit(0);
}