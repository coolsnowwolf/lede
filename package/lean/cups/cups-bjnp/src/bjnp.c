/*
 * bjnp backend for the Common UNIX Printing System (CUPS).
 * Copyright 2008-2014 by Louis Lagendijk
 *
 * Based on cups AppSocket sources
 * Copyright 2007 by Apple Inc.
 * Copyright 1997-2007 by Easy Software Products, all rights reserved.
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

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

#ifdef WIN32
#  include <winsock.h>
#else
#  include <unistd.h>
#  include <fcntl.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#endif /* WIN32 */
#include "bjnp.h"


/*
 * 'main()' - Send a file to the printer or server.
 *
 * Usage:
 *
 *    printer-uri job-id user title copies options [file]
 */

int                               /* O - Exit status */
main(int argc,                    /* I - Number of command-line arguments */
     /*     (6 or 7) */
     char *argv[])                /* I - Command-line arguments */
{
    char method[255];             /* Method in URI */
    char hostname[1024];          /* Hostname */
    char username[255];           /* Username info (not used) */
    char resource[1024];          /* Resource info */
    char *options;                /* Pointer to options in device URI*/
    char *name;                   /* Name of option in device URI*/
    char *value;                  /* Value of option in device URI*/
    char sep;                     /* Option separator */
    int print_fd;                 /* Print file */
    int copies;                   /* Number of copies to print */
    time_t start_time;            /* Time of first connect */
    int recoverable;              /* Recoverable error shown? */
    int contimeout;               /* Connection timeout */
    int port;                     /* Port number */
    char portname[255];           /* Port name */
    char family[BJNP_FAMILY_MAX]; /* address family */
    int i;                        /* loop variable */
    int bjnp_error;               /* printer error */
    http_addrlist_t *addrlist;    /* Address list */
    printer_t *printer;           /* Connected printer */
    char addrname[256];           /* Address name */
    ssize_t tbytes;               /* Total number of bytes written */
    char *debug_level = NULL;     /* debug level to be applied */
    char *debug_file_name = NULL; /* filename for own debugging */
#if defined(HAVE_SIGACTION) && !defined(HAVE_SIGSET)
    struct sigaction action;      /* Actions for POSIX signals */
#endif /* HAVE_SIGACTION && !HAVE_SIGSET */


    /*
     * Make sure status messages are not buffered...
     */

    setbuf(stderr, NULL);

    /*
     * Ignore SIGPIPE signals...
     */

#ifdef HAVE_SIGSET
    sigset(SIGPIPE, SIG_IGN);
#elif defined(HAVE_SIGACTION)
    memset(&action, 0, sizeof(action));
    action.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &action, NULL);
#else
    signal(SIGPIPE, SIG_IGN);
