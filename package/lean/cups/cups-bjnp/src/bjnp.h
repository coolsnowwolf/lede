/*
 * Data structures and definitions for
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
#ifndef _CUPS_BJNP_H_
#define _CUPS_BJNP_H_

#include "config.h"
#include <cups/cups.h>
#include <cups/backend.h>
#include <cups/http.h>

#define BJNP_CUPS_VERSION  (100 * CUPS_VERSION_MAJOR + CUPS_VERSION_MINOR )

#if BJNP_CUPS_VERSION  >= 103
#include <cups/sidechannel.h>
#endif

/*
 *  BJNP definitions
 */

#define BJNP_PRINTBUF_MAX 4096  /* size of printbuffer */
#define BJNP_CMD_MAX 2048       /* size of BJNP response buffer */
#define BJNP_RESP_MAX 2048      /* size of BJNP response buffer */
#define BJNP_STATUS_MAX 2048    /* max size for status string */
#define BJNP_IEEE1284_MAX 1024  /* max. allowed size of IEEE1284 id */
#define BJNP_METHOD_MAX 16      /* max length of method */
#define BJNP_HOST_MAX 128       /* max length of hostname or address */
#define BJNP_PORT_MAX 64        /* max length of port string */
#define BJNP_FAMILY_MAX 16      /* max length of human readable addr family */
#define BJNP_ARGS_MAX 128       /* max size of argument string */
#define BJNP_MODEL_MAX 64       /* max allowed size for make&model */
#define BJNP_SERIAL_MAX 16      /* size of serial/mac-address string */
#define BJNP_SOCK_MAX 256       /* maximum number of open sockets */
#define BJNP_UDP_MAX 65536      /* maximum size of a UDP packet */
#define BJNP_PRINTERS_MAX 64    /* max. number of printers in discovery */
#define BJNP_REPORT_MAX 256     /* max length of a status line reported to cups */
#define BJNP_ARG_MAX 256        /* max lenght of status arguments */
#define KEEP_ALIVE_SECONDS 3    /* max interval/2 seconds before we */
/* send a keep alive packet */
#define BJNP_MAX_BROADCAST_ATTEMPTS 2
/* number of broadcast packets to be sent */
#define BJNP_BROADCAST_INTERVAL 10
/* ms between broadcasts */
#define BJNP_REPORT_INTERVAL 3  /* seconds between status checks */
#define BJNP_MARKER_INTERVAL 30 /* max. nr of seconds between level reports */

#define BJNP_FAMILY_IPV4 "IPv4"
#define BJNP_FAMILY_IPV6 "IPv6"
#define BJNP_FAMILY_UNKNOWN "Unknown"

/* prefix for backend specific warnings */
#define BJNP_VENDOR_PREFIX "bjnp"

#define USLEEP_MS 1000                  /* sleep for 1 msec */
#define BJNP_BC_RESPONSE_TIMEOUT 500    /* waiting time for broadc. responses */
#define BJNP_PORT_PRINT 8611

#define BJNP_REPORT_URL "http://sourceforge.net/tracker/?func=add&group_id=234369&atid=1093281"

typedef enum {
    BJNP_ADDRESS_IS_LINK_LOCAL = 0,
    BJNP_ADDRESS_IS_GLOBAL = 1,
    BJNP_ADDRESS_HAS_FQDN = 2
} bjnp_address_type_t;

typedef enum {
    BJNP_STATUS_GOOD,
    BJNP_STATUS_INVAL,
    BJNP_STATUS_ALREADY_ALLOCATED
} BJNP_Status;

/*
 * structure that stores information on found printers
 */

struct printer_list {
    http_addr_t *addr;            /* adress of printer */
    char hostname[BJNP_HOST_MAX];     /* hostame, if found, else ip-address */
    int host_type;            /* indicates how desirable it is to use */
    /* this address: */
    /* 0 = link local address */
    /* 1 = global address without a FQDN */
    /* 2 = globall address with FQDN */
    int port;             /* port number */
    char IEEE1284_id[BJNP_IEEE1284_MAX];  /* IEEE1284 printer id */
    char model[BJNP_MODEL_MAX];           /* printer make and model */
    char mac_address[BJNP_SERIAL_MAX];    /* unique serial number (mac_address) */
};

