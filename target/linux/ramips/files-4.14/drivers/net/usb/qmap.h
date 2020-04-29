/* Copyright (c) 2015, The Linux Foundation. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __QMAP_H
#define __QMAP_H

#include "Structs.h"

#define ETH_TYPE_ARP  0x0806
#define ETH_TYPE_IPV4 0x0800
#define ETH_TYPE_IPV6 0x86DD

typedef struct _QC_ETH_HDR
{
    unsigned char DstMacAddress[ETH_ALEN];
    unsigned char SrcMacAddress[ETH_ALEN];
    unsigned short EtherType;
}__attribute__ ((aligned (1))) QC_ETH_HDR, *PQC_ETH_HDR;

typedef struct _QC_ARP_HDR
{
   unsigned short HardwareType;
   unsigned short ProtocolType;
   unsigned char HLEN;        // length of HA  (6)
   unsigned char PLEN;        // length of IP  (4)
   unsigned short Operation;
   unsigned char SenderHA[ETH_ALEN];  // 6
   unsigned long SenderIP;
   unsigned char TargetHA[ETH_ALEN];  // 6
   unsigned long TargetIP;
}__attribute__ ((aligned (1))) QC_ARP_HDR, *PQC_ARP_HDR;

typedef struct qmap_hdr
{
#if 0
#ifndef LITTLE_ENDIAN
        unsigned char pad_len:6;
        unsigned char reserved_bit:1;
        unsigned char cd_bit:1;
#else
        unsigned char cd_bit:1;
        unsigned char reserved_bit:1;
        unsigned char pad_len:6;
#endif
#endif
   unsigned char cd_rsvd_pad;
        unsigned char mux_id;
        unsigned char pkt_len[2];
}  __attribute__ ((aligned (1))) *qmap_t;

typedef struct qmap_ipv4_header
{
#ifdef LITTLE_ENDIAN
   unsigned char ihl:4;
   unsigned char version:4;
   unsigned char ecn:2;
   unsigned char dscp:6;
#else
   unsigned char version:4;
   unsigned char ihl:4;
   unsigned char dscp:6;
   unsigned char ecn:2;
#endif
   unsigned short total_length;
   unsigned short identification;
   unsigned short fragment_offset;
   unsigned char ttl;
   unsigned char protocol;
   unsigned short header_checksum;
   unsigned int src_address;
   unsigned int dst_address;
} __attribute__ ((aligned (1))) *qmap_ipv4_header_t;

void qmap_mux(struct sk_buff *skb, sGobiUSBNet *pGobiNet, int data_offset);

void qmap_demux(struct usbnet *dev, struct sk_buff *skb);

enum qmap_mux_errors
{
        QMAP_MUX_SUCCESS,
        QMAP_MUX_INVALID_MUX_ID,
        QMAP_MUX_INVALID_PAD_LENGTH,
        QMAP_MUX_INVALID_PKT_LENGTH,
        QMAP_MUX_ENUM_LENGTH /* This should always be the last element */

};

#endif /* __QMAP_H */

