#ifndef __IFUTILS_H__
#define __IFUTILS_H__

typedef union {
    in_addr_t ip;
    struct in6_addr ip6;
} addr_t;

#define MAX_IP_NUM 32
struct addrinfo_t
{
    int iface;
    int num;
    struct
    {
        int prefix;
        addr_t address;
    } addrs[MAX_IP_NUM];
};

const char *ipaddr_to_string_v4(in_addr_t ipaddr);
const char *ipaddr_to_string_v6(uint8_t *ipaddr);
int mask_to_prefix_v4(in_addr_t mask);

int if_get_hwaddr(const char *name, void *ptr);

int if_link_down(const char *ifname);
int if_link_up(const char *ifname);
int if_set_mtu(const char *ifname, uint32_t mtu);

int if_set_addr_v4(const char *name, in_addr_t address, uint32_t prefixlen);
int if_del_addr_v4(const char *name, in_addr_t address, uint32_t prefixlen);
int if_set_addr_v6(const char *name, uint8_t *address, uint32_t prefixlen);
int if_del_addr_v6(const char *name, uint8_t *address, uint32_t prefixlen);
int if_flush_v4_addr(const char *ifname);
int if_flush_v6_addr(const char *ifname);

int if_set_route_gw_v4(const char *ifname, in_addr_t gwaddr);
int if_del_route_gw_v4(const char *ifname, in_addr_t gwaddr);
int if_set_default_route_v4(const char *ifname);
int if_del_default_route_v4(const char *ifname);

int if_set_route_gw_v6(const char *ifname, uint8_t *gwaddr);
int if_del_route_gw_v6(const char *ifname, uint8_t *gwaddr);
int if_set_default_route_v6(const char *ifname);
int if_del_default_route_v6(const char *ifname);

int if_set_network_v4(const char *ifname, in_addr_t ipaddr, uint32_t prefix,
                      in_addr_t gwaddr, in_addr_t dns1, in_addr_t dns2);
int if_set_network_v6(const char *ifname, uint8_t *ipaddr, uint32_t prefix,
                      uint8_t *gwaddr, uint8_t *dns1, uint8_t *dns2);

#endif //__IFUTILS_H__