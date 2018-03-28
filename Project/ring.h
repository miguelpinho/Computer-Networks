/*
  authors: Miguel "the white" Malaca, Miguel "Baggins" Pinho
  one ring to serve them all

  description:
    functions for ring and availability management
*/

#ifndef RING_H
#define RING_H

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
  int nw_arrival_flag; /* a new fellow is connecting */
  int prev_flag; /* there is a previous */

  /* next */
  struct next_fellow next;

  /* sockets */
  int fd_listen, fd_prev;
  int fd_nw_arrival; /* for ring transient states */
}

void new_fellow(struct fellow *this, int id, char *ip, int tpt);
void destroy_fellow(struct fellow *this);

void send_token( char token_type, struct fellow *fellow, int id2, char *ip, int tpt, int id_start);
void join_ring( struct fellow *fellow , int tpt_start , char *ip_start, int id_start);
void new_arrival_ring(struct fellow *fellow, int id_new, int tpt_new, char *ip_new, int id_start);
void token_new(struct fellow *fellow, int id2, char *ip, int tpt);
void token_exit(struct fellow *fellow, int id_out, int id_next, int tpt2, char *ip2);
void token_newstart( struct fellow fellow , int service, int fd_central , struct sockaddr addr_central);

void process_message (char *msg, struct fellow *fellow, int fd_central, struct sockaddr addr_central);

#endif
