#include <dirent.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>

//2021-02-25 willa.liu@fibocom.com changed begin for support eipd SN-20210129001
#include <string.h>
#include <stdio.h>
//2021-02-25 willa.liu@fibocom.com changed end for support eipd SN-20210129001

//2020-12-23 willa.liu@fibocom.com changed begin for support mantis 0065286
#include <semaphore.h>
#include <unistd.h>
//2020-12-23 willa.liu@fibocom.com changed end for support mantis 0065286

#include "QMIThread.h"
#include "util.h"
//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
#include "query_pcie_mode.h"
//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817

#define MAJOR 2
#define MINOR 0
#define REVISION 10
/*
 * Generally, we do not modify version info, so several modifications will share
 * the same version code. SUBVERSION is used for customized modification to
 * distinguise this version from previous one. SUBVERSION adds up before you
 * send the code to customers and it should be set to 0 if VERSION_STRING info
 * is changed.
 */
#define SUBVERSION 0
#define STRINGIFY_HELPER(v) #v
#define STRINGIFY(v) STRINGIFY_HELPER(v)
#define VERSION_STRING() \
    STRINGIFY(MAJOR) "." STRINGIFY(MINOR) "." STRINGIFY(REVISION)
#define MAX_PATH 256

//2020-12-23 willa.liu@fibocom.com changed begin for support mantis 0065286
struct sockaddr_un {
    sa_family_t    sun_family;
    char    sun_path[100];
};
//2020-12-23 willa.liu@fibocom.com changed end for support mantis 0065286

int query_pcie_mode = 0;
int debug_qmi = 0;
int main_loop = 0;
int qmidevice_control_fd[2];
static int signal_control_fd[2];
pthread_t gQmiThreadID;
USHORT g_MobileCountryCode = 0;
USHORT g_MobileNetworkCode = 0;

//2020-12-23 willa.liu@fibocom.com changed begin for support mantis 0065286
sem_t sem;
int socket_server_fd = 0;
int cli_accept_fd = 0;
char fibo_dial_filepath[100] = {0};
//2020-12-23 willa.liu@fibocom.com changed end for support mantis 0065286

extern const struct qmi_device_ops qmiwwan_qmidev_ops;
extern const struct qmi_device_ops gobi_qmidev_ops;

//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
//extern int *speed_arr;
//extern int *name_arr;
//extern int get_private_gateway_debug;
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817

static int s_link = -1;

static int is_pcie_dial()
{
    return (access("/dev/mhi_QMI0", F_OK) == 0);
}

//2021-02-25 willa.liu@fibocom.com changed begin for support eipd SN-20210129001
int split_gateway(char *str ,char* split, int index, char *outgateway)
{
    char chBuffer[1024];
    char *pchStrTmpIn = NULL;
    char *pchTmp = NULL;
    int i = 1;
    strncpy(chBuffer, str, sizeof(chBuffer)-1);
    pchTmp = chBuffer;
    //printf("pchTmp is [%s]\n", pchTmp);
    while(NULL != (pchTmp = strtok_r( pchTmp, split, &pchStrTmpIn)))
    {
        //printf("line %d::::::::::::[%s]\n", i, pchTmp);
        if(i == index)
        {
            memcpy(outgateway, pchTmp, strlen(pchTmp));
            break;
        }
        //if(strstr(pchTmp, "+CGCONTRDP") != -1)
            i++;
        pchTmp = NULL;
    }
    return strlen(outgateway);
}

int get_private_gateway(char *outgateway)
{
    int i;
    int fd;
    int ret;
    char buffer[1024] = {0};
    int rate;
    char *sendbuffer;
    char prigateways[1024*2] = {0};
    char prigateway[256] = {0};
    int totallen = 0;
    fd_set readfds;
    struct timeval timeout;
    struct termios tiosfd, tio;

    int timeoutVal = 5;

    char *dev = "/dev/ttyUSB1"; //The port under Linux is operated by opening the device file
    rate = 115200;
    sendbuffer = "AT+CGCONTRDP=1";

    fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(fd < 0)
        goto ERR;
    fcntl(fd, F_SETFL, O_RDWR);

    xget1(fd, &tio, &tiosfd);

    for(i = 0; i < sizeof(speed_arr)/sizeof(int); i++)
    {
        if(rate == name_arr[i])//Judge if the pass is equal to the pass
        {
            break;
        }
    }

    if(i >= sizeof(speed_arr)/sizeof(int))
    {
        printf("bound rate set failed\n");
        goto ERR;
    }

    cfsetspeed(&tio, speed_arr[i]);
    if(xset1(fd, &tio, dev))
        goto ERR;

    tcflush(fd, TCIOFLUSH);

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    sprintf(buffer, "%s\r", sendbuffer);
    ret = write (fd, buffer, strlen(buffer));
    if(ret < 0)
    {
        printf("write failed\n");
        goto ERR;
    }
    printf("dev: %s\nrate:%d\nsendbuffer:%s\n", dev, rate, sendbuffer);
    sleep(1);
    if(get_private_gateway_debug)printf("write %d\n", ret);
    while(1)
    {
        timeout.tv_sec = timeoutVal;
        timeout.tv_usec = 0;

        ret = select(fd+1, &readfds, (fd_set *)0, (fd_set *)0, &timeout);
        if(ret > 0)
        {
            ret = read(fd, buffer+totallen, sizeof(buffer)-totallen-1);
            if(ret < 0)
            {
                printf("read failed\n");
                goto ERR;
            }

            if(ret == 0)
            {
                goto ERR;
            }

            totallen += ret;
            buffer[totallen] = '\0';
            if(get_private_gateway_debug)printf("read %d(%s)\n", ret, &buffer[totallen-ret]);
            //printf("buffer is %s\n", buffer);

            if(totallen == sizeof(buffer)-1)
                break;

            if(strstr(buffer, "\nOK"))
            {
                split_gateway(buffer, "\r", 3, prigateways);
                //printf("test1 %s\n", prigateways);
                split_gateway(prigateways, ",", 5, prigateway);
                //printf("test2 %s\n", prigateway);
                break;
            }
            else
            {
                goto ERR;
            }

        }
        else
        {
            printf("select timeout\n");
            goto ERR;
        }

    }

    tcsetattr(fd, TCSAFLUSH, &tiosfd);
    if(strstr(buffer, "\nERROR") || strstr(buffer, "\n+CME ERROR:") || strstr(buffer, "\n+CMS ERROR:"))
        goto ERR;

    close(fd);

    memcpy(outgateway, prigateway+1, strlen(prigateway)-2);
    printf("private gateway is %s\n", outgateway);
    return 0;
ERR:
    if(fd > 0)
    close(fd);
    return -1;
}
//2021-02-25 willa.liu@fibocom.com changed end for support eipd SN-20210129001

// UINT ifc_get_addr(const char *ifname);
static void usbnet_link_change(int link, PROFILE_T *profile)
{
    //    static int s_link = 0;
    dbg_time("%s :link:%d ", __func__, link);

    if (s_link == link)
    return;

    s_link = link;

    if (link) {
    if (profile->ipv4_flag)
        requestGetIPAddress(profile, IpFamilyV4);
    if (profile->ipv6_flag)
        requestGetIPAddress(profile, IpFamilyV6);
       if (qmidev_is_pciemhi(profile->qmichannel))
            udhcpc_start_pcie(profile);
       else
        udhcpc_start(profile);
    } else {
       if (qmidev_is_pciemhi(profile->qmichannel))
            udhcpc_stop_pcie(profile);
       else
        udhcpc_stop(profile);
    }
}

