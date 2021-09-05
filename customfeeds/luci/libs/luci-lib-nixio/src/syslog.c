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

#ifndef __WINNT__
#include <syslog.h>


static int nixio_openlog(lua_State *L) {
	int option = 0, facility = LOG_USER;

	const char *ident = luaL_optstring(L, 1, "nixio");
	const int j = lua_gettop(L);
	for (int i=2; i<=j; i++) {
		const char *flag = luaL_checkstring(L, i);
		if (!strcmp(flag, "cons")) {
			option |= LOG_CONS;
		} else if (!strcmp(flag, "nowait")) {
			option |= LOG_NOWAIT;
		} else if (!strcmp(flag, "pid")) {
			option |= LOG_PID;
		} else if (!strcmp(flag, "perror")) {
#ifdef LOG_PERROR
			option |= LOG_PERROR;
#endif
		} else if (!strcmp(flag, "ndelay")) {
			option |= LOG_NDELAY;
		} else if (!strcmp(flag, "odelay")) {
			option |= LOG_ODELAY;
		} else {
			return luaL_argerror(L, i,
				"supported values: cons, nowait, pid, perror, ndelay, odelay");
		}
	}

	openlog(ident, option, facility);
	return 0;
}

static int nixio_closelog(lua_State *L) {
	closelog();
	return 0;
}

static int nixio__syslogmask(lua_State *L, int dolog) {
	int priority;

	const char *flag = luaL_checkstring(L, 1);
	if (!strcmp(flag, "emerg")) {
		priority = LOG_EMERG;
	} else if (!strcmp(flag, "alert")) {
		priority = LOG_ALERT;
	} else if (!strcmp(flag, "crit")) {
		priority = LOG_CRIT;
	} else if (!strcmp(flag, "err")) {
		priority = LOG_ERR;
	} else if (!strcmp(flag, "warning")) {
		priority = LOG_WARNING;
	} else if (!strcmp(flag, "notice")) {
		priority = LOG_NOTICE;
	} else if (!strcmp(flag, "info")) {
		priority = LOG_INFO;
	} else if (!strcmp(flag, "debug")) {
		priority = LOG_DEBUG;
	} else {
		return luaL_argerror(L, 1, "supported values: emerg, alert, crit, err, "
				"warning, notice, info, debug");
	}

	if (dolog) {
		const char *msg = luaL_checkstring(L, 2);
		syslog(priority, "%s", msg);
	} else {
		setlogmask(LOG_UPTO(priority));
	}
	return 0;
}

static int nixio_setlogmask(lua_State *L) {
	return nixio__syslogmask(L, 0);
}

static int nixio_syslog(lua_State *L) {
	return nixio__syslogmask(L, 1);
}

/* module table */
static const luaL_reg R[] = {
	{"openlog",		nixio_openlog},
	{"syslog",		nixio_syslog},
	{"setlogmask",	nixio_setlogmask},
	{"closelog",	nixio_closelog},
	{NULL,			NULL}
};

void nixio_open_syslog(lua_State *L) {
	luaL_register(L, NULL, R);
}

#else /* __WINNT__ */

void nixio_open_syslog(lua_State *L) {
}

#endif /* __WINNT__ */
