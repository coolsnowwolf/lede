#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "lock.h"

#define LOCKFILE "/var/run/dclient.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int already_running(const char *filename)
{
    int fd, t; // t用来消除警告
    char buf[16];
    if(filename == NULL) filename = LOCKFILE;
    fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
    if(fd < 0)
    {
        fprintf(stderr, "pid file open faild!");
        exit(EXIT_FAILURE);
    }

    if (lockfile(fd) == -1)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            fprintf(stderr, "dclient already running!");
            close(fd);
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "Can't lock pid file!");
        exit(EXIT_FAILURE);
    }
    t = ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    t = write(fd, buf, strlen(buf) + 1);
    if(t) { }
    return 0;
}


int lockfile(int fd)
{
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    return (fcntl(fd, F_SETLK, &fl));
}

