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
      sprintf(msg_out, "TOKEN %d;%c;%d;%s;%d\n", id, token_type, id2, ip, tpt);
      break;
    case 'O':
      sprintf(msg_out, "TOKEN %d;%c;%d;%s;%d\n", id, token_type, id2, ip, tpt);
      break;
    default:
      printf("Error: Token type not known - %c\n", token_type);
      return;
      break;
  }

	ptr = &msg_out[0];

	nleft = strlen(msg_out);
	while(nleft > 0) {
		nwritten = write(fellow->next.fd_next, ptr, nleft);
		if (nwritten <= 0) {
      perror("Error: Invalid write sending token\nDescription");
      brute_exit(fellow);
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
      perror("Error: Invalid write sending NEW\nDescription:");
      brute_exit(fellow);
    }

		nleft -= nwritten;
		ptr += nwritten;
	}
}

void send_new_start(struct fellow *fellow) {
  char msg_out[MAX_STR], *ptr;
  int nwritten, nleft;

  sprintf(msg_out, "NEW_START\n");

  ptr = &msg_out[0];

  nleft = strlen(msg_out);
	while(nleft > 0) {
		nwritten = write(fellow->next.fd_next, ptr, nleft);
		if (nwritten <= 0) {
      perror("Error: Invalid write sending NEW_START\nDescription:");
      brute_exit(fellow);
    }

		nleft -= nwritten;
		ptr += nwritten;
	}
}

int read_stream(struct fellow *fellow) {
  int n;
  char msg_in[MAX_STR], tmp[MAX_STR], msg_parse[MAX_STR], *ch, *cur;

  strcpy(tmp, fellow->in_buffer);

  n = read(fellow->fd_prev, msg_in, MAX_STR);

  if (n == -1) {
    perror("Error: read TCP\nDescription");
    brute_exit(fellow);
  } else if (n == 0) {
    /* The previous disconnected */

    return 0;
  }

  msg_in[n] = '\0';

  strcat(tmp, msg_in);

  cur = &(tmp[0]);
  while ((ch = strchr(cur, '\n')) != NULL) {
    *(ch) = '\0';

    strcpy(msg_parse, cur);

    /* Parse the message. */

    if (process_message(msg_parse, fellow) == 0) {
      /* Error on tcp message protocol. */

      printf("Error: Error on TCP message protocol");
      brute_exit(fellow);
    }

    cur = ch+1;
  }

  strcpy(fellow->in_buffer, cur);

  return n;
}

void parse_buffer(struct fellow *fellow) {
  char tmp[MAX_STR], msg_parse[MAX_STR], *ch, *cur;

  strcpy(tmp, fellow->in_buffer);

  cur = &(tmp[0]);
  while ((ch = strchr(cur, '\n')) != NULL) {
    *(ch) = '\0';

    strcpy(msg_parse, cur);

    /* Parse the message. */

    if (process_message(msg_parse, fellow) == 0) {
      /* Error on tcp message protocol. */

      printf("Error: Error on TCP message protocol");
      brute_exit(fellow);
    }

    cur = ch+1;
  }

  strcpy(fellow->in_buffer, cur);
}

int read_aux_stream(struct fellow *fellow) {
  int n;
  char msg_in[MAX_STR], tmp[MAX_STR], msg_parse[MAX_STR], *ch, *cur;

  strcpy(tmp, fellow->aux_in_buffer);

  n = read(fellow->fd_new_arrival, msg_in, MAX_STR);

  if (n == -1) {
    perror("Error: read NEW TCP\nDescription");
    return(-1);
  } else if (n == 0) {
    /* The previous disconnected */

    return 0;
  }

  msg_in[n] = '\0';

  strcat(tmp, msg_in);

  cur = &(tmp[0]);
  if (fellow->nw_arrival_flag == TRIG_NEW) {
    if ((ch = strchr(cur, '\n')) != NULL) {
      *(ch) = '\0';

      strcpy(msg_parse, cur);

      /* Parse new arrival message. */

      if (message_nw_arrival(msg_parse, fellow) == 0) {
        /* TODO: Error on tcp msg_parse protocol. */
        printf("Error: Error on TCP message protocol");
        return -1;
      }

      cur = ch+1;
    }
  }

  strcpy(fellow->aux_in_buffer, cur);

  return n;
}

