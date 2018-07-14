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


#define MESSAGE "tosta_mista_is_very_great_sake_indeed"

#define MAX_STR 128
#define STREAM_STR 160

struct stream_buffer {
  char stream[STREAM_STR];
  int begin;
  int end;
};

void init_stream(struct stream_buffer *buffer);
int readto_stream(int fd, struct stream_buffer *str_buffer);
int get_stream(char *dest, struct stream_buffer *buffer);
void insert_n(struct stream_buffer *buffer, char *src, int n);

int main () {
  struct stream_buffer buffer;
  char read[MAX_STR], write[MAX_STR];
  int i, n;

  init_stream(&buffer);

  for (i = 0; i < 3; i++) {
    insert_n(&buffer, MESSAGE, strlen(MESSAGE));
    sprintf(write, "n=%d\n", i);
    insert_n(&buffer, write, strlen(write));
  }

  n = get_stream(read, &buffer);
  printf("%s\n", read);
  n = get_stream(read, &buffer);
  printf("%s\n", read);

  insert_n(&buffer, MESSAGE, strlen(MESSAGE));
  sprintf(write, "n=%d\n", 3);
  insert_n(&buffer, write, strlen(write));

  insert_n(&buffer, MESSAGE, strlen(MESSAGE));

  while ((n = get_stream(read, &buffer)) != -1) {
    printf("%s\n", read);
  }

  insert_n(&buffer, "\n", strlen("\n"));

  while ((n = get_stream(read, &buffer)) != -1) {
    printf("%s\n", read);
  }

  return 0;
}

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
