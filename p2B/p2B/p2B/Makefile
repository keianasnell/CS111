CC=gcc
CFLAGS= -Wextra -Wall -lpthread -lprofiler
SOURCES = lab2_list.c SortedList.c SortedList.h
DATA = lab2b_list.csv
GRAPHS = lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png
GP = lab2_list.gp
PROFILES = profile.out

default: build


build:	lab2_list.c SortedList.h SortedList.c
	$(CC) $(CFLAGS) -o lab2_list lab2_list.c SortedList.c

tests:
	#GRAPH 1
	./lab2_list --threads=1 --iterations=1000 --sync=m > lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2b_list.csv
	#GRAPH 2
	./lab2_list --threads=1 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv
	#GRAPH 3
	- ./lab2_list --threads=1 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=2 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=2 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=2 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=2 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=2 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=2 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=2 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=2 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=2 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=10 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=20 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=40 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=80 --yield=id --lists=4 --sync=m >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=1 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=2 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=2 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=2 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=2 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=4 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=8 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=12 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=10 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=20 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=40 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	- ./lab2_list --threads=16 --iterations=80 --yield=id --lists=4 --sync=s >> lab2b_list.csv
	#GRAPH 4 
	./lab2_list --threads=1 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	#GRAPH 5
	./lab2_list --threads=1 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv



profile:
	LD_PRELOAD=~/lib/libprofiler.so.0 CPUPROFILE=cpu_profile ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text lab2_list cpu_profile > profile.out
	pprof --list=list_update lab2_list cpu_profile >> profile.out


graphs: tests
	./lab2_list.gp


clean:
	rm -f *.tar.gz lab2_list *.o

dist:
	tar -czf lab2b-504804776.tar.gz Makefile README $(GRAPHS) $(DATA) $(SOURCES) $(GP) $(PROFILES)
