#ifndef LOCK_H_INCLUDE
#define LOCK_H_INCLUDE

int already_running(const char *filename = NULL);
int lockfile(int fd);

#endif
