/*
 * Printer status, Ink-level and paper status related functions for the
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

#include <stdio.h>
#include <errno.h>
#include "bjnp.h"
#include "bjnp-protocol.h"
#include "bjnp-commands.h"
#include "bjnp-io.h"

/*
 * Ink level warnings as reported by the printer
 */

static struct {
    char *string;
    int warning_level;
} warning_level[] = {
    { "SET", LEVEL_OK },
    { "LOW", LEVEL_LOW },
    { "IO",  LEVEL_EMPTY },
    { "",    LEVEL_UNKNOWN }
};


/*
 * Lookup table for cartridge properties
*/

static struct {
    char *cart_type;          /* string identifying type of cartridge */
    char *marker_type;        /* type of marker, e.g. "ink" */
    char *marker_color;       /* sRGB encoded color */
    char *marker_name;        /* supply name */
    char *marker_low_level;   /* almost empty level */
    char *marker_high_level;  /* full level */
} cartridge_types[] = {
    /* first define values for unknown cartridges */

    { "",    "ink", "#FFFFFF", "Unknown",      "10", "100" },

    /* This are the types we know about */
    /* all strings must be less than 16 chars in length */

    { "K",   "ink", "#000000",                 "Black",        "10", "100" },
    { "C",   "ink", "#00FFFF",                 "Cyan",         "10", "100" },
    { "M",   "ink", "#FF00FF",                 "Magenta",      "10", "100" },
    { "Y",   "ink", "#FFFF00",                 "Yellow",       "10", "100" },
    { "BK",  "ink", "#000000",                 "Black",        "10", "100" },
    { "LC",  "ink", "#E0FFFF",                 "LightCyan",    "10", "100" },
    { "LM",  "ink", "#FF77FF",                 "LightMagenta", "10", "100" },
    { "GY",  "ink", "#808080",                 "Gray",         "10", "100" },
    { "PBK", "ink", "#000000",                 "PhotoBlack",   "10", "100" },
    { "CL",  "ink", "#00FFFF#FF00FF#FFFF00",   "Color",        "10", "100" },

    /* end of array record */

    { NULL,  NULL,  NULL,      			NULL,          "10", "100" }
};

#define reset_capability(a,b) b &= ~(a)

/*
 * Find cartridge properties
 * Returns index in table
 */
static int cart_type(char *desc)
{
    int i;

    /* index 0 is for unknown cartridges, so we start from 1 */
    for (i = 1; cartridge_types[i].cart_type != NULL; i++) {
        if (strcmp(cartridge_types[i].cart_type, desc) == 0) {
            return i;
        }
    }

    /* unrecognized carttridge */
    bjnp_debug(LOG_NOTICE, "Unknown cartdige type %s, please report via %s!\n", 
               desc, BJNP_REPORT_URL);
    return 0;
}

static int
get_argument(char *buf, char *token, char *argument, int size, char *error_string)
{
    /*
     * retrieve argument for parameter token
     *  stores argument in buffer argument of max size size
     */
    char *parameter_str;
    int i;

    /* find token string in status */
    if ((parameter_str = strstr(buf, token)) == NULL) {
        bjnp_debug(LOG_NOTICE,
                   "Cannot report %s, (token %s not found in status reponse)!\n", error_string, token);
        return 0;
    }

    parameter_str += strlen(token);

    /* copy argument */
    for (i = 0; i < size - 1; i++) {
        if (parameter_str[i] == PARAMETER_SEPARATOR ||
            parameter_str[i] == '\0') {
            break;
        }

        argument[i] = parameter_str[i];
    }

    argument[i] = '\0';
    return strlen(argument);
}

