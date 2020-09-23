/*
 * Higher level bjnp IO communication implementation for
 * bjnp backend for the Common UNIX Printing System (CUPS).
 * Copyright 2014 by Louis Lagendijk
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

#include <fcntl.h>
#include <errno.h>

#include "bjnp.h"
#include "bjnp-protocol.h"
#include "bjnp-io.h"
#include "bjnp-commands.h"

/*
 * new_printer - allocate printer struct and set defaults
 */
static printer_t *new_printer(void)
{
    int i;
    printer_t *printer = malloc(sizeof(printer_t));
    memset(printer, 0, sizeof(*printer));
    printer->global_ink_warning_level = LEVEL_UNKNOWN;
    printer->paper_out = -1;
    printer->first_output = 1;
    printer->reporting_capabilities = BJNP_REPORT_ALL;

    for (i = 0; i < BJNP_CARTRIDGES_MAX; i++) {
        printer->cartridges[i].cart_index = -1;
        printer->cartridges[i].warning = LEVEL_UNKNOWN;
        printer->cartridges[i].marker_level = -1;
    }

    printer->no_cartridges = -1;
    printer->last_level_report = 0;
    printer->fd = -1;
    return printer;
}

void free_printer(printer_t *printer)
{
    free(printer);
}

/*
 * Close printer and cleanup & free the structure
 */
void
bjnp_close_printer(printer_t *printer)
{
    /*
     * Signal end of printjob to printer
     */
    if (printer->fd >= 0) {
        close(printer->fd);
    }

    bjnp_debug(LOG_DEBUG2, "Finish printjob\n");
    bjnp_send_close(printer);
    free_printer(printer);
}

/*
 * bjnp_tcp_connect - Setup a TCP connection to the printer.
 *
 */

static int
bjnp_tcp_connect(printer_t *printer)
{
    int val;
    int tcp_socket = -1;
#ifdef __APPLE__
    struct timeval timeout;
#endif /* __APPLE__ */
    char host[BJNP_HOST_MAX];
    char family[BJNP_FAMILY_MAX];
    int port;

    /*
     * Create the socket...
     */

    if ((tcp_socket = (int)socket(get_protocol_family(printer->printer_sa),
                                  SOCK_STREAM,
                                  0)) < 0) {
        return tcp_socket;
    }

    /*
     * Set options...
     */
    val = 1;
    setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

#if 0
    val = 1;
    setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));

    val = 1;
    setsockopt(tcp_socket, SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof(val));
#endif

#ifdef __APPLE__
    /*
     * Use a 30-second read timeout when connecting to limit the amount of time
     * we block...
     */

    timeout.tv_sec  = 30;
    timeout.tv_usec = 0;
    setsockopt(tcp_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif /* __APPLE__ */

    /*
     * Using TCP_NODELAY improves responsiveness, especially on systems
     * with a slow loopback interface...
     */

    val = 1;
    setsockopt(tcp_socket, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));

    /*
     * Close this socket when starting another process...
     */

    fcntl(tcp_socket, F_SETFD, FD_CLOEXEC);

    /*
     * Then connect...
     */

    get_printer_address_info(printer, host, &port, family);

    if (connect(tcp_socket, &(printer->printer_sa.addr),
                sa_size(printer->printer_sa)) != 0) {
        bjnp_debug(LOG_CRIT,
                   "Failed to establish TCP connection to printer %s port %d\n",
                   host, port);
        close(tcp_socket);
        return -1;
    }

    bjnp_debug(LOG_DEBUG, "Established TCP connection (fd = %d) to printer %s port %d\n",
               tcp_socket, host, port);
    return tcp_socket;
}

static printer_t *
bjnp_init_printer(printer_t *printer, http_addr_t *addr, const char *user, const char *title)
{
    uint16_t session_id;
    char host[BJNP_HOST_MAX];
    int port;
    char family[BJNP_FAMILY_MAX];


    get_address_info(addr, host, &port, family);
    bjnp_debug(LOG_DEBUG, "Connecting to %s port %d (%s)\n",
               host, port, family);

    if ((session_id = bjnp_send_job_details(addr, user, title)) < 0) {

        /* printer init failed, release struct */

        if (printer != NULL) {
            bjnp_close_printer(printer);
        }

        return NULL;
    }

    /* found printer, now fill the printer struct */
    if (printer == NULL) {
        printer = new_printer();
    }

    printer->io_free = 1;
    printer->session_id = session_id;
    memcpy(&(printer->printer_sa), addr, sa_size(printer->printer_sa));
    /* set printer information in case it is needed later */

    get_printer_id(&(printer->printer_sa), printer->model,
                   printer->IEEE1284_id);
    return printer;
}


