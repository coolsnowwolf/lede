/******************************************************************************
  @file    quectel-qmi-proxy.c
  @brief   The qmi proxy.

  DESCRIPTION
  Connectivity Management Tool for USB network adapter of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2023 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/un.h>
#include <linux/if.h>
#include <dirent.h>
#include <signal.h>
#include <inttypes.h>

#include "qendian.h"
#include "qlist.h"
#include "QCQMI.h"
#include "QCQCTL.h"
#include "QCQMUX.h"

#ifndef MIN
#define MIN(a, b)	((a) < (b)? (a): (b))
#endif

const char * get_time(void) {
    static char time_buf[128];
    struct timeval  tv;
    time_t time;
    suseconds_t millitm;
    struct tm *ti;

    gettimeofday (&tv, NULL);

    time= tv.tv_sec;
    millitm = (tv.tv_usec + 500) / 1000;

    if (millitm == 1000) {
        ++time;
        millitm = 0;
    }

    ti = localtime(&time);
    sprintf(time_buf, "[%02d-%02d_%02d:%02d:%02d:%03d]", ti->tm_mon+1, ti->tm_mday, ti->tm_hour, ti->tm_min, ti->tm_sec, (int)millitm);
    return time_buf;
}

#define dprintf(fmt, args...) do { fprintf(stdout, "%s " fmt, get_time(), ##args); } while(0);
#define SYSCHECK(c) do{if((c)<0) {dprintf("%s %d error: '%s' (code: %d)\n", __func__, __LINE__, strerror(errno), errno); return -1;}}while(0)
#define cfmakenoblock(fd) do{fcntl(fd, F_SETFL, fcntl(fd,F_GETFL) | O_NONBLOCK);}while(0)

typedef struct {
    struct qlistnode qnode;
    int ClientFd;
    QCQMIMSG qmi[0];
} QMI_PROXY_MSG;

typedef struct {
    struct qlistnode qnode;
    uint8_t QMIType;
    uint8_t ClientId;
    unsigned AccessTime;
} QMI_PROXY_CLINET;

typedef struct {
    struct qlistnode qnode;
    struct qlistnode client_qnode;
    int ClientFd;
    unsigned AccessTime;
} QMI_PROXY_CONNECTION;

#ifdef QUECTEL_QMI_MERGE
#define MERGE_PACKET_IDENTITY 0x2c7c
#define MERGE_PACKET_VERSION 0x0001
#define MERGE_PACKET_MAX_PAYLOAD_SIZE 56
typedef struct __QMI_MSG_HEADER {
    uint16_t idenity;
    uint16_t version;
    uint16_t cur_len;
    uint16_t total_len;
} QMI_MSG_HEADER;

typedef struct __QMI_MSG_PACKET {
    QMI_MSG_HEADER header;
    uint16_t len;
    char buf[4096];
} QMI_MSG_PACKET;
#endif

static int qmi_proxy_quit = 0;
static pthread_t thread_id = 0;
static int cdc_wdm_fd = -1;
static int qmi_proxy_server_fd = -1;
static struct qlistnode qmi_proxy_connection;
static struct qlistnode qmi_proxy_ctl_msg;
static int verbose_debug = 0;
static int modem_reset_flag = 0;
static int qmi_sync_done = 0;
static uint8_t qmi_buf[4096];

static int send_qmi_to_cdc_wdm(PQCQMIMSG pQMI);

#ifdef QUECTEL_QMI_MERGE
static int merge_qmi_rsp_packet(void *buf, ssize_t *src_size) {
    static QMI_MSG_PACKET s_QMIPacket;
    QMI_MSG_HEADER *header = NULL;
    ssize_t size = *src_size;

    if((uint16_t)size < sizeof(QMI_MSG_HEADER))
        return -1;

    header = (QMI_MSG_HEADER *)buf;
    if(le16toh(header->idenity) != MERGE_PACKET_IDENTITY || le16toh(header->version) != MERGE_PACKET_VERSION || le16toh(header->cur_len) > le16toh(header->total_len)) 
        return -1;

    if(le16toh(header->cur_len) == le16toh(header->total_len)) {
        *src_size = le16toh(header->total_len);
        memcpy(buf, buf + sizeof(QMI_MSG_HEADER), *src_size);
        s_QMIPacket.len = 0;  
        return 0;
    } 

    memcpy(s_QMIPacket.buf + s_QMIPacket.len, buf + sizeof(QMI_MSG_HEADER), le16toh(header->cur_len));
    s_QMIPacket.len += le16toh(header->cur_len);

    if (le16toh(header->cur_len) < MERGE_PACKET_MAX_PAYLOAD_SIZE || s_QMIPacket.len >= le16toh(header->total_len)) { 
       memcpy(buf, s_QMIPacket.buf, s_QMIPacket.len);      
       *src_size = s_QMIPacket.len;
       s_QMIPacket.len = 0;
       return 0;           
    }

    return -1;
}
#endif

static int create_local_server(const char *name) {
    int sockfd = -1;
    int reuse_addr = 1;
    struct sockaddr_un sockaddr;
    socklen_t alen;

    /*Create server socket*/
    SYSCHECK(sockfd = socket(AF_LOCAL, SOCK_STREAM, 0));

    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sun_family = AF_LOCAL;
    sockaddr.sun_path[0] = 0;
    memcpy(sockaddr.sun_path + 1, name, strlen(name) );

    alen = strlen(name) + offsetof(struct sockaddr_un, sun_path) + 1;
    SYSCHECK(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr)));
    if(bind(sockfd, (struct sockaddr *)&sockaddr, alen) < 0) {
        dprintf("bind %s errno: %d (%s)\n", name, errno, strerror(errno));
        close(sockfd);
        return -1;
    }

    dprintf("local server: %s sockfd = %d\n", name, sockfd);
    cfmakenoblock(sockfd);
    listen(sockfd, 1);    

    return sockfd;
}

