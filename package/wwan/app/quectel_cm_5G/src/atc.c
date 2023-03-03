/******************************************************************************
  @file    atc.c
  @brief   at command.

  DESCRIPTION
  Connectivity Management Tool for USB network adapter of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <getopt.h>
#include <poll.h>
#include <sys/time.h>
#include <endian.h>
#include <time.h>
#include <sys/types.h>
#include <limits.h>
#include <inttypes.h>

extern int asprintf(char **s, const char *fmt, ...);

#include "QMIThread.h"

#include "atchannel.h"
#include "at_tok.h"

static int asr_style_atc = 0;
static int s_pdp;
#define safe_free(__x) do { if (__x) { free((void *)__x); __x = NULL;}} while(0)
#define safe_at_response_free(__x) { if (__x) { at_response_free(__x); __x = NULL;}}

#define at_response_error(err, p_response) \
    (err \
    || p_response == NULL \
    || p_response->finalResponse == NULL \
    || p_response->success == 0)

static int atc_init(PROFILE_T *profile) {
    int err;
    char *cmd;
    ATResponse *p_response = NULL;

    if (profile->proxy[0])  {
        s_pdp = profile->pdp;
        err = at_send_command_singleline("AT+QNETDEVSTATUS=?", "+QNETDEVSTATUS:", &p_response);
        if (at_response_error(err, p_response))
            asr_style_atc = 1; //EC200T/EC100Y do not support this AT, but RG801/RG500U support
        safe_at_response_free(p_response);
		
        return err;
    }

    err = at_handshake();
    if (err) {
        dbg_time("handshake fail, TODO ... ");
        goto exit;
    }
	
    s_pdp = profile->pdp;
    at_send_command_singleline("AT+QCFG=\"usbnet\"", "+QCFG:", NULL);
    at_send_command_multiline("AT+QNETDEVCTL=?", "+QNETDEVCTL:", NULL);
    at_send_command("AT+CGREG=2", NULL); //GPRS Network Registration Status
    at_send_command("AT+CEREG=2", NULL); //EPS Network Registration Status
    at_send_command("AT+C5GREG=2", NULL); //5GS Network Registration Status

    err = at_send_command_singleline("AT+QNETDEVSTATUS=?", "+QNETDEVSTATUS:", &p_response);
    if (at_response_error(err, p_response))
        asr_style_atc = 1; //EC200T/EC100Y do not support this AT, but RG801/RG500U support 
    safe_at_response_free(p_response);

    err = at_send_command_singleline("AT+QCFG=\"NAT\"", "+QCFG:", &p_response);
    if (!at_response_error(err, p_response)) {
        int old_nat, new_nat = asr_style_atc ? 1 : 0;

        err = at_tok_scanf(p_response->p_intermediates->line, "%s%d", NULL, &old_nat);
        if (err == 2 && old_nat != new_nat) {
            safe_at_response_free(p_response);
            asprintf(&cmd, "AT+QCFG=\"NAT\",%d", new_nat);
            err = at_send_command(cmd, &p_response);
            safe_free(cmd);
            if (!at_response_error(err, p_response)) {
                err = at_send_command("at+cfun=1,1",NULL);
                if (!err)
                    g_donot_exit_when_modem_hangup = 1;
                //reboot to take effect
            }
            safe_at_response_free(p_response);
        }
        err = 0;
    }
    safe_at_response_free(p_response);

exit:
    return err;
}

static int atc_deinit(void) {
    return 0;
}

/**
 * Called by atchannel when an unsolicited line appears
 * This is called on atchannel's reader thread. AT commands may
 * not be issued here
 */
