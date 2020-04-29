#include "QMIThread.h"
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <dirent.h>

//#define CONFIG_EXIT_WHEN_DIAL_FAILED
//#define CONFIG_BACKGROUND_WHEN_GET_IP
//#define CONFIG_PID_FILE_FORMAT "/var/run/quectel-CM-%s.pid" //for example /var/run/quectel-CM-wwan0.pid

int debug_qmi = 0;
int main_loop = 0;
char * qmichannel;
int qmidevice_control_fd[2];
static int signal_control_fd[2];

#ifdef CONFIG_BACKGROUND_WHEN_GET_IP
static int daemon_pipe_fd[2];

static void ql_prepare_daemon(void) {
    pid_t daemon_child_pid;

    if (pipe(daemon_pipe_fd) < 0) {
        dbg_time("%s Faild to create daemon_pipe_fd: %d (%s)", __func__, errno, strerror(errno));
        return;
    }

    daemon_child_pid = fork();
    if (daemon_child_pid > 0) {
        struct pollfd pollfds[] = {{daemon_pipe_fd[0], POLLIN, 0}, {0, POLLIN, 0}};
        int ne, ret, nevents = sizeof(pollfds)/sizeof(pollfds[0]);
        int signo;

        //dbg_time("father");

        close(daemon_pipe_fd[1]);

        if (socketpair( AF_LOCAL, SOCK_STREAM, 0, signal_control_fd) < 0 ) {
            dbg_time("%s Faild to create main_control_fd: %d (%s)", __func__, errno, strerror(errno));
            return;
        }

        pollfds[1].fd = signal_control_fd[1];

        while (1) {
            do {
                ret = poll(pollfds, nevents, -1);
            } while ((ret < 0) && (errno == EINTR));

            if (ret < 0) {
                dbg_time("%s poll=%d, errno: %d (%s)", __func__, ret, errno, strerror(errno));
                goto __daemon_quit;
            }

            for (ne = 0; ne < nevents; ne++) {
                int fd = pollfds[ne].fd;
                short revents = pollfds[ne].revents;

                if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                    //dbg_time("%s poll err/hup", __func__);
                    //dbg_time("poll fd = %d, events = 0x%04x", fd, revents);
                    if (revents & POLLHUP)
                        goto __daemon_quit;
                }

                if ((revents & POLLIN) &&  read(fd, &signo, sizeof(signo)) == sizeof(signo)) {
                    if (signal_control_fd[1] == fd) {
                        if (signo == SIGCHLD) {
                            int status;
                            int pid = waitpid(daemon_child_pid, &status, 0);
                            dbg_time("waitpid pid=%d, status=%x", pid, status);
                            goto __daemon_quit;
                        } else {
                            kill(daemon_child_pid, signo);
                        }
                    } else if (daemon_pipe_fd[0] == fd) {
                        //dbg_time("daemon_pipe_signo = %d", signo);
                        goto __daemon_quit;
                    }
                }
            }
        }
__daemon_quit:
        //dbg_time("father exit");
        _exit(0);
    } else if (daemon_child_pid == 0) {
        close(daemon_pipe_fd[0]);
        //dbg_time("child", getpid());
    } else {
        close(daemon_pipe_fd[0]);
        close(daemon_pipe_fd[1]);
        dbg_time("%s Faild to create daemon_child_pid: %d (%s)", __func__, errno, strerror(errno));
    }
}

static void ql_enter_daemon(int signo) {
    if (daemon_pipe_fd[1] > 0)
        if (signo) {
            write(daemon_pipe_fd[1], &signo, sizeof(signo));
            sleep(1);
        }
        close(daemon_pipe_fd[1]);
        daemon_pipe_fd[1] = -1;
        setsid();
    }
#endif

//UINT ifc_get_addr(const char *ifname);

