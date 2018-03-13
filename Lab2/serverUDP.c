#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

int main(void)
{
  int fd, ret, nread;
  struct sockaddr_in addr_server, addr_client;
  char buffer[128];
  socklen_t addrlen;

  if((fd=socket(AF_INET,SOCK_DGRAM,0))==-1)exit(1);//error
  memset((void*)&addr_server,(int)'\0',sizeof(addr_server));
  addr_server.sin_family=AF_INET;
  addr_server.sin_addr.s_addr=htonl(INADDR_ANY);
  addr_server.sin_port=htons(9000);

  ret=bind(fd,(struct sockaddr*)&addr_server,sizeof(addr_server));
  if(ret==-1) {
    printf("Error: bind \n");

    exit(1);//error
  }

  memset((void*)&addr_client,(int)'\0',sizeof(addr_client));

  while(1){
    addrlen = sizeof(addr_client);

    nread=recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr_client,&addrlen);
    if(nread==-1)exit(1);//error

    ret=sendto(fd,buffer,nread,0,(struct sockaddr*)&addr_client,addrlen);
    if(ret==-1)exit(1);//error
  }

  close(fd);
  exit(0);
}