static void onUnsolicited (const char *s, const char *sms_pdu)
{
    (void)sms_pdu;

    if (strStartsWith(s, "+QNETDEVSTATUS:")) {
        qmidevice_send_event_to_main(RIL_UNSOL_DATA_CALL_LIST_CHANGED);
    }
    else if (strStartsWith(s, "+CGREG:")
        || strStartsWith(s, "+CEREG:")
        || strStartsWith(s, "+C5GREG:")) {
        qmidevice_send_event_to_main(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
    }
}

static void onTimeout(void) {
    dbg_time("%s", __func__);
    //TODO
}

static void onClose(void) {
    dbg_time("%s", __func__);
}

static void * atc_read_thread(void *param) {
    PROFILE_T *profile = (PROFILE_T *)param;
    const char *cdc_wdm = (const char *)profile->qmichannel;
    int wait_for_request_quit = 0;
    int atc_fd;

    atc_fd = cm_open_dev(cdc_wdm);
    if (atc_fd <= 0) {
        dbg_time("fail to open (%s), errno: %d (%s)", cdc_wdm, errno, strerror(errno));
        goto __quit;
    }

    dbg_time("atc_fd = %d", atc_fd);

    if (at_open(atc_fd, onUnsolicited, 0))
        goto __quit;

    at_set_on_timeout(onTimeout);
    at_set_on_reader_closed(onClose);
    qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_CONNECTED);

    while (atc_fd > 0) {
        struct pollfd pollfds[] = {{atc_fd, POLLIN, 0}, {qmidevice_control_fd[1], POLLIN, 0}};
        int ne, ret, nevents = 2;

        ret = poll(pollfds, nevents, wait_for_request_quit ? 1000 : -1);

        if (ret == 0 && wait_for_request_quit) {
            break;
        }

        if (ret < 0) {
            dbg_time("%s poll=%d, errno: %d (%s)", __func__, ret, errno, strerror(errno));
            break;
        }

        for (ne = 0; ne < nevents; ne++) {
            int fd = pollfds[ne].fd;
            short revents = pollfds[ne].revents;

            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                dbg_time("%s poll err/hup/inval", __func__);
                dbg_time("epoll fd = %d, events = 0x%04x", fd, revents);
                if (revents & (POLLERR | POLLHUP | POLLNVAL))
                goto __quit;
            }

            if ((revents & POLLIN) == 0)
                continue;

            if (atc_fd == fd) {
                usleep(10*1000); //let atchannel.c read at response.
            }
            else if (fd == qmidevice_control_fd[1]) {
                int triger_event;
                if (read(fd, &triger_event, sizeof(triger_event)) == sizeof(triger_event)) {
                    //dbg_time("triger_event = 0x%x", triger_event);
                    switch (triger_event) {
                        case RIL_REQUEST_QUIT:
                            goto __quit;
                        break;
                        case SIG_EVENT_STOP:
                            wait_for_request_quit = 1;
                        break;
                        default:
                        break;
                    }
                }
            }
        }
    }

__quit:
    at_close();
    qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_DISCONNECTED);
    dbg_time("%s exit", __func__);

    return NULL;
}

const struct qmi_device_ops atc_dev_ops = {
    .init = atc_init,
    .deinit = atc_deinit,
    .read = atc_read_thread,
};

static int requestBaseBandVersion(PROFILE_T *profile) {
    int retVal = -1;
    int err;
    ATResponse *p_response = NULL;

    (void)profile;

    err = at_send_command_multiline("AT+CGMR", "\0", &p_response);
    if (at_response_error(err, p_response))
        goto exit;

    if (p_response->p_intermediates && p_response->p_intermediates->line) {
        strncpy(profile->BaseBandVersion, p_response->p_intermediates->line, sizeof(profile->BaseBandVersion) - 1);
        retVal = 0;
    }

exit:
    safe_at_response_free(p_response);
    return retVal;
}

static int requestGetSIMStatus(SIM_Status *pSIMStatus)
{
    int err;
    ATResponse *p_response = NULL;
    char *cpinLine;
    char *cpinResult;
    int ret = SIM_NOT_READY;

    err = at_send_command_singleline("AT+CPIN?", "+CPIN:", &p_response);
    if (at_response_error(err, p_response))
        goto done;

    switch (at_get_cme_error(p_response))
    {
    case CME_SUCCESS:
        break;

    case CME_SIM_NOT_INSERTED:
    case CME_OPERATION_NOT_ALLOWED:
    case CME_FAILURE:
        ret = SIM_ABSENT;
        goto done;

    default:
        ret = SIM_NOT_READY;
        goto done;
    }

    cpinLine = p_response->p_intermediates->line;
    err = at_tok_start (&cpinLine);

    if (err < 0)
    {
        ret = SIM_NOT_READY;
        goto done;
    }

    err = at_tok_nextstr(&cpinLine, &cpinResult);

    if (err < 0)
    {
        ret = SIM_NOT_READY;
        goto done;
    }

    if (0 == strcmp (cpinResult, "SIM PIN"))
    {
        ret = SIM_PIN;
        goto done;
    }
    else if (0 == strcmp (cpinResult, "SIM PUK"))
    {
        ret = SIM_PUK;
        goto done;
    }
    else if (0 == strcmp (cpinResult, "PH-NET PIN"))
    {
        return SIM_NETWORK_PERSONALIZATION;
    }
    else if (0 != strcmp (cpinResult, "READY"))
    {
        /* we're treating unsupported lock types as "sim absent" */
        ret = SIM_ABSENT;
        goto done;
    }

    ret = SIM_READY;

done:
    safe_at_response_free(p_response);
    *pSIMStatus = ret;
    return err;
}

static int requestEnterSimPin(const char *pPinCode) {
    int retVal = -1;
    int err;
    ATResponse *p_response = NULL;
    char *cmd = NULL;

    asprintf(&cmd, "AT+CPIN=%s", pPinCode);
    err = at_send_command(cmd, NULL);
    safe_free(cmd);

    if (!at_response_error(err, p_response)) {
        retVal = 0;
    }

    safe_at_response_free(p_response);
    return retVal;
}

