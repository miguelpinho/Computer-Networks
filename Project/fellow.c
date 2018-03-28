/*
  authors: Miguel Malaca, Miguel Pinho

  description:
    variables for each server on the service ring
*/

#include "fellow.h"

void new_fellow(struct fellow *this) {
  struct sockaddr_in addr_fellow;

  /* FIXME: initialize state variables */
  this->start = 0;
  this->available = -1;
  this->ring_available = -1;
  this->dispatch = 0;

  this->service = -1;

  /* FIXME: set next to none */
  this->next.id = -1;

}

void creat_sockets(struct fellow *fellow) {
  int ret;

  /* Create socket for communication with central. */
  fellow->fd_central = socket(AF_INET, SOCK_DGRAM, 0);
	if(fellow->fd_central == -1) {
    printf("Error: socket central");
    exit(1); /*error*/
  }

  /* Create address of the central server. */
  memset((void*) &addr_central, (int) '\0', sizeof(addr_central));
	addr_central.sin_family = AF_INET;
	addr_central.sin_addr.s_addr = inet_addr(csip);
	addr_central.sin_port = htons(cspt);

  /* Create socket for the service to the client. */
  fellow->fd_service = socket(AF_INET, SOCK_DGRAM, 0);
	if (fellow->fd_service == -1) {
    printf("Error: socket serving");
    exit(1); /*error*/
  }

  /* Binds client serving socket to the given address. */
  memset((void*) &addr_service, (int) '\0', sizeof(addr_service));
  addr_service.sin_family = AF_INET;
  addr_service.sin_addr.s_addr = inet_addr(ip);
  addr_service.sin_port = htons(upt);

  ret = bind(fellow->fd_service, (struct sockaddr*) &addr_service, sizeof(addr_service));
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
  addr_fellow.sin_port = htons(tpt);

  if (bind(fellow->fd_listen, (struct sockaddr*) &addr_fellow, sizeof(addr_fellow)) == -1) {
    exit(1); /* error */
  }

  if (listen(fd,5) == -1) {
    exit(1); /* error */
  }
  memset((void*) &addr_fellow, (int) '\0', sizeof(addr_fellow));

  addr_fellow.sin_family = AF_INET;
  addr_fellow.sin_addr.s_addr = htonl(INADDR_ANY);
  addr_fellow.sin_port = htons(tpt);

  if (bind(fellow->fd_listen, (struct sockaddr*) &addr_fellow, sizeof(addr_fellow)) == -1) {
    exit(1); /* error */
  }

  if (listen(fd,5) == -1) {
    exit(1); /* error */
  }
}

void destroy_fellow(struct fellow *this) {
  /* close sockets */
  close(this->next.fd_next);
  close(this->fd_listen);
  close(this->fd_prev);
  close(this->fd_aux);
}

void regist_on_central(struct fellow *fellow) {
  int id_start;
  int nsend, nrecv;
  char buffer[MAX_STR], msg_out[MAX_STR], msg_type[MAX_STR], msg_data[MAX_STR];
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
  addrlen = sizeof(fellow->addr_central);
	nrecv = recvfrom( fellow->fd_central, buffer, 128, 0,
                    (struct sockaddr*) &(fellow->addr_central), addrlen );
	if( nrecv == -1 ) {
    printf("Error: recv");
    exit(1); /*error*/
  }
  buffer[nrecv] = '\0';
  printf("%s\n", buffer);

  sscanf(buffer, "%s %s", msg_type, msg_data);
  if (strcmp(msg_type, "OK") != 0) {
    printf("Erro: msg\n");
  } else {
    sprintf(test_data, "%d;0;0.0.0.0;0", fellow->id);
    if (strcmp(msg_data, test_data) == 0) {
      /* This is the start server. */
      fellow->start = 1;

      /* Set start */
      sprintf(msg_out, "SET_START %d;%d;%s;%d", service, fellow->id, fellow->ip, fellow->tpt);
      nsend = sendto( fellow->fd_central, msg_out, strlen(msg_out), 0,
                      (struct sockaddr*) &(fellow->addr_central),
                      sizeof(fellow->addr_central) );
    	if( nsend == -1 ) {
        printf("Error: send");
        exit(1); /*error*/
      }
      addrlen = sizeof(fellow->addr_central);
    	nrecv = recvfrom( fellow->fd_central, buffer, 128, 0,
                        (struct sockaddr*) &(fellow->addr_central), addrlen );
    	if( nrecv == -1 ) {
        printf("Error: recv");
        exit(1); /*error*/
      }
      /* TODO: Check if message is OK */
      buffer[nrecv] = '\0';
      printf("%s\n", buffer);

      /* Set the server to dispatch */
      fellow->dispatch = 1;
      this->available = 1;
      fellow->ring_ring_unavailable = 0;

      sprintf(msg_out, "SET_DS %d;%d;%s;%d", service, fellow->id, fellow->ip, upt);
      nsend = sendto(fd_central, msg_out, strlen(msg_out), 0, (struct sockaddr*)&addr_central, sizeof(addr_central));
      if( nsend == -1 ) {
        printf("Error: send");
        exit(1); /*error*/
      }
      *addrlen = sizeof(addr_central);
      nrecv = recvfrom(fd_central, buffer, 128, 0, (struct sockaddr*)&addr_central, addrlen);
      if( nrecv == -1 ) {
        printf("Error: recv");
        exit(1); /*error*/
      }
      /* TODO: Check if message is OK */
      buffer[nrecv] = '\0';
      printf("%s\n", buffer);

    } else {
      sscanf(msg_data, "%*d;%d;%[^; ];%d", &id_start, ip_start, &tpt_start);

      join_ring(fellow, tpt_start, ip_start, id_start);
    }
  }


}

void unregister_central (struct fellow *fellow) {
  char buffer[MAX_STR], msg_out[MAX_STR];
  int nsend, nrecv;
  socklen_t addrlen;

  if(fellow->start) {
    sprintf(msg_out, "WITHDRAW_START %d;%d", fellow->service, fellow->id);
    nsend = sendto( fellow->fd_central, msg_out, strlen(msg_out), 0,
                    (struct sockaddr*) &(fellow->addr_central),
                    sizeof(fellow->addr_central) );
    if( nsend == -1 ) {
      printf("Error: send");
      exit(1); /*error*/
    }
    addrlen = sizeof(fellow->addr_central);
    nrecv = recvfrom( fellow->fd_central, buffer, MAX_STR, 0,
                      (struct sockaddr*) &(fellow->addr_central), addrlen );
    if( nrecv == -1 ) {
      printf("Error: recv");
      exit(1); /*error*/
    }
    buffer[nrecv] = '\0';
    printf("%s\n", buffer);

  }

  sprintf(msg_out, "WITHDRAW_DS %d;%d", fellow->service, fellow->id);
  nsend = sendto( fellow->fd_central, msg_out, strlen(msg_out), 0,
                  (struct sockaddr*) &(fellow->addr_central),
                  sizeof(fellow->addr_central) );
  if( nsend == -1 ) {
    printf("Error: send");
    exit(1); /*error*/
  }
  addrlen = sizeof(fellow->addr_central);
  nrecv = recvfrom( fellow->fd_central, buffer, MAX_STR, 0,
                    (struct sockaddr*) &(fellow->addr_central), addrlen );
  if( nrecv == -1 ) {
    printf("Error: recv");
    exit(1); /*error*/
  }
  buffer[nrecv] = '\0';
  printf("%s\n", buffer);

  fellow->service = -1;

}
