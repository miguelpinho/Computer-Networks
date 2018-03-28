/*
  authors: Miguel Malaca, Miguel Pinho

  description:
    variables for each server on the service ring
*/

#include "fellow.h"

void new_fellow(struct fellow *this) {

  /* FIXME: initialize state variables */
  this->start = 0;
  this->available = -1;
  this->ring_unavailable = -1;
  this->dispatch = 0;

  this->service = -1;

  /* FIXME: set next to none */
  this->next.id = -1;

}

void create_sockets(struct fellow *fellow) {
  int ret;
  struct sockaddr_in addr_fellow;

  /* Create socket for communication with central. */
  fellow->fd_central = socket(AF_INET, SOCK_DGRAM, 0);
	if(fellow->fd_central == -1) {
    printf("Error: socket central");
    exit(1); /*error*/
  }

  /* Create address of the central server. */
  memset((void*) &(fellow->addr_central), (int) '\0', sizeof(fellow->addr_central));
	fellow->addr_central.sin_family = AF_INET;
	fellow->addr_central.sin_addr.s_addr = inet_addr(fellow->csip);
	fellow->addr_central.sin_port = htons(fellow->cspt);

  /* Create socket for the service to the client. */
  fellow->fd_service = socket(AF_INET, SOCK_DGRAM, 0);
	if (fellow->fd_service == -1) {
    printf("Error: socket serving");
    exit(1); /*error*/
  }

  /* Binds client serving socket to the given address. */
  memset((void*) &(fellow->addr_service), (int) '\0', sizeof(fellow->addr_service));
  fellow->addr_service.sin_family = AF_INET;
  fellow->addr_service.sin_port = htons(fellow->upt);
  fellow->addr_service.sin_addr.s_addr = inet_addr(fellow->ip);

  ret = bind( fellow->fd_service, (struct sockaddr*) &(fellow->addr_service),
              sizeof(fellow->addr_service) );
  if (ret == -1) {
    printf("Error: bind\n");

    exit(1); /*error*/
  }

  /* Create listen socket */
  if ((fellow->fd_listen = socket(AF_INET,SOCK_STREAM,0)) == -1) {
    exit(1); /* error */
  }

  memset((void*) &addr_fellow, (int) '\0', sizeof(addr_fellow));

  addr_fellow.sin_family = AF_INET;
  addr_fellow.sin_addr.s_addr = htonl(INADDR_ANY);
  addr_fellow.sin_port = htons(fellow->tpt);

  if (bind(fellow->fd_listen, (struct sockaddr*) &addr_fellow, sizeof(addr_fellow)) == -1) {
    exit(1); /* error */
  }

  if (listen(fellow->fd_listen, 5) == -1) {
    exit(1); /* error */
  }

  /* Adress of the client being served. */
  memset((void*) &(fellow->addr_client), (int) '\0', sizeof(fellow->addr_client));
}

void destroy_fellow(struct fellow *this) {
  /* close sockets */
  close(this->fd_central);
  close(this->fd_service);
  close(this->next.fd_next);
  close(this->fd_listen);
  close(this->fd_prev);
  close(this->fd_nw_arrival);
}

void register_cs(char *reply, struct fellow *fellow) {
  char msg_out[MAX_STR], msg_in[MAX_STR];
  int nsend, nrecv;
  socklen_t addrlen;

  /* Regist this service server in the central server (UDP). */
  /* get start server */
  sprintf(msg_out, "GET_START %d;%d", fellow->service, fellow->id);
  nsend = sendto( fellow->fd_central, msg_out, strlen(msg_out), 0,
                  (struct sockaddr*) &(fellow->addr_central),
                  sizeof(fellow->addr_central) );
  if( nsend == -1 ) {
    printf("Error: send");
    exit(1); /*error*/
  }

  addrlen = sizeof(msg_in->addr_central);

  nrecv = recvfrom( msg_in->fd_central, msg_in, 128, 0,
                    (struct sockaddr*) &(fellow->addr_central), &addrlen );
  if( nrecv == -1 ) {
    printf("Error: recv");
    exit(1); /*error*/
  }

  msg_in[nrecv] = '\0';
  printf("%s\n", msg_in);

  strcpy(reply, msg_in);
}

void set_cs(char *query, struct fellow *fellow) {
  char msg_out[MAX_STR], msg_in[MAX_STR];
  int nsend, nrecv;
  socklen_t addrlen;

  sprintf( msg_out, "%s %d;%d;%s;%d", query, fellow->service, fellow->id,
           fellow->ip, fellow->tpt );
  nsend = sendto( fellow->fd_central, msg_out, strlen(msg_out), 0,
                  (struct sockaddr*) &(fellow->addr_central),
                  sizeof(fellow->addr_central) );
  if( nsend == -1 ) {
    printf("Error: send");
    exit(1); /*error*/
  }

  addrlen = sizeof(fellow->addr_central);

  nrecv = recvfrom( fellow->fd_central, msg_in, 128, 0,
                    (struct sockaddr*) &(fellow->addr_central), &addrlen );
  if( nrecv == -1 ) {
    printf("Error: recv");
    exit(1); /*error*/
  }

  /* TODO: Check if message is OK */
  msg_in[nrecv] = '\0';
  printf("%s\n", msg_in);

}

void withdraw_cs(char *query, struct fellow *fellow) {
  char msg_out[MAX_STR], msg_in[MAX_STR];
  int nsend, nrecv;
  socklen_t addrlen;

  sprintf(msg_out, "%s %d;%d", query, fellow->service, fellow->id);
  nsend = sendto( fellow->fd_central, msg_out, strlen(msg_out), 0,
                  (struct sockaddr*) &(fellow->addr_central),
                  sizeof(fellow->addr_central) );
  if( nsend == -1 ) {
    printf("Error: send");
    exit(1); /*error*/
  }

  addrlen = sizeof(fellow->addr_central);

  nrecv = recvfrom( fellow->fd_central, msg_in, MAX_STR, 0,
                    (struct sockaddr*) &(fellow->addr_central), &addrlen );
  if( nrecv == -1 ) {
    printf("Error: recv");
    exit(1); /*error*/
  }

  /* TODO: Check if reply is an OK? */
  msg_in[nrecv] = '\0';
  printf("%s\n", msg_in);
}
