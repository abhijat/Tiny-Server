CC = gcc
CFLAGS = -g -Wall 

server: server.c file_utils.c
	$(CC) $(CFLAGS) -o server $^
