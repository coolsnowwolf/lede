/*
 * LuCI Template - Utility functions
 *
 *   Copyright (C) 2010 Jo-Philipp Wich <jow@openwrt.org>
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

#include "template_utils.h"
#include "template_lmo.h"

/* initialize a buffer object */
struct template_buffer * buf_init(int size)
{
	struct template_buffer *buf;

	if (size <= 0)
		size = 1024;

	buf = (struct template_buffer *)malloc(sizeof(struct template_buffer));

	if (buf != NULL)
	{
		buf->fill = 0;
		buf->size = size;
		buf->data = malloc(buf->size);

		if (buf->data != NULL)
		{
			buf->dptr = buf->data;
			buf->data[0] = 0;

			return buf;
		}

		free(buf);
	}

	return NULL;
}

/* grow buffer */
int buf_grow(struct template_buffer *buf, int size)
{
	unsigned int off = (buf->dptr - buf->data);
	char *data;

	if (size <= 0)
		size = 1024;

	data = realloc(buf->data, buf->size + size);

	if (data != NULL)
	{
		buf->data  = data;
		buf->dptr  = data + off;
		buf->size += size;

		return buf->size;
	}

	return 0;
}

/* put one char into buffer object */
int buf_putchar(struct template_buffer *buf, char c)
{
	if( ((buf->fill + 1) >= buf->size) && !buf_grow(buf, 0) )
		return 0;

	*(buf->dptr++) = c;
	*(buf->dptr) = 0;

	buf->fill++;
	return 1;
}

/* append data to buffer */
int buf_append(struct template_buffer *buf, const char *s, int len)
{
	if ((buf->fill + len + 1) >= buf->size)
	{
		if (!buf_grow(buf, len + 1))
			return 0;
	}

	memcpy(buf->dptr, s, len);
	buf->fill += len;
	buf->dptr += len;

	*(buf->dptr) = 0;

	return len;
}

/* read buffer length */
int buf_length(struct template_buffer *buf)
{
	return buf->fill;
}

/* destroy buffer object and return pointer to data */
char * buf_destroy(struct template_buffer *buf)
{
	char *data = buf->data;

	free(buf);
	return data;
}


/* calculate the number of expected continuation chars */
static inline int mb_num_chars(unsigned char c)
{
	if ((c & 0xE0) == 0xC0)
		return 2;
	else if ((c & 0xF0) == 0xE0)
		return 3;
	else if ((c & 0xF8) == 0xF0)
		return 4;
	else if ((c & 0xFC) == 0xF8)
		return 5;
	else if ((c & 0xFE) == 0xFC)
		return 6;

	return 1;
}

/* test whether the given byte is a valid continuation char */
static inline int mb_is_cont(unsigned char c)
{
	return ((c >= 0x80) && (c <= 0xBF));
}

/* test whether the byte sequence at the given pointer with the given
 * length is the shortest possible representation of the code point */
static inline int mb_is_shortest(unsigned char *s, int n)
{
	switch (n)
	{
		case 2:
			/* 1100000x (10xxxxxx) */
			return !(((*s >> 1) == 0x60) &&
					 ((*(s+1) >> 6) == 0x02));

		case 3:
			/* 11100000 100xxxxx (10xxxxxx) */
			return !((*s == 0xE0) &&
					 ((*(s+1) >> 5) == 0x04) &&
					 ((*(s+2) >> 6) == 0x02));

		case 4:
			/* 11110000 1000xxxx (10xxxxxx 10xxxxxx) */
			return !((*s == 0xF0) &&
					 ((*(s+1) >> 4) == 0x08) &&
					 ((*(s+2) >> 6) == 0x02) &&
					 ((*(s+3) >> 6) == 0x02));

		case 5:
			/* 11111000 10000xxx (10xxxxxx 10xxxxxx 10xxxxxx) */
			return !((*s == 0xF8) &&
					 ((*(s+1) >> 3) == 0x10) &&
					 ((*(s+2) >> 6) == 0x02) &&
					 ((*(s+3) >> 6) == 0x02) &&
					 ((*(s+4) >> 6) == 0x02));

		case 6:
			/* 11111100 100000xx (10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx) */
			return !((*s == 0xF8) &&
					 ((*(s+1) >> 2) == 0x20) &&
					 ((*(s+2) >> 6) == 0x02) &&
					 ((*(s+3) >> 6) == 0x02) &&
					 ((*(s+4) >> 6) == 0x02) &&
					 ((*(s+5) >> 6) == 0x02));
	}

	return 1;
}

