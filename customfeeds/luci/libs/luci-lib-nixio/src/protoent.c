/*
 * nixio - Linux I/O library for lua
 *
 *   Copyright (C) 2011 Jo-Philipp Wich <jow@openwrt.org>
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

#ifndef __WINNT__
#include <netdb.h>
#endif

/**
 * protoent conversion helper
 */
static int nixio__pushprotoent(lua_State *L, struct protoent *e) {
	int i;
	if (e) {
		lua_newtable(L);

		lua_pushstring(L, e->p_name);
		lua_setfield(L, -2, "name");

		lua_pushnumber(L, e->p_proto);
		lua_setfield(L, -2, "proto");

		lua_newtable(L);
		for (i = 0; e->p_aliases[i]; i++) {
			lua_pushstring(L, e->p_aliases[i]);
			lua_rawseti(L, -2, i+1);
		}
		lua_setfield(L, -2, "aliases");
		return 1;
	} else {
		return 0;
	}
}

/**
 * getprotobyname(name)
 */
static int nixio_getprotobyname(lua_State *L) {
	const char *name = luaL_checkstring(L, 1);
	struct protoent *res = getprotobyname(name);
	return nixio__pushprotoent(L, res);
}

/**
 * getprotobynumber(proto)
 */
static int nixio_getprotobynumber(lua_State *L) {
	int proto = luaL_checkinteger(L, 1);
	struct protoent *res = getprotobynumber(proto);
	return nixio__pushprotoent(L, res);
}

/**
 * getproto(name_or_proto)
 */
static int nixio_getproto(lua_State *L) {
	int i = 1;
	struct protoent *res;
	if (lua_isnumber(L, 1)) {
		return nixio_getprotobynumber(L);
	} else if (lua_isstring(L, 1)) {
		return nixio_getprotobyname(L);
	} else if (lua_isnoneornil(L, 1)) {
		setprotoent(1);
		lua_newtable(L);
		while ((res = getprotoent()) != NULL) {
			nixio__pushprotoent(L, res);
			lua_rawseti(L, -2, i++);
		}
		endprotoent();
		return 1;
	} else {
		return luaL_argerror(L, 1, "supported values: <protoname>, <protonumber>");
	}
}

/* module table */
static const luaL_reg R[] = {
    {"getprotobyname",		nixio_getprotobyname},
    {"getprotobynumber",	nixio_getprotobynumber},
    {"getproto",			nixio_getproto},
    {NULL,					NULL}
};

void nixio_open_protoent(lua_State *L) {
    luaL_register(L, NULL, R);
}
