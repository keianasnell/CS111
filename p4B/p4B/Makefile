CC = gcc
CFLAGS = -Wall -Wextra -lmraa -lm

default:
	$(CC) $(CFLAGS) -o lab4b lab4b.c

clean:
	rm -f *.tar.gz *.o

check: default
	echo "SCALE=C" >> test; \
	echo "PERIOD=2" >> test; \
	echo "START" >> test; \
	echo "STOP" >> test; \
	echo "OFF" >> test; \
	(cat test | ./lab4b --log=log.txt && echo "make check test PASSED") || (echo "make check test FAILED. Returned $$?" && exit 1); \
	rm -f log.txt test


dist:		
	tar -czf lab4b-504804776.tar.gz Makefile README lab4b.c
