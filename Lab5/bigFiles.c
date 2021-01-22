#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    char *lines = "10";
    int pipe1[2] = {-1, -1};
    int pipe2[2] = {-1, -1};
    int pipe3[2] = {-1, -1};
    pid_t cpid = -1;
    { // evaluate command line arguments
        int opt = 0;
        while ((opt = getopt(argc, argv, "n:h")) != -1) {
            switch(opt) {
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

    pipe(pipe1);
    pipe(pipe2);
    pipe(pipe3);
    cpid = fork();
    switch(cpid) {
    case -1:
        perror("'ls' fork failed");
        exit(EXIT_FAILURE);
    case 0:
    { // child process: ls
        char **argv1 = NULL;
        char *proc1 = NULL;

        if (dup2(pipe1[STDOUT_FILENO], STDOUT_FILENO) < 0) {
            perror("Failed to duplicate output descriptor");
            _exit(2);
        }
        close(pipe1[STDIN_FILENO]);
        close(pipe1[STDOUT_FILENO]);
        close(pipe2[STDIN_FILENO]);
        close(pipe2[STDOUT_FILENO]);
        close(pipe3[STDIN_FILENO]);
        close(pipe3[STDOUT_FILENO]);

        proc1 = strdup("ls");
        argv1 = calloc(2, sizeof(char *));
        argv1[0] = proc1;
        execvp(argv1[0], argv1);
        perror("Child 'ls' failed to run");
        _exit(3);
        break;
    }
    }

    pipe(pipe2);
    cpid = fork();
    switch(cpid) {
    case -1:
        perror("'xargs' fork failed");
        exit(EXIT_FAILURE);
    case 0:
    { // child process: xargs
        char **argv2 = NULL;
        char *proc2 = NULL;

        if (dup2(pipe1[STDIN_FILENO], STDIN_FILENO) < 0) {
            perror("Failed to duplicate input descriptor");
            _exit(1);
        }
        if (dup2(pipe2[STDOUT_FILENO], STDOUT_FILENO) < 0) {
            perror("Failed to duplicate output descriptor");
            _exit(2);
        }
        close(pipe1[STDIN_FILENO]);
        close(pipe1[STDOUT_FILENO]);
        close(pipe2[STDIN_FILENO]);
        close(pipe2[STDOUT_FILENO]);
        close(pipe3[STDIN_FILENO]);
        close(pipe3[STDOUT_FILENO]);

        proc2 = strdup("xargs");
        argv2 = calloc(4, sizeof(char *));
        argv2[0] = proc2;
        argv2[1] = strdup("du");
        argv2[2] = strdup("-s");

        execvp(argv2[0], argv2);
        perror("Child 'xargs' failed to run");
        _exit(3);
        break;
    }
    }

    pipe(pipe3);
    cpid = fork();
    switch(cpid) {
    case -1:
        perror("'sort' fork failed");
        exit(EXIT_FAILURE);
    case 0:
    { // child process: sort
        char **argv3 = NULL;
        char *proc3 = NULL;

        if (dup2(pipe2[STDIN_FILENO], STDIN_FILENO) < 0) {
            perror("Failed to duplicate input descriptor");
            _exit(1);
        }
        if (dup2(pipe3[STDOUT_FILENO], STDOUT_FILENO) < 0) {
            perror("Failed to duplicate output descriptor");
            _exit(2);
        }
        close(pipe1[STDIN_FILENO]);
        close(pipe1[STDOUT_FILENO]);
        close(pipe2[STDIN_FILENO]);
        close(pipe2[STDOUT_FILENO]);
        close(pipe3[STDIN_FILENO]);
        close(pipe3[STDOUT_FILENO]);

        proc3 = strdup("sort");
        argv3 = calloc(3, sizeof(char *));
        argv3[0] = proc3;
        argv3[1] = strdup("-nr");
        execvp(argv3[0], argv3);
        perror("Child 'sort' failed to run");
        _exit(3);
        break;
    }
    }

    { // Parent process: head
    char *par_argv[] = {
        "head"
        , "-n"
        , lines
        , 0x0
    };
    if (dup2(pipe3[STDIN_FILENO], STDIN_FILENO) < 0) {
        perror("Failed to duplicate input descriptor");
        exit(1);
    }

    close(pipe1[STDIN_FILENO]);
    close(pipe1[STDOUT_FILENO]);
    close(pipe2[STDIN_FILENO]);
    close(pipe2[STDOUT_FILENO]);
    close(pipe3[STDIN_FILENO]);
    close(pipe3[STDOUT_FILENO]);
    execvp(par_argv[0], par_argv);
    perror("Parent 'head' failed to run");
    exit(3);
    }

    return(EXIT_SUCCESS);
}
