//test.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	int fd, n, addrlen;
	int nbytes, nleft, nwritten, nread;
	struct sockaddr_in addr_server;
	struct hostent *h;
	struct in_addr *a;
	char *ptr, buffer[128];

	#define IP_SERVER "194.210.134.31"
	#define MESSAGE "Oh migaaaaa, queres uma tosta mista?\n"

	fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket
	if(fd==-1) exit(1);//error

	memset((void*)&addr_server,(int)'\0',sizeof(addr_server));
	addr_server.sin_family=AF_INET;

	//memcpy(&(addr.sin_addr), a, sizeof(struct in_addr));
	addr_server.sin_addr.s_addr=inet_addr(IP_SERVER);
	addr_server.sin_port=htons(9000);

	n=connect(fd, (struct sockaddr*)&addr_server,sizeof(addr_server));
	if(n==-1) exit(1); //error

	ptr=strcpy(buffer, MESSAGE);
	nbytes=strlen(MESSAGE);

	nleft=nbytes;
	while(nleft>0) {
		nwritten=write(fd,ptr,nleft);
		if(nwritten<=0) exit(1);//error

		nleft-=nwritten;
		ptr+=nwritten;
	}

	nleft=nbytes;
	ptr=buffer;
	while(nleft>0) {
		nread=read(fd,ptr,nleft);
		if(nread==-1) exit(1); //error
		else if(nread==0) break; //closed by peer
		nleft-=nread;
		ptr+=nread;
	}
	nread=nbytes-nleft;

	close(fd);

	write(1,"echo: ",6); //stdout
	write(1,buffer,nread);

	exit(0);
}
