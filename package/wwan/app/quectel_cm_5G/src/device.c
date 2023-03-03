/******************************************************************************
  @file    device.c
  @brief   QMI device dirver.

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
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <strings.h>
#include <stdlib.h>
#include <limits.h>
#include <linux/usbdevice_fs.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <time.h>
#include <pthread.h>

#include "QMIThread.h"
#include "ethtool-copy.h"

#define USB_CLASS_VENDOR_SPEC		0xff
#define USB_CLASS_COMM			2
#define USB_CDC_SUBCLASS_ACM			0x02
#define USB_CDC_SUBCLASS_ETHERNET       0x06
#define USB_CDC_SUBCLASS_NCM			0x0d
#define USB_CDC_SUBCLASS_MBIM			0x0e
#define USB_CLASS_WIRELESS_CONTROLLER	0xe0

#define CM_MAX_PATHLEN 256

#define CM_INVALID_VAL (~((int)0))
/* get first line from file 'fname'
 * And convert the content into a hex number, then return this number */
static int file_get_value(const char *fname, int base)
{
    FILE *fp = NULL;
    long num;
    char buff[32 + 1] = {'\0'};
    char *endptr = NULL;

    fp = fopen(fname, "r");
    if (!fp) goto error;
    if (fgets(buff, sizeof(buff), fp) == NULL)
        goto error;
    fclose(fp);

    num = (int)strtol(buff, &endptr, base);
    if (errno == ERANGE && (num == LONG_MAX || num == LONG_MIN))
        goto error;
    /* if there is no digit in buff */
    if (endptr == buff)
        goto error;

    if (debug_qmi)
        dbg_time("(%s) = %lx", fname, num);
    return (int)num;

error:
    if (fp) fclose(fp);
    return CM_INVALID_VAL;
}

/*
 * This function will search the directory 'dirname' and return the first child.
 * '.' and '..' is ignored by default
 */
static int dir_get_child(const char *dirname, char *buff, unsigned bufsize, const char *prefix)
{
    struct dirent *entptr = NULL;
    DIR *dirptr;

    buff[0] = 0;

    dirptr = opendir(dirname);
    if (!dirptr)
        return -1;

    while ((entptr = readdir(dirptr))) {
        if (entptr->d_name[0] == '.')
            continue;
        if (prefix && strlen(prefix) && strncmp(entptr->d_name, prefix, strlen(prefix)))
            continue;
        snprintf(buff, bufsize, "%.31s", entptr->d_name);
        break;
    }
    closedir(dirptr);

    return 0;
}

static int conf_get_val(const char *fname, const char *key)
{
    char buff[128] = {'\0'};
    FILE *fp = fopen(fname, "r");
    if (!fp)
        return CM_INVALID_VAL;

    while (fgets(buff, sizeof(buff)-1, fp)) {
        char prefix[128] = {'\0'};
        char tail[128] = {'\0'};
        /* To eliminate cppcheck warnning: Assume string length is no more than 15 */
        sscanf(buff, "%15[^=]=%15s", prefix, tail);
        if (!strncasecmp(prefix, key, strlen(key))) {
            fclose(fp);
            return atoi(tail);
        }
    }

    fclose(fp);
    return CM_INVALID_VAL;
}

static void query_usb_device_info(char *path, struct usb_device_info *p) {
    size_t offset = strlen(path);

    memset(p, 0, sizeof(*p));

    path[offset] = '\0';
    strcat(path, "/idVendor");
    p->idVendor = file_get_value(path, 16);

    if (p->idVendor == CM_INVALID_VAL)
        return;

    path[offset] = '\0';
    strcat(path, "/idProduct");
    p->idProduct = file_get_value(path, 16);

    path[offset] = '\0';
    strcat(path, "/busnum");
    p->busnum = file_get_value(path, 10);

    path[offset] = '\0';
    strcat(path, "/devnum");
    p->devnum = file_get_value(path, 10);

    path[offset] = '\0';
    strcat(path, "/bNumInterfaces");
    p->bNumInterfaces = file_get_value(path, 10);

    path[offset] = '\0';
}