/* test whether the byte sequence at the given pointer with the given
 * length is an UTF-16 surrogate */
static inline int mb_is_surrogate(unsigned char *s, int n)
{
	return ((n == 3) && (*s == 0xED) && (*(s+1) >= 0xA0) && (*(s+1) <= 0xBF));
}

/* test whether the byte sequence at the given pointer with the given
 * length is an illegal UTF-8 code point */
static inline int mb_is_illegal(unsigned char *s, int n)
{
	return ((n == 3) && (*s == 0xEF) && (*(s+1) == 0xBF) &&
			(*(s+2) >= 0xBE) && (*(s+2) <= 0xBF));
}


/* scan given source string, validate UTF-8 sequence and store result
 * in given buffer object */
static int _validate_utf8(unsigned char **s, int l, struct template_buffer *buf)
{
	unsigned char *ptr = *s;
	unsigned int o = 0, v, n;

	/* ascii byte without null */
	if ((*(ptr+0) >= 0x01) && (*(ptr+0) <= 0x7F))
	{
		if (!buf_putchar(buf, *ptr++))
			return 0;

		o = 1;
	}

	/* multi byte sequence */
	else if ((n = mb_num_chars(*ptr)) > 1)
	{
		/* count valid chars */
		for (v = 1; (v <= n) && ((o+v) < l) && mb_is_cont(*(ptr+v)); v++);

		switch (n)
		{
			case 6:
			case 5:
				/* five and six byte sequences are always invalid */
				if (!buf_putchar(buf, '?'))
					return 0;

				break;

			default:
				/* if the number of valid continuation bytes matches the
				 * expected number and if the sequence is legal, copy
				 * the bytes to the destination buffer */
				if ((v == n) && mb_is_shortest(ptr, n) &&
					!mb_is_surrogate(ptr, n) && !mb_is_illegal(ptr, n))
				{
					/* copy sequence */
					if (!buf_append(buf, (char *)ptr, n))
						return 0;
				}

				/* the found sequence is illegal, skip it */
				else
				{
					/* invalid sequence */
					if (!buf_putchar(buf, '?'))
						return 0;
				}

				break;
		}

		/* advance beyound the last found valid continuation char */
		o = v;
		ptr += v;
	}

	/* invalid byte (0x00) */
	else
	{
		if (!buf_putchar(buf, '?')) /* or 0xEF, 0xBF, 0xBD */
			return 0;

		o = 1;
		ptr++;
	}

	*s = ptr;
	return o;
}

/* sanitize given string and replace all invalid UTF-8 sequences with "?" */
char * utf8(const char *s, unsigned int l)
{
	struct template_buffer *buf = buf_init(l);
	unsigned char *ptr = (unsigned char *)s;
	unsigned int v, o;

	if (!buf)
		return NULL;

	for (o = 0; o < l; o++)
	{
		/* ascii char */
		if ((*ptr >= 0x01) && (*ptr <= 0x7F))
		{
			if (!buf_putchar(buf, (char)*ptr++))
				break;
		}

		/* invalid byte or multi byte sequence */
		else
		{
			if (!(v = _validate_utf8(&ptr, l - o, buf)))
				break;

			o += (v - 1);
		}
	}

	return buf_destroy(buf);
}

/* Sanitize given string and strip all invalid XML bytes
 * Validate UTF-8 sequences
 * Escape XML control chars */
