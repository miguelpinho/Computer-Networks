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
#include <signal.h>
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
void intHandler(int);

int main(int argc, char const *argv[]) {
  int sel_in, exit_f = 0, exit_delay = 0, max_fd = 0;
  int fd_tmp;
  int counter, ret, nrecv;
  fd_set rfds;
  struct fellow fellow;
  struct sockaddr addr_acpt;
  struct sockaddr_in addr_central;
  socklen_t addrlen = sizeof(addr_acpt), addrlen_c;
  char msg_in[MAX_STR];

  signal(SIGINT, intHandler);
  new_fellow(&fellow);

  get_arguments(argc, argv, &(fellow.id), fellow.ip, &(fellow.upt),
                &(fellow.tpt), fellow.csip, &(fellow.cspt));

  create_sockets(&fellow);

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
    if (fellow.nw_arrival_flag != NO_NEW) {
      FD_SET(fellow.fd_new_arrival, &rfds); max_fd = max(max_fd, fellow.fd_new_arrival);
    }

    counter = select(max_fd+1, &rfds, (fd_set*) NULL, (fd_set*) NULL, (struct timeval *) NULL);
    if (counter <= 0) {
      perror("Error: select\nDescription:");
      brute_exit(&fellow);
    }

    if (fellow.prev_flag) {
      /* There is a previous. */

      if (FD_ISSET(fellow.fd_prev, &rfds)) {
        /* Message(s) from previous. */

        /* Read input. */
        ret = read_stream(&fellow);
        if (ret == 0) {
          /* The previous disconnected */
          fellow.prev_flag = 0;
          close(fellow.fd_prev);

          if (fellow.nw_arrival_flag == DONE_NEW) {
            /* The NEW is promoted to previous */

            new_to_prev(&fellow);
          } else if (fellow.wait_connect != 1) {
            /* This seems an unexpected disconnection, warn user. */

            printf("WARNING: unwarned disconnect from previous. Ring seems broken. Disconnecting graciously\n");
            brute_exit(&fellow);
          }
        } else if (ret == -1) {
          /* Invalid input message. */
          printf("Error: Invalid message from previous server\n");
          brute_exit(&fellow);
        }
      }

    }

    if (fellow.nw_arrival_flag != NO_NEW) {
      /* A NEW arrival is still pending */

      if (FD_ISSET(fellow.fd_new_arrival, &rfds)) {
        ret = read_aux_stream(&fellow);
        if (ret == 0) {
          /* NEW disconnected */

          fellow.nw_arrival_flag = NO_NEW;
          close(fellow.fd_new_arrival);
        } else if (ret == -1) {
          /* Invalid input message. */

          brute_exit(&fellow);
        }
      }
    }

    if (FD_ISSET(fellow.fd_central, &rfds)) {
      /* Ignore out of time messages from central server*/

      memset((void*) &(addr_central), (int) '\0', sizeof(addr_central));
    	addr_central.sin_family = AF_INET;
    	addr_central.sin_addr.s_addr = inet_addr(fellow.csip);
    	addr_central.sin_port = htons(fellow.cspt);

      addrlen_c = sizeof(addr_central);

      nrecv = recvfrom( fellow.fd_central, msg_in, MAX_STR, 0,
                        (struct sockaddr*) &(addr_central), &addrlen_c );
      if( nrecv == -1 ) {
        perror("Error: Receive from central\nDescription:");
        brute_exit(&fellow);
      }

      printf("Warning: Received out of time message - %s", msg_in);

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

          trigger_exit_ring(&fellow);
          break;
        case IN_EXIT:
          exit_delay = 1;

          if (fellow.service != -1) {
            /* Has to exit the service ring. */

            trigger_exit_ring(&fellow);
          } else {
            exit_f = 1;
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

      if (fellow.wait_connect == 1) {
        /* Connection remake in exit protocol. */

        if (fellow.prev_flag) {
          /* Disconnects from previous. */

          if (fellow.prev_flag == 1) {
            close(fellow.fd_prev);
            fellow.prev_flag = 0;
          }
        }

        fellow.wait_connect = 0;

        /* Accept fellow. */
        addrlen = sizeof(addr_acpt);

        if ( (fellow.fd_prev = accept( fellow.fd_listen,
            (struct sockaddr*) &addr_acpt, &addrlen) ) == -1 ) {
          perror("Error: TCP Accept\nDescription:");
          brute_exit(&fellow);
        }

        /* Mark it as listining to a fellow and clean input buffer. */
        fellow.prev_flag = 1;
        fellow.in_buffer[0] = '\0';
      } else if (fellow.start == 1) {
        /* NEW fellow. */

        if (fellow.nw_arrival_flag == NO_NEW) {
          /* Accept NEW fellow. */
          addrlen = sizeof(addr_acpt);

          if ( (fellow.fd_new_arrival = accept( fellow.fd_listen,
              (struct sockaddr*) &addr_acpt, &addrlen) ) == -1 ) {
            perror("Error: TCP Accept\nDescription:");
            brute_exit(&fellow);
          }

          /* Mark it as listining to a new fellow and clean aux input buffer. */
          fellow.nw_arrival_flag = TRIG_NEW;
          fellow.aux_in_buffer[0] = '\0';
        } else {
          /* Already accepting a new fellow, reject this one. */
          printf("WARNING: a NEW server was trying to connect, but was rejected because there is already one waiting\n");

          if ( (fd_tmp = accept( fellow.fd_listen,
              (struct sockaddr*) &addr_acpt, &addrlen) ) == -1 ) {
            perror("Error: TCP Accept\nDescription:");
            brute_exit(&fellow);
          }

          close(fd_tmp);
        }


      } else {
        /* Out of time connection. Only makes sense if this is start? */

        printf("Error: TCP accept out of time");
        brute_exit(&fellow);
      }
    }

    /* Check if exit state has changed */
    if (fellow.exiting == DONE_EXIT) {

      if (exit_delay == 1) {

        exit_f = 1;
      } else {

        fellow.exiting = NO_EXIT;
      }
    }
  }
  /* Exit. */
  destroy_fellow(&fellow);

  return 0;
}