static void accept_qmi_connection(int serverfd) {
    int clientfd = -1;
    unsigned char addr[128];
    socklen_t alen = sizeof(addr);
    QMI_PROXY_CONNECTION *qmi_con;

    clientfd = accept(serverfd, (struct sockaddr *)addr, &alen);

    qmi_con = (QMI_PROXY_CONNECTION *)malloc(sizeof(QMI_PROXY_CONNECTION));
    if (qmi_con) {
        qlist_init(&qmi_con->qnode);
        qlist_init(&qmi_con->client_qnode);
        qmi_con->ClientFd= clientfd;
        qmi_con->AccessTime = 0;
        dprintf("+++ ClientFd=%d\n", qmi_con->ClientFd);
        qlist_add_tail(&qmi_proxy_connection, &qmi_con->qnode);
    }

    cfmakenoblock(clientfd);
}

static void cleanup_qmi_connection(int clientfd, int clientDisconnect) {
    struct qlistnode *con_node, *qmi_node;
    
    qlist_for_each(con_node, &qmi_proxy_connection) {
        QMI_PROXY_CONNECTION *qmi_con = qnode_to_item(con_node, QMI_PROXY_CONNECTION, qnode);

        if (qmi_con->ClientFd == clientfd) {
            while (!qlist_empty(&qmi_con->client_qnode)) {
                QMI_PROXY_CLINET *qmi_client = qnode_to_item(qlist_head(&qmi_con->client_qnode), QMI_PROXY_CLINET, qnode);

                if (clientDisconnect) {
                    int size = 17;
                    QMI_PROXY_MSG *qmi_msg = malloc(sizeof(QMI_PROXY_MSG) + size);
                    PQCQMIMSG pQMI = &qmi_msg->qmi[0];

                    dprintf("xxx ClientFd=%d QMIType=%d ClientId=%d\n", qmi_con->ClientFd, qmi_client->QMIType, qmi_client->ClientId);
                    qlist_init(&qmi_msg->qnode);
                    qmi_msg->ClientFd = qmi_proxy_server_fd;
                    pQMI->QMIHdr.IFType   = USB_CTL_MSG_TYPE_QMI;
                    pQMI->QMIHdr.Length = htole16(16);
                    pQMI->QMIHdr.CtlFlags = 0x00;
                    pQMI->QMIHdr.QMIType  = QMUX_TYPE_CTL;
                    pQMI->QMIHdr.ClientId= 0x00;
                    pQMI->CTLMsg.ReleaseClientIdReq.CtlFlags = QMICTL_FLAG_REQUEST;
                    pQMI->CTLMsg.ReleaseClientIdReq.TransactionId = 255;    
                    pQMI->CTLMsg.ReleaseClientIdReq.QMICTLType = htole16(QMICTL_RELEASE_CLIENT_ID_REQ);
                    pQMI->CTLMsg.ReleaseClientIdReq.Length = htole16(5);
                    pQMI->CTLMsg.ReleaseClientIdReq.TLVType = QCTLV_TYPE_REQUIRED_PARAMETER;
                    pQMI->CTLMsg.ReleaseClientIdReq.TLVLength = htole16(2);
                    pQMI->CTLMsg.ReleaseClientIdReq.QMIType = qmi_client->QMIType;
                    pQMI->CTLMsg.ReleaseClientIdReq.ClientId = qmi_client->ClientId;

                    if (qlist_empty(&qmi_proxy_ctl_msg))
                        send_qmi_to_cdc_wdm(pQMI);
                    qlist_add_tail(&qmi_proxy_ctl_msg, &qmi_msg->qnode);
                }
                
                qlist_remove(&qmi_client->qnode);
                free(qmi_client);
            }
           
            qlist_for_each(qmi_node, &qmi_proxy_ctl_msg) {
                QMI_PROXY_MSG *qmi_msg = qnode_to_item(qmi_node, QMI_PROXY_MSG, qnode);

                if (qmi_msg->ClientFd == qmi_con->ClientFd) {
                    qlist_remove(&qmi_msg->qnode);
                    free(qmi_msg);
                    break;
                 }
            }

            dprintf("--- ClientFd=%d\n", qmi_con->ClientFd);    
            close(qmi_con->ClientFd);
            qlist_remove(&qmi_con->qnode);
            free(qmi_con);
            break;
        }
    }
}

