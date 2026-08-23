/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _QDMA_FRAG_REASSEMBLE_H_
#define _QDMA_FRAG_REASSEMBLE_H_

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/	
#include <linux/netdevice.h>
#include <linux/types.h>
#include <asm/tc3162/tc3162.h>

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#if defined(QDMA_LAN)
#define qdma_frag_reassemble_path "qdma_lan/frag_reassemble"
#else
#define qdma_frag_reassemble_path "qdma_wan/frag_reassemble"
#endif


/************************************************************************
*                  M A C R O S
*************************************************************************
*/	
#define FRAG_TX_RING_IDX 5


/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/
typedef enum {
	HOP_NORMAL_PACKET = 0,
	HOP_TO_FRAGMENT,
	HOP_TO_ASSEMBLE,
	HOP_FROM_SOE_ENC,
	HOP_FROM_SOE_DEC,
	HOP_FROM_SOE_DEC_ICV_ERROR,
	HOP_FROM_SOE_DEC_SEQ_ERROR,
	HOP_FROM_SOE_DEC_PAD_ERROR,
} HOP_FLAGS_t ;

typedef union
{
	struct {
#ifdef __BIG_ENDIAN
		uint32 usr_info		:18;
		uint32 longf 		:1;
		uint32 runtf		:1;
		uint32 crcer		:1;
		uint32 				:1;
		uint32 ptp			:1;
		uint32 oam			:1;
		uint32 channel		:5;
		uint32 hop_flags    :3;
#else
		uint32 hop_flags    :3;
		uint32 channel		:5;
		uint32 oam			:1;
		uint32 ptp			:1;
		uint32 				:1;
		uint32 crcer		:1;
		uint32 runtf		:1;
		uint32 longf 		:1;
		uint32 usr_info		:18;
#endif
	}raw;
    uint32 word;
} rxMsgWord0_hop_t;

typedef union
{
    struct {
#ifdef __BIG_ENDIAN
        uint32 dei			:1;
        uint32 ip6			:1;
        uint32 ip4			:1;
        uint32 ip4f         :1;
        uint32 l4vld        :1;
        uint32 l4f          :1;
        uint32 sport        :5;
        uint32 crsn         :5;
        uint32 ppe_entry    :16;
#else
        uint32 ppe_entry    :16;
        uint32 crsn         :5;
        uint32 sport        :5;
        uint32 l4f          :1;
        uint32 l4vld        :1;
        uint32 ip4f         :1;
        uint32 ip4			:1;
        uint32 ip6			:1;
        uint32 dei			:1;
#endif
    }raw;
    uint32 word;
} rxMsgWord1_hop_t;

typedef union
{
	struct {
#ifdef __BIG_ENDIAN
		uint32 mtu          :16;
		uint32 ifc_id       :8;
		uint32 ifc_hit      :1;
		uint32 l2_len       :7;
#else
		uint32 l2_len       :7;
		uint32 ifc_hit      :1;
		uint32 ifc_id       :8;
		uint32 mtu          :16;
#endif
    }raw;
    uint32 word;
} rxMsgWord2_hop_t;

typedef union
{
	struct {
#ifdef __BIG_ENDIAN
		uint32 hop_fp		:9;
		uint32 hop_info		:9;
		uint32 sa_id		:6;
		uint32 sw_udf		:8;
#else
		uint32 sw_udf		:8;
		uint32 sa_id		:6;
		uint32 hop_info		:9;
		uint32 hop_fp		:9;
#endif
	}raw;
    uint32  word;
} rxMsgWord3_hop_t;

typedef struct qdmaRxMsgHop_s
{
    rxMsgWord0_hop_t rxMsgHopW0;
    rxMsgWord1_hop_t rxMsgHopW1;
    rxMsgWord2_hop_t rxMsgHopW2;
    rxMsgWord3_hop_t rxMsgHopW3;
}qdmaRxMsgHop_t;

/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/



/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
int qdma_frag_reassemble_init(void);
void qdma_frag_reassemble_deinit(void);
int qdma_fragment_reassemble_handler(struct sk_buff *skb, uint rx_len, void *dscp_p, int ringIdx);

#endif /* _QDMA_VIRTUAL_IF_H_ */

