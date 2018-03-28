/*
  authors: Miguel Malaca, Miguel Pinho

  description:
    functions for input stream processing
*/

#ifndef STREAM_MESSAGES_H
#define STREAM_MESSAGES_H

#define MAX_STR 128
#define STREAM_STR 1024

struct stream_buffer {
  char stream[STREAM_STR];
  int begin;
  int end;
};

void init_stream(struct stream_buffer *buffer);
int readto_stream(int fd, struct stream_buffer *str_buffer);
int get_stream(char *dest, struct stream_buffer *buffer);

#endif
