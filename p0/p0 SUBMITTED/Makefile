#gcc compiler
CC = gcc

#Compiler flags
CFLAGS = -Wall -Wextra -g


default:
	$(CC) $(CFLAGS) -o lab0 lab0.c

check: default testInputOutput testSegFault testCatch

testInputOutput:
	echo "testing input & output" > input.txt 
	./lab0 --input=input.txt --output=output.txt  
	if [ $$? -eq 0 ]; then \
	echo "Checking for --input --output ret 0 SUCCESS"; \
	else \
	echo "Checking for --input --output ret 0 FAILURE"; \
	fi; \
	cmp -s input.txt output.txt; \
	if [ $$? -eq 0 ]; then \
	echo "Checking for correct output SUCCESS"; \
	else \
	echo "Checking for correct output FAILURE"; \
	fi; \
	rm -f test.txt output.txt

testSegFault:
	echo "testing segfault" > in.txt; \
	touch out.txt; \
	./lab0 --input=in.txt --output=out.txt --segfault; \
	if [[ $$? -eq 0 ]]; then \
	touch error; \
	fi; \
	rm -f in.txt out.txt

testCatch:
	echo "testing catch of segfault" > in.txt; \
	touch out.txt; \
	./lab0 --input=in.txt --output=out.txt --segfault --catch; \
        if [[ $$? -ne 4 ]]; then \
        touch error; \
        fi; \
        rm -f in.txt out.txt


clean:
	rm -rf lab0 *.tar.gz *.txt

dist:
	tar -czf lab0-504804776.tar.gz lab0.c Makefile backtrace.png breakpoint.png README