static int requestSetProfile(PROFILE_T *profile) {
    int err;
    ATResponse *p_response = NULL;
    char *cmd = NULL;
    const char *new_apn = profile->apn ? profile->apn : "";
    const char *new_user = profile->user ? profile->user : "";
    const char *new_password = profile->password ? profile->password : "";
    const char *ipStr[] = {"NULL", "IPV4", "IPV6", "IPV4V6"};

    dbg_time("%s[%d] %s/%s/%s/%d/%s", __func__,
        profile->pdp, profile->apn, profile->user, profile->password,
        profile->auth,ipStr[profile->iptype]);

    if ( !strcmp(profile->old_apn, new_apn) && !strcmp(profile->old_user, new_user)
        && !strcmp(profile->old_password, new_password)
        && profile->old_iptype == profile->iptype
        && profile->old_auth == profile->auth)
    {
        dbg_time("no need to set skip the rest");
        return 0;
    }

    asprintf(&cmd, "AT+QICSGP=%d,%d,\"%s\",\"%s\",\"%s\",%d",
        profile->pdp, profile->iptype, new_apn, new_user, new_password, profile->auth);
    err = at_send_command(cmd, &p_response);
    safe_free(cmd);
    if (at_response_error(err, p_response)) {
        safe_at_response_free(p_response);
        asprintf(&cmd, "AT+CGDCONT=%d,\"%s\",\"%s\"", profile->pdp, ipStr[profile->iptype], new_apn);
        err = at_send_command(cmd, &p_response);
        safe_free(cmd);
    }

    safe_at_response_free(p_response);
    return 1;
}

static int requestGetProfile(PROFILE_T *profile) {
    int retVal = -1;
    int err;
    ATResponse *p_response = NULL;
    char *cmd = NULL;
    int pdp;
    int old_iptype = 1; // 1 ~ IPV4, 2 ~ IPV6, 3 ~ IPV4V6
    char *old_apn = "", *old_user = "", *old_password = "";
    int old_auth = 0;
    const char *ipStr[] = {"NULL", "IPV4", "IPV6", "IPV4V6"};

    if (profile->enable_ipv4 && profile->enable_ipv6)
        profile->iptype = 3;
    else if (profile->enable_ipv6)
        profile->iptype = 2;
    else
        profile->iptype = 1;

_re_check:
    asprintf(&cmd, "AT+QICSGP=%d", profile->pdp);
    err = at_send_command_singleline(cmd, "+QICSGP:", &p_response);
    safe_free(cmd);
    if (err == AT_ERROR_INVALID_RESPONSE && p_response == NULL) {
        //bug of RG801H
        safe_at_response_free(p_response);
        asprintf(&cmd, "AT+QICSGP=%d,%d,\"\",\"\",\"\",0", profile->pdp, profile->iptype);
        err = at_send_command(cmd, &p_response);
        safe_free(cmd);
        if (!at_response_error(err, p_response)) {
            safe_at_response_free(p_response);
            goto _re_check;
        }
    }

    if (!at_response_error(err, p_response)) {
        err = at_tok_scanf(p_response->p_intermediates->line,
            "%d%s%s%s%d", &old_iptype, &old_apn,  &old_user, &old_password, &old_auth);

        if (err != 4 || pdp != profile->pdp)
            goto _error;
    }
    else {
        ATLine *atLine = NULL;
        char *cgdcont_iptype = NULL;

        safe_at_response_free(p_response);
        err = at_send_command_multiline("AT+CGDCONT?", "+CGDCONT:", &p_response);
        if (at_response_error(err, p_response))
            goto _error;

        atLine = p_response->p_intermediates;
        while (atLine) {
            err = at_tok_scanf(atLine->line, "%d%s%s", &pdp, &cgdcont_iptype, &old_apn);
            if (err == 3 && pdp == profile->pdp) {
                if (!strcasecmp(cgdcont_iptype, ipStr[3]))
                    old_iptype = 3;
                else if (!strcasecmp(cgdcont_iptype, ipStr[2]))
                     old_iptype = 2;
                else
                    old_iptype = 1;
               break;
            }
            old_apn = NULL;
            atLine = atLine->p_next;
        }
    }

    retVal = 0;

_error:
    if (!old_apn) old_apn = "";
    if (!old_user) old_user = "";
    if (!old_password) old_password = "";

    strncpy(profile->old_apn, old_apn, sizeof(profile->old_apn));
    strncpy(profile->old_user, old_user, sizeof(profile->old_user));
    strncpy(profile->old_password, old_password, sizeof(profile->old_password));
    profile->old_auth = old_auth;
    profile->old_iptype = old_iptype; 

    dbg_time("%s[%d] %s/%s/%s/%d/%s", __func__,
        profile->pdp, profile->old_apn, profile->old_user, profile->old_password,
        profile->old_auth, ipStr[profile->old_iptype]);

    safe_at_response_free(p_response);

    return retVal;
}

