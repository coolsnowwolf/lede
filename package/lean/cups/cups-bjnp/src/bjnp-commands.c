/*
 * Low level TCP and UDP IO communication implementation for
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

#include <errno.h>
#include <stdio.h>
#include "bjnp.h"
#include "bjnp-protocol.h"
#include "bjnp-commands.h"
#include "bjnp-io.h"

void
clear_cmd(bjnp_command_t *cmd)
{
    memset(cmd, 0, sizeof(*cmd));
}

static void
bjnp_set_command_header(uint8_t dev_type, uint16_t seq_no, uint16_t session_id, bjnp_command_t *cmd, char cmd_code,
                        int command_len)
{
    /*
     * Set command buffer with command code, session_id and lenght of payload
     * Returns: sequence number of command
     */
    memcpy(cmd->header.BJNP_id, BJNP_STRING, sizeof(cmd->header.BJNP_id));
    cmd->header.dev_type = dev_type;
    cmd->header.cmd_code = cmd_code;
    cmd->header.unknown1 = htons(0);
    cmd->header.payload_len = htonl(command_len - bjnp_header_size);
    cmd->header.seq_no = htons(seq_no);
    cmd->header.session_id = htons(session_id);
}

void bjnp_defaults_set_command_header(bjnp_command_t *cmd, char cmd_code,
                                     int command_len)
{
    bjnp_set_command_header(BJNP_CMD_PRINT, 0, 0, cmd, cmd_code, command_len);
}


int
bjnp_printer_set_command_header(printer_t *printer, bjnp_command_t *cmd, char cmd_code,
                        int command_len)
{
    bjnp_set_command_header( /* printer-> dev_type */ BJNP_CMD_PRINT, ++printer->serial,
                                    printer->session_id, cmd, cmd_code, command_len);
    return printer->serial;
}

static int
bjnp_process_udp_command(http_addr_t *addr, bjnp_command_t *command,
                         int cmd_len, bjnp_response_t *response)
{
    /*
     * Send UDP command and retrieve response
     * Returns: length of response or -1 in case of error
     */

    int sockfd;
    int numbytes;
    fd_set fdset;
    struct timeval timeout;
    int attempt;
    char ipaddress[128];
    int port;
    int size;
    char family[BJNP_FAMILY_MAX];

    get_address_info(addr, ipaddress, &port, family);
    bjnp_debug(LOG_DEBUG, "Sending UDP command to %s port %d(%s)\n",
               ipaddress, port, family);

    if ((sockfd = socket(get_protocol_family(*addr), SOCK_DGRAM, 0)) == -1) {
        bjnp_debug(LOG_CRIT, "bjnp_process_udp_command: sockfd - %s\n",
                   strerror(errno));
        return -1;
    }

    size = sa_size(*addr);

    if (connect(sockfd, &(addr->addr), (socklen_t) size)
        != 0) {
        bjnp_debug(LOG_CRIT, "bjnp_process_udp_command: connect - %s\n",
                   strerror(errno));
        close(sockfd);
        return -1;
    }

    for (attempt = 0; attempt < 3; attempt++) {
        if ((numbytes = send(sockfd, command, cmd_len, 0)) != cmd_len) {
            bjnp_debug(LOG_CRIT, "bjnp_process_udp_command: Sent only %d bytes of packet",
                       numbytes);
        }

        FD_ZERO(&fdset);
        FD_SET(sockfd, &fdset);
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        if (select(sockfd + 1, &fdset, NULL, NULL, &timeout) <= 0) {
            /* no data received OR error, in either case retry */
            continue;
        }

        if ((numbytes = recv(sockfd, response, sizeof(*response), MSG_WAITALL)) == -1) {
            bjnp_debug(LOG_CRIT, "bjnp_process_udp_command: no data received (recv)");
            continue;
        }

        close(sockfd);
        return numbytes;
    }

    /* max attempts reached, return failure */
    close(sockfd);
    return -1;
}

int
get_printer_id(http_addr_t *addr, char *model, char *IEEE1284_id)
{
    /*
     * get printer identity
     * Sets model (make and model) and IEEE1284_id
     */

    bjnp_command_t cmd;
    bjnp_response_t id;
    char printer_id[BJNP_IEEE1284_MAX];
    int resp_len;
    int id_len;
    int min_size;

    /* set defaults */

    strcpy(model, "Unidentified printer");
    strcpy(IEEE1284_id, "");

    clear_cmd(&cmd);
    bjnp_defaults_set_command_header(&cmd, CMD_UDP_GET_ID, sizeof(cmd.udp_get_id));

    bjnp_hexdump(LOG_DEBUG2, "Get printer identity", (char *) &cmd,
                 sizeof(cmd.udp_get_id));

    resp_len =
        bjnp_process_udp_command(addr, &cmd, sizeof(cmd.udp_get_id),
                                 &id);

    min_size = bjnp_header_size + sizeof(id.udp_identity_response.id_len);

    if (resp_len <= min_size) {
        return -1;
    }

    bjnp_hexdump(LOG_DEBUG2, "Printer identity:", &id, resp_len);

    id_len = ntohs(id.udp_identity_response.id_len) - sizeof(
                 id.udp_identity_response.id_len);

    /* check id_len */

    if ((id_len < 0) || (id_len > (resp_len - bjnp_header_size)) ||
        (id_len > BJNP_IEEE1284_MAX)) {
        bjnp_debug(LOG_ERROR,
                   "Id - length received is invalid: %d (total response length = %d\n",
                   id_len, resp_len);
        return -1;
    }

    /* set IEEE1284_id */

    if (printer_id != NULL) {
        strncpy(printer_id, id.udp_identity_response.id, id_len);
        printer_id[id_len] = '\0';
    }

    bjnp_debug(LOG_INFO, "Identity = %s\n", printer_id);

    if (IEEE1284_id != NULL) {
        strcpy(IEEE1284_id, printer_id);
    }

    /* get make&model from IEEE1284 id  */

    if (model != NULL) {
        model[0] = '\0';
        parse_IEEE1284_to_model(printer_id, model);
        bjnp_debug(LOG_DEBUG, "Printer model = %s\n", model);
    }

    return 0;
}

