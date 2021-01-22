#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

// gcc -g -Wall -o front4 front4.c
// ./front4 < front4.c
// ./front4 front4.c
// ./front4 front4.c -n 7
// ./front4 -n 5 front3.c front4.c front1.c

#define BUFFER_SIZE 1024
#define MAX_LINES 5

int main(int argc, char *argv[]) {
    FILE *ifile = stdin;
    int line_count = 0;
    int max_lines = MAX_LINES;
    char buf[BUFFER_SIZE] = {0};
    char *file_name = NULL;
    int opt = 0;

    while ((opt = getopt(argc, argv, "f:n:h")) != -1) {
        switch(opt) {
        case ('n'):
            max_lines = atoi(optarg);
            if (max_lines <= 0) {
                max_lines = MAX_LINES;
            }
            break;
        case ('h'):
            printf("%s [-n #] [-h]\n", argv[0]);
            exit(EXIT_SUCCESS);
        default:
            exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        int j;
        for (j = optind; j < argc; j++) {
            file_name = argv[j];
            ifile = fopen(file_name, "r");
            if (ifile == NULL) {
                perror("Failed to open file.");
                fprintf(stderr, "  could not open file %s\n", file_name);
            }
            else {
                line_count = 0;
                printf("==> %s <==\n", file_name);
                while ((line_count++ < max_lines) && fgets(buf, BUFFER_SIZE, ifile) != NULL) {
                    printf("%s", buf);
                }
                printf("\n");
                fclose(ifile);
            }
        }
    }
    else {
        while ((line_count++ < max_lines) && fgets(buf, BUFFER_SIZE, ifile) != NULL) {
            printf("%s", buf);
        }

        if (file_name != NULL) {
            fclose(ifile);
        }
    }
    return(EXIT_SUCCESS);
}
