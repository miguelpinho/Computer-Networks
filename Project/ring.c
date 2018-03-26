/*
  authors: Miguel "the white" Malaca, Miguel "Baggins" Pinho
  one ring to serve them all

  description:
    functions for ring and availability management
*/

/* next server in the fellowship ring */
struct next_fellow {
  int id;
  char ip[MAX_STR];
  int tpt;
  int fd_next;
};

/* state and sockets each fellow stores */
struct fellow {
  /* address */
  int id, tpt;
  char ip[MAX_STR];

  /* state */
  int start, available, ring_unavailable, dispatch;

  /* next */
  struct next_fellow next;

  /* sockets */
  int fd_listen, fd_prev;
  int fd_aux; /* for ring transient states */
}

void new_fellow(struct fellow *this, int id, char *ip, int tpt) {
  struct sockaddr_in addr_fellow;

  /* store id and address */
  this->id = id;
  strncpy(this->ip, ip, MAX_STR-1);
  this->tpt = tpt;

  /* FIXME: initialize state variables */
  start = -1;
  available = -1;
  ring_available = -1;
  dispatch = -1;

  /* FIXME: set next to none */
  this->next.id = -1;

  /* create listen socket */
  if ((this->fd_listen = socket(AF_INET,SOCK_STREAM,0)) == -1) {
    exit(1); /* error */
  }

  memset((void*) &addr_fellow, (int) '\0', sizeof(addr_fellow));

  addr_fellow.sin_family = AF_INET;
  addr_fellow.sin_addr.s_addr = htonl(INADDR_ANY);
  addr_fellow.sin_port = htons(tpt);

  if (bind(this->fd_listen, (struct sockaddr*) &addr_fellow, sizeof(addr_fellow)) == -1) {
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

void send_token( char token_type, struct fellow, int id2, char *ip, int tpt, int id_start) {

int n, nbytes, nleft, nwritten, ptr;
char msg_out[MAX_STR], buffer[MAX_STR];

  switch (token_type) {
    case 'S': case 'T': case 'I': case 'D':
      sprintf(msg_out, "TOKEN %d;%c\n", fellow.id, token_type);
      break;
    case 'N':
      sprintf(msg_out, "%d;%c;%d;%s;%d\n", id_start, token_type, id2, ip, tpt);
      break;
    case 'O':
      sprintf(msg_out, "%d;%c;%d;%s;%d\n", fellow.id, token_type, fellow.next.id, fellow.next.ip, fellow.next.tpt);
      break;
    default:
      printf("Error: Token type not known\n");
      return;
      break;
  }

  /* addr_next.sin_family = AF_INET;
  addr_next.sin_addr.s_addr = inet_addr(fellow.next.ip);
  addr_next.sin_port = htons(fellow.next.tpt); */

  /* n=connect(fellow.next.fd_next, (struct sockaddr*)&addr_next,sizeof(addr_next));
	if(n==-1) exit(1); /* error */

	ptr=strcpy(buffer, msg_out);
	nbytes=strlen(msg_out);

	nleft=nbytes;
	while(nleft>0) {
		nwritten=write(fellow.next.fd_next,ptr,nleft);
		if(nwritten<=0) exit(1);//error

		nleft-=nwritten;
		ptr+=nwritten;
	}

	/* nleft=nbytes;
	ptr=buffer;
	while(nleft>0) {
		nread=read(fellow.fd_prev,ptr,nleft);
		if(nread==-1) exit(1); //error
		else if(nread==0) break; //closed by peer
		nleft-=nread;
		ptr+=nread;
	}
	nread=nbytes-nleft; */
}

/*****STEP 2 BEGIN: ring maintenance*****/
/* GET_start is not null */
void join_ring( struct fellow *fellow , int tpt_start , char *ip_start, int id_start) {
  /* connect to start server */

  fellow->next.id = id_start;
  fellow->next.tpt = tpt_start;
  strcpy(fellow->next.ip, ip_start);

  fellow->next.fd_next=socket(AF_INET,SOCK_STREAM,0); //TCP socket
	if(fellow->next.fd_next==-1) exit(1);//error

  addr_next.sin_family = AF_INET;
  addr_next.sin_addr.s_addr = inet_addr(ip_start);
  addr_next.sin_port = htons(tpt_start);

  n=connect(fellow.next.fd_next, (struct sockaddr*)&addr_next,sizeof(addr_next));
	if(n==-1) exit(1); /* error */

  sprintf(msg_out, "NEW %d;%s;%d\n", fellow->id, fellow->ip, fellow->tpt);

  ptr=strcpy(buffer, msg_out);
	nbytes=strlen(msg_out);

	nleft=nbytes;
	while(nleft>0) {
		nwritten=write(fellow.next.fd_next,ptr,nleft);
		if(nwritten<=0) exit(1);//error

		nleft-=nwritten;
		ptr+=nwritten;
	}


  /* wait for connection? */
  /* listen?? */

  /* how does it communicate it is available??? send D anyway? */
  /* FIXME: ask Saruman */
}

/* Start accepted new tcp */
void new_arrival_ring(struct fellow *fellow, int id_new, int tpt_new, char *ip_new, int id_start) {
  /* Fellow received a tcp connection to it. */
  /* Only makes sense if this is start? Check? */

  if (fellow->next.id == -1) {
    /* Start was alone in the ring. Connect it to the new. */

    fellow->next.id = id_new;
    fellow->next.tpt = tpt_new;
    strcpy(fellow->next.ip, ip_new);

    fellow->next.fd_next=socket(AF_INET,SOCK_STREAM,0); //TCP socket
  	if(fellow->next.fd_next==-1) exit(1);/* error */

    addr_new.sin_family = AF_INET;
    addr_new.sin_addr.s_addr = inet_addr(ip_new);
    addr_new.sin_port = htons(tpt);

    n = connect(fellow->next.fd_next, (struct sockaddr*)&addr_new,sizeof(addr_new));
    if (n==-1) {
      exit(1); /* error */
    }

  } else {
    /* There are more fellows in the ring. Pass token to warn tail. */

    send_token('N', *fellow, id_new, ip_new, tpt_new, id_start);

    /* TODO: wait for desconnection from tail?? */
  }

}

/* Receives token new */
void token_new(struct fellow *fellow, int id2, char *ip, int tpt) {
  int fd_destroy; /* Tmp, for connection to start to delete */

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

      fellow->next.fd_next=socket(AF_INET,SOCK_STREAM,0); //TCP socket
    	if(fellow->next.fd_next==-1) exit(1);/* error */

      addr_new.sin_family = AF_INET;
      addr_new.sin_addr.s_addr = inet_addr(ip);
      addr_new.sin_port = htons(tpt);

      n = connect(fellow->next.fd_next, (struct sockaddr*)&addr_new,sizeof(addr_new));
    	if (n==-1) {
        exit(1); /* error */
      }

    } else {
      /* This is not the tail. Pass the token. */

      send_token('N', *fellow, id2, ip, tpt);
    }

}

/* User input exit */
int exit_ring( struct fellow fellow, int id_start, int fd_udp, int service, struct sockaddr addr_central) {
  /* if (start) */
    /* withdraw start */
  if ( felow.id == id_start) {

    /* Check if there is one server with the wanted service. */
    sprintf(msg_out, "WITHDRAW_START %d;%d", service, fellow.id);
    nsend = sendto(fd_udp, msg_out, strlen(msg_out), 0, (struct sockaddr*) &addr_central, sizeof(addr_central));
  	if( nsend == -1 ) {
      printf("Error: send");
      exit(1); /*error*/
    }
    *addrlen = sizeof(addr_central);
  	nrecv = recvfrom(fd_udp, msg_in, 128, 0, (struct sockaddr*) &addr_central, addrlen);
  	if( nrecv == -1 ) {
      printf("Error: recv");
      exit(1); /*error*/
    }
    msg_in[nrecv] = '\0';
    printf("%s\n", msg_in);

  }

    /* if (next is null) */
      /* return (ring is over) */
  if (fellow.next.id == -1) {
    /* Ring is over */
    return 0;

    /* else */
      /* set next as start */
  } else {
    /* This is wrong, it has to send the new_start message */
    sprintf(msg_out, "SET_START %d;%d;%s;%d", service, fellow.next.id, fellow.next.ip, fellow.next.tpt);
    nsend = sendto(fd_udp, msg_out, strlen(msg_out), 0, (struct sockaddr*) &addr_central, sizeof(addr_central));
  	if( nsend == -1 ) {
      printf("Error: send");
      exit(1); /*error*/
    }
    *addrlen = sizeof(addr_central);
  	nrecv = recvfrom(fd_udp, msg_in, 128, 0, (struct sockaddr*) &addr_central, addrlen);
  	if( nrecv == -1 ) {
      printf("Error: recv");
      exit(1); /*error*/
    }
    msg_in[nrecv] = '\0';
    printf("%s\n", msg_in);
  }

  /* pass token exit */
  send_token('O', fellow, 0, "0", 0, 0, 0);

  /* wait for disconnect? */
}

/* Receives token exit */
void token_exit(struct fellow *fellow, int id_out, int id_next, int tpt2, char *ip2) {
  /* if (id == next) */
    /* if (id2 == self) */
      /* disconnect from next */
      /* set next to null */
  if (id_out == fellow->next.id) {
    if (fellow.id == id_next) {
      close(fellow->next.fd_next);
      fellow->next.id = -1;
    } else {
      /* else *
        /* disconnect id */
        /* connect to id2 */
      close(fellow->next.fd_next);
      fellow->next.id = id2;
      strcpy(fellow->next.ip, ip2);
      fellow->next.tpt = tpt2;

      fellow->next.fd_next=socket(AF_INET,SOCK_STREAM,0); //TCP socket
    	if(fellow->next.fd_next==-1) exit(1);/* error */

      addr_new.sin_family = AF_INET;
      addr_new.sin_addr.s_addr = inet_addr(ip2);
      addr_new.sin_port = htons(tpt2);

      n = connect(fellow->next.fd_next, (struct sockaddr*)&addr_new,sizeof(addr_new));
    	if (n==-1) {
        exit(1); /* error */
      }

    }
  } else {
    /* else */
      /* pass the O token */
    send_token('O', fellow, 0, "0", 0, 0, 0);
  }




}

void process_message (char *msg, struct fellow *fellow) {

  char buffer[MAX_STR], msg_data[MAX_STR], msg_type[MAX_STR], token_type, ip[MAX_STR];
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
        token_new(fellow, id2, ip, tpt);
        break;
      case 'O':
        sscanf(msg_data, "%*d;%*c;%d;%[^; ];%d", &id2, ip, &tpt);
        token_exit(fellow, id, id2, tpt, ip);
        break;
      default:
        printf("Error: Token type not known\n");
        return 1;
        break;
    }
  } else if (strcmp(msg_type, "NEW")== 0) {
    sscanf(msg_data, "%d;%[^;];%d", &id, ip, &tpt);
    new_arrival_ring(fellow, id, tpt, ip, fellow.id);
  } else if (strcmp(msg_type, "NEW_START") == 0)   {

  } else {
    printf("Error: Message not known\n");
    return 0;
  }


  printf("%d %s %d %d \n", id, ip, id2, tpt);
  return 1;
}
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
  //
}

void token_transfer() {

}

void become_available() {

}

void get_state() {

}
/*****STEP 3 END*****/