static printer_t *
bjnp_start_job(printer_t *printer, http_addrlist_t *list, const char *user,
               const char *title)
{
    /*
     * send details of printjob to printer
     * Returns: addrlist set to address details of used printer
     */
    http_addr_t *addr;

    if (printer == NULL) {
        while (list != NULL) {
            addr = (http_addr_t *) & (list->addr);

            if ((printer = bjnp_init_printer(printer, addr, user, title)) != NULL) {
                return printer;
            }

            list = list->next;
        }
    } else {

        /* we already found the printer, but could not connect to it */

        printer = bjnp_init_printer(printer, &(printer->printer_sa), user, title);
    }

    return printer;
}

printer_t *
bjnp_printer_connect(printer_t *printer,  http_addrlist_t *list, const char *user,
                     const char *title, int *bjnp_error)
{
    /* look for printer address and tell printer that we want to start printing */

    if ((printer = bjnp_start_job(printer, list, user, title)) != NULL) {

        /* found printer, check resources & state */

        if ((*bjnp_error = bjnp_report_levels(printer)) != 0) {

            /* printer not ready, we keep the printer struct around, */
            /* but tell the printer to close the job. */
            /* This avoids cluttering the printing log */

            bjnp_send_close(printer);
        } else {

            /* printer is ready, setup tcp-connection */
            /* this should succeed */

            if ((printer->fd = bjnp_tcp_connect(printer)) < 0) {

                /* remove the printer struct as we did fail */

                bjnp_close_printer(printer);
                printer = NULL;
            }
        }
    }

    return printer;
}


ssize_t
bjnp_write2(printer_t *printer, const void *buf, size_t count)
{
    /*
     * This function writes printdata to the printer.  This function mimicks the std.
     * lib. write function as much as possible.
     * Returns: number of bytes written to the printer
     */
    int sent_bytes;
    int terrno;
    int command_len;

    if (!printer->io_free) {
        errno = EAGAIN;
        return -1;
    }

    /* set BJNP command header */

    command_len =  bjnp_header_size + count;
    printer->seq_no = bjnp_printer_set_command_header(printer, &(printer->print_buf),
                      CMD_TCP_PRINT,
                      command_len);
    printer->io_count = count;
    memcpy(&(printer->print_buf.tcp_print.data), buf, count);


    bjnp_debug(LOG_DEBUG, "bjnp_write2: printing %d bytes\n", count);
    bjnp_hexdump(LOG_DEBUG2, "Print data:", (char *) & (printer->print_buf),
                 command_len);

    if ((sent_bytes =
             write(printer->fd, &(printer->print_buf), command_len)) < 0) {
        /* return result from write */
        terrno = errno;
        bjnp_debug(LOG_CRIT, "bjnp_write2: Could not send data!\n");
        errno = terrno;
        return sent_bytes;
    } else if (sent_bytes != command_len) {
        errno = EIO;
        return -1;
    }

    printer->io_free = 0;

    /* correct nr of bytes sent for length of command */
    /* sent_byte < expected is an unrecoverable error */

    return sent_bytes - bjnp_header_size;
}