static void query_usb_interface_info(char *path, struct usb_interface_info *p) {
    char driver[128];
    size_t offset = strlen(path);
    int n;

    memset(p, 0, sizeof(*p));

    path[offset] = '\0';
    strcat(path, "/bNumEndpoints");
    p->bInterfaceClass = file_get_value(path, 16);

    path[offset] = '\0';
    strcat(path, "/bInterfaceClass");
    p->bInterfaceClass = file_get_value(path, 16);

    path[offset] = '\0';
    strcat(path, "/bInterfaceSubClass");
    p->bInterfaceSubClass = file_get_value(path, 16);

    path[offset] = '\0';
    strcat(path, "/bInterfaceProtocol");
    p->bInterfaceProtocol = file_get_value(path, 16);

    path[offset] = '\0';
    strcat(path, "/driver");
    n = readlink(path, driver, sizeof(driver));
    if (n > 0) {
        driver[n] = 0;
        if (debug_qmi) dbg_time("driver -> %s", driver);
        n = strlen(driver);
        while (n > 0) {
            if (driver[n] == '/')
                break;
            n--;
        }
        strncpy(p->driver, &driver[n+1], sizeof(p->driver) - 1);
    }

    path[offset] = '\0';
}

static int detect_path_cdc_wdm_or_qcqmi(char *path, char *devname, size_t bufsize)
{
    size_t offset = strlen(path);
    char tmp[32];
    
    devname[0] = 0;

    if (access(path, R_OK))
        return -1;

    path[offset] = '\0';
    strcat(path, "/GobiQMI");
    if (!access(path, R_OK))
        goto step_1;

    path[offset] = '\0';
    strcat(path, "/usbmisc");
    if (!access(path, R_OK))
        goto step_1;

    path[offset] = '\0';
    strcat(path, "/usb");
    if (!access(path, R_OK))
        goto step_1;

    return -1;

step_1: 
    /* get device(qcqmiX|cdc-wdmX) */
    if (debug_qmi) dbg_time("%s", path);
    dir_get_child(path, tmp, sizeof(tmp), NULL);
    if (tmp[0] == '\0')
        return -1;

    /* There is a chance that, no device(qcqmiX|cdc-wdmX) is generated. We should warn user about that! */
    snprintf(devname, bufsize, "/dev/%s", tmp);
    if (access(devname, R_OK | F_OK) && errno == ENOENT)
    {
        int major, minor;

        dbg_time("access %s  failed, errno: %d (%s)", devname, errno, strerror(errno));
        strcat(path, "/");
        strcat(path, tmp);
        strcat(path, "/uevent");
        major = conf_get_val(path, "MAJOR");
        minor = conf_get_val(path, "MINOR");

        if(major == CM_INVALID_VAL || minor == CM_INVALID_VAL)
            dbg_time("get major and minor failed");
         else if (mknod(devname, S_IFCHR|0666, (((major & 0xfff) << 8) | (minor & 0xff) | ((minor & 0xfff00) << 12))))
            dbg_time("please mknod %s c %d %d", devname, major, minor);
    }

    return 0;
}

/* To detect the device info of the modem.
 * return:
 *  FALSE -> fail
 *  TRUE -> ok
 */
