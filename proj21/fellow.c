/*
  authors: Miguel Malaca, Miguel Pinho

  description:
    variables for each server on the service ring
*/

#include "fellow.h"

void new_fellow(struct fellow *this) {

  /* FIXME: initialize state variables */
  this->start = 0;
  this->available = 1;
  this->ring_unavailable = 0;
  this->dispatch = 0;
  this->prev_flag = 0;
  this->nw_arrival_flag = NO_NEW;
  this->nw_available_flag = 0;
  this->exiting = NO_EXIT;
  this->wait_connect = 0;

  this->service = -1;

  /* FIXME: set next to none */
  this->next.id = -1;

  /* Init in buffers */
  this->in_buffer[0] = '\0';
  this->aux_in_buffer[0] = '\0';
}

void create_sockets(struct fellow *fellow) {
  int ret;
  struct sockaddr_in addr_fellow, addr_service;

  struct timeval timeout;
  timeout.tv_sec = 2;
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

  fellow->fd_new_arrival = -1;
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

  if (fellow->service == -1) {
    printf("This server is not associated with any service ring\n");
  } else {
    printf("Service state (id = %d): \n", fellow->id);
    if (fellow->start == 1) {
      printf("\tThis is the START server\n");
    }
    if (fellow->dispatch == 1) {
      printf("\tThis is the DISPATCH server\n");
    }
    printf("\tThis: %s\n", (fellow->available == 1)?"Available":"Unavailable");
    printf("\tRing: %s\n", (fellow->ring_unavailable == 0)?"Available":"Unavailable");
    if (fellow->next.id != -1) {
      printf("\tNext: id = %d\n", fellow->next.id);
    } else {
      printf("\tThis server is alone in the ring\n");
    }
  }
}


void register_cs(char *reply, struct fellow *fellow) {
  char msg_out[MAX_STR], msg_in[MAX_STR], verifier[MAX_STR];
  int nsend, nrecv, c_msg = 0, count = 0, arg_read;
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

  while (count < N_CHANCES && c_msg == 0) {
    nsend = sendto( fellow->fd_central, msg_out, strlen(msg_out), 0,
                    (struct sockaddr*) &(addr_central),
                    sizeof(addr_central) );
    if( nsend == -1 ) {
      perror("Error: Send to central\nDescription:");
      lastoption_exit(fellow);
    }

    addrlen = sizeof(addr_central);

    nrecv = recvfrom( fellow->fd_central, msg_in, 128, 0,
                      (struct sockaddr*) &(addr_central), &addrlen );
    if( nrecv == -1 ) {
      perror("Error: Receive from central\nDescription:");
      count++;
      continue;
    }

    msg_in[nrecv] = '\0';

    arg_read = sscanf(msg_in, "%s", verifier);
    if (arg_read != 1) {
      printf("Error: Invalid message");
      count++;
      continue;
    }

    if (strcmp(verifier, "OK") != 0) {
      printf("Error: Invalid message - %s", msg_in);
      count++;
      continue;
    } else {
      c_msg = 1;
    }


  }

  if ( c_msg == 0 || nrecv == -1 ) {
    printf("Coundn't get an available answer from central server\n");
		lastoption_exit(fellow);
  }

  strcpy(reply, msg_in);
}

void set_cs(char *query, struct fellow *fellow, int pt) {
  char msg_out[MAX_STR], msg_in[MAX_STR], msg_data[MAX_STR], verifier[MAX_STR];
  int nsend, nrecv, arg_read, c_msg = 0, count = 0;
  struct sockaddr_in addr_central;
  socklen_t addrlen;

  /* Create address of the central server. */
  memset((void*) &(addr_central), (int) '\0', sizeof(addr_central));
	addr_central.sin_family = AF_INET;
	addr_central.sin_addr.s_addr = inet_addr(fellow->csip);
	addr_central.sin_port = htons(fellow->cspt);

  sprintf( msg_out, "%s %d;%d;%s;%d", query, fellow->service, fellow->id,
           fellow->ip, pt );

  while( count < N_CHANCES && c_msg == 0) {
    nsend = sendto( fellow->fd_central, msg_out, strlen(msg_out), 0,
                    (struct sockaddr*) &(addr_central),
                    sizeof(addr_central) );
    if( nsend == -1 ) {
      perror("Error: Send to central\nDescription:");
      lastoption_exit(fellow);
    }

    addrlen = sizeof(addr_central);

    nrecv = recvfrom( fellow->fd_central, msg_in, 128, 0,
                      (struct sockaddr*) &(addr_central), &addrlen );
    if( nrecv == -1 ) {
      perror("Error: Receive from central\nDescription:");
      count++;
      continue;
    }

    /* Check if message is OK */
    arg_read = sscanf(msg_in, "%s %s", verifier, msg_data);
    if (arg_read != 2) {
      printf("Error: Invalid message - %s", msg_in);
      count++;
      continue;
    }

    if (strcmp(verifier, "OK") != 0) {
      printf("Error: Invalid message");
      count++;
      continue;
    } else {
      c_msg = 1;
    }
  }

  if (c_msg == 0 || nrecv == -1) {
    printf("Coundn't get an available answer from central server\n");
		lastoption_exit(fellow);
  }

  msg_in[nrecv] = '\0';


  /* Something went wrong, trying to set start or ds when there was already one */
  if (strcmp(msg_in, "OK 0;0.0.0.0;0") == 0)
  {
    printf("Error: There is already one server in that position (Start/Dispatch)\n");
    lastoption_exit(fellow);
  }

}

