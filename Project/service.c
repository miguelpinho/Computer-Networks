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
#include "fellow.h"
#include "ring.h"

#define USAGE "service –n id –j ip -u upt –t tpt [-i csip] [-p cspt]"
#define DEFAULT_HOST "tejo.tecnico.ulisboa.pt"
#define DEFAULT_PORT 59000
#define max(A,B) ((A)>=(B)?(A):(B))

enum input_type {IN_ERROR, IN_JOIN, IN_NO_JOIN, IN_STATE, IN_LEAVE, IN_NO_LEAVE, IN_EXIT};

void get_arguments(int argc, const char *argv[], int *id, char *ip, int *upt, int *tpt, char *csip, int *cspt);
int parse_user_input(int *service);
void serve_client(struct fellow *fellow);

int main(int argc, char const *argv[]) {
  int sel_in, exit_f = 0, max_fd = 0;
  int counter, ret;
  fd_set rfds;
  char buffer[MAX_STR];
  struct fellow fellow;
  struct sockaddr addr_acpt;
  socklen_t addrlen = sizeof(addr_acpt);

  buffer[0] = '\0';

  new_fellow(&fellow);

  get_arguments(argc, argv, &(fellow.id), fellow.ip, &(fellow.upt),
                &(fellow.tpt), fellow.csip, &(fellow.cspt));

  create_sockets(&fellow);

  /*
  printf("dummy: %d %s %d %d %s %d\n", id, ip, upt, tpt, csip, cspt);
  */

  while (exit_f == 0) {
    /* Prepare select, to monitor stdin and the sockets */
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    FD_SET(fellow.fd_central, &rfds); max_fd = fellow.fd_central;
    FD_SET(fellow.fd_service, &rfds); max_fd = max(max_fd, fellow.fd_service);
    FD_SET(fellow.fd_listen, &rfds); max_fd = max(max_fd, fellow.fd_listen);
    if (fellow.prev_flag) {
      FD_SET(fellow.fd_prev, &rfds); max_fd = max(max_fd, fellow.fd_prev);
    }

    counter = select(max_fd+1, &rfds, (fd_set*) NULL, (fd_set*) NULL, (struct timeval *) NULL);
    if (counter <= 0) {
      printf("Error: select\n");

      exit(1); /*error*/
    }

    if (fellow.prev_flag) {
      /* There is a previous. */

      if (FD_ISSET(fellow.fd_prev, &rfds)) {
        /* Message(s) from previous. */

        /* Read input. */
        ret = read_stream(&fellow, buffer);
        if (ret == 0) {
          /* The previous disconnected */
          fellow.prev_flag = 0;
          close(fellow.fd_prev);

          printf("PROTOCOL: the previous disconnect TCP\n");
        } else if (ret == 1) {
          /* Invalid input message. */

          exit_f = 1;
        }
      }

    }

    if (FD_ISSET(fellow.fd_central, &rfds)) {
      /* TODO: ignorar mensagem fora de tempo */

    }

    if (FD_ISSET(0, &rfds)) {
      /* Read user input. */
      sel_in = parse_user_input(&(fellow.service));

      switch (sel_in) {
        case IN_JOIN:
          /* Regist server in central, with the input service. */
          regist_on_central(&fellow);

          break;
        case IN_STATE:
          /* Print server state. */
          show_state(&fellow);

          break;
        case IN_LEAVE:
          /* Leave the service ring. */

          exit_ring(&fellow);
          break;
        case IN_EXIT:
          exit_f = 1;

          if (fellow.service != -1) {
            /* Has to exit the service ring. */

            exit_ring(&fellow);
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

    if (FD_ISSET(fellow.fd_service, &rfds)) {
      /* Respond to a client request. */

      serve_client(&fellow);
    }

    if (FD_ISSET(fellow.fd_listen, &rfds)) {
      /* A fellow is trying to connect to this one. */
      if (!fellow.start) {
        /* Out of time connection. Only makes sense if this is start? */
        printf("Erro: TCP accept by a non-start server");
      }

      if (fellow.prev_flag) {
        /* Disconnects from previous. */

        printf("PROTOCOL: disconnect from previous\n");
        close(fellow.fd_prev);

        fellow.prev_flag = 0;
      }

      /* Accept new fellow. */
      printf("PROTOCOL: will accept NEW\n");
      addrlen = sizeof(addr_acpt);
      if ( (fellow.fd_prev = accept( fellow.fd_listen,
           (struct sockaddr*) &addr_acpt, &addrlen) ) == -1 ) {
        printf("Erro: TCP Accept");
        exit(1); /* error */
      }
      printf("PROTOCOL: accepted NEW\n");

      /* Mark it is listining to a fellow and clean input buffer. */
      fellow.prev_flag = 1;
      buffer[0] = '\0';

      fellow.nw_arrival_flag = 1;
    }
  }

  /* Exit. */
  destroy_fellow(&fellow);

  return 0;
}

void get_arguments(int argc, const char *argv[], int *id, char *ip, int *upt, int *tpt, char *csip, int *cspt) {
  int i;
  char ident;
  int csi = 0, csp = 0;
  struct hostent *h;
  struct in_addr *a;

  if (argc < 9) {
    printf("Error: incorrect number of arguments\n");
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
        printf("Error: Argument not known\n");
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
  int new_sv, arg_read;

  if (fgets(buffer, MAX_STR, stdin) == NULL) {
    /* Nothing to read. */

    return IN_ERROR;
  }

  arg_read = sscanf(buffer, "%s", cmd);
  if (arg_read != 1) {
		/*Argument not read*/
		printf("Error: Invalid message\n");
	}

  /* Parse input. */
  if (strcmp(cmd, "join") == 0) {
    /* Read service id. */
    arg_read = sscanf(buffer, "%*s %d", &new_sv);
    if (arg_read != 1) {
      /*Argument not read*/
      printf("Error: Invalid message\n");
    }

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

void serve_client(struct fellow *fellow) {
  int ret, nread, arg_read, char_read;
  struct sockaddr_in addr_client;
  socklen_t addrlen;
  char toggle[MAX_STR], msg_in[MAX_STR], msg_out[MAX_STR];

  memset((void*) &(addr_client), (int) '\0', sizeof(addr_client));
  addrlen = sizeof(struct sockaddr_in);

  nread = recvfrom( fellow->fd_service, msg_in, MAX_STR, 0,
                    (struct sockaddr*) &(addr_client), &addrlen );
  if (nread==-1) {
    exit(1); /*error*/
  }
  printf("CLIENT: Entered in serving\n");
  msg_in[nread] = '\0';
  printf("%s\n", msg_in);

  arg_read = sscanf(msg_in, "MY_SERVICE %s%n", toggle, &char_read);
  if (arg_read != 1) {
    /*Argument not read*/
    printf("Error: Invalid message");
    exit(1);
  }
  if (char_read != strlen(msg_in)) {
    printf("Error: Not every character was read");
    exit(1);
  }

  if (strcmp(toggle, "ON") == 0){
    become_unavailable(fellow);
    sprintf(msg_out, "YOUR_SERVICE ON");
    ret = sendto( fellow->fd_service, msg_out, strlen(msg_out), 0,
                  (struct sockaddr*) &(addr_client), addrlen );
    if (ret==-1) {
      exit(1); /*error*/
    }

    /* FIXME: rework this all */
  } else if (strcmp(toggle, "OFF") == 0) {
    become_available(fellow);
    sprintf(msg_out, "YOUR_SERVICE OFF");
    ret = sendto( fellow->fd_service, msg_out, strlen(msg_out), 0,
                  (struct sockaddr*) &(addr_client), addrlen );
    if (ret==-1) {
      exit(1); /*error*/
    }

    /* FIXME: rework this all */
  } else {
    /* TODO: invalid message. */

  }
}
