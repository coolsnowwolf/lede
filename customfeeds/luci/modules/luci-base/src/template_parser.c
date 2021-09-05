/*
 * LuCI Template - Parser implementation
 *
 *   Copyright (C) 2009-2012 Jo-Philipp Wich <jow@openwrt.org>
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

#include "template_parser.h"
#include "template_utils.h"
#include "template_lmo.h"


/* leading and trailing code for different types */
const char *gen_code[9][2] = {
	{ NULL,					NULL			},
	{ "write(\"",			"\")"			},
	{ NULL,					NULL			},
	{ "write(tostring(",	" or \"\"))"	},
	{ "include(\"",			"\")"			},
	{ "write(\"",			"\")"			},
	{ "write(\"",			"\")"			},
	{ NULL,					" "				},
	{ NULL,					NULL			},
};

/* Simple strstr() like function that takes len arguments for both haystack and needle. */
static char *strfind(char *haystack, int hslen, const char *needle, int ndlen)
{
	int match = 0;
	int i, j;

	for( i = 0; i < hslen; i++ )
	{
		if( haystack[i] == needle[0] )
		{
			match = ((ndlen == 1) || ((i + ndlen) <= hslen));

			for( j = 1; (j < ndlen) && ((i + j) < hslen); j++ )
			{
				if( haystack[i+j] != needle[j] )
				{
					match = 0;
					break;
				}
			}

			if( match )
				return &haystack[i];
		}
	}

	return NULL;
}

struct template_parser * template_open(const char *file)
{
	struct stat s;
	struct template_parser *parser;

	if (!(parser = malloc(sizeof(*parser))))
		goto err;

	memset(parser, 0, sizeof(*parser));
	parser->fd = -1;
	parser->file = file;

	if (stat(file, &s))
		goto err;

	if ((parser->fd = open(file, O_RDONLY)) < 0)
		goto err;

	parser->size = s.st_size;
	parser->data = mmap(NULL, parser->size, PROT_READ, MAP_PRIVATE,
						parser->fd, 0);

	if (parser->data != MAP_FAILED)
	{
		parser->off = parser->data;
		parser->cur_chunk.type = T_TYPE_INIT;
		parser->cur_chunk.s    = parser->data;
		parser->cur_chunk.e    = parser->data;

		return parser;
	}

err:
	template_close(parser);
	return NULL;
}

struct template_parser * template_string(const char *str, uint32_t len)
{
	struct template_parser *parser;

	if (!str) {
		errno = EINVAL;
		goto err;
	}

	if (!(parser = malloc(sizeof(*parser))))
		goto err;

	memset(parser, 0, sizeof(*parser));
	parser->fd = -1;

	parser->size = len;
	parser->data = (char*)str;

	parser->off = parser->data;
	parser->cur_chunk.type = T_TYPE_INIT;
	parser->cur_chunk.s    = parser->data;
	parser->cur_chunk.e    = parser->data;

	return parser;

err:
	template_close(parser);
	return NULL;
}

void template_close(struct template_parser *parser)
{
	if (!parser)
		return;

	if (parser->gc != NULL)
		free(parser->gc);

	/* if file is not set, we were parsing a string */
	if (parser->file) {
		if ((parser->data != NULL) && (parser->data != MAP_FAILED))
			munmap(parser->data, parser->size);

		if (parser->fd >= 0)
			close(parser->fd);
	}

	free(parser);
}

void template_text(struct template_parser *parser, const char *e)
{
	const char *s = parser->off;

	if (s < (parser->data + parser->size))
	{
		if (parser->strip_after)
		{
			while ((s <= e) && isspace(*s))
				s++;
		}

		parser->cur_chunk.type = T_TYPE_TEXT;
	}
	else
	{
		parser->cur_chunk.type = T_TYPE_EOF;
	}

	parser->cur_chunk.line = parser->line;
	parser->cur_chunk.s = s;
	parser->cur_chunk.e = e;
}

void template_code(struct template_parser *parser, const char *e)
{
	const char *s = parser->off;

	parser->strip_before = 0;
	parser->strip_after = 0;

	if (*s == '-')
	{
		parser->strip_before = 1;
		for (s++; (s <= e) && (*s == ' ' || *s == '\t'); s++);
	}

	if (*(e-1) == '-')
	{
		parser->strip_after = 1;
		for (e--; (e >= s) && (*e == ' ' || *e == '\t'); e--);
	}

	switch (*s)
	{
		/* comment */
		case '#':
			s++;
			parser->cur_chunk.type = T_TYPE_COMMENT;
			break;

		/* include */
		case '+':
			s++;
			parser->cur_chunk.type = T_TYPE_INCLUDE;
			break;

		/* translate */
		case ':':
			s++;
			parser->cur_chunk.type = T_TYPE_I18N;
			break;

		/* translate raw */
		case '_':
			s++;
			parser->cur_chunk.type = T_TYPE_I18N_RAW;
			break;

		/* expr */
		case '=':
			s++;
			parser->cur_chunk.type = T_TYPE_EXPR;
			break;

		/* code */
		default:
			parser->cur_chunk.type = T_TYPE_CODE;
			break;
	}

	parser->cur_chunk.line = parser->line;
	parser->cur_chunk.s = s;
	parser->cur_chunk.e = e;
}

