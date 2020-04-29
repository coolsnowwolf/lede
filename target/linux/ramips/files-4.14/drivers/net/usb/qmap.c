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

#include <linux/if_ether.h>
#include "qmap.h"

/************************************************************
 Function: qmap_mux()

 Inserts qmap header at front of packet with the correct
 Mux ID. skb length values and pointers are adjusted to
 compensate
************************************************************/
void qmap_mux(struct sk_buff *skb, sGobiUSBNet *pGobiNet, int data)
{
   qmap_t qhdr;
   int command_packet;
   int i;
   unsigned short MuxId = pGobiNet->mQMIDev.MuxId;
   
   switch (skb->data[0] & 0xf0) 
   {
      case 0x40:
	  {
	  	  printk("Packet src IP address : %x\n", (((qmap_ipv4_header_t)(skb->data))->src_address));
	  	  if (((qmap_ipv4_header_t)(skb->data))->src_address == htonl(pGobiNet->mQMIDev.IPv4Addr))
	  	  {
	  	     MuxId = pGobiNet->mQMIDev.MuxId;
	  	  }
		  for (i=0;i<MAX_MUX_DEVICES;i++)
		  {
			  if (((qmap_ipv4_header_t)(skb->data))->src_address == htonl(pGobiNet->mQMIMUXDev[i].IPv4Addr))
			  {
				 MuxId = pGobiNet->mQMIMUXDev[i].MuxId;
			  }
		  }
		  command_packet = 0;
		  break;
      }
      case 0x60:
      {
         command_packet = 0;
         break;
      }
      default :
      {
         return;
      }
      //if (memcmp(skb->data, BroadcastAddr, ETH_ALEN) == 0)
      //{
      //   skb_pull(skb, ETH_HLEN);
      //}
      //else
      //{
         /* ignoring QMAP command packets, handling only data packets */
      //   
      //}
   }

   qhdr = (struct qmap_hdr *)skb_push(skb, sizeof(struct qmap_hdr));
   memset(qhdr, 0, sizeof(struct qmap_hdr));
   qhdr->pkt_len[0] = (unsigned char)((skb->len - sizeof(struct qmap_hdr)) >> 8);
   qhdr->pkt_len[1] = (unsigned char)((skb->len - sizeof(struct qmap_hdr)) & 0xff);
   qhdr->mux_id = MuxId;
   qhdr->cd_rsvd_pad = (command_packet << 7);
}

static unsigned short qmap_ip_ethertype(struct sk_buff *skb)
{
        switch (skb->data[0] & 0xf0) {
        case 0x40:
                skb->protocol = __cpu_to_be16(ETH_P_IP);
                break;
        case 0x60:
                skb->protocol = __cpu_to_be16(ETH_P_IPV6);
                break;
        default:
                printk("L3 protocol decode error: 0x%02x, len %d\n",
                       skb->data[0] & 0xf0, skb->len);
        }

        return skb->protocol;
}

void qmap_demux(struct usbnet *dev, struct sk_buff *skb)
{
        qmap_t qhdr;

   if (skb->data[0] & 0x80) {
      printk("command packet\n");
      return;
   }
      qhdr = (struct qmap_hdr *)skb_pull(skb, sizeof(struct qmap_hdr));
      qmap_ip_ethertype(skb);
      usbnet_skb_return(dev, skb);
}

