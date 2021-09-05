/*
Copyright 2015 Jo-Philipp Wich <jow@openwrt.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#define _GNU_SOURCE

#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <json-c/json.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define LUCI_JSONC "luci.jsonc"
#define LUCI_JSONC_PARSER "luci.jsonc.parser"

struct json_state {
	struct json_object *obj;
	struct json_tokener *tok;
	enum json_tokener_error err;
};

static void _json_to_lua(lua_State *L, struct json_object *obj);
static struct json_object * _lua_to_json(lua_State *L, int index);

static int json_new(lua_State *L)
{
	struct json_state *s;
	struct json_tokener *tok = json_tokener_new();

	if (!tok)
		return 0;

	s = lua_newuserdata(L, sizeof(*s));

	if (!s)
	{
		json_tokener_free(tok);
		return 0;
	}

	s->tok = tok;
	s->obj = NULL;
	s->err = json_tokener_continue;

	luaL_getmetatable(L, LUCI_JSONC_PARSER);
	lua_setmetatable(L, -2);

	return 1;
}

static int json_parse(lua_State *L)
{
	size_t len;
	const char *json = luaL_checklstring(L, 1, &len);
	struct json_state s = {
		.tok = json_tokener_new()
	};

	if (!s.tok)
		return 0;

	s.obj = json_tokener_parse_ex(s.tok, json, len);
	s.err = json_tokener_get_error(s.tok);

	if (s.obj)
	{
		_json_to_lua(L, s.obj);
		json_object_put(s.obj);
	}
	else
	{
		lua_pushnil(L);
	}

	if (s.err == json_tokener_continue)
		s.err = json_tokener_error_parse_eof;

	if (s.err)
		lua_pushstring(L, json_tokener_error_desc(s.err));

	json_tokener_free(s.tok);
	return (1 + !!s.err);
}

static int json_stringify(lua_State *L)
{
	struct json_object *obj = _lua_to_json(L, 1);
	bool pretty = lua_toboolean(L, 2);
	int flags = 0;

	if (pretty)
		flags |= JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED;

	lua_pushstring(L, json_object_to_json_string_ext(obj, flags));
	json_object_put(obj);
	return 1;
}


static int json_parse_chunk(lua_State *L)
{
	size_t len;
	struct json_state *s = luaL_checkudata(L, 1, LUCI_JSONC_PARSER);
	const char *chunk = luaL_checklstring(L, 2, &len);

	s->obj = json_tokener_parse_ex(s->tok, chunk, len);
    s->err = json_tokener_get_error(s->tok);

	if (!s->err)
	{
		lua_pushboolean(L, true);
		return 1;
	}
	else if (s->err == json_tokener_continue)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushnil(L);
	lua_pushstring(L, json_tokener_error_desc(s->err));
	return 2;
}

static void _json_to_lua(lua_State *L, struct json_object *obj)
{
	int64_t v;
	int n;

	switch (json_object_get_type(obj))
	{
	case json_type_object:
		lua_newtable(L);
		json_object_object_foreach(obj, key, val)
		{
			_json_to_lua(L, val);
			lua_setfield(L, -2, key);
		}
		break;

	case json_type_array:
		lua_newtable(L);
		for (n = 0; n < json_object_array_length(obj); n++)
		{
			_json_to_lua(L, json_object_array_get_idx(obj, n));
			lua_rawseti(L, -2, n + 1);
		}
		break;

	case json_type_boolean:
		lua_pushboolean(L, json_object_get_boolean(obj));
		break;

	case json_type_int:
		v = json_object_get_int64(obj);
		if (sizeof(lua_Integer) > sizeof(int32_t) ||
		    (v >= INT32_MIN && v <= INT32_MAX))
			lua_pushinteger(L, (lua_Integer)v);
		else
			lua_pushnumber(L, (lua_Number)v);
		break;

	case json_type_double:
		lua_pushnumber(L, json_object_get_double(obj));
		break;

	case json_type_string:
		lua_pushstring(L, json_object_get_string(obj));
		break;

	case json_type_null:
		lua_pushnil(L);
		break;
	}
}

static int json_parse_get(lua_State *L)
{
	struct json_state *s = luaL_checkudata(L, 1, LUCI_JSONC_PARSER);

	if (!s->obj || s->err)
		lua_pushnil(L);
	else
		_json_to_lua(L, s->obj);

	return 1;
}

static int _lua_test_array(lua_State *L, int index)
{
	int max = 0;
	lua_Number idx;

	lua_pushnil(L);

	/* check for non-integer keys */
	while (lua_next(L, index))
	{
		if (lua_type(L, -2) != LUA_TNUMBER)
			goto out;

		idx = lua_tonumber(L, -2);

		if (idx != (lua_Number)(lua_Integer)idx)
			goto out;

		if (idx <= 0)
			goto out;

		if (idx > max)
			max = idx;

		lua_pop(L, 1);
		continue;

out:
		lua_pop(L, 2);
		return -1;
	}

	/* check for holes */
	//for (i = 1; i <= max; i++)
	//{
	//	lua_rawgeti(L, index, i);
	//
	//	if (lua_isnil(L, -1))
	//	{
	//		lua_pop(L, 1);
	//		return 0;
	//	}
	//
	//	lua_pop(L, 1);
	//}

	return max;
}

