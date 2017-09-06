#include <iconv.h>
#include <errno.h>
#include <wchar.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <limits.h>

#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

/* builtin charmaps */
#include "charmaps.h"

/* only 0-7 are valid as dest charset */
#define UTF_16BE    000
#define UTF_16LE    001
#define UTF_32BE    002
#define UTF_32LE    003
#define WCHAR_T     004
#define UTF_8       005
#define US_ASCII    006
#define LATIN_1     007

/* additional charsets with algorithmic conversion */
#define LATIN_9     010
#define TIS_620     011
#define JIS_0201    012

/* some programs like php need this */
int _libiconv_version = _LIBICONV_VERSION;

/* these must match the constants above */
static const unsigned char charsets[] =
	"\005" "UTF-8"      "\0"
	"\004" "WCHAR_T"    "\0"
	"\000" "UTF-16BE"   "\0"
	"\001" "UTF-16LE"   "\0"
	"\002" "UTF-32BE"   "\0"
	"\003" "UTF-32LE"   "\0"
	"\006" "ASCII"      "\0"
	"\006" "US-ASCII"   "\0"
	"\006" "ISO646-US"  "\0"
	"\006" "ISO_646.IRV:1991"  "\0"
	"\006" "ISO-IR-6"   "\0"
	"\006" "ANSI_X3.4-1968"    "\0"
	"\006" "ANSI_X3.4-1986"    "\0"
	"\006" "CP367"      "\0"
	"\006" "IBM367"     "\0"
	"\006" "US"         "\0"
	"\006" "CSASCII"    "\0"
	"\007" "ISO-8859-1" "\0"
	"\007" "LATIN1"     "\0"
	"\010" "ISO-8859-15""\0"
	"\010" "LATIN9"     "\0"
	"\011" "ISO-8859-11""\0"
	"\011" "TIS-620"    "\0"
	"\012" "JIS-0201"   "\0"
	"\377";

/* separate identifiers for sbcs/dbcs/etc map type */
#define UCS2_8BIT   000
#define UCS3_8BIT   001
#define EUC         002
#define EUC_TW      003
#define SHIFT_JIS   004
#define BIG5        005
#define GBK         006

/* FIXME: these are not implemented yet
// EUC:   A1-FE A1-FE
// GBK:   81-FE 40-7E,80-FE
// Big5:  A1-FE 40-7E,A1-FE
*/

static const unsigned short maplen[] = {
	[UCS2_8BIT] = 4+ 2* 128,
	[UCS3_8BIT] = 4+ 3* 128,
	[EUC]       = 4+ 2* 94*94,
	[SHIFT_JIS] = 4+ 2* 94*94,
	[BIG5]      = 4+ 2* 94*157,
	[GBK]       = 4+ 2* 126*190,
	[EUC_TW]    = 4+ 2* 2*94*94,
};

static int find_charmap(const char *name)
{
	int i;
	for (i = 0; i < (sizeof(charmaps) / sizeof(charmaps[0])); i++)
		if (!strcasecmp(charmaps[i].name, name))
			return i;
	return -1;
}

static int find_charset(const char *name)
{
	const unsigned char *s;
	for (s=charsets; *s<0xff && strcasecmp(s+1, name); s+=strlen(s)+1);
	return *s;
}

iconv_t iconv_open(const char *to, const char *from)
{
	unsigned f, t;
	int m;

	if ((t = find_charset(to)) > 8)
		return -1;

	if ((f = find_charset(from)) < 255)
		return 0 | (t<<1) | (f<<8);

	if ((m = find_charmap(from)) > -1)
		return 1 | (t<<1) | (m<<8);

	return -1;
}

int iconv_close(iconv_t cd)
{
	return 0;
}

static inline wchar_t get_16(const unsigned char *s, int endian)
{
	endian &= 1;
	return s[endian]<<8 | s[endian^1];
}

static inline void put_16(unsigned char *s, wchar_t c, int endian)
{
	endian &= 1;
	s[endian] = c>>8;
	s[endian^1] = c;
}

static inline int utf8enc_wchar(char *outb, wchar_t c)
{
	if (c <= 0x7F) {
		*outb = c;
		return 1;
	}
	else if (c <= 0x7FF) {
		*outb++ = ((c >>  6) & 0x1F) | 0xC0;
		*outb++ = ( c        & 0x3F) | 0x80;
		return 2;
	}
	else if (c <= 0xFFFF) {
		*outb++ = ((c >> 12) & 0x0F) | 0xE0;
		*outb++ = ((c >>  6) & 0x3F) | 0x80;
		*outb++ = ( c        & 0x3F) | 0x80;
		return 3;
	}
	else if (c <= 0x10FFFF) {
		*outb++ = ((c >> 18) & 0x07) | 0xF0;
		*outb++ = ((c >> 12) & 0x3F) | 0x80;
		*outb++ = ((c >>  6) & 0x3F) | 0x80;
		*outb++ = ( c        & 0x3F) | 0x80;
		return 4;
	}
	else {
		*outb++ = '?';
		return 1;
	}
}

