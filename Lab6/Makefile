
debug = -g -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement -Wno-return-local-addr -Wuninitialized

all: BennySh

BennySh: BennySh.o
	gcc $(debug) -o BennySh BennySh.o

BennySh.o: BennySh.c
	gcc $(debug) -c BennySh.c

clean:
	rm -f *.o BennySh *~ /#*
