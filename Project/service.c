#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_STR 50
#define USAGE "service –n id –j ip -u upt –t tpt [-i csip] [-p cspt]"
#define DEFAULT_HOST "tejo.tecnico.ulisboa.pt"
#define DEFAULT_PORT 59000

void get_arguments (int argc, const char *argv[], int *id, char *ip, int *upt, int *tpt, char *csip, int *cspt);
void regist_on_central (int service, int fd_udp, int id, struct sockaddr_in addr_central, int *own_id, char *ip, int upt, int tpt, int *addrlen);
void serve_client (int fd_service, struct sockaddr_in *addr_client);

int main(int argc, char const *argv[]) {
  int id, upt, tpt, cspt, service=-1, ret, nread;
  int fd_udp, fd_service, addrlen, own_id, st_id, st_tpt;
  char ip[MAX_STR], csip[MAX_STR], st_ip[MAX_STR], toggle[MAX_STR], msg_out[MAX_STR], buffer[MAX_STR];
  struct sockaddr_in addr_central, addr_serve, addr_client;

  get_arguments (argc, argv, &id, ip, &upt, &tpt, csip, &cspt);

  printf("dummy: %d %s %d %d %s %d\n", id, ip, upt, tpt, csip, cspt);

  /* Create socket for communication with central. */
  fd_udp = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
	if(fd_udp == -1) {
    printf("Error: socket UDP");
    exit(1);//error
  }

  /* Create address of the central server. */
  memset((void*)&addr_central,(int)'\0',sizeof(addr_central));
	addr_central.sin_family=AF_INET;
	addr_central.sin_addr.s_addr = inet_addr(csip);
	addr_central.sin_port=htons(cspt);

  /* Create socket for the service to the client. */
  fd_service = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
	if (fd_service == -1) {
    printf("Error: socket serving");
    exit(1);//error
  }

  /* Binds client serving socket to the given address. */
  memset((void*)&addr_serve,(int)'\0',sizeof(addr_serve));
  addr_serve.sin_family=AF_INET;
  addr_serve.sin_addr.s_addr=inet_addr(ip);
  addr_serve.sin_port=htons(upt);

  ret=bind (fd_service,(struct sockaddr*)&addr_serve,sizeof(addr_serve));
  if(ret==-1) {
    printf("Error: bind \n");

    exit(1);//error
  }

  /* Regist server in central, with service x. */
  service = 4;
  regist_on_central(service, fd_udp, id, addr_central, &own_id, ip, upt, tpt, &addrlen);

  /* Read input. */
  /* TODO: parse args */

  /* Adress of the client being served. */
  memset((void*)&addr_client,(int)'\0',sizeof(addr_client));

  while(1){
    // Respond to a client request.
    serve_client(fd_service, &addr_client);
  }
  /* Exit. */
  close(fd_udp);
  close(fd_service);

  return 0;
}

void get_arguments (int argc, const char *argv[], int *id, char *ip, int *upt, int *tpt, char *csip, int *cspt) {
  int i;
  char ident;
  int csi = 0, csp = 0;
  struct hostent *h;
  struct in_addr *a;

  if (argc < 9) {
    printf("Error: incorrect number of arguments");
    exit(1);
  }

  for ( i = 1 ; i <= argc/2 ; i++){

    sscanf(argv[2*i-1],"-%c", &ident);

    switch(ident){
      case 'n' :
        sscanf(argv[2*i],"%d", id);
        break;
      case 'j' :
        strcpy(ip, argv[2*i]);
        break;
      case 'u' :
        sscanf(argv[2*i],"%d", upt);
        break;
      case 't' :
        sscanf(argv[2*i],"%d", tpt);
        break;
      case 'i' :
        strcpy(csip, argv[2*i]);
        csi = 1;
        break;
      case 'p' :
        sscanf(argv[2*i],"%d", cspt);
        csp = 1;
        break;
      default:
        printf("Error: Argument not known");
        exit(1);
    }
  }
    if (csi != 1) {
      if((h=gethostbyname(DEFAULT_HOST))==NULL) {
        exit(1);
      }

      a=(struct in_addr*) h->h_addr_list[0];
      sprintf(csip, "%s", inet_ntoa(*a), (long unsigned int) ntohl(a->s_addr));
    }

    if (csp != 1) {
      *cspt = DEFAULT_PORT;
    }


  }

