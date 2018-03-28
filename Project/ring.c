/*
  authors: Miguel "the white" Malaca, Miguel "Baggins" Pinho
  one ring to serve them all

  description:
    functions for ring and availability management
*/

#include "ring.h"

void send_token(char token_type, struct fellow *fellow, int id, int id2, char *ip, int tpt) {
  int nleft, nwritten;
  char msg_out[MAX_STR], *ptr;

  switch (token_type) {
    case 'S': case 'T': case 'I': case 'D':
      sprintf(msg_out, "TOKEN %d;%c\n", id, token_type);
      break;
    case 'N':
      sprintf( msg_out, "%d;%c;%d;%s;%d\n", id, token_type, id2, ip, tpt);
      break;
    case 'O':
      sprintf( msg_out, "%d;%c;%d;%s;%d\n", id, token_type, id2, ip, tpt);
      break;
    default:
      printf("Error: Token type not known\n");
      return;
      break;
  }

	ptr = &msg_out[0];

	nleft = strlen(msg_out);
	while(nleft > 0) {
		nwritten = write(fellow->next.fd_next, ptr, nleft);
		if (nwritten <= 0) {
      exit(1); /* error */
    }

		nleft -= nwritten;
		ptr += nwritten;
	}
}

void send_new(struct fellow *fellow) {
  int nleft, nwritten;
  char msg_out[MAX_STR], *ptr;

  sprintf(msg_out, "NEW %d;%s;%d\n", fellow->id, fellow->ip, fellow->tpt);

  ptr = &msg_out[0];

  nleft = strlen(msg_out);
	while(nleft > 0) {
		nwritten = write(fellow->next.fd_next, ptr, nleft);
		if (nwritten <= 0) {
      exit(1); /* error */
    }

		nleft -= nwritten;
		ptr += nwritten;
	}
}

void send_new_start(struct fellow *fellow) {
  int nleft, nwritten;
  char msg_out[MAX_STR], *ptr;

  sprintf(msg_out, "NEW_START\n");

  ptr = &msg_out[0];

  nleft = strlen(msg_out);
	while(nleft > 0) {
		nwritten = write(fellow->next.fd_next, ptr, nleft);
		if (nwritten <= 0) {
      exit(1); /* error */
    }

		nleft -= nwritten;
		ptr += nwritten;
	}
}

int process_message (char *msg, struct fellow *fellow) {

  char msg_data[MAX_STR], msg_type[MAX_STR], token_type, ip[MAX_STR];
  int id, tpt, id2;

  sscanf(msg, "%s %s", msg_type, msg_data);

  if (strcmp(msg_type, "TOKEN") == 0) {
    sscanf(msg_data, "%d;%c",&id, &token_type);

    printf("%c\n", token_type);

    switch (token_type) {
      case 'S': case 'T': case 'I': case 'D':
        /*TODO*/
        break;
      case 'N':
        sscanf(msg_data, "%*d;%*c;%d;%[^; ];%d", &id2, ip, &tpt);
        token_new(fellow, id, id2, ip, tpt);
        break;
      case 'O':
        sscanf(msg_data, "%*d;%*c;%d;%[^; ];%d", &id2, ip, &tpt);
        token_exit(fellow, id, id2, ip, tpt);
        break;
      default:
        printf("Error: Token type not known\n");
        return 0;
        break;
    }
  /*} else if (strcmp(msg_type, "NEW")== 0) {
      sscanf(msg_data, "%d;%[^;];%d", &id, ip, &tpt);
      new_arrival_ring(fellow, id, tpt, ip, fellow->id);*/
  } else if (strcmp(msg_type, "NEW_START") == 0)   {
      token_new_start(fellow);
  } else {
    printf("Error: Message not known: \"%s\"\n", msg);
    return 0;
  }

  printf("%d %s %d %d\n", id, ip, id2, tpt);
  return 1;
}

int message_nw_arrival (char *msg, struct fellow *fellow) {

  char msg_data[MAX_STR], msg_type[MAX_STR], ip[MAX_STR];
  int id, tpt;

  sscanf(msg, "%s %s", msg_type, msg_data);

  if (strcmp(msg_type, "NEW")== 0) {
    sscanf(msg_data, "%d;%[^;];%d", &id, ip, &tpt);
    new_arrival_ring(fellow, id, tpt, ip, fellow->id);
  } else {
    printf("Error: Invalid message\n");
    return 0;
  }

  printf("%d %s %d \n", id, ip, tpt);
  return 1;
}

