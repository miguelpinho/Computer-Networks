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

#define MAX_STR 128
#define USAGE "service –n id –j ip -u upt –t tpt [-i csip] [-p cspt]"
#define DEFAULT_HOST "tejo.tecnico.ulisboa.pt"
#define DEFAULT_PORT 59000
#define max(A,B) ((A)>=(B)?(A):(B))

enum status {AVAILABLE, BUSY, IDLE};
enum input_type {IN_ERROR, IN_JOIN, IN_NO_JOIN, IN_STATE, IN_LEAVE, IN_NO_LEAVE, IN_EXIT};

void prepare_sockets();
void get_arguments (int argc, const char *argv[], int *id, char *ip, int *upt, int *tpt, char *csip, int *cspt);
int parse_user_input(int *service);
void regist_on_central (int service, int fd_central, int id, int *id_start, struct sockaddr_in addr_central, int *my_id, char *ip, int upt, int tpt, socklen_t *addrlen);
void serve_client (int fd_service, struct sockaddr_in *addr_client, enum status *my_status);
void unregister_central (int id, int *id_start, int *service, int fd_central, struct sockaddr_in addr_central, socklen_t *addrlen);

int main(int argc, char const *argv[]) {
  int sel_in, exit_f = 0, counter;
  int max_fd = 0, n;
  fd_set rfds;
  /*int nread, st_id, st_tpt;*/
  enum status my_status = IDLE;
  char buffer[MAX_STR];

  struct fellow fellow;
  struct stream_buffer ring_buffer, nw_arrival_buffer;

  new_fellow(&fellow, id, ip, tpt);

  get_arguments(argc, argv, &(fellow.id), fellow.ip, &(fellow.upt),
                &(fellow.tpt), fellow.csip, &(fellow.cspt));

  create_sockets(&fellow);

  //printf("dummy: %d %s %d %d %s %d\n", id, ip, upt, tpt, csip, cspt);

  /* Adress of the client being served. */
  memset((void*) &addr_client, (int) '\0', sizeof(addr_client));

  while (exit_f == 0) {
    /* Prepare select, to monitor stdin and the sockets: central and service */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(fellow.fd_central, &rfds); max_fd = fellow.fd_central;
    FD_SET(fellow.fd_service, &rfds); max_fd = max(max_fd, fellow.fd_service);
    if (!fellow.nw_arrival_flag) {
      FD_SET(fellow.fd_listen, &rfds); max_fd = max(max_fd, fellow.fd_listen);
    } else () {
      FD_SET(fellow.fd_nw_arrival, &rfds); max_fd = max(max_fd, fellow.fd_nw_arrival);
    }
    if (fellow.prev_flag) {
      FD_SET(fellow.fd_prev, &rfds); max_fd = max(max_fd, fellow.fd_prev);
    }

    counter = select(max_fd+1, &rfds, (fd_set*) NULL, (fd_set*) NULL, (struct timeval *) NULL);
    if (counter <= 0) {
      printf("Error: select\n");

      exit(1); /*error*/
    }

    if (FD_ISSET(fd_central, &rfds)) {
      /* TODO: ignorar mensagem fora de tempo */

    }

    if (FD_ISSET(0, &rfds)) {
      /* Read user input. */
      sel_in = parse_user_input(&fellow.service);

      switch (sel_in) {
        case IN_JOIN:
          /* Regist server in central, with the input service. */
          regist_on_central(service, fd_central, id, &id_start, addr_central, &my_id, ip, upt, tpt, &addrlen);
          my_status = AVAILABLE;
          /* TODO: Join the service ring. */

          break;
        case IN_STATE:
          /* TODO: print state. */
          switch (my_status) {
            case AVAILABLE:
              printf("AVAILABLE\n");
              break;
            case BUSY:
              printf("BUSY\n");
              break;
            case IDLE:
              printf("IDLE\n");
              break;
            }
          break;
        case IN_LEAVE:
          /* TODO: Leave the service ring. */

          unregister_central(id, &id_start, &service, fd_central, addr_central, &addrlen);
          my_status = IDLE;
          break;
        case IN_EXIT:
          exit_f = 1;
          /*When you exit the api we have to unregister the server, otherwise it will be rubbish in the server*/
          if (my_status != IDLE) {
            unregister_central(id, &id_start, &service, fd_central, addr_central, &addrlen);
          }
          break;
        case IN_NO_JOIN:
          printf("Error: Is already providing another service\n");
          break;
        case IN_NO_LEAVE:
          printf("Error: Not providing any service\n");
          break;
        case IN_ERROR:
          printf("Error: Invalid input\n");
          break;
      }
    }

    if (FD_ISSET(fd_service, &rfds)) {
      /* Respond to a client request. */

      serve_client(fd_service, &addr_client, &my_status);
    }

    if (!fellow.nw_arrival_flag) {
      /* One new arrival is not already connected. */

      if (FD_ISSET(fellow.fd_listen, &rfds)) {
        /* A fellow is trying to connect to this one. */
        if (!fellow.start) {
          /* Out of time connection. Only makes sense if this is start? */
          /* TODO */
        }

        /* Accept new fellow. */
        if ((fellow.fd_aux = accept(fellow->fd_listen, (struct sockaddr*) &addr, &addrlen)) == -1) {
          exit(1); /* error */
        }

        fellow.nw_arrival_flag = 1;
        init_stream(&nw_arrival_buffer);
      }
    } else {
      /* Deals with the new arrival. */

      if (FD_ISSET(fellow.fd_nw_arrival, &rfds)) {
        /* Receiving first message from new. */

        /* Read all. */
        if (readto_stream(fellow.fd_prev, &ring_buffer) == -1) {
          /* TODO */
        }

        /* Parse one message. */
        if (get_stream(buffer, &ring_buffer) != -1) {
          if (message_nw_arrival(buffer, &fellow) == 0) {
            /* TODO: Error on tcp message protocol. */
          }

        }
      }
    }

    if (fellow.prev_flag) {
      /* There is a previous. */

      if (FD_ISSET(fellow.fd_prev, &rfds)) {
        /* Message(s) from previous. */

        /* Read all. */
        if (readto_stream(fellow.fd_prev, &ring_buffer) == -1) {
          /* TODO: error */
        }

        /* Parse while possible. */
        while (get_stream(buffer, &ring_buffer) != -1) {
          if (process_message(buffer, &fellow, fd_central, addr_central) == 0) {
            /* TODO: Error on tcp message protocol. */
          }

        }
      }
    }
  }

  /* Exit. */
  close(fd_central);
  close(fd_service);
void prepare_sockets();
  return 0;
}

