/* This example is placed in the public domain. */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <net/if.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include <linux/if_link.h>
#include <linux/if_ether.h>
#include <linux/rtnetlink.h>
//#include <linux/if.h>

#include "libmnl.h"
#include "ifutils.h"

#define ERRMSG(v...) printf("%s-%d: error=%s %s\n", __func__, __LINE__, strerror(errno), ##v)
extern void dbg_time(const char *fmt, ...);
int mask_to_prefix_v4(uint32_t mask)
{
    int ret = 0;
    while (mask)
    {
        mask = mask & (mask - 1);
        ret++;
    }
    return ret;
}

const char *ipaddr_to_string_v4(in_addr_t ipaddr)
{
    static char buf[INET6_ADDRSTRLEN] = {'\0'};
    buf[0] = '\0';
    uint32_t addr = ipaddr;
    return inet_ntop(AF_INET, &addr, buf, sizeof(buf));
}

const char *ipaddr_to_string_v6(uint8_t *ipaddr)
{
    static char buf[INET6_ADDRSTRLEN] = {'\0'};
    buf[0] = '\0';
    return inet_ntop(AF_INET6, ipaddr, buf, sizeof(buf));
}

static void ifc_init_ifr(const char *name, struct ifreq *ifr)
{
    memset(ifr, 0, sizeof(struct ifreq));
    strncpy(ifr->ifr_name, name, IFNAMSIZ);
    ifr->ifr_name[IFNAMSIZ - 1] = 0;
}

int if_get_hwaddr(const char *name, void *ptr)
{
    int r;
    struct ifreq ifr;
    ifc_init_ifr(name, &ifr);

    int ifc_ctl_sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (ifc_ctl_sock < 0)
    {
        return -1;
    }
    r = ioctl(ifc_ctl_sock, SIOCGIFHWADDR, &ifr);
    if (r < 0)
        return -1;

    memcpy(ptr, &ifr.ifr_hwaddr.sa_data, ETH_ALEN);
    return 0;
}

static int if_act_on_link(const char *ifname, int state)
{
    struct mnl_socket *nl;
    char buf[MNL_SOCKET_BUFFER_SIZE];
    struct nlmsghdr *nlh;
    struct ifinfomsg *ifm;
    int ret;
    unsigned int seq, portid, change = 0, flags = 0;
    static int oldstate = -1;

    if (state == oldstate)
        return 0;
    oldstate = state;

    if (state)
    {
        change |= IFF_UP;
        flags |= IFF_UP;
    }
    else
    {
        change |= IFF_UP;
        flags &= ~IFF_UP;
    }

    nlh = mnl_nlmsg_put_header(buf);
    nlh->nlmsg_type = RTM_NEWLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq = seq = time(NULL);
    ifm = mnl_nlmsg_put_extra_header(nlh, sizeof(*ifm));
    ifm->ifi_family = AF_UNSPEC;
    ifm->ifi_change = change;
    ifm->ifi_flags = flags;

    mnl_attr_put_str(nlh, IFLA_IFNAME, ifname);

    nl = mnl_socket_open(NETLINK_ROUTE);
    if (nl == NULL)
    {
        ERRMSG("mnl_socket_open");
        return -1;
    }

    if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0)
    {
        ERRMSG(" mnl_socket_bind");
        return -1;
    }
    portid = mnl_socket_get_portid(nl);

    if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0)
    {
        ERRMSG(" mnl_socket_sendto");
        return -1;
    }

    ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
    if (ret == -1)
    {
        ERRMSG(" mnl_socket_recvfrom");
        return -1;
    }

    ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
    if (ret == -1)
    {
        ERRMSG(" mnl_cb_run");
        return -1;
    }

    mnl_socket_close(nl);

    return 0;
}

int if_link_up(const char *ifname)
{
    dbg_time("if_link_up %s",ifname);
    return if_act_on_link(ifname, 1);
}

int if_link_down(const char *ifname)
{

    dbg_time("if_link_down %s",ifname);
    return if_act_on_link(ifname, 0);
}

