/*
  authors: Miguel Malaca, Miguel Pinho

  description:
    variables for each server on the service ring
*/

#ifndef FELLOW_H
#define FELLOW_H

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
  int id, upt, tpt, cspt;
  char ip[MAX_STR], csip[MAX_STR];
  struct sockaddr_in addr_central, addr_service, addr_client;

  /* state */
  int start, available, ring_unavailable, dispatch;
  int nw_arrival_flag; /* a new fellow is connecting */
  int prev_flag; /* there is a previous */

  /* service */
  int service;/

  /* next */
  struct next_fellow next;

  /* sockets */
  int fd_central, fd_service;
  int fd_listen, fd_prev;
  int fd_nw_arrival; /* for ring transient states */
}

void new_fellow(struct fellow *this);
void creat_sockets(struct fellow *fellow);
void destroy_fellow(struct fellow *this);
void regist_on_central(struct fellow *fellow);
void unregister_central (struct fellow *fellow);

#endif
