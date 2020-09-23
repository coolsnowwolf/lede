/*
 * utility functions
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
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "bjnp.h"
#include "bjnp-protocol.h"
#include "bjnp-io.h"
#include "bjnp-commands.h"
#include "bjnp-io.h"

#define DES_TOKEN "DES:"

static int sa_is_equal(const http_addr_t  sa1, const http_addr_t  sa2)
{
    if (sa1.addr.sa_family == sa2.addr.sa_family) {
        if (sa1.addr.sa_family == AF_INET) {
            if ((sa1.ipv4.sin_port == sa2.ipv4.sin_port) &&
                (sa1.ipv4.sin_addr.s_addr == sa2.ipv4.sin_addr.s_addr)) {
                return 1;
            }
        }

#ifdef ENABLE_IPV6
        else if (sa1.addr.sa_family == AF_INET6) {
            if ((sa1.ipv6.sin6_port == sa2.ipv6.sin6_port) &&
                (memcmp(&(sa1.ipv6.sin6_addr), &(sa2.ipv6.sin6_addr),
                        sizeof(struct in6_addr)) == 0)) {
                return 1;
            }
        }

#endif
    }

    return 0;
}

int sa_size(const http_addr_t sa)
{
    switch (sa.addr.sa_family) {
        case AF_INET:
            return (sizeof(struct sockaddr_in));
#ifdef ENABLE_IPV6

        case AF_INET6:
            return (sizeof(struct sockaddr_in6));
#endif

        default:
            /* should not occur */
            return sizeof(http_addr_t);
    }
}

int get_protocol_family(const http_addr_t sa)
{
    switch (sa.addr.sa_family) {
        case AF_INET:
            return PF_INET;
            break;
#ifdef ENABLE_IPV6

        case AF_INET6:
            return PF_INET6;
            break;
#endif

        default:
            /* should not occur */
            return -1;
    }
}

void get_address_info(const http_addr_t *addr, char *addr_string, int *port,
                      char *family)
{
    char tmp_addr[BJNP_HOST_MAX];

    if (addr->addr.sa_family == AF_INET) {
        inet_ntop(AF_INET, &(addr -> ipv4.sin_addr.s_addr), addr_string,
                  BJNP_HOST_MAX);
        *port = ntohs(addr->ipv4.sin_port);
        strcpy(family, BJNP_FAMILY_IPV4);
    }

#ifdef ENABLE_IPV6
    else if (addr->addr.sa_family == AF_INET6) {
        inet_ntop(AF_INET6, addr->ipv6.sin6_addr.s6_addr, tmp_addr,
                  sizeof(tmp_addr));

        if (IN6_IS_ADDR_LINKLOCAL(&(addr->ipv6.sin6_addr))) {
            sprintf(addr_string, "[%s%%%d]", tmp_addr,
                    addr->ipv6.sin6_scope_id);
        } else {
            sprintf(addr_string, "[%s]", tmp_addr);
        }

        *port = ntohs(addr->ipv6.sin6_port);
        strcpy(family, BJNP_FAMILY_IPV6);
    }

#endif
    else {
        /* unknown address family, should not occur */
        strcpy(addr_string, "Unknown address family");
        *port = 0;
        strcpy(family, BJNP_FAMILY_UNKNOWN);
    }
}

void get_printer_address_info(const printer_t *printer, char *addr_string,
                              int *port, char *family)
{
    get_address_info(&(printer->printer_sa), addr_string, port, family);
}

int
parse_IEEE1284_to_model(char *printer_id, char *model)
{
    /*
     * parses the  IEEE1284  ID of the printer to retrieve make and model
     * of the printer
     * Returns: 0 = not found
     *          1 = found, model is set
     */

    char s[BJNP_IEEE1284_MAX];
    char *tok;
    int len;

    model[0] = '\0';
    len  = strlen(printer_id);

    if ((len  > BJNP_IEEE1284_MAX) || (len < 0)) {
        bjnp_debug(LOG_ERROR, "printer id string (length) incorrect: %d\n",
                   len);
    }

    strcpy(s, printer_id);

    tok = strtok(s, ";");

    while (tok != NULL) {
        /* DES contains make and model */

        if (strncmp(tok, DES_TOKEN, strlen(DES_TOKEN)) == 0) {
            strcpy(model, tok + strlen(DES_TOKEN));
            return 1;
        }

        tok = strtok(NULL, ";");
    }

    return 0;
}

