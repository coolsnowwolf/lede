/***************************************************************
Copyright Statement:

This software/firmware and related documentation (��EcoNet Software��) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (��EcoNet��) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (��ECONET SOFTWARE��) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ��AS IS�� 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER��S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER��S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _ECNT_HOOK_ETHER_H
#define _ECNT_HOOK_ETHER_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/net.h>
#include <linux/if.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/compiler.h>
#include <ecnt_hook/ecnt_hook.h>
#include <modules/eth_global_def.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define ECNT_DRIVER_API  0

#ifdef TCSUPPORT_MAX_PACKET_2000
#define GDM1_LONG_LEN_VALUE 4004  //(RX_MAX_PKT_LEN -20),RX_MAX_PKT_LEN is defined in femac.c,20 is used for 4*VLAN,
#else
#define GDM1_LONG_LEN_VALUE 1700  //(RX_MAX_PKT_LEN-20),RX_MAX_PKT_LEN is defined in femac.c,20 is used for 4*VLAN
#endif

/************************************************************************
*               M A C R O S
*************************************************************************
*/


/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/

typedef enum {
	NOT_ETHER_USE_QDMA_WAN = 0,
	USE_QDMA_WAN_ETHER,
	USE_QDMA_WAN_PON_XDSL,
} Wan_Mode;

/* Warning: same sequence with function array 'qdma_operation' */
typedef enum { 
    ETHER_FUNCTION_MDIO_READ,
    ETHER_FUNCTION_MDIO_WRITE,
    ETHER_FUNCTION_MAC_SNED,
    ETHER_FUNCTION_GSW_PBUS_READ,
    ETHER_FUNCTION_GSW_PBUS_WRITE,
    ETHER_FUNCTION_EXT_GSW_PBUS_READ,
    ETHER_FUNCTION_EXT_GSW_PBUS_WRITE,
    ETHER_FUNCTION_SET_RATELIMIT_SWITCH,
    ETHER_FUNCTION_SET_MACTABLE_SYNC_EN,
	ETHER_FUNCTION_RGMII_SETTING,
	ETHER_FUNCTION_RGMII_MODE,
    ETHER_FUNCTION_SET_PORT_MATRIX,
    ETHER_FUNCTION_GET_PORT_MAP,
    ETHER_FUNCTION_SET_PORT_LINKSTATE,
    ETHER_FUNCTION_SET_PER_VLAN_ACTION,
    ETHER_FUNCTION_SET_PER_PORT_VLAN_ACTION,
    ETHER_FUNCTION_MAC_AUTOBENCH_LOOPBACK,
    ETHER_FUNCTION_CLEAN_MACTABLE,
    ETHER_FUNCTION_USE_QDMA_WAN,
    ETHER_FUNCTION_RECV_PKT,
    ETHER_FUNCTION_GET_FORCE_DSTQ,
    ETHER_FUNCTION_CTAG_WHITE_LIST_MODE,
    ETHER_FUNCTION_CTAG_WHITE_LIST_PER_PORT_MODE,
    ETHER_FUNCTION_CTAG_WHITE_LIST_ADD,
    ETHER_FUNCTION_CTAG_WHITE_LIST_DEL,
    ETHER_FUNCTION_GET_PHY_ADDR,
    ETHER_FUNCTION_ADD_ARL_DIPTBL,
    ETHER_FUNCTION_ADD_ARL_SIPTBL,
    ETHER_FUNCTION_ADD_ARL_IPTBL_MULTI,
    ETHER_FUNCTION_GET_PER_PORT_MIB_COUNTER,
    ETHER_FUNCTION_GET_DROP_CRC_COUNTER,
    ETHER_FUNCTION_SET_PORT_MIRROR,
    ETHER_FUNCTION_SET_FLOW_CONTROL,
    ETHER_FUNCTION_GET_FLOW_CONTROL,
    ETHER_FUNCTION_MAX_NUM,
} ETHER_HookFunction_t ;

typedef enum{
	PORT_LINK_DOWN = 0,
	PORT_LINK_UP,
}ETHER_PORT_LINKSTATE;

typedef enum{	
	ECNT_ETH_RX = 1,
	ECNT_ETH_TX,
}ecnt_eth_subtype;

