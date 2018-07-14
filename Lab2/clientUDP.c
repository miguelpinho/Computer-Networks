#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define IP_SERVER "194.210.228.226"
#define MESSAGE "Oh migaaaaa, queres uma tosta mista?\n"

int main(void)
{
	int fd, n, addrlen;
	struct sockaddr_in addr_server;
	char buffer[128];

	/* ... */
	fd=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
	if(fd==-1) {
    printf("Error: socket");
    exit(1);//error
  }

	memset((void*)&addr_server,(int)'\0',sizeof(addr_server));
	addr_server.sin_family=AF_INET;
	addr_server.sin_addr.s_addr = inet_addr(IP_SERVER);
	addr_server.sin_port=htons(9000);

  n=sendto(fd, MESSAGE, strlen(MESSAGE), 0, (struct sockaddr*)&addr_server,sizeof(addr_server));
	if(n==-1) {
    printf("Error: send");
    exit(1);//error
  }

	addrlen=sizeof(addr_server);
	n=recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr_server,&addrlen);
	if(n==-1) {
    printf("Error: recv");
    exit(1);//error
  }

	close(fd);

  buffer[n] = '\0';
  printf("echo: %s", buffer);

	exit(0);
}
