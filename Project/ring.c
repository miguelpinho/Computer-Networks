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

void send_token() {


}

/*****STEP 2 BEGIN: ring maintenance*****/
/* GET_start is not null */
void join_ring() {
  /* connect to start server */

  /* wait for connection? */
  /* listen?? */

  /* how does it communicate it is available??? send D anyway? */
  /* FIXME: ask Saruman */
}

/* Start accepted new tcp */
void new_arrival_ring() {
  /* if (there is a next) */
    /* conects itself to the new fellow */

  /* else */
    /* passes the token*/

    /* wait for desconnection from tail?? */

}

/* Receives token new */
void token_new(int id, int id2, char *ip, int tpt) {
  /* if (id == next) */
    /* connect to id2 */

  /* else */
    /* pass token to next */

}

/* User input exit */
void exit_ring() {
  /* if (start) */
    /* withdraw start */

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
