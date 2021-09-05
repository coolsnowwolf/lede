/*
 * LuCI Template - Lua binding
 *
 *   Copyright (C) 2009 Jo-Philipp Wich <jow@openwrt.org>
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

#include "template_lualib.h"

static int template_L_do_parse(lua_State *L, struct template_parser *parser, const char *chunkname)
{
	int lua_status, rv;

	if (!parser)
	{
		lua_pushnil(L);
		lua_pushinteger(L, errno);
		lua_pushstring(L, strerror(errno));
		return 3;
	}

	lua_status = lua_load(L, template_reader, parser, chunkname);

	if (lua_status == 0)
		rv = 1;
	else
		rv = template_error(L, parser);

	template_close(parser);

	return rv;
}

int template_L_parse(lua_State *L)
{
	const char *file = luaL_checkstring(L, 1);
	struct template_parser *parser = template_open(file);

	return template_L_do_parse(L, parser, file);
}

int template_L_parse_string(lua_State *L)
{
	size_t len;
	const char *str = luaL_checklstring(L, 1, &len);
	struct template_parser *parser = template_string(str, len);

	return template_L_do_parse(L, parser, "[string]");
}

int template_L_utf8(lua_State *L)
{
	size_t len = 0;
	const char *str = luaL_checklstring(L, 1, &len);
	char *res = utf8(str, len);

	if (res != NULL)
	{
		lua_pushstring(L, res);
		free(res);

		return 1;
	}

	return 0;
}

int template_L_pcdata(lua_State *L)
{
	size_t len = 0;
	const char *str = luaL_checklstring(L, 1, &len);
	char *res = pcdata(str, len);

	if (res != NULL)
	{
		lua_pushstring(L, res);
		free(res);

		return 1;
	}

	return 0;
}

int template_L_striptags(lua_State *L)
{
	size_t len = 0;
	const char *str = luaL_checklstring(L, 1, &len);
	char *res = striptags(str, len);

	if (res != NULL)
	{
		lua_pushstring(L, res);
		free(res);

		return 1;
	}

	return 0;
}

static int template_L_load_catalog(lua_State *L) {
	const char *lang = luaL_optstring(L, 1, "en");
	const char *dir  = luaL_optstring(L, 2, NULL);
	lua_pushboolean(L, !lmo_load_catalog(lang, dir));
	return 1;
}

static int template_L_close_catalog(lua_State *L) {
	const char *lang = luaL_optstring(L, 1, "en");
	lmo_close_catalog(lang);
	return 0;
}

static int template_L_change_catalog(lua_State *L) {
	const char *lang = luaL_optstring(L, 1, "en");
	lua_pushboolean(L, !lmo_change_catalog(lang));
	return 1;
}

static int template_L_translate(lua_State *L) {
	size_t len;
	char *tr;
	int trlen;
	const char *key = luaL_checklstring(L, 1, &len);

	switch (lmo_translate(key, len, &tr, &trlen))
	{
		case 0:
			lua_pushlstring(L, tr, trlen);
			return 1;

		case -1:
			return 0;
	}

	lua_pushnil(L);
	lua_pushstring(L, "no catalog loaded");
	return 2;
}

static int template_L_hash(lua_State *L) {
	size_t len;
	const char *key = luaL_checklstring(L, 1, &len);
	lua_pushinteger(L, sfh_hash(key, len));
	return 1;
}


/* module table */
static const luaL_reg R[] = {
	{ "parse",				template_L_parse },
	{ "parse_string",		template_L_parse_string },
	{ "utf8",				template_L_utf8 },
	{ "pcdata",				template_L_pcdata },
	{ "striptags",			template_L_striptags },
	{ "load_catalog",		template_L_load_catalog },
	{ "close_catalog",		template_L_close_catalog },
	{ "change_catalog",		template_L_change_catalog },
	{ "translate",			template_L_translate },
	{ "hash",				template_L_hash },
	{ NULL,					NULL }
};

LUALIB_API int luaopen_luci_template_parser(lua_State *L) {
	luaL_register(L, TEMPLATE_LUALIB_META, R);
	return 1;
}