/*****STEP 2 BEGIN: ring maintenance*****/
/* GET_start is not null */
void join_ring(struct fellow *fellow , int tpt_start , char *ip_start, int id_start) {
  int n;
  socklen_t addrlen;
  struct sockaddr_in addr, addr_next;

  /* connect to start server */
  fellow->next.id = id_start;
  fellow->next.tpt = tpt_start;
  strcpy(fellow->next.ip, ip_start);

  fellow->next.fd_next = socket(AF_INET,SOCK_STREAM,0);
	if(fellow->next.fd_next==-1) {
    exit(1); /* error */
  }

  addr_next.sin_family = AF_INET;
  addr_next.sin_addr.s_addr = inet_addr(ip_start);
  addr_next.sin_port = htons(tpt_start);

  n = connect(fellow->next.fd_next, (struct sockaddr*) &addr_next, sizeof(addr_next));
	if (n == -1) {
    exit(1); /* error */
  }

  /* Communicate to start it wants to enter the ring. */
  send_new(fellow);

  /* wait for connection */
  if ((fellow->fd_prev = accept(fellow->fd_listen, (struct sockaddr*) &addr, &addrlen)) == -1) {
    exit(1); /* error */
  }

  /* how does it communicate it is available??? send D anyway? */
  /* FIXME: ask Saruman */
}

void new_arrival_ring(struct fellow *fellow, int id_new, int tpt_new, char *ip_new, int id_start) {
  int n;
  struct sockaddr_in addr_new;

  /* Received message NEW. */
  if (fellow->next.id == -1) {
    /* Start was alone in the ring. Connect it to the new. */

    fellow->next.id = id_new;
    fellow->next.tpt = tpt_new;
    strcpy(fellow->next.ip, ip_new);

    fellow->next.fd_next=socket(AF_INET,SOCK_STREAM,0);
  	if(fellow->next.fd_next==-1) exit(1);/* error */

    addr_new.sin_family = AF_INET;
    addr_new.sin_addr.s_addr = inet_addr(ip_new);
    addr_new.sin_port = htons(tpt_new);

    n = connect(fellow->next.fd_next, (struct sockaddr*) &addr_new, sizeof(addr_new));
    if (n==-1) {
      exit(1); /* error */
    }

  } else {
    /* There are more fellows in the ring. Pass token to warn tail. */
    send_token('N', fellow, fellow->id, id_new, ip_new, tpt_new);

    /* TODO: wait for desconnection from tail?? */

  }

}

/* Receives token new */
void token_new(struct fellow *fellow, int id_start, int id_new, char *ip_new, int tpt_new) {
  /*int fd_destroy; // Tmp, for connection to start to delete */
  int n;
  struct sockaddr_in addr_new;

  if(fellow->next.id == id_start) {
    /* This element is the tail of the ring. */

    /* FIXME: is order of connect and disconnect important??? */
    /* Disconnect from the start. */
    fellow->next.id = -1;
    close(fellow->next.fd_next);

    /* Connect to the new element. */
    fellow->next.id = id_new;
    fellow->next.tpt = tpt_new;
    strcpy(fellow->next.ip, ip_new);

    fellow->next.fd_next=socket(AF_INET,SOCK_STREAM,0);
  	if(fellow->next.fd_next==-1) exit(1);/* error */

    addr_new.sin_family = AF_INET;
    addr_new.sin_addr.s_addr = inet_addr(ip_new);
    addr_new.sin_port = htons(tpt_new);

    n = connect(fellow->next.fd_next, (struct sockaddr*) &addr_new, sizeof(addr_new));
  	if (n==-1) {
      exit(1); /* error */
    }

  } else {
    /* This is not the tail. Pass the token. */

    send_token('N', fellow, id_start, id_new, ip_new, tpt_new);
  }

}

/* User input exit */
int exit_ring(struct fellow *fellow) {
  int nsend, nrecv;
  socklen_t addrlen;
  char msg_out[MAX_STR], msg_in[MAX_STR];

  /* Manage availability inheritance. */
  /* TODO */

  /* Manage start inheritance. */
  if (fellow->start == 1) {
    /* Withdraw as start from central */

    withdraw_cs("WITHDRAW_START", fellow);

    if (fellow->next.id != -1) {
      /* Makes the next server the start */

      send_new_start(fellow);
    }

    fellow->start = 0;
  }

  if (fellow->next.id != -1) {
    /* pass token exit */
    send_token('O', fellow, fellow->id, fellow->next.id, fellow->next.ip, fellow->next.tpt);

    /* wait for disconnects, from previous and next */
    /* TODO */
  }

  fellow->next.id = -1;
  fellow->service = -1;

  return 1;
}

