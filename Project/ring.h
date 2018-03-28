/*
  authors: Miguel "the white" Malaca, Miguel "Baggins" Pinho
  one ring to serve them all

  description:
    functions for ring and availability management
*/

#ifndef RING_H
#define RING_H

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
#include "stream_msg.h"
#include "fellow.h"

void send_token(char token_type, struct fellow *fellow, int id, int id2, char *ip, int tpt);
void join_ring(struct fellow *fellow , int tpt_start , char *ip_start, int id_start);
void new_arrival_ring(struct fellow *fellow, int id_new, int tpt_new, char *ip_new, int id_start);
void token_new(struct fellow *fellow, int id_start, int id_new, char *ip_new, int tpt_new);
void token_exit(struct fellow *fellow, int id_out, int id_next, char *ip_next, int tpt_next);
int exit_ring(struct fellow *fellow);
void token_newstart( struct fellow fellow , int service, int fd_central , struct sockaddr addr_central);

int process_message (char *msg, struct fellow *fellow);
int message_nw_arrival (char *msg, struct fellow *fellow);

#endif
