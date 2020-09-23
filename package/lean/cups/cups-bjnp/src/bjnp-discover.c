/*
 * Printer discovery implementation for
 * bjnp backend for the Common UNIX Printing System (CUPS).
 * Copyright 2008-2014 by Louis Lagendijk
 *
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

#include "config.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#ifdef HAVE_GETIFADDRS
#include <ifaddrs.h>
#endif
#include "bjnp.h"
#include "bjnp-protocol.h"
#include "bjnp-commands.h"

static int create_broadcast_socket(const http_addr_t *local_addr)
{
    int sockfd = -1;
    int broadcast = 1;
    int ipv6_v6only = 1;


    if ((sockfd = socket(local_addr-> addr.sa_family, SOCK_DGRAM, 0)) == -1) {
        bjnp_debug
        (LOG_CRIT, "create_broadcast_socket: cannot open socket - %s",
         strerror(errno));
        return -1;
    }

    /* Set broadcast flag on socket */

    if (setsockopt
        (sockfd, SOL_SOCKET, SO_BROADCAST, (const char *) &broadcast,
         sizeof(broadcast)) != 0) {
        bjnp_debug
        (LOG_CRIT,
         "create_broadcast_socket: setting socket option SO_BROADCAST failed - %s",
         strerror(errno));
        close(sockfd);
        return -1;
    };

#ifdef ENABLE_IPV6

    /* For an IPv6 socket, bind to v6 only so a V6 socket can co-exist with a v4 socket */
    if ((local_addr -> addr.sa_family == AF_INET6) && (setsockopt
            (sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (const char *) &ipv6_v6only,
             sizeof(ipv6_v6only)) != 0)) {
        bjnp_debug
        (LOG_CRIT,
         "create_broadcast_socket: setting socket option IPV6_V6ONLY failed - %s",
         strerror(errno));
        close(sockfd);
        return -1;
    };

#endif

    if (bind(sockfd, &(local_addr->addr), (socklen_t) sa_size(*local_addr)) != 0) {
        bjnp_debug
        (LOG_CRIT,
         "create_broadcast_socket: bind socket to local address failed - %s\n",
         strerror(errno));
        close(sockfd);
        return -1;
    }

    return sockfd;
}

static int
prepare_socket(const char *if_name, const http_addr_t *local_sa,
               const http_addr_t *broadcast_sa, http_addr_t *dest_sa)
{
    /*
     * Prepare a socket for broadcast or multicast
     * Input:
     * if_name: the name of the interface
     * local_sa: local address to use
     * broadcast_sa: broadcast address to use, if NULL we use all hosts
     * dest_sa: (write) where to return destination address of broadcast
     * retuns: open socket or -1
     */

    int socket = -1;
    http_addr_t local_sa_copy;

    if (local_sa == NULL) {
        bjnp_debug(LOG_DEBUG,
                   "%s is not a valid IPv4 interface, skipping...\n",
                   if_name);
        return -1;
    }

    memset(&local_sa_copy, 0, sizeof(local_sa_copy));
    memcpy(&local_sa_copy, local_sa, sa_size(*local_sa));

    switch (local_sa_copy.addr.sa_family) {
        case AF_INET: {
                local_sa_copy.ipv4.sin_port = htons(BJNP_PORT_PRINT);

                if (local_sa_copy.ipv4.sin_addr.s_addr == htonl(INADDR_LOOPBACK)) {
                    /* not a valid interface */

                    bjnp_debug(LOG_DEBUG,
                               "%s is not a valid IPv4 interface, skipping...\n",
                               if_name);
                    return -1;
                }


                /* send broadcasts to the broadcast address of the interface */

                memcpy(dest_sa, broadcast_sa, sa_size(*dest_sa));
                dest_sa -> ipv4.sin_port = htons(BJNP_PORT_PRINT);

                if ((socket = create_broadcast_socket(&local_sa_copy)) != -1) {
                    bjnp_debug(LOG_DEBUG, "%s is IPv4 capable, sending broadcast, socket = %d\n",
                               if_name, socket);
                } else {
                    bjnp_debug(LOG_DEBUG, "%s is IPv4 capable, but failed to create a socket.\n",
                               if_name);
                    return -1;
                }
            }
            break;
#ifdef ENABLE_IPV6

        case AF_INET6: {
                local_sa_copy.ipv6.sin6_port = htons(BJNP_PORT_PRINT);

                if (IN6_IS_ADDR_LOOPBACK(&(local_sa_copy.ipv6.sin6_addr))) {
                    /* not a valid interface */

                    bjnp_debug(LOG_DEBUG,
                               "%s is not a valid IPv6 interface, skipping...\n",
                               if_name);
                    return -1;
                } else {
                    dest_sa -> ipv6.sin6_family = AF_INET6;
                    dest_sa -> ipv6.sin6_port = htons(BJNP_PORT_PRINT);
                    inet_pton(AF_INET6, "ff02::1", dest_sa -> ipv6.sin6_addr.s6_addr);

                    if ((socket = create_broadcast_socket(&local_sa_copy)) != -1) {
                        bjnp_debug(LOG_DEBUG, "%s is IPv6 capable, sending broadcast, socket = %d\n",
                                   if_name, socket);
                    } else {
                        bjnp_debug(LOG_DEBUG, "%s is IPv6 capable, but failed to create a socket.\n",
                                   if_name);
                        return -1;
                    }
                }
            }
            break;
#endif

        default:
            socket = -1;
    }

    return socket;
}