static int requestRegistrationState(UCHAR *pPSAttachedState) {
    int retVal = -1;
    int err;
    ATResponse *p_response = NULL;
    ATLine *p_cur;
    int i;
    int cops_act = -1;
    int state = 0, lac = 0, cid = 0, act = 0;
    int commas;
    char *line;

    *pPSAttachedState = 0;

    err = at_send_command_multiline(
              "AT+COPS=3,0;+COPS?;+COPS=3,1;+COPS?;+COPS=3,2;+COPS?",
              "+COPS:", &p_response);
    if (at_response_error(err, p_response))
        goto error;

/*
AT< +COPS: 0,0,"CHINA MOBILE",13
AT< +COPS: 0,1,"CMCC",13
AT< +COPS: 0,2,"46000",13
AT< OK
*/
    retVal = 0;
    for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next, i++) {
        err = at_tok_scanf(p_cur->line, "%d%d%s%d", NULL, NULL, NULL, &cops_act);
        if (err != 4) goto error;

        break;
    }

    safe_at_response_free(p_response);
    switch (cops_act) {
        case 2: //UTRAN
        case 4: //UTRAN W/HSDPA
        case 5: //UTRAN W/HSUPA
        case 6: //UTRAN W/HSDPA and HSUPA
            //AT+CGREG  GPRS Network Registration Status
            err = at_send_command_singleline("AT+CGREG?", "+CGREG:", &p_response);
        break;

        case 7: //E-UTRAN
        case 13: //E-UTRAN-NR dual connectivity
            //AT+CEREG  EPS Network Registration Status
            err = at_send_command_singleline("AT+CEREG?", "+CEREG:", &p_response);
        break;

        case 10: //E-UTRAN connected to a 5GCN
        case 11: //NR connected to a 5GCN
        case 12: //NG-RAN
            //AT+C5GREG  5GS Network Registration Status
            err = at_send_command_singleline("AT+C5GREG?", "+C5GREG:", &p_response);
        break;

        default:
            goto error;
        break;
    }

    if (at_response_error(err, p_response))
        goto error;
    if (!p_response->p_intermediates || !p_response->p_intermediates->line) goto error;

    line = p_response->p_intermediates->line;
    commas = at_tok_count(line);
     switch (commas) {
        case 0: /* +CREG: <stat> */
            err = at_tok_nextint(&line, &state);
            if (err < 0) goto error;
            break;

        case 1: /* +CREG: <n>, <stat> */
            err = at_tok_scanf(line, "%d%d", NULL, &state);
            if (err != 2) goto error;
            break;

        case 2: /* +CREG: <stat>, <lac>, <cid> */
            err = at_tok_scanf(line, "%d%x%x", NULL, &state, &lac, &cid);
            if (err != 3) goto error;
            break;

        case 3: /* +CREG: <n>, <stat>, <lac>, <cid> */
            err = at_tok_scanf(line, "%d%d%x%x", NULL, &state, &lac, &cid);
            if (err != 4) goto error;
            break;

        case 4: //<n>, <stat>, <lac>, <cid>, <networkType> */
        case 5:
        case 6: 
        case 7:
            err = at_tok_scanf(line, "%d%d%x%x%d", NULL, &state, &lac, &cid, &act);
            if (err != 5) goto error;
        break;

        default:
            goto error;
    }

    //dbg_time("state=%d", state);

    if (state == 1 || state == 5) { //Registered, home network  / roaming
        *pPSAttachedState = 1;
    }

error:
    safe_at_response_free(p_response);
    return retVal;
}

static int requestSetupDataCall(PROFILE_T *profile, int curIpFamily) {
    int err;
    ATResponse *p_response = NULL;
    char *cmd = NULL;
    ATLine *p_cur = NULL;
    int pdp = profile->pdp;
    int state = 0;

    (void)curIpFamily;

    if (strStartsWith(profile->BaseBandVersion, "RG801H") || strStartsWith(profile->BaseBandVersion, "EC200H")) {
        //RG801H will miss USB_CDC_NOTIFY_NETWORK_CONNECTION
        asprintf(&cmd, "ifconfig %s up", profile->usbnet_adapter);
        if (system(cmd)) {};
        safe_free(cmd);
    }

    if (asr_style_atc) {
        err = at_send_command_multiline("AT+CGACT?", "+CGACT:", &p_response);
        if (at_response_error(err, p_response))
            goto _error;

        for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next) {
            int cid = 0;
			state = 0;
			
            err = at_tok_scanf(p_cur->line, "%d%d", &cid, &state);
            if (cid == pdp)
                break;
			else if(state)
				state = 0;
        }
        safe_at_response_free(p_response);

        if (state == 0) {
            asprintf(&cmd, "AT+CGACT=1,%d", pdp);
            err = at_send_command(cmd, &p_response);
            safe_free(cmd);
            if (at_response_error(err, p_response))
                goto _error;
        }
    }

    if(asr_style_atc)
        asprintf(&cmd, "AT+QNETDEVCTL=1,%d,%d", pdp, 1);
    else
        asprintf(&cmd, "AT+QNETDEVCTL=%d,1,%d", pdp, 1);
    err = at_send_command(cmd, &p_response);
    safe_free(cmd);

    if (at_response_error(err, p_response))
        goto _error;

    if (!asr_style_atc) { //TODO some modems do not sync return setup call resule
        int t = 0;

        while (t++ < 15) {
            asprintf(&cmd, "AT+QNETDEVSTATUS=%d", pdp);
            err = at_send_command_singleline(cmd, "+QNETDEVSTATUS", &p_response);
            safe_free(cmd);
            if (err) goto _error;

            if (!at_response_error(err, p_response)) {
                break;
            }
            safe_at_response_free(p_response);
            sleep(1);
        }
    }

    //some modem do not report URC
    qmidevice_send_event_to_main(RIL_UNSOL_DATA_CALL_LIST_CHANGED);