int if_set_mtu(const char *ifname, uint32_t mtu)
{
    char buf[MNL_SOCKET_BUFFER_SIZE];
    unsigned int seq, portid;
    struct mnl_socket *nl;
    struct nlmsghdr *nlh;
    struct ifinfomsg *ifm;
    int ret;
    int iface;
    static uint32_t oldmtu = 1500;

    if (mtu == oldmtu)
        return 0;
    oldmtu = mtu;

    iface = if_nametoindex(ifname);
    if (iface == 0)
    {
        ERRMSG(" if_nametoindex");
        return -1;
    }

    nlh = mnl_nlmsg_put_header(buf);
    nlh->nlmsg_type = RTM_NEWLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq = seq = time(NULL);
    ifm = mnl_nlmsg_put_extra_header(nlh, sizeof(struct ifinfomsg));
    ifm->ifi_family = AF_UNSPEC;
    ifm->ifi_index = iface;
    ifm->ifi_change = 0xFFFFFFFF;
    ifm->ifi_type = 0;
	ifm->ifi_flags = IFF_NOARP | IFF_MULTICAST;

    mnl_attr_put_u32(nlh, IFLA_MTU, mtu);

    nl = mnl_socket_open(NETLINK_ROUTE);
    if (nl == NULL)
    {
        ERRMSG(" mnl_socket_open");
        return -1;
    }

    if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0)
    {
        ERRMSG(" mnl_socket_bind");
        return -1;
    }
    portid = mnl_socket_get_portid(nl);

    if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0)
    {
        ERRMSG(" mnl_socket_sendto");
        return -1;
    }

    ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
    if (ret == -1)
    {
        ERRMSG(" mnl_socket_recvfrom");
        return -1;
    }

    ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
    if (ret == -1)
    {
        ERRMSG(" mnl_cb_run");
        return -1;
    }

    mnl_socket_close(nl);

    return 0;
}

/**
 * @brief Set the ip addr object
 * 
 * @param operate 
 *  0 -> add address on interface
 *  1 -> delete address on interface
 * @param ifname 
 * @param ipaddr 
 * @param prefix 
 * @return int 
 */
static int if_act_on_addr(bool operate, int proto, const char *ifname, addr_t *ipaddr, uint32_t prefix)
{
    struct mnl_socket *nl;
    char buf[MNL_SOCKET_BUFFER_SIZE];
    struct nlmsghdr *nlh;
    struct ifaddrmsg *ifm;
    uint32_t seq, portid;
    int ret, family = proto;

    int iface;

    iface = if_nametoindex(ifname);
    if (iface == 0)
    {
        ERRMSG(" if_nametoindex");
        return -1;
    }

    nlh = mnl_nlmsg_put_header(buf);
    if (operate)
        nlh->nlmsg_type = RTM_NEWADDR;
    else
        nlh->nlmsg_type = RTM_DELADDR;

    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_REPLACE | NLM_F_ACK;
    nlh->nlmsg_seq = seq = time(NULL);

    ifm = mnl_nlmsg_put_extra_header(nlh, sizeof(struct ifaddrmsg));

    ifm->ifa_family = family;
    ifm->ifa_prefixlen = prefix;
    ifm->ifa_flags = IFA_F_PERMANENT;

    ifm->ifa_scope = RT_SCOPE_UNIVERSE;
    ifm->ifa_index = iface;

    /*
	 * The exact meaning of IFA_LOCAL and IFA_ADDRESS depend
	 * on the address family being used and the device type.
	 * For broadcast devices (like the interfaces we use),
	 * for IPv4 we specify both and they are used interchangeably.
	 * For IPv6, only IFA_ADDRESS needs to be set.
	 */
    if (family == AF_INET)
    {
        mnl_attr_put_u32(nlh, IFA_LOCAL, ipaddr->ip);
        mnl_attr_put_u32(nlh, IFA_ADDRESS, ipaddr->ip);
    }
    else
    {
        mnl_attr_put(nlh, IFA_ADDRESS, sizeof(struct in6_addr), ipaddr);
    }

    nl = mnl_socket_open(NETLINK_ROUTE);
    if (nl == NULL)
    {
        ERRMSG(" mnl_socket_open");
        return -1;
    }

    if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0)
    {
        ERRMSG(" mnl_socket_bind");
        return -1;
    }
    portid = mnl_socket_get_portid(nl);

    if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0)
    {
        ERRMSG(" mnl_socket_sendto");
        return -1;
    }

    ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
    if (ret < 0)
    {
        ERRMSG(" mnl_socket_recvfrom");
        return -1;
    }

    ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
    if (ret < 0)
    {
        ERRMSG(" mnl_cb_run");
        return -1;
    }

    mnl_socket_close(nl);

    return 0;
}