static void get_client_id(QMI_PROXY_CONNECTION *qmi_con, PQMICTL_GET_CLIENT_ID_RESP_MSG pClient) {
    if (pClient->QMIResult == 0 && pClient->QMIError == 0) {
        QMI_PROXY_CLINET *qmi_client = (QMI_PROXY_CLINET *)malloc(sizeof(QMI_PROXY_CLINET));

        qlist_init(&qmi_client->qnode);
        qmi_client->QMIType = pClient->QMIType;
        qmi_client->ClientId = pClient->ClientId;
        qmi_client->AccessTime = 0;

        dprintf("+++ ClientFd=%d QMIType=%d ClientId=%d\n", qmi_con->ClientFd, qmi_client->QMIType, qmi_client->ClientId);
        qlist_add_tail(&qmi_con->client_qnode, &qmi_client->qnode);
    }
}

static void release_client_id(QMI_PROXY_CONNECTION *qmi_con, PQMICTL_RELEASE_CLIENT_ID_RESP_MSG pClient) {
    struct qlistnode *client_node;
    
    if (pClient->QMIResult == 0 && pClient->QMIError == 0) {
        qlist_for_each (client_node, &qmi_con->client_qnode) {
            QMI_PROXY_CLINET *qmi_client = qnode_to_item(client_node, QMI_PROXY_CLINET, qnode);
            
            if (pClient->QMIType == qmi_client->QMIType && pClient->ClientId == qmi_client->ClientId) {
                dprintf("--- ClientFd=%d QMIType=%d ClientId=%d\n", qmi_con->ClientFd, qmi_client->QMIType, qmi_client->ClientId);
                qlist_remove(&qmi_client->qnode);
                free(qmi_client);
                break;
            }
        }
    }
}

static void dump_qmi(PQCQMIMSG pQMI, int fd, const char flag)
{
    if (verbose_debug)
    {
        unsigned i;
        unsigned size = le16toh(pQMI->QMIHdr.Length) + 1;
        char buf[128];
        int cnt = 0;

        cnt += snprintf(buf + cnt, sizeof(buf) - cnt, "%c %d %u: ", flag, fd, size);
        for (i = 0; i < size && i < 24; i++)
            cnt += snprintf(buf + cnt, sizeof(buf) - cnt, "%02x ", ((uint8_t *)pQMI)[i]);
        dprintf("%s\n", buf)
    }
}