int process_message (char *msg, struct fellow *fellow) {

  char msg_type[MAX_STR], token_type, ip[MAX_STR];
  int id, tpt, id2, arg_read, char_read;

  arg_read = sscanf(msg, "%s", msg_type);
  if (arg_read != 1) {
    /*Argument not read*/
    goto error_msg;
  }

  if (strcmp(msg_type, "TOKEN") == 0) {
    arg_read = sscanf(msg, "%*s %d;%c",&id, &token_type);
    if (arg_read != 2) {
      /*Argument not read*/
      goto error_msg;
    }

    switch (token_type) {
      case 'S':
        token_search(fellow, id);
        break;
      case 'T':
        token_transfer(fellow, id);
        break;
      case 'I':
        token_unavailable(fellow, id);
        break;
      case 'D':
        token_available(fellow, id);
        break;
      case 'N':
        arg_read = sscanf(msg, "%*s %*d;%*c;%d;%[^; ];%d%n", &id2, ip, &tpt, &char_read);
        if (arg_read != 3) {
          /*Argument not read*/
          goto error_msg;
        }
        if (char_read != strlen(msg)) {
          /*Garbage characters*/
          goto error_msg;
        }

        token_new(fellow, id, id2, ip, tpt);
        break;
      case 'O':
        arg_read = sscanf(msg, "%*s %*d;%*c;%d;%[^; ];%d%n", &id2, ip, &tpt, &char_read);
        if (arg_read != 3) {
          /*Argument not read*/
          goto error_msg;
        }
        if (char_read != strlen(msg)) {
          /*Garbage characters*/
          goto error_msg;
        }

        token_exit(fellow, id, id2, ip, tpt);
        break;
      default:
        goto error_msg;
    }
  /* FIXME: check NEW??? */
  } else if (strcmp(msg, "NEW_START") == 0)   {

    token_new_start(fellow);
  } else {

    goto error_msg;
  }

  return 1;

error_msg:
  printf("Error: Invalid message: \"%s\"\n", msg);
  return 0;
}

int message_nw_arrival (char *msg, struct fellow *fellow) {

  char msg_data[MAX_STR], msg_type[MAX_STR], ip[MAX_STR];
  int id, tpt, arg_read, char_read;

  arg_read = sscanf(msg, "%s %s%n", msg_type, msg_data, &char_read);
  if (arg_read != 2) {
    /*Argument not read*/

    goto error_msg;
  }
  if (char_read != strlen(msg)) {
    /*Garbage characters*/

    goto error_msg;
  }

  if (strcmp(msg_type, "NEW")== 0) {
    arg_read = sscanf(msg_data, "%d;%[^;];%d%n", &id, ip, &tpt, &char_read);
    if (arg_read != 3) {
      /*Argument not read*/

      goto error_msg;
    }
    if (char_read != strlen(msg_data)) {
      /*Garbage characters*/

      goto error_msg;
    }

    new_arrival_ring(fellow, id, tpt, ip, fellow->id);
  } else {

    goto error_msg;
  }

  return 1;

error_msg:
  printf("Error: Invalid message (expected NEW): \"%s\"\n", msg);
  return 0;
}

/*****STEP 2 BEGIN: ring maintenance*****/
/* GET_start is not null */
void join_ring(struct fellow *fellow , int tpt_start , char *ip_start, int id_start) {
  int n;
  struct sockaddr_in addr, addr_next;
  socklen_t addrlen = sizeof(addr);

  /* connect to start server */
  fellow->next.id = id_start;
  fellow->next.tpt = tpt_start;
  strcpy(fellow->next.ip, ip_start);

  fellow->next.fd_next = socket(AF_INET,SOCK_STREAM,0);
	if(fellow->next.fd_next==-1) {
    destroy_fellow(fellow);
    perror("Error: Creating socket\nDescription:");
    exit(1); /* error */
  }

  addr_next.sin_family = AF_INET;
  addr_next.sin_addr.s_addr = inet_addr(ip_start);
  addr_next.sin_port = htons(tpt_start);

  n = connect(fellow->next.fd_next, (struct sockaddr*) &addr_next, sizeof(addr_next));
	if (n == -1) {
    perror("Error: TCP Connect\nDescription:");
    brute_exit(fellow);
  }

  /* Communicate to start it wants to enter the ring. */
  send_new(fellow);

  /* wait for connection */
  addrlen = sizeof(addr);
  if ((fellow->fd_prev = accept(fellow->fd_listen, (struct sockaddr*) &addr, &addrlen)) == -1) {
    perror("Error: TCP accecpt\nDescription:");
    brute_exit(fellow);
  }

  fellow->prev_flag = 1;
  fellow->in_buffer[0] = '\0';

  printf("This server has joined ring\n");
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

    fellow->next.fd_next = socket(AF_INET,SOCK_STREAM,0);
  	if(fellow->next.fd_next==-1) {
      perror("Error: Creating socket\nDescription:");
      brute_exit(fellow);
    }

    addr_new.sin_family = AF_INET;
    addr_new.sin_addr.s_addr = inet_addr(ip_new);
    addr_new.sin_port = htons(tpt_new);

    sleep(1);

    n = connect(fellow->next.fd_next, (struct sockaddr*) &addr_new, sizeof(addr_new));
    if (n==-1) {
      perror("Error: failed to connect to NEW\nDescription");
      brute_exit(fellow);
    }

    new_to_prev(fellow);
  } else {
    /* There are more fellows in the ring. Pass token to warn tail. */
    send_token('N', fellow, fellow->id, id_new, ip_new, tpt_new);

    /* The next messages will only be stored in buffer and not processed */
    fellow->nw_arrival_flag = DONE_NEW;
  }

}