static struct json_object * _lua_to_json(lua_State *L, int index)
{
	lua_Number nd, ni;
	struct json_object *obj;
	const char *key;
	int i, max;

	switch (lua_type(L, index))
	{
	case LUA_TTABLE:
		max = _lua_test_array(L, index);

		if (max >= 0)
		{
			obj = json_object_new_array();

			if (!obj)
				return NULL;

			for (i = 1; i <= max; i++)
			{
				lua_rawgeti(L, index, i);

				json_object_array_put_idx(obj, i - 1,
				                          _lua_to_json(L, lua_gettop(L)));

				lua_pop(L, 1);
			}

			return obj;
		}

		obj = json_object_new_object();

		if (!obj)
			return NULL;

		lua_pushnil(L);

		while (lua_next(L, index))
		{
			lua_pushvalue(L, -2);
			key = lua_tostring(L, -1);

			if (key)
				json_object_object_add(obj, key,
				                       _lua_to_json(L, lua_gettop(L) - 1));

			lua_pop(L, 2);
		}

		return obj;

	case LUA_TNIL:
		return NULL;

	case LUA_TBOOLEAN:
		return json_object_new_boolean(lua_toboolean(L, index));

	case LUA_TNUMBER:
		nd = lua_tonumber(L, index);
		if(nd >= INT64_MIN && nd <= INT64_MAX)
			return json_object_new_int64(nd);
		else
		return json_object_new_double(nd);

	case LUA_TSTRING:
		return json_object_new_string(lua_tostring(L, index));
	}

	return NULL;
}

static int json_parse_set(lua_State *L)
{
	struct json_state *s = luaL_checkudata(L, 1, LUCI_JSONC_PARSER);

	s->err = 0;
	s->obj = _lua_to_json(L, 2);

	return 0;
}

static int json_parse_sink_closure(lua_State *L)
{
	bool finished = lua_toboolean(L, lua_upvalueindex(2));
	if (lua_isnil(L, 1))
	{
		// no more data available
		if (finished)
		{
			// we were finished parsing
			lua_pushboolean(L, true);
			return 1;
		}
		else
		{
			lua_pushnil(L);
			lua_pushstring(L, "Incomplete JSON data");
			return 2;
		}
	}
	else
	{
		if (finished)
		{
			lua_pushnil(L);
			lua_pushstring(L, "Unexpected data after complete JSON object");
			return 2;
		}
		else
		{
			// luci.jsonc.parser.chunk()
			lua_pushcfunction(L, json_parse_chunk);
			// parser object from closure
			lua_pushvalue(L, lua_upvalueindex(1));
			// chunk
			lua_pushvalue(L, 1);
			lua_call(L, 2, 2);

			if (lua_isnil(L, -2))
			{
				// an error occurred, leave (nil, errmsg) on the stack and return it
				return 2;
			}
			else if (lua_toboolean(L, -2))
			{
				// finished reading, set finished=true and return nil to prevent further input
				lua_pop(L, 2);
				lua_pushboolean(L, true);
				lua_replace(L, lua_upvalueindex(2));
				lua_pushnil(L);
				return 1;
			}
			else
			{
				// not finished reading, return true
				lua_pop(L, 2);
				lua_pushboolean(L, true);
				return 1;
			}
		}
	}
}

static int json_parse_sink(lua_State *L)
{
	luaL_checkudata(L, 1, LUCI_JSONC_PARSER);
	lua_pushboolean(L, false);
	lua_pushcclosure(L, json_parse_sink_closure, 2);
	return 1;
}

static int json_tostring(lua_State *L)
{
	struct json_state *s = luaL_checkudata(L, 1, LUCI_JSONC_PARSER);
	bool pretty = lua_toboolean(L, 2);
	int flags = 0;

	if (pretty)
		flags |= JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED;

	lua_pushstring(L, json_object_to_json_string_ext(s->obj, flags));
	return 1;
}

static int json_gc(lua_State *L)
{
	struct json_state *s = luaL_checkudata(L, 1, LUCI_JSONC_PARSER);

	if (s->obj)
		json_object_put(s->obj);

	if (s->tok)
		json_tokener_free(s->tok);

	return 0;
}


static const luaL_reg jsonc_methods[] = {
	{ "new",			json_new          },
	{ "parse",			json_parse        },
	{ "stringify",		json_stringify    },

	{ }
};

static const luaL_reg jsonc_parser_methods[] = {
	{ "parse",			json_parse_chunk  },
	{ "get",			json_parse_get    },
	{ "set",			json_parse_set    },
	{ "sink",			json_parse_sink   },
	{ "stringify",		json_tostring     },

	{ "__gc",			json_gc           },
	{ "__tostring",		json_tostring     },

	{ }
};


int luaopen_luci_jsonc(lua_State *L)
{
	luaL_register(L, LUCI_JSONC, jsonc_methods);

	luaL_newmetatable(L, LUCI_JSONC_PARSER);
	luaL_register(L, NULL, jsonc_parser_methods);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);

	return 1;
}
