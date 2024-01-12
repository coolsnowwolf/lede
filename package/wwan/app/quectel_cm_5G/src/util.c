/******************************************************************************
  @file    util.c
  @brief   some utils for this QCM tool.

  DESCRIPTION
  Connectivity Management Tool for USB network adapter of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <sys/time.h>
#include <net/if.h>
typedef unsigned short sa_family_t;
#include <linux/un.h>

#if defined(__STDC__)
#include <stdarg.h>
#define __V(x)	x
#else
#include <varargs.h>
#define __V(x)	(va_alist) va_dcl
#define const
#define volatile
#endif

#include <syslog.h>

#include "QMIThread.h"

pthread_mutex_t cm_command_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cm_command_cond = PTHREAD_COND_INITIALIZER;
unsigned int cm_recv_buf[1024];

int cm_open_dev(const char *dev) {
    int fd;

    fd = open(dev, O_RDWR | O_NONBLOCK | O_NOCTTY);
    if (fd != -1) {
        fcntl(fd, F_SETFL, fcntl(fd,F_GETFL) | O_NONBLOCK);
        fcntl(fd, F_SETFD, FD_CLOEXEC);

        if (!strncmp(dev, "/dev/tty", strlen("/dev/tty")))
        {
            //disable echo on serial ports 
            struct termios  ios;

            memset(&ios, 0, sizeof(ios));
            tcgetattr( fd, &ios );
            cfmakeraw(&ios);
            cfsetispeed(&ios, B115200);
            cfsetospeed(&ios, B115200);
            tcsetattr( fd, TCSANOW, &ios );
            tcflush(fd, TCIOFLUSH);
        }
    } else {
        dbg_time("Failed to open %s, errno: %d (%s)", dev, errno, strerror(errno));
    }

    return fd;
}

int cm_open_proxy(const char *name) {
    int sockfd = -1;
    int reuse_addr = 1;
    struct sockaddr_un sockaddr;
    socklen_t alen;

    /*Create server socket*/
    sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sockfd < 0)
        return sockfd;

    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sun_family = AF_LOCAL;
    sockaddr.sun_path[0] = 0;
    memcpy(sockaddr.sun_path + 1, name, strlen(name) );

    alen = strlen(name) + offsetof(struct sockaddr_un, sun_path) + 1;
    if(connect(sockfd, (struct sockaddr *)&sockaddr, alen) < 0) {
        close(sockfd);
        dbg_time("connect %s errno: %d (%s)", name, errno, strerror(errno));
        return -1;
    }
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,sizeof(reuse_addr));
    fcntl(sockfd, F_SETFL, fcntl(sockfd,F_GETFL) | O_NONBLOCK);
    fcntl(sockfd, F_SETFD, FD_CLOEXEC);

    dbg_time("connect to %s sockfd = %d", name, sockfd);

    return sockfd;
}

static void setTimespecRelative(struct timespec *p_ts, long long msec)
{
    struct timeval tv;

    gettimeofday(&tv, (struct timezone *) NULL);

    /* what's really funny about this is that I know
       pthread_cond_timedwait just turns around and makes this
       a relative time again */
    p_ts->tv_sec = tv.tv_sec + (msec / 1000);
    p_ts->tv_nsec = (tv.tv_usec + (msec % 1000) * 1000L ) * 1000L;
    if ((unsigned long)p_ts->tv_nsec >= 1000000000UL) {
        p_ts->tv_sec += 1;
        p_ts->tv_nsec -= 1000000000UL;
    }
}

int pthread_cond_timeout_np(pthread_cond_t *cond, pthread_mutex_t * mutex, unsigned msecs) {
    if (msecs != 0) {
        unsigned i;
        unsigned t = msecs/4;
        int ret = 0;

        if (t == 0)
            t = 1;

        for (i = 0; i < msecs; i += t) {
            struct timespec ts;
            setTimespecRelative(&ts, t);
//very old uclibc do not support pthread_condattr_setclock(CLOCK_MONOTONIC)
            ret = pthread_cond_timedwait(cond, mutex, &ts); //to advoid system time change
            if (ret != ETIMEDOUT) {
                if(ret) dbg_time("ret=%d, msecs=%u, t=%u", ret, msecs, t);
                break;
            }
        }

        return ret;
    } else {
        return pthread_cond_wait(cond, mutex);
    }
}

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
    sprintf(time_buf, "%02d-%02d_%02d:%02d:%02d:%03d", ti->tm_mon+1, ti->tm_mday, ti->tm_hour, ti->tm_min, ti->tm_sec, (int)millitm);
    return time_buf;
}

unsigned long clock_msec(void)
{
	struct timespec tm;
	clock_gettime( CLOCK_MONOTONIC, &tm);
	return (unsigned long)(tm.tv_sec*1000 + (tm.tv_nsec/1000000));
}

FILE *logfilefp = NULL;

