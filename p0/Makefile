#gcc compiler
CC = gcc

#Compiler flags
CFLAGS = -Wall -Wextra

TARGET = lab0

default:
	$(CC) $(CFLAGS) -o (TARGET) $(TARGET).c

check: default testReadWrite testInvalidOptions testInvalidInput testInvalidOutput testSegfaultCatch
#all different smoke tests

#testReadWrite:
#should be linux commands to check these
# Please include a brief description (in your README) of what checks you chose to include in your smoke-test.

clean:
	rm $(TARGET)

dist:
	tar -czf lab0-504804776.tar.gz lab0.c Makefile backtrace.png breakpoint.png README.md