int if_set_addr_v4(const char *ifname, in_addr_t ipaddr, uint32_t prefix)
{
    addr_t addr;
    addr.ip = ipaddr;
    return if_act_on_addr(1, AF_INET, ifname, &addr, prefix);
}

int if_del_addr_v4(const char *ifname, in_addr_t ipaddr, uint32_t prefix)
{
    addr_t addr;
    addr.ip = ipaddr;
    return if_act_on_addr(0, AF_INET, ifname, &addr, prefix);
}

int if_set_addr_v6(const char *ifname, uint8_t *ipaddr, uint32_t prefix)
{
    addr_t addr;
    memcpy(&addr.ip6, ipaddr, 16);
    return if_act_on_addr(1, AF_INET6, ifname, &addr, prefix);
}

int if_del_addr_v6(const char *ifname, uint8_t *ipaddr, uint32_t prefix)
{
    addr_t addr;
    memcpy(&addr.ip6, ipaddr, 16);
    return if_act_on_addr(0, AF_INET6, ifname, &addr, prefix);
}

static int data_attr_cb(const struct nlattr *attr, void *data)
{
    const struct nlattr **tb = data;
    int type = mnl_attr_get_type(attr);

    /* skip unsupported attribute in user-space */
    if (mnl_attr_type_valid(attr, IFA_MAX) < 0)
        return MNL_CB_OK;

    switch (type)
    {
    case IFA_ADDRESS:
        if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0)
        {
            ERRMSG(" mnl_attr_validate");
            return MNL_CB_ERROR;
        }
        break;
    }
    tb[type] = attr;
    return MNL_CB_OK;
}

static int data_cb(const struct nlmsghdr *nlh, void *data)
{
    struct nlattr *tb[IFA_MAX + 1] = {};
    struct ifaddrmsg *ifa = mnl_nlmsg_get_payload(nlh);
    struct addrinfo_t *addrinfo = (struct addrinfo_t *)data;
    void *addr = NULL;

    mnl_attr_parse(nlh, sizeof(*ifa), data_attr_cb, tb);
    if (tb[IFA_ADDRESS])
    {
        char out[INET6_ADDRSTRLEN];

        addr = mnl_attr_get_payload(tb[IFLA_ADDRESS]);
        addr = mnl_attr_get_payload(tb[IFA_ADDRESS]);
        if (!inet_ntop(ifa->ifa_family, addr, out, sizeof(out)))
            ERRMSG("inet_ntop");
        // printf("%d %d-> %d %s\n", addrinfo->iface, ifa->ifa_index, ifa->ifa_scope, out);

        addrinfo->addrs[addrinfo->num].prefix = ifa->ifa_prefixlen;
        if (ifa->ifa_index == addrinfo->iface)
        {
            if (ifa->ifa_family == AF_INET6)
                memcpy(addrinfo->addrs[addrinfo->num].address.ip6.s6_addr, addr, 16);
            if (ifa->ifa_family == AF_INET)
                memcpy(&(addrinfo->addrs[addrinfo->num].address.ip), addr, 4);
            addrinfo->num++;
        }
    }

    // ifa->ifa_scope
    // 0: global
    // 200: site
    // 253: link
    // 254: host
    // 255: nowhere

    return MNL_CB_OK;
}

/**
 * @brief 
 * 
 * @param ifname 
 * @param proto
 *  AF_INET -> for IPv4
 *  AF_INET6 -> for IPv6
 * @return int 
 */
static int if_get_addr(const char *ifname, int proto, struct addrinfo_t *addrinfo)
{
    char buf[MNL_SOCKET_BUFFER_SIZE];
    unsigned int seq, portid;
    struct mnl_socket *nl;
    struct nlmsghdr *nlh;
    struct rtgenmsg *rt;
    int ret;

    addrinfo->iface = if_nametoindex(ifname);
    if (addrinfo->iface == 0)
    {
        ERRMSG(" if_nametoindex");
        return -1;
    }

    nlh = mnl_nlmsg_put_header(buf);
    nlh->nlmsg_type = RTM_GETADDR;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq = seq = time(NULL);
    rt = mnl_nlmsg_put_extra_header(nlh, sizeof(struct rtgenmsg));
    if (proto == AF_INET)
        rt->rtgen_family = AF_INET;
    else if (proto == AF_INET6)
        rt->rtgen_family = AF_INET6;

    nl = mnl_socket_open(NETLINK_ROUTE);
    if (nl == NULL)
    {
        ERRMSG(" mnl_socket_open");
        return -1;
    }

    if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0)
    {
        ERRMSG(" mnl_socket_bind");
        return -1;
    }
    portid = mnl_socket_get_portid(nl);

    if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0)
    {
        ERRMSG(" mnl_socket_sendto");
        return -1;
    }

    ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
    while (ret > 0)
    {
        ret = mnl_cb_run(buf, ret, seq, portid, data_cb, addrinfo);
        if (ret <= MNL_CB_STOP)
            break;
        ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
    }
    if (ret == -1)
    {
        ERRMSG(" error");
        return -1;
    }
    mnl_socket_close(nl);

    return 0;
}