typedef struct ECNT_ETHER_Data {
	ETHER_HookFunction_t function_id;	/* need put at first item */
	int retValue;
	
	union {
	    struct {
	        uint add;
	        uint reg;
	        uint data;
	    } phy;
	    struct sk_buff *skb;
	    struct 
        {
            u8 wan_type;
            u8 interface;
            u8 mode;
        }traffic_setting;
	    unsigned char ratelimit_En ;
	    unsigned char enable ;
	    struct
            {
                int portMatrixGroup[6];
                int type;
            }matrix_setting;
		struct
		{
			unsigned char lan_port;
			unsigned char switch_port;
		}port_map;
		struct
		{
			unsigned char port_no;
			unsigned char linkstate;
		}port_state;
        struct
        {
            unsigned char port_id;
            unsigned int o_vid;
            unsigned int n_vid;
            ECNT_SWITCH_VLAN_MODE vlan_mode;
            unsigned char enable;
        }vlantable_setting;
        int set_wan_mode;
            struct{
                void *msg_p;
                uint msg_len;
                struct sk_buff *skb;
                uint rx_len;
            }rx_info;
        int force_dstq;
		struct
		{
			u8 mode;
			u8 port;
			u16 vid;
		}ctag_white_list;
        struct
        {
            u32 dip;
            u32 destportmap;
            u32 status;
            u32 leaky_en;
            u32 eg_tag;
            u32 usr_pri;
            u32 is_extend_gsw;
        }arl_diptbl_info;
        struct
        {
            u32 dip;
            u32 sip;
            u32 destportmap;
            u32 is_extend_gsw;
        }arl_siptbl_info;
        struct
        {
            u32 dip;
            u32 sip;
            u32 sw_mask;
            u32 is_extend_gsw;
        }arl_mul_iptbl_info;
                struct
        {
            mt7530_switch_api_MibCntType MibCntType;
            unsigned char port_id;
            unsigned int cnt;
        }mib_counter;
		struct
		{
			unsigned char port_id;
			unsigned int rx_discard;
			unsigned int tx_discard;
			unsigned int rx_error;
			unsigned int tx_error;
		}drop_crc_counter;
		struct sk_buff **pskb;
	} ether_private;
}ECNT_ETHER_Data_s;


/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
static inline int  ETHER_MDIO_READ(unsigned int  phy_add, unsigned int  phy_reg){
    ECNT_ETHER_Data_s in_data;
    int ret=0;

    in_data.function_id = ETHER_FUNCTION_MDIO_READ;
    in_data.ether_private.phy.add = phy_add;
    in_data.ether_private.phy.reg = phy_reg;
    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.ether_private.phy.data;
    else
        return ECNT_HOOK_ERROR;
}

