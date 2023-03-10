/******************************************************************************
  @file    QmiWwanCM.c
  @brief   QMI WWAN connectivity manager.

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

#ifdef CONFIG_QMIWWAN
static int cdc_wdm_fd = -1;
static UCHAR qmiclientId[QMUX_TYPE_ALL];

static UCHAR GetQCTLTransactionId(void) {
    static int TransactionId = 0;
    if (++TransactionId > 0xFF)
        TransactionId = 1;
    return TransactionId;
}

typedef USHORT (*CUSTOMQCTL)(PQMICTL_MSG pCTLMsg, void *arg);

static PQCQMIMSG ComposeQCTLMsg(USHORT QMICTLType, CUSTOMQCTL customQctlMsgFunction, void *arg) {
    UCHAR QMIBuf[WDM_DEFAULT_BUFSIZE];
    PQCQMIMSG pRequest = (PQCQMIMSG)QMIBuf;
    int Length;

    pRequest->QMIHdr.IFType   = USB_CTL_MSG_TYPE_QMI;
    pRequest->QMIHdr.CtlFlags = 0x00;
    pRequest->QMIHdr.QMIType  = QMUX_TYPE_CTL;
    pRequest->QMIHdr.ClientId= 0x00;

    pRequest->CTLMsg.QMICTLMsgHdr.CtlFlags = QMICTL_FLAG_REQUEST;
    pRequest->CTLMsg.QMICTLMsgHdr.TransactionId = GetQCTLTransactionId();
    pRequest->CTLMsg.QMICTLMsgHdr.QMICTLType = cpu_to_le16(QMICTLType);
    if (customQctlMsgFunction)
        pRequest->CTLMsg.QMICTLMsgHdr.Length = cpu_to_le16(customQctlMsgFunction(&pRequest->CTLMsg, arg) - sizeof(QCQMICTL_MSG_HDR));
    else
        pRequest->CTLMsg.QMICTLMsgHdr.Length = cpu_to_le16(0x0000);

    pRequest->QMIHdr.Length = cpu_to_le16(le16_to_cpu(pRequest->CTLMsg.QMICTLMsgHdr.Length) + sizeof(QCQMICTL_MSG_HDR) + sizeof(QCQMI_HDR) - 1);
    Length = le16_to_cpu(pRequest->QMIHdr.Length) + 1;

    pRequest = (PQCQMIMSG)malloc(Length);
    if (pRequest == NULL) {
        dbg_time("%s fail to malloc", __func__);
    } else {
        memcpy(pRequest, QMIBuf, Length);
    }

    return pRequest;
}

static USHORT CtlGetVersionReq(PQMICTL_MSG QCTLMsg, void *arg)
{
    (void)arg;
    QCTLMsg->GetVersionReq.TLVType = QCTLV_TYPE_REQUIRED_PARAMETER;
    QCTLMsg->GetVersionReq.TLVLength = cpu_to_le16(0x0001);
    QCTLMsg->GetVersionReq.QMUXTypes = QMUX_TYPE_ALL;
    return sizeof(QMICTL_GET_VERSION_REQ_MSG);
}

static USHORT CtlGetClientIdReq(PQMICTL_MSG QCTLMsg, void *arg) {
   QCTLMsg->GetClientIdReq.TLVType       = QCTLV_TYPE_REQUIRED_PARAMETER;
   QCTLMsg->GetClientIdReq.TLVLength     = cpu_to_le16(0x0001);
   QCTLMsg->GetClientIdReq.QMIType     = ((UCHAR *)arg)[0];
   return sizeof(QMICTL_GET_CLIENT_ID_REQ_MSG);
}

static USHORT CtlReleaseClientIdReq(PQMICTL_MSG QCTLMsg, void *arg) {
   QCTLMsg->ReleaseClientIdReq.TLVType       = QCTLV_TYPE_REQUIRED_PARAMETER;
   QCTLMsg->ReleaseClientIdReq.TLVLength     = cpu_to_le16(0x0002);
   QCTLMsg->ReleaseClientIdReq.QMIType     = ((UCHAR *)arg)[0];
   QCTLMsg->ReleaseClientIdReq.ClientId = ((UCHAR *)arg)[1] ;
   return sizeof(QMICTL_RELEASE_CLIENT_ID_REQ_MSG);
}

static USHORT CtlLibQmiProxyOpenReq(PQMICTL_MSG QCTLMsg, void *arg)
{
    (void)arg;
    const char *device_path = (const char *)(arg);
    QCTLMsg->LibQmiProxyOpenReq.TLVType = 0x01;
    QCTLMsg->LibQmiProxyOpenReq.TLVLength = cpu_to_le16(strlen(device_path));
    //strcpy(QCTLMsg->LibQmiProxyOpenReq.device_path, device_path);
    //__builtin___strcpy_chk
    memcpy(QCTLMsg->LibQmiProxyOpenReq.device_path, device_path, strlen(device_path));
    return sizeof(QMICTL_LIBQMI_PROXY_OPEN_MSG) + (strlen(device_path));
}

static int libqmi_proxy_open(const char *cdc_wdm) {
    int ret;
    PQCQMIMSG pResponse;

    ret = QmiThreadSendQMI(ComposeQCTLMsg(QMI_MESSAGE_CTL_INTERNAL_PROXY_OPEN,
        CtlLibQmiProxyOpenReq, (void *)cdc_wdm), &pResponse);
    if (!ret && pResponse
        && pResponse->CTLMsg.QMICTLMsgHdrRsp.QMUXResult == 0
        && pResponse->CTLMsg.QMICTLMsgHdrRsp.QMUXError == 0) {
        ret = 0;
    }
    else {
        return -1;
    }

    if (pResponse)
            free(pResponse);

    return ret;
}

static int QmiWwanSendQMI(PQCQMIMSG pRequest) {
    struct pollfd pollfds[]= {{cdc_wdm_fd, POLLOUT, 0}};
    int ret;

    if (cdc_wdm_fd == -1) {
        dbg_time("%s cdc_wdm_fd = -1", __func__);
        return -ENODEV;
    }

    if (pRequest->QMIHdr.QMIType != QMUX_TYPE_CTL) {
        pRequest->QMIHdr.ClientId = qmiclientId[pRequest->QMIHdr.QMIType];
        if (pRequest->QMIHdr.ClientId == 0) {
            dbg_time("QMIType %d has no clientID", pRequest->QMIHdr.QMIType);
            return -ENODEV;
        }

        if (pRequest->QMIHdr.QMIType == QMUX_TYPE_WDS_IPV6)
            pRequest->QMIHdr.QMIType = QMUX_TYPE_WDS;
    }

    do {
        ret = poll(pollfds, sizeof(pollfds)/sizeof(pollfds[0]), 5000);
    } while ((ret < 0) && (errno == EINTR));

    if (pollfds[0].revents & POLLOUT) {
        ssize_t nwrites = le16_to_cpu(pRequest->QMIHdr.Length) + 1;
        ret = write(cdc_wdm_fd, pRequest, nwrites);
        if (ret == nwrites) {
            ret = 0;
        } else {
            dbg_time("%s write=%d, errno: %d (%s)", __func__, ret, errno, strerror(errno));
        }
    } else {
        dbg_time("%s poll=%d, revents = 0x%x, errno: %d (%s)", __func__, ret, pollfds[0].revents, errno, strerror(errno));
    }

    return ret;
}

static UCHAR QmiWwanGetClientID(UCHAR QMIType) {
    PQCQMIMSG pResponse;

    QmiThreadSendQMI(ComposeQCTLMsg(QMICTL_GET_CLIENT_ID_REQ, CtlGetClientIdReq, &QMIType), &pResponse);

    if (pResponse) {
        USHORT QMUXResult = cpu_to_le16(pResponse->CTLMsg.QMICTLMsgHdrRsp.QMUXResult);       // QMI_RESULT_SUCCESS
        USHORT QMUXError = cpu_to_le16(pResponse->CTLMsg.QMICTLMsgHdrRsp.QMUXError);        // QMI_ERR_INVALID_ARG
        //UCHAR QMIType = pResponse->CTLMsg.GetClientIdRsp.QMIType;
        UCHAR ClientId = pResponse->CTLMsg.GetClientIdRsp.ClientId;

        if (!QMUXResult && !QMUXError && (QMIType == pResponse->CTLMsg.GetClientIdRsp.QMIType)) {
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
    }
    return 0;
}

static int QmiWwanReleaseClientID(QMI_SERVICE_TYPE QMIType, UCHAR ClientId) {
    UCHAR argv[] = {QMIType, ClientId};
    QmiThreadSendQMI(ComposeQCTLMsg(QMICTL_RELEASE_CLIENT_ID_REQ, CtlReleaseClientIdReq, argv), NULL);
    return 0;
}

static int QmiWwanInit(PROFILE_T *profile) {
    unsigned i;
    int ret;
    PQCQMIMSG pResponse;

    if (profile->proxy[0] && !strcmp(profile->proxy, LIBQMI_PROXY)) {
        ret = libqmi_proxy_open(profile->qmichannel);
        if (ret)
            return ret;
    }

    if (!profile->proxy[0]) {
        for (i = 0; i < 10; i++) {
            ret = QmiThreadSendQMITimeout(ComposeQCTLMsg(QMICTL_SYNC_REQ, NULL, NULL), NULL, 1 * 1000, __func__);
            if (!ret)
                break;
            sleep(1);
        }
        if (ret)
            return ret;
    }

    QmiThreadSendQMI(ComposeQCTLMsg(QMICTL_GET_VERSION_REQ, CtlGetVersionReq, NULL), &pResponse);
    if (profile->qmap_mode) {
        if (pResponse) {
            if (pResponse->CTLMsg.QMICTLMsgHdrRsp.QMUXResult == 0 && pResponse->CTLMsg.QMICTLMsgHdrRsp.QMUXError == 0) {
                uint8_t  NumElements = 0;

                for (NumElements = 0; NumElements < pResponse->CTLMsg.GetVersionRsp.NumElements; NumElements++) {
#if 0
                    dbg_time("QMUXType = %02x Version = %d.%d",
                        pResponse->CTLMsg.GetVersionRsp.TypeVersion[NumElements].QMUXType,
                        pResponse->CTLMsg.GetVersionRsp.TypeVersion[NumElements].MajorVersion,
                        pResponse->CTLMsg.GetVersionRsp.TypeVersion[NumElements].MinorVersion);
#endif
                    if (pResponse->CTLMsg.GetVersionRsp.TypeVersion[NumElements].QMUXType == QMUX_TYPE_WDS_ADMIN)
                        profile->qmap_version = (pResponse->CTLMsg.GetVersionRsp.TypeVersion[NumElements].MinorVersion > 16);
                }
            }
        }
    }
    if (pResponse) free(pResponse);
    qmiclientId[QMUX_TYPE_WDS] = QmiWwanGetClientID(QMUX_TYPE_WDS);
    if (profile->enable_ipv6)
        qmiclientId[QMUX_TYPE_WDS_IPV6] = QmiWwanGetClientID(QMUX_TYPE_WDS);
    qmiclientId[QMUX_TYPE_DMS] = QmiWwanGetClientID(QMUX_TYPE_DMS);
    qmiclientId[QMUX_TYPE_NAS] = QmiWwanGetClientID(QMUX_TYPE_NAS);
    qmiclientId[QMUX_TYPE_UIM] = QmiWwanGetClientID(QMUX_TYPE_UIM);
    qmiclientId[QMUX_TYPE_WDS_ADMIN] = QmiWwanGetClientID(QMUX_TYPE_WDS_ADMIN);
#ifdef CONFIG_COEX_WWAN_STATE
    qmiclientId[QMUX_TYPE_COEX] = QmiWwanGetClientID(QMUX_TYPE_COEX);
#endif
#ifdef CONFIG_ENABLE_QOS
    qmiclientId[QMUX_TYPE_QOS] = QmiWwanGetClientID(QMUX_TYPE_QOS);
#endif
    profile->wda_client = qmiclientId[QMUX_TYPE_WDS_ADMIN];

    return 0;
}

static int QmiWwanDeInit(void) {
    unsigned int i;
    for (i = 0; i < sizeof(qmiclientId)/sizeof(qmiclientId[0]); i++)
    {
        if (qmiclientId[i] != 0)
        {
                QmiWwanReleaseClientID(i, qmiclientId[i]);
                qmiclientId[i] = 0;
        }
    }

    return 0;
}

static ssize_t qmi_proxy_read (int fd, void *buf, size_t size) {
    ssize_t nreads;
    PQCQMI_HDR pHdr = (PQCQMI_HDR)buf;

    nreads = read(fd, pHdr, sizeof(QCQMI_HDR));
    if (nreads == sizeof(QCQMI_HDR) && le16_to_cpu(pHdr->Length) < size) {
        nreads += read(fd, pHdr+1, le16_to_cpu(pHdr->Length) + 1 - sizeof(QCQMI_HDR));
    }

    return nreads;
}

#ifdef QUECTEL_QMI_MERGE
static int QmiWwanMergeQmiRsp(void *buf, ssize_t *src_size) {
    static QMI_MSG_PACKET s_QMIPacket;
    QMI_MSG_HEADER *header = NULL;
    ssize_t size = *src_size;

    if((uint16_t)size < sizeof(QMI_MSG_HEADER))
        return -1;

    header = (QMI_MSG_HEADER *)buf;
    if(le16_to_cpu(header->idenity) != MERGE_PACKET_IDENTITY || le16_to_cpu(header->version) != MERGE_PACKET_VERSION || le16_to_cpu(header->cur_len) > le16_to_cpu(header->total_len)) 
        return -1;

    if(le16_to_cpu(header->cur_len) == le16_to_cpu(header->total_len)) {
        *src_size = le16_to_cpu(header->total_len);
        memcpy(buf, buf + sizeof(QMI_MSG_HEADER), *src_size);
        s_QMIPacket.len = 0;  
        return 0;
    } 

    memcpy(s_QMIPacket.buf + s_QMIPacket.len, buf + sizeof(QMI_MSG_HEADER), le16_to_cpu(header->cur_len));
    s_QMIPacket.len += le16_to_cpu(header->cur_len);

    if (le16_to_cpu(header->cur_len) < MERGE_PACKET_MAX_PAYLOAD_SIZE || s_QMIPacket.len >= le16_to_cpu(header->total_len)) { 
       memcpy(buf, s_QMIPacket.buf, s_QMIPacket.len);      
       *src_size = s_QMIPacket.len;
       s_QMIPacket.len = 0;
       return 0;           
    }

    return -1;
}
#endif

static void * QmiWwanThread(void *pData) {
    PROFILE_T *profile = (PROFILE_T *)pData;
    const char *cdc_wdm = (const char *)profile->qmichannel;
    int wait_for_request_quit = 0;
    char num = cdc_wdm[strlen(cdc_wdm)-1];
	
    if (profile->proxy[0]) {
         if (!strncmp(profile->proxy, QUECTEL_QMI_PROXY, strlen(QUECTEL_QMI_PROXY))) {
            snprintf(profile->proxy, sizeof(profile->proxy), "%s%c", QUECTEL_QMI_PROXY, num);
         }
    }
    else if (!strncmp(cdc_wdm, "/dev/mhi_IPCR", strlen("/dev/mhi_IPCR"))) {
        snprintf(profile->proxy, sizeof(profile->proxy), "%s%c", QUECTEL_QRTR_PROXY, num);
    }
    else if (profile->qmap_mode > 1) {
        snprintf(profile->proxy, sizeof(profile->proxy), "%s%c", QUECTEL_QMI_PROXY, num);
    }
    
    if (profile->proxy[0])
        cdc_wdm_fd = cm_open_proxy(profile->proxy);
    else
        cdc_wdm_fd = cm_open_dev(cdc_wdm);

    if (cdc_wdm_fd == -1) {
        dbg_time("%s Failed to open %s, errno: %d (%s)", __func__, cdc_wdm, errno, strerror(errno));
        qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_DISCONNECTED);
        pthread_exit(NULL);
        return NULL;
    }

    dbg_time("cdc_wdm_fd = %d", cdc_wdm_fd);

    qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_CONNECTED);
    while (1) {
        struct pollfd pollfds[] = {{qmidevice_control_fd[1], POLLIN, 0}, {cdc_wdm_fd, POLLIN, 0}};
        int ne, ret, nevents = sizeof(pollfds)/sizeof(pollfds[0]);

        do {
            ret = poll(pollfds, nevents, wait_for_request_quit ? 1000 : -1);
         } while ((ret < 0) && (errno == EINTR));

	if (ret == 0 && wait_for_request_quit) {
            QmiThreadRecvQMI(NULL);
            continue;
	}

        if (ret <= 0) {
            dbg_time("%s poll=%d, errno: %d (%s)", __func__, ret, errno, strerror(errno));
            break;
        }

        for (ne = 0; ne < nevents; ne++) {
            int fd = pollfds[ne].fd;
            short revents = pollfds[ne].revents;

            //dbg_time("{%d, %x, %x}", pollfds[ne].fd, pollfds[ne].events, pollfds[ne].revents);

            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                dbg_time("%s poll err/hup/inval", __func__);
                dbg_time("poll fd = %d, events = 0x%04x", fd, revents);
                if (fd == cdc_wdm_fd) {
                } else {
                }
                if (revents & (POLLHUP | POLLNVAL)) //EC20 bug, Can get POLLERR
                    goto __QmiWwanThread_quit;
            }

            if ((revents & POLLIN) == 0)
                continue;

            if (fd == qmidevice_control_fd[1]) {
                int triger_event;
                if (read(fd, &triger_event, sizeof(triger_event)) == sizeof(triger_event)) {
                    //DBG("triger_event = 0x%x", triger_event);
                    switch (triger_event) {
                        case RIL_REQUEST_QUIT:
                            goto __QmiWwanThread_quit;
                        break;
                        case SIG_EVENT_STOP:
                            wait_for_request_quit = 1;
                        break;
                        default:
                        break;
                    }
                }
            }

            if (fd == cdc_wdm_fd) {
                ssize_t nreads;
                PQCQMIMSG pResponse = (PQCQMIMSG)cm_recv_buf;
                
                if (!profile->proxy[0])
                    nreads = read(fd, cm_recv_buf, sizeof(cm_recv_buf));
                else
                    nreads = qmi_proxy_read(fd, cm_recv_buf, sizeof(cm_recv_buf));
                //dbg_time("%s read=%d errno: %d (%s)",  __func__, (int)nreads, errno, strerror(errno));
                if (nreads <= 0) {
                    dbg_time("%s read=%d errno: %d (%s)",  __func__, (int)nreads, errno, strerror(errno));
                    break;
                }
#ifdef QUECTEL_QMI_MERGE
                if((profile->qmap_mode == 0 || profile->qmap_mode == 1) && QmiWwanMergeQmiRsp(cm_recv_buf, &nreads))
                    continue;             
#endif
                if (nreads != (le16_to_cpu(pResponse->QMIHdr.Length) + 1)) {
                    dbg_time("%s nreads=%d,  pQCQMI->QMIHdr.Length = %d",  __func__, (int)nreads, le16_to_cpu(pResponse->QMIHdr.Length));
                    continue;
                }

                QmiThreadRecvQMI(pResponse);
            }
        }
    }

__QmiWwanThread_quit:
    if (cdc_wdm_fd != -1) { close(cdc_wdm_fd); cdc_wdm_fd = -1; }
    qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_DISCONNECTED);
    QmiThreadRecvQMI(NULL); //main thread may pending on QmiThreadSendQMI()
    dbg_time("%s exit", __func__);
    pthread_exit(NULL);
    return NULL;
}

const struct qmi_device_ops qmiwwan_qmidev_ops = {
    .init = QmiWwanInit,
    .deinit = QmiWwanDeInit,
    .send = QmiWwanSendQMI,
    .read = QmiWwanThread,
};

uint8_t qmi_over_mbim_get_client_id(uint8_t QMIType) {
    return QmiWwanGetClientID(QMIType);
}

uint8_t qmi_over_mbim_release_client_id(uint8_t QMIType, uint8_t ClientId) {
    return QmiWwanReleaseClientID(QMIType, ClientId);
}
#endif

