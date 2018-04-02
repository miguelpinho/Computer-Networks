/*
  authors: Miguel Malaca, Miguel Pinho

  description:
    variables for each server on the service ring
*/

#ifndef FELLOW_H
#define FELLOW_H

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

#define MAX_STR 128

/* next server in the fellowship ring */
struct next_fellow {
  int id;
  char ip[MAX_STR];
  int tpt;
  int fd_next;
};

/* state and sockets each fellow stores */
struct fellow {
  /* state */
  int start, available, ring_unavailable, dispatch;
  int nw_arrival_flag; /* a new fellow is connecting */
  int nw_available_flag; /* a fellow is available when the ring is unavailable */
  int prev_flag; /* there is a previous */

  /* service */
  int service;

  /* sockets */
  int fd_central, fd_service;
  int fd_listen, fd_prev;

  /* next */
  struct next_fellow next;

  /* address */
  int id, upt, tpt, cspt;
  char ip[MAX_STR], csip[MAX_STR];
};

void new_fellow(struct fellow *this);
void create_sockets(struct fellow *fellow);
void destroy_fellow(struct fellow *this);
void show_state(struct fellow *fellow);
void register_cs(char *reply, struct fellow *fellow);
void set_cs(char *query, struct fellow *fellow, int pt);
void withdraw_cs(char *query, struct fellow *fellow);

#endif
