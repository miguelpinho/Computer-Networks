#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_STR 50
#define DEFAULT_HOST "tejo.tecnico.ulisboa.pt"
#define DEFAULT_PORT 59000
#define max(A,B) ((A)>=(B)?(A):(B))

enum input_type {IN_ERROR, IN_RS, IN_TS, IN_EXIT, IN_NO_RS, IN_NO_TS};

void get_arguments(int argc, const char *argv[], char *csip, int *cspt);
int parse_user_input(int *service);
void request_service(int *service, int fd_udp, int fd_udp_serv, struct sockaddr_in addr_central, struct sockaddr_in *addr_service, socklen_t *addrlen, int *id, char *ip, int *upt);
void terminate_service(int fd_udp_serv, struct sockaddr_in addr_service, socklen_t *addrlen);

int main(int argc, char const *argv[])
{
	int fd_udp, fd_udp_serv, cspt, service = -1, id, upt, exit_f = 0, sel_in, max_fd = 0;
	socklen_t addrlen;
	struct sockaddr_in addr_central, addr_service;
	char csip[MAX_STR], ip[MAX_STR];
	fd_set rfds;
	int counter;

	struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  get_arguments(argc, argv, csip, &cspt);

	/* ... */
	fd_udp = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd_udp == -1) {
    printf("Error: socket");
    exit(1); /*error*/
  }

	if (setsockopt ( fd_udp, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
									 sizeof(timeout)) < 0 ) {
		perror("Error: timeout central socket\nDescription:");
		exit(1); /* error */
	}

	fd_udp_serv = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd_udp_serv == -1) {
		printf("Error: socket");
		exit(1); /*error*/
	}

	if (setsockopt ( fd_udp_serv, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
									 sizeof(timeout)) < 0 ) {
		perror("Error: timeout central socket\nDescription:");
		exit(1); /* error */
	}

	memset((void*) &addr_central, (int) '\0', sizeof(addr_central));
	addr_central.sin_family = AF_INET;
	addr_central.sin_addr.s_addr = inet_addr(csip);
	addr_central.sin_port = htons(cspt);

	while(exit_f == 0){
		/* Prepare select, to monitor stdin and the sockets */
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		FD_SET(fd_udp_serv, &rfds); max_fd = fd_udp_serv;
		FD_SET(fd_udp, &rfds); max_fd = max(max_fd, fd_udp);

		counter = select(max_fd+1, &rfds, (fd_set*) NULL, (fd_set*) NULL, (struct timeval *) NULL);
		if (counter <= 0) {
      printf("Error: select\n");

      exit(1); /*error*/
    }

		if (FD_ISSET(fd_udp, &rfds)) {
      /* TODO: ignorar mensagem fora de tempo */

    }

		if (FD_ISSET(0, &rfds)) {

			sel_in = parse_user_input(&service);

			switch(sel_in){
				case IN_RS:
					request_service(&service, fd_udp, fd_udp_serv, addr_central, &addr_service, &addrlen, &id, ip, &upt);
					break;
				case IN_TS:
					terminate_service(fd_udp_serv, addr_service, &addrlen);
					break;
				case IN_EXIT:
					exit_f = 1;
					if (service != -1) {
						terminate_service(fd_udp_serv, addr_service, &addrlen);
					}
					break;
				case IN_ERROR:
					printf("Error: Client interface\n");
					exit(1);
					break;
				case IN_NO_RS:
					printf("One service is already running\n");
					break;
				case IN_NO_TS:
					printf("No service is running\n");
					break;
				}
			}
		if(FD_ISSET(fd_udp_serv, &rfds)) {
			/* No idea what to do here */

		}
	}

	close(fd_udp);
	close(fd_udp_serv);
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

int parse_user_input(int *service) {
  char buffer[MAX_STR], cmd[MAX_STR];
	int arg_read, char_read;

  if (fgets(buffer, MAX_STR, stdin) == NULL) {
    /* Nothing to read. */
		return IN_ERROR;
  }

  arg_read = sscanf(buffer, "%s%n", cmd, &char_read);
	if (arg_read != 1) {
		/*Argument not read*/
		printf("Error: Invalid message");
		exit(1);
	}

  /* Parse input. */
  if (strcmp(cmd, "rs") == 0) {
    /* Request service x */
		if (*service == -1) {
			arg_read = sscanf(buffer, "%*s %d%n", service, &char_read);
			if (arg_read != 1) {
	      /*Argument not read*/
	      printf("Error: Invalid message");
	      exit(1);
	    }
			return IN_RS;
		} else {
			return IN_NO_RS;
		}

	} else if (strcmp(cmd, "ts") == 0) {
    /* Terminate service. */
		if (*service != -1) {
			*service = -1;
			return IN_TS;
		} else {
			return IN_NO_TS;
		}


  } else if (strcmp(cmd, "exit") == 0) {
    /* Exit. */
		return IN_EXIT;

  } else {
    /* Invalid message. */
		return IN_ERROR;
  }

}

