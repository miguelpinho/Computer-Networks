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
  /* if (there is a next) */
    /* conects itself to the new fellow */

  if(fellow->next.id == id_start) {


    fellow->next.id = id_new;
    fellow->next.tpt = tpt_new;
    strcpy(fellow->next.ip, ip_new);

    fellow->next.fd_next=socket(AF_INET,SOCK_STREAM,0); //TCP socket
  	if(fellow->next.fd_next==-1) exit(1);/* error */

    addr_new.sin_family = AF_INET;
    addr_new.sin_addr.s_addr = inet_addr(ip_new);
    addr_new.sin_port = htons(tpt);

    n=connect(fellow->next.fd_next, (struct sockaddr*)&addr_new,sizeof(addr_new));
    if(n==-1) exit(1); /* error */

    /* desconnection from tail ???? TODO*/

    /* else */
      /* passes the token*/

  } else {

    send_token('N', *fellow, id_new, ip_new, tpt_new, id_start);

  }


    /* wait for desconnection from tail?? */

}

/* Receives token new */
void token_new(struct fellow *fellow, int id2, char *ip, int tpt) {
  /* if (id == next) */
    /* connect to id2 */
    if(fellow->next.id == -1) {
      fellow->next.id = id_new;
      fellow->next.tpt = tpt_new;
      strcpy(fellow->next.ip, ip_new);

      fellow->next.fd_next=socket(AF_INET,SOCK_STREAM,0); //TCP socket
    	if(fellow->next.fd_next==-1) exit(1);/* error */

      addr_new.sin_family = AF_INET;
      addr_new.sin_addr.s_addr = inet_addr(ip);
      addr_new.sin_port = htons(tpt);

      n=connect(fellow->next.fd_next, (struct sockaddr*)&addr_new,sizeof(addr_new));
    	if(n==-1) exit(1); /* error */

  /* else */
    /* pass token to next */

    } else {
      send_token('N', *fellow, id2, ip, tpt);
    }
}

/* User input exit */
void exit_ring( struct fellow *fellow, int id_start) {
  /* if (start) */
    /* withdraw start */
  if ( felow->id == id_start) {

  }

    /* if (next is null) */
      /* return (ring is over) */

    /* else */
      /* set next as start */

  /* pass token exit */

  /* wait for disconnect? */
}

/* Receives token exit */
void token_exit() {
  /* if (id == next) */
    /* if (id2 == self) */
      /* disconnect from next */
      /* set next to null */

    /* else *
      /* disconnect id */
      /* connect to id2 */

  /* else */
    /* pass the E token */

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