BOOL qmidevice_detect(char *qmichannel, char *usbnet_adapter, unsigned bufsize, PROFILE_T *profile) {
    struct dirent* ent = NULL;
    DIR *pDir;
    const char *rootdir = "/sys/bus/usb/devices";
    struct {
        char path[255*2];
    } *pl;
    pl = (typeof(pl)) malloc(sizeof(*pl));
    memset(pl, 0x00, sizeof(*pl));

    pDir = opendir(rootdir);
    if (!pDir) {
        dbg_time("opendir %s failed: %s", rootdir, strerror(errno));
        goto error;
    }

    while ((ent = readdir(pDir)) != NULL)  {
        char netcard[32+1] = {'\0'};
        char devname[32+5] = {'\0'}; //+strlen("/dev/")
        int netIntf;
        int driver_type;

        if (ent->d_name[0] == 'u')
            continue;

        snprintf(pl->path, sizeof(pl->path), "%s/%s", rootdir, ent->d_name);
        query_usb_device_info(pl->path, &profile->usb_dev);
        if (profile->usb_dev.idVendor == CM_INVALID_VAL)
            continue;

        if (profile->usb_dev.idVendor == 0x2c7c || profile->usb_dev.idVendor == 0x05c6) {
            dbg_time("Find %s/%s idVendor=0x%x idProduct=0x%x, bus=0x%03x, dev=0x%03x",
                rootdir, ent->d_name, profile->usb_dev.idVendor, profile->usb_dev.idProduct,
                profile->usb_dev.busnum, profile->usb_dev.devnum);
        }

        /* get network interface */
        /* NOTICE: there is a case that, bNumberInterface=6, but the net interface is 8 */
        /* toolchain-mips_24kc_gcc-5.4.0_musl donot support GLOB_BRACE */
        /* RG500U's MBIM is at inteface 0 */
        for (netIntf = 0;  netIntf < (profile->usb_dev.bNumInterfaces + 8); netIntf++) {
            snprintf(pl->path, sizeof(pl->path), "%s/%s:1.%d/net", rootdir, ent->d_name, netIntf);
            dir_get_child(pl->path, netcard, sizeof(netcard), NULL);
            if (netcard[0])
                break;
        }

        if (netcard[0] == '\0') { //for centos 2.6.x
            const char *n= "usb0";
            const char *c = "qcqmi0";

            snprintf(pl->path, sizeof(pl->path), "%s/%s:1.4/net:%s", rootdir, ent->d_name, n);
            if (!access(pl->path, F_OK)) {
                snprintf(pl->path, sizeof(pl->path), "%s/%s:1.4/GobiQMI:%s", rootdir, ent->d_name, c);
                if (!access(pl->path, F_OK)) {
                    snprintf(qmichannel, bufsize, "/dev/%s", c);
                    snprintf(usbnet_adapter, bufsize, "%s", n);
                    snprintf(pl->path, sizeof(pl->path), "%s/%s:1.4", rootdir, ent->d_name);
                    query_usb_interface_info(pl->path, &profile->usb_intf);
                    break;
                }
            }
        }

        if (netcard[0] == '\0')
            continue;

        /* not '-i iface' */
        if (usbnet_adapter[0] && strcmp(usbnet_adapter, netcard))
            continue;

        snprintf(pl->path, sizeof(pl->path), "%s/%s:1.%d", rootdir, ent->d_name, netIntf);
        query_usb_interface_info(pl->path, &profile->usb_intf);
        driver_type = get_driver_type(profile);

        if (driver_type == SOFTWARE_QMI || driver_type == SOFTWARE_MBIM) {
            detect_path_cdc_wdm_or_qcqmi(pl->path, devname, sizeof(devname));
        }
        else if (driver_type == SOFTWARE_ECM_RNDIS_NCM)
        {
            int atIntf = -1;

            if (profile->usb_dev.idVendor == 0x2c7c) { //Quectel
                switch (profile->usb_dev.idProduct) { //EC200U
                case 0x0901: //EC200U
                case 0x8101: //RG801H
                    atIntf = 2;
                break;
                case 0x0900: //RG500U
                    atIntf = 4;
                break;
                case 0x6026: //EC200T
                case 0x6005: //EC200A
                case 0x6002: //EC200S
                case 0x6001: //EC100Y
                    atIntf = 3;
                break;
                default:
                   dbg_time("unknow at interface for USB idProduct:%04x\n", profile->usb_dev.idProduct);
                break;
                }
            }

            if (atIntf != -1) {
                snprintf(pl->path, sizeof(pl->path), "%s/%s:1.%d", rootdir, ent->d_name, atIntf);
                dir_get_child(pl->path, devname, sizeof(devname), "tty");
                if (devname[0] && !strcmp(devname, "tty")) {
                    snprintf(pl->path, sizeof(pl->path), "%s/%s:1.%d/tty", rootdir, ent->d_name, atIntf);
                    dir_get_child(pl->path, devname, sizeof(devname), "tty");
                }
            }
        }
        
        if (netcard[0] && devname[0]) {
            if (devname[0] == '/')
                snprintf(qmichannel, bufsize, "%s", devname);
            else
                snprintf(qmichannel, bufsize, "/dev/%s", devname);
            snprintf(usbnet_adapter, bufsize, "%s", netcard);
            dbg_time("Auto find qmichannel = %s", qmichannel);
            dbg_time("Auto find usbnet_adapter = %s", usbnet_adapter);
            break;
        }
    }
    closedir(pDir);

    if (qmichannel[0] == '\0' || usbnet_adapter[0] == '\0') {
        dbg_time("network interface '%s' or qmidev '%s' is not exist", usbnet_adapter, qmichannel);
        goto error;
    }
    free(pl);
    return TRUE;
error:
    free(pl);
    return FALSE;
}

