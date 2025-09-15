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
#include <stdarg.h>
#include <string.h>
#include <netinet/in.h>

#include "dhcpmsg.h"

static void *init_dhcp_msg(dhcp_msg *msg, int type, void *hwaddr, uint32_t xid)
{
    uint8_t *x;

    memset(msg, 0, sizeof(dhcp_msg));

    msg->op = OP_BOOTREQUEST;
    msg->htype = HTYPE_ETHER;
    msg->hlen = 6;
    msg->hops = 0;

    msg->flags = htons(FLAGS_BROADCAST);

    msg->xid = xid;

    memcpy(msg->chaddr, hwaddr, 6);

    x = msg->options;

    *x++ = OPT_COOKIE1;
    *x++ = OPT_COOKIE2;
    *x++ = OPT_COOKIE3;
    *x++ = OPT_COOKIE4;

    *x++ = OPT_MESSAGE_TYPE;
    *x++ = 1;
    *x++ = type;

    return x;
}

int init_dhcp_discover_msg(dhcp_msg *msg, void *hwaddr, uint32_t xid)
{
    uint8_t *x;

    x = init_dhcp_msg(msg, DHCPDISCOVER, hwaddr, xid);

    *x++ = OPT_PARAMETER_LIST;
    *x++ = 4;
    *x++ = OPT_SUBNET_MASK;
    *x++ = OPT_GATEWAY;
    *x++ = OPT_DNS;
    *x++ = OPT_BROADCAST_ADDR;

    *x++ = OPT_END;

    return DHCP_MSG_FIXED_SIZE + (x - msg->options);
}

int init_dhcp_request_msg(dhcp_msg *msg, void *hwaddr, uint32_t xid,
                          uint32_t ipaddr, uint32_t serveraddr)
{
    uint8_t *x;

    x = init_dhcp_msg(msg, DHCPREQUEST, hwaddr, xid);

    *x++ = OPT_PARAMETER_LIST;
    *x++ = 4;
    *x++ = OPT_SUBNET_MASK;
    *x++ = OPT_GATEWAY;
    *x++ = OPT_DNS;
    *x++ = OPT_BROADCAST_ADDR;

    *x++ = OPT_REQUESTED_IP;
    *x++ = 4;
    memcpy(x, &ipaddr, 4);
    x +=  4;

    *x++ = OPT_SERVER_ID;
    *x++ = 4;
    memcpy(x, &serveraddr, 4);
    x += 4;

    *x++ = OPT_END;

    return DHCP_MSG_FIXED_SIZE + (x - msg->options);
}
