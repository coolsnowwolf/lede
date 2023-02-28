//https://github.com/andersson/qrtr
/******************************************************************************
  @file    QrtrCM.c
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

typedef struct {
    uint32_t service;
    uint32_t version;
    uint32_t instance;
    uint32_t node;
    uint32_t port;
} QrtrService;

#define QRTR_MAX (QMUX_TYPE_WDS_ADMIN + 1)
static QrtrService service_list[QRTR_MAX];
static int qmiclientId[QRTR_MAX];
static int get_client(UCHAR QMIType);
static uint32_t node_modem = 3; //IPQ ~ 3, QCM ~ 0

#ifdef USE_LINUX_MSM_IPC
#include <linux/msm_ipc.h>

struct xport_ipc_router_server_addr {
        uint32_t service;
        uint32_t instance;
        uint32_t node_id;
        uint32_t port_id;
};

union ctl_msg {
        uint32_t cmd;
                struct {
                uint32_t cmd;
                        uint32_t service;
                        uint32_t instance;
                uint32_t node_id;
                uint32_t port_id;
        } srv;
                struct {
                uint32_t cmd;
                uint32_t node_id;
                uint32_t port_id;
        } cli;
        };
#define CTL_CMD_NEW_SERVER          4
#define CTL_CMD_REMOVE_SERVER       5

#define VERSION_MASK 0xff
#define GET_VERSION(x) (x & 0xff)
#define GET_XPORT_SVC_INSTANCE(x) GET_VERSION(x)
#define GET_INSTANCE(x) ((x & 0xff00) >> 8)

static int msm_ipc_socket(const char *name)
{
    int sock;
    int flags;

    sock = socket(AF_MSM_IPC, SOCK_DGRAM, 0);
    if (sock < 0) {
        dbg_time("%s(%s) errno: %d (%s)\n", __func__, name, errno, strerror(errno));
        return -1;
    }

    fcntl(sock, F_SETFD, FD_CLOEXEC);
    flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    return sock;
}

static uint32_t xport_lookup
(
 int lookup_sock_fd,
 uint32_t service_id,
 uint32_t version
)
{
  uint32_t num_servers_found = 0;
  uint32_t num_entries_to_fill = 4;
  struct server_lookup_args *lookup_arg;
  int i;

  lookup_arg = (struct server_lookup_args *)malloc(sizeof(*lookup_arg)
      + (num_entries_to_fill * sizeof(struct msm_ipc_server_info)));
  if (!lookup_arg)
  {
    dbg_time("%s: Malloc failed\n", __func__);
    return 0;
  }

  lookup_arg->port_name.service = service_id;
  lookup_arg->port_name.instance = GET_XPORT_SVC_INSTANCE(version);
  lookup_arg->num_entries_in_array = num_entries_to_fill;
  lookup_arg->lookup_mask =  VERSION_MASK;
  lookup_arg->num_entries_found = 0;
  if (ioctl(lookup_sock_fd, IPC_ROUTER_IOCTL_LOOKUP_SERVER, lookup_arg) < 0)
  {
    dbg_time("%s: Lookup failed for %08x: %08x\n", __func__, service_id, version);
    free(lookup_arg);
    return 0;
  }

  dbg_time("%s: num_entries_found %d for type=%d instance=%d", __func__,
    lookup_arg->num_entries_found, service_id, version);
  num_servers_found = 0;
  for (i = 0; ((i < (int)num_entries_to_fill) && (i < lookup_arg->num_entries_found)); i++)
  {
     QrtrService service_info[1];

    if (lookup_arg->srv_info[i].node_id != node_modem)
        continue;
    num_servers_found++;

    service_info[0].service = lookup_arg->srv_info[i].service;
    service_info[0].version = GET_VERSION(lookup_arg->srv_info[i].instance);
    service_info[0].instance = GET_INSTANCE(lookup_arg->srv_info[i].instance);
    service_info[0].node = lookup_arg->srv_info[i].node_id;
    service_info[0].port = lookup_arg->srv_info[i].port_id;

    service_list[service_id] = service_info[0];
    qmiclientId[service_id] = get_client(service_id);
  }

  free(lookup_arg);
  return num_servers_found;
}

static int xport_send(int sock, uint32_t node, uint32_t port, const void *data, unsigned int sz)
{
    struct sockaddr_msm_ipc addr = {};
    int rc;

    addr.family = AF_MSM_IPC;
    addr.address.addrtype = MSM_IPC_ADDR_ID;
    addr.address.addr.port_addr.node_id = node;
    addr.address.addr.port_addr.port_id = port;

    rc = sendto(sock, data, sz, MSG_DONTWAIT, (void *)&addr, sizeof(addr));
    if (rc < 0) {
        dbg_time("xport_send errno: %d (%s)\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}

static int xport_recv(int sock, void *data, unsigned int sz, uint32_t *node, uint32_t *port)
{
    struct sockaddr_msm_ipc addr = {};
    socklen_t addr_size = sizeof(struct sockaddr_msm_ipc);
    int rc;

    rc = recvfrom(sock, data, sz, MSG_DONTWAIT, (void *)&addr, &addr_size);
    if (rc < 0) {
        dbg_time("xport_recv errno: %d (%s)\n", errno, strerror(errno));
    }
    else if (addr.address.addrtype != MSM_IPC_ADDR_ID) {
        dbg_time("xport_recv addrtype is NOT MSM_IPC_ADDR_ID\n");
        rc = -1;
    }

    *node = addr.address.addr.port_addr.node_id;
    *port = addr.address.addr.port_addr.port_id;
    return rc;
}
#define qmi_recv xport_recv

static int xport_ctrl_init(void)
{
    int ctrl_sock;
    int rc;
    uint32_t instance = 1; //modem
    uint32_t version;

    ctrl_sock = msm_ipc_socket("ctrl_port");
    if (ctrl_sock == -1)
        return -1;

    rc = ioctl(ctrl_sock, IPC_ROUTER_IOCTL_GET_VERSION, &version);
    if (rc < 0) {
        dbg_time("%s: failed to get ipc version\n", __func__);
        goto init_close_ctrl_fd;
    }
    dbg_time("%s ipc_version = %d", __func__, version);

    rc = ioctl(ctrl_sock, IPC_ROUTER_IOCTL_BIND_CONTROL_PORT, NULL);
    if (rc < 0) {
        dbg_time("%s: failed to bind as control port\n", __func__);
        goto init_close_ctrl_fd;
    }

    //cat /sys/kernel/debug/msm_ipc_router/dump_servers
    rc = 0;
    rc += xport_lookup(ctrl_sock, QMUX_TYPE_WDS, instance);
    if (service_list[QMUX_TYPE_WDS].port) {
        qmiclientId[QMUX_TYPE_WDS_IPV6] = get_client(QMUX_TYPE_WDS);
    }
    rc += xport_lookup(ctrl_sock, QMUX_TYPE_NAS, instance);
    rc += xport_lookup(ctrl_sock, QMUX_TYPE_UIM, instance);
    rc += xport_lookup(ctrl_sock, QMUX_TYPE_DMS, instance);
    rc += xport_lookup(ctrl_sock, QMUX_TYPE_WDS_ADMIN, instance);

    if (rc == 0) {
        dbg_time("%s: failed to lookup qmi service\n", __func__);
        goto init_close_ctrl_fd;
    }

    return ctrl_sock;

init_close_ctrl_fd:
    close(ctrl_sock);
    return -1;
}

static void handle_ctrl_pkt(int sock) {
    union ctl_msg pkt;
    uint32_t type;
    int rc;

    rc = recvfrom(sock, &pkt, sizeof(pkt), 0, NULL, NULL);
    if (rc < 0)
        return;

    type = le32toh(pkt.cmd);
    if (CTL_CMD_NEW_SERVER == type || CTL_CMD_REMOVE_SERVER == type) {
        QrtrService s;

        s.service = le32toh(pkt.srv.service);
        s.version = le32toh(pkt.srv.instance) & 0xff;
        s.instance = le32toh(pkt.srv.instance) >> 8;
        s.node = le32toh(pkt.srv.node_id);
        s.port = le32toh(pkt.srv.port_id);

        if (debug_qmi)
            dbg_time ("[qrtr] %s  server on %u:%u -> service %u, version %u, instance %u",
                    CTL_CMD_NEW_SERVER == type ? "add" : "remove",
                     s.node, s.port, s.service, s.version, s.instance);

        if (CTL_CMD_NEW_SERVER == type) {
            if (s.service < QRTR_MAX) {
                service_list[s.service] = s;
            }
        }
        else if (CTL_CMD_REMOVE_SERVER == type) {
            if (s.service < QRTR_MAX) {
                memset(&service_list[s.service], 0, sizeof(QrtrService));
            }
        }
    }
}
#else
#include <linux/socket.h>
#include "qrtr.h"
#endif

static int qrtr_socket(void)
{
    struct sockaddr_qrtr sq;
    socklen_t sl = sizeof(sq);
    int sock;
    int rc;

    sock = socket(AF_QIPCRTR, SOCK_DGRAM, 0);
    if (sock < 0) {
        dbg_time("qrtr_socket errno: %d (%s)\n", errno, strerror(errno));
        return -1;
    }

    rc = getsockname(sock, (void *)&sq, &sl);
    if (rc || sq.sq_family != AF_QIPCRTR || sl != sizeof(sq)) {
        dbg_time("getsockname: %d (%s)\n", errno, strerror(errno));
        close(sock);
        return -1;
    }

    return sock;
}

static int qrtr_send(int sock, uint32_t node, uint32_t port, const void *data, unsigned int sz)
{
    struct sockaddr_qrtr sq = {};
    int rc;

    sq.sq_family = AF_QIPCRTR;
    sq.sq_node = node;
    sq.sq_port = port;

    rc = sendto(sock, data, sz, MSG_DONTWAIT, (void *)&sq, sizeof(sq));
    if (rc < 0) {
        dbg_time("sendto errno: %d (%s)\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}

static int qrtr_recv(int sock, void *data, unsigned int sz, uint32_t *node, uint32_t *port)
{
    struct sockaddr_qrtr sq = {};
    socklen_t sl = sizeof(sq);
    int rc;

    rc = recvfrom(sock, data, sz, MSG_DONTWAIT, (void *)&sq, &sl);
    if (rc < 0) {
        dbg_time("qrtr_recv errno: %d (%s)\n", errno, strerror(errno));
    }

    *node  = sq.sq_node;
    *port  = sq.sq_port;
    return rc;
    }
#define qmi_recv qrtr_recv

static int qrtr_ctrl_init(void) {
    int sock;
    int rc;
    struct qrtr_ctrl_pkt pkt;
    struct sockaddr_qrtr sq;
    socklen_t sl = sizeof(sq);

    sock = qrtr_socket();
    if (sock == -1)
        return -1;

    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd = htole32(QRTR_TYPE_NEW_LOOKUP);

    getsockname(sock, (void *)&sq, &sl);
    rc = qrtr_send(sock, sq.sq_node, QRTR_PORT_CTRL, &pkt, sizeof(pkt));
    if (rc == -1) {
        dbg_time("qrtr_send errno: %d (%s)\n", errno, strerror(errno));
        close(sock);
        return -1;
    }

    return sock;
}

static void handle_server_change(uint32_t type, struct qrtr_ctrl_pkt *ppkt) {
    struct qrtr_ctrl_pkt pkt = *ppkt;
        QrtrService s;

        s.service = le32toh(pkt.server.service);
        s.version = le32toh(pkt.server.instance) & 0xff;
        s.instance = le32toh(pkt.server.instance) >> 8;
        s.node = le32toh(pkt.server.node);
        s.port = le32toh(pkt.server.port);

        if (debug_qmi)
            dbg_time ("[qrtr] %s  server on %u:%u -> service %u, version %u, instance %u",
                    QRTR_TYPE_NEW_SERVER == type ? "add" : "remove",
                     s.node, s.port, s.service, s.version, s.instance);

        if (s.node != node_modem)
            return; //we only care modem

        if (QRTR_TYPE_NEW_SERVER == type) {
            if (s.service < QRTR_MAX) {
                service_list[s.service] = s;
            }
        }
        else if (QRTR_TYPE_DEL_SERVER == type) {
            if (s.service < QRTR_MAX) {
                memset(&service_list[s.service], 0, sizeof(QrtrService));
            }
        }
    }

static void handle_ctrl_pkt(int sock) {
    struct qrtr_ctrl_pkt pkt;
    struct sockaddr_qrtr sq;
    socklen_t sl = sizeof(sq);
    uint32_t type;
    int rc;

    rc = recvfrom(sock, &pkt, sizeof(pkt), 0, (void *)&sq, &sl);
    if (rc < 0)
        return;

    type = le32toh(pkt.cmd);
    if (debug_qmi)
        dbg_time("type %u, node %u, sq.port %x, len: %d", type, sq.sq_node, sq.sq_port, rc);

    if (sq.sq_port != QRTR_PORT_CTRL)
        return;

    if (QRTR_TYPE_NEW_SERVER == type || QRTR_TYPE_DEL_SERVER == type) {
        handle_server_change(type, &pkt);
    }
}

static int get_client(UCHAR QMIType) {
    int ClientId;
    QrtrService *s = &service_list[QMIType];

    if (!s ->service) {
        dbg_time("%s service: %d for QMIType: %d", __func__, s ->service, QMIType);
        return -ENODEV;
    }

#ifdef USE_LINUX_MSM_IPC
    ClientId = msm_ipc_socket("xport");
#else
    ClientId = qrtr_socket();
#endif
    if (ClientId == -1) {
        return 0;
    }

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

static void handle_alloc_client(PROFILE_T *profile) {
    int srv_list[] = {QMUX_TYPE_WDS, QMUX_TYPE_NAS, QMUX_TYPE_UIM, QMUX_TYPE_DMS, QMUX_TYPE_WDS_ADMIN};
    size_t i = 0, srv_ready = 0;
    static int report = -1;

    if (report != -1)
        return;

    for(i = 0; i < sizeof(srv_list)/sizeof(srv_list[0]); i++) {
        int srv = srv_list[i];

        if (service_list[srv].service)
            srv_ready++;
        else
            continue;

        if (qmiclientId[srv] == 0) {
            qmiclientId[srv] = get_client(srv);

            if (qmiclientId[srv] != 0) {
                if (srv == QMUX_TYPE_WDS) {
                    qmiclientId[QMUX_TYPE_WDS_IPV6] = get_client(QMUX_TYPE_WDS);
                }
                else if (srv == QMUX_TYPE_WDS_ADMIN) {
                     profile->wda_client = qmiclientId[QMUX_TYPE_WDS_ADMIN];
                }
            }
        }
    }

    if (srv_ready == sizeof(srv_list)/sizeof(srv_list[0])) {
        if (qmiclientId[QMUX_TYPE_WDS]) {
            qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_CONNECTED);
        } else {
            qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_DISCONNECTED);
        }
        report = 1;
    }
}

static int qmi_send(PQCQMIMSG pRequest) {
    uint8_t QMIType = pRequest->QMIHdr.QMIType;
    int sock;
    QrtrService *s = &service_list[QMIType == QMUX_TYPE_WDS_IPV6 ? QMUX_TYPE_WDS: QMIType];
    sock = qmiclientId[QMIType];

    pRequest->QMIHdr.ClientId = 0xaa;
    if (!s ->service || !sock) {
        dbg_time("%s service: %d, sock: %d for QMIType: %d", __func__, s ->service, sock, QMIType);
        return -ENODEV;
    }

#ifdef USE_LINUX_MSM_IPC
    return xport_send(sock, s->node, s->port, &pRequest->MUXMsg,
            le16_to_cpu(pRequest->QMIHdr.Length) + 1 - sizeof(QCQMI_HDR));
#else
    return qrtr_send(sock, s->node, s->port, &pRequest->MUXMsg,
            le16_to_cpu(pRequest->QMIHdr.Length) + 1 - sizeof(QCQMI_HDR));
#endif
}

static int qmi_deinit(void) {
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

static void * qmi_read(void *pData) {
    PROFILE_T *profile = (PROFILE_T *)pData;
    int ctrl_sock;
    int wait_for_request_quit = 0;   

#ifdef USE_LINUX_MSM_IPC
    ctrl_sock = xport_ctrl_init();
    if (ctrl_sock != -1)
        qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_CONNECTED);
#else
    ctrl_sock = qrtr_ctrl_init();
#endif

    if (ctrl_sock == -1)
        goto _quit;

    while (1) {
        struct pollfd pollfds[16] = {{qmidevice_control_fd[1], POLLIN, 0}, {ctrl_sock, POLLIN, 0}};
        int ne, ret, nevents = 2;
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
            ret = poll(pollfds, nevents, wait_for_request_quit ? 1000 : -1);
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
                    goto _quit;
            }

            if ((revents & POLLIN) == 0)
                continue;

            if (fd == qmidevice_control_fd[1]) {
                int triger_event;
                if (read(fd, &triger_event, sizeof(triger_event)) == sizeof(triger_event)) {
                    //DBG("triger_event = 0x%x", triger_event);
                    switch (triger_event) {
                        case RIL_REQUEST_QUIT:
                            goto _quit;
                        break;
                        case SIG_EVENT_STOP:
                            wait_for_request_quit = 1;   
                        break;
                        default:
                        break;
                    }
                }
            }
            else  if (fd == ctrl_sock) {
                handle_ctrl_pkt(ctrl_sock);
                handle_alloc_client(profile);
            }
            else
            {
                PQCQMIMSG pResponse = (PQCQMIMSG)cm_recv_buf;
                int rc;
                uint32_t sq_node = 0;
                uint32_t sq_port = 0;

                rc = qmi_recv(fd, &pResponse->MUXMsg, sizeof(cm_recv_buf) - sizeof(QCQMI_HDR), &sq_node, &sq_port);
                if (debug_qmi)
                    dbg_time("fd %d, node %u, port %x, len: %d", fd, sq_node, sq_port, rc);

                if (rc <= 0)
                {
                    dbg_time("%s read=%d errno: %d (%s)",  __func__, rc, errno, strerror(errno));
                    break;
                }

                for (i = 0; i < sizeof(qmiclientId)/sizeof(qmiclientId[0]); i++)
                {
                    if (qmiclientId[i] == fd)
                    {
                        pResponse->QMIHdr.QMIType = i;

                        if (service_list[i].node != sq_node || service_list[i].port != sq_port) {
                            continue;
                        }
                    }
                }

                pResponse->QMIHdr.IFType = USB_CTL_MSG_TYPE_QMI;
                pResponse->QMIHdr.Length = cpu_to_le16(rc + sizeof(QCQMI_HDR)  - 1);
                pResponse->QMIHdr.CtlFlags = 0x00;
                pResponse->QMIHdr.ClientId = 0xaa;

                QmiThreadRecvQMI(pResponse);
            }
        }
    }

_quit:
    qmi_deinit();
    close(ctrl_sock);
    qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_DISCONNECTED);
    QmiThreadRecvQMI(NULL); //main thread may pending on QmiThreadSendQMI()
    dbg_time("%s exit", __func__);
    pthread_exit(NULL);
    return NULL;
}

const struct qmi_device_ops qrtr_qmidev_ops = {
	.deinit = qmi_deinit,
	.send = qmi_send,
	.read = qmi_read,
};

