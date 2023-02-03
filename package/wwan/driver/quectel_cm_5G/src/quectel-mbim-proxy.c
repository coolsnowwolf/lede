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
#include <linux/in.h>
#include <linux/if.h>
#include <dirent.h>
#include <signal.h>
#include <inttypes.h>
#include <getopt.h>
#include "qendian.h"

#define QUECTEL_MBIM_PROXY "quectel-mbim-proxy"
#define safe_close(_fd) do { if (_fd > 0) { close(_fd); _fd = -1; } } while(0)

#define CM_MAX_CLIENT 32
#define TID_MASK (0xFFFFFF)
#define TID_SHIFT (24)

typedef enum {
    MBIM_OPEN_MSG = 1,
    MBIM_CLOSE_MSG = 2,
    MBIM_OPEN_DONE = 0x80000001,
    MBIM_CLOSE_DONE = 0x80000002,
} MBIM_MSG;

typedef struct {
    unsigned int  MessageType;
    unsigned int  MessageLength;
    unsigned int  TransactionId;
} MBIM_MESSAGE_HEADER;

typedef struct {
    MBIM_MESSAGE_HEADER MessageHeader;
    unsigned int MaxControlTransfer;
} MBIM_OPEN_MSG_T;

typedef struct {
    MBIM_MESSAGE_HEADER MessageHeader;
    unsigned int Status;
} MBIM_OPEN_DONE_T;

typedef struct {
    int client_fd;
    int client_idx;
} CM_CLIENT_T;

static unsigned char cm_recv_buffer[4096];
static CM_CLIENT_T cm_clients[CM_MAX_CLIENT];
static int verbose = 0;

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

#define mbim_debug(fmt, args...) do { fprintf(stdout, "%s " fmt, get_time(), ##args); } while(0);

static int non_block_write(int fd, void *data, int len)
{
    int ret;
    struct pollfd pollfd = {fd, POLLOUT, 0};
    ret = poll(&pollfd, 1, 3000);

    if (ret <= 0) {
        mbim_debug("%s poll ret=%d, errno: %d(%s)\n", __func__, ret, errno, strerror(errno));
    }

    ret = write (fd, data, len);
    if (ret != len)
        mbim_debug("%s write ret=%d, errno: %d(%s)\n", __func__, ret, errno, strerror(errno));

    return len;
}

static int mbim_send_open_msg(int mbim_dev_fd, uint32_t MaxControlTransfer) {
    MBIM_OPEN_MSG_T open_msg;
    MBIM_OPEN_MSG_T *pRequest = &open_msg;

    pRequest->MessageHeader.MessageType = htole32(MBIM_OPEN_MSG);
    pRequest->MessageHeader.MessageLength = htole32(sizeof(MBIM_OPEN_MSG_T));
    pRequest->MessageHeader.TransactionId = htole32(1);
    pRequest->MaxControlTransfer = htole32(MaxControlTransfer);

    mbim_debug("%s()\n", __func__);
    return non_block_write(mbim_dev_fd, pRequest, sizeof(MBIM_OPEN_MSG_T));
}

/*
 * parameter: proxy name
 * return: local proxy server fd or -1
*/
static int proxy_make_server(const char *proxy_name)
{
    int len, flag;
    struct sockaddr_un sockaddr;
    int mbim_server_fd;

    mbim_server_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (mbim_server_fd < 0) {
        mbim_debug("socket failed: %s\n", strerror(errno));
        return -1;
    }
    if (fcntl(mbim_server_fd, F_SETFL, fcntl(mbim_server_fd, F_GETFL) | O_NONBLOCK) < 0)
        mbim_debug("fcntl set server(%d) NONBLOCK attribute failed: %s\n", mbim_server_fd, strerror(errno));

    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sun_family = AF_LOCAL;
    sockaddr.sun_path[0] = 0;
    snprintf(sockaddr.sun_path, UNIX_PATH_MAX, "0%s", proxy_name);
    sockaddr.sun_path[0] = '\0';  // string starts with leading '\0'
    flag = 1;
    if (setsockopt(mbim_server_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        safe_close(mbim_server_fd);
        mbim_debug("setsockopt failed\n");
    }

    len = strlen(proxy_name) + offsetof(struct sockaddr_un, sun_path) + 1;
    if (bind(mbim_server_fd, (struct sockaddr*)&sockaddr, len) < 0) {
        safe_close(mbim_server_fd);
        mbim_debug("bind failed: %s\n", strerror(errno));
        return -1;
    }

    listen(mbim_server_fd, 4);
    return mbim_server_fd;
}

static int handle_client_connect(int server_fd)
{
    int i, client_fd;
    struct sockaddr_in cli_addr;
    socklen_t len = sizeof(cli_addr);

    client_fd = accept(server_fd, (struct sockaddr *)&cli_addr, &len);
    if (client_fd < 0) {
        mbim_debug("proxy accept failed: %s\n", strerror(errno));
        return -1;
    }

    if (fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFL) | O_NONBLOCK) < 0)
        mbim_debug("fcntl set client(%d) NONBLOCK attribute failed: %s\n", client_fd, strerror(errno));

    for (i = 0; i < CM_MAX_CLIENT; i++) {
        if (cm_clients[i].client_fd <= 0) {
            cm_clients[i].client_fd = client_fd;
            cm_clients[i].client_idx= i+1;
            mbim_debug("%s client_fd=%d, client_idx=%d\n", __func__, cm_clients[i].client_fd, cm_clients[i].client_idx);
            return 0;
        }
    }

    close(client_fd);
    return -1;
}