static inline int utf8seq_is_overlong(char *s, int n)
{
	switch (n)
	{
	case 2:
		/* 1100000x (10xxxxxx) */
		return (((*s >> 1) == 0x60) &&
				((*(s+1) >> 6) == 0x02));

	case 3:
		/* 11100000 100xxxxx (10xxxxxx) */
		return ((*s == 0xE0) &&
				((*(s+1) >> 5) == 0x04) &&
				((*(s+2) >> 6) == 0x02));

	case 4:
		/* 11110000 1000xxxx (10xxxxxx 10xxxxxx) */
		return ((*s == 0xF0) &&
				((*(s+1) >> 4) == 0x08) &&
				((*(s+2) >> 6) == 0x02) &&
				((*(s+3) >> 6) == 0x02));
	}

	return 0;
}

static inline int utf8seq_is_surrogate(char *s, int n)
{
	return ((n == 3) && (*s == 0xED) && (*(s+1) >= 0xA0) && (*(s+1) <= 0xBF));
}

static inline int utf8seq_is_illegal(char *s, int n)
{
	return ((n == 3) && (*s == 0xEF) && (*(s+1) == 0xBF) &&
	        (*(s+2) >= 0xBE) && (*(s+2) <= 0xBF));
}

static inline int utf8dec_wchar(wchar_t *c, unsigned char *in, size_t inb)
{
	int i;
	int n = -1;

	/* trivial char */
	if (*in <= 0x7F) {
		*c = *in;
		return 1;
	}

	/* find utf8 sequence length */
	if      ((*in & 0xE0) == 0xC0) n = 2;
	else if ((*in & 0xF0) == 0xE0) n = 3;
	else if ((*in & 0xF8) == 0xF0) n = 4;
	else if ((*in & 0xFC) == 0xF8) n = 5;
	else if ((*in & 0xFE) == 0xFC) n = 6;

	/* starved? */
	if (n > inb)
		return -2;

	/* decode ... */
	if (n > 1 && n < 5) {
		/* reject invalid sequences */
		if (utf8seq_is_overlong(in, n) ||
			utf8seq_is_surrogate(in, n) ||
			utf8seq_is_illegal(in, n))
			return -1;

		/* decode ... */
		*c = (char)(*in++ & (0x7F >> n));

		for (i = 1; i < n; i++) {
			/* illegal continuation byte */
			if (*in < 0x80 || *in > 0xBF)
				return -1;

			*c = (*c << 6) | (*in++ & 0x3F);
		}

		return n;
	}

	/* unmapped sequence (> 4) */
	return -1;
}

static inline wchar_t latin9_translit(wchar_t c)
{
	/* a number of trivial iso-8859-15 <> utf-8 transliterations */
	switch (c) {
	case 0x20AC: return 0xA4; /* Euro */
	case 0x0160: return 0xA6; /* S caron */
	case 0x0161: return 0xA8; /* s caron */
	case 0x017D: return 0xB4; /* Z caron */
	case 0x017E: return 0xB8; /* z caron */
	case 0x0152: return 0xBC; /* OE */
	case 0x0153: return 0xBD; /* oe */
	case 0x0178: return 0xBE; /* Y diaeresis */
	default:     return 0xFFFD; /* cannot translate */
	}
}