int mhidevice_detect(char *qmichannel, char *usbnet_adapter, PROFILE_T *profile) {
    struct dirent* ent = NULL;
    DIR *pDir;
    const char *rootdir_mhi[] = {"/sys/bus/mhi_q/devices", "/sys/bus/mhi/devices", NULL};
    int i = 0;
    char path[256];
    int find = 0;

    while (rootdir_mhi[i]) {
        const char *rootdir = rootdir_mhi[i++];

        pDir = opendir(rootdir);
        if (!pDir) {
            if (errno != ENOENT)
                    dbg_time("opendir %s failed: %s", rootdir, strerror(errno));
            continue;
        }

        while ((ent = readdir(pDir)) != NULL)  {
            char netcard[32] = {'\0'};
            char devname[32] = {'\0'};
            int software_interface = SOFTWARE_QMI;
            char *pNode = NULL;

            pNode = strstr(ent->d_name, "_IP_HW0"); //0306_00.01.00_IP_HW0
            if (!pNode)
                continue;

            snprintf(path, sizeof(path), "%s/%.32s/net", rootdir, ent->d_name);
            dir_get_child(path, netcard, sizeof(netcard), NULL);
            if (!netcard[0])
                continue;

            if (usbnet_adapter[0] && strcmp(netcard, usbnet_adapter)) //not '-i x'
                continue;

            if (!strcmp(rootdir, "/sys/bus/mhi/devices")) {
                snprintf(path, sizeof(path), "%s/%.13s_IPCR", rootdir, ent->d_name); // 13 is sizeof(0306_00.01.00)
                if (!access(path, F_OK)) {
                    /* we also need 'cat /dev/mhi_0306_00.01.00_pipe_14' to enable rmnet as like USB's DTR 
                         or will get error 'requestSetEthMode QMUXResult = 0x1, QMUXError = 0x46' */
                    sprintf(usbnet_adapter, "%s", netcard);
                    sprintf(qmichannel, "qrtr-%d", 3); // 3 is sdx modem's node id
                    profile->software_interface = SOFTWARE_QRTR;
                    find = 1;
                    break;
                }
                continue;
            }

            snprintf(path, sizeof(path), "%s/%.13s_IPCR", rootdir, ent->d_name);
            if (access(path, F_OK)) {
                snprintf(path, sizeof(path), "%s/%.13s_QMI0", rootdir, ent->d_name);
                if (access(path, F_OK)) {
                    snprintf(path, sizeof(path), "%s/%.13s_MBIM", rootdir, ent->d_name);
                    if (!access(path, F_OK))
                        software_interface = SOFTWARE_MBIM;
                }
            }
            if (access(path, F_OK))
                continue;

            strncat(path, "/mhi_uci_q", sizeof(path)-1);
            dir_get_child(path, devname, sizeof(devname), NULL);
            if (!devname[0])
                continue;      

            sprintf(usbnet_adapter, "%s", netcard);
            sprintf(qmichannel, "/dev/%s", devname);
            profile->software_interface = software_interface;
            find = 1;
            break;
        }

        closedir(pDir);
    }

    return find;
}

int atdevice_detect(char *atchannel, char *usbnet_adapter, PROFILE_T *profile) {
    if (!access("/sys/class/net/sipa_dummy0", F_OK)) {
		strcpy(usbnet_adapter, "sipa_dummy0");
		snprintf(profile->qmapnet_adapter, sizeof(profile->qmapnet_adapter), "%s%d", "pcie", profile->pdp - 1);
    }
	else {
        dbg_time("atdevice_detect failed");
        goto error;
    }

    if (!access("/dev/stty_nr31", F_OK)) {
        strcpy(atchannel, "/dev/stty_nr31");
        profile->software_interface = SOFTWARE_ECM_RNDIS_NCM;
    }
    else {
        goto error;
    }

    return 1;
error:
    return 0;
}


