/*
  authors: Miguel Malaca, Miguel Pinho

  description:
    variables for each server on the service ring
*/

#include "fellow.h"

void new_fellow(struct fellow *this) {

  /* FIXME: initialize state variables */
  this->start = -1;
  this->available = -1;
  this->ring_unavailable = -1;
  this->dispatch = -1;
  this->prev_flag = 0;
  this->nw_arrival_flag = 0;
  this->nw_available_flag = 0;
  this->exiting = NO_EXIT;
  this->wait_connect = 0;

  this->service = -1;

  /* FIXME: set next to none */
  this->next.id = -1;

  /* Init in buffer */
  this->in_buffer[0] = '\0';
}

void create_sockets(struct fellow *fellow) {
  int ret;
  struct sockaddr_in addr_fellow, addr_service;

  struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  /* Create socket for communication with central. */
  fellow->fd_central = socket(AF_INET, SOCK_DGRAM, 0);
	if(fellow->fd_central == -1) {
    perror("Error: Creating central socket\nDescription:");
    exit(1); /* error */
  }

  /* Set recv timeout for central socket. */
  if (setsockopt ( fellow->fd_central, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                   sizeof(timeout)) < 0 ) {
    perror("Error: timeout central socket\nDescription:");
    exit(1); /* error */
  }

  /* Create socket for the service to the client. */
  fellow->fd_service = socket(AF_INET, SOCK_DGRAM, 0);
	if (fellow->fd_service == -1) {
    perror("Error: Creating serving socket\nDescription:");
    close(fellow->fd_central);
    exit(1); /* error */
  }

  /* Binds client serving socket to the given address. */
  memset((void*) &(addr_service), (int) '\0', sizeof(addr_service));
  addr_service.sin_family = AF_INET;
  addr_service.sin_port = htons(fellow->upt);
  addr_service.sin_addr.s_addr = inet_addr(fellow->ip);

  ret = bind( fellow->fd_service, (struct sockaddr*) &(addr_service),
              sizeof(addr_service) );
  if (ret == -1) {
    perror("Error: bind service socket (upt)\nDescription:");
    close(fellow->fd_central);
    close(fellow->fd_service);
    exit(1); /*error*/
  }

  /* Create listen socket */
  if ((fellow->fd_listen = socket(AF_INET,SOCK_STREAM,0)) == -1) {
    perror("Error: Creating listen socket\nDescription:");
    close(fellow->fd_service);
    close(fellow->fd_central);
    exit(1); /* error */
  }

  memset((void*) &addr_fellow, (int) '\0', sizeof(addr_fellow));

  addr_fellow.sin_family = AF_INET;
  addr_fellow.sin_addr.s_addr = htonl(INADDR_ANY);
  addr_fellow.sin_port = htons(fellow->tpt);

  if (bind(fellow->fd_listen, (struct sockaddr*) &addr_fellow, sizeof(addr_fellow)) == -1) {
    perror("Error: bind listen socket (tpt)\nDescription");
    brute_exit(fellow);
  }

  if (listen(fellow->fd_listen, 5) == -1) {
    perror("Error: listen\nDescription:");
    brute_exit(fellow);
  }
}

void destroy_fellow(struct fellow *this) {
  /* close sockets */
  close(this->fd_central);
  close(this->fd_service);
  close(this->fd_listen);
  if (this->next.id == -1) {
    close(this->next.fd_next);
  }

  if (this->prev_flag == 1) {
    close(this->fd_prev);
  }
}

void show_state(struct fellow *fellow) {
  printf("Service state (id = %d): \n", fellow->id);
  printf("\tThis: %s\n", (fellow->available == 1)?"Available":"Unavailable");
  printf("\tRing: %s\n", (fellow->ring_unavailable == 0)?"Available":"Unavailable");
  printf("\tNext: id = %d\n", fellow->next.id);
}