_error:
    safe_at_response_free(p_response);
    //dbg_time("%s err=%d", __func__, err);
    return err;
}

static int at_netdevstatus(int pdp, unsigned int *pV4Addr) {
    int err;
    ATResponse *p_response = NULL;
    char *cmd = NULL;
    char *ipv4_address = NULL;
    char *ipv4_gate = NULL;
    char *ipv4_DHCP = NULL;
    char *ipv4_pDNS = NULL;
    char *ipv4_sDNS = NULL;
    char *ipv6_address = NULL;
    char *ipv6_gate = NULL;
    char *ipv6_DHCP = NULL;
    char *ipv6_pDNS = NULL;
    char *ipv6_sDNS = NULL;

    *pV4Addr = 0;

    asprintf(&cmd, "AT+QNETDEVSTATUS=%d", pdp);
    err = at_send_command_singleline(cmd, "+QNETDEVSTATUS", &p_response);
    safe_free(cmd);
    if (at_response_error(err, p_response)) goto _error;
    if (!p_response->p_intermediates || !p_response->p_intermediates->line) goto _error;

    err = at_tok_scanf(p_response->p_intermediates->line, "%s%s%s%s%s%s%s%s%s%s",
        &ipv4_address, &ipv4_gate, &ipv4_DHCP, &ipv4_pDNS, &ipv4_sDNS,
        &ipv6_address, &ipv6_gate, &ipv6_DHCP, &ipv6_pDNS, &ipv6_sDNS);
    if (err > 0) {
#if 0
        dbg_time("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
            ipv4_address, ipv4_gate, ipv4_DHCP, ipv4_pDNS, ipv4_sDNS,
            ipv6_address, ipv6_gate, ipv6_DHCP, ipv6_pDNS, ipv6_sDNS);
#endif

        if (ipv4_address && ipv4_address[0]) {
            int addr[4] = {0, 0, 0, 0};

            if (strstr(ipv4_address, ".")) {
                sscanf(ipv4_address, "%d.%d.%d.%d", &addr[0], &addr[1], &addr[2], &addr[3]);
            }
            else {
                sscanf(ipv4_address, "%02X%02X%02X%02X", &addr[3], &addr[2], &addr[1], &addr[0]);
            }
            *pV4Addr = (addr[0]) | (addr[1]<<8) | (addr[2]<<16) | (addr[3]<<24);
       }
    }

_error:
    safe_at_response_free(p_response);
    return 0;
}

static int requestQueryDataCall(UCHAR  *pConnectionStatus, int curIpFamily) {
    int err;
    ATResponse *p_response = NULL;
    ATLine *p_cur = NULL;
    int state = 0;
    int bind = 0;
    int cid;
    int pdp = s_pdp;
    unsigned int v4Addr = 0;

    (void)curIpFamily;

    *pConnectionStatus = QWDS_PKT_DATA_DISCONNECTED;

    if (!asr_style_atc) {
        err = at_netdevstatus(pdp, &v4Addr);
        if (!err && v4Addr) {
            *pConnectionStatus = QWDS_PKT_DATA_CONNECTED;
            //if (profile->ipv4.Address == v4Addr) {} //TODO
        }
        return err;
    }

    err = at_send_command_multiline("AT+QNETDEVCTL?", "+QNETDEVCTL:", &p_response);
    if (at_response_error(err, p_response))
        goto _error;

    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next)
    {
        //+QNETDECTL:<op>,<cid>,<urc_en>,<state>
        err = at_tok_scanf(p_cur->line, "%d%d%d%d", &bind, &cid, NULL, &state);
        if (err != 4 || cid != pdp)
            continue;
        if (bind != 1)
            bind = 0;
    }
    safe_at_response_free(p_response);

    if (bind == 0 || state == 0)
        goto _error;

    err = at_send_command_multiline("AT+CGACT?", "+CGACT:", &p_response);
    if (at_response_error(err, p_response))
        goto _error;

    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next)
    {
		state = 0;
        err = at_tok_scanf(p_cur->line, "%d%d", &cid, &state);
        if (cid == pdp)
            break;
		else if(state)
			state = 0;
    }
    safe_at_response_free(p_response);

    if (bind && state)
        *pConnectionStatus = QWDS_PKT_DATA_CONNECTED;

