#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define LINE_MAX 1024

void cd(char *);

char **parse(char *, char *);

int ver = 0; // change to 1 for verbose mode
int inp = -1;
int out = -1;

int main(void) {
    char *user = getlogin(); // grab username
    char grab[LINE_MAX] = {0};
    char *quit; // gimmicky signal catcher

    if (user == NULL) { // just in case
        fprintf(stderr, "Zoinks! Unable to find username\n");
        strcpy(user, "mystery_machine");
    }

    while(1) {
        char *token;

        printf("BennySh %s :-) ", user);
        quit = fgets(grab, LINE_MAX, stdin); // grab input
        if (quit == NULL) { // Ctrl+D = EOF
            printf("\nExiting BennySh...\n");
            exit(EXIT_SUCCESS);
        }

        token = strtok(grab, " \n"); // grab command from input
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

        else { // external functions
            pid_t cpid = -1;
            int cstat;
            char **cargv = NULL;
            char cproc[LINE_MAX] = {0};

            cpid = fork();
            switch(cpid) {
            case -1:
                perror("Fork failed");
                exit(EXIT_FAILURE);
            case 0:
            { // child process
                strcpy(cproc, token);
                token = strtok(NULL, "\n");
                cargv = parse(token, cproc);
                if (out != -1) close(out);
                if (inp != -1) close(inp);
                execvp(cargv[0], cargv);
                fprintf(stderr, "Process %s failed to run\n", cproc);
                _exit(3);
            }
            default:
                break;
            }
            while((cpid = wait(&cstat)) > 0) ;
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

char **parse(char *args, char *command) {
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
            strcpy(output, token);
            out = creat(output, S_IRWXU|S_IRWXG);
            if(dup2(out, STDOUT_FILENO) < 0) {
                perror("Failed to duplicate output descriptor");
                _exit(2);
            }
        }
        else if (strcmp(token, "<") == 0) { // redirect input
            token = strtok(NULL, " \0");
            strcpy(input, token);
            inp = open(input, O_RDONLY);
            if(dup2(inp, STDIN_FILENO) < 0) {
                perror("Failed to duplicate input descriptor");
                _exit(1);
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

    if(ver) {
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
    }
    return(argv);
}
