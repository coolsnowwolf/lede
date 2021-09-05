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
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#define NIXIO_EXECVE	0x01
#define NIXIO_EXECV		0x02
#define NIXIO_EXECVP	0x03

int nixio__exec(lua_State *L, int m) {
	const char *path = luaL_checkstring(L, 1);
	const char *arg;
	int argn, i;

	if (m == NIXIO_EXECVE) {
		luaL_checktype(L, 2, LUA_TTABLE);
		argn = lua_objlen(L, 2) + 1;
	} else {
		argn = lua_gettop(L);
	}

	char **args = lua_newuserdata(L, sizeof(char*) * (argn + 1));
	args[argn] = NULL;
	args[0] = (char *)path;

	if (m == NIXIO_EXECVE) {
		for (i = 1; i < argn; i++) {
			lua_rawgeti(L, 2, i);
			arg = lua_tostring(L, -1);
			luaL_argcheck(L, arg, 2, "invalid argument");
			args[i] = (char *)arg;
		}

		if (lua_isnoneornil(L, 3)) {
			execv(path, args);
		} else {
			luaL_checktype(L, 3, LUA_TTABLE);
			argn = 0;
			lua_pushnil(L);
			while (lua_next(L, 3)) {
				if (!lua_checkstack(L, 1)) {
					lua_settop(L, 0);
					return luaL_error(L, "stack overflow");
				}

				if (lua_type(L, -2) != LUA_TSTRING || !lua_isstring(L, -1)) {
					return luaL_argerror(L, 3, "invalid environment");
				}

				lua_pushfstring(L, "%s=%s",
						lua_tostring(L, -2), lua_tostring(L, -1));

				lua_insert(L, 5);
				lua_pop(L, 1);
				argn++;
			}

			char **env = lua_newuserdata(L, sizeof(char*) * (argn + 1));
			env[argn] = NULL;

			for (i = 1; i <= argn; i++) {
				env[i-1] = (char *)lua_tostring(L, -(i+1));
			}

			execve(path, args, env);
		}
	} else {
		for (i = 2; i <= argn; i++) {
			arg = luaL_checkstring(L, i);
			args[i-1] = (char *)arg;
		}

		if (m == NIXIO_EXECV) {
			execv(path, args);
		} else {
			execvp(path, args);
		}
	}

	return nixio__perror(L);
}

#ifndef __WINNT__
#include <sys/utsname.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>

static int nixio_fork(lua_State *L) {
	pid_t pid = fork();
	if (pid == -1) {
		return nixio__perror(L);
	} else {
		lua_pushinteger(L, pid);
		return 1;
	}
}

static int nixio_kill(lua_State *L) {
	return nixio__pstatus(L, !kill(luaL_checkint(L, 1), luaL_checkint(L, 2)));
}

static int nixio_getppid(lua_State *L) {
	lua_pushinteger(L, getppid());
	return 1;
}

static int nixio_getuid(lua_State *L) {
	lua_pushinteger(L, getuid());
	return 1;
}

static int nixio_getgid(lua_State *L) {
	lua_pushinteger(L, getgid());
	return 1;
}

static int nixio_setgid(lua_State *L) {
	return nixio__pstatus(L, !setgid(nixio__check_group(L, 1)));
}

static int nixio_setuid(lua_State *L) {
	return nixio__pstatus(L, !setuid(nixio__check_user(L, 1)));
}

static int nixio_nice(lua_State *L) {
	int nval = luaL_checkint(L, 1);

	errno = 0;
	nval = nice(nval);

	if (nval == -1 && errno) {
		return nixio__perror(L);
	} else {
		lua_pushinteger(L, nval);
		return 1;
	}
}

static int nixio_setsid(lua_State *L) {
	pid_t pid = setsid();

	if (pid == -1) {
		return nixio__perror(L);
	} else {
		lua_pushinteger(L, pid);
		return 1;
	}
}