static int send_qmi_to_cdc_wdm(PQCQMIMSG pQMI) {
    struct pollfd pollfds[]= {{cdc_wdm_fd, POLLOUT, 0}};
    ssize_t ret = 0;

    do {
        ret = poll(pollfds, sizeof(pollfds)/sizeof(pollfds[0]), 5000);
    } while (ret == -1 && errno == EINTR && qmi_proxy_quit == 0);

    if (pollfds[0].revents & POLLOUT) {
        ssize_t size = le16toh(pQMI->QMIHdr.Length) + 1;
        ret = write(cdc_wdm_fd, pQMI, size);
        dump_qmi(pQMI, cdc_wdm_fd, 'w');
    }

    return ret;
}

static int send_qmi_to_client(PQCQMIMSG pQMI, int clientFd) {
    struct pollfd pollfds[]= {{clientFd, POLLOUT, 0}};
    ssize_t ret = 0;

    do {
        ret = poll(pollfds, sizeof(pollfds)/sizeof(pollfds[0]), 5000);
    } while (ret == -1 && errno == EINTR && qmi_proxy_quit == 0);

    if (pollfds[0].revents & POLLOUT) {
        ssize_t size = le16toh(pQMI->QMIHdr.Length) + 1;
        ret = write(clientFd, pQMI, size);
        dump_qmi(pQMI, clientFd, 'w');
    }

    return ret;
}

static void recv_qmi_from_dev(PQCQMIMSG pQMI) {
    struct qlistnode *con_node, *client_node;

    if (qmi_proxy_server_fd == -1) {
        qmi_sync_done = 1;
    }
    else if (pQMI->QMIHdr.QMIType == QMUX_TYPE_CTL) {
        if (pQMI->CTLMsg.QMICTLMsgHdr.CtlFlags == QMICTL_CTL_FLAG_RSP) {            
            if (!qlist_empty(&qmi_proxy_ctl_msg)) {
                QMI_PROXY_MSG *qmi_msg = qnode_to_item(qlist_head(&qmi_proxy_ctl_msg), QMI_PROXY_MSG, qnode);

                if (qmi_msg->qmi[0].CTLMsg.QMICTLMsgHdrRsp.TransactionId != pQMI->CTLMsg.QMICTLMsgHdrRsp.TransactionId
                    || qmi_msg->qmi[0].CTLMsg.QMICTLMsgHdrRsp.QMICTLType != pQMI->CTLMsg.QMICTLMsgHdrRsp.QMICTLType) {
                    dprintf("ERROR: ctl rsp tid:%d, type:%d - ctl req %d, %d\n",
                        pQMI->CTLMsg.QMICTLMsgHdrRsp.TransactionId, pQMI->CTLMsg.QMICTLMsgHdrRsp.QMICTLType,
                        qmi_msg->qmi[0].CTLMsg.QMICTLMsgHdrRsp.TransactionId, qmi_msg->qmi[0].CTLMsg.QMICTLMsgHdrRsp.QMICTLType);
                }
                else if (qmi_msg->ClientFd == qmi_proxy_server_fd) {
                    if (le16toh(pQMI->CTLMsg.QMICTLMsgHdrRsp.QMICTLType) == QMICTL_RELEASE_CLIENT_ID_RESP) {
                        dprintf("--- ClientFd=%d QMIType=%d ClientId=%d\n", qmi_proxy_server_fd,
                                pQMI->CTLMsg.ReleaseClientIdRsp.QMIType,  pQMI->CTLMsg.ReleaseClientIdRsp.ClientId);
                    }
                }
                else {
                    qlist_for_each(con_node, &qmi_proxy_connection) {
                        QMI_PROXY_CONNECTION *qmi_con = qnode_to_item(con_node, QMI_PROXY_CONNECTION, qnode);

                        if (qmi_con->ClientFd == qmi_msg->ClientFd) {
                            send_qmi_to_client(pQMI, qmi_msg->ClientFd);

                            if (le16toh(pQMI->CTLMsg.QMICTLMsgHdrRsp.QMICTLType) == QMICTL_GET_CLIENT_ID_RESP) {
                                get_client_id(qmi_con, &pQMI->CTLMsg.GetClientIdRsp);                                                        
                            }
                            else if (le16toh(pQMI->CTLMsg.QMICTLMsgHdrRsp.QMICTLType) == QMICTL_RELEASE_CLIENT_ID_RESP) {
                                release_client_id(qmi_con, &pQMI->CTLMsg.ReleaseClientIdRsp);
                            }
                            else {
                            }
                        }
                    }
                }

                qlist_remove(&qmi_msg->qnode);
                free(qmi_msg);

                if (!qlist_empty(&qmi_proxy_ctl_msg)) {
                    QMI_PROXY_MSG *qmi_msg = qnode_to_item(qlist_head(&qmi_proxy_ctl_msg), QMI_PROXY_MSG, qnode);

                    send_qmi_to_cdc_wdm(qmi_msg->qmi);
                }
            }
        } 
        else if (pQMI->QMIHdr.QMIType == QMICTL_CTL_FLAG_IND) {
            if (le16toh(pQMI->CTLMsg.QMICTLMsgHdrRsp.QMICTLType) == QMICTL_REVOKE_CLIENT_ID_IND) {
                modem_reset_flag = 1;
            }
        }
    }
    else  {
        qlist_for_each(con_node, &qmi_proxy_connection) {
            QMI_PROXY_CONNECTION *qmi_con = qnode_to_item(con_node, QMI_PROXY_CONNECTION, qnode);
            
            qlist_for_each(client_node, &qmi_con->client_qnode) {
                QMI_PROXY_CLINET *qmi_client = qnode_to_item(client_node, QMI_PROXY_CLINET, qnode);
                if (pQMI->QMIHdr.QMIType == qmi_client->QMIType) {
                    if (pQMI->QMIHdr.ClientId == 0 || pQMI->QMIHdr.ClientId == qmi_client->ClientId) {
                        send_qmi_to_client(pQMI, qmi_con->ClientFd);
                    }
                }
            }
        }
    }
}

