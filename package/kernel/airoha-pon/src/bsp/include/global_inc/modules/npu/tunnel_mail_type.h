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
#ifndef TUNNEL_MAIL_TYPE_H_
#define TUNNEL_MAIL_TYPE_H_

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define NPU_MAILBOX_ERROR 0
#define NPU_MAILBOX_SUCCESS 1

#define VXLAN_HDR_SIZE 50
#define SRV6_HDR_MAX_SIZE 124
	
typedef enum {
	TUNNLE_MAIL_FUNCTION_STORE_VXLAN_HDR = 0,        
	TUNNLE_MAIL_FUNCTION_RELEASE_VXLAN_HDR,	
	TUNNLE_MAIL_FUNCTION_SET_VXLAN_MTU,
	TUNNLE_MAIL_FUNCTION_STORE_SRV6_HDR,
	TUNNLE_MAIL_FUNCTION_SET_SRV6_MYIP,
	TUNNLE_MAIL_FUNCTION_SET_FRAG_MTU,
	TUNNLE_MAIL_FUNCTION_SET_NPU_BRIDGE_DEBUG,
	TUNNLE_MAIL_FUNCTION_STORE_MAPT_ADDR,
	TUNNLE_MAIL_FUNCTION_MAX_NUM
} TUNNEL_MAIL_Func_t;

typedef enum {
	NPU_BRIDGE_DEBUG_CMD_DUMP_CNT = 0,        
	NPU_BRIDGE_DEBUG_CMD_CLEAR_CNT,
	NPU_BRIDGE_DEBUG_CMD_RELEASE_BUFF,
	NPU_BRIDGE_DEBUG_CMD_SET_LEVEL,
	NPU_BRIDGE_DEBUG_CMD_SET_LOOPBACK,
	NPU_BRIDGE_DEBUG_CMD_MAX_NUM
} NPU_BRIDGE_DEBUG_CMD_t;

typedef struct vxlan_hdr_s
{
   unsigned char idx;
   unsigned char data[VXLAN_HDR_SIZE];  
}vxlan_hdr_t;

typedef struct srv6_hdr_s
{
	unsigned char idx;
	unsigned char tot_len;
	unsigned char data[SRV6_HDR_MAX_SIZE];  
}srv6_hdr_t;

typedef struct frag_mtu_s
{
	unsigned char idx;
	unsigned int mtu;
}frag_mtu_t;

typedef struct npu_bridge_debug_s
{
	NPU_BRIDGE_DEBUG_CMD_t debug_cmd;
	unsigned int debug_level;
}npu_bridge_debug_t;

typedef struct TUNNEL_MAIL_Data {
	unsigned int funcId;
	union {
        vxlan_hdr_t vxlan_hdr;
        unsigned int vxlan_mtu;
		srv6_hdr_t srv6_hdr;
		unsigned char my_ip6[16]; 
		frag_mtu_t frag_mtu;
		npu_bridge_debug_t debug;
		unsigned long long mapt_info_addr;
	} tunnel_mail_private;
}TUNNEL_MAIL_Data_t;


#endif 


