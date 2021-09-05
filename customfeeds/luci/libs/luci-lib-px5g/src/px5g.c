/*
 * px5g - Embedded x509 key and certificate generator based on PolarSSL
 *
 *   Copyright (C) 2009 Steven Barth <steven@midlink.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License, version 2.1 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#include "px5g.h"
#include <time.h>
#include <string.h>
#define VERSION 0.1

static char *xfields[] = {"CN", "O", "C", "OU", "ST", "L", "R"};

static int px5g_genkey(lua_State *L) {
	int keysize = luaL_checkint(L, 1), pexp = luaL_optint(L, 2, 65537), ret;
	px5g_rsa *px5g = lua_newuserdata(L, sizeof(px5g_rsa));
	if (!px5g) {
		return luaL_error(L, "out of memory");
	}

	px5g->stat = 1;
	havege_init(&px5g->hs);
	rsa_init(&px5g->rsa, RSA_PKCS_V15, 0, havege_rand, &px5g->hs);

	if ((ret = rsa_gen_key(&px5g->rsa, keysize, pexp))) {
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}

	luaL_getmetatable(L, PX5G_KEY_META);
	lua_setmetatable(L, -2);
	return 1;
}

static int px5g_rsa_asn1(lua_State *L) {
	int ret;
	px5g_rsa *px5g = luaL_checkudata(L, 1, PX5G_KEY_META);
	x509_node node;

	x509write_init_node(&node);
	if ((ret = x509write_serialize_key(&px5g->rsa, &node))) {
		x509write_free_node(&node);
		lua_pushnil(L);
		lua_pushinteger(L, ret);
		return 2;
	}

	lua_pushlstring(L, (char*)node.data, node.len);
	x509write_free_node(&node);
	return 1;
}

static int px5g_rsa_create_selfsigned(lua_State *L) {
	px5g_rsa *px5g = luaL_checkudata(L, 1, PX5G_KEY_META);
	luaL_checktype(L, 2, LUA_TTABLE);
	time_t from = (time_t)luaL_checknumber(L, 3);
	time_t to = (time_t)luaL_checknumber(L, 4);
	char fstr[20], tstr[20];

	lua_pushliteral(L, "CN");
	lua_rawget(L, 2);
	luaL_argcheck(L, lua_isstring(L, -1), 2, "CN missing");
	lua_pop(L, 1);

	luaL_argcheck(L,
		strftime(fstr, sizeof(fstr), "%F %H:%M:%S", gmtime(&from)),
	3, "Invalid Time");

	luaL_argcheck(L,
		strftime(tstr, sizeof(tstr), "%F %H:%M:%S", gmtime(&to)),
	4, "Invalid Time");

	size_t join = 1;
	lua_pushliteral(L, "");
	for (int i = 0; i < (sizeof(xfields) / sizeof(*xfields)); i++) {
		lua_pushstring(L, xfields[i]);
		lua_rawget(L, 2);
		if (lua_isstring(L, -1)) {
			const char *val = lua_tostring(L, -1);
			luaL_argcheck(L, !strchr(val, ';'), 2, "Invalid Value");
			lua_pushfstring(L, "%s=%s;", xfields[i], val);
			lua_remove(L, -2);
			join++;
		} else {
			lua_pop(L, 1);
		}
	}
	lua_concat(L, join);

	x509_raw cert;
	x509write_init_raw(&cert);
	x509write_add_pubkey(&cert, &px5g->rsa);
	x509write_add_subject(&cert, (unsigned char*)lua_tostring(L, -1));
	x509write_add_validity(&cert, (unsigned char*)fstr, (unsigned char*)tstr);
	x509write_create_selfsign(&cert, &px5g->rsa);

	lua_pushlstring(L, (char*)cert.raw.data, cert.raw.len);
	x509write_free_raw(&cert);
	return 1;
}

static int px5g_rsa__gc(lua_State *L) {
	px5g_rsa *px5g = luaL_checkudata(L, 1, PX5G_KEY_META);
	if (px5g->stat) {
		rsa_free(&px5g->rsa);
		px5g->stat = 0;
	}
	return 0;
}

static int px5g_rsa__tostring(lua_State *L) {
	px5g_rsa *px5g = luaL_checkudata(L, 1, PX5G_KEY_META);
	lua_pushfstring(L, "px5g context %p", px5g);
	return 1;
}

/* method table */
static const luaL_reg M[] = {
	{"asn1",				px5g_rsa_asn1},
	{"create_selfsigned",	px5g_rsa_create_selfsigned},
	{"__gc",				px5g_rsa__gc},
	{"__tostring",			px5g_rsa__tostring},
	{NULL,			NULL}
};

/* module table */
static const luaL_reg R[] = {
	{"genkey",		px5g_genkey},
	{NULL,			NULL}
};

int luaopen_px5g(lua_State *L) {
	/* register module */
	luaL_register(L, "px5g", R);

	/* Meta Table */
	luaL_newmetatable(L, PX5G_KEY_META);
	luaL_register(L, NULL, M);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	lua_setfield(L, -2, "meta_key");
	return 1;
}
