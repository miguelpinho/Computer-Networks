/* Ver código client.c do lab 2. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_STR 50
#define DEFAULT_HOST "tejo.tecnico.ulisboa.pt"
#define DEFAULT_PORT 59000

void get_arguments (int argc, const char *argv[], char *csip, int *cspt);
void request_service(int service, int fd_udp, struct sockaddr_in addr_central, int *addrlen, int *id, char *ip, int *upt);

int main(int argc, char const *argv[])
{
	int fd_udp, n, addrlen, cspt, service, id, upt;
	struct sockaddr_in addr_central;
	char buffer[MAX_STR], csip[MAX_STR], ip[MAX_STR];

  get_arguments ( argc, argv, csip, &cspt);

	/* ... */
	fd_udp=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
	if(fd_udp==-1) {
    printf("Error: socket");
    exit(1);//error
  }

	memset((void*)&addr_central,(int)'\0',sizeof(addr_central));
	addr_central.sin_family=AF_INET;
	addr_central.sin_addr.s_addr = inet_addr(csip);
	addr_central.sin_port=htons(cspt);

  /* Getting terminal input */
  service = 4;
  request_service(service, fd_udp, addr_central, &addrlen, &id, ip, &upt);
}

  void get_arguments (int argc, const char *argv[], char *csip, int *cspt) {
    int i;
    char ident;
    int csi = 0, csp = 0;
    struct hostent *h;
    struct in_addr *a;

    if (argc < 1) {
      printf("Error: incorrect number of arguments");
      exit(1);
    }

    for ( i = 1 ; i <= argc/2 ; i++){

      sscanf(argv[2*i-1],"-%c", &ident);

      switch(ident){

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

    void request_service(int service, int fd_udp, struct sockaddr_in addr_central, int *addrlen, int *id, char *ip, int *upt) {

      char buffer[MAX_STR], msg_out[MAX_STR], msg_type[MAX_STR], msg_data[MAX_STR];
      int nsend, nrecv, fd_udp_serv, ip1, ip2, ip3, ip4;
      struct sockaddr_in addr_service;

      // Check if there is one server with the wanted service
      sprintf(msg_out, "GET_DS_SERVER %d", service);
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

      sscanf(buffer, "%s %d;%s", msg_type, id, msg_data);
      printf("dummy scan: %s\n", msg_data);
      if (strcmp(msg_type, "OK") != 0) {
        printf("Erro: msg\n");
      } else {
        // Service found
        if (strcmp(msg_data, "0.0.0.0;0") != 0 && *id != 0) {

          sscanf(msg_data, "%d.%d.%d.%d;%d", &ip1, &ip2, &ip3, &ip4, upt);
          sprintf(ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

          printf("dummy req: %d;%s;%d\n", *id, ip, *upt);

          fd_udp_serv=socket(AF_INET,SOCK_DGRAM,0); //UDP socket
        	if(fd_udp_serv==-1) {
            printf("Error: socket");
            exit(1);//error
          }

          memset((void*)&addr_service,(int)'\0',sizeof(addr_service));
        	addr_service.sin_family=AF_INET;
        	addr_service.sin_addr.s_addr = inet_addr(ip);
        	addr_service.sin_port=htons(*upt);

          sprintf(msg_out, "MY_SERVICE ON");
          nsend = sendto(fd_udp_serv, msg_out, strlen(msg_out), 0, (struct sockaddr*)&addr_service, sizeof(addr_service));
        	if( nsend == -1 ) {
            printf("Error: send");
            exit(1); //error
          }
          *addrlen = sizeof(addr_service); // rewrite address????
        	nrecv = recvfrom(fd_udp_serv, buffer, 128, 0, (struct sockaddr*)&addr_service, addrlen);
        	if( nrecv == -1 ) {
            printf("Error: recv");
            exit(1);//error
          }
          buffer[nrecv] = '\0';
          printf("%s\n", buffer);
        } else {
          // No server providing the Service
          printf("No server providing that service\n");
          //Error????

        }
      }



    }
