/*
 * debug support code
 * Part of:
 * bjnp backend for the Common UNIX Printing System (CUPS).
 * Copyright 2008-2014 by Louis Lagendijk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 or later.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <stdarg.h>
#include <sys/timeb.h>
#include <errno.h>
#include "bjnp.h"

#ifdef __GNUC__
# define UNUSED(v) (void) v
#else
# define UNUSED(v)
#endif



typedef struct {
    bjnp_loglevel_t level;
    char string[10];
} logtable_entry_t;

static logtable_entry_t logtable[] = {
    {LOG_NONE,  "NONE"},
    {LOG_EMERG, "EMERG"},
    {LOG_ALERT, "ALERT"},
    {LOG_CRIT,  "CRIT"},
    {LOG_ERROR, "ERROR"},
    {LOG_WARN,  "WARNING"},
    {LOG_NOTICE, "NOTICE"},
    {LOG_INFO,  "INFO"},
    {LOG_DEBUG, "DEBUG"},
    {LOG_DEBUG2, "DEBUG2"},
    {LOG_END, ""}
};

/*
 * static data
 */

static bjnp_loglevel_t debug_level = LOG_ERROR;
static FILE *debug_file = NULL;
static time_t start_sec = 0;
static int start_msec;

/*
 * local functions
 */

#ifndef NDEBUG

static void
u8tohex(uint8_t x, char *str)
{
    static const char hdigit[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };
    str[0] = hdigit[(x >> 4) & 0xf];
    str[1] = hdigit[x & 0xf];
    str[2] = '\0';
}

static void
u8tochar(uint8_t x, char *str)
{
    if ((x > 0x20) && (x < 0x7f)) {
        str[0] = (char) x;
    } else {
        str[0] = (char) '.';
    }
}

static void
u32tohex(uint32_t x, char *str)
{
    u8tohex(x >> 24, str);
    u8tohex(x >> 16, str + 2);
    u8tohex(x >> 8, str + 4);
    u8tohex(x, str + 6);
}

char *
level2str(bjnp_loglevel_t level)
{
    int i;

    for (i = 0; logtable[i].level != LOG_END; i++) {
        if (logtable[i].level == level) {
            return logtable[i].string;
        }
    }

    return "UNDEF";
}

bjnp_loglevel_t
str2level(const char *level)
{
    int i;

    for (i = 0; strlen(logtable[i].string) != 0; i++) {
        if (strncasecmp(level, logtable[i].string, 10) == 0) {
            return logtable[i].level;
        }
    }

    return LOG_END;

}

void
bjnp_hexdump(bjnp_loglevel_t level, char *header, const void *d_,
             unsigned len)
{
    const uint8_t *d = (const uint8_t *)(d_);
    unsigned ofs, c;
    char line[100];       /* actually only 1+8+1+8*3+1+8*3+1+4+16 = 80 bytes needed */

    if (level > debug_level) {
        return;
    }


    bjnp_debug(level, "%s\n", header);
    ofs = 0;

    while (ofs < len) {
        char *p;

        memset(line, ' ', sizeof(line));

        line[0] = ' ';
        u32tohex(ofs, line + 1);
        line[9] = ':';
        p = line + 10;

        for (c = 0; c != 16 && (ofs + c) < len; c++) {
            u8tohex(d[ofs + c], p);
            p[2] = ' ';
            p += 3;

            if (c == 7) {
                p[0] = ' ';
                p++;
            }
        }

        p[0] = p[1] = p[2] = ' ';
        p = line + 61;

        for (c = 0; c != 16 && (ofs + c) < len; c++) {
            u8tochar(d[ofs + c], p);

            p++;

            if (c == 7) {
                p[0] = ' ';
                p++;
            }
        }

        p[0] = '\0';
        bjnp_debug(level, "%s\n", line);
        ofs += c;
    }

    bjnp_debug(level, "\n\n");
}

#endif /* NDEBUG */

void
bjnp_debug(bjnp_loglevel_t level, const char *fmt, ...)
{
    va_list ap;
    char printbuf[1024];
    struct timeb timebuf;
    int sec;
    int msec;

    if (level <= debug_level) {
        /* print received data into a string */
        va_start(ap, fmt);
        vsnprintf(printbuf, sizeof(printbuf), fmt, ap);
        va_end(ap);

        /* we only send real errors & warnings to the cups logging facility */

        if (level <= LOG_WARN) {
            fprintf(stderr, "%s: %s", level2str(level), printbuf);
        }

        /* all log messages may go to the own logfile */

        if (debug_file != NULL) {
            ftime(&timebuf);

            if ((msec = timebuf.millitm - start_msec) < 0) {
                msec += 1000;
                timebuf.time -= 1;
            }

            sec = timebuf.time - start_sec;

            fprintf(debug_file, "%8s: %03d.%03d %s", level2str(level), sec, msec,
                    printbuf);
            fflush(debug_file);
        }
    }
}

void
bjnp_set_debug_level(const char *level, const char *filename)
{
    /*
     * set debug level to level (string)
     */

    struct timeb timebuf;
    char loglevel[16];

    ftime(&timebuf);
    start_sec = timebuf.time;
    start_msec = timebuf.millitm;

    /*
     * Set log level
     */

    if (level == NULL) {
        debug_level = LOG_ERROR;
    } else {
        strncpy(loglevel, level, 15);
        loglevel[15] = '\0';
        debug_level = str2level(loglevel);
    }

    if (filename == NULL) {
        filename = CUPS_LOGDIR "/" LOGFILE;
    }

    if ((debug_file = fopen(filename, "w")) == NULL) {
        bjnp_debug(LOG_WARN,
                   "Cannot open logfile: %s - %s\n",
                   filename, strerror(errno));
        return;
    }

    bjnp_debug(LOG_WARN, "BJNP logging to %s, debug level = %s\n", filename, level2str(debug_level));
}