static void handle_client_disconnect(int client_fd)
{
    int i;

    for (i = 0; i < CM_MAX_CLIENT; i++) {
        if (cm_clients[i].client_fd == client_fd) {
            mbim_debug("%s client_fd=%d, client_idx=%d\n", __func__, cm_clients[i].client_fd, cm_clients[i].client_idx);
            safe_close(cm_clients[i].client_fd);
            return;
        }
    }
}

static int handle_client_request(int mbim_dev_fd, int client_fd, void *pdata, int len)
{
    int i;
    int client_idx = -1;
    int ret;
    MBIM_MESSAGE_HEADER *pRequest = (MBIM_MESSAGE_HEADER *)pdata;
    unsigned int  TransactionId = le32toh(pRequest->TransactionId);

    for (i = 0; i < CM_MAX_CLIENT; i++) {
        if (cm_clients[i].client_fd == client_fd) {
            client_idx = cm_clients[i].client_idx;
            break;
        }
    }

    if (client_idx == -1) {
        goto error;
    }

    if (le32toh(pRequest->MessageType) == MBIM_OPEN_MSG
        || le32toh(pRequest->MessageType) == MBIM_CLOSE_MSG) {
        MBIM_OPEN_DONE_T OpenDone;
        OpenDone.MessageHeader.MessageType = htole32(le32toh(pRequest->MessageType) | 0x80000000);
        OpenDone.MessageHeader.MessageLength = htole32(sizeof(OpenDone));
        OpenDone.MessageHeader.TransactionId = htole32(TransactionId);
        OpenDone.Status = htole32(0);
        non_block_write (client_fd, &OpenDone, sizeof(OpenDone));
        return 0;
    }

    /* transfer TransicationID to proxy transicationID and record in sender list */
    pRequest->TransactionId = htole32(TransactionId | (client_idx << TID_SHIFT));
    if (verbose) mbim_debug("REQ client_fd=%d, client_idx=%d, tid=%u\n",
        cm_clients[client_idx].client_fd, cm_clients[client_idx].client_idx, TransactionId);
    ret = non_block_write (mbim_dev_fd, pRequest, len);
    if (ret == len)
        return 0;

error:
    return -1;
}

/*
 * Will read message from device and transfer it to clients/client
 * Notice:
 *  unsocial message will be send to all clients
 */
static int handle_device_response(void *pdata, int len)
{
    int i;
    MBIM_MESSAGE_HEADER *pResponse = (MBIM_MESSAGE_HEADER *)pdata;
    unsigned int  TransactionId = le32toh(pResponse->TransactionId);

    /* unsocial/function error message */
    if (TransactionId == 0) {
        for (i = 0; i < CM_MAX_CLIENT; i++) {
            if (cm_clients[i].client_fd > 0) {
                non_block_write(cm_clients[i].client_fd, pResponse, len);
            }
        }
    }
    else {
        /* try to find the sender */
        int client_idx = (TransactionId >> TID_SHIFT);

        for (i = 0; i < CM_MAX_CLIENT; i++) {
            if (cm_clients[i].client_idx == client_idx && cm_clients[i].client_fd > 0) {
                TransactionId &= TID_MASK;
                pResponse->TransactionId = htole32(TransactionId);
                if (verbose) mbim_debug("RSP client_fd=%d, client_idx=%d, tid=%u\n",
                    cm_clients[i].client_fd, cm_clients[i].client_idx, TransactionId);
                non_block_write(cm_clients[i].client_fd, pResponse, len);
                break;
            }
        }

        if (i == CM_MAX_CLIENT) {
           mbim_debug("%s nobody care tid=%u\n", __func__, TransactionId);     
        }
    }

    return 0;
}

