/*
 * Higher level bjnp IO communication implementation for
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

#ifndef BJNP_IO_H
#define BJNP_IO_H

/* IEEE1284 related definitons */

#define INK_LEVEL_TOKEN "CIR:"
#define INK_WARNING_TOKEN "CTK:"
#define PRINTER_STATUS_TOKEN "BST:"
#define PAPER_STATUS_TOKEN "DJS:"

#define PARAMETER_SEPARATOR ';'
#define INK_LEVEL_DELIMITER ","
#define INK_WARNING_DELIMITER ",/,"
#define PAPER_PARAM_DELIMETER ","

#define DJS_PAPER_OUT "ID"
#define DJS_CANCELLING "CC"
#define BJNP_PAPER_OUT_THRESHOLD 5

#define BST_PRINTING 0x80
#define BST_BUSY     0x20
#define BST_ERROR    0x08

/* printer status capabilities */
#define BJNP_REPORT_MARKER_LEVELS 1
#define BJNP_REPORT_INK_STATUS 2
#define BJNP_REPORT_PRINTER_STATUS 4
#define BJNP_REPORT_PAPER_STATUS 8

#define BJNP_REPORT_ALL (BJNP_REPORT_MARKER_LEVELS | BJNP_REPORT_INK_STATUS | BJNP_REPORT_PRINTER_STATUS | BJNP_REPORT_PAPER_STATUS)

#define BJNP_CARTRIDGES_MAX 16

/*
 * Ink level warning levels
*/

/* TODO: Convert to enum */

typedef enum bjnp_ink_level_t {
    LEVEL_OK = 0,
    LEVEL_LOW = 1,
    LEVEL_EMPTY = 2,
    LEVEL_UNKNOWN = 3
} bjnp_ink_level_t;

struct printer_s {
    int fd;                   /* tcp socket for the printer */
    http_addr_t printer_sa;   /* address struct for printer */

    /* printer information for side channel */
    char IEEE1284_id[BJNP_IEEE1284_MAX];
    char model[BJNP_MODEL_MAX];

    /* Protocol information */
    int serial;               /* last used serial number for command */
    uint16_t  session_id;     /* session-id for this printjob */

    uint16_t seq_no;          /* Last command */
    ssize_t io_count;         /* last sent TCP data count */
    bjnp_command_t print_buf; /* print buffer */
    char io_free;             /* is printbuffer free to be used? */

    /*
     * Printer reporting capabilities
     */
    int reporting_capabilities;

    /*
     * Warning levels per cartridge
     */

    int no_cartridges;            /* number of warning levels (ink cartridges) found */
    struct {
        int cart_index;       /* type of cartridge as index in cartridge_types */
        int warning;          /* warning level */
        int marker_level;
    } cartridges[BJNP_CARTRIDGES_MAX];

    /*
     * warnings set
     */
    int global_ink_warning_level;
    int paper_out;            /* number of consecutive times paper out is reported */

    int first_output;         /* at first output we report additional info */
    time_t last_level_report; /* last time we reported the levels */
} printer_s;

#endif