int if_flush_v4_addr(const char *ifname)
{
    struct addrinfo_t addrinfo;
    int i = 0;
    
    memset(&addrinfo, 0, sizeof(struct addrinfo_t));
    if_get_addr(ifname, AF_INET, &addrinfo);
    for (; i < addrinfo.num; i++)
    {
        // printf("remove address: %s\n", ipaddr_to_string_v4(addrinfo.addrs[i].address.ip));
        if_del_addr_v4(ifname, addrinfo.addrs[i].address.ip, addrinfo.addrs[i].prefix);
    }
    return 0;
}

int if_flush_v6_addr(const char *ifname)
{
    struct addrinfo_t addrinfo;
    int i = 0;
    
    memset(&addrinfo, 0, sizeof(struct addrinfo_t));
    if_get_addr(ifname, AF_INET6, &addrinfo);
    for (; i < addrinfo.num; i++)
    {
        // printf("remove address: %s\n", ipaddr_to_string_v6(addrinfo.addrs[i].address.ip6.s6_addr));
        if_del_addr_v6(ifname, addrinfo.addrs[i].address.ip6.s6_addr, addrinfo.addrs[i].prefix);
    }
    return 0;
}

/**
 * @brief Set the route addr object
 *   Usage: 
 *      iface destination cidr [gateway]
 *   Example:
 *      eth0 10.0.1.12 32 10.0.1.11
 *      eth0 ffff::10.0.1.12 128 fdff::1
 * @param operate
 *  add or del
 * @param ifname 
 * @param dstaddr 
 * @param prefix 
 * @param gwaddr 
 * @return int 
 */
int if_act_on_route(bool operate, int proto, const char *ifname, addr_t *dstaddr, uint32_t prefix, addr_t *gwaddr)
{
    struct mnl_socket *nl;
    char buf[MNL_SOCKET_BUFFER_SIZE];
    struct nlmsghdr *nlh;
    struct rtmsg *rtm;
    uint32_t seq, portid;
    int iface, ret, family = proto;

    iface = if_nametoindex(ifname);
    if (iface == 0)
    {
        ERRMSG(" if_nametoindex");
        return -1;
    }

    nlh = mnl_nlmsg_put_header(buf);
    if (operate)
        nlh->nlmsg_type = RTM_NEWROUTE;
    else
        nlh->nlmsg_type = RTM_DELROUTE;

    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
    nlh->nlmsg_seq = seq = time(NULL);

    rtm = mnl_nlmsg_put_extra_header(nlh, sizeof(struct rtmsg));
    rtm->rtm_family = family;
    rtm->rtm_dst_len = prefix;
    rtm->rtm_src_len = 0;
    rtm->rtm_tos = 0;
    rtm->rtm_protocol = RTPROT_STATIC;
    rtm->rtm_table = RT_TABLE_MAIN;
    rtm->rtm_type = RTN_UNICAST;
    /* is there any gateway? */
    rtm->rtm_scope = gwaddr ? RT_SCOPE_UNIVERSE : RT_SCOPE_LINK;
    rtm->rtm_flags = 0;

    if (family == AF_INET)
        mnl_attr_put_u32(nlh, RTA_DST, dstaddr->ip);
    else
        mnl_attr_put(nlh, RTA_DST, sizeof(struct in6_addr), dstaddr);

    mnl_attr_put_u32(nlh, RTA_OIF, iface);
    if (gwaddr)
    {
        if (family == AF_INET)
            mnl_attr_put_u32(nlh, RTA_GATEWAY, gwaddr->ip);
        else
        {
            mnl_attr_put(nlh, RTA_GATEWAY, sizeof(struct in6_addr), gwaddr);
        }
    }

    nl = mnl_socket_open(NETLINK_ROUTE);
    if (nl == NULL)
    {
        ERRMSG(" mnl_socket_open");
        return -1;
    }

    if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0)
    {
        ERRMSG(" mnl_socket_bind");
        return -1;
    }
    portid = mnl_socket_get_portid(nl);

    if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0)
    {
        ERRMSG(" mnl_socket_sendto");
        return -1;
    }

    ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
    if (ret < 0)
    {
        ERRMSG(" mnl_socket_recvfrom");
        return -1;
    }

    ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
    if (ret < 0)
    {
        ERRMSG(" mnl_cb_run");
        return -1;
    }

    mnl_socket_close(nl);

    return 0;
}