void register_cs(char *reply, struct fellow *fellow) {
  char msg_out[MAX_STR], msg_in[MAX_STR];
  int nsend, nrecv;
  struct sockaddr_in addr_central;
  socklen_t addrlen;

  /* Create address of the central server. */
  memset((void*) &(addr_central), (int) '\0', sizeof(addr_central));
	addr_central.sin_family = AF_INET;
	addr_central.sin_addr.s_addr = inet_addr(fellow->csip);
	addr_central.sin_port = htons(fellow->cspt);

  /* Regist this service server in the central server (UDP). */
  /* get start server */
  sprintf(msg_out, "GET_START %d;%d", fellow->service, fellow->id);
  nsend = sendto( fellow->fd_central, msg_out, strlen(msg_out), 0,
                  (struct sockaddr*) &(addr_central),
                  sizeof(addr_central) );
  if( nsend == -1 ) {
    perror("Error: Send to central\nDescription:");
    brute_exit(fellow);
  }

  addrlen = sizeof(addr_central);

  nrecv = recvfrom( fellow->fd_central, msg_in, 128, 0,
                    (struct sockaddr*) &(addr_central), &addrlen );
  if( nrecv == -1 ) {
    perror("Error: Receive from central\nDescription:");
    brute_exit(fellow);
  }

  msg_in[nrecv] = '\0';
  printf("%s\n", msg_in);

  strcpy(reply, msg_in);
}

void set_cs(char *query, struct fellow *fellow, int pt) {
  char msg_out[MAX_STR], msg_in[MAX_STR], verifier[MAX_STR];
  int nsend, nrecv, arg_read;
  struct sockaddr_in addr_central;
  socklen_t addrlen;

  /* Create address of the central server. */
  memset((void*) &(addr_central), (int) '\0', sizeof(addr_central));
	addr_central.sin_family = AF_INET;
	addr_central.sin_addr.s_addr = inet_addr(fellow->csip);
	addr_central.sin_port = htons(fellow->cspt);

  sprintf( msg_out, "%s %d;%d;%s;%d", query, fellow->service, fellow->id,
           fellow->ip, pt );
  nsend = sendto( fellow->fd_central, msg_out, strlen(msg_out), 0,
                  (struct sockaddr*) &(addr_central),
                  sizeof(addr_central) );
  if( nsend == -1 ) {
    perror("Error: Send to central\nDescription:");
    brute_exit(fellow);
  }

  addrlen = sizeof(addr_central);

  nrecv = recvfrom( fellow->fd_central, msg_in, 128, 0,
                    (struct sockaddr*) &(addr_central), &addrlen );
  if( nrecv == -1 ) {
    perror("Error: Receive from central\nDescription:");
    brute_exit(fellow);
  }

  /* Check if message is OK */
  arg_read = sscanf(msg_in, "%s", verifier);
  if (arg_read != 1) {
    printf("Error: Invalid message");
    brute_exit(fellow);
  }

  if (strcmp(verifier, "OK") != 0) {
    printf("Error: Invalid message");
    brute_exit(fellow);
  }

  msg_in[nrecv] = '\0';
  printf("%s\n", msg_in);

}

void withdraw_cs(char *query, struct fellow *fellow) {
  char msg_out[MAX_STR], msg_in[MAX_STR], verifier[MAX_STR];
  int nsend, nrecv, arg_read;
  struct sockaddr_in addr_central;
  socklen_t addrlen;

  /* Create address of the central server. */
  memset((void*) &(addr_central), (int) '\0', sizeof(addr_central));
	addr_central.sin_family = AF_INET;
	addr_central.sin_addr.s_addr = inet_addr(fellow->csip);
	addr_central.sin_port = htons(fellow->cspt);

  sprintf(msg_out, "%s %d;%d", query, fellow->service, fellow->id);
  nsend = sendto( fellow->fd_central, msg_out, strlen(msg_out), 0,
                  (struct sockaddr*) &(addr_central),
                  sizeof(addr_central) );
  if( nsend == -1 ) {
    perror("Error: Send to central\nDescription:");
    brute_exit(fellow);
  }

  addrlen = sizeof(addr_central);

  nrecv = recvfrom( fellow->fd_central, msg_in, MAX_STR, 0,
                    (struct sockaddr*) &(addr_central), &addrlen );
  if( nrecv == -1 ) {
    perror("Error: Receive from central\nDescription:");
    brute_exit(fellow);
  }

  /* Check if message is OK */
  arg_read = sscanf(msg_in, "%s", verifier);
  if (arg_read != 1) {
    printf("Error: Invalid message");
    brute_exit(fellow);
  }

  if (strcmp(verifier, "OK") != 0) {
    printf("Error: Invalid message");
    brute_exit(fellow);
  }

  msg_in[nrecv] = '\0';
  printf("%s\n", msg_in);
}

void brute_exit(struct fellow *fellow) {
  if (fellow->dispatch == 1) {
    withdraw_cs("WITHDRAW_DS", fellow);
  }

  if (fellow->start == 1) {
    withdraw_cs("WITHDRAW_START", fellow);
  }

  destroy_fellow(fellow);

  exit(1);
}