static void
report_marker_levels(printer_t *printer, char *status_buf, char first_output)
{
    char levels[BJNP_ARG_MAX];
    char *token;
    int no_cartridges = 0;
    char *p;
    int i;
    int type;
    long val;
    int level_changed = 0;
    int expected_type;
    char marker_colors[BJNP_REPORT_MAX] = "";
    char marker_low_levels[BJNP_REPORT_MAX] = "";
    char marker_high_levels[BJNP_REPORT_MAX] = "";
    char marker_names[BJNP_REPORT_MAX] = "";
    char marker_types[BJNP_REPORT_MAX] = "";
    char marker_levels[BJNP_REPORT_MAX] = "";
    char level[16];

    if ((printer->reporting_capabilities & BJNP_REPORT_MARKER_LEVELS) == 0 ||
        (get_argument(status_buf, INK_LEVEL_TOKEN, levels, BJNP_ARG_MAX, "marker levels") == 0)) {
        reset_capability(BJNP_REPORT_MARKER_LEVELS, printer->reporting_capabilities);
        return;
    }

    /* now parse cartridges */
    token = strtok(levels, INK_LEVEL_DELIMITER);

    while ((token != NULL) && (no_cartridges < 16)) {
        if ((p = index(token, '=')) == NULL) {
            bjnp_debug(LOG_ERROR, "error parsing ink levels in %s, no ink levels output!\n",
                       token);
            return;
        }

        *p = '\0';
        p++;

        /* get ink level */

        errno = 0;
        val = strtol(p, NULL, 10);

        if (errno != 0 || val > 100 || val < 0) {
            bjnp_debug(LOG_ERROR,
                       "error parsing ink level %s for %s, no ink levels output!\n",
                       p, token);
        }

        type = cart_type(token);

        if (printer->no_cartridges == -1) {

            /* first time we find cartridges, just store the type */

            printer->cartridges[no_cartridges].cart_index = type;
        } else if (printer->cartridges[no_cartridges].cart_index != type) {

            /* unexpected cartridge type found */

            expected_type = printer->cartridges[no_cartridges].cart_index;
            bjnp_debug(LOG_ERROR, "Cartridge type %s (%s) expected, but found type %s (%s) in ink levels\n",
                       cartridge_types[expected_type].cart_type, cartridge_types[expected_type].marker_name,
                       token, cartridge_types[type].marker_name);
            return;
        }

        /* check the level */
        if (val != printer->cartridges[no_cartridges].marker_level) {
            level_changed = 1;
            printer->cartridges[no_cartridges].marker_level = val;
        }

        /* Next token & cartridge */
        no_cartridges++;
        token = strtok(NULL, INK_LEVEL_DELIMITER);
    }

    /* check if the number of cartridges found matches our expectations */

    if (printer->no_cartridges != -1 && printer->no_cartridges != no_cartridges) {
        bjnp_debug(LOG_ERROR, "Found %d ink levels, but expected %d\n",
                   no_cartridges, printer->no_cartridges);
    } else if (printer->no_cartridges == -1) {
        printer->no_cartridges = no_cartridges;
    }

    if (no_cartridges == 0) {
        /* nothing to output */
        bjnp_debug(LOG_ERROR, "No cartridges found!\n");
        return;
    }

    /* When a level level_changed or reporting interval is exceeded, report ink levels */
    if (level_changed || (printer->last_level_report - BJNP_MARKER_INTERVAL > 0)) {
        printer->last_level_report = time(NULL);

        /* report ink levels */
        if (first_output) {

            i = 0;

            type = printer->cartridges[i].cart_index;

            strcat(marker_colors, cartridge_types[type].marker_color);
            strcat(marker_low_levels, cartridge_types[type].marker_low_level);
            strcat(marker_high_levels, cartridge_types[type].marker_high_level);
            strcat(marker_names, cartridge_types[type].marker_name);
            strcat(marker_types, cartridge_types[type].marker_type);

            while (++i < no_cartridges) {
                type = printer->cartridges[i].cart_index;

                strcat(marker_colors, ",");
                strcat(marker_colors,  cartridge_types[type].marker_color);

                strcat(marker_low_levels, ",");
                strcat(marker_low_levels,  cartridge_types[type].marker_low_level);

                strcat(marker_high_levels, ",");
                strcat(marker_high_levels,  cartridge_types[type].marker_high_level);

                strcat(marker_names, ",");
                strcat(marker_names,  cartridge_types[type].marker_name);

                strcat(marker_types, ",");
                strcat(marker_types,  cartridge_types[type].marker_type);

            }

            fprintf(stderr, "ATTR: marker-colors=\"%s\"\n", marker_colors);
            fprintf(stderr, "ATTR: marker-low-levels=\"%s\"\n", marker_low_levels);
            fprintf(stderr, "ATTR: marker-high-levels=\"%s\"\n", marker_high_levels);
            fprintf(stderr, "ATTR: marker-names=\"%s\"\n", marker_names);
            fprintf(stderr, "ATTR: marker-types=\"%s\"\n", marker_types);
        }

        i = 0;
        sprintf(level, "%d", printer->cartridges[i].marker_level);
        strcat(marker_levels, level);

        while (++i < no_cartridges) {
            strcat(marker_levels, ",");
            sprintf(level, "%d", printer->cartridges[i].marker_level);
            strcat(marker_levels, level);
        }

        fprintf(stderr, "ATTR: marker-levels=\"%s\"\n", marker_levels);
    }
}