#define BJNP_THROTTLE 2

typedef enum bjnp_loglevel_e {
    LOG_NONE,
    LOG_EMERG,
    LOG_ALERT,
    LOG_CRIT,
    LOG_ERROR,
    LOG_WARN,
    LOG_NOTICE,
    LOG_INFO,
    LOG_DEBUG,
    LOG_DEBUG2,
    LOG_END       /* not a real loglevel, but indicates end of list */
} bjnp_loglevel_t;

#ifndef CUPS_LOGDIR
#define CUPS_LOGDIR "/var/log/cups"
#endif /* CUPS_LOGDIR */

#define LOGFILE "bjnp_log"

/* opaque types, defined in bjnp-protocol.h */
union bjnp_command_u;
union bjnp_response_u;
typedef union bjnp_command_u bjnp_command_t;
typedef union bjnp_response_u bjnp_response_t;

typedef struct printer_s printer_t;

/*
 * bjnp-runloop.c
 */
extern ssize_t bjnp_backendRunLoop(int print_fd, printer_t *printer);

/*
 * bjnp-io.c
 */

printer_t *bjnp_printer_connect(printer_t *printer, http_addrlist_t *list,
                                const char *user, const char *title, int *bjnp_error);
void bjnp_close_printer(printer_t *printer);
int bjnp_backchannel(printer_t *printer, ssize_t *written);
ssize_t bjnp_write(printer_t *printer, const void *buf, size_t count);
extern int bjnp_backendGetDeviceID(printer_t *printer, char *device_id,
                                   int device_id_size, char *make_model,
                                   int make_model_size);
int bjnp_get_device_fd(printer_t *printer);

/*
 * bjnp-discover.c
 */
int bjnp_discover_printers(struct printer_list *list);

/*
 * bjnp-ink-level.c
 */
int bjnp_report_levels(printer_t *printer);

/*
 * bjnp-utils.c
 */
int sa_size(const http_addr_t sa);
int get_protocol_family(const http_addr_t sa);
void get_printer_address_info(const printer_t *printer, char *addr_string,
                              int *port, char *family);
void get_address_info(const http_addr_t *addr, char *addr_string, int *port,
                      char *family);
int parse_IEEE1284_to_model(char *printer_id, char *model);
int charTo2byte(char d[], const char s[], int len);
int find_bin_string(const void *in, int len, char *lookfor, int size);
bjnp_address_type_t get_printer_host(const http_addr_t printer_addr,
                                     char *name, int *port, char *family);
void u8tohex_string(uint8_t *input, char *str, int size);
char *bjnp_map_status(cups_sc_status_t status);

/*
 * bjnp-debug.c
 */
void bjnp_set_debug_level(const char *level, const char *filename);
void bjnp_debug(bjnp_loglevel_t, const char *, ...);
void bjnp_hexdump(bjnp_loglevel_t level, char *header, const void *d_,
                  unsigned len);

/*
 * return values
 */
#define BJNP_OK 0
#define BJNP_IO_ERROR -1
#define BJNP_NOT_AN_ACK 1

/*
 * Return values for bjnp_report_levels & bjnp_error
 */
#define BJNP_NO_PAPER 1
#define BJNP_NO_INK 2
#define BJNP_PRINTER_BUSY 4
#define BJNP_PRINTER_ERROR 8

/* definitions for functions available in cups 1.3 and later source tree only*/

#define _cupsLangPrintf fprintf
#define _cupsLangPuts(a,b)  fputs(b,a)
#define _(x) (x)

#ifndef CUPS_LLCAST
#  define CUPS_LLCAST   (long)
#endif
#endif /* ! CUPS_BJNP_H_ */