static int check_address(PROFILE_T *now_profile,int ipfamily)
{
    PROFILE_T new_profile_v;
    PROFILE_T *new_profile = &new_profile_v;
    static int time_out_count = 0;
    int ret = 0;
    #define QMITHREADSENDQMITIMEOUT 110

    memcpy(new_profile, now_profile, sizeof(PROFILE_T));
    if(ipfamily == IpFamilyV4)
    {
        ret = requestGetIPAddress(new_profile, 0x04);
        if (ret == 0)
        {
            time_out_count = 0;
            if (new_profile->ipv4.Address != now_profile->ipv4.Address || debug_qmi)
            {
                unsigned char *l = (unsigned char *)&now_profile->ipv4.Address;
                unsigned char *r = (unsigned char *)&new_profile->ipv4.Address;
                dbg_time("localIP: %d.%d.%d.%d VS remoteIP: %d.%d.%d.%d", l[3],
                     l[2], l[1], l[0], r[3], r[2], r[1], r[0]);
            }
            return (new_profile->ipv4.Address == now_profile->ipv4.Address);
        }
        else if (ret == QMITHREADSENDQMITIMEOUT)
        {
            time_out_count++;
            dbg_time("%s %d time_out_count %d\n", __func__, __LINE__, time_out_count);
            if (time_out_count >= 3)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
    }
    if(ipfamily == IpFamilyV6)
    {
        return 1;
    }

    return 0;
}

static void main_send_event_to_qmidevice(int triger_event)
{
    write(qmidevice_control_fd[0], &triger_event, sizeof(triger_event));
}

static void send_signo_to_main(int signo)
{
    write(signal_control_fd[0], &signo, sizeof(signo));
    dbg_time("write signo: %d to signal_control_fd",signo);
}

void qmidevice_send_event_to_main(int triger_event)
{
    write(qmidevice_control_fd[1], &triger_event, sizeof(triger_event));
    dbg_time("write triger_event: %d to qmidevice_control_fd",triger_event);
}

static int ls_dir(const char *dir,
          int (*match)(const char *dir, const char *file, void *argv[]),
          void *argv[])
{
    DIR *pDir;
    struct dirent *ent = NULL;
    int match_times = 0;

    pDir = opendir(dir);
    if (pDir == NULL) {
    dbg_time("Cannot open directory: %s, errno: %d (%s)", dir, errno,
         strerror(errno));
    return 0;
    }

    while ((ent = readdir(pDir)) != NULL) {
        match_times += match(dir, ent->d_name, argv);
    }
    closedir(pDir);

    return match_times;
}

static int is_same_linkfile(const char *dir, const char *file, void *argv[])
{
    const char *qmichannel = (const char *)argv[1];
    char linkname[MAX_PATH];
    char filename[MAX_PATH];
    int linksize;

    snprintf(linkname, MAX_PATH, "%s/%s", dir, file);
    linksize = readlink(linkname, filename, MAX_PATH);
    if (linksize <= 0)
    return 0;

    filename[linksize] = 0;
    if (strcmp(filename, qmichannel))
    return 0;

    dbg_time("%s -> %s", linkname, filename);
    return 1;
}

static int is_brother_process(const char *dir, const char *file, void *argv[])
{
    // const char *myself = (const char *)argv[0];
    char linkname[MAX_PATH];
    char filename[MAX_PATH];
    int linksize;
    int i = 0, kill_timeout = 15;
    pid_t pid;

    // dbg_time("%s", file);
    while (file[i]) {
    if (!isdigit(file[i]))
        break;
    i++;
    }

    if (file[i]) {
    // dbg_time("%s not digit", file);
    return 0;
    }

    snprintf(linkname, MAX_PATH, "%s/%s/exe", dir, file);
    linksize = readlink(linkname, filename, MAX_PATH);
    if (linksize <= 0)
    return 0;

    filename[linksize] = 0;

    pid = atoi(file);
    if (pid >= getpid())
    return 0;

    snprintf(linkname, MAX_PATH, "%s/%s/fd", dir, file);
    if (!ls_dir(linkname, is_same_linkfile, argv))
    return 0;

    dbg_time("%s/%s/exe -> %s", dir, file, filename);
    while (kill_timeout-- && !kill(pid, 0)) {
    kill(pid, SIGTERM);
    sleep(1);
    }
    if (!kill(pid, 0)) {
    dbg_time("force kill %s/%s/exe -> %s", dir, file, filename);
    kill(pid, SIGKILL);
    sleep(1);
    }

    return 1;
}

static int kill_brothers(const char *qmichannel)
{
    char myself[MAX_PATH];
    int filenamesize;
    void *argv[2] = {myself, (void *)qmichannel};

    filenamesize = readlink("/proc/self/exe", myself, MAX_PATH);
    if (filenamesize <= 0)
    return 0;
    myself[filenamesize] = 0;

    if (ls_dir("/proc", is_brother_process, argv))
    sleep(1);

    return 0;
}

static void fibo_sigaction(int signo)
{
    if (SIGCHLD == signo)
    waitpid(-1, NULL, WNOHANG);
    else if (SIGALRM == signo)
    send_signo_to_main(SIGUSR1);
    else {
    if (SIGTERM == signo || SIGHUP == signo || SIGINT == signo)
        main_loop = 0;
    send_signo_to_main(signo);
    main_send_event_to_qmidevice(signo);  // main may be wating qmi response
    }
}

static int usage(const char *progname)
{
    dbg_time("Usage: %s [options]", progname);
    dbg_time(
    "-s [apn [user password auth]]          Set "
    "apn/user/password/auth get from your network provider");
    dbg_time(
    "-p pincode                             Verify sim card pin if "
    "sim card is locked");
    dbg_time(
    "-f logfilename                         Save log message of this "
    "program to file");
    dbg_time(
    "-i interface                           Specify network "
    "interface(default auto-detect)");
    dbg_time("-4                                     IPv4 protocol");
    dbg_time("-6                                     IPv6 protocol");

//2021-02-25 willa.liu@fibocom.com changed begin for support eipd SN-20210129001
    dbg_time("-g                                     IPv6 private gateway");
//2021-02-25 willa.liu@fibocom.com changed begin for support eipd SN-20210129001

//2020-12-29 willa.liu@fibocom.com changed begin for support mantis 0066901
    //dbg_time(
    //"-P profile index                       Specify profile index "
    //"when start network data connection(default 1).");
//2020-12-29 willa.liu@fibocom.com changed end for support mantis 0066901

    dbg_time(
    "-m muxID                               Specify muxid when set "
    "multi-pdn data connection.");
    dbg_time(
    "-n channelID                           Specify channelID when "
    "set multi-pdn data connection(default 1).");
    dbg_time(
    "-N Number of channel                   Specify total channels "
    "when set multi-pdn data connection(default 1).");

//2020-12-23 willa.liu@fibocom.com changed begin for support mantis 0065286
    dbg_time(
    "-k number                             kill specified process ");
//2020-12-23 willa.liu@fibocom.com changed end for support mantis 0065286

    dbg_time("[Examples]");
    dbg_time("Example 1: %s ", progname);
    dbg_time("Example 2: %s -s 3gnet ", progname);
    dbg_time("Example 3: %s -s 3gnet carl 1234 0 -p 1234 -f gobinet_log.txt",
         progname);
    return 0;
}

static int charsplit(const char *src, char *desc, int n, const char *splitStr)
{
    char *p;
    char *p1;
    int len;

    len = strlen(splitStr);
    p = strstr(src, splitStr);
    if (p == NULL)
    return -1;
    p1 = strstr(p, "\n");
    if (p1 == NULL)
    return -1;
    memset(desc, 0, n);
    memcpy(desc, p + len, p1 - p - len);

    return 0;
}

static int get_dev_major_minor(char *path, int *major, int *minor)
{
    int fd = -1;
    char desc[128] = {0};
    char devmajor[64], devminor[64];
    int n = 0;
    if (access(path, R_OK | W_OK)) {
    return 1;
    }
    if ((fd = open(path, O_RDWR)) < 0) {
    return 1;
    }
    n = read(fd, desc, sizeof(desc));
    if (n == sizeof(desc)) {
    dbg_time("may be overflow");
    }
    close(fd);
    if (charsplit(desc, devmajor, 64, "MAJOR=") == -1 ||
    charsplit(desc, devminor, 64, "MINOR=") == -1) {
    return 2;
    }
    *major = atoi(devmajor);
    *minor = atoi(devminor);
    return 0;
}
#ifdef GHT_FEATURE_PCIE_AUTO
//begin modified by zhangkaibo add PCIe auto detect feature on x55/x24 platform. 20200605
static int qmipciedevice_detect(char **pp_qmichannel, char **pp_usbnet_adapter)
{
    struct dirent *ent = NULL;
    DIR *pDir;
    char dir[255] = "/sys/bus/pci/devices";

    struct {
    char subdir[255 * 2];
    char subdir2[255 * 3 + 16];
    char qmifile[255 * 2];
    } * pl;
    pl = (typeof(pl))malloc(sizeof(*pl));
    memset(pl, 0x00, sizeof(*pl));

    pDir = opendir(dir);
    if (pDir) {
    while ((ent = readdir(pDir)) != NULL) {

        struct dirent *subent = NULL;
        DIR *psubDir;
        char idVendor[4 + 1] = {0};
        char idDevice[4 + 1] = {0};
        char tempNetpath[256] = {0};
        int fd = 0;
        char netcard[32] = "\0";

        memset(pl, 0x00, sizeof(*pl));
        snprintf(pl->subdir, sizeof(pl->subdir), "%s/%s/vendor", dir,
             ent->d_name);
        fd = open(pl->subdir, O_RDONLY);
        if (fd > 0) {
        read(fd, idVendor, 6);
        close(fd);
        }

        snprintf(pl->subdir, sizeof(pl->subdir), "%s/%s/device", dir,
             ent->d_name);
        fd = open(pl->subdir, O_RDONLY);
        if (fd > 0) {
        read(fd, idDevice, 6);
        close(fd);
        }

        if (!strncasecmp(idVendor, "0x17cb", 6))
        ;
        else
        continue;

        dbg_time("Find %s/%s vendor=%s device=%s", dir, ent->d_name,
             idVendor,idDevice);
        //find pcie netdevicename
        snprintf(tempNetpath,256,"%s%s_IP_HW0", idDevice+2 , ent->d_name+2);
        snprintf(pl->subdir, sizeof(pl->subdir), "%s/%s/%s/net", dir,
             ent->d_name,tempNetpath);
        psubDir = opendir(pl->subdir);
        if (psubDir == NULL) {
        dbg_time("Cannot open directory: %s, errno: %d (%s)",
             pl->subdir, errno, strerror(errno));
        continue;
        }

        while ((subent = readdir(psubDir)) != NULL) {
        if (subent->d_name[0] == '.')
            continue;
        dbg_time("Find pcienet_adapter = %s", subent->d_name);
        strcpy(netcard, subent->d_name);
        break;
        }

        closedir(psubDir);

        if (netcard[0]) {
        } else {
        continue;
        }

        *pp_usbnet_adapter = strdup(netcard);
        break;
        }
        closedir(pDir);
    }
    //find pcie qmi control name
     if ((pDir = opendir("/dev")) == NULL) {
    dbg_time("Cannot open directory: %s, errno:%d (%s)", "/dev", errno,
         strerror(errno));
    return -ENODEV;
    }

    while ((ent = readdir(pDir)) != NULL) {
    if ( strncmp(ent->d_name, "mhi_QMI", strlen("mhi_QMI")) == 0 ) {
        *pp_qmichannel = (char *)malloc(32);
        sprintf(*pp_qmichannel, "/dev/%s", ent->d_name);
        dbg_time("Find qmichannel = %s", *pp_qmichannel);
        break;
        }
    }
    closedir(pDir);
    free(pl);

    return (*pp_qmichannel && *pp_usbnet_adapter);
}
//end modified by zhangkaibo add PCIe auto detect feature on x55/x24 platform. 20200605
#endif
static int qmidevice_detect(char **pp_qmichannel, char **pp_usbnet_adapter,PROFILE_T *profile)
{
    struct dirent *ent = NULL;
    DIR *pDir;
    char dir[255] = "/sys/bus/usb/devices";
    int major = 0, minor = 0;
    int indexid = 4;
    struct {
    char subdir[255 * 3];
    char subdir2[255 * 4 + 16];
    char qmifile[255 * 2];
    } * pl;
#ifdef GHT_FEATURE_PCIE_AUTO
//begin modified by zhangkaibo add PCIe auto detect feature on x55/x24 platform. 20200605
    if (qmipciedevice_detect(pp_qmichannel, pp_usbnet_adapter))
    {
        return (*pp_qmichannel && *pp_usbnet_adapter);
    }
//end modified by zhangkaibo add PCIe auto detect feature on x55/x24 platform. 20200605
#endif
    pl = (typeof(pl))malloc(sizeof(*pl));
    memset(pl, 0x00, sizeof(*pl));

    pDir = opendir(dir);
    if (pDir) {
    while ((ent = readdir(pDir)) != NULL) {
        struct dirent *subent = NULL;
        DIR *psubDir;
        char idVendor[4 + 1] = {0};
        char idProduct[4 + 1] = {0};
        int fd = 0;
        char netcard[32] = "\0";

        memset(pl, 0x00, sizeof(*pl));

        snprintf(pl->subdir, sizeof(pl->subdir), "%s/%s/idVendor", dir,
             ent->d_name);
        fd = open(pl->subdir, O_RDONLY);
        if (fd > 0) {
        read(fd, idVendor, 4);
        close(fd);
        }

        snprintf(pl->subdir, sizeof(pl->subdir), "%s/%s/idProduct", dir,
             ent->d_name);
        fd = open(pl->subdir, O_RDONLY);
        if (fd > 0) {
        read(fd, idProduct, 4);
        close(fd);
        }

        if (!strncasecmp(idVendor, "05c6", 4) ||
        !strncasecmp(idVendor, "2cb7", 4))
        ;
        else
        continue;

        dbg_time("Find %s/%s idVendor=%s idProduct=%s", dir, ent->d_name,
             idVendor, idProduct);
//2021-02-08 zhangkaibo@fibocom.com changed start for mantis 0070613
        if(!strncasecmp(idProduct, "0109", 4))
        {
            indexid = 2;
            profile->interfacenum = 2;
        }
//2021-09-09 willa.liu@fibocom.com changed start for mantis 0086219
        else if(!strncasecmp(idProduct, "0113", 4))
        {
            indexid = 0;
            profile->interfacenum = 0;
        }
//2021-09-09 willa.liu@fibocom.com changed end for mantis 0086219
        else
        {
            profile->interfacenum = 4;
        }
        snprintf(pl->subdir, sizeof(pl->subdir), "%s/%s:1.%d/net", dir,
             ent->d_name,indexid);
        psubDir = opendir(pl->subdir);
        if (psubDir == NULL) {
        dbg_time("Cannot open directory: %s, errno: %d (%s)",
             pl->subdir, errno, strerror(errno));
        continue;
        }

        while ((subent = readdir(psubDir)) != NULL) {
        if (subent->d_name[0] == '.')
            continue;
        dbg_time("Find %s/%s", pl->subdir, subent->d_name);
        dbg_time("Find usbnet_adapter = %s", subent->d_name);
        strcpy(netcard, subent->d_name);
        break;
        }

        closedir(psubDir);

        if (netcard[0]) {
        } else {
        continue;
        }

        if (*pp_usbnet_adapter && strcmp(*pp_usbnet_adapter, netcard))
        continue;

        snprintf(pl->subdir, sizeof(pl->subdir), "%s/%s:1.%d/GobiQMI", dir,
             ent->d_name,indexid);
        if (access(pl->subdir, R_OK)) {
        snprintf(pl->subdir, sizeof(pl->subdir), "%s/%s:1.%d/usbmisc",
             dir, ent->d_name,indexid);
        if (access(pl->subdir, R_OK)) {
            snprintf(pl->subdir, sizeof(pl->subdir), "%s/%s:1.%d/usb",
                 dir, ent->d_name,indexid);
            if (access(pl->subdir, R_OK)) {
            dbg_time("no GobiQMI/usbmic/usb found in %s/%s:1.%d",
                 dir, ent->d_name,indexid);
            continue;
            }
        }
        }
        
//2021-02-08 zhangkaibo@fibocom.com changed end for mantis 0070613

        psubDir = opendir(pl->subdir);
        if (pDir == NULL) {
        dbg_time("Cannot open directory: %s, errno: %d (%s)", dir,
             errno, strerror(errno));
        continue;
        }

        while ((subent = readdir(psubDir)) != NULL) {
        if (subent->d_name[0] == '.')
            continue;
        dbg_time("Find %s/%s", pl->subdir, subent->d_name);
        dbg_time("Find qmichannel = /dev/%s", subent->d_name);
        snprintf(pl->qmifile, sizeof(pl->qmifile), "/dev/%s",
             subent->d_name);

        // get major minor
        snprintf(pl->subdir2, sizeof(pl->subdir2), "%s/%s/uevent",
             pl->subdir, subent->d_name);
        if (!get_dev_major_minor(pl->subdir2, &major, &minor)) {
            // dbg_time("%s major = %d, minor = %d\n",pl->qmifile,
            // major, minor);
        } else {
            dbg_time("get %s major and minor failed\n", pl->qmifile);
        }
        // get major minor

        if ((fd = open(pl->qmifile, R_OK)) < 0) {
            dbg_time("%s open failed", pl->qmifile);
            dbg_time("please mknod %s c %d %d", pl->qmifile, major,
                 minor);
        } else {
            close(fd);
        }
        break;
        }

        closedir(psubDir);

        if (netcard[0] && pl->qmifile[0]) {
        *pp_qmichannel = strdup(pl->qmifile);
        *pp_usbnet_adapter = strdup(netcard);
        closedir(pDir);
        return 1;
        }
    }

    closedir(pDir);
    }

    if ((pDir = opendir("/dev")) == NULL) {
    dbg_time("Cannot open directory: %s, errno:%d (%s)", "/dev", errno,
         strerror(errno));
    return -ENODEV;
    }

    while ((ent = readdir(pDir)) != NULL) {
    if ((strncmp(ent->d_name, "cdc-wdm", strlen("cdc-wdm")) == 0) ||
        (strncmp(ent->d_name, "qcqmi", strlen("qcqmi")) == 0)) {
        char net_path[255*2];

        *pp_qmichannel = (char *)malloc(32);
        sprintf(*pp_qmichannel, "/dev/%s", ent->d_name);
        dbg_time("Find qmichannel = %s", *pp_qmichannel);

        if (strncmp(ent->d_name, "cdc-wdm", strlen("cdc-wdm")) == 0)
        sprintf(net_path, "/sys/class/net/wwan%s",
            &ent->d_name[strlen("cdc-wdm")]);
        else {
        sprintf(net_path, "/sys/class/net/usb%s",
            &ent->d_name[strlen("qcqmi")]);

        if (access(net_path, R_OK) && errno == ENOENT)
            sprintf(net_path, "/sys/class/net/eth%s",
                &ent->d_name[strlen("qcqmi")]);
        }

        if (access(net_path, R_OK) == 0) {
        if (*pp_usbnet_adapter &&
            strcmp(*pp_usbnet_adapter,
               (net_path + strlen("/sys/class/net/")))) {
            free(*pp_qmichannel);
            *pp_qmichannel = NULL;
            continue;
        }
        *pp_usbnet_adapter =
            strdup(net_path + strlen("/sys/class/net/"));
        dbg_time("Find usbnet_adapter = %s", *pp_usbnet_adapter);
        break;
        } else {
        dbg_time("Failed to access %s, errno:%d (%s)", net_path, errno,
             strerror(errno));
        free(*pp_qmichannel);
        *pp_qmichannel = NULL;
        }
    }
    }
    closedir(pDir);
    free(pl);

    return (*pp_qmichannel && *pp_usbnet_adapter);
}
//begin modified by zhangkaibo limit input parameters. mantis 0048965  20200605
int IsInt(char* str)
{
    int len;
    len = strlen(str);
    int i=0;
    for(; i < len ; i++)
    {
        if(!(isdigit(str[i])))
            return 0;
    }
    return 1;
}
//end modified by zhangkaibo limit input parameters. mantis 0048965  20200605

//2020-12-23 willa.liu@fibocom.com changed begin for support mantis 0065286
int  rdwr_init(int clienfd, char * msg)
{
    char buf[512];
    int  rv = 0;
    memset(buf, 0, 512);
    while(1)
    {
    if ((rv = read(clienfd, buf, 512)) < 0)
        {
            dbg_time("Read by socket[%d] error:%s", clienfd, strerror(errno));
            goto STOP;
        }
        else if (0 == rv)
        {
            goto STOP;
        }
        else if (rv > 0)
        {
            dbg_time("Read %d bytes data from client, there are :[%s]", rv, buf);
        }
        if ((rv =write(clienfd, msg, strlen(msg))) < 0)
        {
            dbg_time("write to cilent by socket[%d] error:%s",clienfd, strerror(errno));
            goto STOP;
        }
    }
STOP:
    dbg_time("The socket[%d] will exit!", clienfd);
    return 0;
}

void *thread_socket_server(void *args)
{
    int on = 1;
    int rv = 0;
    struct sockaddr_un serv_addr;
    struct sockaddr_un cli_addr;
    socklen_t len = sizeof(serv_addr);
    sprintf(fibo_dial_filepath,"/tmp/fibocom_dial_%d",*(int*)args);
    char* msg = "receive quit request\n";
    if ((socket_server_fd = socket(AF_UNIX,SOCK_STREAM,0))< 0)
    {
        dbg_time("Socket error:%s\a\n", strerror(errno));
        return ;
    }
    dbg_time("socket[%d] successfuly!", socket_server_fd);
    if (!access(fibo_dial_filepath, F_OK))
    {
        unlink(fibo_dial_filepath);
    }
    setsockopt(socket_server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, fibo_dial_filepath, sizeof(serv_addr.sun_path) - 1);
    if ((rv = bind(socket_server_fd, (struct sockaddr *)&serv_addr, len)) < 0)
    {
        dbg_time("Bind error %s", strerror(errno));
        goto EXIT;
    }
    if ((rv = listen(socket_server_fd, 13)) < 0)
    {
        dbg_time("Listen error:%s", strerror(errno));
        goto EXIT;
    }
    dbg_time("Waiting client to connect.....");
    sem_post(&sem);
    while(1)
    {
        if ((cli_accept_fd = accept(socket_server_fd, (struct sockaddr *)&cli_addr, &len))< 0)
        {
            dbg_time("accept error:%s", strerror(errno));
            goto EXIT;
        }
        dbg_time("client to connected");
        rv = rdwr_init(cli_accept_fd, msg);
        goto EXIT;
    }
EXIT:
    unlink(fibo_dial_filepath);
    close(socket_server_fd );
    close(cli_accept_fd );
    send_signo_to_main(SIGTERM);
    return ((void *)0);

}

//2020-12-29 willa.liu@fibocom.com changed begin for support mantis 0066901
int startSocketserver(int pdpindex)
{
    pthread_t ntid;
    //sem_init(&sem, 0, 2);
    sem_init(&sem, 0, 0);
    if (pthread_create(&ntid, 0, thread_socket_server,
               (void *)(&pdpindex)) != 0) {
    dbg_time("%s Failed to create QMIThread: %d (%s)", __func__, errno,
         strerror(errno));
    }
    sem_wait(&sem);
    sem_destroy(&sem);
    return 0;
}
//2020-12-29 willa.liu@fibocom.com changed begin for support mantis 0066901

int startSocketclient(int pdpindex)
{
    int socket_client_fd;
    int rv = -1;
    struct sockaddr_un servaddr;
    char fibo_dial_filepath[100] = {0};
    sprintf(fibo_dial_filepath,"/tmp/fibocom_dial_%d",pdpindex);
    dbg_time("enter startSocketclient %s.",fibo_dial_filepath);
    char* message = "fibocom-dial quit request";
    char buf[1024];

    if(access(fibo_dial_filepath, F_OK) != 0)
    {
       return -1;
    }
    socket_client_fd = socket(AF_UNIX,SOCK_STREAM,0);
    if(socket_client_fd < 0)
    {
        dbg_time("Cearte socket failure :%s\a",strerror(errno));
        return -1;
    }
    dbg_time("Create socket [%d] sucessfully.",socket_client_fd);
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strncpy(servaddr.sun_path, fibo_dial_filepath, sizeof(servaddr.sun_path) - 1);
    rv = connect(socket_client_fd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    if(rv <0)
    {
        dbg_time("connect to server [%s] failure :%s\a",fibo_dial_filepath,strerror(errno));
        return -2;
    }
    dbg_time("connect to server [%s] successfully!",fibo_dial_filepath);
    rv = write(socket_client_fd,message,strlen(message));
    if(rv <0)
    {
        dbg_time("Write to server failure by socket[%d] failure: %s",socket_client_fd,strerror(errno));
        return -3;
    }
    else
    {
        dbg_time("Write [%s] to server by socket[%d] successfuly!", message, socket_client_fd);
    }
    memset(buf,0,sizeof(buf));
    rv = read(socket_client_fd,buf,1024);
    if(rv < 0)
    {
        dbg_time("Read to server socket[%d] failure: %s",socket_client_fd,strerror(errno));
        return -4;
    }
    else if(rv == 0)
    {
        dbg_time("socket [%d] get disconnected",socket_client_fd);
        return -4;
    }
    else if(rv >0)
    {
        dbg_time("Read [%d] bytes data from Server: [%s]",rv,buf);
    }
    close(socket_client_fd);
    unlink(fibo_dial_filepath);

    return 0;
}
//2020-12-23 willa.liu@fibocom.com changed begin for support mantis 0065286

int main(int argc, char *argv[])
{
    int triger_event = 0;
    int opt = 1;
    int signo;
//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
    int sim_number = 0;
//begin modified by zhangkaibo limit input parameters. mantis 0048965  20200605
    char argvtemp[256] = {0};
//end modified by zhangkaibo limit input parameters. mantis 0048965  20200605

//2020-12-23 willa.liu@fibocom.com changed begin for support mantis 0065286
    int killnumber = -1;
//2020-12-23 willa.liu@fibocom.com changed end for support mantis 0065286

#ifdef CONFIG_SIM
    SIM_Select SIMNumber;
//2021-03-24 willa.liu@fibocom.com changed end for support mantis 0071817
    SIM_Status SIMStatus;
#endif
    UCHAR PSAttachedState;
    UCHAR IPv4ConnectionStatus = 0xff;    // unknow state
    UCHAR IPV6ConnectionStatus = 0xff;    // unknow state
    int qmierr = 0;
    char *save_usbnet_adapter = NULL;
    PROFILE_T profile;
#ifdef CONFIG_RESET_RADIO
    struct timeval resetRadioTime = {0};
    struct timeval FailDailRadioTime = {0};
    struct timeval nowTime;
    gettimeofday(&resetRadioTime, (struct timezone *)NULL);
#endif

    dbg_time("Fibocom-dial_Linux_Tool_V%s", VERSION_STRING());
    memset(&profile, 0x00, sizeof(profile));
    profile.pdp = CONFIG_DEFAULT_PDP;
    profile.pdpindex = CONFIG_DEFAULT_PDPINDEX;
    profile.pdpnum = 1;

    if (!strcmp(argv[argc - 1], "&"))
    argc--;

    opt = 1;
    while (opt < argc) {
    if (argv[opt][0] != '-')
        return usage(argv[0]);

    switch (argv[opt++][1]) {
#define has_more_argv() ((opt < argc) && (argv[opt][0] != '-'))
    case 's':
        profile.apn = profile.user = profile.password = "";
        if (has_more_argv())
        profile.apn = argv[opt++];
        if (has_more_argv())
        profile.user = argv[opt++];
        if (has_more_argv()) {
        profile.password = argv[opt++];
        if (profile.password && profile.password[0])
            profile.auth = 2;  // default chap, customers may miss auth
        }
        if (has_more_argv())
//begin modified by zhangkaibo limit input parameters. mantis 0048965  20200605
        {
            memset(argvtemp,0,256);
            strcpy(argvtemp,argv[opt++]);
            if(strlen(argvtemp) > 1 ||  !IsInt(argvtemp))
            {
                dbg_time("Invalid parament for \"-s\"");
                return -1;
            }
            profile.auth = argvtemp[0] - '0';
//begin modified by zhangkaibo limit input parameters. mantis 0077949  20200513
            if(profile.auth < 0 || profile.auth > 3)
            {
                dbg_time("auth must be 0~3");
//end modified by zhangkaibo limit input parameters. mantis 0077949  20200513
                return -1;
            }
        }
//end modified by zhangkaibo limit input parameters. mantis 0048965  20200605
        break;

    case 'm':
        if (has_more_argv())
        {
                if (is_pcie_dial())
                    profile.muxid = argv[opt++][0] - '0';
                else
                {
                    memset(argvtemp,0,256);
                    strcpy(argvtemp,argv[opt++]);
                    if(strlen(argvtemp) > 1 ||  !IsInt(argvtemp))
                    {
                        dbg_time("Invalid parament for \"-P\"");
                        return -1;
                    }
                profile.pdpindex = argvtemp[0] - '0';
                if(profile.pdpindex < 1 || profile.pdpindex > 10)
                {
                            dbg_time("pdpindex must be 1~10");
                        return -1;
                }
                }
        }
        break;

    case 'p':
        if (has_more_argv())
        profile.pincode = argv[opt++];
        break;
#if 0
    case 'P':
        if (has_more_argv())
//begin modified by zhangkaibo limit input parameters. mantis 0048965  20200605
        {
            memset(argvtemp,0,256);
            strcpy(argvtemp,argv[opt++]);
            if(strlen(argvtemp) > 1 ||  !IsInt(argvtemp))
            {
                dbg_time("Invalid parament for \"-P\"");
                return -1;
            }
            profile.pdpindex = argvtemp[0] - '0';
            if(profile.pdpindex < 1 || profile.pdpindex > 10)
            {
                dbg_time("pdpindex must be 1~10");
                return -1;
            }
        }
//end modified by zhangkaibo limit input parameters. mantis 0048965  20200605
        break;
#endif
    case 'n':
        if (has_more_argv())
//begin modified by zhangkaibo limit input parameters. mantis 0048965  20200605
        {
            memset(argvtemp,0,256);
            strcpy(argvtemp,argv[opt++]);
            if(strlen(argvtemp) > 1 ||  !IsInt(argvtemp))
            {
                dbg_time("Invalid parament for \"-n\"");
                return -1;
            }
            profile.pdp = argvtemp[0] - '0';
            if(profile.pdp < 1 || profile.pdp > 8)
            {
                dbg_time("pdpindex must be 1~8");
                return -1;
            }

            if (is_pcie_dial())
                profile.pdpindex = profile.pdp;
        }
        break;
//end modified by zhangkaibo limit input parameters. mantis 0048965  20200605
    case 'N':
        if (has_more_argv())
//begin modified by zhangkaibo limit input parameters. mantis 0048965  20200605
        {
            memset(argvtemp,0,256);
            strcpy(argvtemp,argv[opt++]);
            if(strlen(argvtemp) > 1 ||  !IsInt(argvtemp))
            {
                dbg_time("Invalid parament for \"-N\"");
                return -1;
            }
            profile.pdpnum = argvtemp[0] - '0';
            if(profile.pdpnum < 0 || profile.pdpnum > 8)
            {
                dbg_time("pdpnum must be 0~8");
                return -1;
            }
        }
//end modified by zhangkaibo limit input parameters. mantis 0048965  20200605
        break;

    case 'f':
        if (has_more_argv()) {
        const char *filename = argv[opt++];
        logfilefp = fopen(filename, "a+");
        if (!logfilefp) {
            dbg_time("Fail to open %s, errno: %d(%s)", filename, errno,
                 strerror(errno));
        }
        }
        break;

    case 'i':
        if (has_more_argv())
        profile.usbnet_adapter = save_usbnet_adapter = argv[opt++];
        break;

    case 'v':
        debug_qmi = 1;
        break;

    case 'l':
        if (has_more_argv()) {
            profile.replication_factor = atoi(argv[opt++]);
            if (profile.replication_factor > 0)
                profile.loopback_state = 1;
        }
        else
         main_loop = 1;

        break;

    case '4':
        profile.ipv4_flag = 1;
        break;

    case '6':
        profile.ipv6_flag = 1;
        break;

//2021-02-25 willa.liu@fibocom.com changed begin for support eipd SN-20210129001
    case 'g':
        profile.ipv6_prigateway_flag = 1;
        break;
//2021-02-25 willa.liu@fibocom.com changed end for support eipd SN-20210129001

    case 'd':
        if (has_more_argv()) {
        profile.qmichannel = argv[opt++];
        if (qmidev_is_pciemhi(profile.qmichannel))
            profile.usbnet_adapter = "pcie_mhi0";
        }
        break;

        case 'U':
           query_pcie_mode = 1;
            break;

//2020-12-23 willa.liu@fibocom.com changed begin for support mantis 0065286
    case 'k':
        if (has_more_argv())
        {
            memset(argvtemp,0,256);
            strcpy(argvtemp,argv[opt++]);
            if(strlen(argvtemp) > 1 ||  !IsInt(argvtemp))
            {
                dbg_time("Invalid parament for \"-n\"");
                return -1;
            }
            killnumber = argvtemp[0] - '0';
//2021-02-01 willa.liu@fibocom.com changed begin for support mantis 0069837
            //if(killnumber < 1 || killnumber > 5)
            if(killnumber < 1 || killnumber > 8)
            {
                dbg_time("pdpindex must be 1~8");
                return -1;
            }
//2021-02-01 willa.liu@fibocom.com changed end for support mantis 0069837
        }
        break;
//2020-12-23 willa.liu@fibocom.com changed end for support mantis 0065286

    default:
        return usage(argv[0]);
        break;
    }
    }

    if (profile.ipv4_flag != 1 &&
    profile.ipv6_flag != 1) {  // default enable IPv4
    profile.ipv4_flag = 1;
    }

    dbg_time("%s profile[%d] = %s/%s/%s/%d, pincode = %s", argv[0], profile.pdp,
         profile.apn, profile.user, profile.password, profile.auth,
         profile.pincode);
    signal(SIGUSR1, fibo_sigaction);
    signal(SIGUSR2, fibo_sigaction);
    signal(SIGINT, fibo_sigaction);
    signal(SIGTERM, fibo_sigaction);
    signal(SIGHUP, fibo_sigaction);
    signal(SIGCHLD, fibo_sigaction);
    signal(SIGALRM, fibo_sigaction);

    //2020-12-23 willa.liu@fibocom.com changed begin for support mantis 0065286
        if(killnumber == -1)
        {
            startSocketserver(profile.pdp);
        }
        else{
            startSocketclient(killnumber);
            return 0;
        }
    //2020-12-23 willa.liu@fibocom.com changed end for support mantis 0065286

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, signal_control_fd) < 0) {
    dbg_time("%s Faild to create main_control_fd: %d (%s)", __func__, errno,
         strerror(errno));
    return -1;
    }

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, qmidevice_control_fd) < 0) {
    dbg_time("%s Failed to create thread control socket pair: %d (%s)",
         __func__, errno, strerror(errno));
    return 0;
    }