static int nixio_wait(lua_State *L) {
	pid_t pidin = luaL_optinteger(L, 1, -1), pidout;
	int options = 0, status;

	const int j = lua_gettop(L);
	for (int i=2; i<=j; i++) {
		const char *flag = luaL_checkstring(L, i);
		if (!strcmp(flag, "nohang")) {
			options |= WNOHANG;
		} else if (!strcmp(flag, "untraced")) {
			options |= WUNTRACED;
		} else if (!strcmp(flag, "continued")) {
			options |= WCONTINUED;
		} else {
			return luaL_argerror(L, i,
					"supported values: nohang, untraced, continued");
		}
	}

	do {
		pidout = waitpid(pidin, &status, options);
	} while (pidout == -1 && errno == EINTR);

	if (pidout == 0) {
		lua_pushboolean(L, 0);
		return 1;
	} else if (pidout == -1) {
		return nixio__perror(L);
	} else {
		lua_pushinteger(L, pidout);
	}

	if (WIFEXITED(status)) {
		lua_pushliteral(L, "exited");
		lua_pushinteger(L, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
    	lua_pushliteral(L, "signaled");
    	lua_pushinteger(L, WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
    	lua_pushliteral(L, "stopped");
    	lua_pushinteger(L, WSTOPSIG(status));
    } else {
    	return 1;
    }

    return 3;
}

static int nixio_times(lua_State *L) {
	struct tms buf;
	if (times(&buf) == -1) {
		return nixio__perror(L);
	} else {
		lua_createtable(L, 0, 4);
		nixio__pushnumber(L, buf.tms_cstime);
		lua_setfield(L, -2, "cstime");

		nixio__pushnumber(L, buf.tms_cutime);
		lua_setfield(L, -2, "cutime");

		nixio__pushnumber(L, buf.tms_stime);
		lua_setfield(L, -2, "stime");

		nixio__pushnumber(L, buf.tms_utime);
		lua_setfield(L, -2, "utime");

		return 1;
	}
}

static int nixio_uname(lua_State *L) {
	struct utsname buf;
	if (uname(&buf)) {
		return nixio__perror(L);
	}

	lua_createtable(L, 0, 5);

	lua_pushstring(L, buf.machine);
	lua_setfield(L, -2, "machine");

	lua_pushstring(L, buf.version);
	lua_setfield(L, -2, "version");

	lua_pushstring(L, buf.release);
	lua_setfield(L, -2, "release");

	lua_pushstring(L, buf.nodename);
	lua_setfield(L, -2, "nodename");

	lua_pushstring(L, buf.sysname);
	lua_setfield(L, -2, "sysname");

	return 1;
}

#endif /* !__WINNT__ */

static int nixio_chdir(lua_State *L) {
	return nixio__pstatus(L, !chdir(luaL_checkstring(L, 1)));
}

static int nixio_signal(lua_State *L) {
	int sig = luaL_checkinteger(L, 1);
	const char *val = luaL_checkstring(L, 2);

	if (!strcmp(val, "ign") || !strcmp(val, "ignore")) {
		return nixio__pstatus(L, signal(sig, SIG_IGN) != SIG_ERR);
	} else if (!strcmp(val, "dfl") || !strcmp(val, "default")) {
		return nixio__pstatus(L, signal(sig, SIG_DFL) != SIG_ERR);
	} else {
		return luaL_argerror(L, 2, "supported values: ign, dfl");
	}
}

static int nixio_getpid(lua_State *L) {
	lua_pushinteger(L, getpid());
	return 1;
}

static int nixio_getenv(lua_State *L) {
	const char *key = luaL_optstring(L, 1, NULL);
	if (key) {
		const char *val = getenv(key);
		if (val) {
			lua_pushstring(L, val);
		} else {
			lua_pushnil(L);
		}
	} else {
		lua_newtable(L);
		extern char **environ;
		for (char **c = environ; *c; c++) {
			const char *delim = strchr(*c, '=');
			if (!delim) {
				return luaL_error(L, "invalid environment");
			}
			lua_pushlstring(L, *c, delim-*c);
			lua_pushstring(L, delim + 1);
			lua_rawset(L, -3);
		}
	}
	return 1;
}

static int nixio_setenv(lua_State *L) {
	const char *key = luaL_checkstring(L, 1);
	const char *val = luaL_optstring(L, 2, NULL);
	return nixio__pstatus(L, (val) ? !setenv(key, val, 1) : !unsetenv(key));
}

static int nixio_exec(lua_State *L) {
	return nixio__exec(L, NIXIO_EXECV);
}

static int nixio_execp(lua_State *L) {
	return nixio__exec(L, NIXIO_EXECVP);
}

static int nixio_exece(lua_State *L) {
	return nixio__exec(L, NIXIO_EXECVE);
}

static int nixio_getcwd(lua_State *L) {
	char path[PATH_MAX];

	if (getcwd(path, sizeof(path))) {
		lua_pushstring(L, path);
		return 1;
	} else {
		return nixio__perror(L);
	}
}

static int nixio_umask(lua_State *L) {
	char mask[9];
	lua_pushinteger(L,
			nixio__mode_write(umask(nixio__check_mode(L, 1, -1)), mask));
	lua_pushlstring(L, mask, 9);
	return 2;
}

#ifdef __linux__

#include <sys/sysinfo.h>

static int nixio_sysinfo(lua_State *L) {
	struct sysinfo info;
	if (sysinfo(&info)) {
		return nixio__perror(L);
	}

	lua_createtable(L, 0, 12);

	nixio__pushnumber(L, info.bufferram);
	lua_setfield(L, -2, "bufferram");

	nixio__pushnumber(L, info.freehigh);
	lua_setfield(L, -2, "freehigh");

	nixio__pushnumber(L, info.freeram);
	lua_setfield(L, -2, "freeram");

	nixio__pushnumber(L, info.freeswap);
	lua_setfield(L, -2, "freeswap");

	lua_createtable(L, 0, 3);
	for (int i=0; i<3; i++) {
		lua_pushnumber(L, info.loads[i] / 65536.);
		lua_rawseti(L, -2, i+1);
	}
	lua_setfield(L, -2, "loads");

	lua_pushinteger(L, info.mem_unit);
	lua_setfield(L, -2, "mem_unit");

	lua_pushinteger(L, info.procs);
	lua_setfield(L, -2, "procs");

	nixio__pushnumber(L, info.sharedram);
	lua_setfield(L, -2, "sharedram");

	nixio__pushnumber(L, info.totalhigh);
	lua_setfield(L, -2, "totalhigh");

	nixio__pushnumber(L, info.totalram);
	lua_setfield(L, -2, "totalram");

	nixio__pushnumber(L, info.totalswap);
	lua_setfield(L, -2, "totalswap");

	lua_pushinteger(L, info.uptime);
	lua_setfield(L, -2, "uptime");

	return 1;
}

#endif


/* module table */
static const luaL_reg R[] = {
#ifdef __linux__
	{"sysinfo",		nixio_sysinfo},
#endif
#ifndef __WINNT__
	{"fork",		nixio_fork},
	{"kill",		nixio_kill},
	{"nice",		nixio_nice},
	{"getppid",		nixio_getppid},
	{"getuid",		nixio_getuid},
	{"getgid",		nixio_getgid},
	{"setuid",		nixio_setuid},
	{"setgid",		nixio_setgid},
	{"setsid",		nixio_setsid},
	{"wait",		nixio_wait},
	{"waitpid",		nixio_wait},
	{"times",		nixio_times},
	{"uname",		nixio_uname},
#endif
	{"chdir",		nixio_chdir},
	{"signal",		nixio_signal},
	{"getpid",		nixio_getpid},
	{"getenv",		nixio_getenv},
	{"setenv",		nixio_setenv},
	{"putenv",		nixio_setenv},
	{"exec",		nixio_exec},
	{"execp",		nixio_execp},
	{"exece",		nixio_exece},
	{"getcwd",		nixio_getcwd},
	{"umask",		nixio_umask},
	{NULL,			NULL}
};

void nixio_open_process(lua_State *L) {
	luaL_register(L, NULL, R);
}
