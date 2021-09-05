/*
 * LuCI Template - Parser header
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

#ifndef _TEMPLATE_PARSER_H_
#define _TEMPLATE_PARSER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


/* code types */
#define T_TYPE_INIT			0
#define T_TYPE_TEXT			1
#define T_TYPE_COMMENT		2
#define T_TYPE_EXPR			3
#define T_TYPE_INCLUDE 		4
#define T_TYPE_I18N			5
#define T_TYPE_I18N_RAW		6
#define T_TYPE_CODE			7
#define T_TYPE_EOF			8


struct template_chunk {
	const char *s;
	const char *e;
	int type;
	int line;
};

/* parser state */
struct template_parser {
	int fd;
	uint32_t size;
	char *data;
	char *off;
	char *gc;
	int line;
	int in_expr;
	int strip_before;
	int strip_after;
	struct template_chunk prv_chunk;
	struct template_chunk cur_chunk;
	const char *file;
};

struct template_parser * template_open(const char *file);
struct template_parser * template_string(const char *str, uint32_t len);
void template_close(struct template_parser *parser);

const char *template_reader(lua_State *L, void *ud, size_t *sz);
int template_error(lua_State *L, struct template_parser *parser);

#endif