static int recv_qmi_from_client(PQCQMIMSG pQMI, unsigned size, int clientfd) {
    if (qmi_proxy_server_fd == -1)
        return -1;

    if (pQMI->QMIHdr.QMIType == QMUX_TYPE_CTL) {  
        QMI_PROXY_MSG *qmi_msg;

        if (pQMI->CTLMsg.QMICTLMsgHdr.QMICTLType == QMICTL_SYNC_REQ) {
            dprintf("do not allow client send QMICTL_SYNC_REQ\n");
            return 0;
        }

        qmi_msg = malloc(sizeof(QMI_PROXY_MSG) + size);
        qlist_init(&qmi_msg->qnode);
        qmi_msg->ClientFd = clientfd;
        memcpy(qmi_msg->qmi, pQMI, size);

        if (qlist_empty(&qmi_proxy_ctl_msg))
            send_qmi_to_cdc_wdm(pQMI);
        qlist_add_tail(&qmi_proxy_ctl_msg, &qmi_msg->qnode);
    }
    else {
        send_qmi_to_cdc_wdm(pQMI);
    }

    return 0;
}

static int qmi_proxy_init(unsigned retry) {
    unsigned i;
    QCQMIMSG _QMI;
    PQCQMIMSG pQMI = &_QMI;

    dprintf("%s enter\n", __func__);

    pQMI->QMIHdr.IFType   = USB_CTL_MSG_TYPE_QMI;
    pQMI->QMIHdr.CtlFlags = 0x00;
    pQMI->QMIHdr.QMIType  = QMUX_TYPE_CTL;
    pQMI->QMIHdr.ClientId= 0x00;
    
    pQMI->CTLMsg.QMICTLMsgHdr.CtlFlags = QMICTL_FLAG_REQUEST;

    qmi_sync_done = 0;
    for (i = 0; i < retry; i++) {
        pQMI->CTLMsg.SyncReq.TransactionId = i+1;    
        pQMI->CTLMsg.SyncReq.QMICTLType = htole16(QMICTL_SYNC_REQ);
        pQMI->CTLMsg.SyncReq.Length = htole16(0);

        pQMI->QMIHdr.Length = 
            htole16(le16toh(pQMI->CTLMsg.QMICTLMsgHdr.Length) + sizeof(QCQMI_HDR) + sizeof(QCQMICTL_MSG_HDR) - 1);

        if (send_qmi_to_cdc_wdm(pQMI) <= 0)
            break;

        sleep(1);
        if (qmi_sync_done)
            break;
    }

    dprintf("%s %s\n", __func__, qmi_sync_done ? "succful" : "fail");
    return qmi_sync_done ? 0 : -1;
}