void new_to_prev(struct fellow *fellow) {
  /* End of nw_arrival state */
  fellow->nw_arrival_flag = NO_NEW;

  /* Swap buffers */
  fellow->in_buffer[0] = '\0';
  strcpy(fellow->in_buffer, fellow->aux_in_buffer);

  /* Swap sockets */
  fellow->fd_prev = fellow->fd_new_arrival;
  fellow->fd_new_arrival = -1;

  fellow->prev_flag = 1;

  printf("This (START) server has joined a NEW server to the ring\n");

  if (fellow->ring_unavailable == 1) {
    /* Needs to warn new ring is unavailable */

    send_token('I', fellow, fellow->id, 0, 0, 0);
  }

  /* Process messages stored in buffer? */
  parse_buffer(fellow);
}

/* Receives token new */
void token_new(struct fellow *fellow, int id_start, int id_new, char *ip_new, int tpt_new) {

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

    fellow->next.fd_next = socket(AF_INET, SOCK_STREAM, 0);
  	if(fellow->next.fd_next == -1) {
      perror("Error: Creating socket\nDescription:");
      brute_exit(fellow);
    }

    addr_new.sin_family = AF_INET;
    addr_new.sin_addr.s_addr = inet_addr(ip_new);
    addr_new.sin_port = htons(tpt_new);

    n = connect(fellow->next.fd_next, (struct sockaddr*) &addr_new, sizeof(addr_new));
  	if (n == -1) {
      perror("Error: TCP connect\nDescription:");
      brute_exit(fellow);
    }

  } else {
    /* This is not the tail. Pass the token. */

    send_token('N', fellow, id_start, id_new, ip_new, tpt_new);
  }

}

/* User input exit */
int trigger_exit_ring(struct fellow *fellow) {

  /* If this server is providing a service it ends it before exiting */
  if (fellow->available == 0) {
    stop_service(fellow);
  }

  if (fellow ->next.id == -1) {
    /* This fellow is alone, ring exit is fast (jump exit protocol). */
    fellow->exiting = DONE_EXIT; /* Exit */

    if (fellow->dispatch == 1) {
      become_unavailable(fellow);
    }

    if (fellow->start == 1) {
      withdraw_cs("WITHDRAW_START", fellow);
    }

    fellow->service = -1;

    return 2;
  }

  /* Signal this fellow has entered the exit protocol */
  fellow->exiting = TRIG_EXIT;

  if (fellow->dispatch == 1) {
    /* Before it can leave it has to manage the ring availability */

    /* Manage availability inheritance. */
    become_unavailable(fellow);

    return 0;
  } else {
    /* Check if it is locked in the dispactch inheritance protocol */
    /* Before it can leave that has to be solved */
    if (fellow->nw_available_flag != 1) {
      /* Can proceed with exit protocol. */

      launch_exit_ring(fellow);
    }
  }

  return 1;
}

/* Trigered exit ring is ready to be processed */
int launch_exit_ring(struct fellow *fellow) {

  if (fellow->start == 1) {
    /* Manage start inheritance. */
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
  }

  /* Disconnect from next (FIXME) */
  close(fellow->next.fd_next);
  fellow->next.id = -1;

  return 0;
}

