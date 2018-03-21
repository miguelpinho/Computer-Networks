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


int main(int argc, char const *argv[]) {

  if (fgets(buffer, MAX_STR, stdin) == NULL) {
    /* Nothing to read. */

    return 1;
  }

  sscanf(buffer, "%s %s", msg_type, msg_data);

  if (strcmp(msg_type, "TOKEN") == 0) {
    sscanf(msg_data, "%d %d", &id, &token_type);
/*TODO*/
    switch (token_type) {
      case S T I D:
        break;
      case N O:
        break;
      default:
        break;
    }


  } else if (strcmp(msg_type, "TOKEN") {

  } else if (strcmp(msg_type, "TOKEN") {

  } else {

  }
}