_error:
    safe_at_response_free(p_response);
    //dbg_time("%s err=%d, call_state=%d", __func__, err, *pConnectionStatus);
    return 0;
}

static int requestDeactivateDefaultPDP(PROFILE_T *profile, int curIpFamily) {
    char *cmd = NULL;
    int pdp = profile->pdp;

    (void)curIpFamily;

    if (asr_style_atc)
        asprintf(&cmd, "AT+QNETDEVCTL=0,%d,%d", pdp, 0);
    else
        asprintf(&cmd, "AT+QNETDEVCTL=%d,0,%d", pdp, 0);
    at_send_command(cmd, NULL);
    safe_free(cmd);

    //dbg_time("%s err=%d", __func__, err);
    return 0;
}

static int requestGetIPAddress(PROFILE_T *profile, int curIpFamily) {
    int err;
    ATResponse *p_response = NULL;
    char *cmd = NULL;
    ATLine *p_cur = NULL;
    int pdp = profile->pdp;
    unsigned int v4Addr = 0;

    (void)curIpFamily;

    if (!asr_style_atc) {
        err = at_netdevstatus(pdp, &v4Addr);
        goto _error;
    }

    asprintf(&cmd, "AT+CGPADDR=%d", profile->pdp);
    err = at_send_command_singleline(cmd, "+CGPADDR:", &p_response);
    safe_free(cmd);
    if (at_response_error(err, p_response))
        goto _error;

    //+CGPADDR: 1,"10.201.80.91","2409:8930:4B3:41C7:F9B8:3D9B:A2F7:CA96"
    for (p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next)
    {
        char *ipv4 = NULL;
        char *ipv6 = NULL;

        err = at_tok_scanf(p_cur->line, "%d%s%s", &pdp, &ipv4, &ipv6);
        if (err < 2 || pdp != profile->pdp)
            continue;

        if (ipv4) {
            int addr[4] = {0, 0, 0, 0};

            sscanf(ipv4, "%d.%d.%d.%d", &addr[0], &addr[1], &addr[2], &addr[3]);
            v4Addr = (addr[0]) | (addr[1]<<8) | (addr[2]<<16) | (addr[3]<<24);
            break;
        }
    }

_error:
    if (v4Addr && profile->ipv4.Address != v4Addr) {
        unsigned char *v4 = (unsigned char *)&v4Addr;

        profile->ipv4.Address = v4Addr;
        dbg_time("%s %d.%d.%d.%d", __func__, v4[0], v4[1], v4[2], v4[3]);    
    }
        
    //dbg_time("%s err=%d", __func__, err);
    return v4Addr ? 0 : -1;
}

