#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static char *song_lines[] = {
    "Whether you're a brother or whether you're a mother"
    , "You're stayin' alive, stayin' alive"
    , "Feel the city breakin' and everybody shakin'"
    , "And we're stayin' alive, stayin' alive"
    , "Ha, ha, ha, ha, stayin' alive, stayin' alive"
    , "Ha, ha, ha, ha, stayin' aliiiiiiiiiive"
    , NULL };

void stayin_alive(int sig) {
    static int song_line = 0;
    fprintf(stderr, " %s\n", song_lines[song_line++]);
    if (song_lines[song_line] == NULL) {
        fprintf(stderr, "\n");
        song_line = 0;
    }
}

int main (int argc, char *argv[]) {
    void *sig_ret;

    sig_ret = signal(SIGINT, stayin_alive);
    if (sig_ret == SIG_ERR) {
        perror("Failed to set signal handler");
        exit(EXIT_FAILURE);
    }

    while(pause())
        ;
   
    return(EXIT_SUCCESS);
}
