#ifdef ANDROID
/*
 * Copyright 2008, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include "QMIThread.h"
#ifdef USE_NDK
extern int (*ifc_init)(void);
extern void (*ifc_close)(void);
extern int (*do_dhcp)(const char *iname);
extern void (*get_dhcp_info)(uint32_t *ipaddr, uint32_t *gateway, uint32_t *prefixLength,
                   uint32_t *dns1, uint32_t *dns2, uint32_t *server,
                   uint32_t *lease);  
extern int (*property_set)(const char *key, const char *value);
#else
#include <cutils/properties.h>
#include <netutils/ifc.h>
extern int do_dhcp(const char *iname);
extern void get_dhcp_info(uint32_t *ipaddr, uint32_t *gateway, uint32_t *prefixLength,
                   uint32_t *dns1, uint32_t *dns2, uint32_t *server,
                   uint32_t *lease);  
#endif

static const char *ipaddr_to_string(in_addr_t addr)
{
    struct in_addr in_addr;

    in_addr.s_addr = addr;
    return inet_ntoa(in_addr);
}

void do_dhcp_request(PROFILE_T *profile) {
#ifdef USE_NDK
    if (!ifc_init ||!ifc_close ||!do_dhcp || !get_dhcp_info || !property_set) {
        return;
    }    
#endif

    char *ifname = profile->usbnet_adapter;
    uint32_t ipaddr, gateway, prefixLength, dns1, dns2, server, lease;
    char propKey[128];

#if 0
    if (profile->rawIP && ((profile->IPType==0x04 && profile->ipv4.Address)))
    {
        snprintf(propKey, sizeof(propKey), "net.%s.dns1", ifname);
        property_set(propKey, profile->ipv4.DnsPrimary ? ipaddr_to_string(ql_swap32(profile->ipv4.DnsPrimary)) : "8.8.8.8");
        snprintf(propKey, sizeof(propKey), "net.%s.dns2", ifname);
        property_set(propKey, profile->ipv4.DnsSecondary ? ipaddr_to_string(ql_swap32(profile->ipv4.DnsSecondary)) : "8.8.8.8");
        snprintf(propKey, sizeof(propKey), "net.%s.gw", ifname);
        property_set(propKey, profile->ipv4.Gateway ? ipaddr_to_string(ql_swap32(profile->ipv4.Gateway)) : "0.0.0.0");
        return;
    }
#endif

    if(ifc_init()) {
        dbg_time("failed to ifc_init(%s): %s\n", ifname, strerror(errno));
    }

    if (do_dhcp(ifname) < 0) {
        dbg_time("failed to do_dhcp(%s): %s\n", ifname, strerror(errno));
    }

    ifc_close();

    get_dhcp_info(&ipaddr,  &gateway,  &prefixLength, &dns1, &dns2, &server, &lease);
    snprintf(propKey, sizeof(propKey), "net.%s.gw", ifname);
    property_set(propKey, gateway ? ipaddr_to_string(gateway) : "0.0.0.0");
}
#endif