static void *qmi_proxy_loop(void *param)
{
    PQCQMIMSG pQMI = (PQCQMIMSG)qmi_buf;
    struct qlistnode *con_node;
    QMI_PROXY_CONNECTION *qmi_con;

    (void)param;
    dprintf("%s enter thread_id %p\n", __func__, (void *)pthread_self());

    qlist_init(&qmi_proxy_connection);
    qlist_init(&qmi_proxy_ctl_msg);

    while (cdc_wdm_fd > 0 && qmi_proxy_quit == 0) {
        struct pollfd pollfds[2+64];
        int ne, ret, nevents = 0;
        ssize_t nreads;

        pollfds[nevents].fd = cdc_wdm_fd;
        pollfds[nevents].events = POLLIN;
        pollfds[nevents].revents= 0;
        nevents++;
        
        if (qmi_proxy_server_fd > 0) {
            pollfds[nevents].fd = qmi_proxy_server_fd;
            pollfds[nevents].events = POLLIN;
            pollfds[nevents].revents= 0;
            nevents++;
        }

        qlist_for_each(con_node, &qmi_proxy_connection) {
            qmi_con = qnode_to_item(con_node, QMI_PROXY_CONNECTION, qnode);
            
            pollfds[nevents].fd = qmi_con->ClientFd;
            pollfds[nevents].events = POLLIN;
            pollfds[nevents].revents= 0;
            nevents++;

            if (nevents == (sizeof(pollfds)/sizeof(pollfds[0])))
                break;
        }

#if 0
        dprintf("poll ");
        for (ne = 0; ne < nevents; ne++) {
            dprintf("%d ", pollfds[ne].fd);
        }
        dprintf("\n");
#endif

        do {
            //ret = poll(pollfds, nevents, -1);
            ret = poll(pollfds, nevents, (qmi_proxy_server_fd > 0) ? -1 : 200);
        } while (ret == -1 && errno == EINTR && qmi_proxy_quit == 0);
         
        if (ret < 0) {
            dprintf("%s poll=%d, errno: %d (%s)\n", __func__, ret, errno, strerror(errno));
            goto qmi_proxy_loop_exit;
        }

        for (ne = 0; ne < nevents; ne++) {
            int fd = pollfds[ne].fd;
            short revents = pollfds[ne].revents;

            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                dprintf("%s poll fd = %d, revents = %04x\n", __func__, fd, revents);
                if (fd == cdc_wdm_fd) {
                    goto qmi_proxy_loop_exit;
                } else if(fd == qmi_proxy_server_fd) {
                
                } else {
                    cleanup_qmi_connection(fd, 1);
                }

                continue;
            }

            if (!(pollfds[ne].revents & POLLIN)) {
                continue;
            }

            if (fd == qmi_proxy_server_fd) {
                accept_qmi_connection(fd);
            }
            else if (fd == cdc_wdm_fd) {
                nreads = read(fd, pQMI, sizeof(qmi_buf));
                if (nreads <= 0) {
                    dprintf("%s read=%d errno: %d (%s)\n",  __func__, (int)nreads, errno, strerror(errno));
                    goto qmi_proxy_loop_exit;
                }
#ifdef QUECTEL_QMI_MERGE
                if(merge_qmi_rsp_packet(pQMI, &nreads))
                    continue;             
#endif
                if (nreads != (le16toh(pQMI->QMIHdr.Length) + 1)) {
                    dprintf("%s nreads=%d,  pQCQMI->QMIHdr.Length = %d\n",  __func__, (int)nreads, le16toh(pQMI->QMIHdr.Length));
                    continue;
                }

                dump_qmi(pQMI, fd, 'r');
                recv_qmi_from_dev(pQMI);
                if (modem_reset_flag)
                    goto qmi_proxy_loop_exit;
            }
            else {
                nreads = read(fd, pQMI, sizeof(qmi_buf));
  
                if (nreads <= 0) {
                    dprintf("%s read=%d errno: %d (%s)",  __func__, (int)nreads, errno, strerror(errno));
                    cleanup_qmi_connection(fd, 1);
                    break;
                }

                if (nreads != (le16toh(pQMI->QMIHdr.Length) + 1)) {
                    dprintf("%s nreads=%d,  pQCQMI->QMIHdr.Length = %d\n",  __func__, (int)nreads, le16toh(pQMI->QMIHdr.Length));
                    continue;
                }

                dump_qmi(pQMI, fd, 'r');
                recv_qmi_from_client(pQMI, nreads, fd);
            }
        }
    }

