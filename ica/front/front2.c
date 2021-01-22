#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

// gcc -g -Wall -o front2 front2.c
// ./front2 < front2.c
// ./front2 front2.c 7

#define BUFFER_SIZE 1024
#define MAX_LINES 5

int main(int argc, char *argv[]) {
    FILE *ifile = stdin;
    int line_count = 0;
    int max_lines = MAX_LINES;
    char buf[BUFFER_SIZE] = {0};
    char *file_name = NULL;

    if (argc > 1) {
        file_name = argv[1];
        ifile = fopen(file_name, "r");
        if (ifile == NULL) {
            perror("Failed to open file.");
            fprintf(stderr, "  could not open file %s\n", file_name);
            exit(EXIT_FAILURE);
        }
    }

    if (argc > 2) {
        max_lines = atoi(argv[2]);
        if (max_lines <= 0) {
            max_lines = MAX_LINES;
        }
    }

    while ((line_count++ < max_lines) && fgets(buf, BUFFER_SIZE, ifile) != NULL) {
        printf("%s", buf);
}
    if (file_name != NULL) {
    fclose(ifile);
}
    return(EXIT_SUCCESS);
}
