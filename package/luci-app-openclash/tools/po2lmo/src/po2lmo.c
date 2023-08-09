/*
 * lmo - Lua Machine Objects - PO to LMO conversion tool
 *
 *   Copyright (C) 2009-2012 Jo-Philipp Wich <xm@subsignal.org>
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

#include "template_lmo.h"

static void die(const char *msg)
{
	fprintf(stderr, "Error: %s\n", msg);
	exit(1);
}

static void usage(const char *name)
{
	fprintf(stderr, "Usage: %s input.po output.lmo\n", name);
	exit(1);
}

static void print(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	if( fwrite(ptr, size, nmemb, stream) == 0 )
		die("Failed to write stdout");
}

static int extract_string(const char *src, char *dest, int len)
{
	int pos = 0;
	int esc = 0;
	int off = -1;

	for( pos = 0; (pos < strlen(src)) && (pos < len); pos++ )
	{
		if( (off == -1) && (src[pos] == '"') )
		{
			off = pos + 1;
		}
		else if( off >= 0 )
		{
			if( esc == 1 )
			{
				switch (src[pos])
				{
				case '"':
				case '\\':
					off++;
					break;
				}
				dest[pos-off] = src[pos];
				esc = 0;
			}
			else if( src[pos] == '\\' )
			{
				dest[pos-off] = src[pos];
				esc = 1;
			}
			else if( src[pos] != '"' )
			{
				dest[pos-off] = src[pos];
			}
			else
			{
				dest[pos-off] = '\0';
				break;
			}
		}
	}

	return (off > -1) ? strlen(dest) : -1;
}

static int cmp_index(const void *a, const void *b)
{
	uint32_t x = ((const lmo_entry_t *)a)->key_id;
	uint32_t y = ((const lmo_entry_t *)b)->key_id;

	if (x < y)
		return -1;
	else if (x > y)
		return 1;

	return 0;
}

static void print_uint32(uint32_t x, FILE *out)
{
	uint32_t y = htonl(x);
	print(&y, sizeof(uint32_t), 1, out);
}

static void print_index(void *array, int n, FILE *out)
{
	lmo_entry_t *e;

	qsort(array, n, sizeof(*e), cmp_index);

	for (e = array; n > 0; n--, e++)
	{
		print_uint32(e->key_id, out);
		print_uint32(e->val_id, out);
		print_uint32(e->offset, out);
		print_uint32(e->length, out);
	}
}

int main(int argc, char *argv[])
{
	char line[4096];
	char key[4096];
	char val[4096];
	char tmp[4096];
	int state  = 0;
	int offset = 0;
	int length = 0;
	int n_entries = 0;
	void *array = NULL;
	lmo_entry_t *entry = NULL;
	uint32_t key_id, val_id;

	FILE *in;
	FILE *out;

	if( (argc != 3) || ((in = fopen(argv[1], "r")) == NULL) || ((out = fopen(argv[2], "w")) == NULL) )
		usage(argv[0]);

	memset(line, 0, sizeof(key));
	memset(key, 0, sizeof(val));
	memset(val, 0, sizeof(val));

	while( (NULL != fgets(line, sizeof(line), in)) || (state >= 2 && feof(in)) )
	{
		if( state == 0 && strstr(line, "msgid \"") == line )
		{
			switch(extract_string(line, key, sizeof(key)))
			{
				case -1:
					die("Syntax error in msgid");
				case 0:
					state = 1;
					break;
				default:
					state = 2;
			}
		}
		else if( state == 1 || state == 2 )
		{
			if( strstr(line, "msgstr \"") == line || state == 2 )
			{
				switch(extract_string(line, val, sizeof(val)))
				{
					case -1:
						state = 4;
						break;
					default:
						state = 3;
				}
			}
			else
			{
				switch(extract_string(line, tmp, sizeof(tmp)))
				{
					case -1:
						state = 2;
						break;
					default:
						strcat(key, tmp);
				}
			}
		}
		else if( state == 3 )
		{
			switch(extract_string(line, tmp, sizeof(tmp)))
			{
				case -1:
					state = 4;
					break;
				default:
					strcat(val, tmp);
			}
		}

		if( state == 4 )
		{
			if( strlen(key) > 0 && strlen(val) > 0 )
			{
				key_id = sfh_hash(key, strlen(key));
				val_id = sfh_hash(val, strlen(val));

				if( key_id != val_id )
				{
					n_entries++;
					array = realloc(array, n_entries * sizeof(lmo_entry_t));
					entry = (lmo_entry_t *)array + n_entries - 1;

					if (!array)
						die("Out of memory");

					entry->key_id = key_id;
					entry->val_id = val_id;
					entry->offset = offset;
					entry->length = strlen(val);

					length = strlen(val) + ((4 - (strlen(val) % 4)) % 4);

					print(val, length, 1, out);
					offset += length;
				}
			}

			state = 0;
			memset(key, 0, sizeof(key));
			memset(val, 0, sizeof(val));
		}

		memset(line, 0, sizeof(line));
	}

	print_index(array, n_entries, out);

	if( offset > 0 )
	{
		print_uint32(offset, out);
		fsync(fileno(out));
		fclose(out);
	}
	else
	{
		fclose(out);
		unlink(argv[2]);
	}

	fclose(in);
	return(0);
}