int
bjnp_backchannel(printer_t *printer, ssize_t *written)
{
    /*
     * This function receives the responses to the write commands.
     * written wil be set to the number of bytes confirmed by the printer
     * Returns:
     * BJNP_OK when valid ack is received, written is set to number of bytes
     *         sent to and accepted by printer (could be 0 for keep-alive)
     * BJNP_IO_ERROR when any io-error occurred
     * BJNP_NOT_AN_ACK when the packet received was not an ack, must be ignored
     * BJNP_THROTTLE when printer indicated it could not handle the input data
     */
    bjnp_response_t response;
    unsigned int recv_bytes;
    unsigned int resp_seqno;
    int terrno;
    uint32_t payload_len;
    fd_set input;
    struct timeval timeout;

    bjnp_debug(LOG_DEBUG, "bjnp_backchannel: receiving response on fd = %d\n", printer->fd);

    /* get response header, we can unfortunately not */
    /* rely on getting the payload in the first read */

    if ((recv_bytes =
             read(printer->fd, &response,
                  bjnp_header_size)) < bjnp_header_size) {
        terrno = errno;
        bjnp_debug(LOG_CRIT,
                   "bjnp_backchannel: (recv) could not read response header, received %d bytes!\n",
                   recv_bytes);

        if (terrno < 0) {
            bjnp_debug(LOG_CRIT, "bjnp_backchannel: (recv) error: %s!\n",
                       strerror(terrno));
        }

        errno = terrno;
        return BJNP_IO_ERROR;
    }

    /* got response header back, get payload length */

    payload_len = ntohl(response.tcp_print_response.header.payload_len);

    /* it should be at least the same size as the accepted field */

    if ((payload_len >= sizeof(response.tcp_print_response.accepted)) &&
        (payload_len < (sizeof(response) - sizeof(struct bjnp_header)))) {
        /* read nr of bytes accepted by printer */

        FD_ZERO(&input);
        FD_SET(printer->fd, &input);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        if (select(printer->fd + 1, &input, NULL, NULL, &timeout) <= 0) {
            terrno = errno;
            bjnp_debug(LOG_CRIT,
                       "bjnp_backchannel: could not read response payload (select)!\n");
            errno = terrno;
            return BJNP_IO_ERROR;
        }

        if ((recv_bytes =
                 read(printer->fd, &response.tcp_print_response.accepted,
                      payload_len)) < payload_len) {
            terrno = errno;
            bjnp_debug(LOG_CRIT,
                       "bjnp_backchannel: could not read response payload (recv)!\n");
            errno = terrno;
            return BJNP_IO_ERROR;
        }

        *written = ntohl(response.tcp_print_response.accepted);
    } else {
        /* there is no payload, assume 0 bytes received */
        *written = 0;
        errno = EIO;
        return BJNP_IO_ERROR;
    }


    bjnp_hexdump(LOG_DEBUG2, "TCP response:", &response,
                 bjnp_header_size + recv_bytes);

    if (response.tcp_print_response.header.cmd_code != CMD_TCP_PRINT) {
        /* not a print response, discard */

        bjnp_debug(LOG_DEBUG, "Not a printing response packet, discarding!");
        return BJNP_NOT_AN_ACK;
    }

    resp_seqno = ntohs(response.header.seq_no);

    /* do sanity check on sequence number of response */
    if (resp_seqno != printer->seq_no) {
        bjnp_debug(LOG_CRIT,
                   "bjnp_backchannel: printer reported sequence number %d, expected %d\n",
                   resp_seqno, printer->seq_no);

        errno = EIO;
        return BJNP_IO_ERROR;
    }

    bjnp_debug(LOG_DEBUG,
               "bjnp_backchannel: response: written = %lx, seqno = %lx\n",
               *written, resp_seqno);

    printer->io_free = 1;

    /* check length reported by printer */

    if (printer->io_count == *written) {
        /* printer reported expected number of bytes */
        return BJNP_OK;
    } else if (*written == 0) {
        /* data was sent to printer, but printer reports that it is busy */
        /* add a delay before we try again */

        bjnp_debug(LOG_INFO, "Printer does not accept data, throttling....\n");
        usleep(40000);
        return BJNP_THROTTLE;
    }

    /* printer reports unexpected number of bytes */
    bjnp_debug(LOG_CRIT,
               "bjnp_backchannel: printer reported %d bytes received, expected %d\n",
               written, printer->io_count);
    errno = EIO;
    return BJNP_IO_ERROR;
}

ssize_t
bjnp_write(printer_t *printer, const void *buf, size_t count)
{
    /* This is a wrapper around bjnp_write2. It parses the input stream for
     * BJL commands and outputs these in a new/separate tcp packet. Each call
     * prints at most buffer upto next command
     * It is an ugly hack, I know....
     * This function can also be used to send keep-alive packets when count = 0
     */

    char start_cmd[] = { 0x1b, 0x5b, 0x4b, 0x2, 0x0, 0x0 };
    int print_count;
    int result;
    int terrno;

    /* TODO: allow scanning over buffer borders */

    bjnp_debug(LOG_DEBUG, "bjnp_write: starting printing of %d characters\n",
               count);

    if ((print_count =
             find_bin_string(buf, count, start_cmd, sizeof(start_cmd))) == -1) {
        /* no command found, print whole buffer */

        print_count = count;
    }

    /* print content of buf upto command */

    result = bjnp_write2(printer, buf, print_count);
    terrno = errno;
    bjnp_debug(LOG_DEBUG,
               "bjnp_write: Printed %d bytes, last command sent: %d\n",
               result, printer->seq_no);
    errno = terrno;

    return result;
}

int
bjnp_backendGetDeviceID(printer_t *printer, char *device_id, int device_id_size,
                        char *make_model, int make_model_size)
{
    /*
     * Returns the printer information for the active printer
     * Returns: 0 if ok
     *          -1 if not found
     */
    strncpy(device_id, printer->IEEE1284_id, device_id_size);
    device_id[device_id_size] = '\0';

    strncpy(make_model, printer->model, make_model_size);
    make_model[make_model_size] = '\0';

    if ((strlen(make_model) == 0) && (strlen(device_id) == 0)) {
        return -1;
    }

    return 0;
}

int bjnp_get_device_fd(printer_t *printer)
{
    return printer->fd;
}