void get_arguments(int argc, const char *argv[], int *id, char *ip, int *upt, int *tpt, char *csip, int *cspt) {
  int i;
  char ident;
  int csi = 0, csp = 0, arg_read;
  struct hostent *h;
  struct in_addr *a;

  if (argc < 9 || argc > 13) {
    printf("Error: incorrect number of arguments\n");
    exit(1);
  }

  for (i = 1; i < argc; i+=2) {

    arg_read = sscanf(argv[i],"-%c", &ident);
    if (arg_read != 1) {
      printf("Error: Invalid argument. Usage: %s\n", USAGE);
      exit(1);
    }

    switch(ident){
      case 'n' :
        arg_read = sscanf(argv[i+1],"%d", id);
        if (arg_read != 1) {
          printf("Error: Invalid argument. Usage: %s\n", USAGE);
          exit(1);
        }
        break;
      case 'j' :
        strcpy(ip, argv[i+1]);
        break;
      case 'u' :
        arg_read = sscanf(argv[i+1],"%d", upt);
        if (arg_read != 1) {
          printf("Error: Invalid argument. Usage: %s\n", USAGE);
          exit(1);
        }
        break;
      case 't' :
        arg_read = sscanf(argv[i+1],"%d", tpt);
        if (arg_read != 1) {
          printf("Error: Invalid argument. Usage: %s\n", USAGE);
          exit(1);
        }
        break;
      case 'i' :
        strcpy(csip, argv[i+1]);
        csi = 1;
        break;
      case 'p' :
        arg_read = sscanf(argv[i+1],"%d", cspt);
        if (arg_read != 1) {
          printf("Error: Invalid argument. Usage: %s\n", USAGE);
          exit(1);
        }
        csp = 1;
        break;
      default:
        printf("Error: Argument not known. Usage: %s\n", USAGE);
        exit(1);
    }
  }

  if (csi != 1) {
    if((h = gethostbyname(DEFAULT_HOST)) == NULL) {
      exit(1);
    }

    a = (struct in_addr*) h->h_addr_list[0];
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
    return IN_ERROR;
	}

  /* Parse input. */
  if (strcmp(cmd, "join") == 0) {
    /* Read service id. */
    arg_read = sscanf(buffer, "%*s %d", &new_sv);

    if (arg_read != 1) {
      /*Argument not read*/

      printf("Error: Invalid message\n");
      return IN_ERROR;
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

    if (*service != -1) {
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
  socklen_t addrlen;
  char toggle[MAX_STR], msg_in[MAX_STR], msg_out[MAX_STR];

  memset((void*) &(fellow->addr_client), (int) '\0', sizeof(fellow->addr_client));
  addrlen = sizeof(struct sockaddr_in);

  nread = recvfrom( fellow->fd_service, msg_in, MAX_STR, 0,
                    (struct sockaddr*) &(fellow->addr_client), &addrlen );
  if (nread==-1) {
    perror("Error: Receive from client\nDescription:");
    brute_exit(fellow);
  }

  msg_in[nread] = '\0';

  arg_read = sscanf(msg_in, "MY_SERVICE %s%n", toggle, &char_read);
  if (arg_read != 1) {
    /*Argument not read*/
    printf("Error: Invalid message from client\n");
    brute_exit(fellow);
  }
  if (char_read != strlen(msg_in)) {
    printf("Error: Not every character was read\n");
    brute_exit(fellow);
  }

  if (strcmp(toggle, "ON") == 0){
    become_unavailable(fellow);
    sprintf(msg_out, "YOUR_SERVICE ON");
    ret = sendto( fellow->fd_service, msg_out, strlen(msg_out), 0,
                  (struct sockaddr*) &(fellow->addr_client), addrlen );
    if (ret==-1) {
      perror("Error: Sending to client\nDescription:");
      brute_exit(fellow);
    }

  } else if (strcmp(toggle, "OFF") == 0) {
    become_available(fellow);
    sprintf(msg_out, "YOUR_SERVICE OFF");
    ret = sendto( fellow->fd_service, msg_out, strlen(msg_out), 0,
                  (struct sockaddr*) &(fellow->addr_client), addrlen );
    if (ret==-1) {
      perror("Error: Sending to client\nDescription:");
      brute_exit(fellow);
    }

  } else {
    /* invalid message. */
    printf("Error: Invalid message\n");
    brute_exit(fellow);
  }
}

void intHandler ( int exit_f ) {
  return;
}
