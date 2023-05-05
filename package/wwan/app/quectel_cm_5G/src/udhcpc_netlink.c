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

static int ql_raw_ip_mode_check(const char *ifname)
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

void ql_set_driver_link_state(PROFILE_T *profile, int link_state)
{
    char link_file[128];
    int fd;
    int new_state = 0;

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
            if_link_down(profile->usbnet_adapter);
        }
    }

    close(fd);
}

void udhcpc_start(PROFILE_T *profile)
{
    char *ifname = profile->usbnet_adapter;

    ql_set_driver_link_state(profile, 1);
    ql_raw_ip_mode_check(ifname);

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
        if_link_up(profile->usbnet_adapter);
    }

    if_link_up(ifname);

#if 1 //for bridge mode, only one public IP, so do udhcpc manually
    if (ql_bridge_mode_detect(profile))
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

    if (profile->ipv6.Address[0] && profile->ipv6.PrefixLengthIPAddr)
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
        dbg_time("IPv6 Netmask: %d", profile->ipv6.PrefixLengthIPAddr);
        dbg_time("IPv6 Gateway: %s", ipaddr_to_string_v6(profile->ipv6.Gateway));
        dbg_time("IPv6 DNS1: %s", ipaddr_to_string_v6(profile->ipv6.DnsPrimary));
        dbg_time("IPv6 DNS2: %s", ipaddr_to_string_v6(profile->ipv6.DnsSecondary));
        if_set_network_v6(ifname, profile->ipv6.Address, profile->ipv6.PrefixLengthIPAddr,
                       profile->ipv6.Gateway, profile->ipv6.DnsPrimary, profile->ipv6.DnsSecondary);
    }
}

void udhcpc_stop(PROFILE_T *profile)
{
    char *ifname = profile->usbnet_adapter;

    ql_set_driver_link_state(profile, 0);

    if (profile->qmapnet_adapter)
    {
        ifname = profile->qmapnet_adapter;
    }

    if_link_down(ifname);
    if_flush_v4_addr(ifname);
    if_flush_v6_addr(ifname);
}
