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
#define MAX_IP 20
#define N_CHANCES 3

enum new_type {NO_NEW, TRIG_NEW, DONE_NEW};
enum exit_type {NO_EXIT, TRIG_EXIT, DONE_EXIT};

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
  int exiting; /* this fellow is waiting for confirmation of exit */
  int wait_connect; /* waiting from a reconnection from a peer */
  int prev_flag; /* there is a previous */

  /* service */
  int service;

  /* sockets */
  int fd_central, fd_service;
  int fd_listen, fd_prev;
  int fd_new_arrival;

  /* next */
  struct next_fellow next;

  /* address */
  int id, upt, tpt, cspt;
  char ip[MAX_IP], csip[MAX_IP];

  /* in buffer */
  char in_buffer[MAX_STR], aux_in_buffer[MAX_STR];

  struct sockaddr_in addr_client;
};

void new_fellow(struct fellow *this);
void create_sockets(struct fellow *fellow);
void destroy_fellow(struct fellow *this);
void show_state(struct fellow *fellow);
void register_cs(char *reply, struct fellow *fellow);
void set_cs(char *query, struct fellow *fellow, int pt);
void withdraw_cs(char *query, struct fellow *fellow);
void brute_exit(struct fellow *fellow);
void stop_service(struct fellow *fellow);
void lastoption_exit(struct fellow *fellow);


#endif
