/******************************************************************************
  @file    quectel-atc-proxy.c
  @brief   atc proxy.

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
#include <sys/socket.h>
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
#include <endian.h>
#include <inttypes.h>

#include "qlist.h"
#include "QMIThread.h"
#include "atchannel.h"
#include "at_tok.h"

#define dprintf(fmt, args...) do { fprintf(stdout, "%s " fmt, get_time(), ##args); } while(0);
#define SYSCHECK(c) do{if((c)<0) {dprintf("%s %d error: '%s' (code: %d)\n", __func__, __LINE__, strerror(errno), errno); return -1;}}while(0)
#define cfmakenoblock(fd) do{fcntl(fd, F_SETFL, fcntl(fd,F_GETFL) | O_NONBLOCK);}while(0)

#define safe_free(__x) do { if (__x) { free((void *)__x); __x = NULL;}} while(0)
#define safe_at_response_free(__x) { if (__x) { at_response_free(__x); __x = NULL;}}

#define at_response_error(err, p_response) \
    (err \
    || p_response == NULL \
    || p_response->finalResponse == NULL \
    || p_response->success == 0)

typedef struct {
    struct qlistnode qnode;
    int ClientFd;
    unsigned AccessTime;
} ATC_PROXY_CONNECTION;

static int atc_proxy_quit = 0;
static pthread_t thread_id = 0;
static int atc_dev_fd = -1;
static int atc_proxy_server_fd = -1;
static struct qlistnode atc_proxy_connection;
static int verbose_debug = 0;
static int modem_reset_flag = 0;
static uint8_t atc_buf[4096];
static int asr_style_atc =  0;
extern int asprintf(char **s, const char *fmt, ...);
static ATC_PROXY_CONNECTION *current_client_fd = NULL;

static void dump_atc(uint8_t *pATC, int fd,int size, const char flag)
{
    if (verbose_debug) {
        printf("%c %d:\n", flag, fd);
        printf("%.*s\n", size, pATC);
    }
}

static int send_atc_to_client(int clientFd, uint8_t *pATC, int size) {
    struct pollfd pollfds[]= {{clientFd, POLLOUT, 0}};
    ssize_t ret = 0;

    do {
        ret = poll(pollfds, sizeof(pollfds)/sizeof(pollfds[0]), 5000);
    } while (ret == -1 && errno == EINTR && atc_proxy_quit == 0);

    if (pollfds[0].revents & POLLOUT) {
        ret = write(clientFd, pATC, size);
    }

    return ret;
}

static void onUnsolicited (const char *s, const char *sms_pdu)
{
    struct qlistnode *con_node;
    int ret;
    char buf[1024];

    if(s) {
        strcpy(buf, s);
        strcat(buf, "\r\n");
    }
    if(sms_pdu) {
        strcat(buf, sms_pdu);
        strcat(buf, "\r\n");
    }

    if(current_client_fd) {
        ATC_PROXY_CONNECTION *atc_con = current_client_fd;
        ret = send_atc_to_client(atc_con->ClientFd, (uint8_t *)buf, strlen(buf));
        if(ret < 0) {
            close(atc_con->ClientFd);
            qlist_remove(&atc_con->qnode);
            free(atc_con); 
        }
        return;
    }

    qlist_for_each(con_node, &atc_proxy_connection) {
        ATC_PROXY_CONNECTION *atc_con = qnode_to_item(con_node, ATC_PROXY_CONNECTION, qnode);
        if(atc_con && atc_con->ClientFd > 0) {
            ret = send_atc_to_client(atc_con->ClientFd, (uint8_t *)buf, strlen(buf));
            if(ret < 0) {
                close(atc_con->ClientFd);
                con_node = con_node->prev;
                qlist_remove(&atc_con->qnode); 
                free(atc_con);
                continue;
            }
        }
    }
}

static void onTimeout(void) {
    dprintf("%s", __func__);
    //TODO
}

static void onClose(void) {
    dprintf("%s", __func__);
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

static void accept_atc_connection(int serverfd) {
    int clientfd = -1;
    unsigned char addr[128];
    socklen_t alen = sizeof(addr);
    ATC_PROXY_CONNECTION *atc_con;

    clientfd = accept(serverfd, (struct sockaddr *)addr, &alen);

    atc_con = (ATC_PROXY_CONNECTION *)malloc(sizeof(ATC_PROXY_CONNECTION));
    if (atc_con) {
        qlist_init(&atc_con->qnode);
        atc_con->ClientFd= clientfd;
        atc_con->AccessTime = 0;
        dprintf("+++ ClientFd=%d\n", atc_con->ClientFd);
        qlist_add_tail(&atc_proxy_connection, &atc_con->qnode);
    }

    cfmakenoblock(clientfd);
}

static void cleanup_atc_connection(int clientfd) {
    struct qlistnode *con_node;
    
    qlist_for_each(con_node, &atc_proxy_connection) {
        ATC_PROXY_CONNECTION *atc_con = qnode_to_item(con_node, ATC_PROXY_CONNECTION, qnode);
        if (atc_con->ClientFd == clientfd) {
            dprintf("--- ClientFd=%d\n", atc_con->ClientFd);    
            close(atc_con->ClientFd);
            qlist_remove(&atc_con->qnode);
            free(atc_con);
            if (current_client_fd == atc_con)
                current_client_fd = NULL;
            break;
        }
    }
}

static int atc_proxy_init(void) {
    int err;
    char *cmd;
    ATResponse *p_response = NULL;

    err = at_handshake();
    if (err) {
        dprintf("handshake fail, TODO ... ");
        goto exit;
    }
	
    at_send_command_singleline("AT+QCFG=\"usbnet\"", "+QCFG:", NULL);
    at_send_command_multiline("AT+QNETDEVCTL=?", "+QNETDEVCTL:", NULL);
    at_send_command("AT+CGREG=2", NULL); //GPRS Network Registration Status
    at_send_command("AT+CEREG=2", NULL); //EPS Network Registration Status
    at_send_command("AT+C5GREG=2", NULL); //5GS Network Registration Status

    at_send_command_singleline("AT+QNETDEVSTATUS=?", "+QNETDEVSTATUS:", &p_response);
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
            }
            safe_at_response_free(p_response);
        }
        err = 0;
    }
    safe_at_response_free(p_response);

exit:
    return err;
}

static void atc_start_server(const char* servername) {
    atc_proxy_server_fd = create_local_server(servername);
    dprintf("atc_proxy_server_fd = %d\n", atc_proxy_server_fd);
    if (atc_proxy_server_fd == -1) {
        dprintf("Failed to create %s, errno: %d (%s)\n", servername, errno, strerror(errno));
    }
}

static void atc_close_server(const char* servername) {
    if (atc_proxy_server_fd != -1) {
        dprintf("%s %s close server\n", __func__, servername);
        close(atc_proxy_server_fd);
        atc_proxy_server_fd = -1;
    }
}

static void *atc_proxy_loop(void *param)
{
    uint8_t *pATC = atc_buf;
    struct qlistnode *con_node;
    ATC_PROXY_CONNECTION *atc_con;

    (void)param;
    dprintf("%s enter thread_id %p\n", __func__, (void *)pthread_self());

    qlist_init(&atc_proxy_connection);
    while (atc_dev_fd > 0 && atc_proxy_quit == 0) {
        struct pollfd pollfds[2+64];
        int ne, ret, nevents = 0;
        ssize_t nreads;

        pollfds[nevents].fd = atc_dev_fd;
        pollfds[nevents].events = POLLIN;
        pollfds[nevents].revents= 0;
        nevents++;
        
        if (atc_proxy_server_fd > 0) {
            pollfds[nevents].fd = atc_proxy_server_fd;
            pollfds[nevents].events = POLLIN;
            pollfds[nevents].revents= 0;
            nevents++;
        }

        qlist_for_each(con_node, &atc_proxy_connection) {
            atc_con = qnode_to_item(con_node, ATC_PROXY_CONNECTION, qnode);
            
            pollfds[nevents].fd = atc_con->ClientFd;
            pollfds[nevents].events = POLLIN;
            pollfds[nevents].revents= 0;
            nevents++;

            if (nevents == (sizeof(pollfds)/sizeof(pollfds[0])))
                break;
        }

        do {
            ret = poll(pollfds, nevents, (atc_proxy_server_fd > 0) ? -1 : 200);
         } while (ret == -1 && errno == EINTR && atc_proxy_quit == 0);
         
        if (ret < 0) {
            dprintf("%s poll=%d, errno: %d (%s)\n", __func__, ret, errno, strerror(errno));
            goto atc_proxy_loop_exit;
        }

        for (ne = 0; ne < nevents; ne++) {
            int fd = pollfds[ne].fd;
            short revents = pollfds[ne].revents;

            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                dprintf("%s poll fd = %d, revents = %04x\n", __func__, fd, revents);
                if (fd == atc_dev_fd) {
                    goto atc_proxy_loop_exit;
                } else if(fd == atc_proxy_server_fd) {
                
                } else {
                    cleanup_atc_connection(fd);
                }
                continue;
            }

            if (!(pollfds[ne].revents & POLLIN)) {
                continue;
            }

            if (fd == atc_proxy_server_fd) {
                accept_atc_connection(fd);
            }
            else if (fd == atc_dev_fd) {
                usleep(10*1000); //let atchannel.c read at response.
                if (modem_reset_flag)
                    goto atc_proxy_loop_exit;
            }
            else {
                memset(atc_buf, 0x0, sizeof(atc_buf));
                nreads = read(fd, pATC, sizeof(atc_buf));
                if (nreads <= 0) {
                    dprintf("%s read=%d errno: %d (%s)",  __func__, (int)nreads, errno, strerror(errno));
                    cleanup_atc_connection(fd);
                    break;
                }

                dump_atc(pATC, fd, nreads, 'r');
                qlist_for_each(con_node, &atc_proxy_connection) {
                    atc_con = qnode_to_item(con_node, ATC_PROXY_CONNECTION, qnode);
                    if (atc_con->ClientFd == pollfds[nevents].fd) {
                        current_client_fd = atc_con;
                        break;
                    }
                }
                at_send_command ((const char *)pATC, NULL);
                current_client_fd = NULL;
            }
        }
    }

atc_proxy_loop_exit:
    at_close();
    while (!qlist_empty(&atc_proxy_connection)) {
        ATC_PROXY_CONNECTION *atc_con = qnode_to_item(qlist_head(&atc_proxy_connection), ATC_PROXY_CONNECTION, qnode);
        cleanup_atc_connection(atc_con->ClientFd);
    }
    dprintf("%s exit, thread_id %p\n", __func__, (void *)pthread_self());

    return NULL;
}

static void usage(void) {
    dprintf(" -d <device_name>                      A valid atc device\n"
            "                                       default /dev/ttyUSB2, but /dev/ttyUSB2 may be invalid\n"
            " -i <netcard_name>                     netcard name\n"
            " -v                                    Will show all details\n");
}

static void sig_action(int sig) {
    if (atc_proxy_quit == 0) {
        atc_proxy_quit = 1;
        if (thread_id)
            pthread_kill(thread_id, sig);
    }
}

int main(int argc, char *argv[]) {
    int opt;
    char atc_dev[32+1] = "/dev/ttyUSB2";
    int retry_times = 0;
    char servername[64] = {0};

    optind = 1;
    signal(SIGINT, sig_action);

    while ( -1 != (opt = getopt(argc, argv, "d:i:vh"))) {
        switch (opt) {
            case 'd':
                strcpy(atc_dev, optarg);
                break;
            case 'v':
                verbose_debug = 1;
                break;
            default:
                usage();
                return 0;
        }
    }

    if (access(atc_dev, R_OK | W_OK)) {
        dprintf("Fail to access %s, errno: %d (%s). break\n", atc_dev, errno, strerror(errno));
        return -1;
    }

          sprintf(servername, "quectel-atc-proxy%c", atc_dev[strlen(atc_dev) - 1]);
    dprintf("Will use atc-dev='%s', proxy='%s'\n", atc_dev, servername);

    while (atc_proxy_quit == 0) {
        if (access(atc_dev, R_OK | W_OK)) {
            dprintf("Fail to access %s, errno: %d (%s). continue\n", atc_dev, errno, strerror(errno));
            // wait device
            sleep(3);
            continue;
        }

        atc_dev_fd = open(atc_dev, O_RDWR | O_NONBLOCK | O_NOCTTY);
        if (atc_dev_fd == -1) {
            dprintf("Failed to open %s, errno: %d (%s). break\n", atc_dev, errno, strerror(errno));
            return -1;
        }
        cfmakenoblock(atc_dev_fd);
        if (at_open(atc_dev_fd, onUnsolicited, 1)) {
            close(atc_dev_fd);
            atc_dev_fd = -1;
        }
        at_set_on_timeout(onTimeout);
        at_set_on_reader_closed(onClose);

        /* no atc_proxy_loop lives, create one */
        pthread_create(&thread_id, NULL, atc_proxy_loop, NULL);
        /* try to redo init if failed, init function must be successfully */
        while (atc_proxy_init() != 0) {
            if (retry_times < 5) {
                dprintf("fail to init proxy, try again in 2 seconds.\n");
                sleep(2);
                retry_times++;
            } else {
                dprintf("has failed too much times, restart the modem and have a try...\n");
                break;
            }
            /* break loop if modem is detached */
            if (access(atc_dev, F_OK|R_OK|W_OK))
                break;
        }
        retry_times = 0;
        atc_start_server(servername);
        if (atc_proxy_server_fd == -1)
            pthread_cancel(thread_id); 
        pthread_join(thread_id, NULL);

        /* close local server at last */
        atc_close_server(servername);
        close(atc_dev_fd);
        /* DO RESTART IN 20s IF MODEM RESET ITSELF */
        if (modem_reset_flag) {
            unsigned int time_to_wait = 20;
            while (time_to_wait) {
                time_to_wait = sleep(time_to_wait);
            }
            modem_reset_flag = 0;
        }
    }

    return 0;
}