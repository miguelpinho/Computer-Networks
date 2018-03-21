/*
  authors: Miguel "the white" Malaca, Miguel "Baggins" Pinho
  one ring to serve them all
*/

struct state_fellow {
  int start;
  int available;
  int dispatch;
}

struct state_ring {
  int available;
}

# next server in the fellowship ring
struct next_fellow {
  int id;
  char ip[MAX_STR];
  int tpt;
};

void enter_ring() {
  /* set next to start server */

  /* how does it communicate it is available??? send D anyway? */

  /* wait for connection? */
}

void new_arrival_ring() {
  /* if (there is a next ) */
    /* conects itself to the new fellow */

  /* else */
    /* passes the  */

    /* wait for desconnection from tail?? */

}

void token_new(int id, int id2, char *ip, int tpt) {
  /* if (id == next) */
    /* connect to id2 */

  /* else */
    /* pass token to next */

}

void exit_ring() {

}

void token_exit() {
  /* if (id == next) */
    /* if (id2 == self) */
      /* set next to null */

    /* else */
      /* connect to id2 */

  /* else */
    /* pass the token */

}

void become_unavailable() {

}

void become_available() {

}

void recv_unavailable() {

}

void recv_available() {

}

void state() {

}
