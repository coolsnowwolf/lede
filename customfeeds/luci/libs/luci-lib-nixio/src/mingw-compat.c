/*
 * nixio - Linux I/O library for lua
 *
 *   Copyright (C) 2009 Steven Barth <steven@midlink.org>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "nixio.h"
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/locking.h>
#include <sys/time.h>
#include <sys/utime.h>

void nixio_open__mingw(lua_State *L) {
	_fmode = _O_BINARY;

	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa)) {
		luaL_error(L, "Unable to initialize Winsock");
	}

	lua_newtable(L);

	NIXIO_WSA_CONSTANT(WSAEACCES);
	NIXIO_WSA_CONSTANT(WSAEINTR);
	NIXIO_WSA_CONSTANT(WSAEINVAL);
	NIXIO_WSA_CONSTANT(WSAEBADF);
	NIXIO_WSA_CONSTANT(WSAEFAULT);
	NIXIO_WSA_CONSTANT(WSAEMFILE);
	NIXIO_WSA_CONSTANT(WSAENAMETOOLONG);
	NIXIO_WSA_CONSTANT(WSAELOOP);
	NIXIO_WSA_CONSTANT(WSAEAFNOSUPPORT);
	NIXIO_WSA_CONSTANT(WSAENOBUFS);
	NIXIO_WSA_CONSTANT(WSAEPROTONOSUPPORT);
	NIXIO_WSA_CONSTANT(WSAENOPROTOOPT);
	NIXIO_WSA_CONSTANT(WSAEADDRINUSE);
	NIXIO_WSA_CONSTANT(WSAENETDOWN);
	NIXIO_WSA_CONSTANT(WSAENETUNREACH);
	NIXIO_WSA_CONSTANT(WSAECONNABORTED);
	NIXIO_WSA_CONSTANT(WSAECONNRESET);

	lua_setfield(L, -2, "const_sock");
}

const char* nixio__mgw_inet_ntop
(int af, const void *src, char *dst, socklen_t size) {
	struct sockaddr_storage saddr;
	memset(&saddr, 0, sizeof(saddr));

	DWORD hostlen = size, sl;
	if (af == AF_INET) {
		struct sockaddr_in *saddr4 = (struct sockaddr_in *)&saddr;
		memcpy(&saddr4->sin_addr, src, sizeof(saddr4->sin_addr));
		saddr4->sin_family = AF_INET;
		saddr4->sin_port = 0;
		sl = sizeof(struct sockaddr_in);
	} else if (af == AF_INET6) {
		struct sockaddr_in6 *saddr6 = (struct sockaddr_in6 *)&saddr;
		memcpy(&saddr6->sin6_addr, src, sizeof(saddr6->sin6_addr));
		saddr6->sin6_family = AF_INET6;
		saddr6->sin6_port = 0;
		sl = sizeof(struct sockaddr_in6);
	} else {
		return NULL;
	}
	if (WSAAddressToString((struct sockaddr*)&saddr, sl, NULL, dst, &hostlen)) {
		return NULL;
	}
	return dst;
}

int nixio__mgw_inet_pton (int af, const char *src, void *dst) {
	struct sockaddr_storage sa;
	int sl = sizeof(sa);

	if (!WSAStringToAddress((char*)src, af, NULL, (struct sockaddr*)&sa, &sl)) {
		if (af == AF_INET) {
			struct in_addr ina = ((struct sockaddr_in *)&sa)->sin_addr;
			memcpy(dst, &ina, sizeof(ina));
			return 1;
		} else if (af == AF_INET6) {
			struct in_addr6 ina6 = ((struct sockaddr_in6 *)&sa)->sin6_addr;
			memcpy(dst, &ina6, sizeof(ina6));
			return 1;
		} else {
			WSASetLastError(WSAEAFNOSUPPORT);
			return -1;
		}
	} else {
		return -1;
	}
}

int nixio__mgw_nanosleep(const struct timespec *req, struct timespec *rem) {
	if (rem) {
		rem->tv_sec = 0;
		rem->tv_nsec = 0;
	}
	Sleep(req->tv_sec * 1000 + req->tv_nsec * 1000000);
	return 0;
}

int nixio__mgw_poll(struct pollfd *fds, int nfds, int timeout) {
	if (!fds || !nfds) {
		Sleep(timeout);
		return 0;
	}

	struct timeval tv;
	int high = 0, rf = 0, wf = 0, ef = 0;
	fd_set rfds, wfds, efds;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

	for (int i = 0; i < nfds; i++) {
		if (fds->events & POLLIN) {
			FD_SET(fds->fd, &rfds);
			rf++;
		}
		if (fds->events & POLLOUT) {
			FD_SET(fds->fd, &wfds);
			wf++;
		}
		if (fds->events & POLLERR) {
			FD_SET(fds->fd, &efds);
			ef++;
		}
		if (fds->fd > high) {
			high = fds->fd;
		}
	}

	int stat = select(high + 1, (rf) ? &rfds : NULL,
	 (wf) ? &wfds : NULL, (ef) ? &efds : NULL, &tv);
	if (stat < 1) {
		errno = WSAGetLastError();
		return stat;
	}

	high = 0;

	for (int i = 0; i < nfds; i++) {
		fds->revents = 0;
		if ((fds->events & POLLIN) && FD_ISSET(fds->fd, &rfds)) {
			fds->revents |= POLLIN;
		}
		if ((fds->events & POLLOUT) && FD_ISSET(fds->fd, &wfds)) {
			fds->revents |= POLLOUT;
		}
		if ((fds->events & POLLERR) && FD_ISSET(fds->fd, &efds)) {
			fds->revents |= POLLERR;
		}
		if (fds->revents) {
			high++;
		}
	}

	return high;
}

int nixio__mgw_lockf(int fd, int cmd, off_t len) {
	int stat;
	if (cmd == F_LOCK) {
		do {
			stat = _locking(fd, _LK_LOCK, len);
		} while (stat == -1 && errno == EDEADLOCK);
	} else if (cmd == F_TLOCK) {
		stat = _locking(fd, _LK_NBLCK, len);
	} else if (cmd == F_ULOCK) {
		stat = _locking(fd, _LK_UNLCK, len);
	} else {
		stat = -1;
		errno = EINVAL;
	}
	return stat;
}

char* nixio__mgw_realpath(const char *path, char *resolved) {
	if (GetFullPathName(path, PATH_MAX, resolved, NULL)) {
		return resolved;
	} else {
		errno = GetLastError();
		return NULL;
	}
}

int nixio__mgw_link(const char *oldpath, const char *newpath) {
	if (!CreateHardLink(newpath, oldpath, NULL)) {
		errno = GetLastError();
		return -1;
	} else {
		return 0;
	}
}

int nixio__mgw_utimes(const char *filename, const struct timeval times[2]) {
	struct _utimbuf timebuffer;
	timebuffer.actime = times[0].tv_sec;
	timebuffer.modtime = times[1].tv_sec;

	return _utime(filename, &timebuffer);
}
