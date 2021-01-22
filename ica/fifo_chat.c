#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <linux/limits.h>
#include <stdlib.h>

// gcc -g -Wall -o fifo_chat fifo_chat.c

#define FIFO_FILENAME "ChatFifo__"
#define FIFO_PERMISSIONS (S_IRUSR | S_IWUSR)
#define FIFO_NAME(_BUF_, _suffix) sprintf((_BUF_),"%s/%s_%s",getenv("HOME"),FIFO_FILENAME,_suffix)

#define PROMPT ">>> "
#define LISTENING "listening... \n"
#define REPLY "reply>>>"
#define BUFFER_SIZE 200

#ifndef NULL
# define NULL 0
#endif // NULL

#ifndef FALSE
# define FALSE 0
#endif // FALSE

#ifndef TRUE
# define  TRUE 1
#endif // TRUE

static int isWriter = TRUE;
static int isVerbose = FALSE;

static char fifoNameW[PATH_MAX] = { '\0' };
static char fifoNameR[PATH_MAX] = { '\0' };

extern void sigint_handler(int sig) {
    fprintf(stderr,  "++ signal handler called for %d: %d\n", sig, getpid());
    exit(EXIT_SUCCESS);
}

extern void remove_pipes( void ) {
    fprintf(stderr, "++ exit handler called: %d\n", getpid());
    unlink(fifoNameW);
    unlink(fifoNameR);
}

extern void writer( void ) {
    int fifoFdW = -1;
    int fifoFdR = -1;
    ssize_t result= 0;
    char buffer[BUFFER_SIZE] = {0};
    char *rez = NULL;

    FIFO_NAME(fifoNameW, "w");
    FIFO_NAME(fifoNameR, "r");

    if (isVerbose == TRUE) {
        fprintf(stderr, "verbose %d: FIFO names: \twriter FIFO: %s\n\t reader FIFO: %s\n"
                , __LINE__, fifoNameW, fifoNameR);
    }

    mkfifo(fifoNameW, FIFO_PERMISSIONS);
    mkfifo(fifoNameR, FIFO_PERMISSIONS);

    fifoFdW = open(fifoNameW, O_WRONLY);
    fifoFdR = open(fifoNameR, O_RDONLY);

    while (TRUE) {
        fputs(PROMPT, stdout);
        rez = fgets(buffer, sizeof(buffer), stdin);
        if (rez == NULL) {
            break;
        }
        result = write(fifoFdW, buffer, strlen(buffer));
        memset(buffer, 0, sizeof(buffer));

        fputs(LISTENING, stdout);
        result = read(fifoFdR, buffer, sizeof(buffer));
        if (result == 0) {
            break;
        }
        fputs(REPLY, stdout);
        fputs(buffer, stdout);
    }
    close(fifoFdW);
    close(fifoFdR);
}

extern void reader( void ) {
    int fifoFdW = -1;
    int fifoFdR = -1;
    ssize_t result = 0;
    char buffer[BUFFER_SIZE] = {0};

    FIFO_NAME(fifoNameW, "w");
    FIFO_NAME(fifoNameR, "r");
    if (isVerbose == TRUE) {
        fprintf(stderr, "verbose %d: FIFO names: \twriter FIFO: %s\n\t reader F\
IFO: %s\n"
                , __LINE__, fifoNameW, fifoNameR);
    }
    fifoFdW = open(fifoNameW, O_RDONLY);
    fifoFdR = open(fifoNameR, O_WRONLY);

    while(TRUE) {
        char *r;
        fputs(LISTENING, stdout);
        result = read(fifoFdW, buffer, sizeof(buffer));
        if (result == 0) {
            break;
        }
        buffer[result] = 0;
        fputs(REPLY, stdout);
        fputs(buffer, stdout);
        fputs(PROMPT, stdout);
        r = fgets(buffer, sizeof(buffer), stdin);
        if (r == NULL) {
            break;
        }
        result = write(fifoFdR, buffer, strlen(buffer));
    }
}

int main(int argc, char *argv[]) {
    int opt;

    umask(0);
    if(argc < 2) {
        printf("Must give command line option -w or -r\n");
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "wvr")) != -1) {
        switch(opt) {
        case 'v':
            isVerbose = TRUE;
            break;
        case 'w':
            isWriter = TRUE;
            break;
        case 'r':
            isWriter = FALSE;
            break;

        default:
            fprintf(stderr
                    , "*** opt:%c ptarg: <%s> optind: %d opterr: %d optopt: %d ***\n"
                    , opt, optarg, optind, opterr, optopt);
            break;
        }
    }

    if (isWriter == TRUE) {
        if (isVerbose == TRUE) {
            fprintf(stderr, "verbose %d: Writer starting\n", __LINE__);
        }
        atexit(remove_pipes);
        signal(SIGINT, sigint_handler);
        writer();
    }
    else {
        if (isVerbose == TRUE) {
            fprintf(stderr, "verbose %d: Reader starting\n", __LINE__);
        }
        reader();
    }

    return (EXIT_SUCCESS);
}
