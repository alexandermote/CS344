#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>

// gcc -g -Wall -o zhead2 zhead2.c
// rm -f ztail2; ln zhead2 ztail2

// last > JUNK
// gzip JUNK
// ./zhead2 JUNK.gz

int main(int argc, char *argv[]) {
    char *lines = NULL;
    char *filename = NULL;

    {
        int c = 0;
        while (( c = getopt(argc, argv, "n:h")) != -1) {
            switch(c) {
            case 'n':
                lines = optarg;
                break;
            case 'h':
                printf("%s [-n #] [-h]\n", argv[0]);
                exit(EXIT_SUCCESS);
            default:
                break;
            }
        }
    }

    if (optind < argc) {
        filename = argv[optind];
    }
    else {
        perror("No file given");
        exit(EXIT_FAILURE);
    }

    {
        int pipes[2] = {-1, -1};
        pid_t cpid = -1;
        int status = -1;

        pipe(pipes);
        cpid = fork();
        switch(cpid) {
        case -1:
            perror("Fork failed");
            exit(6);
        case 0:
        {
            // child process                                                    
            char **rhp_argv = NULL;
            char *rhp = NULL;

            if (dup2(pipes[STDIN_FILENO], STDIN_FILENO) < 0) {
                perror("Failed to duplicate input descriptor");
                _exit(2);
            }
            close(pipes[STDIN_FILENO]);
            close(pipes[STDOUT_FILENO]);

            rhp = basename(argv[0]);
            if (strcmp(rhp, "ztail") == 0) {
                rhp = strdup("tail");
            }
            else {
                rhp = strdup("head");
            }
            if (lines == NULL) {
                rhp_argv = calloc(2, sizeof(char *));
                rhp_argv[0] = rhp;
            }
            else {
                rhp_argv = calloc(4, sizeof(char *));
                rhp_argv[0] = rhp;
                rhp_argv[1] = strdup("-n");
                rhp_argv[2] = strdup(lines);
            }
            execvp(rhp_argv[0], rhp_argv);
            perror("Child failed to run");
            _exit(3);
            break;
        }
        default:
            // parent process; see below
            break;
        }
        cpid = fork();
        switch(cpid) {
        case -1:
            perror("Fork failed");
            exit(7);
            break;
        case 0:
        {
            // new child process
            char *lhp_argv[] = {
                "gunzip"
                , "-c"
                ,  filename
                , 0x0
            };
            if (dup2(pipes[STDOUT_FILENO], STDOUT_FILENO) < 0) {
                perror("Failed to duplicate output descriptor");
                exit(4);
            }
            close(pipes[STDIN_FILENO]);
            close(pipes[STDOUT_FILENO]);
            execvp(lhp_argv[0], lhp_argv);
            perror("Parent failed to run");
            _exit(5);
            break;
        }
        default:
            close(pipes[STDIN_FILENO]);
            close(pipes[STDOUT_FILENO]);
            break;
        }
        while ((cpid = wait(&status)) > 0) {
            fprintf(stderr, "parent: child %d exited\n", cpid);
            if (WIFEXITED(status)) {
                fprintf(stderr, "\twith exit status %d\n", WEXITSTATUS(status));
            }
        }
        fprintf(stderr, "parent: all child processes done\n");
    }

    return(EXIT_SUCCESS);
}

