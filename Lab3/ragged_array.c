#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

// gcc -g -Wall -o ragged_array ragged_array.c
// ./ragged_array < ingredients1.txt

#define LINE_MAX 1024

int main(int argc, char *argv[]) {
    FILE *ifile = stdin;
    char buf[LINE_MAX] = {0};
    int linecount = 0;

    char **ragged_array;
    ragged_array = (char**) malloc(sizeof(char*));

    while (fgets(buf, LINE_MAX, ifile) != NULL) { // Loop 1: Create the array
        ragged_array[linecount] = strdup(buf);
        linecount++;
        ragged_array = realloc(ragged_array, ((linecount+1) * sizeof(char*)));
    }
    ragged_array[linecount] = NULL;

    linecount = 1;
    while (ragged_array[linecount] != NULL) { // Loop 2: Print the array
        printf("%3d: %s", linecount, ragged_array[linecount]);
        linecount++;
    }

    linecount = 0;
    while (ragged_array[linecount] != NULL) { // Loop 3: Free the array
        free(ragged_array[linecount]);
        linecount++;
    }
    free(ragged_array);

    return(EXIT_SUCCESS);
}