int if_set_default_route_v4(const char *ifname)
{
    return if_act_on_route(1, AF_INET, ifname, (addr_t *)&in6addr_any, 0, NULL);
}

int if_del_default_route_v4(const char *ifname)
{
    return if_act_on_route(0, AF_INET, ifname, (addr_t *)&in6addr_any, 0, NULL);
}

int if_set_default_route_v6(const char *ifname)
{
    return if_act_on_route(1, AF_INET6, ifname, (addr_t *)&in6addr_any, 0, NULL);
}

int if_del_default_route_v6(const char *ifname)
{
    return if_act_on_route(0, AF_INET6, ifname, (addr_t *)&in6addr_any, 0, NULL);
}

/**
 * @brief Set the default gwaddr object
 *  set default gw
 * @param operate 
 * @param ifname 
 * @param gwaddr 
 *  gateway ip
 * @return int 
 */
int if_set_route_gw_v4(const char *ifname, in_addr_t gwaddr)
{
    addr_t addr;
    memset(&addr, 0, sizeof(addr_t));
    addr.ip = gwaddr;
    return if_act_on_route(1, AF_INET, ifname, (addr_t *)&in6addr_any, 0, &addr);
}

int if_del_route_gw_v4(const char *ifname, in_addr_t gwaddr)
{
    addr_t addr;
    memset(&addr, 0, sizeof(addr_t));
    addr.ip = gwaddr;
    return if_act_on_route(0, AF_INET, ifname, (addr_t *)&in6addr_any, 0, &addr);
}

int if_set_route_gw_v6(const char *ifname, uint8_t *gwaddr)
{
    addr_t addr;
    memset(&addr, 0, sizeof(addr_t));
    memcpy(&addr.ip6, gwaddr, 16);
    return if_act_on_route(1, AF_INET6, ifname, (addr_t *)&in6addr_any, 0, &addr);
}

int if_del_route_gw_v6(const char *ifname, uint8_t *gwaddr)
{
    addr_t addr;
    memset(&addr, 0, sizeof(addr_t));
    memcpy(&addr.ip6, gwaddr, 16);
    return if_act_on_route(0, AF_INET6, ifname, (addr_t *)&in6addr_any, 0, &addr);
}

int if_set_dns(const char *dns1, const char *dns2)
{
    int ret = 0;
    char buf[128] = {'\0'};
    int fd = open("/etc/resolv.conf", O_CREAT | O_WRONLY | O_TRUNC);
    if (fd < 0)
    {
        ERRMSG(" fail to open /etc/resolv.conf");
        return -1;
    }

    if (dns1)
        snprintf(buf, sizeof(buf), "nameserver %s\n", dns1);
    if (dns2)
        snprintf(buf, sizeof(buf), "nameserver %s\n", dns2);
    ret = write(fd, buf, strlen(buf));
    if (ret < 0)
    {
        ERRMSG(" write dns");
    }
    close(fd);
    return ret > 0 ? 0 : -1;
}

int if_set_network_v4(const char *ifname, in_addr_t ipaddr, uint32_t prefix,
                      in_addr_t gwaddr, in_addr_t dns1, in_addr_t dns2)
{
    if_link_up(ifname);
    sleep(2);
    if_set_addr_v4(ifname, ipaddr, prefix);
    if_set_default_route_v4(ifname);
    if_set_dns(ipaddr_to_string_v4(dns1), ipaddr_to_string_v4(dns2));
    return 0;
}

int if_set_network_v6(const char *ifname, uint8_t *ipaddr, uint32_t prefix,
                      uint8_t *gwaddr, uint8_t *dns1, uint8_t *dns2)
{
    if_link_up(ifname);
    sleep(2);
    if_set_addr_v6(ifname, ipaddr, prefix);
    if_set_default_route_v6(ifname);
    if_set_dns(ipaddr_to_string_v6(dns1), ipaddr_to_string_v6(dns2));
    return 0;
}