int get_driver_type(PROFILE_T *profile)
{
    /* QMI_WWAN */
    if (profile->usb_intf.bInterfaceClass == USB_CLASS_VENDOR_SPEC) {
        return SOFTWARE_QMI;
    }
    else if (profile->usb_intf.bInterfaceClass == USB_CLASS_COMM) {
        switch (profile->usb_intf.bInterfaceSubClass) {
            case USB_CDC_SUBCLASS_MBIM:
                return SOFTWARE_MBIM;
            break;
            case USB_CDC_SUBCLASS_ETHERNET:
            case USB_CDC_SUBCLASS_NCM:
                return SOFTWARE_ECM_RNDIS_NCM;
            break;
            default:
            break;
        }
    }
    else if (profile->usb_intf.bInterfaceClass == USB_CLASS_WIRELESS_CONTROLLER) {
        if (profile->usb_intf.bInterfaceSubClass == 1 && profile->usb_intf.bInterfaceProtocol == 3)
            return SOFTWARE_ECM_RNDIS_NCM;
    }

    dbg_time("%s unknow bInterfaceClass=%d, bInterfaceSubClass=%d", __func__,
        profile->usb_intf.bInterfaceClass, profile->usb_intf.bInterfaceSubClass);
    return DRV_INVALID;
}

struct usbfs_getdriver
{
    unsigned int interface;
    char driver[255 + 1];
};

struct usbfs_ioctl
{
    int ifno;       /* interface 0..N ; negative numbers reserved */
    int ioctl_code; /* MUST encode size + direction of data so the
			 * macros in <asm/ioctl.h> give correct values */
    void *data;     /* param buffer (in, or out) */
};

#define IOCTL_USBFS_DISCONNECT	_IO('U', 22)
#define IOCTL_USBFS_CONNECT	_IO('U', 23)

int usbfs_is_kernel_driver_alive(int fd, int ifnum)
{
    struct usbfs_getdriver getdrv;
    getdrv.interface = ifnum;
    if (ioctl(fd, USBDEVFS_GETDRIVER, &getdrv) < 0) {
        dbg_time("%s ioctl USBDEVFS_GETDRIVER failed, kernel driver may be inactive", __func__);
        return 0;
    }
    dbg_time("%s find interface %d has match the driver %s", __func__, ifnum, getdrv.driver);
    return 1;
}

void usbfs_detach_kernel_driver(int fd, int ifnum)
{
    struct usbfs_ioctl operate;
    operate.data = NULL;
    operate.ifno = ifnum;
    operate.ioctl_code = IOCTL_USBFS_DISCONNECT;
    if (ioctl(fd, USBDEVFS_IOCTL, &operate) < 0) {
        dbg_time("%s detach kernel driver failed", __func__);
    } else {
        dbg_time("%s detach kernel driver success", __func__);
    }
}

void usbfs_attach_kernel_driver(int fd, int ifnum)
{
    struct usbfs_ioctl operate;
    operate.data = NULL;
    operate.ifno = ifnum;
    operate.ioctl_code = IOCTL_USBFS_CONNECT;
    if (ioctl(fd, USBDEVFS_IOCTL, &operate) < 0) {
        dbg_time("%s detach kernel driver failed", __func__);
    } else {
        dbg_time("%s detach kernel driver success", __func__);
    }
}

int reattach_driver(PROFILE_T *profile)
{
    int ifnum = 4;
    int fd;
    char devpath[128] = {'\0'};
    snprintf(devpath, sizeof(devpath), "/dev/bus/usb/%03d/%03d", profile->usb_dev.busnum, profile->usb_dev.devnum);
    fd = open(devpath, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        dbg_time("%s fail to open %s", __func__, devpath);
        return -1;
    }
    usbfs_detach_kernel_driver(fd, ifnum);
    usbfs_attach_kernel_driver(fd, ifnum);
    close(fd);
    return 0;
}

