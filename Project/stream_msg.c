/*
  authors: Miguel Malaca, Miguel Pinho

  description:
    functions for input stream processing
*/

#include "stream_msg.h"

void insert_n(struct stream_buffer *buffer, char *src, int n);

void init_stream(struct stream_buffer *buffer) {

  buffer->begin = 0;
  buffer->end = 0;
}

int readto_stream(int fd, struct stream_buffer *str_buffer) {
  int n, total = 0;
  char buffer[MAX_STR];

  while ((n = read(fd, buffer, MAX_STR)) != 0) {
    if (n == -1) {
      exit(1); /* error */
    }

    insert_n(str_buffer, buffer, n);

    total += n;
  }

  return total;
}

int get_stream(char *dest, struct stream_buffer *buffer) {
  int i = buffer->begin, n = 0;
  char ch;

  while (i != buffer->end) {

    if ((ch = buffer->stream[i]) == '\n') {
      if (++i >= STREAM_STR) i = 0;
      buffer->begin = i;

      dest[n] = '\0';
      return n;
    }

    dest[n] = ch;

    if (++i >= STREAM_STR) i = 0;
    n++;
  }

  return -1;
}

void insert_n(struct stream_buffer *buffer, char *src, int n) {
  int i, end = buffer->end;

  for (i = 0; i < n; i++) {
    buffer->stream[end] = src[i];

    end++;
    if (end >= STREAM_STR) end = 0;
  }

  buffer->end = end;
}