static int
get_warning_level(char *string)
{
    int i = 0;

    while (warning_level[i].warning_level != LEVEL_UNKNOWN) {
        if (strcmp(string, warning_level[i].string) == 0) {
            return i;
        }

        i++;
    }

    return LEVEL_UNKNOWN;
}

static char *strstrtok(char *in, const char *sep)
{
    static char *next = NULL;
    char *p;
    char *ret;

    if (in != NULL) {
        next = in;
    }

    if (next == NULL) {
        return NULL;
    }

    ret = next;

    if ((p = strstr(next, sep)) == NULL) {
        next = NULL;
        return ret;
    }

    *p = '\0';
    next = p + strlen(sep);
    return ret;
}

static int report_standard_ink_warnings(int old_level, int low, int empty)
{
    int new_level;

    if (empty) {
        new_level = LEVEL_EMPTY;
    } else if (low) {
        new_level = LEVEL_LOW;
    } else {
        new_level = LEVEL_OK;
    }

    if (new_level != old_level) {
        /* reset old warning levels */
        switch (old_level) {
            case LEVEL_LOW:
                fputs("STATE: -marker-supply-low-warning\n", stderr);
                break;

            case LEVEL_EMPTY:
                fputs("STATE: -marker-supply-empty-error\n", stderr);
                break;

            case LEVEL_UNKNOWN:
                if (new_level != LEVEL_LOW) {
                    fputs("STATE: -marker-supply-low-warning\n", stderr);
                }

                if (new_level != LEVEL_EMPTY) {
                    fputs("STATE: -marker-supply-empty-error\n", stderr);
                }

                break;
        }

        /* set new warning levels */
        switch (new_level) {
            case LEVEL_LOW:
                fputs("STATE: +marker-supply-low-warning\n", stderr);
                break;

            case LEVEL_EMPTY:
                fputs("STATE: +marker-supply-empty-error\n", stderr);
                break;

            default:
                break;
        }
    }

    return new_level;
}

static void report_vendor_ink_warnings(char *marker_color, int old, int new)
{
    if (old == new) {
        /* nothing to do */

        return;
    }

    /* remove no longer valid warnings */
    switch (old) {
        case LEVEL_LOW:
            fprintf(stderr, "STATE: -%s.%s-ink-low-warning\n", BJNP_VENDOR_PREFIX,
                    marker_color);
            break;

        case LEVEL_EMPTY:
            fprintf(stderr, "STATE: -%s.%s-ink-empty-error\n", BJNP_VENDOR_PREFIX,
                    marker_color);
            break;

        case LEVEL_UNKNOWN:
            if (new != LEVEL_LOW) {
                fprintf(stderr, "STATE: -%s.%s-ink-low-warning\n", BJNP_VENDOR_PREFIX,
                        marker_color);
            }

            if (new != LEVEL_EMPTY) {
                fprintf(stderr, "STATE: -%s.%s-ink-empty-error\n", BJNP_VENDOR_PREFIX,
                        marker_color);
            }

            break;
    }

    /* Set new level warnings */
    switch (new) {
        case LEVEL_LOW:
            fprintf(stderr, "STATE: +%s.%s-ink-low-warning\n", BJNP_VENDOR_PREFIX,
                    marker_color);
            break;

        case LEVEL_EMPTY:
            fprintf(stderr, "STATE: +%s.%s-ink-empty-error\n", BJNP_VENDOR_PREFIX,
                    marker_color);
            break;

        default:
            break;
    }
}