// sudo apt-get install udhcpc
// sudo apt-get remove ModemManager
__main_loop:
    while (!profile.qmichannel) {
//2021-02-08 zhangkaibo@fibocom.com changed start for mantis 0070613
    if (qmidevice_detect(&profile.qmichannel, &profile.usbnet_adapter,&profile))
//2021-02-08 zhangkaibo@fibocom.com changed end for mantis 0070613
    {
           if (query_pcie_mode)
                get_pcie_mode();
        break;
    }
    if (main_loop) {
        int wait_for_device = 3000;
        dbg_time("Wait for fibo modules connect");
        while (wait_for_device && main_loop) {
        wait_for_device -= 100;
        usleep(100 * 1000);
        }
        continue;
    }
    dbg_time(
        "Cannot find qmichannel(%s) usbnet_adapter(%s) for fibo modules",
        profile.qmichannel, profile.usbnet_adapter);
    return -ENODEV;
    }

    if (access(profile.qmichannel, R_OK | W_OK)) {
    dbg_time("Fail to access %s, errno: %d (%s)", profile.qmichannel, errno,
         strerror(errno));
    return errno;
    }
    dbg_time(
        "qmichannel(%s) usbnet_adapter(%s) ",
        profile.qmichannel, profile.usbnet_adapter);

    if (qmidev_is_gobinet(profile.qmichannel)) {
    profile.qmi_ops = &gobi_qmidev_ops;
    dbg_time("usb rmnet mode");
    } else {
    profile.qmi_ops = &qmiwwan_qmidev_ops;
    dbg_time("pcie mode");
    }
    qmidev_send = profile.qmi_ops->send;
    fibo_qmap_mode_detect(&profile);
    if (profile.pdpnum > 1 && profile.pdpnum != profile.qmap_mode &&  !qmidev_is_pciemhi(profile.qmichannel)) {
    if (profile.qmap_mode > 1) {
           dbg_time("qmap_mode=%d,profile.pdpnum %d",profile.qmap_mode,profile.pdpnum);
        kill_brothers(profile.qmichannel);
    }
    fibo_qmap_mode_set(&profile);
    }
    fibo_qmap_mode_detect(&profile);
