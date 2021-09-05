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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define VERSION 0.4


/* pushes nil, error number and errstring on the stack */
int nixio__perror(lua_State *L) {
	if (errno == EAGAIN || errno == EWOULDBLOCK) {
		lua_pushboolean(L, 0);
	} else {
		lua_pushnil(L);
	}
    lua_pushinteger(L, errno);
    lua_pushstring(L, strerror(errno));
    return 3;
}

/* pushes true, if operation succeeded, otherwise call nixio__perror */
int nixio__pstatus(lua_State *L, int condition) {
	if (condition) {
		lua_pushboolean(L, 1);
		return 1;
	} else {
		return nixio__perror(L);
	}
}

/* checks whether the first argument is a socket and returns it */
nixio_sock* nixio__checksock(lua_State *L) {
    nixio_sock *sock = (nixio_sock*)luaL_checkudata(L, 1, NIXIO_META);
    luaL_argcheck(L, sock->fd != -1, 1, "invalid socket object");
    return sock;
}

/* read fd from nixio_sock object */
int nixio__checksockfd(lua_State *L) {
	return nixio__checksock(L)->fd;
}

/* return any possible fd, otherwise error out */
int nixio__checkfd(lua_State *L, int ud) {
	int fd = nixio__tofd(L, ud);
	return (fd != -1) ? fd : luaL_argerror(L, ud, "invalid file descriptor");
}

/* return any possible fd */
int nixio__tofd(lua_State *L, int ud) {
	void *udata = lua_touserdata(L, ud);
	int fd = -1;
	if (lua_getmetatable(L, ud)) {
		luaL_getmetatable(L, NIXIO_META);
		luaL_getmetatable(L, NIXIO_FILE_META);
		luaL_getmetatable(L, LUA_FILEHANDLE);
		if (lua_rawequal(L, -3, -4)) {
			fd = ((nixio_sock*)udata)->fd;
		} else if (lua_rawequal(L, -2, -4)) {
			fd = *((int*)udata);
		} else if (lua_rawequal(L, -1, -4)) {
			fd = (*((FILE **)udata)) ? fileno(*((FILE **)udata)) : -1;
		}
		lua_pop(L, 4);
	}
	return fd;
}

/* An empty iterator */
int nixio__nulliter(lua_State *L) {
	lua_pushnil(L);
	return 1;
}

static int nixio_errno(lua_State *L) {
	lua_pushinteger(L, errno);
	return 1;
}

static int nixio_strerror(lua_State *L) {
	lua_pushstring(L, strerror(luaL_checkinteger(L, 1)));
	return 1;
}

/* object table */
static const luaL_reg R[] = {
	{"errno",		nixio_errno},
	{"strerror",	nixio_strerror},
	{NULL,			NULL}
};

