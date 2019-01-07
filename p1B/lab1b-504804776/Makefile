CC = gcc

CFLAGS = -Wall -Wextra -g

TARGET_CLIENT = lab1b-client

TARGET_SERVER = lab1b-server

default:
	$(CC) $(CFLAGS) -lmcrypt -o $(TARGET_CLIENT) $(TARGET_CLIENT).c
	$(CC) $(CFLAGS) -lmcrypt -o $(TARGET_SERVER) $(TARGET_SERVER).c

client:
	$(CC) $(CFLAGS) -lmcrypt -o $(TARGET_CLIENT) $(TARGET_CLIENT).c

server:
	$(CC) $(CFLAGS) -lmcrypt -o $(TARGET_SERVER) $(TARGET_SERVER).c

clean:
	rm -f $(TARGET_CLIENT) $(TARGET_SERVER) *.tar.gz
dist: 
	tar -czf lab1b-504804776.tar.gz $(TARGET_CLIENT).c $(TARGET_SERVER).c Makefile README my.key

