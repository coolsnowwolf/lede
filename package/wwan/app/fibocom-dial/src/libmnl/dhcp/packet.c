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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <unistd.h>
#include <stdio.h>

#include "dhcpmsg.h"

int fatal();

int open_raw_socket(const char *ifname __attribute__((unused)), uint8_t *hwaddr, int if_index)
{
    int s;
    struct sockaddr_ll bindaddr;

    if((s = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
        return fatal("socket(PF_PACKET)");
    }

    memset(&bindaddr, 0, sizeof(bindaddr));
    bindaddr.sll_family = AF_PACKET;
    bindaddr.sll_protocol = htons(ETH_P_IP);
    bindaddr.sll_halen = ETH_ALEN;
    memcpy(bindaddr.sll_addr, hwaddr, ETH_ALEN);
    bindaddr.sll_ifindex = if_index;

    if (bind(s, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) < 0) {
        return fatal("Cannot bind raw socket to interface");
    }

    return s;
}

static uint32_t checksum(void *buffer, unsigned int count, uint32_t startsum)
{
    uint16_t *up = (uint16_t *)buffer;
    uint32_t sum = startsum;
    uint32_t upper16;

    while (count > 1) {
        sum += *up++;
        count -= 2;
    }
    if (count > 0) {
        sum += (uint16_t) *(uint8_t *)up;
    }
    while ((upper16 = (sum >> 16)) != 0) {
        sum = (sum & 0xffff) + upper16;
    }
    return sum;
}

static uint32_t finish_sum(uint32_t sum)
{
    return ~sum & 0xffff;
}

int send_packet(int s, int if_index, struct dhcp_msg *msg, int size,
                uint32_t saddr, uint32_t daddr, uint32_t sport, uint32_t dport)
{
    struct iphdr ip;
    struct udphdr udp;
    struct iovec iov[3];
    uint32_t udpsum;
    uint16_t temp;
    struct msghdr msghdr;
    struct sockaddr_ll destaddr;

    ip.version = IPVERSION;
    ip.ihl = sizeof(ip) >> 2;
    ip.tos = 0;
    ip.tot_len = htons(sizeof(ip) + sizeof(udp) + size);
    ip.id = 0;
    ip.frag_off = 0;
    ip.ttl = IPDEFTTL;
    ip.protocol = IPPROTO_UDP;
    ip.check = 0;
    ip.saddr = saddr;
    ip.daddr = daddr;
    ip.check = finish_sum(checksum(&ip, sizeof(ip), 0));

    udp.source = htons(sport);
    udp.dest = htons(dport);
    udp.len = htons(sizeof(udp) + size);
    udp.check = 0;

    /* Calculate checksum for pseudo header */
    udpsum = checksum(&ip.saddr, sizeof(ip.saddr), 0);
    udpsum = checksum(&ip.daddr, sizeof(ip.daddr), udpsum);
    temp = htons(IPPROTO_UDP);
    udpsum = checksum(&temp, sizeof(temp), udpsum);
    temp = udp.len;
    udpsum = checksum(&temp, sizeof(temp), udpsum);

    /* Add in the checksum for the udp header */
    udpsum = checksum(&udp, sizeof(udp), udpsum);

    /* Add in the checksum for the data */
    udpsum = checksum(msg, size, udpsum);
    udp.check = finish_sum(udpsum);

    iov[0].iov_base = (char *)&ip;
    iov[0].iov_len = sizeof(ip);
    iov[1].iov_base = (char *)&udp;
    iov[1].iov_len = sizeof(udp);
    iov[2].iov_base = (char *)msg;
    iov[2].iov_len = size;
    memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sll_family = AF_PACKET;
    destaddr.sll_protocol = htons(ETH_P_IP);
    destaddr.sll_ifindex = if_index;
    destaddr.sll_halen = ETH_ALEN;
    memcpy(destaddr.sll_addr, "\xff\xff\xff\xff\xff\xff", ETH_ALEN);

    msghdr.msg_name = &destaddr;
    msghdr.msg_namelen = sizeof(destaddr);
    msghdr.msg_iov = iov;
    msghdr.msg_iovlen = sizeof(iov) / sizeof(struct iovec);
    msghdr.msg_flags = 0;
    msghdr.msg_control = 0;
    msghdr.msg_controllen = 0;
    return sendmsg(s, &msghdr, 0);
}

int receive_packet(int s, struct dhcp_msg *msg)
{
    int nread;
    int is_valid;
    struct dhcp_packet {
        struct iphdr ip;
        struct udphdr udp;
        struct dhcp_msg dhcp;
    } packet;
    int dhcp_size;
    uint32_t sum;
    uint16_t temp;
    uint32_t saddr, daddr;

    nread = read(s, &packet, sizeof(packet));
    if (nread < 0) {
        return -1;
    }
    /*
     * The raw packet interface gives us all packets received by the
     * network interface. We need to filter out all packets that are
     * not meant for us.
     */
    is_valid = 0;
    if (nread < (int)(sizeof(struct iphdr) + sizeof(struct udphdr))) {
#if VERBOSE
        ALOGD("Packet is too small (%d) to be a UDP datagram", nread);
#endif
    } else if (packet.ip.version != IPVERSION || packet.ip.ihl != (sizeof(packet.ip) >> 2)) {
#if VERBOSE
        ALOGD("Not a valid IP packet");
#endif
    } else if (nread < ntohs(packet.ip.tot_len)) {
#if VERBOSE
        ALOGD("Packet was truncated (read %d, needed %d)", nread, ntohs(packet.ip.tot_len));
#endif
    } else if (packet.ip.protocol != IPPROTO_UDP) {
#if VERBOSE
        ALOGD("IP protocol (%d) is not UDP", packet.ip.protocol);
#endif
    } else if (packet.udp.dest != htons(PORT_BOOTP_CLIENT)) {
#if VERBOSE
        ALOGD("UDP dest port (%d) is not DHCP client", ntohs(packet.udp.dest));
#endif
    } else {
        is_valid = 1;
    }

    if (!is_valid) {
        return -1;
    }

    /* Seems like it's probably a valid DHCP packet */
    /* validate IP header checksum */
    sum = finish_sum(checksum(&packet.ip, sizeof(packet.ip), 0));
    if (sum != 0) {
        printf("IP header checksum failure (0x%x)\n", packet.ip.check);
        return -1;
    }
    /*
     * Validate the UDP checksum.
     * Since we don't need the IP header anymore, we "borrow" it
     * to construct the pseudo header used in the checksum calculation.
     */
    dhcp_size = ntohs(packet.udp.len) - sizeof(packet.udp);
    /*
     * check validity of dhcp_size.
     * 1) cannot be negative or zero.
     * 2) src buffer contains enough bytes to copy
     * 3) cannot exceed destination buffer
     */
    if ((dhcp_size <= 0) ||
        ((int)(nread - sizeof(struct iphdr) - sizeof(struct udphdr)) < dhcp_size) ||
        ((int)sizeof(struct dhcp_msg) < dhcp_size)) {
#if VERBOSE
        printf("Malformed Packet\n");
#endif
        return -1;
    }
    saddr = packet.ip.saddr;
    daddr = packet.ip.daddr;
    nread = ntohs(packet.ip.tot_len);
    memset(&packet.ip, 0, sizeof(packet.ip));
    packet.ip.saddr = saddr;
    packet.ip.daddr = daddr;
    packet.ip.protocol = IPPROTO_UDP;
    packet.ip.tot_len = packet.udp.len;
    temp = packet.udp.check;
    packet.udp.check = 0;
    sum = finish_sum(checksum(&packet, nread, 0));
    packet.udp.check = temp;
    if (!sum)
        sum = finish_sum(sum);
    if (temp != sum) {
        printf("UDP header checksum failure (0x%x should be 0x%x)\n", sum, temp);
        return -1;
    }
    memcpy(msg, &packet.dhcp, dhcp_size);
    return dhcp_size;
}
