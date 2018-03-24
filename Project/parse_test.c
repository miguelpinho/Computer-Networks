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

#define MAX_STR 200

int main() {

char buffer[MAX_STR], msg_data[MAX_STR], msg_type[MAX_STR], token_type, ip[MAX_STR];
int id, tpt, id2;

  if (fgets(buffer, MAX_STR, stdin) == NULL) {
    /* Nothing to read. */

    return 1;
  }

  sscanf(buffer, "%s %s", msg_type, msg_data);

  printf("%s\n", msg_type);
    printf("%s\n", msg_data);

  if (strcmp(msg_type, "TOKEN") == 0) {
    sscanf(msg_data, "%d;%c",&id, &token_type);

    printf("%c\n", token_type);
/*TODO*/
    switch (token_type) {
      case 'S': case 'T': case 'I': case 'D':
        break;
      case 'N': case 'O':
        sscanf(msg_data, "%*d;%*c;%d;%[^; ];%d", &id2, ip, &tpt);
        break;
      default:
        printf("Error: Token type not known\n");
        exit(1);
        break;
    }
  } else if (strcmp(msg_type, "NEW")== 0) {
    sscanf(msg_data, "%d;%[^; ];%d", &id, ip, &tpt);

  } else if (strcmp(msg_type, "NEW_START") == 0)   {

  } else {
    printf("Error: Message not known\n");
    exit(1);
  }

  printf("%d %s %d %d \n", id, ip, id2, tpt);
  return 0;
}
