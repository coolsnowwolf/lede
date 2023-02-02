/******************************************************************************
  @file    quectel-qrtr-proxy.c
  @brief   The qrtr proxy.

  DESCRIPTION
  Connectivity Management Tool for USB/PCIE network adapter of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
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
#include <linux/socket.h>
#include "qrtr.h"

#include "qendian.h"
#include "qlist.h"
#include "MPQMI.h"
#include "MPQCTL.h"
#include "MPQMUX.h"

static const char * get_time(void) {
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
#define align_4(_len) (((_len) + 3) & ~3)

typedef struct {
    struct qlistnode qnode;
    int ClientFd;
    QCQMIMSG qrtr[0];
} QRTR_PROXY_MSG;

typedef struct {
    struct qlistnode qnode;
    uint8_t QMIType;
    uint8_t ClientId;
    uint32_t node_id;
    uint32_t port_id;
    unsigned AccessTime;
} QRTR_PROXY_CLINET;

typedef struct {
    struct qlistnode qnode;
    struct qlistnode client_qnode;
    int ClientFd;
    unsigned AccessTime;
} QRTR_PROXY_CONNECTION;

typedef struct {
    struct qlistnode qnode;
    uint32_t service;
    uint32_t version;
    uint32_t instance;
    uint32_t node;
    uint32_t port;

    __le32 src_node_id;
    __le32 src_port_id;
} QRTR_SERVICE;

static int qrtr_proxy_quit = 0;
static pthread_t thread_id = 0;
static int cdc_wdm_fd = -1;
static int qrtr_proxy_server_fd = -1;
static struct qlistnode qrtr_proxy_connection;
static struct qlistnode qrtr_server_list;
static int verbose_debug = 0;
static uint32_t node_modem = 3; //IPQ ~ 3, QCM ~ 0
static uint32_t node_myself = 1;

static QRTR_SERVICE *find_qrtr_service(uint8_t QMIType)
{
    struct qlistnode *node;

    qlist_for_each (node, &qrtr_server_list) {
        QRTR_SERVICE *srv = qnode_to_item(node, QRTR_SERVICE, qnode);
        if (srv->service == QMIType)
            return srv;
    }
        
    return NULL;
}

static uint8_t client_bitmap[0xf0];
static uint8_t port_bitmap[0xff0];
static int alloc_client_id(void) {
    int id = 1;

    for (id = 1; id < (int)sizeof(client_bitmap); id++) {
        if (client_bitmap[id] == 0) {
            client_bitmap[id] = id;
            return id;
        }
    }

    dprintf("NOT find %s()\n", __func__);
    return 0;
}

static void free_client_id(int id) {
    if (id < (int)sizeof(client_bitmap) && client_bitmap[id] == id) {
        client_bitmap[id] = 0;
        return;
    }
    dprintf("NOT find %s(id=%d)\n", __func__, id);
}

static int alloc_port_id(void) {
    int id = 1;

    for (id = 1; id < (int)sizeof(port_bitmap); id++) {
        if (port_bitmap[id] == 0) {
            port_bitmap[id] = id;
            return id;
        }
    }

    dprintf("NOT find %s()\n", __func__);
   return 0;
}

static void free_port_id(int id) {
    if (id < (int)sizeof(port_bitmap) && port_bitmap[id] == id) {
        port_bitmap[id] = 0;
        return;
    }
    dprintf("NOT find %s(id=%d)\n", __func__, id);
}

static void dump_qrtr(void *buf, size_t len, char flag)
{
    size_t i;
    static char printf_buf[1024];
    int cnt = 0, limit=1024;
    unsigned char *d = (unsigned char *)buf;
    struct qrtr_hdr_v1 *hdr = (struct qrtr_hdr_v1 *)buf;
    const char *ctrl_pkt_strings[] = {
    	[QRTR_TYPE_DATA]	= "data",
    	[QRTR_TYPE_HELLO]	= "hello",
    	[QRTR_TYPE_BYE]		= "bye",
    	[QRTR_TYPE_NEW_SERVER]	= "new-server",
    	[QRTR_TYPE_DEL_SERVER]	= "del-server",
    	[QRTR_TYPE_DEL_CLIENT]	= "del-client",
    	[QRTR_TYPE_RESUME_TX]	= "resume-tx",
    	[QRTR_TYPE_EXIT]	= "exit",
    	[QRTR_TYPE_PING]	= "ping",
    	[QRTR_TYPE_NEW_LOOKUP]	= "new-lookup",
    	[QRTR_TYPE_DEL_LOOKUP]	= "del-lookup",
    };

    for (i = 0; i < len && i < 64; i++) {
        if (i%4 == 0)
            cnt += snprintf(printf_buf+cnt, limit-cnt, " ");
        cnt += snprintf(printf_buf+cnt, limit-cnt, "%02x", d[i]);
    }
    dprintf("%s\n", printf_buf);

    dprintf("%c ver=%d, type=%d(%s), %x,%x -> %x,%x, confirm_rx=%d, size=%u\n",
        flag,
        le32toh(hdr->version), le32toh(hdr->type), ctrl_pkt_strings[le32toh(hdr->type)],
        le32toh(hdr->src_node_id), le32toh(hdr->src_port_id), le32toh(hdr->dst_node_id), le32toh(hdr->dst_port_id),
        le32toh(hdr->confirm_rx), le32toh(hdr->size));
}

static int send_qmi_to_client(PQCQMIMSG pQMI, int fd) {
    struct pollfd pollfds[]= {{fd, POLLOUT, 0}};
    ssize_t ret = 0;
    ssize_t size = le16toh(pQMI->QMIHdr.Length) + 1;

    do {
        ret = poll(pollfds, sizeof(pollfds)/sizeof(pollfds[0]), 5000);
    } while (ret == -1 && errno == EINTR && qrtr_proxy_quit == 0);

    if (pollfds[0].revents & POLLOUT) {
        ret = write(fd, pQMI, size);
    }

    return ret == size ? 0 : -1;
}

static int send_qrtr_to_dev(struct qrtr_hdr_v1 *hdr, int fd) {
    struct pollfd pollfds[]= {{fd, POLLOUT, 0}};
    ssize_t ret = 0;
    ssize_t size = align_4(le32toh(hdr->size) + sizeof(*hdr));

    do {
        ret = poll(pollfds, sizeof(pollfds)/sizeof(pollfds[0]), 5000);
    } while (ret == -1 && errno == EINTR && qrtr_proxy_quit == 0);

    if (pollfds[0].revents & POLLOUT) {
        ret = write(fd, hdr, size);
    }

    return ret == size ? 0 : -1;
}

static int qrtr_node_enqueue(const void *data, size_t len,
			     int type, struct sockaddr_qrtr *from,
			     struct sockaddr_qrtr *to, unsigned int confirm_rx)
{
    int rc = -1;
    size_t size = sizeof(struct qrtr_hdr_v1) + len;
    struct qrtr_hdr_v1 *hdr = (struct qrtr_hdr_v1 *)malloc(align_4(size));

    if (hdr) {
        hdr->version = htole32(QRTR_PROTO_VER_1);
        hdr->type = htole32(type);
        hdr->src_node_id = htole32(from->sq_node);
        hdr->src_port_id = htole32(from->sq_port);
        hdr->dst_node_id = htole32(to->sq_node);
        hdr->dst_port_id = htole32(to->sq_port);
        hdr->size = htole32(len);
        hdr->confirm_rx = htole32(!!confirm_rx);

        memcpy(hdr + 1, data, len);
        dump_qrtr(hdr, size, '>');
        send_qrtr_to_dev(hdr, cdc_wdm_fd);
        free(hdr);
    }

    return rc;
}

static int send_ctrl_hello(__u32 sq_node, __u32 sq_port)
{
    struct qrtr_ctrl_pkt pkt;
    int rc;
    struct sockaddr_qrtr to = {AF_QIPCRTR, sq_node, sq_port};
    struct sockaddr_qrtr from = {AF_QIPCRTR, node_myself, QRTR_PORT_CTRL};

    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd =  htole32(QRTR_TYPE_HELLO);

    rc = qrtr_node_enqueue(&pkt, sizeof(pkt), QRTR_TYPE_HELLO, &from, &to, 0);
    if (rc < 0)
        return rc;

    return 0;
}

static int ctrl_cmd_del_client(__u32 sq_node, __u32 sq_port, uint8_t QMIType) 
{
    struct qrtr_ctrl_pkt pkt;
    int rc;
    struct sockaddr_qrtr to = {AF_QIPCRTR, QRTR_NODE_BCAST, QRTR_PORT_CTRL};
    struct sockaddr_qrtr from = {AF_QIPCRTR, sq_node, sq_port};
    QRTR_SERVICE *srv = find_qrtr_service(QMIType);

    if (srv) {
        to.sq_node = srv->src_node_id;
    }

    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd = htole32(QRTR_TYPE_DEL_CLIENT);
    pkt.client.node = htole32(sq_node);
    pkt.client.port = htole32(sq_port);

    rc = qrtr_node_enqueue(&pkt, sizeof(pkt), QRTR_TYPE_DATA, &from, &to, 0);
    if (rc < 0)
        return rc;

    return 0;
}

static void handle_server_change(struct qrtr_hdr_v1 *hdr) {
    struct qrtr_ctrl_pkt *pkt = (struct qrtr_ctrl_pkt *)(hdr + 1);
    QRTR_SERVICE *s;

    dprintf ("[qrtr] %s  server on %u:%u(%u:%u) -> service %u, instance %x\n",
            QRTR_TYPE_NEW_SERVER == hdr->type ? "add" : "remove",
             le32toh(pkt->server.node), le32toh(pkt->server.port),
             le32toh(hdr->src_node_id),  le32toh(hdr->src_port_id),
             le32toh(pkt->server.service), le32toh(pkt->server.instance));

    if (le32toh(pkt->server.node) != node_modem) {
        return; //we only care modem
    }

    s = (QRTR_SERVICE *)malloc(sizeof(QRTR_SERVICE));
    if (!s)
        return;

    qlist_init(&s->qnode);
    s->service = le32toh(pkt->server.service);
    s->version = le32toh(pkt->server.instance) & 0xff;
    s->instance = le32toh(pkt->server.instance) >> 8;
    s->node = le32toh(pkt->server.node);
    s->port = le32toh(pkt->server.port);

    s->src_node_id = le32toh(hdr->src_node_id);
    s->src_port_id = le32toh(hdr->src_port_id);

    if (QRTR_TYPE_NEW_SERVER == hdr->type) {
        qlist_add_tail(&qrtr_server_list, &s->qnode);
    }
    else if (QRTR_TYPE_DEL_SERVER == hdr->type) {
        qlist_remove(&s->qnode);
    }
}

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
        close(sockfd);
        dprintf("bind %s errno: %d (%s)\n", name, errno, strerror(errno));
        return -1;
    }

    dprintf("local server: %s sockfd = %d\n", name, sockfd);
    cfmakenoblock(sockfd);
    listen(sockfd, 1);    

    return sockfd;
}

static uint8_t alloc_qrtr_client_id(QRTR_PROXY_CONNECTION *qrtr_con, uint8_t QMIType) {
    QRTR_PROXY_CLINET *qrtr_client = (QRTR_PROXY_CLINET *)malloc(sizeof(QRTR_PROXY_CLINET));

    qlist_init(&qrtr_client->qnode);
    qrtr_client->QMIType = QMIType;
    qrtr_client->ClientId = alloc_client_id();
    qrtr_client->node_id = 1;
    qrtr_client->port_id = alloc_port_id();
    qrtr_client->AccessTime = 0;

    dprintf("+++ ClientFd=%d QMIType=%d ClientId=%d, node_id=%d, port_id=%d\n",
        qrtr_con->ClientFd, qrtr_client->QMIType, qrtr_client->ClientId,
        qrtr_client->node_id, qrtr_client->port_id);
    qlist_add_tail(&qrtr_con->client_qnode, &qrtr_client->qnode);

    return qrtr_client->ClientId;
}

static void release_qrtr_client_id(QRTR_PROXY_CONNECTION *qrtr_con, uint8_t QMIType, uint8_t ClientId) {
    struct qlistnode *client_node;
    int find = 0;

    qlist_for_each (client_node, &qrtr_con->client_qnode) {
        QRTR_PROXY_CLINET *qrtr_client = qnode_to_item(client_node, QRTR_PROXY_CLINET, qnode);
        
        if (QMIType == qrtr_client->QMIType && ClientId == qrtr_client->ClientId) {
            dprintf("--- ClientFd=%d QMIType=%d ClientId=%d, node_id=%d, port_id=%d\n",
                qrtr_con->ClientFd, qrtr_client->QMIType, qrtr_client->ClientId,
                qrtr_client->node_id, qrtr_client->port_id);
            ctrl_cmd_del_client(qrtr_client->node_id, qrtr_client->port_id, qrtr_client->QMIType);
            free_client_id(qrtr_client->ClientId);
            free_port_id(qrtr_client->port_id);
            qlist_remove(&qrtr_client->qnode);
            free(qrtr_client);
            find++;
            break;
        }
    }

    if (!find) {
        dprintf("NOT find on %s(ClientFd=%d, QMIType=%d, ClientId=%d)\n",
            __func__, qrtr_con->ClientFd, QMIType, ClientId);
    }
}

static void accept_qrtr_connection(int serverfd) {
    int clientfd = -1;
    unsigned char addr[128];
    socklen_t alen = sizeof(addr);
    QRTR_PROXY_CONNECTION *qrtr_con;

    clientfd = accept(serverfd, (struct sockaddr *)addr, &alen);

    qrtr_con = (QRTR_PROXY_CONNECTION *)malloc(sizeof(QRTR_PROXY_CONNECTION));
    if (qrtr_con) {
        qlist_init(&qrtr_con->qnode);
        qlist_init(&qrtr_con->client_qnode);
        qrtr_con->ClientFd= clientfd;
        qrtr_con->AccessTime = 0;
        dprintf("+++ ClientFd=%d\n", qrtr_con->ClientFd);
        qlist_add_tail(&qrtr_proxy_connection, &qrtr_con->qnode);
    }

    cfmakenoblock(clientfd);
}

static void cleanup_qrtr_connection(int clientfd) {
    struct qlistnode *con_node;
    int find = 0;
    
    qlist_for_each(con_node, &qrtr_proxy_connection) {
        QRTR_PROXY_CONNECTION *qrtr_con = qnode_to_item(con_node, QRTR_PROXY_CONNECTION, qnode);

        if (qrtr_con->ClientFd == clientfd) {
            while (!qlist_empty(&qrtr_con->client_qnode)) {
                QRTR_PROXY_CLINET *qrtr_client = qnode_to_item(qlist_head(&qrtr_con->client_qnode), QRTR_PROXY_CLINET, qnode);

                release_qrtr_client_id(qrtr_con, qrtr_client->QMIType, qrtr_client->ClientId);
            }
           
            dprintf("--- ClientFd=%d\n", qrtr_con->ClientFd);    
            close(qrtr_con->ClientFd);
            qlist_remove(&qrtr_con->qnode);
            free(qrtr_con);
            find = 1;
            break;
        }
    }

    if (!find) {
        dprintf("NOT find on %s(ClientFd=%d)\n", __func__, clientfd);
    }
}

static void recv_qrtr_from_dev(struct qrtr_hdr_v1 *hdr) {
    int find = 0;
    uint32_t type = le32toh(hdr->type);

    if (type == QRTR_TYPE_HELLO) {
        send_ctrl_hello(le32toh(hdr->src_node_id), le32toh(hdr->src_port_id));
        find++;
    }
    else if (type == QRTR_TYPE_NEW_SERVER || type == QRTR_TYPE_DEL_SERVER) {
        handle_server_change(hdr);
        find++;
    }
    else if (type == QRTR_TYPE_DATA) {
        struct qlistnode *con_node, *client_node;

        qlist_for_each(con_node, &qrtr_proxy_connection) {
            QRTR_PROXY_CONNECTION *qrtr_con = qnode_to_item(con_node, QRTR_PROXY_CONNECTION, qnode);
            
            qlist_for_each(client_node, &qrtr_con->client_qnode) {
                QRTR_PROXY_CLINET *qrtr_client = qnode_to_item(client_node, QRTR_PROXY_CLINET, qnode);

                if (qrtr_client->node_id == le32toh(hdr->dst_node_id) && qrtr_client->port_id == le32toh(hdr->dst_port_id)) {
                    PQCQMIMSG pQMI = (PQCQMIMSG)malloc(hdr->size + sizeof(QCQMI_HDR));

                    if (pQMI) {
                        pQMI->QMIHdr.IFType = USB_CTL_MSG_TYPE_QMI;
                        pQMI->QMIHdr.Length = htole16(hdr->size + sizeof(QCQMI_HDR) - 1);
                        pQMI->QMIHdr.CtlFlags = 0x00;
                        pQMI->QMIHdr.QMIType = qrtr_client->QMIType;
                        pQMI->QMIHdr.ClientId = qrtr_client->ClientId;
                        memcpy(&pQMI->MUXMsg, hdr + 1, hdr->size);
                        send_qmi_to_client(pQMI, qrtr_con->ClientFd);
                        free(pQMI);
                        find++;
                    }
                }
            }
        }

        if (hdr->confirm_rx) {
            struct qrtr_ctrl_pkt pkt;
            struct sockaddr_qrtr from = {AF_QIPCRTR, le32toh(hdr->dst_node_id), le32toh(hdr->dst_port_id)};
            struct sockaddr_qrtr to = {AF_QIPCRTR, le32toh(hdr->src_node_id), le32toh(hdr->src_port_id)};

            memset(&pkt, 0, sizeof(pkt));
            pkt.cmd = htole32(QRTR_TYPE_RESUME_TX);
            pkt.client.node = hdr->dst_node_id;
            pkt.client.port = hdr->dst_port_id;

            qrtr_node_enqueue(&pkt, sizeof(pkt), QRTR_TYPE_RESUME_TX, &from, &to, 0);
        }
    }
    else if (type == QRTR_TYPE_RESUME_TX) {
    }

    if (!find) {
        dprintf("NOT find on %s()\n", __func__);
    }    
}

static int recv_qmi_from_client(PQCQMIMSG pQMI, int clientfd) {
    QRTR_PROXY_CONNECTION *qrtr_con;
    struct qlistnode *con_node, *client_node;
    int find = 0;

    qlist_for_each(con_node, &qrtr_proxy_connection) {
        qrtr_con = qnode_to_item(con_node, QRTR_PROXY_CONNECTION, qnode);
        if (qrtr_con->ClientFd == clientfd)
            break;
        qrtr_con = NULL;
    }

    if (!qrtr_con) {
        return -1;
    }
            
    if (le16toh(pQMI->QMIHdr.QMIType) == QMUX_TYPE_CTL) {  
        if (pQMI->CTLMsg.QMICTLMsgHdr.QMICTLType == QMICTL_SYNC_REQ) {
            dprintf("do not allow client send QMICTL_SYNC_REQ\n");
            return 0;
        }
        else if (le16toh(pQMI->CTLMsg.QMICTLMsgHdr.QMICTLType) == QMICTL_GET_CLIENT_ID_REQ) {
            uint8_t QMIType = pQMI->CTLMsg.GetClientIdReq.QMIType;
            PQCQMIMSG pRsp = (PQCQMIMSG)malloc(256);

            if (pRsp) {
                uint8_t ClientId = 0;

                if (find_qrtr_service(QMIType)) {
                    ClientId = alloc_qrtr_client_id(qrtr_con, QMIType);
                }

                pRsp->QMIHdr.IFType = USB_CTL_MSG_TYPE_QMI;
                pRsp->QMIHdr.Length = htole16(sizeof(pRsp->CTLMsg.GetClientIdRsp) + sizeof(pRsp->QMIHdr) - 1);
                pRsp->QMIHdr.CtlFlags = 0x00;
                pRsp->QMIHdr.QMIType = QMUX_TYPE_CTL;
                pRsp->QMIHdr.ClientId = 0;

                pRsp->CTLMsg.QMICTLMsgHdrRsp.CtlFlags = QMICTL_FLAG_RESPONSE;
                pRsp->CTLMsg.QMICTLMsgHdrRsp.TransactionId = pQMI->CTLMsg.QMICTLMsgHdr.TransactionId;
                pRsp->CTLMsg.QMICTLMsgHdrRsp.QMICTLType = pQMI->CTLMsg.QMICTLMsgHdr.QMICTLType;
                pRsp->CTLMsg.QMICTLMsgHdrRsp.Length = htole16(sizeof(pRsp->CTLMsg.GetClientIdRsp) - sizeof(pRsp->CTLMsg.QMICTLMsgHdr));
                pRsp->CTLMsg.QMICTLMsgHdrRsp.TLVType = QCTLV_TYPE_RESULT_CODE;
                pRsp->CTLMsg.QMICTLMsgHdrRsp.TLVLength = htole16(4);
                pRsp->CTLMsg.QMICTLMsgHdrRsp.QMUXResult = htole16(ClientId ? 0 : QMI_RESULT_FAILURE);
                pRsp->CTLMsg.QMICTLMsgHdrRsp.QMUXError = htole16(ClientId ? 0 : QMI_ERR_INTERNAL);
                pRsp->CTLMsg.GetClientIdRsp.TLV2Type = QCTLV_TYPE_REQUIRED_PARAMETER;
                pRsp->CTLMsg.GetClientIdRsp.TLV2Length = htole16(2);
                pRsp->CTLMsg.GetClientIdRsp.QMIType = QMIType;
                pRsp->CTLMsg.GetClientIdRsp.ClientId = ClientId;

                send_qmi_to_client(pRsp, clientfd);
                free(pRsp);
                find++;
            }
        }
        else if (le16toh(pQMI->CTLMsg.QMICTLMsgHdr.QMICTLType) == QMICTL_RELEASE_CLIENT_ID_REQ) {
            PQCQMIMSG pRsp = (PQCQMIMSG)malloc(256);
            release_qrtr_client_id(qrtr_con, pQMI->CTLMsg.ReleaseClientIdReq.QMIType, pQMI->CTLMsg.ReleaseClientIdReq.ClientId);

            if (pRsp) {
                pRsp->QMIHdr.IFType = USB_CTL_MSG_TYPE_QMI;
                pRsp->QMIHdr.Length = htole16(sizeof(pRsp->CTLMsg.ReleaseClientIdRsp) + sizeof(pRsp->QMIHdr) - 1);
                pRsp->QMIHdr.CtlFlags = 0x00;
                pRsp->QMIHdr.QMIType = QMUX_TYPE_CTL;
                pRsp->QMIHdr.ClientId = 0;

                pRsp->CTLMsg.QMICTLMsgHdrRsp.CtlFlags = QMICTL_FLAG_RESPONSE;
                pRsp->CTLMsg.QMICTLMsgHdrRsp.TransactionId = pQMI->CTLMsg.QMICTLMsgHdr.TransactionId;
                pRsp->CTLMsg.QMICTLMsgHdrRsp.QMICTLType = pQMI->CTLMsg.QMICTLMsgHdr.QMICTLType;
                pRsp->CTLMsg.QMICTLMsgHdrRsp.Length = htole16(sizeof(pRsp->CTLMsg.ReleaseClientIdRsp) - sizeof(pRsp->CTLMsg.QMICTLMsgHdr));
                pRsp->CTLMsg.QMICTLMsgHdrRsp.TLVType = QCTLV_TYPE_RESULT_CODE;
                pRsp->CTLMsg.QMICTLMsgHdrRsp.TLVLength = htole16(4);
                pRsp->CTLMsg.QMICTLMsgHdrRsp.QMUXResult = htole16(0);
                pRsp->CTLMsg.QMICTLMsgHdrRsp.QMUXError = htole16(0);
                pRsp->CTLMsg.ReleaseClientIdRsp.TLV2Type = QCTLV_TYPE_REQUIRED_PARAMETER;
                pRsp->CTLMsg.ReleaseClientIdRsp.TLV2Length = htole16(2);
                pRsp->CTLMsg.ReleaseClientIdRsp.QMIType = pQMI->CTLMsg.ReleaseClientIdReq.QMIType;
                pRsp->CTLMsg.ReleaseClientIdRsp.ClientId = pQMI->CTLMsg.ReleaseClientIdReq.ClientId;

                send_qmi_to_client(pRsp, clientfd);
                free(pRsp);
                find++;
            }
        }
        else if (le16toh(pQMI->CTLMsg.QMICTLMsgHdr.QMICTLType) == QMICTL_GET_VERSION_REQ) {
            PQCQMIMSG pRsp = (PQCQMIMSG)malloc(256);

            if (pRsp) {
                pRsp->QMIHdr.IFType = USB_CTL_MSG_TYPE_QMI;
                pRsp->QMIHdr.Length = htole16(sizeof(pRsp->CTLMsg.GetVersionRsp) + sizeof(pRsp->QMIHdr) - 1);
                pRsp->QMIHdr.CtlFlags = 0x00;
                pRsp->QMIHdr.QMIType = QMUX_TYPE_CTL;
                pRsp->QMIHdr.ClientId = 0;

                pRsp->CTLMsg.QMICTLMsgHdrRsp.CtlFlags = QMICTL_FLAG_RESPONSE;
                pRsp->CTLMsg.QMICTLMsgHdrRsp.TransactionId = pQMI->CTLMsg.QMICTLMsgHdr.TransactionId;
                pRsp->CTLMsg.QMICTLMsgHdrRsp.QMICTLType = pQMI->CTLMsg.QMICTLMsgHdr.QMICTLType;
                pRsp->CTLMsg.QMICTLMsgHdrRsp.Length = htole16(sizeof(pRsp->CTLMsg.GetVersionRsp) - sizeof(pRsp->CTLMsg.QMICTLMsgHdr));
                pRsp->CTLMsg.QMICTLMsgHdrRsp.TLVType = QCTLV_TYPE_RESULT_CODE;
                pRsp->CTLMsg.QMICTLMsgHdrRsp.TLVLength = htole16(4);
                pRsp->CTLMsg.QMICTLMsgHdrRsp.QMUXResult = htole16(0);
                pRsp->CTLMsg.QMICTLMsgHdrRsp.QMUXError = htole16(0);
                pRsp->CTLMsg.GetVersionRsp.TLV2Type = QCTLV_TYPE_REQUIRED_PARAMETER;
                pRsp->CTLMsg.GetVersionRsp.TLV2Length = htole16(1);
                pRsp->CTLMsg.GetVersionRsp.NumElements = 0;

                send_qmi_to_client(pRsp, clientfd);
                free(pRsp);
                find++;
            }
        }
    }
    else {
        qlist_for_each (client_node, &qrtr_con->client_qnode) {
            QRTR_PROXY_CLINET *qrtr_client = qnode_to_item(client_node, QRTR_PROXY_CLINET, qnode);
            
            if (pQMI->QMIHdr.QMIType == qrtr_client->QMIType && pQMI->QMIHdr.ClientId == qrtr_client->ClientId) {
                QRTR_SERVICE *srv = find_qrtr_service(pQMI->QMIHdr.QMIType);

                if (srv && srv->service) {
                    struct sockaddr_qrtr from = {AF_QIPCRTR, qrtr_client->node_id, qrtr_client->port_id};
                    struct sockaddr_qrtr to = {AF_QIPCRTR, srv->node, srv->port};

                    qrtr_node_enqueue(&pQMI->MUXMsg, le16toh(pQMI->QMIHdr.Length) + 1 - sizeof(QCQMI_HDR),
                        QRTR_TYPE_DATA, &from, &to, 0);
                    find++;
                }
                break;
            }
        }
    }

    if (!find) {
        dprintf("NOT find on %s()\n", __func__);
    } 

    return 0;
}

static int qrtr_proxy_init(void) {
    unsigned i;
    int qrtr_sync_done = 0;

    dprintf("%s enter\n", __func__);
    send_ctrl_hello(QRTR_NODE_BCAST, QRTR_PORT_CTRL);

    for (i = 0; i < 10; i++) {
        sleep(1);
        qrtr_sync_done = !qlist_empty(&qrtr_server_list);
        if (qrtr_sync_done)
            break;
    }

    dprintf("%s %s\n", __func__, qrtr_sync_done ? "succful" : "fail");
    return qrtr_sync_done ? 0 : -1;
}

static void qrtr_start_server(const char* servername) {
    qrtr_proxy_server_fd = create_local_server(servername);
    dprintf("qrtr_proxy_server_fd = %d\n", qrtr_proxy_server_fd);
    if (qrtr_proxy_server_fd == -1) {
        dprintf("Failed to create %s, errno: %d (%s)\n", servername, errno, strerror(errno));
    }
}

static void qrtr_close_server(const char* servername) {
    if (qrtr_proxy_server_fd != -1) {
        dprintf("%s %s\n", __func__, servername);
        close(qrtr_proxy_server_fd);
        qrtr_proxy_server_fd = -1;
    }
}

static void *qrtr_proxy_loop(void *param)
{
    void *rx_buf;
    struct qlistnode *con_node;
    QRTR_PROXY_CONNECTION *qrtr_con;

    (void)param;
    dprintf("%s enter thread_id %p\n", __func__, (void *)pthread_self());
    
    rx_buf = malloc(8192);
    if (!rx_buf)
        return NULL;

    while (cdc_wdm_fd > 0 && qrtr_proxy_quit == 0) {
        struct pollfd pollfds[32];
        int ne, ret, nevents = 0;
        ssize_t nreads;

        pollfds[nevents].fd = cdc_wdm_fd;
        pollfds[nevents].events = POLLIN;
        pollfds[nevents].revents= 0;
        nevents++;
        
        if (qrtr_proxy_server_fd > 0) {
            pollfds[nevents].fd = qrtr_proxy_server_fd;
            pollfds[nevents].events = POLLIN;
            pollfds[nevents].revents= 0;
            nevents++;
        }

        qlist_for_each(con_node, &qrtr_proxy_connection) {
            qrtr_con = qnode_to_item(con_node, QRTR_PROXY_CONNECTION, qnode);
            
            pollfds[nevents].fd = qrtr_con->ClientFd;
            pollfds[nevents].events = POLLIN;
            pollfds[nevents].revents= 0;
            nevents++;

            if (nevents == (sizeof(pollfds)/sizeof(pollfds[0])))
                break;
        }

        do {
            //ret = poll(pollfds, nevents, -1);
            ret = poll(pollfds, nevents, (qrtr_proxy_server_fd > 0) ? -1 : 200);
         } while (ret == -1 && errno == EINTR && qrtr_proxy_quit == 0);
         
        if (ret < 0) {
            dprintf("%s poll=%d, errno: %d (%s)\n", __func__, ret, errno, strerror(errno));
            goto qrtr_proxy_loop_exit;
        }

        for (ne = 0; ne < nevents; ne++) {
            int fd = pollfds[ne].fd;
            short revents = pollfds[ne].revents;

            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                dprintf("%s poll fd = %d, revents = %04x\n", __func__, fd, revents);
                if (fd == cdc_wdm_fd) {
                    goto qrtr_proxy_loop_exit;
                }
                else if (fd == qrtr_proxy_server_fd) {
                
                }
                else {
                    cleanup_qrtr_connection(fd);
                }

                continue;
            }

            if (!(pollfds[ne].revents & POLLIN)) {
                continue;
            }

            if (fd == qrtr_proxy_server_fd) {
                accept_qrtr_connection(fd);
            }
            else if (fd == cdc_wdm_fd) {
                struct qrtr_hdr_v1 *hdr = (struct qrtr_hdr_v1 *)rx_buf;

                nreads = read(fd, rx_buf, 8192);
                if (nreads <= 0) {
                    dprintf("%s read=%d errno: %d (%s)\n",  __func__, (int)nreads, errno, strerror(errno));
                    goto qrtr_proxy_loop_exit;
                }
                else if (nreads != (int)align_4(le32toh(hdr->size) + sizeof(*hdr))) {
                    dprintf("%s nreads=%d,  hdr->size = %d\n",  __func__, (int)nreads, le32toh(hdr->size));
                    continue;
                }

                dump_qrtr(hdr, nreads, '<');
                recv_qrtr_from_dev(hdr);
            }
            else {
                PQCQMIMSG pQMI = (PQCQMIMSG)rx_buf;

                nreads = read(fd, rx_buf, 8192);
                if (nreads <= 0) {
                    dprintf("%s read=%d errno: %d (%s)",  __func__, (int)nreads, errno, strerror(errno));
                    cleanup_qrtr_connection(fd);
                    break;
                }
                else if (nreads != (le16toh(pQMI->QMIHdr.Length) + 1)) {
                    dprintf("%s nreads=%d,  pQCQMI->QMIHdr.Length = %d\n",  __func__, (int)nreads, le16toh(pQMI->QMIHdr.Length));
                    continue;
                }

                recv_qmi_from_client(pQMI, fd);
            }
        }
    }

qrtr_proxy_loop_exit:
    while (!qlist_empty(&qrtr_proxy_connection)) {
        QRTR_PROXY_CONNECTION *qrtr_con = qnode_to_item(qlist_head(&qrtr_proxy_connection), QRTR_PROXY_CONNECTION, qnode);

        cleanup_qrtr_connection(qrtr_con->ClientFd);
    }
    
    dprintf("%s exit, thread_id %p\n", __func__, (void *)pthread_self());
    free(rx_buf);

    return NULL;
}

static void usage(void) {
    dprintf(" -d <device_name>                      A valid qrtr device\n"
            "                                       default /dev/mhi_IPCR, but mhi_IPCR may be invalid\n"
            " -i <netcard_name>                     netcard name\n"
            " -v                                    Will show all details\n");
}

static void sig_action(int sig) {
    if (qrtr_proxy_quit == 0) {
        qrtr_proxy_quit = 1;
        if (thread_id)
            pthread_kill(thread_id, sig);
    }
}

int main(int argc, char *argv[]) {
    int opt;
    char cdc_wdm[32+1] = "/dev/mhi_IPCR";
    char servername[64] = {0};

    signal(SIGINT, sig_action);
    signal(SIGTERM, sig_action);

    optind = 1;
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

    sprintf(servername, "quectel-qrtr-proxy%c", cdc_wdm[strlen(cdc_wdm)-1]);
    dprintf("Will use cdc-wdm='%s', proxy='%s'\n", cdc_wdm, servername);

    while (qrtr_proxy_quit == 0) {
        cdc_wdm_fd = open(cdc_wdm, O_RDWR | O_NONBLOCK | O_NOCTTY);
        if (cdc_wdm_fd == -1) {
            dprintf("Failed to open %s, errno: %d (%s)\n", cdc_wdm, errno, strerror(errno));
            sleep(5);
            continue;
        }
        cfmakenoblock(cdc_wdm_fd);
        qlist_init(&qrtr_proxy_connection);
        qlist_init(&qrtr_server_list);
        pthread_create(&thread_id, NULL, qrtr_proxy_loop, NULL);

        if (qrtr_proxy_init() == 0) {
            qrtr_start_server(servername);
            pthread_join(thread_id, NULL);
            qrtr_close_server(servername);
        }
        else {
            pthread_cancel(thread_id);
            pthread_join(thread_id, NULL);
        }

        close(cdc_wdm_fd);
    }

    return 0;
}
