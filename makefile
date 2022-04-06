ring: main.o node.o calls.o tcp.o udp.o
	gcc -Wall -std=gnu99 -g -o ring main.o node.o calls.o tcp.o udp.o
main.o: main.c node.h calls.h tcp.h udp.h 
	gcc -Wall -std=gnu99 -g -c main.c
node.o: node.c node.h calls.h tcp.h udp.h 
	gcc -Wall -std=gnu99 -g -c node.c
calls.o: calls.c calls.h tcp.h udp.h 
	gcc -Wall -std=gnu99 -g -c calls.c
tcp.o: tcp.c tcp.h
	gcc -Wall -std=gnu99 -g -c tcp.c
udp.o: udp.c udp.h
	gcc -Wall -std=gnu99 -g -c udp.c
clean:
	rm *.o