static int proxy_loop(int mbim_dev_fd)
{
    int i;
    int mbim_server_fd = -1;

    while (mbim_dev_fd > 0) {
        struct pollfd pollfds[2+CM_MAX_CLIENT];
        int ne, ret, nevents = 0;

        pollfds[nevents].fd = mbim_dev_fd;
        pollfds[nevents].events = POLLIN;
        pollfds[nevents].revents= 0;
        nevents++;
        
        if (mbim_server_fd > 0) {
            pollfds[nevents].fd = mbim_server_fd;
            pollfds[nevents].events = POLLIN;
            pollfds[nevents].revents= 0;
            nevents++;

            for (i = 0; i < CM_MAX_CLIENT; i++) {
                if (cm_clients[i].client_fd > 0) {
                    pollfds[nevents].fd = cm_clients[i].client_fd;
                    pollfds[nevents].events = POLLIN;
                    pollfds[nevents].revents= 0;
                    nevents++;
                }
            }
        }

        ret = poll(pollfds, nevents, (mbim_server_fd > 0) ? -1 : (10*1000));
        if (ret <= 0) {
            goto error;
        }

        for (ne = 0; ne < nevents; ne++) {
            int fd = pollfds[ne].fd;
            short revents = pollfds[ne].revents;

            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                mbim_debug("%s poll fd = %d, revents = %04x\n", __func__, fd, revents);
                if (fd == mbim_dev_fd) {
                    goto error;
                } else if(fd == mbim_server_fd) {
                
                } else {
                    handle_client_disconnect(fd);
                }
                continue;
            }

            if (!(pollfds[ne].revents & POLLIN)) {
                continue;
            }

            if (fd == mbim_server_fd) {
                handle_client_connect(fd);
            }
            else {
                    int len = read(fd, cm_recv_buffer, sizeof(cm_recv_buffer));

                    if (len <= 0) {
                        mbim_debug("%s read fd=%d, len=%d, errno: %d(%s)\n", __func__, fd, len, errno, strerror(errno));
                        if (fd == mbim_dev_fd)
                            goto error;
                        else
                            handle_client_disconnect(fd);

                        return len;
                    }

                    if (fd == mbim_dev_fd) {
                        if (mbim_server_fd == -1) {
                            MBIM_OPEN_DONE_T *pOpenDone = (MBIM_OPEN_DONE_T *)cm_recv_buffer;

                            if (le32toh(pOpenDone->MessageHeader.MessageType) == MBIM_OPEN_DONE) {
                                mbim_debug("receive MBIM_OPEN_DONE, status=%d\n", htole32(pOpenDone->Status));
                                if (htole32(pOpenDone->Status))
                                    goto error;
                                mbim_server_fd = proxy_make_server(QUECTEL_MBIM_PROXY);
                                mbim_debug("mbim_server_fd=%d\n", mbim_server_fd);
                            }
                        }
                        else {
                            handle_device_response(cm_recv_buffer, len);
                        }
                    }
                    else {
                        handle_client_request(mbim_dev_fd, fd, cm_recv_buffer, len);
                    }
            }
        }
    }

error:
    safe_close(mbim_server_fd);
    for (i = 0; i < CM_MAX_CLIENT; i++) {
        safe_close(cm_clients[i].client_fd);
    }

    mbim_debug("%s exit\n", __func__);
    return 0;
}

/*
 * How to use this proxy?
 * 1. modprobe -a 8021q
 * 2. Create network interface for channels: 
 *      ip link add link wwan0 name wwan0.1 type vlan id 1
 *      ip link add link wwan0 name wwan0.2 type vlan id 2
 * 3. Start './mbim-proxy' with -d 'device'
 * 4. Start Clients: ./quectel-CM -n id1
 * 5. Start Clients: ./quectel-CM -n id2
 * ...
 * Notice:
 *      mbim-proxy can work in backgroud as a daemon
 *      '-n' sessionID
 *  The modem may not support multi-PDN mode or how many PDN it supports is undefined. It depends!!!
 *  Besides, some modem also may not support some sessionID. For instance EC20 doesn't support SessionId 1... 
 */
int main(int argc, char **argv)
{
    int optidx = 0;
    int opt;
    char *optstr = "d:vh";
    const char *device = "/dev/cdc-wdm0";

    struct option options[] = {
        {"verbose", no_argument,        NULL, 'v'},
        {"device", required_argument,   NULL, 'd'},
        {0, 0, 0, 0},
    };
    while ((opt = getopt_long(argc, argv, optstr, options, &optidx)) != -1) {
        switch (opt) {
        case 'v':
            verbose = 1;
            break;
        case 'd':
            device = optarg;
            break;
        case 'h':
            mbim_debug("-h              Show this message\n");
            mbim_debug("-v              Verbose\n");
            mbim_debug("-d [device]     MBIM device\n");
            return 0;
        default:
            mbim_debug("illegal argument\n");
            return -1;
        }
    }

    if (!device) {
        mbim_debug("Missing parameter: device\n");
        return -1;
    }

    while (1) {
        int mbim_dev_fd = open(device, O_RDWR | O_NONBLOCK | O_NOCTTY);
        if (mbim_dev_fd < 0) {
            mbim_debug("cannot open mbim_device %s: %s\n", device, strerror(errno));
            sleep(2);
            continue;
        }
        mbim_debug ("mbim_dev_fd=%d\n", mbim_dev_fd);

        memset(cm_clients, 0, sizeof(cm_clients));
        mbim_send_open_msg(mbim_dev_fd, sizeof(cm_recv_buffer));
        proxy_loop(mbim_dev_fd);
        safe_close(mbim_dev_fd);
    }

    return -1;
}