static inline int ETHER_MDIO_WRITE(unsigned int  phy_add, unsigned int  phy_reg, unsigned int  phy_data){
    ECNT_ETHER_Data_s in_data;
    int ret=0;

    in_data.function_id = ETHER_FUNCTION_MDIO_WRITE;
    in_data.ether_private.phy.add = phy_add;
    in_data.ether_private.phy.reg = phy_reg;
    in_data.ether_private.phy.data = phy_data;   
    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int ETHER_MAC_SEND(struct sk_buff *skb){
    
    ECNT_ETHER_Data_s in_data;
    int ret=0;
    in_data.function_id = ETHER_FUNCTION_MAC_SNED;
    in_data.ether_private.skb = skb;
    
    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int ETHER_API_GSW_PBUS_READ(unsigned int  pbus_addr){
    ECNT_ETHER_Data_s in_data;
    int ret=0;

    in_data.function_id = ETHER_FUNCTION_GSW_PBUS_READ;
    in_data.ether_private.phy.add = pbus_addr;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.ether_private.phy.data;
    else
        return ECNT_HOOK_ERROR;
}

static inline int ETHER_API_GSW_PBUS_WRITE(unsigned int  pbus_addr, unsigned int  pbus_data){
    ECNT_ETHER_Data_s in_data;
    int ret=0;

    in_data.function_id = ETHER_FUNCTION_GSW_PBUS_WRITE;
    in_data.ether_private.phy.add = pbus_addr;
    in_data.ether_private.phy.data = pbus_data;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int  ETHER_API_EXT_GSW_PBUS_READ(unsigned int  pbus_addr){
    ECNT_ETHER_Data_s in_data;
    int ret=0;

    in_data.function_id = ETHER_FUNCTION_EXT_GSW_PBUS_READ;
    in_data.ether_private.phy.add = pbus_addr;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.ether_private.phy.data;
    else
        return ECNT_HOOK_ERROR;
}

static inline int ETHER_API_EXT_GSW_PBUS_WRITE(unsigned int  pbus_addr, unsigned int  pbus_data){
    ECNT_ETHER_Data_s in_data;
    int ret=0;

    in_data.function_id = ETHER_FUNCTION_EXT_GSW_PBUS_WRITE;
    in_data.ether_private.phy.add = pbus_addr;
    in_data.ether_private.phy.data = pbus_data;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int ETHER_SET_RATELIMIT_SWITCH(unsigned char enable){
    
    ECNT_ETHER_Data_s in_data;
    int ret=0;
    in_data.function_id = ETHER_FUNCTION_SET_RATELIMIT_SWITCH ;
    in_data.ether_private.ratelimit_En = enable ;
    
    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int ETHER_SET_MACTABLE_SYNC_EN(unsigned char enable){
    
    ECNT_ETHER_Data_s in_data;
    int ret=0;
    in_data.function_id = ETHER_FUNCTION_SET_MACTABLE_SYNC_EN ;
    in_data.ether_private.ratelimit_En = enable ;
    
    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int  ETHER_API_TRGMII_SETTING(unsigned char wan_type, unsigned char interface)
{
    ECNT_ETHER_Data_s in_data;
    int ret=0;

    in_data.function_id = ETHER_FUNCTION_RGMII_SETTING;
    in_data.ether_private.traffic_setting.wan_type = wan_type;
    in_data.ether_private.traffic_setting.interface = interface;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int ETHER_API_RGMII_MODE(unsigned char mode)
{
    ECNT_ETHER_Data_s in_data;
    int ret=0;

    in_data.function_id = ETHER_FUNCTION_RGMII_MODE;
    in_data.ether_private.traffic_setting.interface = RGMII;
    in_data.ether_private.traffic_setting.mode = mode;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int ETHER_API_SET_PORTMATRIX(int group[6], int type)
{
	ECNT_ETHER_Data_s in_data;
	int ret=0;
	int i = 0;

	in_data.function_id = ETHER_FUNCTION_SET_PORT_MATRIX;
    for(i = 0;i < 6;i++){
        in_data.ether_private.matrix_setting.portMatrixGroup[i] = group[i];
    }
    in_data.ether_private.matrix_setting.type = type;
	ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
	return in_data.retValue;
	else
	return ECNT_HOOK_ERROR;
}

static inline int ETHER_API_GET_PORTMAP(unsigned char  lan_port,unsigned char* switch_port)
{
	ECNT_ETHER_Data_s in_data;
	int ret=0;

	in_data.function_id = ETHER_FUNCTION_GET_PORT_MAP;
	in_data.ether_private.port_map.lan_port = lan_port;
	
	ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	*switch_port = in_data.ether_private.port_map.switch_port;
	
	if(ret != ECNT_HOOK_ERROR)
	return in_data.retValue;
	else
	return ECNT_HOOK_ERROR;
}

static inline int ETHER_API_SET_PORT_LINKSTATE(unsigned char port_no,unsigned char linkstate)
{
	ECNT_ETHER_Data_s in_data;
	int ret=0;

	in_data.function_id = ETHER_FUNCTION_SET_PORT_LINKSTATE;
	in_data.ether_private.port_state.port_no = port_no;
	in_data.ether_private.port_state.linkstate = linkstate;

	ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
	    return in_data.retValue;
	else
	    return ECNT_HOOK_ERROR;
}

static inline int ETHER_API_PER_VLAN_ACTION(unsigned char port_id, unsigned int o_vid, unsigned int n_vid, ECNT_SWITCH_VLAN_MODE vlan_mode, unsigned char enable)
{
    ECNT_ETHER_Data_s in_data;
    int ret=0;

    in_data.function_id = ETHER_FUNCTION_SET_PER_VLAN_ACTION;
    in_data.ether_private.vlantable_setting.port_id = port_id;
    in_data.ether_private.vlantable_setting.o_vid = o_vid;
    in_data.ether_private.vlantable_setting.n_vid = n_vid;
    in_data.ether_private.vlantable_setting.vlan_mode = vlan_mode;
    in_data.ether_private.vlantable_setting.enable = enable;

	ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
	    return in_data.retValue;
	else
	    return ECNT_HOOK_ERROR;    
}

static inline int ETHER_API_PER_PORT_VLAN_ACTION(unsigned char port_id, unsigned int n_vid, ECNT_SWITCH_VLAN_MODE vlan_mode, unsigned char enable)
{
    ECNT_ETHER_Data_s in_data;
    int ret=0;

    in_data.function_id = ETHER_FUNCTION_SET_PER_PORT_VLAN_ACTION;
    in_data.ether_private.vlantable_setting.port_id = port_id;
    in_data.ether_private.vlantable_setting.n_vid = n_vid;
    in_data.ether_private.vlantable_setting.vlan_mode = vlan_mode;
    in_data.ether_private.vlantable_setting.enable = enable;

	ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
	    return in_data.retValue;
	else
	    return ECNT_HOOK_ERROR;    
}

static inline int ETHER_MAC_AUTOBENCH_LOOPBACK(void)
{
    ECNT_ETHER_Data_s in_data;
    int ret=0;

    in_data.function_id = ETHER_FUNCTION_MAC_AUTOBENCH_LOOPBACK;

	ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
	    return in_data.retValue;
	else
	    return ECNT_HOOK_ERROR;    
}

static inline int ETHER_CLEAN_MACTABLE(void)
{
	ECNT_ETHER_Data_s in_data;
	int ret=0;

	in_data.function_id = ETHER_FUNCTION_CLEAN_MACTABLE;
	
	ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ETHER_WAN_USE_QDMA_WAN(int set_wan_mode)
{
	ECNT_ETHER_Data_s in_data;
	int ret=0;

	in_data.ether_private.set_wan_mode = set_wan_mode;

	in_data.function_id = ETHER_FUNCTION_USE_QDMA_WAN;
	
	ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	
	if(ret != ECNT_HOOK_ERROR)
	return in_data.retValue;
	else
	return ECNT_HOOK_ERROR;
}

static inline unsigned int ETHER_API_RECV_PKT(void *msg_p, unsigned int msg_len, struct sk_buff *skb, unsigned int rx_len)
{
    ECNT_ETHER_Data_s in_data;
    int ret=0;

    in_data.function_id = ETHER_FUNCTION_RECV_PKT;
    in_data.ether_private.rx_info.msg_p     = msg_p;
    in_data.ether_private.rx_info.msg_len   = msg_len;
    in_data.ether_private.rx_info.skb       = skb;
    in_data.ether_private.rx_info.rx_len    = rx_len;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int ETHER_API_GET_FORCE_DSTQ(int *p_force_dstq)
{
    ECNT_ETHER_Data_s in_data;
    int ret = 0;

    in_data.function_id = ETHER_FUNCTION_GET_FORCE_DSTQ;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
    {
        *p_force_dstq = in_data.ether_private.force_dstq;
        return in_data.retValue;
    }
    else
        return ECNT_HOOK_ERROR;    
}

static inline int ETHER_API_CTAG_WHITE_LIST_MODE(u8 mode)
{
    ECNT_ETHER_Data_s in_data;
    int ret = 0;

    in_data.function_id = ETHER_FUNCTION_CTAG_WHITE_LIST_MODE;
	in_data.ether_private.ctag_white_list.mode = mode;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;  
}
static inline int ETHER_API_CTAG_WHITE_LIST_PER_PORT_MODE(u8 port, u8 mode)
{
    ECNT_ETHER_Data_s in_data;
    int ret = 0;

    in_data.function_id = ETHER_FUNCTION_CTAG_WHITE_LIST_PER_PORT_MODE;
	in_data.ether_private.ctag_white_list.mode = mode;
    in_data.ether_private.ctag_white_list.port = port;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;  
}
static inline int ETHER_API_CTAG_WHITE_LIST_ADD(u8 port, u16 vid)
{
    ECNT_ETHER_Data_s in_data;
    int ret = 0;

    in_data.function_id = ETHER_FUNCTION_CTAG_WHITE_LIST_ADD;
	in_data.ether_private.ctag_white_list.port = port;
	in_data.ether_private.ctag_white_list.vid = vid;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}
static inline int ETHER_API_CTAG_WHITE_LIST_DEL(u8 port, u16 vid)
{
    ECNT_ETHER_Data_s in_data;
    int ret = 0;

    in_data.function_id = ETHER_FUNCTION_CTAG_WHITE_LIST_DEL;
	in_data.ether_private.ctag_white_list.port = port;
	in_data.ether_private.ctag_white_list.vid = vid;

   	ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline unsigned int ETHER_API_GET_PHY_ADDR(unsigned int lan_port)
{
    ECNT_ETHER_Data_s in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(ECNT_ETHER_Data_s)) ;

    in_data.function_id = ETHER_FUNCTION_GET_PHY_ADDR;
    in_data.ether_private.phy.data = lan_port;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
    {
        return in_data.ether_private.phy.add;
    }
    else
        return ECNT_HOOK_ERROR;    
}

static inline unsigned int ETHER_API_ADD_ARL_DIPTBL(u32 is_extend_gsw, u32 dip, u32 destportmap, u32 status, u32 leaky_en, u32 eg_tag, u32 usr_pri)
{
    ECNT_ETHER_Data_s in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(ECNT_ETHER_Data_s));

    in_data.function_id = ETHER_FUNCTION_ADD_ARL_DIPTBL;
    in_data.ether_private.arl_diptbl_info.is_extend_gsw = is_extend_gsw;
    in_data.ether_private.arl_diptbl_info.dip = dip;
    in_data.ether_private.arl_diptbl_info.destportmap = destportmap;
    in_data.ether_private.arl_diptbl_info.status = status;
    in_data.ether_private.arl_diptbl_info.leaky_en = leaky_en;
    in_data.ether_private.arl_diptbl_info.eg_tag = eg_tag;
    in_data.ether_private.arl_diptbl_info.usr_pri = usr_pri;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
    {
        return in_data.retValue;
    }
    else
        return ECNT_HOOK_ERROR;     
}

static inline unsigned int ETHER_API_ADD_ARL_SIPTBL(u32 is_extend_gsw,u32 dip, u32 sip, u32 destportmap)
{
    ECNT_ETHER_Data_s in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(ECNT_ETHER_Data_s));

    in_data.function_id = ETHER_FUNCTION_ADD_ARL_DIPTBL;
    in_data.ether_private.arl_siptbl_info.is_extend_gsw = is_extend_gsw;
    in_data.ether_private.arl_siptbl_info.dip = dip;
    in_data.ether_private.arl_siptbl_info.sip = sip;
    in_data.ether_private.arl_siptbl_info.destportmap = destportmap;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
    {
        return in_data.retValue;
    }
    else
        return ECNT_HOOK_ERROR;     
}

static inline unsigned int ETHER_API_ADD_ARL_IPTBL_MULTI(u32 is_extend_gsw, u32 dip, u32 sip, u32 sw_mask)
{
    ECNT_ETHER_Data_s in_data;
    int ret = 0;
    memset(&in_data, 0, sizeof(ECNT_ETHER_Data_s));

    in_data.function_id = ETHER_FUNCTION_ADD_ARL_IPTBL_MULTI;
    in_data.ether_private.arl_mul_iptbl_info.is_extend_gsw = is_extend_gsw;
    in_data.ether_private.arl_mul_iptbl_info.dip = dip;
    in_data.ether_private.arl_mul_iptbl_info.sip = sip;
    in_data.ether_private.arl_mul_iptbl_info.sw_mask = sw_mask;

    ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
    {
        return in_data.retValue;
    }
    else
        return ECNT_HOOK_ERROR;   
}

static inline int ETHER_API_GET_PER_PORT_MIB_COUNTER(mt7530_switch_api_MibCntType MibCntType, unsigned char port_id, unsigned int *cnt)
{
    ECNT_ETHER_Data_s in_data;
    int ret=0;

    in_data.function_id = ETHER_FUNCTION_GET_PER_PORT_MIB_COUNTER;
    in_data.ether_private.mib_counter.MibCntType = MibCntType;
    in_data.ether_private.mib_counter.port_id = port_id;

	ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);    
	*cnt = in_data.ether_private.mib_counter.cnt;
    
	if(ret != ECNT_HOOK_ERROR)
	    return in_data.retValue;
	else
	    return ECNT_HOOK_ERROR;    
}

static inline int ETHER_API_GET_DROP_CRC_COUNTER(unsigned char port_id, unsigned int *cnt_rx_discard,
	unsigned int *cnt_tx_discard, unsigned int *cnt_rx_error, unsigned int *cnt_tx_error)
{
	ECNT_ETHER_Data_s in_data;
	int ret=0;
	
	in_data.function_id = ETHER_FUNCTION_GET_DROP_CRC_COUNTER;
	in_data.ether_private.drop_crc_counter.port_id = port_id;

	ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);  

	*cnt_rx_discard = in_data.ether_private.drop_crc_counter.rx_discard;
	*cnt_tx_discard = in_data.ether_private.drop_crc_counter.tx_discard;
	*cnt_rx_error = in_data.ether_private.drop_crc_counter.rx_error;
	*cnt_tx_error = in_data.ether_private.drop_crc_counter.tx_error;

	if(ret != ECNT_HOOK_ERROR)
	    return in_data.retValue;
	else
	    return ECNT_HOOK_ERROR;   
}

static inline int ECNT_ETH_RECV_HOOK(struct sk_buff **skb)
{	
	int ret = ECNT_CONTINUE;
	ECNT_ETHER_Data_s in_data;

	in_data.ether_private.pskb = skb;
	
	if (!list_empty(&ecnt_hooks[ECNT_ETHER_SWITCH][ECNT_ETH_RX]))
	{

		ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_ETH_RX, (struct ecnt_data *)&in_data);
	}
	
	return ret;
}

static inline int ECNT_ETH_SEND_HOOK(struct sk_buff **skb)
{	
	int ret = ECNT_CONTINUE;
	ECNT_ETHER_Data_s in_data;

	in_data.ether_private.pskb = skb;
	
	if (!list_empty(&ecnt_hooks[ECNT_ETHER_SWITCH][ECNT_ETH_TX]))
	{

		ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_ETH_TX, (struct ecnt_data *)&in_data);
	}
	
	return ret;
}

static inline int ETHER_API_SET_PORT_MIRROR(unsigned char enable, unsigned char port_no)
{
	ECNT_ETHER_Data_s in_data;
	int ret = 0;

	in_data.function_id = ETHER_FUNCTION_SET_PORT_MIRROR;
	in_data.ether_private.port_state.linkstate = enable;
	in_data.ether_private.port_state.port_no = port_no;
	
	ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
	{
		return in_data.retValue;
	}
	else
	{
		return ECNT_HOOK_ERROR;
	}
}

static inline int ETHER_API_SET_FLOW_CONTROL( unsigned char port_no, unsigned char enable)
{
	ECNT_ETHER_Data_s in_data;
	int ret = 0;

	in_data.function_id = ETHER_FUNCTION_SET_FLOW_CONTROL;
	in_data.ether_private.port_state.linkstate = enable;
	in_data.ether_private.port_state.port_no = port_no;
	
	ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
	{
		return in_data.retValue;
	}
	else
	{
		return ECNT_HOOK_ERROR;
	}
}

static inline int ETHER_API_GET_FLOW_CONTROL(unsigned char port_no, unsigned char *enable)
{
	ECNT_ETHER_Data_s in_data;
	int ret = 0;

	in_data.function_id = ETHER_FUNCTION_SET_FLOW_CONTROL;
	in_data.ether_private.port_state.port_no = port_no;
	
	ret = __ECNT_HOOK(ECNT_ETHER_SWITCH, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	*enable = in_data.ether_private.port_state.linkstate;
	
	if(ret != ECNT_HOOK_ERROR)
	    return in_data.retValue;
	else
	    return ECNT_HOOK_ERROR; 
}

#endif /*_ETHER_HOOK_H*/
