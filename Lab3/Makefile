
debug = -g -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement

all: mywc

mywc: mywc.o
	gcc $(debug) -o mywc mywc.o

mywc.o: mywc.c
	gcc $(debug) -c mywc.c

clean:
	rm -f *.o mywc *~ \#*
