
debug = -g -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement -Wno-return-local-addr -Wuninitialized -Wunused -Wextra

all: reclookup bigFiles BennySh

reclookup: reclookup.o
	gcc $(debug) -o reclookup reclookup.o

reclookup.o: reclookup.c file_struct.h
	gcc $(debug) -c reclookup.c

bigFiles: bigFiles.o
	gcc $(debug) -o bigFiles bigFiles.o

bigFiles.o: bigFiles.c
	gcc $(debug) -c bigFiles.c

BennySh: BennySh.o
	gcc $(debug) -o BennySh BennySh.o

BennySh.o: BennySh.c
	gcc $(debug) -c BennySh.c

clean:
	rm -f *.o reclookup bigFiles BennySh *~ /#*
