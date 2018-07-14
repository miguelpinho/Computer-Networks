#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
  int fd, addrlen, newfd;
  struct sockaddr_in addr_server, addr_client;
  int n, nw;
  char *ptr, buffer[128];

  if((fd=socket(AF_INET,SOCK_STREAM,0))==-1)exit(1);//error

  memset((void*)&addr_server,(int)'\0',sizeof(addr_server));

  addr_server.sin_family=AF_INET;
  addr_server.sin_addr.s_addr=htonl(INADDR_ANY);
  addr_server.sin_port=htons(9000);

  if(bind(fd,(struct sockaddr*)&addr_server,sizeof(addr_server))==-1)exit(1);//error

  if(listen(fd,5)==-1)exit(1);//error
  while(1){
  addrlen=sizeof(addr_client);
  if((newfd=accept(fd,(struct sockaddr*)&addr_client,&addrlen))==-1)
  exit(1);//error

  while((n=read(newfd,buffer,128))!=0){
   if(n==-1)exit(1);//error
      ptr=&buffer[0];
      while(n>0){
        if((nw=write(newfd,ptr,n))<=0)exit(1);//error
        n-=nw; ptr+=nw;
      }
   }
   close(newfd);
   }
  /* close(fd); exit(0); */}
