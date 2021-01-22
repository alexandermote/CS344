#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>

// gcc -g -Wall -o whodat whodat.c

#define BUF_SIZE 1024
#define OPTIONS "mu:U:"

void print_user_info(struct passwd *pwd) {
    char filename[BUF_SIZE] = {0};
    char buf[BUF_SIZE] = {0};
    FILE *file = NULL;

    printf("Logname: %s\n\t"
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

    sprintf(filename, "%s/%s", pwd->pw_dir, ".project");
    file = fopen(filename, "r");
    if (file) {
        printf("Project:\n");
        while (fgets(buf, BUF_SIZE, file) != NULL) {
            fputs(buf, stdout);
        }
        fclose(file);
    }
    else {
        printf("Project: N/A\n");
    }

    sprintf(filename, "%s/%s", pwd->pw_dir, ".plan");
    file = fopen(filename, "r");
    if (file) {
        printf("Plan:\n");
        while (fgets(buf, BUF_SIZE, file) != NULL) {
            fputs(buf, stdout);
        }
        fclose(file);
    }
    else {
        printf("Plan: N/A\n");
    }
}

void get_user_by_id(uid_t uid) {
    struct passwd *pwd = NULL;

    pwd = getpwuid(uid);
    if (pwd) {
        print_user_info (pwd);
    }
    else {
        printf("Password entry not found for uid: %d\n", uid);
    }
}

void get_user_by_name(char *logname) {
    struct passwd *pwd = NULL;

    pwd = getpwnam(logname);
    if (pwd) {
        print_user_info (pwd);
    }
    else {
        printf("Password entry not found for logname: %s\n", logname);
    }
}

int main(int argc, char *argv[]) {
    uid_t uid = 0;
    int opt = -1;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch(opt) {
        case 'm':
            get_user_by_id(getuid());
            break;
        case 'u':
            get_user_by_name(optarg);
            break;
        case 'U':
            uid = (uid_t) atol(optarg);
            if (uid > 0) {
                get_user_by_id(uid);
            }
            else {
                printf("Invalid user id: %s\n", optarg);
            }
            break;
        default:
            fprintf(stderr, "%s: [-m] [-u name] [-U uid] \n", argv[0]);
            break;
        }
    }

    return(EXIT_SUCCESS);
}