size_t iconv(iconv_t cd, char **in, size_t *inb, char **out, size_t *outb)
{
	size_t x=0;
	unsigned char to = (cd>>1)&127;
	unsigned char from = 255;
	const unsigned char *map = 0;
	char tmp[MB_LEN_MAX];
	wchar_t c, d;
	size_t k, l;
	int err;

	if (!in || !*in || !*inb) return 0;

	if (cd & 1)
		map = charmaps[cd>>8].map;
	else
		from = cd>>8;

	for (; *inb; *in+=l, *inb-=l) {
		c = *(unsigned char *)*in;
		l = 1;
		if (from >= UTF_8 && c < 0x80) goto charok;
		switch (from) {
		case WCHAR_T:
			l = sizeof(wchar_t);
			if (*inb < l) goto starved;
			c = *(wchar_t *)*in;
			break;
		case UTF_8:
			l = utf8dec_wchar(&c, *in, *inb);
			if (!l) l++;
			else if (l == (size_t)-1) goto ilseq;
			else if (l == (size_t)-2) goto starved;
			break;
		case US_ASCII:
			goto ilseq;
		case LATIN_9:
			if ((unsigned)c - 0xa4 <= 0xbe - 0xa4) {
				static const unsigned char map[] = {
					0, 0x60, 0, 0x61, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0x7d, 0, 0, 0, 0x7e, 0, 0, 0,
					0x52, 0x53, 0x78
				};
				if (c == 0xa4) c = 0x20ac;
				else if (map[c-0xa5]) c = 0x100 | map[c-0xa5];
			}
		case LATIN_1:
			goto charok;
		case TIS_620:
			if (c >= 0xa1) c += 0x0e01-0xa1;
			goto charok;
		case JIS_0201:
			if (c >= 0xa1) {
				if (c <= 0xdf) c += 0xff61-0xa1;
				else goto ilseq;
			}
			goto charok;
		case UTF_16BE:
		case UTF_16LE:
			l = 2;
			if (*inb < 2) goto starved;
			c = get_16(*in, from);
			if ((unsigned)(c-0xdc00) < 0x400) goto ilseq;
			if ((unsigned)(c-0xd800) < 0x400) {
				l = 4;
				if (*inb < 4) goto starved;
				d = get_16(*in + 2, from);
				if ((unsigned)(c-0xdc00) >= 0x400) goto ilseq;
				c = ((c-0xd800)<<10) | (d-0xdc00);
			}
			break;
		case UTF_32BE:
		case UTF_32LE:
			l = 4;
			if (*inb < 4) goto starved;
			// FIXME
			// c = get_32(*in, from);
			break;
		default:
			/* only support ascii supersets */
			if (c < 0x80) break;
			switch (map[0]) {
			case UCS2_8BIT:
				c -= 0x80;
				break;
			case EUC:
				if ((unsigned)c - 0xa1 >= 94) goto ilseq;
				if ((unsigned)in[0][1] - 0xa1 >= 94) goto ilseq;
				c = (c-0xa1)*94 + (in[0][1]-0xa1);
				l = 2;
				break;
			case SHIFT_JIS:
				if ((unsigned)c - 0xa1 <= 0xdf-0xa1) {
					c += 0xff61-0xa1;
					goto charok;
				}
				// FIXME...
				l = 2;
				break;
			default:
				goto badf;
			}
			c = get_16(map + 4 + 2*c, 0);
			if (c == 0xffff) goto ilseq;
			goto charok;
		}

		if ((unsigned)c - 0xd800 < 0x800 || (unsigned)c >= 0x110000)
			goto ilseq;
charok:
		switch (to) {
		case WCHAR_T:
			if (*outb < sizeof(wchar_t)) goto toobig;
			*(wchar_t *)*out = c;
			*out += sizeof(wchar_t);
			*outb -= sizeof(wchar_t);
			break;
		case UTF_8:
			if (*outb < 4) {
				k = utf8enc_wchar(tmp, c);
				if (*outb < k) goto toobig;
				memcpy(*out, tmp, k);
			} else k = utf8enc_wchar(*out, c);
			*out += k;
			*outb -= k;
			break;
		case US_ASCII:
			if (c > 0x7f) c = 0xfffd;
			/* fall thru and count replacement in latin1 case */
		case LATIN_9:
			if (c >= 0x100 && c != 0xfffd)
				c = latin9_translit(c);
			/* fall through */
		case LATIN_1:
			if (c > 0xff) goto ilseq;
			if (!*outb) goto toobig;
			**out = c;
			++*out;
			--*outb;
			break;
		case UTF_16BE:
		case UTF_16LE:
			if (c < 0x10000) {
				if (*outb < 2) goto toobig;
				put_16(*out, c, to);
				*out += 2;
				*outb -= 2;
				break;
			}
			if (*outb < 4) goto toobig;
			put_16(*out, (c>>10)|0xd800, to);
			put_16(*out + 2, (c&0x3ff)|0xdc00, to);
			*out += 4;
			*outb -= 4;
			break;
		default:
			goto badf;
		}
	}
	return x;
ilseq:
	err = EILSEQ;
	x = -1;
	goto end;
badf:
	err = EBADF;
	x = -1;
	goto end;
toobig:
	err = E2BIG;
	x = -1;
	goto end;
starved:
	err = EINVAL;
end:
	errno = err;
	return x;
}
