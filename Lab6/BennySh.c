#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define LINE_MAX 1024

// if you pass more than 20 commands
// into the command line at once,
// please know that there is help out there
#define COMMAND_MAX 20

void cd(char *);
char **parse_simple(char *, char *);

int ver = 1;  // change to >0 for verbose mode
int inp = -1; // input file descriptor
int out = -1; // output file descriptor
int count = 0;      // counts functions while executing
int func_count = 0; // counts functions while collecting

int main(void) {
    char *user = getlogin(); // grab username
    char grab[LINE_MAX] = {0};
    char com_line[LINE_MAX] = {0};
    char *quit; // gimmicky signal catcher

    if (user == NULL) { // just in case
        fprintf(stderr, "Zoinks! Unable to find username\n");
        strcpy(user, "mystery_machine");
    }

    while(1) {
        char *token;
        int scan;
        int place = 0;

        printf("BennySh %s :-) ", user);
        quit = fgets(grab, LINE_MAX, stdin); // grab input
        if (quit == NULL) { // Ctrl+D = EOF
            printf("\nExiting BennySh...\n");
            exit(EXIT_SUCCESS);
        }

        // WARNING: Look away. Abandon all hope ye who enter here.
        for (scan = 0; scan < strlen(grab); scan++) {
            if (grab[scan] == '|') {   // weird trick to let the
                com_line[place] = ' '; // strtok stuff I use
                place++;               // later in the program
                com_line[place] = '|'; // work properly in edge cases
                place++;               // this is top-shelf jank.
            }
            else {
                com_line[place] = grab[scan];
                place++;
            }
        }
        com_line[place] = '\0';
        // It is now safe to resume browsing this code.

        token = strtok(com_line, " \n"); // grab command from input
        if (strcmp(token, "exit") == 0) {
            printf("Exiting BennySh...\n");
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(token, "cd") == 0) {
            char dir[LINE_MAX] = {0};
            token = strtok(NULL, "\n"); // grab path
            if (token == NULL) chdir(getenv("HOME")); // default to home
            else cd(token);
            getcwd(dir, LINE_MAX);
            printf("%s\n", dir); // tell them where they are
        }
        else if (strcmp(token, "echo") == 0) {
            token = strtok(NULL, "\n");
            printf("'%s'\n", token);
        }
        else if (strcmp(token, "pwd") == 0) {
            char dir[LINE_MAX] = {0};
            getcwd(dir, LINE_MAX);
            printf("%s\n", dir);
        }
        else { // external commands
            char *func_array[COMMAND_MAX]; // build a parser they said
            char *arg_array[COMMAND_MAX];  // it will be fun they said

            int p_trail = -1;  // pipe trail
            int p_output = -1; // pipe output
            int p_input = -1;  // pipe input

            int cpid = -1; // used for execs
            int cstat;     // used at end of execs

            while (token != NULL) { // find all commands
                func_array[func_count] = malloc(sizeof(char *));
                strcpy(func_array[func_count], token); // save commands

                token = strtok(NULL, "|\n");
                arg_array[func_count] = malloc(sizeof(char *));
                if (token == NULL) arg_array[func_count] = NULL;
                else strcpy(arg_array[func_count], token); // save arguments

                func_count++;
                token = strtok(NULL, " \n");
            }
            func_array[func_count] = 0x0;
            arg_array[func_count] = 0x0;
            if(ver) { // double-check commands
                int vercount;
                fprintf(stderr, "Commands found:\n");
                for (vercount = 0; vercount < func_count; vercount++) {
                    fprintf(stderr, "  %s:\n    %s\n"
                            , func_array[vercount]
                            , arg_array[vercount]);
                }
            }
            for (count = 0; count < func_count; count++) {
                // loop through list of commands
                int pip[2] = {-1, -1};

                if ((count + 1) != func_count) {  // if ! last
                    pipe(pip);
                    p_output = pip[STDOUT_FILENO];
                    p_input = pip[STDIN_FILENO];

                    if(ver) {
                        fprintf(stderr, "Pipe created:\n"
                                "\tInput Descriptor: %d\n\tOutput Descriptor: %d\n"
                                , p_input
                                , p_output);
                    }
                }
                cpid = fork();
                switch(cpid) {
                case -1:
                    perror("Fork failed");
                    exit(EXIT_FAILURE);
                case 0:
                { // child process
                    char cproc[LINE_MAX] = {0};
                    char **cargv = NULL;

                    if (count != 0) {  // if ! first
                        if (dup2(p_trail, STDIN_FILENO) < 0) {
                            perror("Failed to duplicate input descriptor");
                            _exit(1);
                        }
                    }
                    if ((count + 1) != func_count) { // if ! last
                        if (dup2(p_output, STDOUT_FILENO) < 0) {
                            perror("Failed to duplicate output descriptor");
                            _exit(2);
                        }
                        close(p_input);
                        close(p_output);
                    }
                    
                    // exec with ragged array
                    strcpy(cproc, func_array[count]);
                    if (arg_array[count] == NULL) {
                        execlp(cproc, cproc, (char *) NULL);
                        fprintf(stderr, "Process %s failed to run\n", cproc);
                        _exit(3);
                    }
                    else {
                        cargv = parse_simple(cproc, arg_array[count]);
                        if (out != -1) close(out);
                        if (inp != -1) close(inp);
                        execvp(cargv[0], cargv);
                        fprintf(stderr, "Process %s failed to run\n", cproc);
                        _exit(3);
                    }
                }
                default: // parent
                    if (count != 0) { // if ! first
                        close(p_trail);
                    }
                    if ((count + 1) != func_count) { // if ! last
                        close(p_output);
                        p_trail = p_input;
                    }
                    break;
                }
            }
            while((cpid = wait(&cstat)) > 0) ;
            { // free func and arg arrays
                int freecount;
                for (freecount = 0; freecount < func_count; freecount++) {
                    if (func_array[freecount] != NULL) free(func_array[freecount]);
                    if (arg_array[freecount] != NULL) free(arg_array[freecount]);
                    count++;
                }
                // reset counters
                count = 0;
                func_count = 0;
            }
        }
    }
    return(EXIT_FAILURE); // program shouldn't get here
}