static int
bjnp_send_broadcast(int sockfd, const http_addr_t *broadcast_addr,
                    bjnp_command_t cmd, int size)
{
    int num_bytes;

    /* set address to send packet to */
    /* usebroadcast address of interface */

    if ((num_bytes = sendto(sockfd, &cmd, size, 0,
                            &(broadcast_addr->addr),
                            sa_size(*broadcast_addr))) != size) {
        bjnp_debug(LOG_DEBUG,
                   "bjnp_send_broadcast: Socket: %d: sent only %x = %d bytes of packet, error = %s\n",
                   sockfd, num_bytes, num_bytes, strerror(errno));
        /* not allowed, skip this interface */

        return -1;
    }

    return sockfd;
}

static void add_printer_address(http_addr_t *printer_sa, char *mac_address,
                                int *no_printers, struct printer_list *printers)
{
    char ip_addr[BJNP_HOST_MAX];
    int port;
    char family[BJNP_FAMILY_MAX];
    int i;
    bjnp_address_type_t host_type;
    char host[BJNP_HOST_MAX];


    host_type = get_printer_host(*printer_sa, host, &port, family);

    bjnp_debug(LOG_DEBUG, "add_printer_address(%s) %d\n", host, *no_printers);

    /* Check if a device number is already allocated to any of the printer's addresses */
    for (i = 0; i < *no_printers; i++) {
        /* we check for mac_addresses as */
        /* an IPv6 host can have multiple adresses */

        if (strcmp(printers[i].mac_address, mac_address) == 0) {
            if (host_type > printers[i].host_type) {
                /* This is a better address as it resolves to a FQDN or */
                /* it is a global address, so we do not rely on a scope */
                /* which could change when the number of IP-interfaces changes */
                memcpy(printers[i].addr, printer_sa, sa_size(*printer_sa));
                strcpy(printers[i].hostname, host);
                printers[i].host_type = host_type;
                bjnp_debug(LOG_DEBUG, "Printer at %s found before, but found better address!\n",
                           host);
            } else {
                bjnp_debug(LOG_DEBUG, "Printer at %s found before, not added!\n",
                           host);
            }

            return;
        }
    }

    /* create a new device structure for this address */

    if (*no_printers == BJNP_PRINTERS_MAX) {
        bjnp_debug(LOG_CRIT,
                   "Too many devices, ran out of device structures, cannot add %s\n",
                   host);
        return;
    }

    printers[*no_printers].addr = malloc(sizeof(http_addr_t));
    memset(printers[*no_printers].addr, 0, sizeof(http_addr_t));
    memcpy(printers[*no_printers].addr, printer_sa, sa_size(*printer_sa));
    printers[*no_printers].host_type = host_type;
    printers[*no_printers].port = port;
    strcpy(printers[*no_printers].hostname, host);
    strcpy(printers[*no_printers].mac_address, mac_address);

    if (get_printer_id(printer_sa,  printers[*no_printers].model,
                       printers[*no_printers].IEEE1284_id) != 0) {
        bjnp_debug(LOG_CRIT, "Cannot read printer make & model: %s\n", host);
        free(printers[*no_printers].addr);
        return;
    }

    get_address_info(printer_sa, ip_addr, &port, family);
    bjnp_debug(LOG_DEBUG, "Printer not yet in our list, added it: %s:%d(%s)\n",
               host, port, family);
    (*no_printers)++;
}

