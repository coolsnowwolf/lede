#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <endian.h>

#include "libmnl/ifutils.h"
#include "libmnl/dhcp/dhcp.h"
#include "util.h"
#include "QMIThread.h"

static int fibo_system(const char *shell_cmd)
{
    int ret = 0;
    dbg_time("%s", shell_cmd);
    ret = system(shell_cmd);
    if (ret) {
	// dbg_time("Fail to system(\"%s\") = %d, errno: %d (%s)", shell_cmd,
	// ret, errno, strerror(errno));
    }
    return ret;
}

int fibo_raw_ip_mode_check(const char *ifname)
{
    int fd;
    char raw_ip[128];
    char mode[2] = "X";
    int mode_change = 0;

    snprintf(raw_ip, sizeof(raw_ip), "/sys/class/net/%s/qmi/raw_ip", ifname);
    if (access(raw_ip, F_OK))
        return 0;

    fd = open(raw_ip, O_RDWR | O_NONBLOCK | O_NOCTTY);
    if (fd < 0)
    {
        dbg_time("%s %d fail to open(%s), errno:%d (%s)", __FILE__, __LINE__, raw_ip, errno, strerror(errno));
        return 0;
    }

    read(fd, mode, 2);
    if (mode[0] == '0' || mode[0] == 'N')
    {
        if_link_down(ifname);
        dbg_time("echo Y > /sys/class/net/%s/qmi/raw_ip", ifname);
        mode[0] = 'Y';
        write(fd, mode, 2);
        mode_change = 1;
        if_link_up(ifname);
    }

    close(fd);
    return mode_change;
}

static void fibo_set_driver_link_state(PROFILE_T *profile, int link_state)
{
    char link_file[128];
    int fd;
    int new_state = 0;
    dbg_time("enter %s ", __func__);

    snprintf(link_file, sizeof(link_file), "/sys/class/net/%s/link_state", profile->usbnet_adapter);
    fd = open(link_file, O_RDWR | O_NONBLOCK | O_NOCTTY);
    if (fd == -1)
    {
        if (errno != ENOENT)
            dbg_time("Fail to access %s, errno: %d (%s)", link_file, errno, strerror(errno));
        return;
    }

    if (profile->qmap_mode <= 1)
        new_state = !!link_state;
    else
    {
        //0x80 means link off this pdp
        new_state = (link_state ? 0x00 : 0x80) + profile->pdp;
    }

    snprintf(link_file, sizeof(link_file), "%d\n", new_state);
    write(fd, link_file, sizeof(link_file));

    if (link_state == 0 && profile->qmap_mode > 1)
    {
        size_t rc;

        lseek(fd, 0, SEEK_SET);
        rc = read(fd, link_file, sizeof(link_file));
        if (rc > 1 && (!strcasecmp(link_file, "0\n") || !strcasecmp(link_file, "0x0\n")))
        {
           // if_link_down(profile->usbnet_adapter);
        }
    }

    close(fd);
}

