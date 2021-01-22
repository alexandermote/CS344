#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define LINE_MAX 1024

int main(int argc, char *argv[]) {
    FILE *ifile = stdin;
    bool c = false;
    bool w = false;
    bool l = false;

    char buf[LINE_MAX] = {0};
    int opt = 0;
    char *filename = NULL;

    while ((opt = getopt(argc, argv, "f:cwlh")) != -1) {
        switch(opt) {
        case ('f'):
            filename = optarg;
            break;
        case ('c'):
            c = true;
            break;
        case ('w'):
            w = true;
            break;
        case ('l'):
            l = true;
            break;
        case ('h'):
            printf("%s\noptions\t: clwf:h\n-c\t: display the number of characters in the input\n-l\t: display the number of lines in the input\n-w\t: display the number of words in the input\n-f file\t: use file as input, defaults to stdin\n-h\t: display command options and exit\n", argv[0]);

            exit(0);
        default:
            exit(EXIT_FAILURE);
        }
    }

    if (!(c || w || l)) {
        c = true;
        w = true;
        l = true;
    }

    if (filename != NULL) {
        ifile = fopen(filename, "r");
        if (ifile == NULL) {
            perror("Unable to open file.");
            exit(2);
        }
    }

    if (l) {
        int linecount = 0;
        while (fgets(buf, LINE_MAX, ifile) != NULL) {
            linecount++;
        }
        printf("%d ", linecount);
        rewind(ifile);
    }
    
    if (w) {
        int wordcount = 0;
        while (fgets(buf, LINE_MAX, ifile) != NULL) {
            char *token = strtok(buf, " ");
            while (token != NULL) {
                wordcount++;
                token = strtok(NULL, " ");
            }
        }
        printf("%d ", wordcount);
        rewind(ifile);
    }

    if (c) {
        int charcount = 0;
        while (fgets(buf, LINE_MAX, ifile) != NULL) {
            charcount += strlen(buf);
        }
        printf("%d ", charcount);
    }

    if (ifile != stdin) {
        printf("%s\n", filename);
        fclose(ifile);
    }
    else {
        printf("\n");
    }
    return(EXIT_SUCCESS);
}