/* entry point */
NIXIO_API int luaopen_nixio(lua_State *L) {
	/* create metatable */
	luaL_newmetatable(L, NIXIO_META);

	/* metatable.__index = metatable */
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	/* register module */
	luaL_register(L, "nixio", R);

	/* register metatable as socket_meta */
	lua_pushvalue(L, -2);
	lua_setfield(L, -2, "meta_socket");

	/* register methods */
#ifdef __WINNT__
	nixio_open__mingw(L);
#endif
	nixio_open_file(L);
	nixio_open_socket(L);
	nixio_open_sockopt(L);
	nixio_open_bind(L);
	nixio_open_address(L);
	nixio_open_protoent(L);
	nixio_open_poll(L);
	nixio_open_io(L);
	nixio_open_splice(L);
	nixio_open_process(L);
	nixio_open_syslog(L);
	nixio_open_bit(L);
	nixio_open_bin(L);
	nixio_open_fs(L);
	nixio_open_user(L);

#ifndef NO_TLS
	nixio_open_tls_crypto(L);
	nixio_open_tls_context(L);
	nixio_open_tls_socket(L);
#endif

	/* module version */
	lua_pushinteger(L, VERSION);
	lua_setfield(L, -2, "version");

	/* some constants */
	lua_newtable(L);

	lua_pushliteral(L, NIXIO_SEP);
	lua_setfield(L, -2, "sep");

	lua_pushliteral(L, NIXIO_PATHSEP);
	lua_setfield(L, -2, "pathsep");

	lua_pushinteger(L, NIXIO_BUFFERSIZE);
	lua_setfield(L, -2, "buffersize");

	NIXIO_PUSH_CONSTANT(EACCES);
	NIXIO_PUSH_CONSTANT(EINTR);
	NIXIO_PUSH_CONSTANT(ENOSYS);
	NIXIO_PUSH_CONSTANT(EINVAL);
	NIXIO_PUSH_CONSTANT(EAGAIN);
	NIXIO_PUSH_CONSTANT(ENOMEM);
	NIXIO_PUSH_CONSTANT(ENOENT);
	NIXIO_PUSH_CONSTANT(ECHILD);
	NIXIO_PUSH_CONSTANT(EIO);
	NIXIO_PUSH_CONSTANT(EBADF);
	NIXIO_PUSH_CONSTANT(EFAULT);
	NIXIO_PUSH_CONSTANT(EFBIG);
	NIXIO_PUSH_CONSTANT(ENOSPC);
	NIXIO_PUSH_CONSTANT(EPIPE);
	NIXIO_PUSH_CONSTANT(ESPIPE);
	NIXIO_PUSH_CONSTANT(EISDIR);
	NIXIO_PUSH_CONSTANT(EPERM);
	NIXIO_PUSH_CONSTANT(EEXIST);
	NIXIO_PUSH_CONSTANT(EMFILE);
	NIXIO_PUSH_CONSTANT(ENAMETOOLONG);
	NIXIO_PUSH_CONSTANT(ENFILE);
	NIXIO_PUSH_CONSTANT(ENODEV);
	NIXIO_PUSH_CONSTANT(EXDEV);
	NIXIO_PUSH_CONSTANT(ENOTDIR);
	NIXIO_PUSH_CONSTANT(ENXIO);
	NIXIO_PUSH_CONSTANT(EROFS);
	NIXIO_PUSH_CONSTANT(EBUSY);
	NIXIO_PUSH_CONSTANT(ESRCH);
	NIXIO_PUSH_CONSTANT(SIGINT);
	NIXIO_PUSH_CONSTANT(SIGTERM);
	NIXIO_PUSH_CONSTANT(SIGSEGV);

#ifndef __WINNT__
	NIXIO_PUSH_CONSTANT(EALREADY);
	NIXIO_PUSH_CONSTANT(EINPROGRESS);
	NIXIO_PUSH_CONSTANT(EWOULDBLOCK);
	NIXIO_PUSH_CONSTANT(ELOOP);
	NIXIO_PUSH_CONSTANT(EOVERFLOW);
	NIXIO_PUSH_CONSTANT(ETXTBSY);
	NIXIO_PUSH_CONSTANT(EAFNOSUPPORT);
	NIXIO_PUSH_CONSTANT(ENOBUFS);
	NIXIO_PUSH_CONSTANT(EPROTONOSUPPORT);
	NIXIO_PUSH_CONSTANT(ENOPROTOOPT);
	NIXIO_PUSH_CONSTANT(EADDRINUSE);
	NIXIO_PUSH_CONSTANT(ENETDOWN);
	NIXIO_PUSH_CONSTANT(ENETUNREACH);

	NIXIO_PUSH_CONSTANT(SIGALRM);
	NIXIO_PUSH_CONSTANT(SIGKILL);
	NIXIO_PUSH_CONSTANT(SIGHUP);
	NIXIO_PUSH_CONSTANT(SIGSTOP);
	NIXIO_PUSH_CONSTANT(SIGCONT);
	NIXIO_PUSH_CONSTANT(SIGCHLD);
	NIXIO_PUSH_CONSTANT(SIGQUIT);
	NIXIO_PUSH_CONSTANT(SIGUSR1);
	NIXIO_PUSH_CONSTANT(SIGUSR2);
	NIXIO_PUSH_CONSTANT(SIGIO);
	NIXIO_PUSH_CONSTANT(SIGURG);
	NIXIO_PUSH_CONSTANT(SIGPIPE);

	lua_pushvalue(L, -1);
	lua_setfield(L, -3, "const_sock");

	signal(SIGPIPE, SIG_IGN);
#endif /* !__WINNT__ */
	lua_setfield(L, -2, "const");

	/* remove meta table */
	lua_remove(L, -2);

	return 1;
}