void udhcpc_start(PROFILE_T *profile)
{
//2021-02-01 willa.liu@fibocom.com changed begin for support mantis 0069837
    char *ifname = profile->usbnet_adapter;
    char shell_cmd[512];

    fibo_set_driver_link_state(profile, 1);
    fibo_raw_ip_mode_check(ifname);

    if (profile->qmapnet_adapter)
    {
        ifname = profile->qmapnet_adapter;
    }
    if (profile->rawIP && profile->ipv4.Address && profile->ipv4.Mtu)
    {
        if_set_mtu(ifname, (profile->ipv4.Mtu));
    }

    if (strcmp(ifname, profile->usbnet_adapter))
    {
        //if_link_up(profile->usbnet_adapter);
        snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ifconfig %s up", profile->usbnet_adapter);
        fibo_system(shell_cmd);
    }
//2021-02-01 willa.liu@fibocom.com changed begin for support mantis 0069837
    if_link_up(ifname);

#if 1 //for bridge mode, only one public IP, so do udhcpc manually
    if (fibo_bridge_mode_detect(profile))
    {
        return;
    }
#endif
    // if use DHCP(should make with ${DHCP} src files)
    // do_dhcp(ifname);
    // return 0;
    /* IPv4 Addr Info */
    if (profile->ipv4.Address)
    {
        dbg_time("IPv4 MTU: %d", profile->ipv4.Mtu);
        dbg_time("IPv4 Address: %s", ipaddr_to_string_v4(ntohl(profile->ipv4.Address)));
        dbg_time("IPv4 Netmask: %d", mask_to_prefix_v4(ntohl(profile->ipv4.SubnetMask)));
        dbg_time("IPv4 Gateway: %s", ipaddr_to_string_v4(ntohl(profile->ipv4.Gateway)));
        dbg_time("IPv4 DNS1: %s", ipaddr_to_string_v4(ntohl(profile->ipv4.DnsPrimary)));
        dbg_time("IPv4 DNS2: %s", ipaddr_to_string_v4(ntohl(profile->ipv4.DnsSecondary)));
        if_set_network_v4(ifname, ntohl(profile->ipv4.Address),
                       mask_to_prefix_v4(profile->ipv4.SubnetMask),
                       ntohl(profile->ipv4.Gateway),
                       ntohl(profile->ipv4.DnsPrimary),
                       ntohl(profile->ipv4.DnsSecondary));
    }
    else
    {
        dbg_time("The IPv4 Address in profile is NULL");
    }

    if (profile->ipv6.Address && (profile->ipv6.PrefixLengthIPAddr != 0))
    {
        //module do not support DHCPv6, only support 'Router Solicit'
        //and it seem if enable /proc/sys/net/ipv6/conf/all/forwarding, Kernel do not send RS
        const char *forward_file = "/proc/sys/net/ipv6/conf/all/forwarding";
        int forward_fd = open(forward_file, O_RDONLY);
        if (forward_fd > 0)
        {
            char forward_state[2];
            read(forward_fd, forward_state, 2);
            if (forward_state[0] == '1')
            {
                dbg_time("%s enabled, kernel maybe donot send 'Router Solicit'", forward_file);
            }
            close(forward_fd);
        }

        dbg_time("IPv6 MTU: %d", profile->ipv6.Mtu);
        dbg_time("IPv6 Address: %s", ipaddr_to_string_v6(profile->ipv6.Address));
        dbg_time("IPv6 PrefixLengthIPAddr: %d", profile->ipv6.PrefixLengthIPAddr);
        dbg_time("IPv6 Gateway: %s", ipaddr_to_string_v6(profile->ipv6.Gateway));
        dbg_time("IPv6 DNS1: %s", ipaddr_to_string_v6(profile->ipv6.DnsPrimary));
        dbg_time("IPv6 DNS2: %s", ipaddr_to_string_v6(profile->ipv6.DnsSecondary));
        if_set_network_v6(ifname, profile->ipv6.Address, profile->ipv6.PrefixLengthIPAddr,
                       profile->ipv6.Gateway, profile->ipv6.DnsPrimary, profile->ipv6.DnsSecondary);
    }
    else
    {
        dbg_time("The IPv6 Address in profile is NULL");
    }
}

void udhcpc_stop(PROFILE_T *profile)
{
    char *ifname = profile->usbnet_adapter;
    dbg_time("enter %s ", __func__);

    fibo_set_driver_link_state(profile, 0);

    if (profile->qmapnet_adapter)
    {
        ifname = profile->qmapnet_adapter;
    }

    if_flush_v4_addr(ifname);
    if_flush_v6_addr(ifname);
	if_link_down(ifname);
}

static void fibo_set_mtu(const char *ifname, int ifru_mtu) {
    int inet_sock;
    struct ifreq ifr;

    inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (inet_sock > 0) {
        strcpy(ifr.ifr_name, ifname);

        if (!ioctl(inet_sock, SIOCGIFMTU, &ifr)) {
            if (ifr.ifr_ifru.ifru_mtu != ifru_mtu) {
                dbg_time("change mtu %d -> %d", ifr.ifr_ifru.ifru_mtu , ifru_mtu);
                ifr.ifr_ifru.ifru_mtu = ifru_mtu;
                ioctl(inet_sock, SIOCSIFMTU, &ifr);
            }
        }

        close(inet_sock);
    }
}