static void add_printer(http_addr_t *printer_sa, bjnp_response_t *resp,
                        int *no_printers, struct printer_list *printers)
{
    char mac_address_string[BJNP_SERIAL_MAX];
    int i;

    /* create serial/mac_address string */

    u8tohex_string(resp -> udp_discover_response.mac_addr, mac_address_string,
                   sizeof(resp -> udp_discover_response.mac_addr));

    if (printer_sa->addr.sa_family == AF_INET) {
        add_printer_address(printer_sa, mac_address_string, no_printers, printers);
    }

#ifdef ENABLE_IPV6
    else {
        int no_addresses;
        http_addr_t printer_addr;

        /* IPV6, we also need to try all adresses received in the discover response */

        add_printer_address(printer_sa, mac_address_string, no_printers, printers);
        no_addresses = resp -> udp_discover_response.addr_len >> 4;
        memset(&printer_addr, 0, sizeof(printer_addr));
        memcpy(&printer_addr, printer_sa, sa_size(*printer_sa));

        for (i = 0; i < no_addresses; i++) {
            memcpy(printer_addr.ipv6.sin6_addr.s6_addr,
                   &(resp -> udp_discover_response.addresses.ipv6.ipv6_addr[i]), 16);

            /* we only add link-local address if the response came from a */
            /* link-local aadress (done above) */

            if (! IN6_IS_ADDR_LINKLOCAL(&(printer_addr.ipv6.sin6_addr))) {
                /* address is already in network byte order */

                add_printer_address(&printer_addr, mac_address_string, no_printers, printers);
            }
        }
    }

#endif
}

