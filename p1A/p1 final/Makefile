CC = gcc

CFLAGS = -Wall -Wextra -g

TARGET = lab1a

default:
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	rm -rf lab1a *.tar.gz

dist:
	tar -czf lab1a-504804776.tar.gz lab1a.c Makefile README