qmi_proxy_loop_exit:
    while (!qlist_empty(&qmi_proxy_connection)) {
        QMI_PROXY_CONNECTION *qmi_con = qnode_to_item(qlist_head(&qmi_proxy_connection), QMI_PROXY_CONNECTION, qnode);

        cleanup_qmi_connection(qmi_con->ClientFd, 0);
    }
    
    dprintf("%s exit, thread_id %p\n", __func__, (void *)pthread_self());

    return NULL;
}

static void usage(void) {
    dprintf(" -d <device_name>                      A valid qmi device\n"
            "                                       default /dev/cdc-wdm0, but cdc-wdm0 may be invalid\n"
            " -i <netcard_name>                     netcard name\n"
            " -v                                    Will show all details\n");
}

static void sig_action(int sig) {
    if (qmi_proxy_quit++ == 0) {
        if (thread_id)
            pthread_kill(thread_id, sig);
    }
}

int main(int argc, char *argv[]) {
    int opt;
    char cdc_wdm[32+1] = "/dev/cdc-wdm0";
    char servername[64] = {0};

    optind = 1;

    signal(SIGINT, sig_action);

    while ( -1 != (opt = getopt(argc, argv, "d:i:vh"))) {
        switch (opt) {
            case 'd':
                strcpy(cdc_wdm, optarg);
                break;
            case 'v':
                verbose_debug = 1;
                break;
            default:
                usage();
                return 0;
        }
    }

    sprintf(servername, "quectel-qmi-proxy%c", cdc_wdm[strlen(cdc_wdm)-1]);
    dprintf("Will use cdc-wdm='%s', proxy='%s'\n", cdc_wdm, servername);

    while (qmi_proxy_quit == 0) {
        cdc_wdm_fd = open(cdc_wdm, O_RDWR | O_NONBLOCK | O_NOCTTY);
        if (cdc_wdm_fd == -1) {
            dprintf("Failed to open %s, errno: %d (%s)\n", cdc_wdm, errno, strerror(errno));
            sleep(3);
            continue;
        }
        cfmakenoblock(cdc_wdm_fd);
        
        /* no qmi_proxy_loop lives, create one */
        pthread_create(&thread_id, NULL, qmi_proxy_loop, NULL);

        if (qmi_proxy_init(60) == 0) {
            qmi_proxy_server_fd = create_local_server(servername);
            dprintf("qmi_proxy_server_fd = %d\n", qmi_proxy_server_fd);
            if (qmi_proxy_server_fd == -1) {
                dprintf("Failed to create %s, errno: %d (%s)\n", servername, errno, strerror(errno));
                pthread_cancel(thread_id);
            }
        }
        else {
            pthread_cancel(thread_id);
        }

        pthread_join(thread_id, NULL);
        thread_id = 0;

        if (qmi_proxy_server_fd != -1) {
            dprintf("close server %s\n", servername);
            close(qmi_proxy_server_fd);
            qmi_proxy_server_fd = -1;
        }
        close(cdc_wdm_fd);
        cdc_wdm_fd = -1;

        if (qmi_proxy_quit == 0)
            sleep(modem_reset_flag ? 30 : 3);
        modem_reset_flag = 0;
    }

    return 0;
}