static int requestGetSignalInfo(void) {
    int retVal = -1;
    int err;
    ATResponse *p_response = NULL;
    int i;
    ATLine *p_cur = NULL;
    char *rat = NULL;
    int cops_act = 0;
    int is_nr5g_nsa = 0, nr5g_sa = 0;
    int verbose = 0;

    err = at_send_command_singleline("at+cops?", "+COPS:", &p_response);
    if (at_response_error(err, p_response)) goto _error;
    if (!p_response->p_intermediates || !p_response->p_intermediates->line) goto _error;

    retVal = 0;
    err = at_tok_scanf(p_response->p_intermediates->line, "%d%d%s%d", NULL, NULL, NULL, &cops_act);
    if (err != 4) goto _error;

    nr5g_sa = (cops_act == 11);

    safe_at_response_free(p_response);
    err = at_send_command_multiline("at+qeng=\"servingcell\"", "+QENG:", &p_response);
    if (at_response_error(err, p_response))
        goto _error;

    for (i = 0, p_cur = p_response->p_intermediates; p_cur != NULL; p_cur = p_cur->p_next, i++) {
        char *type, *state;

        err = at_tok_scanf(p_cur->line, "%s%s", &type, &state);
        if (err != 2 || strcmp(type, "servingcell"))
            continue;

        if (!strcmp(state, "SEARCH") || !strcmp(state, "LIMSRV"))
            continue;

        if (!strcmp(state, "NOCONN") || !strcmp(state, "CONNECT")) {
            err = at_tok_scanf(p_cur->line, "%s%s%s", &type, &state, &rat);
            if (err != 3)
                continue;        
        }
        else {
            rat = state;
        }
       
        if (!strcmp(rat, "NR5G-SA"))
        {
            //+QENG: "servingcell",<state>,"NR5G-SA",<duplex_mode>,<MCC>,<MNC>,<cellID>,<PCID>,<TAC>,<ARFCN>,<band>,<NR_DL_bandwidth>,<RSRP>,<RSRQ>,<SINR>,<tx_power>,<srxlev> 
            //+QENG: "servingcell","NOCONN","NR5G-SA","TDD", 454,12,0,21,4ED,636576,78,3,-85,-11,32,0,5184
            struct qeng_servingcell_nr5g_sa {
                char *cell_type, *state, *rat, *is_tdd;
                int MCC, MNC, cellID/*hex*/;
                int PCID, TAC/*hex*/, ARFCN;
                int band, NR_DL_bandwidth;
                int RSRP, RSRQ, RSSI, SINR;
            };
            struct qeng_servingcell_nr5g_sa nr5g_sa;

            memset(&nr5g_sa, 0, sizeof(nr5g_sa));
            err = at_tok_scanf(p_cur->line, "%s,%s,%s,%s,%d,%d,%x,%d,%x,%d,%d,%d,%d,%d,%d,%d",
                &nr5g_sa.cell_type, &nr5g_sa.state, &nr5g_sa.rat, &nr5g_sa.is_tdd,
                &nr5g_sa.MCC, &nr5g_sa.MNC, &nr5g_sa.cellID, &nr5g_sa.PCID, &nr5g_sa.TAC,
                &nr5g_sa.ARFCN, &nr5g_sa.band, &nr5g_sa.NR_DL_bandwidth,
                &nr5g_sa.RSRP, &nr5g_sa.RSRQ, &nr5g_sa.RSSI, &nr5g_sa.SINR);

            if (err >= 13 && verbose) {
                dbg_time("%s,%s,%s,%s,%d,%d,%x,%d,%x,%d,%d,%d,%d,%d,%d,%d",
                    nr5g_sa.cell_type, nr5g_sa.state, nr5g_sa.rat, nr5g_sa.is_tdd,
                    nr5g_sa.MCC, nr5g_sa.MNC, nr5g_sa.cellID, nr5g_sa.PCID, nr5g_sa.TAC,
                    nr5g_sa.ARFCN, nr5g_sa.band, nr5g_sa.NR_DL_bandwidth,
                    nr5g_sa.RSRP, nr5g_sa.RSRQ, nr5g_sa.RSSI, nr5g_sa.SINR);
            }
        }
        else if (!strcmp(rat, "NR5G-NSA"))
        {
            //+QENG: "NR5G-NSA",<MCC>,<MNC>,<PCID>,<RSRP>,< SINR>,<RSRQ>,<ARFCN>,<band>
            struct qeng_servingcell_nr5g_nsa {
                char *mcc, *mnc;
                int pcid, rsrp, sinr, rsrq;
            };
            struct qeng_servingcell_nr5g_nsa nr5g_nsa;
           
            memset(&nr5g_nsa, 0, sizeof(nr5g_nsa));
            err = at_tok_scanf(p_cur->line, "%s%s%s%s%d%d%d%dd",
                NULL, NULL, &nr5g_nsa.mcc, &nr5g_nsa.mnc, &nr5g_nsa.pcid, &nr5g_nsa.rsrp, &nr5g_nsa.sinr, &nr5g_nsa.rsrq);
            if (err == 8 && verbose)
            {
                dbg_time("mcc=%s, mnc=%s, pcid=%d, rsrp=%d, sinr=%d, rsrq=%d",
                    nr5g_nsa.mcc, nr5g_nsa.mnc, nr5g_nsa.pcid, nr5g_nsa.rsrp, nr5g_nsa.sinr, nr5g_nsa.rsrq);
            }

            is_nr5g_nsa = 1;
        }
        else if (!strcmp(rat, "LTE"))
        {
            //+QENG: "LTE",<is_tdd>,<MCC>,<MNC>,<cellID>,<PCID>,<earfcn>,<freq_band_ind>,<UL_bandwidth>,<DL_bandwidth>,<TAC>,<RSRP>,<RSRQ>,<RSSI>,<SINR>,<CQI>,<tx_power>,<srxlev>
            struct qeng_servingcell_lte {
                char *is_tdd, *mcc, *mnc;
                int cellID/*hex*/, pcid, earfcn, freq_band_ind, ul_bandwidth, dl_bandwidth;
                int tac/*hex*/, rsrp, rsrq, rssi, sinr, cqi,tx_power,srxlev;
            };
            struct qeng_servingcell_lte lte;

            memset(&lte, 0, sizeof(lte));
            if (!strcmp(rat, state))
                err = at_tok_scanf(p_cur->line, "%s%s%s%s%s%x%d%d%d%d%d%x%d%d%d%d%d%d%d",
                    NULL, NULL, &lte.is_tdd, &lte.mcc, &lte.mnc,
                    &lte.cellID, &lte.pcid, &lte.earfcn, &lte.freq_band_ind, &lte.ul_bandwidth, &lte.dl_bandwidth,
                    &lte.tac, &lte.rsrp, &lte.rsrq, &lte.rssi, &lte.sinr, &lte.cqi, &lte.tx_power, &lte.srxlev);
            else
                 err = at_tok_scanf(p_cur->line, "%s%s%s%s%s%s%x%d%d%d%d%d%x%d%d%d%d%d%d%d",
                    NULL, NULL, NULL, &lte.is_tdd, &lte.mcc, &lte.mnc,
                    &lte.cellID, &lte.pcid, &lte.earfcn, &lte.freq_band_ind, &lte.ul_bandwidth, &lte.dl_bandwidth,
                    &lte.tac, &lte.rsrp, &lte.rsrq, &lte.rssi, &lte.sinr, &lte.cqi, &lte.tx_power, &lte.srxlev);
            
                if (err >= 18 && verbose)
                {
                    dbg_time("is_tdd=%s, mcc=%s, mnc=%s", lte.is_tdd, lte.mcc, lte.mnc);
                    dbg_time("cellID=%x, pcid=%d, earfcn=%d", lte.cellID, lte.pcid, lte.earfcn);
                    dbg_time("freq_band_ind=%d, ul_bandwidth=%d, dl_bandwidth=%d", lte.freq_band_ind, lte.ul_bandwidth, lte.dl_bandwidth);
                    dbg_time("tac=%x, rsrp=%d, rsrq=%d, rssi=%d, sinr=%d", lte.tac, lte.rsrp, lte.rsrq, lte.rssi, lte.sinr);
                    dbg_time("cqi=%d, tx_power=%d, earfcn=%d", lte.cqi, lte.tx_power, lte.srxlev);
                }
        }
    }

    if (is_nr5g_nsa) {
        int endc_avl, plmn_info_list_r15_avl, endc_rstr, nr5g_basic;

        is_nr5g_nsa = 0;
        safe_at_response_free(p_response);
        err = at_send_command_multiline("at+qendc", "+QENDC:", &p_response);
        if (at_response_error(err, p_response)) goto _error;
        if (!p_response->p_intermediates || !p_response->p_intermediates->line) goto _error;

        err = at_tok_scanf(p_response->p_intermediates->line, "%d%d%d%d",
            &endc_avl, &plmn_info_list_r15_avl, &endc_rstr, &nr5g_basic);
        if (err == 4 && nr5g_basic) {
            is_nr5g_nsa = 1;
        }
    }

    if (verbose)
        dbg_time("cops_act=%d, nr5g_nsa=%d, nr5g_sa=%d", cops_act, is_nr5g_nsa, nr5g_sa);

_error:
    safe_at_response_free(p_response);
    return retVal;
}