static void* udhcpc_thread_function(void* arg) {
    FILE * udhcpc_fp;
    char *udhcpc_cmd = (char *)arg;

    if (udhcpc_cmd == NULL)
        return NULL;

    dbg_time("%s", udhcpc_cmd);
    udhcpc_fp = popen(udhcpc_cmd, "r");
    free(udhcpc_cmd);
    if (udhcpc_fp) {
        char buf[0xff];

        buf[sizeof(buf)-1] = '\0';
        while((fgets(buf, sizeof(buf)-1, udhcpc_fp)) != NULL) {
            if ((strlen(buf) > 1) && (buf[strlen(buf) - 1] == '\n'))
                buf[strlen(buf) - 1] = '\0';
            dbg_time("%s", buf);
        }

        pclose(udhcpc_fp);
    }

    return NULL;
}

void udhcpc_start_pcie(PROFILE_T *profile) {
    char *ifname = profile->usbnet_adapter;
    char sub_intf_name[100] = {0};
    int intf_id = 0;
    char shell_cmd[512];
    dbg_time("enter %s ", __func__);

    if (profile->qmapnet_adapter) {
        ifname = profile->qmapnet_adapter;
    }

    if (profile->muxid > 0x81)
    {
        intf_id = profile->muxid - 0x81;
        snprintf(sub_intf_name, sizeof(sub_intf_name) - 1, "%s.%d", ifname, intf_id);
    }

    if (profile->rawIP && profile->ipv4.Address && profile->ipv4.Mtu) {
        fibo_set_mtu(ifname, (profile->ipv4.Mtu));
    }

    fibo_system("echo 1 > /sys/module/fibo_mhi/parameters/macaddr_check");

    if (strcmp(ifname, profile->usbnet_adapter)) {
        snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ifconfig %s up", profile->usbnet_adapter);
        fibo_system(shell_cmd);
    }

    // For IPv6, down & up will send protocol packets, and that's needed.
    if (profile->ipv6_flag && profile->muxid <= 0x81) {
        snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ifconfig %s down", ifname);
        fibo_system(shell_cmd);
    }

    snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ifconfig %s up", ifname);
    fibo_system(shell_cmd);

    //for bridge mode, only one public IP, so do udhcpc manually
    if (fibo_bridge_mode_detect(profile)) {
        return;
    }

/* Do DHCP using busybox tools */
    {
        char udhcpc_cmd[128];
        pthread_attr_t udhcpc_thread_attr;
        pthread_t udhcpc_thread_id;

        pthread_attr_init(&udhcpc_thread_attr);
        pthread_attr_setdetachstate(&udhcpc_thread_attr, PTHREAD_CREATE_DETACHED);

        if (profile->ipv4.Address) 
        {
             char v4add_str[32] = {0};
             char v4gw_str[32] = {0};
             char v4_netmask_str[32] = {0};
             uint32_t Address = ntohl(profile->ipv4.Address);
             uint32_t Gateway = ntohl(profile->ipv4.Gateway);
             uint32_t SubnetMask = ntohl(profile->ipv4.SubnetMask);
             inet_ntop(AF_INET, &Address, v4add_str, sizeof(v4add_str));
             inet_ntop(AF_INET, &Gateway, v4gw_str, sizeof(v4gw_str));
             inet_ntop(AF_INET, &SubnetMask, v4_netmask_str, sizeof(v4_netmask_str));
            
            if (profile->muxid == 0x81)
            {
                snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ifconfig %s %s netmask %s", ifname, v4add_str, v4_netmask_str);
                fibo_system(shell_cmd);
            
                snprintf(shell_cmd, sizeof(shell_cmd) - 1, "route add default gw %s dev %s", v4gw_str, ifname);
                fibo_system(shell_cmd);
            }
            else if (profile->muxid > 0x81)
            {
                snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ip link add link %s name %s type vlan id %d", ifname, sub_intf_name, intf_id);
                fibo_system(shell_cmd);
            
                snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ip link set %s up", ifname);
                fibo_system(shell_cmd);
            
                snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ip link set %s up", sub_intf_name);
                fibo_system(shell_cmd);
            
                snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ifconfig %s %s netmask %s", sub_intf_name, v4add_str, v4_netmask_str);
                fibo_system(shell_cmd);
            }
        }
        
        if (profile->ipv6.PrefixLengthIPAddr) 
        {
    		//module do not support DHCPv6, only support 'Router Solicit'
    		//and it seem if enable /proc/sys/net/ipv6/conf/all/forwarding, Kernel do not send RS
    		const char *forward_file = "/proc/sys/net/ipv6/conf/all/forwarding";
    		int forward_fd = open(forward_file, O_RDONLY);
    		if (forward_fd > 0) {
    			char forward_state[2];
    			read(forward_fd, forward_state, 2);
    			if (forward_state[0] == '1') {
    				dbg_time("%s enabled, kernel maybe donot send 'Router Solicit'", forward_file);
    			}
    			close(forward_fd);
    		}

             {
                   char v6add_str[100] = {0};
                   char v6gw_str[100] = {0};
                   inet_ntop(AF_INET6, profile->ipv6.Address, v6add_str, sizeof(v6add_str));
                   inet_ntop(AF_INET6, profile->ipv6.Gateway, v6gw_str, sizeof(v6gw_str));

                   if (profile->muxid == 0x81)
                   {
                       snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ifconfig %s add %s/%d", ifname, v6add_str, profile->ipv6.PrefixLengthIPAddr);
                       fibo_system(shell_cmd);

                       snprintf(shell_cmd, sizeof(shell_cmd) - 1, "route -A inet6 add ::/0 gw %s dev %s", v6gw_str, ifname);
                       fibo_system(shell_cmd);
                   }
                   else if (profile->muxid > 0x81)
                   {
                       snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ip link add link %s name %s type vlan id %d", ifname, sub_intf_name, intf_id);
                       fibo_system(shell_cmd);

                       snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ip link set %s up", ifname);
                       fibo_system(shell_cmd);

                       snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ip link set %s up", sub_intf_name);
                       fibo_system(shell_cmd);

                       snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ifconfig %s add %s/%d", sub_intf_name, v6add_str, profile->ipv6.PrefixLengthIPAddr);
                       fibo_system(shell_cmd);
                       /* start 2021-01-21 add by haopengfei to fix mantis 69056 */
                       snprintf(shell_cmd, sizeof(shell_cmd) - 1, "route -A inet6 add ::/0 gw %s dev %s", v6gw_str, sub_intf_name);
                       fibo_system(shell_cmd);
                       /* end 2021-01-21 add by haopengfei to fix mantis 69056 */
                   }
             }

        }
    }
}

void udhcpc_stop_pcie(PROFILE_T *profile) {
    char *ifname = profile->usbnet_adapter;
    char shell_cmd[128];
    char reset_ip[128];
    dbg_time("enter %s ", __func__);

    if (profile->qmapnet_adapter) {
        ifname = profile->qmapnet_adapter;
    }

    if (profile->muxid == 0x81)
    {
        snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ifconfig %s down", ifname);
        fibo_system(shell_cmd);
        snprintf(reset_ip, sizeof(reset_ip) - 1, "ifconfig %s 0.0.0.0", ifname);
        fibo_system(reset_ip);
    }
    else if (profile->muxid > 0x81)
    {
       char sub_intf_name[100];
       int intf_id = profile->muxid - 0x81;
       snprintf(sub_intf_name, sizeof(sub_intf_name) - 1, "%s.%d", ifname, intf_id);

       snprintf(shell_cmd, sizeof(shell_cmd) - 1, "ip link del link dev %s", sub_intf_name);
       fibo_system(shell_cmd);
    }
}