void update_resolv_conf(int iptype, const char *ifname, const char *dns1, const char *dns2) {
    const char *dns_file = "/etc/resolv.conf";
    FILE *dns_fp;
    char dns_line[256];
    #define MAX_DNS 16
    char *dns_info[MAX_DNS];
    char dns_tag[64];
    int dns_match = 0;
    int i;

    snprintf(dns_tag, sizeof(dns_tag), "# IPV%d %s", iptype, ifname);

    for (i = 0; i < MAX_DNS; i++)
        dns_info[i] = NULL;
    
    dns_fp = fopen(dns_file, "r");
    if (dns_fp) {
        i = 0;    
        dns_line[sizeof(dns_line)-1] = '\0';
        
        while((fgets(dns_line, sizeof(dns_line)-1, dns_fp)) != NULL) {
            if ((strlen(dns_line) > 1) && (dns_line[strlen(dns_line) - 1] == '\n'))
                dns_line[strlen(dns_line) - 1] = '\0';
            //dbg_time("%s", dns_line);
            if (strstr(dns_line, dns_tag)) {
                dns_match++;
                continue;
            }
            dns_info[i++] = strdup(dns_line);
            if (i == MAX_DNS)
                break;
        }

        fclose(dns_fp);
    }
    else if (errno != ENOENT) {
        dbg_time("fopen %s fail, errno:%d (%s)", dns_file, errno, strerror(errno));
        return;
    }
    
    if (dns1 == NULL && dns_match == 0)
        return;

    dns_fp = fopen(dns_file, "w");
    if (dns_fp) {
        if (dns1)
            fprintf(dns_fp, "nameserver %s %s\n", dns1, dns_tag);
        if (dns2)
            fprintf(dns_fp, "nameserver %s %s\n", dns2, dns_tag);
        
        for (i = 0; i < MAX_DNS && dns_info[i]; i++)
            fprintf(dns_fp, "%s\n", dns_info[i]);
        fclose(dns_fp);
    }
    else {
        dbg_time("fopen %s fail, errno:%d (%s)", dns_file, errno, strerror(errno));
    }

    for (i = 0; i < MAX_DNS && dns_info[i]; i++)
        free(dns_info[i]);
}

pid_t getpid_by_pdp(int pdp, const char* program_name)
{
    glob_t gt;
    int ret;
    char filter[16];
    pid_t pid;

    snprintf(filter, sizeof(filter), "-n %d", pdp);
    ret = glob("/proc/*/cmdline", GLOB_NOSORT, NULL, &gt);
    if (ret != 0) {
        dbg_time("glob error, errno = %d(%s)", errno, strerror(errno));
		return -1;
    } else {
        int i = 0, fd = -1;
        ssize_t nreads;
        char cmdline[512] = {0};

		for (i = 0; i < (int)gt.gl_pathc; i++) {
            fd = open(gt.gl_pathv[i], O_RDONLY);
            if (fd == -1) {
                dbg_time("open %s failed, errno = %d(%s)", gt.gl_pathv[i], errno, strerror(errno));
                globfree(&gt);
                return -1;
            }
            
            nreads = read(fd, cmdline, sizeof(cmdline));
            if (nreads > 0) {
                int pos = 0;
                while (pos < nreads-1) {
                    if (cmdline[pos] == '\0')
                        cmdline[pos] = ' ';  // space
                    pos++;
                }
                // printf("%s\n", cmdline);
            }

            if (strstr(cmdline, program_name) && strstr(cmdline, filter)) {
            	char path[64] = {0};
            	char pidstr[64] = {0};
            	char *p;
            	
                dbg_time("%s: %s", gt.gl_pathv[i], cmdline);
            	strcpy(path, gt.gl_pathv[i]);
            	p = strstr(gt.gl_pathv[i], "/cmdline");
            	*p = '\0';
            	while (*(--p) != '/') ;
            		
            	strcpy(pidstr, p+1);
            	pid = atoi(pidstr);
            	globfree(&gt);
                
                return pid;
            }
        }
    }

    globfree(&gt);
    return -1;
}

void ql_get_driver_rmnet_info(PROFILE_T *profile, RMNET_INFO *rmnet_info) {
    int ifc_ctl_sock;
    struct ifreq ifr;
    int rc;
    int request = 0x89F3;
    unsigned char data[512];

    memset(rmnet_info, 0x00, sizeof(*rmnet_info));

    ifc_ctl_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (ifc_ctl_sock <= 0) {
        dbg_time("socket() failed: %s\n", strerror(errno));
        return;
    }
    
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, profile->usbnet_adapter, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;    
    ifr.ifr_ifru.ifru_data = (void *)data;
        
    rc = ioctl(ifc_ctl_sock, request, &ifr);
    if (rc < 0) {
        if (errno != ENOTSUP)
            dbg_time("ioctl(0x%x, qmap_settings) errno:%d (%s), rc=%d", request, errno, strerror(errno), rc);
    }
    else {
        memcpy(rmnet_info, data, sizeof(*rmnet_info));
    }

    close(ifc_ctl_sock);
}

void ql_set_driver_qmap_setting(PROFILE_T *profile, QMAP_SETTING *qmap_settings) {
    int ifc_ctl_sock;
    struct ifreq ifr;
    int rc;
    int request = 0x89F2;

    ifc_ctl_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (ifc_ctl_sock <= 0) {
        dbg_time("socket() failed: %s\n", strerror(errno));
        return;
    }
    
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, profile->usbnet_adapter, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;    
    ifr.ifr_ifru.ifru_data = (void *)qmap_settings;
        
    rc = ioctl(ifc_ctl_sock, request, &ifr);
    if (rc < 0) {
        dbg_time("ioctl(0x%x, qmap_settings) failed: %s, rc=%d", request, strerror(errno), rc);
    }

    close(ifc_ctl_sock);	
}

void no_trunc_strncpy(char *dest, const char *src, size_t dest_size)
{
    size_t i = 0;

    for (i = 0; i < dest_size && *src; i++) {
        *dest++ = *src++;
    }

    *dest = 0;
}
