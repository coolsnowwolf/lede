/******************************************************************************
  @file    GobiNetCM.c
  @brief   GobiNet driver.

  DESCRIPTION
  Connectivity Management Tool for USB network adapter of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <ctype.h>
#include "QMIThread.h"

#ifdef CONFIG_GOBINET
static int qmiclientId[QMUX_TYPE_ALL];

// IOCTL to generate a client ID for this service type
#define IOCTL_QMI_GET_SERVICE_FILE 0x8BE0 + 1

// IOCTL to get the VIDPID of the device
#define IOCTL_QMI_GET_DEVICE_VIDPID 0x8BE0 + 2

// IOCTL to get the MEID of the device
#define IOCTL_QMI_GET_DEVICE_MEID 0x8BE0 + 3

static int GobiNetSendQMI(PQCQMIMSG pRequest) {
    int ret, fd;

    fd = qmiclientId[pRequest->QMIHdr.QMIType];
    pRequest->QMIHdr.ClientId = (fd&0xFF) ? fd&0xFF : pRequest->QMIHdr.QMIType;

    if (fd <= 0) {
        dbg_time("%s QMIType: %d has no clientID", __func__, pRequest->QMIHdr.QMIType);
        return -ENODEV;
    }

    // Always ready to write
    if (1 == 1) {
        ssize_t nwrites = le16_to_cpu(pRequest->QMIHdr.Length) + 1 - sizeof(QCQMI_HDR);
        ret = write(fd, &pRequest->MUXMsg, nwrites);
        if (ret == nwrites) {
            ret = 0;
        } else {
            dbg_time("%s write=%d, errno: %d (%s)", __func__, ret, errno, strerror(errno));
        }
    } else {
        dbg_time("%s poll=%d, errno: %d (%s)", __func__, ret, errno, strerror(errno));
    }

    return ret;
}

static int GobiNetGetClientID(const char *qcqmi, UCHAR QMIType) {
    int ClientId;
    ClientId = cm_open_dev(qcqmi);
    if (ClientId == -1) {
        dbg_time("failed to open %s, errno: %d (%s)", qcqmi, errno, strerror(errno));
        return -1;
    }

    if (ioctl(ClientId, IOCTL_QMI_GET_SERVICE_FILE, QMIType) != 0) {
        dbg_time("failed to get ClientID for 0x%02x errno: %d (%s)", QMIType, errno, strerror(errno));
        close(ClientId);
        ClientId = 0;
    }

    switch (QMIType) {
        case QMUX_TYPE_WDS: dbg_time("Get clientWDS = %d", ClientId); break;
        case QMUX_TYPE_DMS: dbg_time("Get clientDMS = %d", ClientId); break;
        case QMUX_TYPE_NAS: dbg_time("Get clientNAS = %d", ClientId); break;
        case QMUX_TYPE_QOS: dbg_time("Get clientQOS = %d", ClientId); break;
        case QMUX_TYPE_WMS: dbg_time("Get clientWMS = %d", ClientId); break;
        case QMUX_TYPE_PDS: dbg_time("Get clientPDS = %d", ClientId); break;
        case QMUX_TYPE_UIM: dbg_time("Get clientUIM = %d", ClientId); break;
        case QMUX_TYPE_COEX: dbg_time("Get clientCOEX = %d", ClientId); break;
        case QMUX_TYPE_WDS_ADMIN: dbg_time("Get clientWDA = %d", ClientId);
        break;
        default: break;
    }

    return ClientId;
}

static int GobiNetDeInit(void) {
    unsigned int i;
    for (i = 0; i < sizeof(qmiclientId)/sizeof(qmiclientId[0]); i++)
    {
        if (qmiclientId[i] != 0)
        {
            close(qmiclientId[i]);
            qmiclientId[i] = 0;
        }
    }

    return 0;
}

static void * GobiNetThread(void *pData) {
    PROFILE_T *profile = (PROFILE_T *)pData;
    const char *qcqmi = (const char *)profile->qmichannel;
    int wait_for_request_quit = 0;   
    
    qmiclientId[QMUX_TYPE_WDS] = GobiNetGetClientID(qcqmi, QMUX_TYPE_WDS);
    if (profile->enable_ipv6)
        qmiclientId[QMUX_TYPE_WDS_IPV6] = GobiNetGetClientID(qcqmi, QMUX_TYPE_WDS);
    qmiclientId[QMUX_TYPE_DMS] = GobiNetGetClientID(qcqmi, QMUX_TYPE_DMS);
    qmiclientId[QMUX_TYPE_NAS] = GobiNetGetClientID(qcqmi, QMUX_TYPE_NAS);
    qmiclientId[QMUX_TYPE_UIM] = GobiNetGetClientID(qcqmi, QMUX_TYPE_UIM);
#ifdef CONFIG_COEX_WWAN_STATE
    qmiclientId[QMUX_TYPE_COEX] = GobiNetGetClientID(qcqmi, QMUX_TYPE_COEX);
#endif
    if (profile->qmap_mode == 0 || profile->loopback_state) {//when QMAP enabled, set data format in GobiNet Driver
        qmiclientId[QMUX_TYPE_WDS_ADMIN] = GobiNetGetClientID(qcqmi, QMUX_TYPE_WDS_ADMIN);
        profile->wda_client = qmiclientId[QMUX_TYPE_WDS_ADMIN];
    }

    //donot check clientWDA, there is only one client for WDA, if quectel-CM is killed by SIGKILL, i cannot get client ID for WDA again!
    if (qmiclientId[QMUX_TYPE_WDS] == 0)  /*|| (clientWDA == -1)*/ {
        GobiNetDeInit();
        dbg_time("%s Failed to open %s, errno: %d (%s)", __func__, qcqmi, errno, strerror(errno));
        qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_DISCONNECTED);
        pthread_exit(NULL);
        return NULL;
    }

    qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_CONNECTED);

    while (1) {
        struct pollfd pollfds[16] = {{qmidevice_control_fd[1], POLLIN, 0}};
        int ne, ret, nevents = 1;
        unsigned int i;

        for (i = 0; i < sizeof(qmiclientId)/sizeof(qmiclientId[0]); i++)
        {
            if (qmiclientId[i] != 0)
            {
                pollfds[nevents].fd = qmiclientId[i];
                pollfds[nevents].events = POLLIN;
                pollfds[nevents].revents = 0;
                nevents++;
            }
        }

        do {
            ret = poll(pollfds, nevents, wait_for_request_quit ? 1000: -1);
         } while ((ret < 0) && (errno == EINTR));

	if (ret == 0 && wait_for_request_quit) {
    		QmiThreadRecvQMI(NULL); //main thread may pending on QmiThreadSendQMI()
		continue;
	}

        if (ret <= 0) {
            dbg_time("%s poll=%d, errno: %d (%s)", __func__, ret, errno, strerror(errno));
            break;
        }

        for (ne = 0; ne < nevents; ne++) {
            int fd = pollfds[ne].fd;
            short revents = pollfds[ne].revents;

            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                dbg_time("%s poll err/hup/inval", __func__);
                dbg_time("epoll fd = %d, events = 0x%04x", fd, revents);
                if (fd == qmidevice_control_fd[1]) {
                } else {
                }
                if (revents & (POLLERR | POLLHUP | POLLNVAL))
                    goto __GobiNetThread_quit;
            }

            if ((revents & POLLIN) == 0)
                continue;

            if (fd == qmidevice_control_fd[1]) {
                int triger_event;
                if (read(fd, &triger_event, sizeof(triger_event)) == sizeof(triger_event)) {
                    //DBG("triger_event = 0x%x", triger_event);
                    switch (triger_event) {
                        case RIL_REQUEST_QUIT:
                            goto __GobiNetThread_quit;
                        break;
                        case SIG_EVENT_STOP:
                            wait_for_request_quit = 1;   
                        break;
                        default:
                        break;
                    }
                }
                continue;
            }

            {
                ssize_t nreads;
                PQCQMIMSG pResponse = (PQCQMIMSG)cm_recv_buf;

                nreads = read(fd, &pResponse->MUXMsg, sizeof(cm_recv_buf) - sizeof(QCQMI_HDR));
                if (nreads <= 0)
                {
                    dbg_time("%s read=%d errno: %d (%s)",  __func__, (int)nreads, errno, strerror(errno));
                    break;
                }

                for (i = 0; i < sizeof(qmiclientId)/sizeof(qmiclientId[0]); i++)
                {
                    if (qmiclientId[i] == fd)
                    {
                        pResponse->QMIHdr.QMIType = i;
                    }
                }

                pResponse->QMIHdr.IFType = USB_CTL_MSG_TYPE_QMI;
                pResponse->QMIHdr.Length = cpu_to_le16(nreads + sizeof(QCQMI_HDR)  - 1);
                pResponse->QMIHdr.CtlFlags = 0x00;
                pResponse->QMIHdr.ClientId = (fd&0xFF) ? fd&0xFF : pResponse->QMIHdr.QMIType;;

                QmiThreadRecvQMI(pResponse);
            }
        }
    }

__GobiNetThread_quit:
    GobiNetDeInit();
    qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_DISCONNECTED);
    QmiThreadRecvQMI(NULL); //main thread may pending on QmiThreadSendQMI()
    dbg_time("%s exit", __func__);
    pthread_exit(NULL);
    return NULL;
}

const struct qmi_device_ops gobi_qmidev_ops = {
	.deinit = GobiNetDeInit,
	.send = GobiNetSendQMI,
	.read = GobiNetThread,
};
#endif

