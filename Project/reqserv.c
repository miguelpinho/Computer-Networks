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

void get_arguments(int argc, const char *argv[], char *csip, int *cspt);
void request_service(int service, int fd_udp, struct sockaddr_in addr_central, socklen_t *addrlen, int *id, char *ip, int *upt);

int main(int argc, char const *argv[])
{
	int fd_udp, cspt, service, id, upt;
	socklen_t addrlen;
	struct sockaddr_in addr_central;
	char csip[MAX_STR], ip[MAX_STR];

  get_arguments(argc, argv, csip, &cspt);

	/* ... */
	fd_udp = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd_udp == -1) {
    printf("Error: socket");
    exit(1); /*error*/
  }

	memset((void*) &addr_central, (int) '\0', sizeof(addr_central));
	addr_central.sin_family = AF_INET;
	addr_central.sin_addr.s_addr = inet_addr(csip);
	addr_central.sin_port = htons(cspt);

  /* Getting terminal input */
  service = 4;
  request_service(service, fd_udp, addr_central, &addrlen, &id, ip, &upt);

	return 0;
}

void get_arguments (int argc, const char *argv[], char *csip, int *cspt) {
  int i;
  char ident;
  int csi = 0, csp = 0;
  struct hostent *h;
  struct in_addr *addr_cs;

  if (argc < 1) {
    printf("Error: incorrect number of arguments");
    exit(1);
  }

  for (i = 1; i < argc; i+=2) {
    sscanf(argv[i],"-%c", &ident);

    switch(ident){
      case 'i' :
        strcpy(csip, argv[i+1]);
        csi = 1;
        break;
      case 'p' :
        sscanf(argv[i+1],"%d", cspt);
        csp = 1;
        break;
      default:
        printf("Error: Argument not known");
        exit(1);
    }
  }

  if (csi != 1) {
    if ((h = gethostbyname(DEFAULT_HOST)) == NULL) {
      exit(1);
    }

    addr_cs = (struct in_addr*) h->h_addr_list[0];
    sprintf(csip, "%s", inet_ntoa(*addr_cs));
  }

  if (csp != 1) {
    *cspt = DEFAULT_PORT;
  }
}

void request_service(int service, int fd_udp, struct sockaddr_in addr_central, socklen_t *addrlen, int *id, char *ip, int *upt) {
  char msg_in[MAX_STR], msg_out[MAX_STR], msg_type[MAX_STR], msg_data[MAX_STR], *split, aux[3][MAX_STR];
  int nsend, nrecv, fd_udp_serv, i = 0;
  struct sockaddr_in addr_service;

  /* Check if there is one server with the wanted service. */
  sprintf(msg_out, "GET_DS_SERVER %d", service);
  nsend = sendto(fd_udp, msg_out, strlen(msg_out), 0, (struct sockaddr*) &addr_central, sizeof(addr_central));
	if( nsend == -1 ) {
    printf("Error: send");
    exit(1); /*error*/
  }
  *addrlen = sizeof(addr_central);
	nrecv = recvfrom(fd_udp, msg_in, 128, 0, (struct sockaddr*) &addr_central, addrlen);
	if( nrecv == -1 ) {
    printf("Error: recv");
    exit(1); /*error*/
  }
  msg_in[nrecv] = '\0';
  printf("%s\n", msg_in);

  sscanf(msg_in, "%s %d;%s", msg_type, id, msg_data);
  printf("dummy scan: %s\n", msg_data);
  if (strcmp(msg_type, "OK") != 0) {
    printf("Erro: msg\n");
  } else {
    /* Service found */
    if (strcmp(msg_data, "0.0.0.0;0") != 0 && *id != 0) {

			split = strtok(msg_data, ";");
			while (split != NULL)
			{
		    strcpy(aux[i], split);
		    split = strtok (NULL, ";");
				i++;
		  }

			strcpy(ip, aux[0]);
			sscanf(aux[1], "%d", upt);

      printf("dummy req: %d;%s;%d\n", *id, ip, *upt);

      fd_udp_serv = socket(AF_INET, SOCK_DGRAM, 0);
    	if (fd_udp_serv == -1) {
        printf("Error: socket");
        exit(1); /*error*/
      }

      memset((void*) &addr_service, (int) '\0', sizeof(addr_service));
    	addr_service.sin_family = AF_INET;
    	addr_service.sin_addr.s_addr = inet_addr(ip);
    	addr_service.sin_port = htons(*upt);

      sprintf(msg_out, "MY_SERVICE ON");
      nsend = sendto(fd_udp_serv, msg_out, strlen(msg_out), 0, (struct sockaddr*)&addr_service, sizeof(addr_service));
    	if( nsend == -1 ) {
        printf("Error: send");
        exit(1); /*error*/
      }
      *addrlen = sizeof(addr_service);

    	nrecv = recvfrom(fd_udp_serv, msg_in, 128, 0, (struct sockaddr*)&addr_service, addrlen);
    	if( nrecv == -1 ) {
        printf("Error: recv");
        exit(1); /*error*/
      }
      msg_in[nrecv] = '\0';
      printf("%s\n", msg_in);
    } else {
      /* No server providing the Service */
      printf("No server providing that service\n");

			/*Error????*/
    }
  }
}