static int report_ink_status_messages(printer_t *printer, char *status_buf,
                                      char first_output)
{
    char *token;
    char warnings[BJNP_ARG_MAX];
    int warning_level;
    char level_low = 0;
    char level_empty = 0;
    int no_cartridges = 0;
    int cartridge_type;
    char *p;

    if ((printer->reporting_capabilities & BJNP_REPORT_INK_STATUS) == 0 ||
        get_argument(status_buf, INK_WARNING_TOKEN, warnings, BJNP_ARG_MAX, "ink status") == 0) {
        reset_capability(BJNP_REPORT_INK_STATUS, printer->reporting_capabilities);
        return LEVEL_OK;
    }

    /* now parse cartridges */

    token = strstrtok(warnings, INK_WARNING_DELIMITER);

    while ((token != NULL) && (no_cartridges < BJNP_CARTRIDGES_MAX)) {
        if ((p = index(token, ',')) == NULL) {
            bjnp_debug(LOG_ERROR,
                       "error in parsing warning levels in status reponse, "
                       "warning levels not supported! token = %s\n", token);
            return !level_empty;
        }

        *p = '\0';
        p++;
        cartridge_type = cart_type(token);

        if (printer->no_cartridges == -1) {
            printer->cartridges[no_cartridges].cart_index = cartridge_type;
        } else {
            if (printer->cartridges[no_cartridges].cart_index !=
                cartridge_type) {
                bjnp_debug(LOG_ERROR,
                           "error in parsing warning levels in status reponse, "
                           "warning sequence does not match! token = %s\n",
                           token);
                return !level_empty;
            }
        }

        warning_level = get_warning_level(p);

        switch (warning_level) {
            case LEVEL_LOW:
                level_low = 1;
                break;

            case LEVEL_EMPTY:
                level_empty = 1;
                break;

            default:
                /* nothing to do */
                break;
        }

        report_vendor_ink_warnings(cartridge_types[cartridge_type].marker_name,
                                   printer->cartridges[no_cartridges].warning,
                                   warning_level);
        printer->cartridges[no_cartridges].warning = warning_level;

        no_cartridges++;
        token = strstrtok(NULL, INK_WARNING_DELIMITER);
    }

    if (printer->no_cartridges == -1) {
        /* nr of cartridges not set before */
        printer->no_cartridges  = no_cartridges;
    } else {
        if (printer->no_cartridges != no_cartridges)
            bjnp_debug(LOG_ERROR, "Number of cartridges in this run is different "
                       "from first run: now: %d, first %d\n",
                       printer->no_cartridges, no_cartridges);
    }

    if (no_cartridges == 0) {
        /* nothing to output */
        bjnp_debug(LOG_ERROR, "No cartridges found!\n");
        return LEVEL_OK;
    }

    printer->global_ink_warning_level = report_standard_ink_warnings(
                                            printer->global_ink_warning_level, level_low, level_empty);
    return level_empty;
}