/* Receives token exit */
void token_exit(struct fellow *fellow, int id_out, int id_next, char *ip_next, int tpt_next) {
  int n;
  struct sockaddr_in addr;
  socklen_t addrlen;

  if (id_out == fellow->id) {
    /* The ring as been rebuilt, this can exit */

    if (fellow->exiting == NO_EXIT) {
      /* Something is wrong. */
      printf("WARNING: Invalid O token\n");
    }

    printf("This server has exited the ring, after making sure it has been rebuilt\n");

    close(fellow->fd_prev);
    fellow->prev_flag = 0;
    fellow->service = -1;

    close(fellow->next.fd_next);
    fellow->next.id = -1;

    fellow->exiting = DONE_EXIT;

    return;
  }

  if (fellow->next.id == id_out) {
    /* This is the previous fellow of the one leaving */

    /* Warn the next it can safely exit */
    send_token('O', fellow, id_out, id_next, ip_next, tpt_next);

    if (fellow->id == id_next) {
      /* This fellow will be alone in the ring */

      close(fellow->next.fd_next);
      fellow->next.id = -1;
      fellow->prev_flag = 0;
    } else {
      /* Disconnect from leaving fellow */
      close(fellow->next.fd_next);

      /* Connect to next */
      fellow->next.id = id_next;
      strcpy(fellow->next.ip, ip_next);
      fellow->next.tpt = tpt_next;

      fellow->next.fd_next = socket(AF_INET, SOCK_STREAM, 0);
    	if (fellow->next.fd_next == -1) {
        perror("Error: Creating socket\nDescription:");
        brute_exit(fellow);
      }

      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = inet_addr(ip_next);
      addr.sin_port = htons(tpt_next);

      addrlen = sizeof(addr);
      n = connect(fellow->next.fd_next, (struct sockaddr*) &addr, addrlen);
    	if (n == -1) {
        perror("Error: TCP connect\nDescription:");
        brute_exit(fellow);
      }

    }
  } else {

    if (fellow->id == id_next) {
      /* Signal this is waiting for a connection from previous */

      fellow->wait_connect = 1;
    }

    /* pass the O token */
    send_token('O', fellow, id_out, id_next, ip_next, tpt_next);
  }
}

/*When a server receives the new start token*/
void token_new_start(struct fellow *fellow) {

  set_cs("SET_START", fellow, fellow->tpt);

  fellow->start = 1;

  printf("This server is now the START\n");
}
/*****STEP 2 END*****/

/*****STEP 3 BEGIN: manage availability*****/
void become_unavailable( struct fellow *fellow) {

  /* This does not make sense if this is not the dispatch/available */
  if ( fellow->dispatch != 1 || fellow->available != 1 ) {
    printf("Error: Not dispatching or available");
    brute_exit(fellow);
  }

  /* withdraw_ds from cs */
  withdraw_cs("WITHDRAW_DS", fellow);
  fellow->dispatch = 0;
  fellow->available = 0;

  printf("This server is no longer DISPATCH\n");

  if( fellow->next.id == -1 ) {
    /* This was the only fellow */

    fellow->ring_unavailable = 1;
  } else {
    /* Search for a new dispatch */

    send_token('S', fellow, fellow->id, 0, 0, 0);
  }
}

/* What happens when receiving a S token*/
void token_search(struct fellow *fellow, int id_sender) {
  fellow->nw_available_flag = 0;

  /* Check if it is the original sender */
  if ( fellow->id == id_sender) {
    fellow->ring_unavailable = 1;

    /* Send token I, informing other servers that the ring is unavailable */
    send_token('I', fellow, fellow->id, 0, 0, 0);

  /* Check if it is available for dispatch */
  } else if ( fellow->available == 1) {
    set_cs("SET_DS", fellow, fellow->upt);
    fellow->dispatch = 1;

    printf("This server is the new DISPATCH\n");

    send_token('T', fellow, id_sender, 0, 0, 0);
  } else {

    /* Send token S again */
    send_token('S', fellow, id_sender, 0, 0, 0);
  }
}

/* What happens when receiving a token T */
void token_transfer( struct fellow *fellow, int id_sender) {
  fellow->nw_available_flag = 0;

  /* Check if it arrived the sender of the token S */
  if ( fellow->id != id_sender) {

    /* Send token T to the next*/
    send_token('T', fellow, id_sender, 0, 0, 0);

  } else {
    /* If this unavailable was caused by exit, continue protocol */
    if (fellow->exiting == TRIG_EXIT) {

      launch_exit_ring(fellow);
    }
  }
}

