/*
 * Low level TCP and UDP command function definitions for
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
#ifndef _CUPS_BJNP_COMMANDS_H_
#define _CUPS_BJNP_COMMANDS_H_

/*
 * bjnp-commands.c
 */
void clear_cmd(bjnp_command_t *cmd);
int bjnp_printer_set_command_header(printer_t *printer, bjnp_command_t *cmd,
                            char cmd_code, int payload_len);
void bjnp_defaults_set_command_header(bjnp_command_t *cmd,
                            char cmd_code, int payload_len);
int get_printer_id(http_addr_t *addr, char *model, char *IEEE1284_id);
int bjnp_get_status(printer_t *printer, char *status_buf);
int bjnp_send_job_details(http_addr_t *addr, const char *user, const char *title);
int bjnp_send_close(printer_t *printer);
#endif

