CC = gcc
CFLAGS = -Wextra -Wall -lmraa -lm

default:
	$(CC) $(CFLAGS) -o lab4c_tcp lab4c_tcp.c
	$(CC) $(CFLAGS) -lssl -lcrypto -o lab4c_tls lab4c_tls.c

clean:
	rm -f *.tar.gz *.o lab4c_tcp lab4c_tls

dist:
	tar -czf lab4c-504804776.tar.gz Makefile README lab4c_tcp.c lab4c_tls.c
