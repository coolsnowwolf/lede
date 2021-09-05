#ifndef NIXIO_MINGW_COMPAT_H_
#define NIXIO_MINGW_COMPAT_H_

#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#include <process.h>
#include <stdint.h>
#include <sys/stat.h>
#include <errno.h>

typedef unsigned int uint;
typedef unsigned long ulong;

#define S_ISLNK(m) 0
#define S_ISSOCK(m) 0

#define EWOULDBLOCK WSAEWOULDBLOCK
#define EAFNOSUPPORT WSAEAFNOSUPPORT
#define ENOPROTOOPT WSAENOPROTOOPT

#define SHUT_RD		SD_RECEIVE
#define SHUT_WR		SD_SEND
#define SHUT_RDWR	SD_BOTH

#define pipe(fds) _pipe(fds, 65536, 0)
#define fsync _commit
#define lseek lseek64
#define stat _stati64
#define lstat _stati64
#define fstat _fstati64


#define F_LOCK 1
#define F_ULOCK 0
#define F_TLOCK 2
#define F_TEST 3
int nixio__mgw_lockf(int fd, int cmd, off_t len);
#define lockf nixio__mgw_lockf

const char* nixio__mgw_inet_ntop
(int af, const void *src, char *dst, socklen_t size);
#define inet_ntop nixio__mgw_inet_ntop

int nixio__mgw_inet_pton (int af, const char *src, void *dst);
#define inet_pton nixio__mgw_inet_pton


#ifndef POLLIN
#define POLLIN	0x001
#define POLLOUT	0x004
#define POLLERR 0x008
struct pollfd {
   int   fd;
   short events;
   short revents;
};
#endif

typedef int nfds_t;
int nixio__mgw_poll(struct pollfd *fds, nfds_t nfds, int timeout);
#define poll nixio__mgw_poll


struct timespec {
      time_t tv_sec;
      long   tv_nsec;
};

int nixio__mgw_nanosleep(const struct timespec *req, struct timespec *rem);
#define nanosleep nixio__mgw_nanosleep


char* nixio__mgw_realpath(const char *path, char *resolved);
#define realpath nixio__mgw_realpath


int nixio__mgw_link(const char *oldpath, const char *newpath);
#define link nixio__mgw_link


int nixio__mgw_utimes(const char *filename, const struct timeval times[2]);
#define utimes nixio__mgw_utimes


#define setenv(k, v, o) !SetEnvironmentVariable(k, v)
#define unsetenv(k) !SetEnvironmentVariable(k, NULL)

#define execv(p, a) execv(p, (const char* const*)a)
#define execvp(p, a) execvp(p, (const char* const*)a)
#define execve(p, a, e) execve(p, (const char* const*)a, (const char* const*)e)

#define mkdir(p, m) mkdir(p)


#define nixio__perror_s(L) \
	errno = WSAGetLastError(); \
	return nixio__perror(L);

#define nixio__pstatus_s(L, c) \
	errno = WSAGetLastError(); \
	return nixio__pstatus(L, c);



#define NIXIO_WSA_CONSTANT(x) \
	lua_pushinteger(L, x); \
	lua_setfield(L, -2, #x+3);

void nixio_open__mingw(lua_State *L);

#endif /* NIXIO_MINGW_COMPAT_H_ */