static int requestGetICCID(void) {
    int retVal = -1;
    int err;
    ATResponse *p_response = NULL;
    char *iccid;

    err = at_send_command_singleline("AT+QCCID", "+QCCID:", &p_response);
    if (at_response_error(err, p_response)) goto _error;
    if (!p_response->p_intermediates || !p_response->p_intermediates->line) goto _error;

    err = at_tok_scanf(p_response->p_intermediates->line, "%s", &iccid);
    if (err != 1) goto _error;

    if (iccid && iccid[0]) {
        dbg_time("%s %s", __func__, iccid);
        retVal = 0;
    }

_error:
    safe_at_response_free(p_response);
    return retVal;
}

static int requestGetIMSI(void) {
    int retVal = -1;
    int err;
    ATResponse *p_response = NULL;
    char *imsi;

    err = at_send_command_numeric("AT+CIMI", &p_response);
    if (at_response_error(err, p_response)) goto _error;
    if (!p_response->p_intermediates || !p_response->p_intermediates->line) goto _error;

    imsi = p_response->p_intermediates->line;
    if (imsi) {
        dbg_time("%s %s", __func__, imsi);
        retVal = 0;
    }

_error:
    safe_at_response_free(p_response);
    return retVal;
}

const struct request_ops atc_request_ops = {
    .requestBaseBandVersion = requestBaseBandVersion,
    .requestGetSIMStatus = requestGetSIMStatus,
    .requestEnterSimPin = requestEnterSimPin,
    .requestSetProfile = requestSetProfile,
    .requestGetProfile = requestGetProfile,
    .requestRegistrationState = requestRegistrationState,
    .requestSetupDataCall = requestSetupDataCall,
    .requestQueryDataCall = requestQueryDataCall,
    .requestDeactivateDefaultPDP = requestDeactivateDefaultPDP,
    .requestGetIPAddress = requestGetIPAddress,
    .requestGetSignalInfo = requestGetSignalInfo,
    .requestGetICCID = requestGetICCID,
    .requestGetIMSI = requestGetIMSI,
};