char * pcdata(const char *s, unsigned int l)
{
	struct template_buffer *buf = buf_init(l);
	unsigned char *ptr = (unsigned char *)s;
	unsigned int o, v;
	char esq[8];
	int esl;

	if (!buf)
		return NULL;

	for (o = 0; o < l; o++)
	{
		/* Invalid XML bytes */
		if (((*ptr >= 0x00) && (*ptr <= 0x08)) ||
		    ((*ptr >= 0x0B) && (*ptr <= 0x0C)) ||
		    ((*ptr >= 0x0E) && (*ptr <= 0x1F)) ||
		    (*ptr == 0x7F))
		{
			ptr++;
		}

		/* Escapes */
		else if ((*ptr == 0x26) ||
		         (*ptr == 0x27) ||
		         (*ptr == 0x22) ||
		         (*ptr == 0x3C) ||
		         (*ptr == 0x3E))
		{
			esl = snprintf(esq, sizeof(esq), "&#%i;", *ptr);

			if (!buf_append(buf, esq, esl))
				break;

			ptr++;
		}

		/* ascii char */
		else if (*ptr <= 0x7F)
		{
			buf_putchar(buf, (char)*ptr++);
		}

		/* multi byte sequence */
		else
		{
			if (!(v = _validate_utf8(&ptr, l - o, buf)))
				break;

			o += (v - 1);
		}
	}

	return buf_destroy(buf);
}

char * striptags(const char *s, unsigned int l)
{
	struct template_buffer *buf = buf_init(l);
	unsigned char *ptr = (unsigned char *)s;
	unsigned char *end = ptr + l;
	unsigned char *tag;
	unsigned char prev;
	char esq[8];
	int esl;

	for (prev = ' '; ptr < end; ptr++)
	{
		if ((*ptr == '<') && ((ptr + 2) < end) &&
			((*(ptr + 1) == '/') || isalpha(*(ptr + 1))))
		{
			for (tag = ptr; tag < end; tag++)
			{
				if (*tag == '>')
				{
					if (!isspace(prev))
						buf_putchar(buf, ' ');

					ptr = tag;
					prev = ' ';
					break;
				}
			}
		}
		else if (isspace(*ptr))
		{
			if (!isspace(prev))
				buf_putchar(buf, *ptr);

			prev = *ptr;
		}
		else
		{
			switch(*ptr)
			{
				case '"':
				case '\'':
				case '<':
				case '>':
				case '&':
					esl = snprintf(esq, sizeof(esq), "&#%i;", *ptr);
					buf_append(buf, esq, esl);
					break;

				default:
					buf_putchar(buf, *ptr);
					break;
			}

			prev = *ptr;
		}
	}

	return buf_destroy(buf);
}

void luastr_escape(struct template_buffer *out, const char *s, unsigned int l,
				   int escape_xml)
{
	int esl;
	char esq[8];
	char *ptr;

	for (ptr = (char *)s; ptr < (s + l); ptr++)
	{
		switch (*ptr)
		{
		case '\\':
			buf_append(out, "\\\\", 2);
			break;

		case '"':
			if (escape_xml)
				buf_append(out, "&#34;", 5);
			else
				buf_append(out, "\\\"", 2);
			break;

		case '\n':
			buf_append(out, "\\n", 2);
			break;

		case '\'':
		case '&':
		case '<':
		case '>':
			if (escape_xml)
			{
				esl = snprintf(esq, sizeof(esq), "&#%i;", *ptr);
				buf_append(out, esq, esl);
				break;
			}

		default:
			buf_putchar(out, *ptr);
		}
	}
}

void luastr_translate(struct template_buffer *out, const char *s, unsigned int l,
					  int escape_xml)
{
	char *tr;
	int trlen;

	if (!lmo_translate(s, l, &tr, &trlen))
		luastr_escape(out, tr, trlen, escape_xml);
	else
		luastr_escape(out, s, l, escape_xml);
}
