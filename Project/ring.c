/*
  authors: Miguel "the white" Malaca, Miguel "Baggins" Pinho
  one ring to serve them all
*/

struct state_fellow {
  int start;
  int available;
  int ring_unavailable;
  int dispatch;
}

/* next server in the fellowship ring */
struct next_fellow {
  int id;
  char ip[MAX_STR];
  int tpt;
};

/*****STEP 2 BEGIN: ring maintenance*****/
void enter_ring() {
  /* set next to start server */

  /* how does it communicate it is available??? send D anyway? */

  /* wait for connection? */

}

void new_arrival_ring() {
  /* if (there is a next) */
    /* conects itself to the new fellow */

  /* else */
    /* passes the token*/

    /* wait for desconnection from tail?? */

}

void token_new(int id, int id2, char *ip, int tpt) {
  /* if (id == next) */
    /* connect to id2 */

  /* else */
    /* pass token to next */

}

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

void send_token() {

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