/* What happens when receiving a token I */
void token_unavailable( struct fellow *fellow, int id_sender) {
  fellow->nw_available_flag = 0;

  if (fellow->id == id_sender) {
    printf("This server has signaled the ring as UNAVAILABLE\n");

    if (fellow->exiting == TRIG_EXIT) {
    /* The sender of I was waiting for exit. Can proceed now */

    launch_exit_ring(fellow);

    }
  } else {
    /* Discover the ring is unavailable and pass that knowledge */
    fellow->ring_unavailable = 1;
    send_token('I', fellow, id_sender, 0, 0, 0);

    /* After, if it is available, revert unavailability */
    if (fellow->available == 1) {

      become_available(fellow);
    }
  }

}

/* A server becomes available */
void become_available( struct fellow *fellow) {
  fellow->available = 1;

  if( fellow->ring_unavailable == 1 ) {

    if( fellow->next.id == -1 ) {
      /* Server alone in the ring */

      fellow->ring_unavailable = 0;
      fellow->dispatch = 1;
      set_cs("SET_DS", fellow, fellow->upt);
    } else {
      /* Turn on the flag responsible for becoming available */

      fellow->nw_available_flag = 1;
      fellow->ring_unavailable = 0;

      /* Inform the other servers that this one is ready for dispatch */
      send_token('D', fellow, fellow->id, 0, 0, 0);
    }
  }
}

/* What happens when you receive a token D */
void token_available( struct fellow *fellow, int id_sender) {
  fellow->ring_unavailable = 0;
  /* Arrives the sender */
  if (fellow->id == id_sender) {
    fellow->ring_unavailable = 0;
    fellow->nw_available_flag = 0;

    /* Can become new dispatch */
    fellow->dispatch = 1;
    set_cs("SET_DS", fellow, fellow->upt);

    printf("This server has made the ring AVAILABLE and is the new DISPATCH\n");

    if (fellow->exiting != NO_EXIT) {
      /* Is in exit, lose dispatch status it has just gained */

      become_unavailable(fellow);
    }
  } else if (fellow->dispatch == 0) {
    /* Only considers token D if it is not the dispatch */

    if (fellow->nw_available_flag == 0) {
      /* Just pass the D token */

      send_token('D', fellow, id_sender, 0, 0, 0);
    } else {
      /* This one is also waiting for its D token */

      if (id_sender < fellow->id) {
        /* This D has advantage, pass it */
        send_token('D', fellow, id_sender, 0, 0, 0);

        /* Clean new available state and continue exit if that is the case */
        fellow->nw_available_flag = 0;

        if (fellow->exiting == TRIG_EXIT) {
          launch_exit_ring(fellow);
        }
      }
    }
  }
}

/*****STEP 3 END*****/

void regist_on_central(struct fellow *fellow) {
  int id_start, tpt_start, arg_read, char_read;
  char ip_start[MAX_STR];
  char msg[MAX_STR], msg_type[MAX_STR], msg_data[MAX_STR];
  char test_data[MAX_STR];

  register_cs(msg, fellow);

  arg_read = sscanf(msg, "%s %s%n", msg_type, msg_data, &char_read);
  if (arg_read != 2) {
    /*Argument not read*/
    printf("Error: Invalid message: \"%s\"", msg);
    brute_exit(fellow);
  }
  if (char_read != strlen(msg)) {
    printf("Error: Invalid message: \"%s\"", msg);
    brute_exit(fellow);
  }

  if (strcmp(msg_type, "OK") != 0) {
    printf("Error: Invalid message: \"%s\"", msg);
    brute_exit(fellow);
  } else {
    sprintf(test_data, "%d;0;0.0.0.0;0", fellow->id);
    if (strcmp(msg_data, test_data) == 0) {
      /* This is the start server. */
      set_cs("SET_START", fellow, fellow->tpt);

      fellow->start = 1;

      /* Also set the server to dispatch */
      set_cs("SET_DS", fellow, fellow->upt);

      fellow->dispatch = 1;
      fellow->available = 1;
      fellow->ring_unavailable = 0;

      printf("This is the FIRST server of the ring\n");
    } else {
      arg_read = sscanf(msg_data, "%*d;%d;%[^; ];%d%n", &id_start, ip_start, &tpt_start, &char_read);
      if (arg_read != 3) {
	      /*Argument not read*/
	     printf("Error: Invalid message: \"%s\"", msg_data);
	      brute_exit(fellow);
	    }
	    if (char_read != strlen(msg_data)) {
        printf("Error: Not every character was read");
	      brute_exit(fellow);
	    }

      fellow->start = 0;
      fellow->available = 1;

      join_ring(fellow, tpt_start, ip_start, id_start);
    }
  }
}