#define SIOCETHTOOL     0x8946
int ql_get_netcard_driver_info(const char *devname)
{
    int fd = -1;
    struct ethtool_drvinfo drvinfo;
    struct ifreq ifr;	/* ifreq suitable for ethtool ioctl */

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, devname);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        dbg_time("Cannot get control socket: errno(%d)(%s)", errno, strerror(errno));
        return -1;
    }

    drvinfo.cmd = ETHTOOL_GDRVINFO;
    ifr.ifr_data = (void *)&drvinfo;

    if (ioctl(fd, SIOCETHTOOL, &ifr) < 0) {
        dbg_time("ioctl() error: errno(%d)(%s)", errno, strerror(errno));
        close(fd);
        return -1;
    }

    dbg_time("netcard driver = %s, driver version = %s", drvinfo.driver, drvinfo.version);

    close(fd);

    return 0;
}

int ql_get_netcard_carrier_state(const char *devname)
{
    int fd = -1;
    struct ethtool_value edata;
    struct ifreq ifr;	/* ifreq suitable for ethtool ioctl */

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, devname);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        dbg_time("Cannot get control socket: errno(%d)(%s)", errno, strerror(errno));
        return -1;
    }

    edata.cmd = ETHTOOL_GLINK;
    edata.data = 0;
    ifr.ifr_data = (void *)&edata;

    if (ioctl(fd, SIOCETHTOOL, &ifr) < 0) {
        dbg_time("ioctl('%s') error: errno(%d)(%s)", devname, errno, strerror(errno));
        return -1;
    }

    if (!edata.data)
        dbg_time("netcard carrier = %d", edata.data);

    close(fd);

    return edata.data;
}

static void *catch_log(void *arg)
{
    PROFILE_T *profile = (PROFILE_T *)arg;
    int nreads = 0;
    char tbuff[256+32];
    char filter[32];
    size_t tsize = strlen(get_time()) + 1;

    snprintf(filter, sizeof(filter), ":%d:%03d:", profile->usb_dev.busnum, profile->usb_dev.devnum);

    while(1) {
        nreads = read(profile->usbmon_fd, tbuff + tsize, sizeof(tbuff) - tsize - 1);
        if (nreads <= 0) {
            if (nreads == -1 && errno == EINTR)
                continue;
            break;
        }

        tbuff[tsize+nreads] = '\0';   // printf("%s", buff);

        if (!strstr(tbuff+tsize, filter))
            continue;

        snprintf(tbuff, sizeof(tbuff), "%s", get_time());
        tbuff[tsize-1] = ' ';

        fwrite(tbuff, strlen(tbuff), 1, profile->usbmon_logfile_fp);
    }

    return NULL;
}

int ql_capture_usbmon_log(PROFILE_T *profile, const char *log_path)
{
    char usbmon_path[256];
    pthread_t pt;
    pthread_attr_t attr;

    if (access("/sys/module/usbmon", F_OK)) {
        dbg_time("usbmon is not load, please execute \"modprobe usbmon\" or \"insmod usbmon.ko\"");
        return -1;
    }

    if (access("/sys/kernel/debug/usb", F_OK)) {
        dbg_time("debugfs is not mount, please execute \"mount -t debugfs none_debugs /sys/kernel/debug\"");
        return -1;
    }

    snprintf(usbmon_path, sizeof(usbmon_path), "/sys/kernel/debug/usb/usbmon/%du", profile->usb_dev.busnum);
    profile->usbmon_fd = open(usbmon_path, O_RDONLY);
    if (profile->usbmon_fd < 0) {
        dbg_time("open %s error(%d) (%s)", usbmon_path, errno, strerror(errno));
        return -1;
    }

    snprintf(usbmon_path, sizeof(usbmon_path), "cat /sys/kernel/debug/usb/devices >> %s", log_path);
    if (system(usbmon_path) == -1) {};

    profile->usbmon_logfile_fp = fopen(log_path, "wb");
    if (!profile->usbmon_logfile_fp) {
      dbg_time("open %s error(%d) (%s)", log_path, errno, strerror(errno));
      close(profile->usbmon_fd);
      profile->usbmon_fd = -1;
      return -1;
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&pt, &attr, catch_log, (void *)profile);

    return 0;
}

void ql_stop_usbmon_log(PROFILE_T *profile) {
    if (profile->usbmon_fd > 0)
        close(profile->usbmon_fd);
    if (profile->usbmon_logfile_fp)
        fclose(profile->usbmon_logfile_fp);
}