void regist_on_central (int service, int fd_udp, int id, struct sockaddr_in addr_central, int *own_id, char *ip, int upt, int tpt, int *addrlen) {

  char buffer[MAX_STR], msg_out[MAX_STR], msg_type[MAX_STR], msg_data[MAX_STR];
  int nsend, nrecv;

  /* Regist this service server in the central server (UDP). */
  // get start server
  sprintf(msg_out, "GET_START %d;%d", service, id);
  nsend = sendto(fd_udp, msg_out, strlen(msg_out), 0, (struct sockaddr*)&addr_central, sizeof(addr_central));
	if( nsend == -1 ) {
    printf("Error: send");
    exit(1); //error
  }
  *addrlen = sizeof(addr_central); // rewrite address????
	nrecv = recvfrom(fd_udp, buffer, 128, 0, (struct sockaddr*)&addr_central, addrlen);
	if( nrecv == -1 ) {
    printf("Error: recv");
    exit(1);//error
  }
  buffer[nrecv] = '\0';
  printf("%s\n", buffer);

  sscanf(buffer, "%s %d;%s", msg_type, own_id, msg_data);
  if (strcmp(msg_type, "OK") != 0) {
    printf("Erro: msg\n");
  } else {
    if (strcmp(msg_data, "0;0.0.0.0;0") == 0) {
      /* This is the start server. */

      // set start
      sprintf(msg_out, "SET_START %d;%d;%s;%d", service, id, ip, tpt);
      nsend = sendto(fd_udp, msg_out, strlen(msg_out), 0, (struct sockaddr*)&addr_central, sizeof(addr_central));
    	if( nsend == -1 ) {
        printf("Error: send");
        exit(1); //error
      }
      *addrlen = sizeof(addr_central); // rewrite address????
    	nrecv = recvfrom(fd_udp, buffer, 128, 0, (struct sockaddr*)&addr_central, addrlen);
    	if( nrecv == -1 ) {
        printf("Error: recv");
        exit(1);//error
      }
      buffer[nrecv] = '\0';
      printf("%s\n", buffer);
    } else {
      // Save the start server.

    }
  }

  /* Set the server to dispatch */
  sprintf(msg_out, "SET_DS %d;%d;%s;%d", service, id, ip, upt);
  nsend = sendto(fd_udp, msg_out, strlen(msg_out), 0, (struct sockaddr*)&addr_central, sizeof(addr_central));
  if( nsend == -1 ) {
    printf("Error: send");
    exit(1); //error
  }
  *addrlen = sizeof(addr_central); // rewrite address????
  nrecv = recvfrom(fd_udp, buffer, 128, 0, (struct sockaddr*)&addr_central, addrlen);
  if( nrecv == -1 ) {
    printf("Error: recv");
    exit(1);//error
  }
  buffer[nrecv] = '\0';
  printf("%s\n", buffer);
}

void serve_client (int fd_service, struct sockaddr_in *addr_client) {
  int ret, nread, addrlen;
  char toggle[MAX_STR], msg_in[MAX_STR], msg_out[MAX_STR];

  addrlen = sizeof(struct sockaddr_in);

  nread=recvfrom(fd_service,msg_in,128,0,(struct sockaddr*)addr_client,&addrlen);
  if(nread==-1)exit(1);//error

  msg_in[nread] = '\0';
  printf("%s\n", msg_in);

  sscanf(msg_in, "MY_SERVICE %s", toggle);

  if (strcmp(toggle, "ON") == 0){
    sprintf(msg_out, "YOUR_SERVICE ON");
    ret=sendto(fd_service,msg_out,strlen(msg_out),0,(struct sockaddr*)addr_client,addrlen);
    if(ret==-1)exit(1);//error
  } else if (strcmp(toggle, "OFF") == 0) {
    sprintf(msg_out, "YOUR_SERVICE OFF");
    ret=sendto(fd_service,msg_out,strlen(msg_out),0,(struct sockaddr*)addr_client,addrlen);
    if(ret==-1)exit(1);//error
  } else {
    /* TODO: invalid message. */
  }
}
