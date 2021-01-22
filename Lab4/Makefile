
debug = -g -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement -Wno-return-local-addr -Wuninitialized -Wunused -Wextra

all: mystat csv2bin bin2csv

mystat: mystat.o
	gcc $(debug) -o mystat mystat.o

mystat.o: mystat.c
	gcc  $(debug) -c mystat.c

csv2bin: csv2bin.o
	gcc $(debug) -o csv2bin csv2bin.o

csv2bin.o: csv2bin.c file_struct.h
	gcc $(debug) -c csv2bin.c

bin2csv: bin2csv.o
	gcc $(debug) -o bin2csv bin2csv.o

bin2csv.o: bin2csv.c file_struct.h
	gcc $(debug) -c bin2csv.c

clean:
	rm -f *.o mystat csv2bin bin2csv *~ \#*
