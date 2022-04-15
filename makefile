OBJS	= main.o node.o tcp.o udp.o calls.o
SOURCE	= main.c node.c tcp.c udp.c calls.c
HEADER	= node.h tcp.h udp.h calls.h
OUT	= ring
CC	 = gcc
FLAGS	 = -g3 -c -Wall

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

main.o: main.c
	$(CC) $(FLAGS) main.c -std=gnu99

node.o: node.c
	$(CC) $(FLAGS) node.c -std=gnu99

tcp.o: tcp.c
	$(CC) $(FLAGS) tcp.c -std=gnu99

udp.o: udp.c
	$(CC) $(FLAGS) udp.c -std=gnu99

calls.o: calls.c
	$(CC) $(FLAGS) calls.c -std=gnu99


clean:
	rm -f $(OBJS) $(OUT)

debug: $(OUT)
	valgrind $(OUT)

valgrind: $(OUT)
	valgrind $(OUT)

valgrind_leakcheck: $(OUT)
	valgrind --leak-check=full $(OUT)
