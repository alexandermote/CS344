#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

// gcc -g -Wall -o dir_watcher dir_watcher.c

#define MAX_EVENTS 1024
#define NAME_LEN 40
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (MAX_EVENTS * (EVENT_SIZE + NAME_LEN))

static int fd = -1;
static int wd = -1;

static void sigint_handler(int);
static void remove_notify(void);

int main(int argc, char *argv[]) {
    int length;
    int event_index = 0;
    int event_count = 0;
    unsigned int total_events = 0;
    unsigned int create_events = 0;
    char *dir_to_monitor = NULL;
    char buf[BUF_LEN] = {0};

    if (argc < 2) {
        dir_to_monitor = ".";
    }
    else {
        dir_to_monitor =  argv[1];
    }

    (void) atexit(remove_notify);
    (void) signal(SIGINT, sigint_handler);

    fd = inotify_init();
    if (fd < 0) {
        perror("inotify failed to initialize");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Adding inotify for '%s'\n", dir_to_monitor);
    wd = inotify_add_watch(fd, dir_to_monitor
                           , IN_CREATE 
                           | IN_MODIFY
                           | IN_DELETE
                           | IN_ATTRIB
                           | IN_CLOSE_WRITE
                           | IN_CLOSE_NOWRITE
                           | IN_MOVED_FROM
                           | IN_MOVED_TO);
    if (wd == -1) {
        perror("Add watch failed");
        exit(EXIT_FAILURE);
    }

    while(1) {
        length = read(fd, buf, BUF_LEN);
        event_count = 0;
        total_events++;
        if (length < 0) {
            perror("Read error on event notifier fd");
            exit(EXIT_FAILURE);
        }
        if (length == 0) {
            printf("Zero length event list read\n");
            continue;
        }
        event_index = 0;
        while (event_index < length) {
            struct inotify_event *event = (struct inotify_event *) &buf[event_index];

            event_count++;
            if (event->len > 0) {
                if (event->mask & IN_CREATE) {
                    if (event->mask & IN_ISDIR) {
                        printf("  A new directory %s was created.\n", event->name);
                        printf("    The contents of this directory are not being monitored.\n");
                    }
                    else {
                        create_events++;
                        printf("  A new file %s was created: %d.\n", event->name, create_events);
                        char *ext = strrchr(event->name, '.');
                        if(ext && strcasecmp(ext, ".xml") == 0) {
                            printf("    This file is an xml file.\n");
                        }
                    }
                }
                else if (event->mask & IN_MODIFY) {
                    if (event->mask & IN_ISDIR) printf("  The directory %s was modified.\n", event->name);
                    else printf("  The file %s was modified.\n", event->name);
                }
                else if (event->mask & IN_DELETE) {
                    if (event->mask & IN_ISDIR) printf("  The directory %s was deleted.\n", event->name);
                    else printf("  The file %s was deleted.\n", event->name);
                }
                else if (event->mask &  IN_ATTRIB) {
                    if (event->mask& IN_ISDIR) printf("  Metadata for directory %s has changed.\n", event->name);
                    else printf("  Metadata for file %s has changed.\n", event->name);
                }
                else if (event->mask & IN_MOVED_FROM) {
                    if (event->mask & IN_ISDIR) printf("  The directory %s moved out; cookie %u.\n", event->name, event->cookie);
                    else printf("  The file %s moved out; cookie %u.\n", event->name, event->cookie);
                }
                else if (event->mask & IN_MOVED_TO) {
                    if (event->mask & IN_ISDIR) printf("  The directory %s was moved in, cookie %u.\n", event->name, event->cookie);
                    else {
                        printf("  The file %s was moved in, cookie %u.\n", event->name, event->cookie);
                        char *ext = strrchr(event->name, '.');
                        if (ext && strcasecmp(ext, ".xml") == 0) {
                            printf("    This file is an xml file.\n");
                        }
                    }
                }
                else if (event->mask & IN_CLOSE_WRITE) {
                    if (event->mask & IN_ISDIR) printf("  The directory %s was closed.\n", event->name);
                    else printf("  The file %s (opened for write) was closed.\n", event->name);
                }
                else if (event->mask & IN_CLOSE_NOWRITE) {
                    if (event->mask & IN_ISDIR) printf("  The directory %s was closed.\n", event->name);
                    else printf("  The file %s (opened for no-write) was closed.\n", event->name);
                }
                else {
                    printf("  Some other event occurred on %s.\n", event->name);
                }
                event_index += EVENT_SIZE + event->len;
            }
            else break;
        }
        printf("Finished with event block %d: %d\n", event_count, total_events);
    }
    return(EXIT_SUCCESS);
}

static void sigint_handler(int sig) {
    exit(EXIT_SUCCESS);
}

static void remove_notify(void) {
    fprintf(stderr, "\nCleaning up inotify structures\n");
    if ((fd >0) && (wd > 0)) {
        inotify_rm_watch(fd, wd);
        close(fd);
    }
}