static int
report_printer_status(printer_t *printer, char *status_buf)
{
    /*
     * parses the  status string of the printer to retrieve status
     * of the printer
     * Returns: BJNP_OK = printer available
     *          BJNP_PRINTER_BUSY = Printer busy
     */

    char argument[BJNP_ARG_MAX];
    unsigned int status;
    int printer_status;
    int ret = 0;

    if ((printer->reporting_capabilities & BJNP_REPORT_PRINTER_STATUS) == 0 ||
        (get_argument(status_buf, PRINTER_STATUS_TOKEN, argument, BJNP_ARG_MAX, "printer status") == 0)) {
        reset_capability(BJNP_REPORT_PRINTER_STATUS, printer->reporting_capabilities);
        return BJNP_OK;
    }

    if (sscanf(argument, "%2x", &status) == 1) {
        bjnp_debug(LOG_DEBUG,
                   "Read printer status: %u, Printing = %d, Busy = %d, Error = %d\n",
                   status,
                   ((status & BST_PRINTING) != 0),
                   ((status & BST_BUSY) != 0),
                   ((status & BST_ERROR) != 0));
        printer_status = status & (BST_PRINTING | BST_BUSY);

        if (printer_status) {
            ret = BJNP_PRINTER_BUSY;
        } else {
            ret = BJNP_OK;
        }

        if (status & BST_ERROR) {
            ret |= BJNP_PRINTER_ERROR;
        }

        return ret;
    }

    bjnp_debug(LOG_WARN, "Could not parse printer status for tag: %s!\n",
               PRINTER_STATUS_TOKEN);
    return  BJNP_OK;
}

static int
report_paper_status(printer_t *printer, char *status_buf, int first_output)
{
    /*
     * parses the  status string of the printer to retrieve paper status
     * of the printer
     * Returns: BJNP_OK
     *          BJNP_NO_PAPER
     */

    char argument[BJNP_ARG_MAX];

    if ((printer->reporting_capabilities & BJNP_REPORT_PAPER_STATUS) == 0 ||
        get_argument(status_buf, PAPER_STATUS_TOKEN, argument, BJNP_ARG_MAX, "paper status") == 0) {
        reset_capability(BJNP_REPORT_PAPER_STATUS, printer->reporting_capabilities);
        return BJNP_OK;
    }

    if (first_output) {

        /* on startup (first output) we set the out of paper state immediately */

        if (strcmp(argument, DJS_PAPER_OUT) == 0) {
            fputs("STATE: +media-empty-error\n", stderr);
            printer->paper_out = BJNP_PAPER_OUT_THRESHOLD;
        } else {
            fputs("STATE: -media-empty-error\n", stderr);
            printer->paper_out = 0;
        }
    } else {

        /* report paper out only when we see the condition reported a number of times */

        if (strcmp(argument, DJS_PAPER_OUT) == 0) {
            if (printer->paper_out < BJNP_PAPER_OUT_THRESHOLD) {
                printer->paper_out++;
                bjnp_debug(LOG_DEBUG, "Paperout, condition seen %d times\n",
                           printer->paper_out);

                if (printer->paper_out >= BJNP_PAPER_OUT_THRESHOLD) {
                    fputs("STATE: +media-empty-error\n", stderr);
                }
            }

        } else  {
            /* paper out condition not found, report so if applicable,
             * unless printing was cancelled on the printer
             */

            if (printer->paper_out >= BJNP_PAPER_OUT_THRESHOLD &&
                strcmp(argument, DJS_CANCELLING) != 0) {
                fputs("STATE: -media-empty-error\n", stderr);
                printer->paper_out = 0;
            }
        }
    }

    return printer->paper_out >= BJNP_PAPER_OUT_THRESHOLD ? BJNP_NO_PAPER : BJNP_OK;
}

int bjnp_report_levels(printer_t *printer)
{
    char status_buf[BJNP_STATUS_MAX];
    int ret = 0;

    if (bjnp_get_status(printer, status_buf) != BJNP_OK) {
        bjnp_debug(LOG_ERROR, "Cannot retrieve status, no level information!\n");
        return BJNP_OK;
    }

    report_marker_levels(printer, status_buf, printer->first_output);
    ret |= report_printer_status(printer, status_buf);
    ret |= report_ink_status_messages(printer, status_buf, printer->first_output);
    ret |= report_paper_status(printer, status_buf,  printer->first_output);
    printer->first_output = 0;
    return ret;
}
