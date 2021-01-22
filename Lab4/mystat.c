#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

// ./mystat [file(s)]

void print_info(char *, struct stat);

void print_info(char *name, struct stat fs) {
    char *type;
    long maj;
    long min;
    char mode[11] = "----------\0";
    struct passwd *user;
    struct group *grp;
    struct tm *at = localtime(&(fs.st_atime));
    struct tm *mt = localtime(&(fs.st_mtime));
    struct tm *ct = localtime(&(fs.st_ctime));
    char a_time[100];
    char m_time[100];
    char c_time[100];

    // used in case of symbolic link
    char *link;
    ssize_t nbytes, bufsize;
    struct stat test;

    // determine file type and device ID
    switch (fs.st_mode & S_IFMT) { // from lstat manpage
    case S_IFBLK:
        type = "block device";
        maj = major(fs.st_rdev);
        min = minor(fs.st_rdev);
        mode[0] = 'b';
        break;
    case S_IFCHR:
        type = "character device";
        maj = major(fs.st_rdev);
        min = minor(fs.st_rdev);
        mode[0] = 'c';
        break;
    case S_IFDIR:
        type = "directory";
        maj = major(fs.st_dev);
        min = minor(fs.st_dev);
        mode[0] = 'd';
        break;
    case S_IFIFO:
        type = "FIFO/pipe";
        maj = major(fs.st_dev);
        min = minor(fs.st_dev);
        mode[0] = 'p';
        break;
    case S_IFLNK: // symbolic link reading from readlink manpage
        bufsize = fs.st_size + 1;
        link = malloc(bufsize);
        type = alloca(bufsize);

        nbytes = readlink(name, link, bufsize);
        link[nbytes] = '\0';
        
        if (stat(link, &test) == -1) {
            link = "DANGLING";
        }
        
        strcpy(type, "symbolic link -> ");
        strcat(type, link);
        maj = major(fs.st_dev);
        min = minor(fs.st_dev);
        mode[0] = 'l';

        if (strcmp(link, "DANGLING") != 0) {
                free(link);
            }
        break;
    case S_IFREG:
        type = "regular file";
        maj = major(fs.st_dev);
        min = minor(fs.st_dev);
        break;
    case S_IFSOCK:
        type = "socket";
        maj = major(fs.st_dev);
        min = minor(fs.st_dev);
        mode[0] = 's';
        break;
    default:
        type = "unknown";
        maj = major(fs.st_dev);
        min = minor(fs.st_dev);
        mode[0] = '?';
        break;
    }

    // determine permission bits
    if (fs.st_mode & S_IRUSR) mode[1] = 'r';    // user
    if (fs.st_mode & S_IWUSR) mode[2] = 'w';
    if (fs.st_mode & S_IXUSR) mode[3] = 'x';
    if (fs.st_mode & S_IRGRP) mode[4] = 'r';    // group
    if (fs.st_mode & S_IWGRP) mode[5] = 'w';
    if (fs.st_mode & S_IXGRP) mode[6] = 'x';
    if (fs.st_mode & S_IROTH) mode[7] = 'r';    // other
    if (fs.st_mode & S_IWOTH) mode[8] = 'w';
    if (fs.st_mode & S_IXOTH) mode[9] = 'x';

    // collect user and group ID info
    user = getpwuid(fs.st_uid);
    grp = getgrgid(fs.st_gid);

    // turn time info into readable format
    strftime(a_time, sizeof(a_time), "%Y-%m-%d %H:%M:%S %z (%Z) %a", at);
    strftime(m_time, sizeof(m_time), "%Y-%m-%d %H:%M:%S %z (%Z) %a", mt);
    strftime(c_time, sizeof(c_time), "%Y-%m-%d %H:%M:%S %z (%Z) %a", ct);

    printf("File: %s\n\t"
           "File type:\t\t\t %s\n\t"
           "Device ID:\t\t\t [%lx,%lx]\n\t"
           "I-node number:\t\t\t %ld\n\t"
           "Mode:\t\t\t\t %s\t (%lo in octal)\n\t"
           "Link count:\t\t\t %ld\n\t"
           "Owner ID:\t\t\t %s   \t (UID = %ld)\n\t"
           "Group ID:\t\t\t %s   \t (GID = %ld)\n\t"
           "Preferred I/O block size:\t %ld bytes\n\t"
           "File size:\t\t\t %lld bytes\n\t"
           "Blocks allocated:\t\t %lld\n\t"
           "Last file access:\t\t %s\n\t"
           "Last file modification:\t\t %s\n\t"
           "Last status change:\t\t %s\n"

           , name
           , type
           , maj, min
           , (long) fs.st_ino
           , mode, (unsigned long) (fs.st_mode & ~S_IFMT)
           , (long) fs.st_nlink
           , user->pw_name, (long) fs.st_uid
           , grp->gr_name, (long) fs.st_gid
           , (long) fs.st_blksize
           , (long long) fs.st_size
           , (long long) fs.st_blocks
           , a_time
           , m_time
           , c_time
        );

}

int main(int argc, char *argv[]) {
    struct stat buf;
    int j;
    for(j = 1; j < argc; j++) {
        if (lstat(argv[j], &buf) == -1) {
            printf("lstat failed to read file %s\n", argv[j]);
        }
        else {
            print_info(argv[j], buf);
        }
    }

    return(EXIT_SUCCESS);
}