int bjnp_discover_printers(struct printer_list *printers)
{
    int numbytes = 0;
    bjnp_command_t cmd;
    bjnp_response_t disc_resp;
    int socket_fd[BJNP_SOCK_MAX];
    int no_sockets = 0;
    int i;
    int attempt;
    int last_socketfd = 0;
    fd_set fdset;
    fd_set active_fdset;
    struct timeval timeout;
    int no_printers = 0;
    http_addr_t broadcast_addr[BJNP_SOCK_MAX];
    http_addr_t printer_sa;
    socklen_t socklen;
    char host[BJNP_HOST_MAX];
    int port;
    char family[BJNP_FAMILY_MAX];

    clear_cmd(&cmd);
    memset(broadcast_addr, 0, sizeof(broadcast_addr));
    memset(&printer_sa, 0 , sizeof(printer_sa));
    bjnp_debug(LOG_DEBUG, "sanei_bjnp_find_devices:\n");

    for (i = 0; i < BJNP_SOCK_MAX; i++) {
        socket_fd[i] = -1;
    }

    bjnp_debug
    (LOG_DEBUG,
     "Do auto detection of printers...\n");

    /*
     * Send UDP DISCOVER to discover printers and return the list of printers found
     */

    FD_ZERO(&fdset);
    bjnp_defaults_set_command_header(&cmd, CMD_UDP_DISCOVER, sizeof(cmd.udp_discover));

    no_sockets = 0;
#ifdef HAVE_GETIFADDRS
    {
        struct ifaddrs *interfaces = NULL;
        struct ifaddrs *interface;
        getifaddrs(&interfaces);

        /* create a socket for each suitable interface */

        interface = interfaces;

        while ((no_sockets < BJNP_SOCK_MAX) && (interface != NULL)) {
            if (!(interface -> ifa_flags & IFF_POINTOPOINT) &&
                ((socket_fd[no_sockets] =
                      prepare_socket(interface -> ifa_name,
                                     (http_addr_t *) interface -> ifa_addr,
                                     (http_addr_t *) interface -> ifa_broadaddr,
                                     &broadcast_addr[no_sockets])) != -1)) {
                /* track highest used socket for later use in select */
                if (socket_fd[no_sockets] > last_socketfd) {
                    last_socketfd = socket_fd[no_sockets];
                }

                FD_SET(socket_fd[no_sockets], &fdset);
                no_sockets++;
            }

            interface = interface->ifa_next;
        }

        freeifaddrs(interfaces);
    }
#else
    /* we have no easy way to find interfaces with their broadcast addresses. */
    /* use global broadcast and all-hosts instead */
    {
        http_addr_t local;
        http_addr_t bc_addr;

        memset(&local, 0, sizeof(local));
        local.ipv4.sin_family = AF_INET;
        local.ipv4.sin_addr.s_addr = htonl(INADDR_ANY);

        bc_addr.ipv4.sin_family = AF_INET;
        bc_addr.ipv4.sin_port = htons(BJNP_PORT_PRINT);
        bc_addr.ipv4.sin_addr.s_addr = htonl(INADDR_BROADCAST);

        socket_fd[no_sockets] = prepare_socket("any_interface",
                                               &local,
                                               &bc_addr,
                                               &broadcast_addr[no_sockets]);

        if (socket_fd[no_sockets] >= 0) {
            FD_SET(socket_fd[no_sockets], &fdset);

            if (socket_fd[no_sockets] > last_socketfd) {
                last_socketfd = socket_fd[no_sockets];
            }

            no_sockets++;
        }

#ifdef ENABLE_IPV6
        local.ipv6.sin6_family = AF_INET6;
        local.ipv6.sin6_addr = in6addr_any;

        socket_fd[no_sockets] = prepare_socket("any_interface",
                                               &local,
                                               NULL,
                                               &broadcast_addr[no_sockets]);

        if (socket_fd[no_sockets] >= 0) {
            FD_SET(socket_fd[no_sockets], &fdset);

            if (socket_fd[no_sockets] > last_socketfd) {
                last_socketfd = socket_fd[no_sockets];
            }

            no_sockets++;
        }

#endif /* ENABLE_IPV6 */
    }
#endif /* HAVE_GETIFADDRS */

    /* send BJNP_MAX_BROADCAST_ATTEMPTS broadcasts on each prepared socket */
    for (attempt = 0; attempt < BJNP_MAX_BROADCAST_ATTEMPTS; attempt++) {
        for (i = 0; i < no_sockets; i++) {
            bjnp_send_broadcast(socket_fd[i], &broadcast_addr[i], cmd,
                                sizeof(cmd.udp_discover));
        }

        /* wait for some time between broadcast packets */
        usleep(BJNP_BROADCAST_INTERVAL * USLEEP_MS);
    }

    /* wait for a UDP response */

    timeout.tv_sec = 0;
    timeout.tv_usec = BJNP_BC_RESPONSE_TIMEOUT * USLEEP_MS;


    active_fdset = fdset;

    while (select(last_socketfd + 1, &active_fdset, NULL, NULL, &timeout) > 0) {
        bjnp_debug(LOG_DEBUG, "Select returned, time left %d.%d....\n",
                   (int) timeout.tv_sec, (int) timeout.tv_usec);

        for (i = 0; i < no_sockets; i++) {
            if (FD_ISSET(socket_fd[i], &active_fdset)) {

                socklen =  sizeof(printer_sa);

                if ((numbytes =
                         recvfrom(socket_fd[i], &disc_resp, sizeof(disc_resp), 0,
                                  &(printer_sa.addr), &socklen)) == -1) {
                    bjnp_debug
                    (LOG_INFO, "find_devices: no data received, while socket is ready");
                    break;
                } else {
                    /* process incoming packet */

                    get_address_info(&printer_sa, host, &port, family);

                    bjnp_debug(LOG_DEBUG, "Response received from %s port %d(%s)\n", host, port,
                               family);
                    bjnp_hexdump(LOG_DEBUG2, "Discover response:\n", &disc_resp, numbytes);

                    /* check if something sensible is returned */
                    if ((numbytes < bjnp_header_size) ||
                        (strncmp("BJNP", disc_resp.header.BJNP_id, 4) != 0)) {
                        /* not a valid response, assume not a printer  */

                        char bjnp_id[5];
                        strncpy(bjnp_id,  disc_resp.header.BJNP_id, 4);
                        bjnp_id[4] = '\0';
                        bjnp_debug(LOG_INFO,
                                   "Invalid discover response! Length = %d, Id = %s\n",
                                   numbytes, bjnp_id);
                        break;
                    }

                    if (!((disc_resp.header.dev_type) & 0x80)) {
                        /* not a response, a discover command from somebody else or */
                        /* a discover command that we generated */
                        break;
                    }
                };

                /* valid response, register printer */
                add_printer(&printer_sa, &disc_resp, &no_printers, printers);

            }
        }

        active_fdset = fdset;
        timeout.tv_sec = 0;
        timeout.tv_usec = BJNP_BC_RESPONSE_TIMEOUT * USLEEP_MS;
    }

    bjnp_debug(LOG_DEBUG, "printer discovery finished...\n");

    for (i = 0; i < no_sockets; i++) {
        close(socket_fd[i]);
    }

    return no_printers;
}