/* Receives token exit */
void token_exit(struct fellow *fellow, int id_out, int id_next, char *ip_next, int tpt_next) {
  int n;
  struct sockaddr_in addr;
  socklen_t addrlen;

  if (fellow->next.id == id_out) {
    /* This is the previous fellow of the one leaving */

    if (fellow->id == id_next) {
      /* This fellow will be alone in the ring */

      close(fellow->next.fd_next);
      fellow->next.id = -1;
    } else {
      /* Disconnect from leaving fellow */
      close(fellow->next.fd_next);

      /* Connect to next */
      fellow->next.id = id_next;
      strcpy(fellow->next.ip, ip_next);
      fellow->next.tpt = tpt_next;

      fellow->next.fd_next = socket(AF_INET, SOCK_STREAM, 0);
    	if (fellow->next.fd_next == -1) {
        exit(1);/* error */
      }

      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = inet_addr(ip_next);
      addr.sin_port = htons(tpt_next);

      n = connect(fellow->next.fd_next, (struct sockaddr*) &addr, sizeof(addr));
    	if (n == -1) {
        exit(1); /* error */
      }

    }
  } else {
    if (fellow->id == id_next) {
      /* This is the one after the one leaving. */

      /* Disconnect from leaving fellow */
      close(fellow->prev);
    }

    /* pass the O token */
    send_token('O', fellow, id_out, id_next, ip_next, tpt_next);

    if (fellow->id == id_next) {
      /* Accept previous previous. */

      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = inet_addr(ip_next);
      addr.sin_port = htons(tpt_next);

      if ( (fellow->fd_prev = accept( fellow.fd_listen,
           (struct sockaddr*) &addr, &addrlen) ) == -1 ) {
        exit(1); /* error */
      }
    }
  }
}

/*When a server receives the new start token*/
void token_new_start(struct fellow *fellow) {
  int nsend, nrecv;
  char msg_out[MAX_STR], msg_in[MAX_STR];
  socklen_t addrlen;

  sprintf( msg_out, "SET_START %d;%d;%s;%d", fellow->service, fellow->id,
           fellow->ip, fellow->tpt );
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
  /* TODO: Check if message is OK */
  msg_in[nrecv] = '\0';
  printf("%s\n", msg_in);
}
/*****STEP 2 END*****/

/*****STEP 3 BEGIN: manage availability*****/
void become_unavailable() {
  /* if (is not dispatch)  ?????? */
  /* if (is not available) ?????? */
  /*  */

  /* withdraw_ds from cs */

  /* if (there is no next) */ /* this is the only fellow */
    /* declare the ring recv_unavailable */


  /* else */
    /* creates and sends S token to next */

}

void token_search() {

}

void token_transfer() {

}

void become_available() {

}

void get_state() {

}
/*****STEP 3 END*****/

void regist_on_central(struct fellow *fellow) {
  int id_start, tpt_start;
  char ip_start[MAX_STR];
  char msg[MAX_STR], msg_type[MAX_STR], msg_data[MAX_STR];
  char test_data[MAX_STR];
  socklen_t addrlen;

  register_cs(msg, fellow);

  sscanf(msg, "%s %s", msg_type, msg_data);
  if (strcmp(msg_type, "OK") != 0) {
    printf("Erro: msg\n");
    /* TODO */
  } else {
    sprintf(test_data, "%d;0;0.0.0.0;0", fellow->id);
    if (strcmp(msg_data, test_data) == 0) {
      /* This is the start server. */
      set_cs("SET_START", fellow);

      fellow->start = 1;

      /* Also set the server to dispatch */
      set_cs("SET_DS", fellow);

      fellow->dispatch = 1;
      fellow->available = 1;
      fellow->ring_unavailable = 0;

    } else {
      sscanf(msg_data, "%*d;%d;%[^; ];%d", &id_start, ip_start, &tpt_start);

      join_ring(fellow, tpt_start, ip_start, id_start);
    }
  }
}
