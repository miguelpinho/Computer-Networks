# Compiler flags
CC = gcc
CFLAGS = -g -Wall -pedantic -ansi

# Sources
SRC_service = service.c
SRC_reqserv = reqserv.c

# Objects
OBJ_service = service.o
OBJ_reqserv = reqserv.o

all: service reqserv

service: $(OBJ_service)
	$(CC) $(CFLAGS) -o $@ $(OBJ_service)

reqserv: $(OBJ_reqserv)
	$(CC) $(CFLAGS) -o $@ $(OBJ_reqserv)

service.o: $(SRC_service)

reqserv.o: $(SRC_reqserv)

clean:
	rm -f *.o *.~ service reqserv *.gch

#	depend::
#		makedepend $(SOURCES)