void get_arguments(int argc, const char *argv[], int *id, char *ip, int *upt, int *tpt, char *csip, int *cspt) {
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
      sprintf(csip, "%s", inet_ntoa(*a));
    }

    if (csp != 1) {
      *cspt = DEFAULT_PORT;
    }

    return;
  }

int parse_user_input(int *service) {
  char buffer[MAX_STR], cmd[MAX_STR];
  int new_sv;

  if (fgets(buffer, MAX_STR, stdin) == NULL) {
    /* Nothing to read. */

    return IN_ERROR;
  }

  sscanf(buffer, "%s", cmd);

  /* Parse input. */
  if (strcmp(cmd, "join") == 0) {
    /* Read service id. */
    sscanf(buffer, "%*s %d", &new_sv);

    /* Check if this server alreay belongs to a service ring. */
    if (*service != -1) {

      return IN_NO_JOIN;
    }

    /* Join the service ring. */

    *service = new_sv;

    return IN_JOIN;

  } else if (strcmp(cmd, "show_state") == 0) {
    /* Show the server state. */

    return IN_STATE;
  } else if (strcmp(cmd, "leave") == 0) {
    /* Leave the service ring. */

    if(*service != -1){
      return IN_LEAVE;
    } else {
      return IN_NO_LEAVE;
    }
  } else if (strcmp(cmd, "exit") == 0) {
    /* Exit. */

    return IN_EXIT;
  } else {
    /* Invalid message. */

    return IN_ERROR;
  }
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

void serve_client(struct fellow *fellow, enum status *my_status) {
  int ret, nread;
  socklen_t addrlen;
  char toggle[MAX_STR], msg_in[MAX_STR], msg_out[MAX_STR];

  addrlen = sizeof(struct sockaddr_in);

  nread = recvfrom( fellow->fd_service, msg_in, MAX_STR, 0,
                    (struct sockaddr*) &(fellow->addr_client), &addrlen );
  if (nread==-1) {
    exit(1); /*error*/
  }

  msg_in[nread] = '\0';
  printf("%s\n", msg_in);

  sscanf(msg_in, "MY_SERVICE %s", toggle);

  if (strcmp(toggle, "ON") == 0){
    sprintf(msg_out, "YOUR_SERVICE ON");
    ret = sendto( fellow->fd_service, msg_out, strlen(msg_out), 0,
                  (struct sockaddr*) &(fellow->addr_client), addrlen );
    if (ret==-1) {
      exit(1); /*error*/
    }

    *my_status = BUSY;
  } else if (strcmp(toggle, "OFF") == 0) {
    sprintf(msg_out, "YOUR_SERVICE OFF");
    ret = sendto( fellow->fd_service, msg_out, strlen(msg_out), 0,
                  (struct sockaddr*) &(fellow->addr_client), addrlen );
    if (ret==-1) {
      exit(1); /*error*/
    }

    *my_status = AVAILABLE;
  } else {
    /* TODO: invalid message. */

  }
}
