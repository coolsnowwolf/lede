/******************************************************************************
  @file    udhcpc.c
  @brief   call DHCP tools to obtain IP address.

  DESCRIPTION
  Connectivity Management Tool for USB network adapter of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2023 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <endian.h>

#include "util.h"
#include "QMIThread.h"

#define IFDOWN_SCRIPT "/etc/quectel/ifdown.sh"
#define IFUP_SCRIPT "/etc/quectel/ifup.sh"

static int ql_system(const char *shell_cmd)
{
    dbg_time("%s", shell_cmd);
    return system(shell_cmd);
}

uint32_t mask_to_prefix_v4(uint32_t mask)
{
    uint32_t prefix = 0;
    while (mask)
    {
        mask = mask & (mask - 1);
        prefix++;
    }
    return prefix;
}

uint32_t mask_from_prefix_v4(uint32_t prefix)
{
    return ~((1 << (32 - prefix)) - 1);
}

/* mask in int */
uint32_t broadcast_from_mask(uint32_t ip, uint32_t mask)
{
    return (ip & mask) | (~mask);
}

const char *ipaddr_to_string_v4(in_addr_t ipaddr, char *buf, size_t size)
{
    //    static char buf[INET6_ADDRSTRLEN] = {'\0'};
    buf[0] = '\0';
    uint32_t addr = ipaddr;
    return inet_ntop(AF_INET, &addr, buf, size);
}

const char *ipaddr_to_string_v6(uint8_t *ipaddr, char *buf, size_t size)
{
    buf[0] = '\0';
    return inet_ntop(AF_INET6, ipaddr, buf, size);
}

/**
 * For more details see default.script
 * 
 * The main aim of this function is offload ip management to script, CM has not interest in manage IP address.
 * just tell script all the info about ip, mask, router, dns...
 */
void udhcpc_start(PROFILE_T *profile)
{
    char shell_cmd[1024];
    char ip[128];
    char subnet[128];
    char broadcast[128];
    char router[128];
    char domain1[128];
    char domain2[128];

    if (NULL == getenv(IFUP_SCRIPT))
        return;

    // manage IPv4???
    // check rawip ???
    snprintf(shell_cmd, sizeof(shell_cmd),
             " netiface=%s interface=%s mtu=%u ip=%s subnet=%s broadcast=%s router=%s"
             " domain=\"%s %s\" %s",
             profile->usbnet_adapter,
             profile->qmapnet_adapter ? profile->qmapnet_adapter : profile->usbnet_adapter,
             profile->ipv4.Mtu,
             ipaddr_to_string_v4(ntohl(profile->ipv4.Address), ip, sizeof(ip)),
             ipaddr_to_string_v4(ntohl(profile->ipv4.SubnetMask), subnet, sizeof(subnet)),
             ipaddr_to_string_v4(ntohl(broadcast_from_mask(profile->ipv4.Address, profile->ipv4.SubnetMask)),
                                 broadcast, sizeof(broadcast)),
             ipaddr_to_string_v4(ntohl(profile->ipv4.Gateway), router, sizeof(router)),
             ipaddr_to_string_v4(ntohl(profile->ipv4.DnsPrimary), domain1, sizeof(domain1)),
             ipaddr_to_string_v4(ntohl(profile->ipv4.DnsSecondary), domain2, sizeof(domain2)),
             getenv(IFUP_SCRIPT));
    ql_system(shell_cmd);

    // manage IPv6???
}

/**
 * For more details see default.script
 * 
 * The main aim of this function is offload ip management to script, CM has not interest in manage IP address.
 * just tell script all the info about ip, mask, router, dns...
 */
void udhcpc_stop(PROFILE_T *profile)
{
    char shell_cmd[1024];

    if (NULL == getenv(IFDOWN_SCRIPT))
        return;

    snprintf(shell_cmd, sizeof(shell_cmd),
             "netiface=%s interface=%s %s",
             profile->usbnet_adapter,
             profile->qmapnet_adapter ? profile->qmapnet_adapter : profile->usbnet_adapter,
             getenv(IFDOWN_SCRIPT));
    ql_system(shell_cmd);
}