//begin modified by zhangkaibo limit input parameters. mantis 0048965  20200605
    if(profile.qmap_mode < profile.pdp && profile.qmap_mode > 0  &&  !qmidev_is_pciemhi(profile.qmichannel))
    {
        dbg_time("invalid pdpindex");
        return -1;
    }
//end modified by zhangkaibo limit input parameters. mantis 0048965  20200605
    if (profile.qmap_mode == 0 || profile.qmap_mode == 1)
    {
        dbg_time("qmap_mode=%d",profile.qmap_mode);
        kill_brothers(profile.qmichannel);
    }
    if (pthread_create(&gQmiThreadID, 0, profile.qmi_ops->read,
               (void *)&profile) != 0) {
    dbg_time("%s Failed to create QMIThread: %d (%s)", __func__, errno,
         strerror(errno));
    return 0;
    }

    if ((read(qmidevice_control_fd[0], &triger_event, sizeof(triger_event)) !=
     sizeof(triger_event)) ||
    (triger_event != RIL_INDICATE_DEVICE_CONNECTED)) {
    dbg_time("%s Failed to init QMIThread: %d (%s)", __func__, errno,
         strerror(errno));
    return 0;
    }

    if (profile.qmi_ops->init && profile.qmi_ops->init(&profile)) {
    dbg_time("%s Failed to qmi init: %d (%s)", __func__, errno,
         strerror(errno));
    return 0;
    }

