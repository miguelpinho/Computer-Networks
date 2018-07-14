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
	int nbytes, nleft, nwritten, nread;
	struct sockaddr_in addr;
	struct hostent *h;
	struct in_addr *a;
	char *ptr, buffer[128];

	/* ... */
	fd=socket(AF_INET,SOCK_STREAM,0); //TCP socket
	if(fd==-1) exit(1);//error

	memset((void*)&addr,(int)'\0',sizeof(addr));
	addr.sin_family=AF_INET;

	if((h=gethostbyname("tejo"))==NULL) exit(1); //error
	a=(struct in_addr*) h->h_addr_list[0];

	//memcpy(&(addr.sin_addr), a, sizeof(struct in_addr));
	addr.sin_addr=*a;
	addr.sin_port=htons(58000);

	n=connect(fd, (struct sockaddr*)&addr,sizeof(addr));
	if(n==-1) exit(1); //error

	ptr=strcpy(buffer, "Hello!\n");
	nbytes=7;

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