void withdraw_cs(char *query, struct fellow *fellow) {
  char msg_out[MAX_STR], msg_in[MAX_STR], verifier[MAX_STR];
  int nsend, nrecv, arg_read, count = 0, c_msg = 0;
  struct sockaddr_in addr_central;
  socklen_t addrlen;

  /* Create address of the central server. */
  memset((void*) &(addr_central), (int) '\0', sizeof(addr_central));
	addr_central.sin_family = AF_INET;
	addr_central.sin_addr.s_addr = inet_addr(fellow->csip);
	addr_central.sin_port = htons(fellow->cspt);

  sprintf(msg_out, "%s %d;%d", query, fellow->service, fellow->id);
  while(count < N_CHANCES && c_msg == 0) {
    nsend = sendto( fellow->fd_central, msg_out, strlen(msg_out), 0,
                    (struct sockaddr*) &(addr_central),
                    sizeof(addr_central) );
    if( nsend == -1 ) {
      perror("Error: Send to central\nDescription:");
      lastoption_exit(fellow);
    }

    addrlen = sizeof(addr_central);

    nrecv = recvfrom( fellow->fd_central, msg_in, MAX_STR, 0,
                      (struct sockaddr*) &(addr_central), &addrlen );
    if( nrecv == -1 ) {
      perror("Error: Receive from central\nDescription:");
      count++;
      continue;
    }

    /* Check if message is OK */
    arg_read = sscanf(msg_in, "%s", verifier);
    if (arg_read != 1) {
      printf("Error: Invalid message");
      count++;
      continue;
    }

    if (strcmp(verifier, "OK") != 0) {
      printf("Error: Invalid message");
      count++;
      continue;
    } else {
      c_msg = 1;
    }
  }

  if (c_msg == 0 || nrecv == -1) {
    printf("Coundn't get an available answer from central server\n");
		lastoption_exit(fellow);
  }

  msg_in[nrecv] = '\0';
}

void stop_service(struct fellow *fellow) {
  char msg_out[MAX_STR];
  int ret;
  socklen_t addrlen = sizeof(fellow->addr_client);

  fellow->available = 0;

  sprintf(msg_out, "YOUR_SERVICE OFF");
  ret = sendto( fellow->fd_service, msg_out, strlen(msg_out), 0,
                (struct sockaddr*) &(fellow->addr_client), addrlen );
  if (ret==-1) {
    perror("Error: Sending to client\nDescription:");
  }

  printf("Stopped providing a service\n");
}

void brute_exit(struct fellow *fellow) {
  if (fellow->dispatch == 1) {
    withdraw_cs("WITHDRAW_DS", fellow);
    fellow->dispatch = 0;
  }

  if (fellow->start == 1) {
    withdraw_cs("WITHDRAW_START", fellow);
    fellow->start = 0;
  }

  if (fellow->available == 0) {
    stop_service(fellow);
    fellow->available = -1;
  }

  destroy_fellow(fellow);
  printf("Forced gracious exit completed\n");

  exit(1);
}

void lastoption_exit(struct fellow *fellow) {
  if (fellow->available == 0) {
    stop_service(fellow);
    fellow->available = -1;
  }

  destroy_fellow(fellow);
  printf("Forced forced gracious exit completed\n");

  exit(1);
}