#ifdef CONFIG_VERSION
    requestBaseBandVersion(NULL);
#endif
    requestSetEthMode(&profile);

#ifdef CONFIG_SIM
//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
    sim_number = requestGetSIMCardNumber(&profile);
    if(sim_number == 1){
        requestSimBindSubscription_NAS_WMS();
        requestSimBindSubscription_WDS_DMS_QOS();
        }
	//qmierr = requestGetSIMStatus(&SIMStatus);
    qmierr = requestGetSIMStatus(&SIMStatus, sim_number);
    while (qmierr == QMI_ERR_OP_DEVICE_UNSUPPORTED) {
    sleep(1);
	//qmierr = requestGetSIMStatus(&SIMStatus);
    qmierr = requestGetSIMStatus(&SIMStatus, sim_number);
//2021-03-24 willa.liu@fibocom.com changed begin for support mantis 0071817
    }
    if ((SIMStatus == SIM_PIN) && profile.pincode) {
    requestEnterSimPin(profile.pincode);
    }
#ifdef CONFIG_IMSI_ICCID
    if (SIMStatus == SIM_READY) {
    requestGetICCID();
    requestGetIMSI();
    }
#endif
#endif
#ifdef CONFIG_APN
    if (profile.apn || profile.user || profile.password) {
    requestSetProfile(&profile);
    }
    requestGetProfile(&profile);