void cd(char *path) {
    char dir[LINE_MAX] = {0};
    char *token = strtok(path, "~\0");
    int newdir;

// if the user inputs a tilde (currently not working)
    if (token == NULL) {
        strcpy(dir, "/nfs/stak/users/");
        token = strtok(NULL, "/\0");
        if(token == NULL) {
            char *user = getlogin();
            strcat(dir, user);
        }
        else while(token != NULL) {
                strcat(dir, token);
                strcat(dir, "/");
                token = strtok(NULL, "/\0");
            };
    }

// if the user doesn't input a tilde
    else {
    getcwd(dir, LINE_MAX);
    strcat(dir, "/");
    strcat(dir, token);
    }
    
// change directory
    newdir = chdir(dir);
    if(newdir == -1) {
        perror("Failed to change directory");
    }
}

char **parse_simple(char *command, char *args) {
    char **argv;
    char proc[LINE_MAX] = {0};
    char *token;
    int argcount = 2;
    char input[LINE_MAX] = {0};
    char output[LINE_MAX] = {0};

    strcpy(proc, command);
    argv = malloc(argcount * sizeof(char *));
    argv[0] = proc;
    token = strtok(args, " \0");
    while (token != NULL) { // interpret arguments
        if (strcmp(token, ">") == 0) { // redirect output
            token = strtok(NULL, " \0");
            if (count + 1 == func_count) {
                strcpy(output, token);
                out = creat(output, S_IRWXU|S_IRWXG);
                if(dup2(out, STDOUT_FILENO) < 0) {
                    perror("Failed to duplicate output descriptor");
                    _exit(2);
                }
            }
        }
        else if (strcmp(token, "<") == 0) { // redirect input
            token = strtok(NULL, " \0");
            if (count == 0) {
                strcpy(input, token);
                inp = open(input, O_RDONLY);
                if(dup2(inp, STDIN_FILENO) < 0) {
                    perror("Failed to duplicate input descriptor");
                    _exit(1);
                }
            }
        }
        else {
            argcount++;
            argv = realloc(argv, argcount * sizeof(char *));
            argv[(argcount - 2)] = token;
        }
        token = strtok(NULL, " \0");
    }
    argv[(argcount - 1)] = 0x0;

    if(ver) { // check ragged array
        int check;
        for (check = 0; check < argcount; check++) {
            fprintf(stderr, "argv[%d] = +%s+\n"
                    , check
                    , argv[check]);
        }
        if (inp != -1) {
            fprintf(stderr, "input: %s\n", input);
        }
        if (out != -1) {
            fprintf(stderr, "output: %s\n", output);
        }
        fprintf(stderr, "\n");
    }
    return(argv);
}
