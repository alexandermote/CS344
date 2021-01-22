#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>

// gcc -g -Wall -pthread -o whodat_server_MT_fork whodat_server_MT_fork.c

#define BUF_SIZE 1024
#define LISTENQ 20
#define SERV_PORT 10001
#define MAX_PROCS 3
#define OPTIONS "hp:"

#ifndef TRUE
# define TRUE 1
#endif //TRUE

#ifndef FALSE
# define FALSE 0
#endif //FALSE

int listen_fd = -1;
int conn_count = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
int is_parent = TRUE;
int accept_count = 0;
pid_t cpids[MAX_PROCS] = {0};

void print_user_info(int, struct passwd *);
void get_user_by_name(int, char *);
void sigint_handler(int, siginfo_t *, void *);
void *thread_get_user_by_name(void *);
void sigchld_handler(int, siginfo_t *, void *);
void shutdown_children(void);

void print_user_info(int sock, struct passwd *pwd) {
// most of this function is taken directly from whodat in-class assnt
// really the "write" functions are the only new thing
    char filename[BUF_SIZE] = {0};
    char buf[BUF_SIZE] = {0};
    FILE *file = NULL;

    sprintf(buf, "Logname: %s\n\t"
           "Name: %s\n\t"
           "uid: %d\n\t"
           "gid: %d\n\t"
           "Home: %s\n\t"
           "Shell: %s\n"
           , pwd->pw_name
           , pwd->pw_gecos
           , pwd->pw_uid
           , pwd->pw_gid
           , pwd->pw_dir
           , pwd->pw_shell);

    write(sock, buf, strlen(buf));
    sprintf(filename, "%s/%s", pwd->pw_dir, ".project");
    file = fopen(filename, "r");
    if (file) {
        write(sock, "Project:\n", strlen("Project:\n"));
        while (fgets(buf, BUF_SIZE, file) != NULL) {
            write(sock, buf, strlen(buf));
        }
        fclose(file);
    }
    else {
        write(sock, "Project: N/A\n", strlen("Project: N/A\n"));
    }

    sprintf(filename, "%s/%s", pwd->pw_dir, ".plan");
    file = fopen(filename, "r");
    if (file) {
        write(sock, "Plan:\n", strlen("Plan:\n"));
        while (fgets(buf, BUF_SIZE, file) != NULL) {
            write(sock, buf, sizeof(buf));
        }
        fclose(file);
    }
    else {
        write(sock, "Plan: N/A\n", strlen("Plan: N/A\n"));
    }
    write(sock, "\n", strlen("\n"));
}

void sigint_handler(int sig, siginfo_t *sig_info, void *context) {
    fprintf(stderr, "\n\n%d: Shutting down; conns: %d accept: %d\n"
            , getpid()
            , conn_count
            , accept_count);
    close(listen_fd);
    exit(EXIT_SUCCESS);
}

void sigchld_handler(int sig, siginfo_t *siginfo, void *context) {
    int status;
    pid_t cpid;

    while ((cpid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            fprintf(stderr, "Normal child exit %d: pid: %d  exit value: %d\n"
                    , sig, cpid, WEXITSTATUS(status));
        }
        else {
            fprintf(stderr, "Signal child exit: %dpid: %d exit value: %d\n"
                    , sig, cpid, WEXITSTATUS(status));
        }
    }
}

void get_user_by_name(int sock, char *logname) {
    struct passwd pwd;
    struct passwd *result;
    char buf[BUF_SIZE] = {0};

    getpwnam_r(logname, &pwd, buf, BUF_SIZE, &result);
    if (result != 0) {
        print_user_info(sock, &pwd);
    }
    else {
        char no_find[BUF_SIZE] = {0};
        sprintf(no_find, "Password entry not found for logname: %s\n", logname);
        write(sock, no_find, strlen(no_find));
    }
}

void *thread_get_user_by_name(void *sock) {
    int sock_fd = (int) ((long) sock);
    int i = 0;
    char buf[BUF_SIZE] = {0};

    pthread_detach(pthread_self());

    memset(buf, 0, sizeof(buf));
    if ((i = read(sock_fd, buf, sizeof(buf))) == 0) {
        close(sock_fd);
    }
    else {
        // client will ask for user info                                         
        fprintf(stderr, "\trequest from client for user %s\n", buf);
        get_user_by_name(sock_fd, buf);
        close(sock_fd);
        pthread_mutex_lock(&count_mutex);
        conn_count++;
        pthread_mutex_unlock(&count_mutex);
    }

    pthread_exit(EXIT_SUCCESS);
}

void shutdown_children(void) {
    int i = 0;
    if(is_parent){
        for (i = 0; i < MAX_PROCS; i++) {
            kill(cpids[i], SIGINT);
        }
    }
}

int main(int argc, char *argv[]) {
    int sock_fd;
    int port = SERV_PORT;
    int i;
    // char buf[BUF_SIZE] = {0};
    socklen_t cli_len;
    struct sockaddr_in cli_addr;
    struct sockaddr_in serv_addr;
    pthread_t thread;

    { // getopts
        int opt = -1;
        while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
            switch(opt) {
            case 'h':
                printf("%s\n  "
                       "-p #: change port number (defaults to 10,001)\n  "
                       "  -h: display this help menu\n"
                       , argv[0]);
                exit(EXIT_SUCCESS);
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Unknown command line argument; Exiting...\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    { // signal handler setup
        struct sigaction new_sig;
        struct sigaction old_sig;

        new_sig.sa_sigaction = sigint_handler;
        sigfillset(&new_sig.sa_mask);
        new_sig.sa_flags = SA_SIGINFO | SA_RESTART;
        sigaction(SIGINT, &new_sig, &old_sig);

        new_sig.sa_sigaction = sigchld_handler;
        sigfillset(&new_sig.sa_mask);
        new_sig.sa_flags = SA_SIGINFO | SA_RESTART;
        sigaction(SIGCHLD, &new_sig, &old_sig);
    }
    atexit(shutdown_children);

    // make and bind socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("Socket initialization failed");
        exit(2);
    }

    { // configure socket for sharing
        int optval = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    if (bind(listen_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
        perror("Socket bind failed");
        exit(3);
    }

    // listen for client
    listen(listen_fd, LISTENQ);
    fprintf(stderr, "Server listening on port %d\n", port);
    cli_len = sizeof(cli_addr);
    for (i = 0; i < MAX_PROCS; i++) {
        int status = -1;
        cpids[i] = fork();
        if (cpids[i] == 0) {
            is_parent = FALSE;
            while(1) {
                sock_fd = accept(listen_fd, (struct sockaddr *) &cli_addr, &cli_len);
                fprintf(stderr, "Connection received\n");
                accept_count++;
                status = pthread_create(&thread, NULL, thread_get_user_by_name, (void *) ((long) sock_fd));
                if (status != 0) {
                    fprintf(stderr, "%d: thread creation failed %d\n", getpid(), cpids[i]);
                }
            }
        }
        fprintf(stderr, "%d: started child %d\n", getpid(), cpids[i]);
    }
    while (1) {
        pause();
    }
    return(EXIT_SUCCESS);
}
