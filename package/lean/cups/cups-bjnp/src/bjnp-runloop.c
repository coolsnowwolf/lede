/*
 *
 * bjnp specifc run loop APIs for the Common UNIX Printing System (CUPS).
 * Copyright 2008-2014 by  Louis Lagendijk
 *
 * based on:
 *
 * Common run loop APIs for the Common UNIX Printing System (CUPS).
 *
 * Copyright 2007-2008 by Apple Inc.
 * Copyright 2006-2007 by Easy Software Products, all rights reserved.
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

/*
 * Include necessary headers.
 */

#include "bjnp.h"
#ifdef __hpux
#  include <sys/time.h>
#else
#  include <sys/select.h>
#endif /* __hpux */
#include <stdio.h>
#include <signal.h>
#include <errno.h>
/*
 * 'backendRunLoop()' - Read and write print and back-channel data.
 */

ssize_t                                   /* O - Total bytes on success, */
/*      -1 on error */
bjnp_backendRunLoop(int print_fd,         /* I - Print file descriptor */
                    printer_t *printer)   /* I - address for printer */
{
    int send_keep_alive;                  /* flag that an empty data packet */
    /* should be sent to printer */
    int nfds;                             /* Maximum file descriptor value + 1 */
    int device_fd;                        /* Device file descriptor */
    fd_set input,                         /* Input set for reading */
           output;                        /* Output set for writing */
    ssize_t print_bytes,                  /* Print bytes read */
            total_bytes,                  /* Total bytes written */
            bytes;                        /* Bytes written */
    int result;                           /* result code from select */
    int ack_pending;                      /* io slot status */
    int draining;                         /* Drain command received? */
    char print_buffer[BJNP_PRINTBUF_MAX], /* Print data buffer */
         *print_ptr;                      /* Pointer into print data buffer */
    struct timeval timeout;
#if defined(HAVE_SIGACTION) && !defined(HAVE_SIGSET)
    struct sigaction action;  /* Actions for POSIX signals */
#endif /* HAVE_SIGACTION && !HAVE_SIGSET */

#if BJNP_CUPS_VERSION >= 103
    int side_channel_open;                /* side channel status */
    cups_sc_command_t command;            /* Request command */
    cups_sc_status_t status;              /* Request/response status */
    char data[16536];                     /* Request/response data */
    int datalen;                          /* Request/response data size */
    char model[BJNP_MODEL_MAX];           /* printer make & model */
    char dev_id[BJNP_IEEE1284_MAX];       /* IEEE1284 device id */
#endif /* cups >= 1.3 */

    fprintf(stderr,
            "DEBUG: bjnp_backendRunLoop(print_fd=%d\n", print_fd);

    /*
     * If we are printing data from a print driver on stdin, ignore SIGTERM
     * so that the driver can finish out any page data, e.g. to eject the
     * current page.  We only do this for stdin printing as otherwise there
     * is no way to cancel a raw print job...
     */

    if (!print_fd) {
#ifdef HAVE_SIGSET      /* Use System V signals over POSIX to avoid bugs */
        sigset(SIGTERM, SIG_IGN);
#elif defined(HAVE_SIGACTION)
        memset(&action, 0, sizeof(action));

        sigemptyset(&action.sa_mask);
        action.sa_handler = SIG_IGN;
        sigaction(SIGTERM, &action, NULL);
#else
        signal(SIGTERM, SIG_IGN);
#endif /* HAVE_SIGSET */
    }

    /*
     * Figure out the maximum file descriptor value to use with select()...
     */

    device_fd = bjnp_get_device_fd(printer);
    nfds = (print_fd > device_fd ? print_fd : device_fd) + 1;

    print_bytes = 0;
    print_ptr = print_buffer,
    total_bytes = 0;
    ack_pending = 0;
    draining = 0;
    send_keep_alive = 0;
    side_channel_open = 1;
    int last_marker_level_time = time(NULL);

    /*
     * Now loop until we are out of data from print_fd...
     */

    while (1) {
        /*
         * Use select() to determine whether we have data to copy around...
         */

        FD_ZERO(&input);
        FD_ZERO(&output);

        /*
         * Accept new printdata only when no data is left
         */

        if (!print_bytes) {
            FD_SET(print_fd, &input);
        }

        /*
         * accept backchannel data from printer, used for acks
         */

        FD_SET(device_fd, &input);


        /*
         * Accept side channel data, unless there is print data pending
         * (cups >= 1.3)
         */

#if  BJNP_CUPS_VERSION >= 103

        if (side_channel_open && !print_bytes && !draining) {
            FD_SET(CUPS_SC_FD, &input);
        }

#endif

        /*
         * Check if printer is ready to receive data when we have something to
         * send (printdata is left or keep-alive is to be sent) but no ack is
         * pending
         */

        if ((send_keep_alive || print_bytes) && !ack_pending) {
            FD_SET(device_fd, &output);
        }

        timeout.tv_sec = KEEP_ALIVE_SECONDS;

        timeout.tv_usec = 0;

        result = select(nfds, &input, &output, NULL, &timeout);

        if (result < 0) {
            if (errno == EINTR && total_bytes == 0) {
                fputs("DEBUG: Received an interrupt before any bytes were "
                      "written, aborting!\n", stderr);
                return 0;
            }

            sleep(1);
            continue;
        }

        /* report ink/marker levels when interval has expired */

        if (time(NULL) - last_marker_level_time > BJNP_REPORT_INTERVAL) {
            bjnp_report_levels(printer);
            last_marker_level_time = time(NULL);
        }

        if (result == 0) {
            /*
             * timeout - no data for printer; make sure that next time we
             * send a keep-alive packet to avoid that  connection to printer
             * times out
             */
            if (!ack_pending) {
                send_keep_alive = 1;
            }

            bjnp_debug(LOG_DEBUG,
                       "bjnp_runloop: select timeout send_keep_alive=%d print_fd=%d "
                       "device_fd=%d print_bytes=%d ack_pending=%d\n",
                       send_keep_alive, print_fd, device_fd, print_bytes,
                       ack_pending);
            continue;
        }

#if BJNP_CUPS_VERSION >= 103

        /*
         * Check if we have a side-channel request ready (cups >= 1.3)...
         */

        if (FD_ISSET(CUPS_SC_FD, &input)) {
            /*
             * Do the side-channel request
             */

            datalen = sizeof(data) - 1;

            if (cupsSideChannelRead(&command, &status, data, &datalen, 1.0) !=
                0) {
                /* side channel is closed, or we lost synchronization */
                side_channel_open = 0;
            } else {
                bjnp_debug(LOG_DEBUG, "Received side-channel request, command is %d\n",
                           command);

                switch (command) {
                    case CUPS_SC_CMD_DRAIN_OUTPUT:

                        /*
                         * Our sockets disable the Nagle algorithm and data is
                         *  sent immediately.
                         *
                         */

                        draining = 1;

                        /*
                         * we will do cupsSideChannelWrite() once there is no
                         * data left !
                         */
                        break;

                    case CUPS_SC_CMD_GET_BIDI:
                        status = CUPS_SC_STATUS_OK;
                        data[0] = CUPS_SC_BIDI_NOT_SUPPORTED;
                        datalen = 1;
                        cupsSideChannelWrite(command, status, data, datalen,
                                             1.0);
                        break;

                    case CUPS_SC_CMD_GET_DEVICE_ID:
                        if (bjnp_backendGetDeviceID
                            (printer, dev_id, sizeof(dev_id), model,
                             sizeof(model)) == 0) {
                            status = CUPS_SC_STATUS_OK;
                            strncpy(data, dev_id, sizeof(data));
                            datalen = (int) strlen(data);
                        } else {
                            status =  CUPS_SC_STATUS_IO_ERROR;
                            datalen = 0;
                        }

                        cupsSideChannelWrite(command, status, data, datalen,
                                             1.0);
                        break;

#if BJNP_CUPS_VERSION >= 105

                    case CUPS_SC_CMD_GET_CONNECTED:
                        status  = CUPS_SC_STATUS_OK;
                        data[0] = (device_fd != -1);
                        datalen = 1;
                        break;
#endif

                    default:

                        /*
                         * this covers the following values
                         *
                         * case CUPS_SC_CMD_GET_STATE:
                        * case CUPS_SC_CMD_SOFT_RESET:
                                      *
                                      * for CUPS 1.4 and later
                                      *
                                      * case CUPS_SC_CMD_SNMP_GET:
                                      * case CUPS_SC_CMD_SNMP_GET_NEXT:
                                      *
                                      * these values should not occur
                          * case CUPS_SC_CMD_NONE:
                                      * case CUPS_SC_CMD_MAX:
                                      */

                        status = CUPS_SC_STATUS_NOT_IMPLEMENTED;
                        datalen = 0;
                        cupsSideChannelWrite(command, status, data, datalen,
                                             1.0);
                        break;
                }

            }
        }

#endif
        /*
         * Check if we have back-channel data (ack) ready...
         */

        if (FD_ISSET(device_fd, &input)) {
            switch (bjnp_backchannel(printer, &bytes)) {
                case BJNP_IO_ERROR:
                    if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                        perror("ERROR: failed to read backchannel data");
                        return -1;
                    }

                    break;

                case BJNP_OK:
                    print_bytes -= bytes;
                    print_ptr += bytes;
                    total_bytes += bytes;
                    ack_pending = 0;

                    /*
                     * Success, reset error conditions
                     */

                    fprintf(stderr, "DEBUG: Wrote %d bytes of print data...\n",
                            (int) bytes);
                    break;

                case BJNP_THROTTLE:

                    /*
                     * Data not accepted by printer, check paper out condition
                     */

                    ack_pending = 0;
                    break;

                case BJNP_NOT_AN_ACK:
                    /* what we received was not an ack, no action */
                    break;

                default:
                    /* no action */
                    break;
            }
        }

        /*
         * Check if we have print data ready...
         */

        if (FD_ISSET(print_fd, &input)) {
            if ((print_bytes = read(print_fd, print_buffer,
                                    sizeof(print_buffer))) < 0) {
                /*
                 * Read error - bail if we don't see EAGAIN or EINTR...
                 */

                if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                    perror("ERROR: Unable to read print data");
                    return -1;
                }

                print_bytes = 0;
            } else if (print_bytes == 0) {
                /*
                 * End of input file, break out of the loop
                 */

#if BJNP_CUPS_VERSION >= 103
                if (draining) {
                    command = CUPS_SC_CMD_DRAIN_OUTPUT;
                    status = CUPS_SC_STATUS_OK;
                    datalen = 0;
                    cupsSideChannelWrite(command, status, data, datalen, 1.0);
                    draining = 0;
                }

#endif

                break;
            } else {
                print_ptr = print_buffer;

                fprintf(stderr, "DEBUG: Read %d bytes of print data...\n",
                        (int) print_bytes);
            }
        }

        /*
         * Check if the device is ready to receive data and we have data to
         * send...
         */

        if ((send_keep_alive || print_bytes) && FD_ISSET(device_fd, &output)) {
            bytes = bjnp_write(printer, print_ptr, print_bytes);
            send_keep_alive = 0;

            if (bytes < 0) {
                /*
                 * Write error - bail if we don't see an error we can retry...
                 */

                if (errno != !EAGAIN && errno != EWOULDBLOCK &&
                    errno != EINTR && errno != ENOTTY && errno != ENOSPC) {
                    fprintf(stderr,
                            _("ERROR: Unable to write print data: %s\n"),
                            strerror(errno));
                    return -1;
                }
            } else {

                /*
                 * we sent data, wait for the ack before sending more data
                 */

                ack_pending = 1;
            }
        }
    }

    /*
     * Return with success...
     */

    return (total_bytes);
}