static void usbnet_link_change(int link, PROFILE_T *profile) {
    static int s_link = 0;

    if (s_link == link)
        return;

    s_link = link;

    if (link) {
        requestGetIPAddress(profile, IpFamilyV4);
        if (profile->IsDualIPSupported)
            requestGetIPAddress(profile, IpFamilyV6);
        udhcpc_start(profile);
    } else {
        udhcpc_stop(profile);
    }

#ifdef LINUX_RIL_SHLIB
    if (link) {
        int timeout = 6;
        while (timeout-- /*&& ifc_get_addr(profile->usbnet_adapter) == 0*/) {
            sleep(1);
        }
    }

    if (link && requestGetIPAddress(profile, 0x04) == 0) {
        unsigned char *r;

        dbg_time("Using interface %s", profile->usbnet_adapter);
        r = (unsigned char *)&profile->ipv4.Address;
        dbg_time("local  IP address %d.%d.%d.%d", r[3], r[2], r[1], r[0]);
        r = (unsigned char *)&profile->ipv4.Gateway;
        dbg_time("remote IP address %d.%d.%d.%d", r[3], r[2], r[1], r[0]);
        r = (unsigned char *)&profile->ipv4.DnsPrimary;
        dbg_time("primary   DNS address %d.%d.%d.%d", r[3], r[2], r[1], r[0]);
        r = (unsigned char *)&profile->ipv4.DnsSecondary;
        dbg_time("secondary DNS address %d.%d.%d.%d", r[3], r[2], r[1], r[0]);
     }
#endif

#ifdef CONFIG_BACKGROUND_WHEN_GET_IP
    if (link && daemon_pipe_fd[1] > 0) {
        int timeout = 6;
        while (timeout-- /*&& ifc_get_addr(profile->usbnet_adapter) == 0*/) {
            sleep(1);
        }
        ql_enter_daemon(SIGUSR1);
    }
#endif
}

static int check_ipv4_address(PROFILE_T *now_profile) {
    PROFILE_T new_profile_v;
    PROFILE_T *new_profile = &new_profile_v;

    memcpy(new_profile, now_profile, sizeof(PROFILE_T));
    if (requestGetIPAddress(new_profile, 0x04) == 0) {
         if (new_profile->ipv4.Address != now_profile->ipv4.Address || debug_qmi) {
             unsigned char *l = (unsigned char *)&now_profile->ipv4.Address;
             unsigned char *r = (unsigned char *)&new_profile->ipv4.Address;
             dbg_time("localIP: %d.%d.%d.%d VS remoteIP: %d.%d.%d.%d",
                     l[3], l[2], l[1], l[0], r[3], r[2], r[1], r[0]);
        }
        return (new_profile->ipv4.Address == now_profile->ipv4.Address);
    }
    return 0;
}

static void main_send_event_to_qmidevice(int triger_event) {
     write(qmidevice_control_fd[0], &triger_event, sizeof(triger_event));
}

static void send_signo_to_main(int signo) {
     write(signal_control_fd[0], &signo, sizeof(signo));
}

void qmidevice_send_event_to_main(int triger_event) {
     write(qmidevice_control_fd[1], &triger_event, sizeof(triger_event));
}

#define MAX_PATH 256

static int ls_dir(const char *dir, int (*match)(const char *dir, const char *file, void *argv[]), void *argv[])
{
    DIR *pDir;
    struct dirent* ent = NULL;
    int match_times = 0;

    pDir = opendir(dir);
    if (pDir == NULL)  {
        dbg_time("Cannot open directory: %s, errno: %d (%s)", dir, errno, strerror(errno));
        return 0;
    }

    while ((ent = readdir(pDir)) != NULL)  {
        match_times += match(dir, ent->d_name, argv);
    }
    closedir(pDir);

    return match_times;
}

static int is_same_linkfile(const char *dir, const char *file,  void *argv[])
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
    //const char *myself = (const char *)argv[0];
    char linkname[MAX_PATH];
    char filename[MAX_PATH];
    int linksize;
    int i = 0, kill_timeout = 15;
    pid_t pid;

    //dbg_time("%s", file);
    while (file[i]) {
        if (!isdigit(file[i]))
            break;
        i++;
    }

    if (file[i]) {
        //dbg_time("%s not digit", file);
        return 0;
    }

    snprintf(linkname, MAX_PATH, "%s/%s/exe", dir, file);
    linksize = readlink(linkname, filename, MAX_PATH);
    if (linksize <= 0)
        return 0;

    filename[linksize] = 0;