int
charTo2byte(char d[], const char s[], int len)
{
    /*
     * copy ASCII string to 2 byte unicode string
     * Returns: number of characters copied
     */

    int done = 0;
    int copied = 0;
    int i;

    for (i = 0; i < len; i++) {
        d[2 * i] = '\0';

        if (s[i] == '\0') {
            done = 1;
        }

        if (done == 0) {
            d[2 * i + 1] = s[i];
            copied++;
        } else {
            d[2 * i + 1] = '\0';
        }
    }

    return copied;
}

void u8tohex_string(uint8_t *input, char *str, int size)
{
    static const char hdigit[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };
    int i;

    for (i = 0; i < size; i++) {
        str[ 2 * i] = hdigit[(input[i]  >> 4) & 0xf];
        str[ 2 * 1 + 1] = hdigit[ input[i] & 0xf];
    }

    str[2 * i] = '\0';
}




int
find_bin_string(const void *in, int len, char *lookfor, int size)
{
    /*
     * looks for a new print command in the input stream
     * Returns: offset where lookfor is found or -1 when not found
     */

    int i;
    const char *buf = in;

    /* start at offset 1 to avoid match at start of input */
    for (i = 1; i < (len - size); i++) {
        if ((buf[i] == lookfor[0]) && (memcmp(buf + i, lookfor, size) == 0)) {
            return i;
        }
    }

    return -1;
}

bjnp_address_type_t
get_printer_host(const http_addr_t printer_addr, char *name, int *port,
                 char *family)
{
    /*
     * lookup hostname for printers address
     */

    struct addrinfo *results;
    struct addrinfo *result;
    http_addr_t *res_address;
    char ip_address[BJNP_HOST_MAX];
    char service[64];
    int error;
    int match = 0;
    bjnp_address_type_t level;

#ifdef ENABLE_IPV6

    if ((printer_addr.addr.sa_family == AF_INET6) &&
        (IN6_IS_ADDR_LINKLOCAL(&(printer_addr.ipv6.sin6_addr)))) {
        level = BJNP_ADDRESS_IS_LINK_LOCAL;
    } else {
        level = BJNP_ADDRESS_IS_GLOBAL;
    }

#else
    level = BJNP_ADDRESS_IS_GLOBAL;
#endif

    get_address_info(&printer_addr, ip_address, port, family);

    bjnp_debug(LOG_INFO, "Found printer at ip address: %s(%s)\n", ip_address,
               family);

    /* do reverse name lookup, if hostname cannot be found, return IP-addr */

    if ((error = getnameinfo(&(printer_addr.addr) , sa_size(printer_addr),
                             name, BJNP_HOST_MAX , NULL, 0, NI_NAMEREQD))
        != 0) {
        bjnp_debug(LOG_DEBUG, "Name for %s not found : %s\n",
                   ip_address, gai_strerror(error));
        strcpy(name, ip_address);
        return level;
    }


    /* some buggy routers return rubbish if reverse lookup fails, so
     * we do a forward lookup to see if the result matches the ip-address */

    sprintf(service, "%d", *port);

    if (getaddrinfo(name, service, NULL, &results) == 0) {

        result = results;

        while (result != NULL) {

            res_address = (http_addr_t *)result-> ai_addr;

            if (sa_is_equal(*res_address, printer_addr)) {

                /* found match, good */

                match = 1;
                break;
            }

            result = result-> ai_next;
        }

        freeaddrinfo(results);

        if (match == 1) {
            bjnp_debug(LOG_DEBUG, "Reverse lookup for %s succeeded, using as hostname\n",
                       name);
            level = BJNP_ADDRESS_HAS_FQDN;
        } else {
            bjnp_debug(LOG_DEBUG,
                       "Reverse lookup for %s succeeded, forward lookup failed, using IP-address %s instead\n",
                       name, ip_address);
            strcpy(name, ip_address);
        }
    } else {
        /* lookup failed, use ip-address */
        bjnp_debug(LOG_DEBUG, "Reverse lookup of %s failed, using IP-address %s\n",
                   name, ip_address);
        strcpy(name, ip_address);
    }

    return level;
}

char *bjnp_map_status(cups_sc_status_t status)
{
    switch (status) {
        case CUPS_SC_STATUS_BAD_MESSAGE:
            return "bad message";

        case CUPS_SC_STATUS_IO_ERROR:
            return "I/O error";

        case CUPS_SC_STATUS_NONE:
            return "no status";

        case CUPS_SC_STATUS_NOT_IMPLEMENTED:
            return "not implemented";

        case CUPS_SC_STATUS_NO_RESPONSE:
            return "no response";

        case CUPS_SC_STATUS_OK:
            return "status ok";

        case CUPS_SC_STATUS_TIMEOUT:
            return "timeout";

        case CUPS_SC_STATUS_TOO_BIG:
            return "message too big";

        default:
            return "unknown status, please report!";
    }
}
