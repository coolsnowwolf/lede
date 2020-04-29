#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <ctype.h>
#include "QMIThread.h"

#ifdef CONFIG_QMIWWAN
int cdc_wdm_fd = -1;
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

static USHORT CtlGetVersionReq(PQMICTL_MSG QCTLMsg, void *arg) {
   QCTLMsg->GetVersionReq.TLVType       = QCTLV_TYPE_REQUIRED_PARAMETER;
   QCTLMsg->GetVersionReq.TLVLength     = cpu_to_le16(0x0001);
   QCTLMsg->GetVersionReq.QMUXTypes     = QMUX_TYPE_ALL;
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

int QmiWwanSendQMI(PQCQMIMSG pRequest) {
    struct pollfd pollfds[]= {{cdc_wdm_fd, POLLOUT, 0}};
    int ret;

    if (cdc_wdm_fd == -1) {
        dbg_time("%s cdc_wdm_fd = -1", __func__);
        return -ENODEV;
    }

    if (pRequest->QMIHdr.QMIType == QMUX_TYPE_WDS_IPV6)
        pRequest->QMIHdr.QMIType = QMUX_TYPE_WDS;

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

static int QmiWwanGetClientID(UCHAR QMIType) {
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

int QmiWwanInit(PROFILE_T *profile) {
    unsigned i;
    int ret;
    PQCQMIMSG pResponse;

    for (i = 0; i < 10; i++) {
        ret = QmiThreadSendQMITimeout(ComposeQCTLMsg(QMICTL_SYNC_REQ, NULL, NULL), NULL, 1 * 1000);
        if (!ret)
            break;
        sleep(1);
    }
    if (ret)
        return ret;

    QmiThreadSendQMI(ComposeQCTLMsg(QMICTL_GET_VERSION_REQ, CtlGetVersionReq, NULL), &pResponse);
    if (pResponse) free(pResponse);
    qmiclientId[QMUX_TYPE_WDS] = QmiWwanGetClientID(QMUX_TYPE_WDS);
    if (profile->IsDualIPSupported)
        qmiclientId[QMUX_TYPE_WDS_IPV6] = QmiWwanGetClientID(QMUX_TYPE_WDS);
    qmiclientId[QMUX_TYPE_DMS] = QmiWwanGetClientID(QMUX_TYPE_DMS);
    qmiclientId[QMUX_TYPE_NAS] = QmiWwanGetClientID(QMUX_TYPE_NAS);
    qmiclientId[QMUX_TYPE_UIM] = QmiWwanGetClientID(QMUX_TYPE_UIM);
    qmiclientId[QMUX_TYPE_WDS_ADMIN] = QmiWwanGetClientID(QMUX_TYPE_WDS_ADMIN);
    return 0;
}

int QmiWwanDeInit(void) {
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

void * QmiWwanThread(void *pData) {
    const char *cdc_wdm = (const char *)pData;
    cdc_wdm_fd = open(cdc_wdm, O_RDWR | O_NONBLOCK | O_NOCTTY);
    if (cdc_wdm_fd == -1) {
        dbg_time("%s Failed to open %s, errno: %d (%s)", __func__, cdc_wdm, errno, strerror(errno));
        qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_DISCONNECTED);
        pthread_exit(NULL);
        return NULL;
    }
    fcntl(cdc_wdm_fd, F_SETFD, FD_CLOEXEC) ;

    dbg_time("cdc_wdm_fd = %d", cdc_wdm_fd);

    qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_CONNECTED);

    while (1) {
        struct pollfd pollfds[] = {{qmidevice_control_fd[1], POLLIN, 0}, {cdc_wdm_fd, POLLIN, 0}};
        int ne, ret, nevents = sizeof(pollfds)/sizeof(pollfds[0]);

        do {
            ret = poll(pollfds, nevents, -1);
         } while ((ret < 0) && (errno == EINTR));

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
                        case SIGTERM:
                        case SIGHUP:
                        case SIGINT:
                            QmiThreadRecvQMI(NULL);
                        break;
                        default:
                        break;
                    }
                }
            }

            if (fd == cdc_wdm_fd) {
                ssize_t nreads;
                UCHAR QMIBuf[512];
                PQCQMIMSG pResponse = (PQCQMIMSG)QMIBuf;

                nreads = read(fd, QMIBuf, sizeof(QMIBuf));
                //dbg_time("%s read=%d errno: %d (%s)",  __func__, (int)nreads, errno, strerror(errno));
                if (nreads <= 0) {
                    dbg_time("%s read=%d errno: %d (%s)",  __func__, (int)nreads, errno, strerror(errno));
                    break;
                }

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

#else
int QmiWwanSendQMI(PQCQMIMSG pRequest) {return -1;}
int QmiWwanInit(PROFILE_T *profile) {return -1;}
int QmiWwanDeInit(void) {return -1;}
void * QmiWwanThread(void *pData) {dbg_time("please set CONFIG_QMIWWAN"); return NULL;}
#endif