#if 0 //check all process
    if (strcmp(filename, myself))
        return 0;
#endif

    pid = atoi(file);
    if (pid == getpid())
        return 0;

    snprintf(linkname, MAX_PATH, "%s/%s/fd", dir, file);
    if (!ls_dir(linkname, is_same_linkfile, argv))
        return 0;

    dbg_time("%s/%s/exe -> %s", dir, file, filename);
    while (kill_timeout-- && !kill(pid, 0))
    {
        kill(pid, SIGTERM);
        sleep(1);
    }
    if (!kill(pid, 0))
    {
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

static void ql_sigaction(int signo) {
     if (SIGCHLD == signo)
         waitpid(-1, NULL, WNOHANG);
     else if (SIGALRM == signo)
         send_signo_to_main(SIGUSR1);
     else
     {
        if (SIGTERM == signo || SIGHUP == signo || SIGINT == signo)
            main_loop = 0;
         send_signo_to_main(signo);
        main_send_event_to_qmidevice(signo); //main may be wating qmi response
    }
}

pthread_t gQmiThreadID;

static int usage(const char *progname) {
     dbg_time("Usage: %s [-s [apn [user password auth]]] [-p pincode] [-f logfilename] ", progname);
     dbg_time("-s [apn [user password auth]] Set apn/user/password/auth get from your network provider");
     dbg_time("-p pincode                    Verify sim card pin if sim card is locked");
     dbg_time("-f logfilename                Save log message of this program to file");
     dbg_time("Example 1: %s ", progname);
     dbg_time("Example 2: %s -s 3gnet ", progname);
     dbg_time("Example 3: %s -s 3gnet carl 1234 0 -p 1234 -f gobinet_log.txt", progname);
     return 0;
}
static int charsplit(const char *src,char* desc,int n,const char* splitStr)
{
	char* p;
	char*p1;
	int len;
	
	len=strlen(splitStr);
	p=strstr(src,splitStr);
	if(p==NULL)
		return -1;
	p1=strstr(p,"\n");
	if(p1==NULL)
		return -1;
	memset(desc,0,n);
	memcpy(desc,p+len,p1-p-len);
	
	return 0;
}

static int get_dev_major_minor(char* path, int *major, int *minor)
{
	int fd = -1;
	char desc[128] = {0};
	char devmajor[64],devminor[64];
	int n = 0;
	if(access(path, R_OK | W_OK))
	{
		return 1;
	}
	if((fd = open(path, O_RDWR)) < 0)
	{
		return 1;
	}
	n = read(fd , desc, sizeof(desc));
	if(n == sizeof(desc))
	{
		dbg_time("may be overflow");
	}
	close(fd);
	if(charsplit(desc,devmajor,64,"MAJOR=")==-1 ||
		charsplit(desc,devminor,64,"MINOR=")==-1 )
	{
		return 2;
	}
	*major = atoi(devmajor);
	*minor = atoi(devminor);
	return 0;
}
static int qmidevice_detect(char **pp_qmichannel, char **pp_usbnet_adapter) {
    struct dirent* ent = NULL;
    DIR *pDir;

    char dir[255] = "/sys/bus/usb/devices";
    int major = 0, minor = 0;
    pDir = opendir(dir);
    if (pDir)  {
        while ((ent = readdir(pDir)) != NULL)  {
            struct dirent* subent = NULL;
            DIR *psubDir;
            char subdir[255];
            char subdir2[255 * 2];

            char idVendor[4+1] = {0};
            char idProduct[4+1] = {0};
            int fd = 0;

            char netcard[32] = "\0";
            char qmifile[32] = "\0";

            snprintf(subdir, sizeof(subdir), "%s/%s/idVendor", dir, ent->d_name);
            fd = open(subdir, O_RDONLY);
            if (fd > 0) {
                read(fd, idVendor, 4);
                close(fd);
             }

            snprintf(subdir, sizeof(subdir), "%s/%s/idProduct", dir, ent->d_name);
            fd  = open(subdir, O_RDONLY);
            if (fd > 0) {
                read(fd, idProduct, 4);
                close(fd);
            }

            if (!strncasecmp(idVendor, "05c6", 4) || !strncasecmp(idVendor, "2c7c", 4))
                ;
            else
                continue;

            dbg_time("Find %s/%s idVendor=%s idProduct=%s", dir, ent->d_name, idVendor, idProduct);

            snprintf(subdir, sizeof(subdir), "%s/%s:1.4/net", dir, ent->d_name);
            psubDir = opendir(subdir);
            if (psubDir == NULL)  {
                dbg_time("Cannot open directory: %s, errno: %d (%s)", subdir, errno, strerror(errno));
                continue;
            }

            while ((subent = readdir(psubDir)) != NULL)  {
                if (subent->d_name[0] == '.')
                    continue;
                dbg_time("Find %s/%s", subdir, subent->d_name);
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

            snprintf(subdir, sizeof(subdir), "%s/%s:1.4/GobiQMI", dir, ent->d_name);
            if (access(subdir, R_OK)) {
                snprintf(subdir, sizeof(subdir), "%s/%s:1.4/usbmisc", dir, ent->d_name);
                if (access(subdir, R_OK)) {
                    snprintf(subdir, sizeof(subdir), "%s/%s:1.4/usb", dir, ent->d_name);
                    if (access(subdir, R_OK)) {
                        dbg_time("no GobiQMI/usbmic/usb found in %s/%s:1.4", dir, ent->d_name);
                        continue;
                    }
                }
            }
            
            psubDir = opendir(subdir);
            if (pDir == NULL)  {
                dbg_time("Cannot open directory: %s, errno: %d (%s)", dir, errno, strerror(errno));
                continue;
            }

            while ((subent = readdir(psubDir)) != NULL)  {
                if (subent->d_name[0] == '.')
                    continue;
                dbg_time("Find %s/%s", subdir, subent->d_name);
                dbg_time("Find qmichannel = /dev/%s", subent->d_name);
                snprintf(qmifile, sizeof(qmifile), "/dev/%s", subent->d_name);

                //get major minor
                snprintf(subdir2, sizeof(subdir), "%s/%s/uevent",subdir, subent->d_name);
                if(!get_dev_major_minor(subdir2, &major, &minor))
                {
                	//dbg_time("%s major = %d, minor = %d\n",qmifile, major, minor);
                }else
                {
                	dbg_time("get %s major and minor failed\n",qmifile);
                }
                //get major minor
                
                if((fd = open(qmifile, R_OK)) < 0)
                {
                	dbg_time("%s open failed", qmifile);
                	dbg_time("please mknod %s c %d %d", qmifile, major, minor);
                }else
                {
                	close(fd);
                }
                break;
            }

            closedir(psubDir);

            if (netcard[0] && qmifile[0]) {
                *pp_qmichannel = strdup(qmifile);
                *pp_usbnet_adapter  = strdup(netcard);
                closedir(pDir);
                return 1;
            }

        }
        
        closedir(pDir);
    }

    if ((pDir = opendir("/dev")) == NULL)  {
        dbg_time("Cannot open directory: %s, errno:%d (%s)", "/dev", errno, strerror(errno));
        return -ENODEV;
    }

    while ((ent = readdir(pDir)) != NULL) {
        if ((strncmp(ent->d_name, "cdc-wdm", strlen("cdc-wdm")) == 0) || (strncmp(ent->d_name, "qcqmi", strlen("qcqmi")) == 0)) {
            char net_path[64];

            *pp_qmichannel = (char *)malloc(32);
            sprintf(*pp_qmichannel, "/dev/%s", ent->d_name);
            dbg_time("Find qmichannel = %s", *pp_qmichannel);

            if (strncmp(ent->d_name, "cdc-wdm", strlen("cdc-wdm")) == 0)
                sprintf(net_path, "/sys/class/net/wwan%s", &ent->d_name[strlen("cdc-wdm")]);
            else
            {
                sprintf(net_path, "/sys/class/net/usb%s", &ent->d_name[strlen("qcqmi")]);
                #if 0//ndef ANDROID
                if (kernel_version >= KVERSION( 2,6,39 ))
                    sprintf(net_path, "/sys/class/net/eth%s", &ent->d_name[strlen("qcqmi")]);
                #else
                if (access(net_path, R_OK) && errno == ENOENT)
                    sprintf(net_path, "/sys/class/net/eth%s", &ent->d_name[strlen("qcqmi")]);
                #endif
#if 0 //openWRT like use ppp# or lte#
                if (access(net_path, R_OK) && errno == ENOENT)
                    sprintf(net_path, "/sys/class/net/ppp%s", &ent->d_name[strlen("qcqmi")]);
                if (access(net_path, R_OK) && errno == ENOENT)
                    sprintf(net_path, "/sys/class/net/lte%s", &ent->d_name[strlen("qcqmi")]);
#endif
            }

            if (access(net_path, R_OK) == 0)
            {
                if (*pp_usbnet_adapter && strcmp(*pp_usbnet_adapter, (net_path + strlen("/sys/class/net/"))))
                {
                    free(*pp_qmichannel); *pp_qmichannel = NULL;
                    continue;
                }
                *pp_usbnet_adapter = strdup(net_path + strlen("/sys/class/net/"));
                dbg_time("Find usbnet_adapter = %s", *pp_usbnet_adapter);
                break;
            }
            else
            {
                dbg_time("Failed to access %s, errno:%d (%s)", net_path, errno, strerror(errno));
                free(*pp_qmichannel); *pp_qmichannel = NULL;
            }
        }
    }
    closedir(pDir);

    return (*pp_qmichannel && *pp_usbnet_adapter);
}

#if defined(ANDROID) || defined(LINUX_RIL_SHLIB)
int quectel_CM(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    int triger_event = 0;
    int opt = 1;
    int signo;
#ifdef CONFIG_SIM
    SIM_Status SIMStatus;
#endif
    UCHAR PSAttachedState;
    UCHAR  IPv4ConnectionStatus = 0xff; //unknow state
    UCHAR  IPV6ConnectionStatus = 0xff; //unknow state
    char * save_usbnet_adapter = NULL;
    PROFILE_T profile;
#ifdef CONFIG_RESET_RADIO
    struct timeval resetRadioTime = {0};
    struct timeval  nowTime;
    gettimeofday(&resetRadioTime, (struct timezone *) NULL);
#endif

    memset(&profile, 0x00, sizeof(profile));
    profile.pdp = CONFIG_DEFAULT_PDP;

    if (!strcmp(argv[argc-1], "&"))
        argc--;

    opt = 1;
    while  (opt < argc)
    {
        if (argv[opt][0] != '-')
            return usage(argv[0]);

        switch (argv[opt++][1])
        {
#define has_more_argv() ((opt < argc) && (argv[opt][0] != '-'))
            case 's':
                profile.apn = profile.user = profile.password = "";
                if (has_more_argv())
                    profile.apn = argv[opt++];
                if (has_more_argv())
                    profile.user = argv[opt++];
                if (has_more_argv())
                {
                    profile.password = argv[opt++];
                    if (profile.password && profile.password[0])
                        profile.auth = 2; //default chap, customers may miss auth
                }
                if (has_more_argv())
                    profile.auth = argv[opt++][0] - '0';
            break;

            case 'p':
                if (has_more_argv())
                    profile.pincode = argv[opt++];
            break;

            case 'n':
                if (has_more_argv())
                    profile.pdp = argv[opt++][0] - '0';
            break;

            case 'f':
                if (has_more_argv())
                {
                    const char * filename = argv[opt++];
                    logfilefp = fopen(filename, "a+");
                    if (!logfilefp) {
                        dbg_time("Fail to open %s, errno: %d(%s)", filename, errno, strerror(errno));
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
                main_loop = 1;
            break;

            case '6':
                profile.IsDualIPSupported |= (1 << IpFamilyV6); //support ipv4&ipv6
            break;

            default:
                return usage(argv[0]);
            break;
        }
    }

    dbg_time("WCDMA&LTE_QConnectManager_Linux&Android_V1.1.34");
    dbg_time("%s profile[%d] = %s/%s/%s/%d, pincode = %s", argv[0], profile.pdp, profile.apn, profile.user, profile.password, profile.auth, profile.pincode);

    signal(SIGUSR1, ql_sigaction);
    signal(SIGUSR2, ql_sigaction);
    signal(SIGINT, ql_sigaction);
    signal(SIGTERM, ql_sigaction);
    signal(SIGHUP, ql_sigaction);
    signal(SIGCHLD, ql_sigaction);
    signal(SIGALRM, ql_sigaction);

#ifdef CONFIG_BACKGROUND_WHEN_GET_IP
    ql_prepare_daemon();
#endif

    if (socketpair( AF_LOCAL, SOCK_STREAM, 0, signal_control_fd) < 0 ) {
        dbg_time("%s Faild to create main_control_fd: %d (%s)", __func__, errno, strerror(errno));
        return -1;
    }

    if ( socketpair( AF_LOCAL, SOCK_STREAM, 0, qmidevice_control_fd ) < 0 ) {
        dbg_time("%s Failed to create thread control socket pair: %d (%s)", __func__, errno, strerror(errno));
        return 0;
    }

//sudo apt-get install udhcpc
//sudo apt-get remove ModemManager
__main_loop:
    while (!profile.qmichannel)
    {
        if (qmidevice_detect(&profile.qmichannel, &profile.usbnet_adapter))
            break;
        if (main_loop)
        {
            int wait_for_device = 3000;
            dbg_time("Wait for Quectel modules connect");
            while (wait_for_device && main_loop) {
                wait_for_device -= 100;
                usleep(100*1000);
            }
            continue;
        }
        dbg_time("Cannot find qmichannel(%s) usbnet_adapter(%s) for Quectel modules", profile.qmichannel, profile.usbnet_adapter);
        return -ENODEV;
    }

    if (access(profile.qmichannel, R_OK | W_OK)) {
        dbg_time("Fail to access %s, errno: %d (%s)", profile.qmichannel, errno, strerror(errno));
        return errno;
    }

#if 0 //for test only, make fd > 255
{
    int max_dup = 255;
    while (max_dup--)
        dup(0);
}
#endif

    kill_brothers(profile.qmichannel);

    qmichannel = profile.qmichannel;
    if (!strncmp(profile.qmichannel, "/dev/qcqmi", strlen("/dev/qcqmi")))
    {
        if (pthread_create( &gQmiThreadID, 0, GobiNetThread, (void *)&profile) != 0)
        {
            dbg_time("%s Failed to create GobiNetThread: %d (%s)", __func__, errno, strerror(errno));
            return 0;
        }
    }
    else
    {
        if (pthread_create( &gQmiThreadID, 0, QmiWwanThread, (void *)profile.qmichannel) != 0)
        {
            dbg_time("%s Failed to create QmiWwanThread: %d (%s)", __func__, errno, strerror(errno));
            return 0;
        }
    }

    if ((read(qmidevice_control_fd[0], &triger_event, sizeof(triger_event)) != sizeof(triger_event))
        || (triger_event != RIL_INDICATE_DEVICE_CONNECTED)) {
        dbg_time("%s Failed to init QMIThread: %d (%s)", __func__, errno, strerror(errno));
        return 0;
    }

    if (!strncmp(profile.qmichannel, "/dev/cdc-wdm", strlen("/dev/cdc-wdm"))) {
        if (QmiWwanInit(&profile)) {
            dbg_time("%s Failed to QmiWwanInit: %d (%s)", __func__, errno, strerror(errno));
            return 0;
        }
    }

#ifdef CONFIG_VERSION
    requestBaseBandVersion(NULL);
#endif
    requestSetEthMode(&profile);
#if 0
    if (profile.rawIP && !strncmp(profile.qmichannel, "/dev/cdc-wdm", strlen("/dev/cdc-wdm"))) {
        char raw_ip_switch[128] = {0};
        sprintf(raw_ip_switch, "/sys/class/net/%s/qmi/raw_ip", profile.usbnet_adapter);
        if (!access(raw_ip_switch, R_OK)) {
            int raw_ip_fd = -1;
            raw_ip_fd = open(raw_ip_switch, O_RDWR);
            if (raw_ip_fd >= 0) {
                write(raw_ip_fd, "1", strlen("1"));
                close(raw_ip_fd);
                raw_ip_fd = -1;
            } else {
                dbg_time("open %s failed, errno = %d(%s)\n", raw_ip_switch, errno, strerror(errno));
            }
        }
    }
#endif
#ifdef CONFIG_SIM
    requestGetSIMStatus(&SIMStatus);
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
    requestRegistrationState(&PSAttachedState);

    if (!requestQueryDataCall(&IPv4ConnectionStatus, IpFamilyV4) && (QWDS_PKT_DATA_CONNECTED == IPv4ConnectionStatus))
        usbnet_link_change(1, &profile);
     else
        usbnet_link_change(0, &profile);

    send_signo_to_main(SIGUSR1);

#ifdef CONFIG_PID_FILE_FORMAT
    {
        char cmd[255];
        sprintf(cmd, "echo %d > " CONFIG_PID_FILE_FORMAT, getpid(), profile.usbnet_adapter);
        system(cmd);
    }
#endif

    while (1)
    {
        struct pollfd pollfds[] = {{signal_control_fd[1], POLLIN, 0}, {qmidevice_control_fd[0], POLLIN, 0}};
        int ne, ret, nevents = sizeof(pollfds)/sizeof(pollfds[0]);

        do {
            ret = poll(pollfds, nevents,  15*1000);
        } while ((ret < 0) && (errno == EINTR));

        if (ret == 0)
        {
            send_signo_to_main(SIGUSR2);
            continue;
        }

        if (ret <= 0) {
            dbg_time("%s poll=%d, errno: %d (%s)", __func__, ret, errno, strerror(errno));
            goto __main_quit;
        }

        for (ne = 0; ne < nevents; ne++) {
            int fd = pollfds[ne].fd;
            short revents = pollfds[ne].revents;

            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                dbg_time("%s poll err/hup", __func__);
                dbg_time("epoll fd = %d, events = 0x%04x", fd, revents);
                main_send_event_to_qmidevice(RIL_REQUEST_QUIT);
                if (revents & POLLHUP)
                    goto __main_quit;
            }

            if ((revents & POLLIN) == 0)
                continue;

            if (fd == signal_control_fd[1])
            {
                if (read(fd, &signo, sizeof(signo)) == sizeof(signo))
                {
                    alarm(0);
                    switch (signo)
                    {
                        case SIGUSR1:
                            requestQueryDataCall(&IPv4ConnectionStatus, IpFamilyV4);
                            if (QWDS_PKT_DATA_CONNECTED != IPv4ConnectionStatus)
                            {
                                usbnet_link_change(0, &profile);
                                requestRegistrationState(&PSAttachedState);
                                if (PSAttachedState == 1 && requestSetupDataCall(&profile, IpFamilyV4) == 0)
                                {
                                    //succssful setup data call
                                    if (profile.IsDualIPSupported) {
                                        requestSetupDataCall(&profile, IpFamilyV6);
                                    }
                                }
                                else
                                {
#ifdef CONFIG_EXIT_WHEN_DIAL_FAILED
                                    kill(getpid(), SIGTERM);
#endif
#ifdef CONFIG_RESET_RADIO
                                    gettimeofday(&nowTime, (struct timezone *) NULL);
                                    if (abs(nowTime.tv_sec - resetRadioTime.tv_sec) > CONFIG_RESET_RADIO) {
                                        resetRadioTime = nowTime;
                                        //requestSetOperatingMode(0x06); //same as AT+CFUN=0
                                        requestSetOperatingMode(0x01); //same as AT+CFUN=4
                                        requestSetOperatingMode(0x00); //same as AT+CFUN=1
                                    }
#endif
                                    alarm(5); //try to setup data call 5 seconds later
                                }
                            }
                        break;

                        case SIGUSR2:
                            if (QWDS_PKT_DATA_CONNECTED == IPv4ConnectionStatus)
                                 requestQueryDataCall(&IPv4ConnectionStatus, IpFamilyV4);

                            //local ip is different with remote ip
                            if (QWDS_PKT_DATA_CONNECTED == IPv4ConnectionStatus && check_ipv4_address(&profile) == 0) {
                                requestDeactivateDefaultPDP(&profile, IpFamilyV4);
                                IPv4ConnectionStatus = QWDS_PKT_DATA_DISCONNECTED;
                            }
                            
                            if (QWDS_PKT_DATA_CONNECTED != IPv4ConnectionStatus) {
                                #if defined(ANDROID) || defined(LINUX_RIL_SHLIB)
                                kill(getpid(), SIGTERM); //android will setup data call again
                                #else
                                send_signo_to_main(SIGUSR1);
                                #endif
                            }
                        break;

                        case SIGTERM:
                        case SIGHUP:
                        case SIGINT:
                            if (QWDS_PKT_DATA_CONNECTED == IPv4ConnectionStatus) {
                                requestDeactivateDefaultPDP(&profile, IpFamilyV4);
                                if (profile.IsDualIPSupported)
                                    requestDeactivateDefaultPDP(&profile, IpFamilyV6);
                           }
                            usbnet_link_change(0, &profile);
                            if (!strncmp(profile.qmichannel, "/dev/cdc-wdm", strlen("/dev/cdc-wdm")))
                                QmiWwanDeInit();
                            main_send_event_to_qmidevice(RIL_REQUEST_QUIT);
                            goto __main_quit;
                        break;

                        default:
                        break;
                    }
                }
            }

            if (fd == qmidevice_control_fd[0]) {
                if (read(fd, &triger_event, sizeof(triger_event)) == sizeof(triger_event)) {
                    switch (triger_event) {
                        case RIL_INDICATE_DEVICE_DISCONNECTED:
                            usbnet_link_change(0, &profile);
                            if (main_loop)
                            {
                                if (pthread_join(gQmiThreadID, NULL)) {
                                    dbg_time("%s Error joining to listener thread (%s)", __func__, strerror(errno));
                                }
                                profile.qmichannel = NULL;
                                profile.usbnet_adapter = save_usbnet_adapter;
                                goto __main_loop;
                            }
                            goto __main_quit;
                        break;

                        case RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED:
                            requestRegistrationState(&PSAttachedState);
                            if (PSAttachedState == 1 && QWDS_PKT_DATA_DISCONNECTED == IPv4ConnectionStatus)
                                send_signo_to_main(SIGUSR1);
                        break;

                        case RIL_UNSOL_DATA_CALL_LIST_CHANGED:
                        {
                            #if defined(ANDROID) || defined(LINUX_RIL_SHLIB)
                            UCHAR oldConnectionStatus = IPv4ConnectionStatus;
                            #endif
                            requestQueryDataCall(&IPv4ConnectionStatus, IpFamilyV4);
                            if (profile.IsDualIPSupported)
                                requestQueryDataCall(&IPV6ConnectionStatus, IpFamilyV6);
                            if (QWDS_PKT_DATA_CONNECTED != IPv4ConnectionStatus)
                            {
                                usbnet_link_change(0, &profile);
                                #if defined(ANDROID) || defined(LINUX_RIL_SHLIB)
                                if (oldConnectionStatus == QWDS_PKT_DATA_CONNECTED) //connected change to disconnect
                                    kill(getpid(), SIGTERM); //android will setup data call again
                                #else
                                send_signo_to_main(SIGUSR1);
                                #endif
                            } else if (QWDS_PKT_DATA_CONNECTED == IPv4ConnectionStatus) {
                                usbnet_link_change(1, &profile);
                            }
                        }
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
        dbg_time("%s Error joining to listener thread (%s)", __func__, strerror(errno));
    }
    close(signal_control_fd[0]);
    close(signal_control_fd[1]);
    close(qmidevice_control_fd[0]);
    close(qmidevice_control_fd[1]);
    dbg_time("%s exit", __func__);
    if (logfilefp)
        fclose(logfilefp);

#ifdef CONFIG_PID_FILE_FORMAT
    {
        char cmd[255];
        sprintf(cmd, "rm  " CONFIG_PID_FILE_FORMAT, profile.usbnet_adapter);
        system(cmd);
    }
#endif

    return 0;
}