#endif /* HAVE_SIGSET */

    /*
     * Check command-line...
     */

    if (argc == 1) {
        struct printer_list printers[BJNP_PRINTERS_MAX];
        int num_printers;
        int i;

        /*
         * get debug level for printer discovery based on environment settings
         */

        debug_level = getenv("BJNP_DEBUG_LEVEL");
        debug_file_name = getenv("BJNP_DEBUG_FILE");

        bjnp_set_debug_level(debug_level, debug_file_name);

        if ((num_printers = bjnp_discover_printers(printers)) == 0) {
            puts("network bjnp \"Unknown\" \"Canon network printer\"");
        } else {
            for (i = 0; i < num_printers; i++) {
                printf("network bjnp://%s:%u \"%s\" \"%s %s\" \"%s\"\n",
                       printers[i].hostname,
                       printers[i].port,
                       printers[i].model,
                       printers[i].model,
                       printers[i].hostname,
                       printers[i].IEEE1284_id);
            }
        }

        return (CUPS_BACKEND_OK);
    } else if (argc < 6 || argc > 7) {
        fprintf(stderr, "cups BJNP backend - version %s\n", VERSION);
        _cupsLangPrintf(stderr,
                        _("Usage: %s job-id user title copies options [file]\n"),
                        argv[0]);
        return CUPS_BACKEND_FAILED;
    }

    /*
     * If we have 7 arguments, print the file named on the command-line.
     * Otherwise, send stdin instead...
     */

    if (argc == 6) {
        print_fd = 0;
        copies = 1;
    } else {
        /*
         * Try to open the print file...
         */

        if ((print_fd = open(argv[6], O_RDONLY)) < 0) {
            perror("ERROR: unable to open print file");
            return CUPS_BACKEND_FAILED;
        }

        copies = atoi(argv[4]);
    }

    /*
     * Extract the hostname and port number from the URI...
     */

    httpSeparateURI(HTTP_URI_CODING_ALL, cupsBackendDeviceURI(argv),
                    method, sizeof(method), username, sizeof(username),
                    hostname, sizeof(hostname), &port,
                    resource, sizeof(resource));

    if (port == 0) {
        port =  BJNP_PORT_PRINT;
    }

    /*
     * Get options from the URI, if any...
     */

    contimeout = 7 * 24 * 60 * 60;

    if ((options = strchr(resource, '?')) != NULL) {
        /*
         * Yup, terminate the device name string and move to the first
         * character of the options...
         */

        *options++ = '\0';

        /*
         * Parse options...
         */

        while (*options) {
            /*
             * Get the name...
             */

            name = options;

            while (*options && *options != '=' && *options != '+'
                   && *options != '&') {
                options++;
            }

            if ((sep = *options) != '\0') {
                *options++ = '\0';
            }

            if (sep == '=') {
                /*
                 * Get the value...
                 */

                value = options;

                while (*options && *options != '+' && *options != '&') {
                    options++;
                }

                if (*options) {
                    *options++ = '\0';
                }
            } else {
                value = (char *) "";
            }

            /*
             * Process the option...
             */

            if (!strcasecmp(name, "contimeout")) {
                /*
                 * Set the connection timeout...
                 */

                if (atoi(value) > 0) {
                    contimeout = atoi(value);
                }
            } else if (!strcasecmp(name, "debuglevel")) {
                debug_level = value;
            } else if (!strcasecmp(name, "debugfile")) {
                debug_file_name = value;
            }
        }
    }

    bjnp_set_debug_level(debug_level, debug_file_name);

    /*
     * print command line arguments to debug log
     * We cannot do that sooner, as we first need to initialize
     * the debug system
     */

    for (i = 0; i < argc; i++) {
        bjnp_debug(LOG_DEBUG, "cups-bjnp: argv[%d] = %s\n", i, argv[i]);
    }

    /*
     * Then try to connect to the remote host...
     */

    recoverable = 0;
    start_time = time(NULL);

    sprintf(portname, "%d", port);

    if ((addrlist = httpAddrGetList(hostname, AF_UNSPEC, portname)) == NULL) {
        _cupsLangPrintf(stderr, _("ERROR: Unable to locate printer \'%s\'!\n"),
                        hostname);
        return (CUPS_BACKEND_STOP);
    }

    _cupsLangPrintf(stderr,
                    _("INFO: Attempting to connect to host %s on port %d\n"),
                    hostname, port);

    fputs("STATE: +connecting-to-device\n", stderr);

    printer = NULL;

    while (1) {
        if (((printer = bjnp_printer_connect(printer, addrlist, argv[2], argv[3], &bjnp_error)) == NULL) ||
            bjnp_error != 0)  {

            if (getenv("CLASS") != NULL) {
                /*
                 * If the CLASS environment variable is set, the job was submitted
                 * to a class and not to a specific queue.  In this case, we want
                 * to abort immediately so that the job can be requeued on the next
                 * available printer in the class.
                 */

                _cupsLangPuts(stderr,
                              _
                              ("INFO: Unable to contact printer, queuing on next "
                               "printer in class...\n"));

                /*
                 * Sleep 5 seconds to keep the job from requeuing too rapidly...
                 */

                sleep(5);

                return CUPS_BACKEND_FAILED;
            }

            if (contimeout && (time(NULL) - start_time) > contimeout) {
                _cupsLangPuts(stderr,
                              _("ERROR: Printer not ready to print!\n"));
                return CUPS_BACKEND_FAILED;
            }

            recoverable = 1;

            if (bjnp_error != 0)  {
                if (bjnp_error & BJNP_PRINTER_BUSY) {
                    _cupsLangPrintf(stderr,
                                    _("WARNING: recoverable: Network printer \'%s\' is busy, "),
                                    hostname);
                }

                if (bjnp_error & BJNP_NO_INK) {
                    _cupsLangPrintf(stderr,
                                    _("WARNING: recoverable: Network printer \'%s\' is out of ink, "),
                                    hostname);
                }

                if (bjnp_error & BJNP_NO_PAPER) {
                    _cupsLangPrintf(stderr,
                                    _("WARNING: recoverable: Network host \'%s\' is out of paper, "),
                                    hostname);
                }

                _cupsLangPrintf(stderr,
                                _("will retry in 30 seconds...\n"));
                sleep(30);

            } else {
                _cupsLangPrintf(stderr, "DEBUG: Connection error: %s\n",
                                strerror(errno));
                _cupsLangPuts(stderr,
                              _
                              ("ERROR: recoverable: Unable to connect to printer; "
                               "will retry in 30 seconds...\n"));

                sleep(30);
            }
        } else {
            break;
        }
    }

    if (recoverable) {
        /*
         * If we've shown a recoverable error make sure the printer proxies
         * have a chance to see the recovered message. Not pretty but
         * necessary for now...
         */

        fputs("INFO: recovered: \n", stderr);
        sleep(5);
    }

    fputs("STATE: -connecting-to-device\n", stderr);
    _cupsLangPrintf(stderr, _("INFO: Connected to %s...\n"), hostname);

    get_printer_address_info(printer, addrname, &port, family);
    fprintf(stderr, "DEBUG: Connected to [%s]:%d (%s)...\n",
            addrname, port, family);

    /*
     * Print everything...
     */

    tbytes = 0;

    while (copies > 0 && tbytes >= 0) {
        copies--;

        if (print_fd != 0) {
            fputs("PAGE: 1 1\n", stderr);
            lseek(print_fd, 0, SEEK_SET);
        }

        tbytes = bjnp_backendRunLoop(print_fd, printer);

        if (print_fd != 0 && tbytes >= 0) {
#ifdef HAVE_LONG_LONG
            _cupsLangPrintf(stderr,
                            _("INFO: Sent print file, %lld bytes...\n"),
                            CUPS_LLCAST tbytes);
#else
            _cupsLangPrintf(stderr,
                            _("INFO: Sent print file, %ld bytes...\n"),
                            CUPS_LLCAST tbytes);
#endif /* HAVE_LONG_LONG */
        }
    }

    /*
     * tell printer to finsh the print job and to close the connection
     */
    bjnp_close_printer(printer);
    printer = NULL;
    httpAddrFreeList(addrlist);
    addrlist = NULL;

    /*
     * Close the input file and return...
     */

    if (print_fd != 0) {
        close(print_fd);
    }

    /*
     * delay a bit as otherwise next job may hang (reported by Zedonet for PIXMA MX7600)
     */

    sleep(15);

    if (tbytes >= 0) {
        _cupsLangPuts(stderr, _("INFO: Ready to print.\n"));
    }

    return (tbytes < 0 ? CUPS_BACKEND_FAILED : CUPS_BACKEND_OK);
}