void request_service(int *service, int fd_udp, int fd_udp_serv, struct sockaddr_in addr_central, struct sockaddr_in *addr_service, socklen_t *addrlen, int *id, char *ip, int *upt) {
  char msg_in[MAX_STR], msg_out[MAX_STR], msg_type[MAX_STR], msg_data[MAX_STR];
  int nsend, nrecv,  arg_read, char_read;

  /* Check if there is one server with the wanted service. */
  sprintf(msg_out, "GET_DS_SERVER %d", *service);
  nsend = sendto(fd_udp, msg_out, strlen(msg_out), 0, (struct sockaddr*) &addr_central, sizeof(addr_central));
	if( nsend == -1 ) {
    perror("Error: Send to Central\nDescription:");
    exit(1); /*error*/
  }
  *addrlen = sizeof(addr_central);
	nrecv = recvfrom(fd_udp, msg_in, 128, 0, (struct sockaddr*) &addr_central, addrlen);
	if( nrecv == -1 ) {
    perror("Error: Receive from Central\nDescription:");
		exit(1);
  }
  msg_in[nrecv] = '\0';
  printf("%s\n", msg_in);

  arg_read = sscanf(msg_in, "%s %d;%s%n", msg_type, id, msg_data, &char_read);
	if (arg_read != 3) {
		/*Argument not read*/
		printf("Error: Invalid message");
		exit(1);
	}
	if (char_read != strlen(msg_in)) {
		printf("Error: Not every character was read");
		exit(1);
	}

  if (strcmp(msg_type, "OK") != 0) {
    printf("Erro: msg\n");
  } else {
    /* Service found */
    if (strcmp(msg_data, "0.0.0.0;0") != 0 && *id != 0) {

			arg_read = sscanf(msg_data, "%[^;];%d%n", ip, upt, &char_read);
			if (arg_read != 2) {
	      /*Argument not read*/
	      printf("Error: Invalid message");
	      exit(1);
	    }
	    if (char_read != strlen(msg_data)) {
	      printf("Error: Not every character was read");
	      exit(1);
	    }

      memset((void*) addr_service, (int) '\0', sizeof(*addr_service));
    	addr_service->sin_family = AF_INET;
    	addr_service->sin_addr.s_addr = inet_addr(ip);
    	addr_service->sin_port = htons(*upt);

      sprintf(msg_out, "MY_SERVICE ON");
      nsend = sendto(fd_udp_serv, msg_out, strlen(msg_out), 0, (struct sockaddr*)addr_service, sizeof(*addr_service));
    	if( nsend == -1 ) {
        printf("Error: send");
        exit(1); /*error*/
      }
      *addrlen = sizeof(*addr_service);

    	nrecv = recvfrom(fd_udp_serv, msg_in, 128, 0, (struct sockaddr*)addr_service, addrlen);
    	if( nrecv == -1 ) {
        printf("Error: recv");
        exit(1); /*error*/
      }
      msg_in[nrecv] = '\0';
      printf("%s\n", msg_in);
    } else {
      /* No server providing the Service */
			*service = -1;
      printf("No server providing that service\n");
    }
  }
}

void terminate_service(int fd_udp_serv, struct sockaddr_in addr_service, socklen_t *addrlen) {

	char msg_out[MAX_STR], msg_in[MAX_STR];
	int nrecv, nsend;

	sprintf(msg_out, "MY_SERVICE OFF");
	nsend = sendto(fd_udp_serv, msg_out, strlen(msg_out), 0, (struct sockaddr*)&addr_service, sizeof(addr_service));
	if( nsend == -1 ) {
		perror("Error: Send to server\nDescription:");
		exit(1); /*error*/
	}
	*addrlen = sizeof(addr_service);

	nrecv = recvfrom(fd_udp_serv, msg_in, 128, 0, (struct sockaddr*)&addr_service, addrlen);
	if( nrecv == -1 ) {
		perror("Error: Receive from server\nDescription:");
		exit(1); /*error*/
	}
	msg_in[nrecv] = '\0';
	printf("%s\n", msg_in);
}
