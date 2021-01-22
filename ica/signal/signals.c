#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define DELAY 2

void handler_simple (int sig) {
    psignal(sig, "\nCaught handler_simple");

    printf("Caught a %s %d: %s\n", __FUNCTION__, sig, sys_siglist[sig]);
}

void handler_usr (int sig) {
    psignal(sig, "\nCaught handler_usr");

    printf("Caught a %s %d: %s\n", __FUNCTION__, sig, sys_siglist[sig]);
}

void handler_alarm (int sig) {
    static int count = 4;
   
    time_t t = time(NULL);
    printf("The time is %s\n", ctime(&t));
    if (--count > 0) {
        alarm(DELAY);
    }
}

int main (int argc, char *argv[]) {
    struct sigaction new_handler;
    struct sigaction old_handler;

    printf("My PID is: %d\n", getpid());

    new_handler.sa_handler = handler_simple;
    new_handler.sa_flags = SA_RESTART;
    sigfillset(&new_handler.sa_mask);
    sigaction(SIGINT, &new_handler, &old_handler);
    sigaction(SIGTERM, &new_handler, &old_handler);

    new_handler.sa_handler = handler_alarm;
    sigaction(SIGALRM, &new_handler, &old_handler);

    new_handler.sa_handler = handler_usr;
    sigaction(SIGUSR1, &new_handler, &old_handler);
    sigaction(SIGUSR2, &new_handler, &old_handler);

    alarm(DELAY);
    pause();

    kill(getpid(), SIGINT);
    raise(SIGTERM);

    while (pause())
        ;

    return(EXIT_SUCCESS);
}