#endif
    requestRegistrationState2(&PSAttachedState);
    /*
    if (!requestQueryDataCall(&IPv4ConnectionStatus, IpFamilyV4) &&
       (QWDS_PKT_DATA_CONNECTED == IPv4ConnectionStatus)) usbnet_link_change(1,
       &profile); else usbnet_link_change(0, &profile);
    */
    send_signo_to_main(SIGUSR2);

    while (1) {
    struct pollfd pollfds[] = {{signal_control_fd[1], POLLIN, 0},
                   {qmidevice_control_fd[0], POLLIN, 0}};
    int ne, ret, nevents = sizeof(pollfds) / sizeof(pollfds[0]);

    do {
        ret = poll(pollfds, nevents, 15 * 1000);
    } while ((ret < 0) && (errno == EINTR));

    if (ret == 0) {
        send_signo_to_main(SIGUSR2);
        continue;
    }

    if (ret <= 0) {
        dbg_time("%s poll=%d, errno: %d (%s)", __func__, ret, errno,
             strerror(errno));
        goto __main_quit;
    }

    for (ne = 0; ne < nevents; ne++) {
        int fd = pollfds[ne].fd;
        short revents = pollfds[ne].revents;
        dbg_time("epoll fd = %d, events = 0x%04x", fd, revents);
        if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
        dbg_time("%s poll err/hup", __func__);
        dbg_time("epoll fd = %d, events = 0x%04x", fd, revents);
        main_send_event_to_qmidevice(RIL_REQUEST_QUIT);
        if (revents & POLLHUP)
            goto __main_quit;
        }

        if ((revents & POLLIN) == 0)
        continue;

        if (fd == signal_control_fd[1]) {
        if (read(fd, &signo, sizeof(signo)) == sizeof(signo)) {
            alarm(0);
            dbg_time("get signo: %d ",signo);
            switch (signo) {
            case SIGUSR1:
            {
                if(profile.ipv4_flag)
                {
                    if (QWDS_PKT_DATA_CONNECTED != IPv4ConnectionStatus) {
                        usbnet_link_change(0, &profile);
                        requestRegistrationState2(&PSAttachedState);

//2021-02-05 willa.liu@fibocom.com changed begin for support mantis 0070460
                        //if (PSAttachedState == 1 || profile.loopback_state == 0) {
                        if (PSAttachedState == 1 || profile.loopback_state == 1) {
                        qmierr =
                            requestSetupDataCall(&profile, IpFamilyV4);

                        if ((qmierr > 0) && profile.user &&
                            profile.user[0] && profile.password &&
                            profile.password[0]) {
                            int old_auto = profile.auth;

                            // may be fail because wrong auth mode, try
                            // pap->chap, or chap->pap
                            profile.auth = (profile.auth == 1) ? 2 : 1;
                            qmierr = requestSetupDataCall(&profile,
                                          IpFamilyV4);

                            if (qmierr)
                            profile.auth =
                                old_auto;  // still fail, restore
                                       // old auth moe
                        }
                        if (!qmierr) {
                            requestGetIPAddress(&profile, IpFamilyV4);
                        }
                        if (!qmierr){
                            IPv4ConnectionStatus = QWDS_PKT_DATA_CONNECTED;
                            FailDailRadioTime.tv_sec = 0;
                        }
                        else if(FailDailRadioTime.tv_sec == 0) {
                            gettimeofday(&FailDailRadioTime, (struct timezone *)NULL);
                        }
                    }
                        }
                }
                if(profile.ipv6_flag)
                {
                    if (QWDS_PKT_DATA_CONNECTED != IPV6ConnectionStatus) {
                        if(!profile.ipv4_flag)
                        {
                        usbnet_link_change(0, &profile);
                        requestRegistrationState2(&PSAttachedState);
                        }
                        else
                        {
                        profile.dual_flag = 1;
                        //PSAttachedState = 1; // for dual stack
                        requestRegistrationState2(&PSAttachedState);
                        }
//2021-02-05 willa.liu@fibocom.com changed end for support mantis 0070460

                        if (PSAttachedState == 1) {
                        qmierr =
                            requestSetupDataCall(&profile, IpFamilyV6);
                        if ((g_MobileCountryCode == 460) && (g_MobileNetworkCode == 1 || g_MobileNetworkCode == 6)) {
                            if (qmierr > 0 && profile.dual_flag == 1) {
                                profile.dual_flag = 0;
                                profile.ipv6_flag = 0;
                                dbg_time("%s Dual stack dial IPv6 failed, quit", __func__);
                                break;
                            }
                        }
                        if ((qmierr > 0) && profile.user &&
                            profile.user[0] && profile.password &&
                            profile.password[0]) {
                            int old_auto = profile.auth;

                            // may be fail because wrong auth mode, try
                            // pap->chap, or chap->pap
                            profile.auth = (profile.auth == 1) ? 2 : 1;
                            qmierr = requestSetupDataCall(&profile,
                                          IpFamilyV6);

                            if (qmierr)
                            profile.auth =
                                old_auto;  // still fail, restore
                                       // old auth moe
                        }
                        if (!qmierr) {
                            requestGetIPAddress(&profile, IpFamilyV6);
                        }

                        if (!qmierr){
                            IPV6ConnectionStatus = QWDS_PKT_DATA_CONNECTED;
                            FailDailRadioTime.tv_sec = 0;
                            }
                        else if(FailDailRadioTime.tv_sec == 0) {
                            gettimeofday(&FailDailRadioTime, (struct timezone *)NULL);
                            }
                        }
                    }
                }
#ifdef CONFIG_RESET_RADIO
                if( ((profile.ipv4_flag)&&(IPv4ConnectionStatus != QWDS_PKT_DATA_CONNECTED))||
                    ((profile.ipv6_flag)&&(IPV6ConnectionStatus != QWDS_PKT_DATA_CONNECTED)))
                    {
                        gettimeofday(&nowTime, (struct timezone *)NULL);
                        if( ((abs(nowTime.tv_sec - FailDailRadioTime.tv_sec) > CONFIG_RESET_RADIO))
                            &&(FailDailRadioTime.tv_sec > resetRadioTime.tv_sec)){
                        resetRadioTime = nowTime;
                        // requestSetOperatingMode(0x06); //same as
                        // AT+CFUN=0
                        requestSetOperatingMode(
                            0x01);  // same as AT+CFUN=4
                        requestSetOperatingMode(
                            0x00);  // same as AT+CFUN=1
                        }
                    }
#endif
                alarm(5);  // try to setup data call 5 seconds later
            }
            break;

            case SIGUSR2:
            {

            if (profile.ipv4_flag)
            {
                requestQueryDataCall(&IPv4ConnectionStatus, IpFamilyV4);
                //local ip is different with remote ip
                if (QWDS_PKT_DATA_CONNECTED == IPv4ConnectionStatus && check_address(&profile,IpFamilyV4) == 0) {
                    requestDeactivateDefaultPDP(&profile, IpFamilyV4);
                    IPv4ConnectionStatus = QWDS_PKT_DATA_DISCONNECTED;
                                }
            }
            else
                IPv4ConnectionStatus = QWDS_PKT_DATA_DISCONNECTED;
            if (profile.ipv6_flag)
                requestQueryDataCall(&IPV6ConnectionStatus, IpFamilyV6);
            else
                IPV6ConnectionStatus = QWDS_PKT_DATA_DISCONNECTED;
            if(IPv4ConnectionStatus == QWDS_PKT_DATA_DISCONNECTED && IPV6ConnectionStatus == QWDS_PKT_DATA_DISCONNECTED)
            {
                usbnet_link_change(0, &profile);
            }
            if((profile.ipv6_flag && profile.ipv6_flag ) && ((IPv4ConnectionStatus == QWDS_PKT_DATA_CONNECTED && IPV6ConnectionStatus == QWDS_PKT_DATA_CONNECTED))    )
                {
                    usbnet_link_change(1, &profile);
                }
            else
                if(IPv4ConnectionStatus == QWDS_PKT_DATA_CONNECTED || IPV6ConnectionStatus == QWDS_PKT_DATA_CONNECTED)
                {
                    usbnet_link_change(1, &profile);
                }
            if((profile.ipv4_flag && IPv4ConnectionStatus == QWDS_PKT_DATA_DISCONNECTED ) ||
            (profile.ipv6_flag && IPV6ConnectionStatus == QWDS_PKT_DATA_DISCONNECTED ))
            {
                send_signo_to_main(SIGUSR1);
            }
            }
            break;

            case SIGTERM:
            case SIGHUP:
            case SIGINT:
                if(profile.ipv4_flag && IPv4ConnectionStatus == QWDS_PKT_DATA_CONNECTED)
                    requestDeactivateDefaultPDP(&profile, IpFamilyV4);
                if(profile.ipv6_flag && IPV6ConnectionStatus == QWDS_PKT_DATA_CONNECTED)
                    requestDeactivateDefaultPDP(&profile, IpFamilyV6);

            usbnet_link_change(0, &profile);
            if (profile.qmi_ops->deinit)
                profile.qmi_ops->deinit();
            main_send_event_to_qmidevice(RIL_REQUEST_QUIT);
            goto __main_quit;
            break;

            default:
            break;
            }
        }
        }

        if (fd == qmidevice_control_fd[0]) {
        if (read(fd, &triger_event, sizeof(triger_event)) ==
            sizeof(triger_event)) {
            switch (triger_event) {
            case RIL_INDICATE_DEVICE_DISCONNECTED:
            usbnet_link_change(0, &profile);
            if (main_loop) {
                if (pthread_join(gQmiThreadID, NULL)) {
                dbg_time(
                    "%s Error joining to listener thread (%s)",
                    __func__, strerror(errno));
                }
                profile.qmichannel = NULL;
                profile.usbnet_adapter = save_usbnet_adapter;
                goto __main_loop;
            }
            goto __main_quit;
            break;

            case RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED:
            requestRegistrationState2(&PSAttachedState);
            if (PSAttachedState == 1)
            {
//begin modify for mantis 0050112 by kaibo.zhangkaibo@fibocom.com
                if((profile.ipv4_flag && IPv4ConnectionStatus == QWDS_PKT_DATA_DISCONNECTED)
                ||(profile.ipv6_flag && IPV6ConnectionStatus == QWDS_PKT_DATA_DISCONNECTED))
//end modify for mantis 0050112 by kaibo.zhangkaibo@fibocom.com
                send_signo_to_main(SIGUSR1);
            }
            break;

            case RIL_UNSOL_DATA_CALL_LIST_CHANGED:
            if (IPv4ConnectionStatus ==  QWDS_PKT_DATA_CONNECTED || IPV6ConnectionStatus ==  QWDS_PKT_DATA_CONNECTED)
                send_signo_to_main(SIGUSR2);
             break;

            default:
            break;
            }
        }
        }
    }
    }

__main_quit:
    usbnet_link_change(0, &profile);
    if (pthread_join(gQmiThreadID, NULL)) {
    dbg_time("%s Error joining to listener thread (%s)", __func__,
         strerror(errno));
    }
    close(signal_control_fd[0]);
    close(signal_control_fd[1]);
    close(qmidevice_control_fd[0]);
    close(qmidevice_control_fd[1]);
    dbg_time("%s exit", __func__);
    if (logfilefp)
    fclose(logfilefp);

//2020-12-23 willa.liu@fibocom.com changed begin for support mantis 0065286
    unlink(fibo_dial_filepath);
    close(socket_server_fd );
    close(cli_accept_fd );
//2020-12-23 willa.liu@fibocom.com changed end for support mantis 0065286

    return 0;
}