int
bjnp_get_status(printer_t *printer, char *status_buf)
{
    /*
     * get printer status
     */

    bjnp_command_t cmd;
    bjnp_response_t response;
    int resp_len;
    uint32_t status_len;

    /* set defaults */

    clear_cmd(&cmd);
    bjnp_printer_set_command_header(printer, &cmd, CMD_UDP_GET_STATUS,
                            sizeof(cmd.udp_get_status));

    bjnp_hexdump(LOG_DEBUG2, "Get printer status", (char *) &cmd,
                 sizeof(cmd.udp_get_status));

    resp_len =
        bjnp_process_udp_command(&(printer->printer_sa), &cmd,
                                 sizeof(cmd.udp_get_status),
                                 &response);

    if (resp_len <= 0) {
        return BJNP_IO_ERROR;
    }

    bjnp_hexdump(LOG_DEBUG2, "Printer status:", &response, resp_len);

    status_len = ntohl(response.udp_status_response.header.payload_len);

    if (status_len >= BJNP_STATUS_MAX ||  status_len < 16) {
        bjnp_debug(LOG_ERROR, "Get printer status: ERROR, invalid response length (%d)\n", status_len);
        return BJNP_IO_ERROR;
    }

    if (status_len != (resp_len - sizeof(response.udp_status_response.header))) {
        bjnp_debug(LOG_ERROR,
                   "status length (%d) does not match response length (%d)!!\n", status_len,
                   resp_len - sizeof(response.udp_status_response.header));
        return BJNP_IO_ERROR;
    }

    /* check which version of the response we received */

    if (strncmp(response.udp_status_response_v2.status, "<?xml", 5) == 0) {

        /* version 2 of the status response */

        response.udp_status_response_v2.status[status_len] = '\0';
        strcpy(status_buf, response.udp_status_response_v2.status);
    } else {
        /* old format response message */
        status_len = status_len - sizeof(response.udp_status_response.status_len);
        response.udp_status_response.status[status_len] = '\0';
        strcpy(status_buf, response.udp_status_response.status);
    }

    bjnp_debug(LOG_INFO, "Printer status: %s\n", status_buf);
    return BJNP_OK;
}

int bjnp_send_job_details(http_addr_t *addr, const char *user, const char *title)
{
    /*
     * send details of printjob to printer
     * Returns: 0 = ok, -1 = error
     */

    char hostname[BJNP_HOST_MAX];
    int resp_len;
    bjnp_command_t cmd;
    bjnp_response_t resp;
    int session_id = 0;

    /* send job details command */

    clear_cmd(&cmd);
    bjnp_defaults_set_command_header(&cmd, CMD_UDP_PRINT_JOB_DET,
                            sizeof(cmd.udp_job_details));

    /* create payload */

    gethostname(hostname, BJNP_HOST_MAX - 1);

    charTo2byte(cmd.udp_job_details.unknown, "",
                sizeof(cmd.udp_job_details.unknown));
    charTo2byte(cmd.udp_job_details.hostname, hostname,
                sizeof(cmd.udp_job_details.hostname));
    charTo2byte(cmd.udp_job_details.username, user,
                sizeof(cmd.udp_job_details.username));
    charTo2byte(cmd.udp_job_details.jobtitle, title, sizeof(cmd.udp_job_details));

    bjnp_hexdump(LOG_DEBUG2, "Job details", &cmd,
                 sizeof(cmd.udp_job_details));
    resp_len = bjnp_process_udp_command(addr, &cmd, sizeof(cmd.udp_job_details),
                                        &resp);

    if (resp_len > 0) {
        bjnp_hexdump(LOG_DEBUG2, "Job details response:", &resp,
                     resp_len);
        session_id = ntohs(resp.udp_print_job_details_response.header.session_id);
        return session_id;
    }

    return -1;
}

int bjnp_send_close(printer_t *printer)
{
    /*
     * Signal end of printjob to printer
     */

    int resp_len;
    bjnp_command_t cmd;
    bjnp_response_t resp;

    clear_cmd(&cmd);
    bjnp_printer_set_command_header(printer, &cmd, CMD_UDP_CLOSE, sizeof(cmd.udp_close));

    bjnp_hexdump(LOG_DEBUG2, "bjnp_send_close", (char *) &cmd,
                 sizeof(cmd.udp_close));
    resp_len =
        bjnp_process_udp_command(&(printer->printer_sa), &cmd, sizeof(cmd.udp_close),
                                 &resp);

    if (resp_len != sizeof(resp.udp_close_response)) {
        bjnp_debug(LOG_CRIT,
                   "Received %d characters in close response, expected %d\n",
                   resp_len, sizeof(resp.udp_close_response));
        return -1;
    }

    bjnp_hexdump(LOG_DEBUG2, "Finish printjob response", &resp, resp_len);
    return 0;
}