static const char *
template_format_chunk(struct template_parser *parser, size_t *sz)
{
	const char *s, *p;
	const char *head, *tail;
	struct template_chunk *c = &parser->prv_chunk;
	struct template_buffer *buf;

	*sz = 0;
	s = parser->gc = NULL;

	if (parser->strip_before && c->type == T_TYPE_TEXT)
	{
		while ((c->e > c->s) && isspace(*(c->e - 1)))
			c->e--;
	}

	/* empty chunk */
	if (c->s == c->e)
	{
		if (c->type == T_TYPE_EOF)
		{
			*sz = 0;
			s = NULL;
		}
		else
		{
			*sz = 1;
			s = " ";
		}
	}

	/* format chunk */
	else if ((buf = buf_init(c->e - c->s)) != NULL)
	{
		if ((head = gen_code[c->type][0]) != NULL)
			buf_append(buf, head, strlen(head));

		switch (c->type)
		{
			case T_TYPE_TEXT:
				luastr_escape(buf, c->s, c->e - c->s, 0);
				break;

			case T_TYPE_EXPR:
				buf_append(buf, c->s, c->e - c->s);
				for (p = c->s; p < c->e; p++)
					parser->line += (*p == '\n');
				break;

			case T_TYPE_INCLUDE:
				luastr_escape(buf, c->s, c->e - c->s, 0);
				break;

			case T_TYPE_I18N:
				luastr_translate(buf, c->s, c->e - c->s, 1);
				break;

			case T_TYPE_I18N_RAW:
				luastr_translate(buf, c->s, c->e - c->s, 0);
				break;

			case T_TYPE_CODE:
				buf_append(buf, c->s, c->e - c->s);
				for (p = c->s; p < c->e; p++)
					parser->line += (*p == '\n');
				break;
		}

		if ((tail = gen_code[c->type][1]) != NULL)
			buf_append(buf, tail, strlen(tail));

		*sz = buf_length(buf);
		s = parser->gc = buf_destroy(buf);

		if (!*sz)
		{
			*sz = 1;
			s = " ";
		}
	}

	return s;
}

const char *template_reader(lua_State *L, void *ud, size_t *sz)
{
	struct template_parser *parser = ud;
	int rem = parser->size - (parser->off - parser->data);
	char *tag;

	parser->prv_chunk = parser->cur_chunk;

	/* free previous string */
	if (parser->gc)
	{
		free(parser->gc);
		parser->gc = NULL;
	}

	/* before tag */
	if (!parser->in_expr)
	{
		if ((tag = strfind(parser->off, rem, "<%", 2)) != NULL)
		{
			template_text(parser, tag);
			parser->off = tag + 2;
			parser->in_expr = 1;
		}
		else
		{
			template_text(parser, parser->data + parser->size);
			parser->off = parser->data + parser->size;
		}
	}

	/* inside tag */
	else
	{
		if ((tag = strfind(parser->off, rem, "%>", 2)) != NULL)
		{
			template_code(parser, tag);
			parser->off = tag + 2;
			parser->in_expr = 0;
		}
		else
		{
			/* unexpected EOF */
			template_code(parser, parser->data + parser->size);

			*sz = 1;
			return "\033";
		}
	}

	return template_format_chunk(parser, sz);
}

int template_error(lua_State *L, struct template_parser *parser)
{
	const char *err = luaL_checkstring(L, -1);
	const char *off = parser->prv_chunk.s;
	const char *ptr;
	char msg[1024];
	int line = 0;
	int chunkline = 0;

	if ((ptr = strfind((char *)err, strlen(err), "]:", 2)) != NULL)
	{
		chunkline = atoi(ptr + 2) - parser->prv_chunk.line;

		while (*ptr)
		{
			if (*ptr++ == ' ')
			{
				err = ptr;
				break;
			}
		}
	}

	if (strfind((char *)err, strlen(err), "'char(27)'", 10) != NULL)
	{
		off = parser->data + parser->size;
		err = "'%>' expected before end of file";
		chunkline = 0;
	}

	for (ptr = parser->data; ptr < off; ptr++)
		if (*ptr == '\n')
			line++;

	snprintf(msg, sizeof(msg), "Syntax error in %s:%d: %s",
			 parser->file ? parser->file : "[string]", line + chunkline, err ? err : "(unknown error)");

	lua_pushnil(L);
	lua_pushinteger(L, line + chunkline);
	lua_pushstring(L, msg);

	return 3;
}
