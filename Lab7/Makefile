
debug = -g -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement -Wno-return-local-addr -Wuninitialized -pthread

all: primesMT

primesMT: primesMT.o
	gcc $(debug) -o primesMT primesMT.o

primesMT.o: primesMT.c
	gcc $(debug) -c primesMT.c

clean:
	rm -f *.o primesMT *~ /#*
