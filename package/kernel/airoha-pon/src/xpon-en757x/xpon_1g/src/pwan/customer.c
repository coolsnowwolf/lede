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

#include <linux/netdevice.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#include <linux/foe_hook.h>
#include "common/drv_global.h"
#include "pwan/gpon_wan.h"
#include "pwan/xpon_netif.h"
#include "gpon/gpon_dvt.h"
#include "gpon/gpon_recovery.h"
#include "common/xpondrv.h"
#include "common/omci_oam_monitor.h"

#include <ecnt_hook/ecnt_hook_traffic_classify.h>
#include <ecnt_hook/ecnt_hook_smux.h>

#include <ecnt_hook/ecnt_hook_vlan.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <ecnt_hook/ecnt_hook_fe.h>
#if defined(TCSUPPORT_XPON_LED)
#include <asm/tc3162/ledcetrl.h>
#endif
#include "common/xpon_led.h"

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HYBIRD)
#include <pwan/hybird_wan.h>
#endif/*TCSUPPORT_COMPILE*/


/************************************************************************************************
							Struct Define
**************************************************************************************************/
struct pwan_stats {
	unsigned long	rx_pkts;		/* total pkts received	*/
	unsigned long	tx_pkts;		/* total pkts transmitted	*/
};


/************************************************************************************************
							Macro Define
**************************************************************************************************/
#define FTP_PORT_NUM 10
#define DHCP_MODE 1
#define PPPOE_MODE 2

/*************************************************************************************************
							Function Declare
***************************************************************************************************/
extern int fb_pwan_tx_vlan_trans_proc(struct sk_buff *skb);
extern int fb_pwan_tx_vlan_proc(struct sk_buff *skb);
extern int tr143RxShortCut(int enable, struct sk_buff *skb, int vlanLayer, int ifaceidx, int iptype
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_IS_CF_PON)
, char *devname
#endif/*TCSUPPORT_COMPILE*/
);
extern int pppoe_bridge_multicast_handle(struct sk_buff* skb);
extern unsigned int* (*recv_rtp_src_port_get_hook)(void);
#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
extern int (*sw_pktqosEnqueue) (struct sk_buff * bp);
extern void (*sw_PKTQOS_SET_STOP) (void);
extern int (*sw_isSWQosActive) (void);
#endif

/**************************************************************************************************
							Variable Define
***************************************************************************************************/
int pppoeBridgeMultiShortCut = 1;  //0: close shortcut; 1: open shortcut
#if defined(TR068_LED)
XPON_ALARM_LED_Status_t alarm_led_status = ALARM_LED_ON;
extern int internet_led_on;
extern int internet_trying_led_on;
#endif
extern int internet_hwnat_pktnum;
extern int internet_hwnat_timer_switch;
extern unsigned int test_ftp_port[FTP_PORT_NUM];
#if defined(TCSUPPORT_XPON_LED)
static struct pwan_stats pwanStats;
#endif
unsigned int hw_qos_enable = 0;

#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
static unsigned char qos_stop_enable = 1;
static unsigned int qos_stop_num = 1000;
#endif

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON_GDV20) || defined(TCSUPPORT_IS_CF_PON)
static unsigned char tr143_test_enable = 0; /* 1: enable, 2: debug on, 0: disabled. */
static unsigned char tr143_test_iface = 0; /* 0~63 */
static unsigned short tr143_test_port = 0;
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_IS_CF_PON)
static char recv_iface[32] = {0};
#endif/*TCSUPPORT_COMPILE*/
#endif/*TCSUPPORT_COMPILE*/

#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
int trtcmEnable = 0;
int trtcmTsid = 0;
__u32 priPktChkLen = DEF_PRIORITY_PKT_CHK_LEN;
int priPktChk = 1;

#ifdef TCSUPPORT_QOS
#if defined(TCSUPPORT_XPON_HAL_API_QOS)|| defined(TCSUPPORT_XPON_HAL_API_EXT)
char qosFlag = QOS_HW_CAR;
#else
char qosFlag = NULLQOS;
#endif
#endif

#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
#if defined(TCSUPPORT_XPON_HAL_API_QOS)|| defined(TCSUPPORT_XPON_HAL_API_EXT)
int gQueueMask = 0;
#else
static int gQueueMask = 0;
#endif
int isNeedHWQoS = 1;
#endif

#ifndef TCSUPPORT_QOS
#define QOS_REMARKING  1  
#endif

#define TCSUPPORT_HW_QOS

#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
static int qos_wrr_info[5] = {0};
static uint8 maxPrio = 3;
#endif
#endif

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_FWC_ENV)
FHNet_Mapping_Result_Out_t gMappingRet = {0};
uint fhNetMappingDebug = 0;
unsigned char  gFhDrop = 0;

typedef int(* P_FHNET_DS_VLAN_HOOK) 		  (FHNet_Mapping_Vlan_Para_In_t *pktInfo, unsigned char *dropFlag);
typedef int(* P_FHNET_L3_DS_VLAN_HOOK)		  (FHNet_L3Pkt_Mapping_Vlan_Para_In_t *pktInfo, unsigned char *dropFlag);
typedef int(* P_FHNET_US_VLAN_MAPPING_HOOK)   (FHNet_Mapping_Vlan_Para_In_t *pktInfo,FHNet_Mapping_Result_Out_t   *mappingRet);
typedef int(* P_FHNET_L3_US_VLAN_MAPPING_HOOK)(FHNet_L3Pkt_Mapping_Vlan_Para_In_t *pktInfo, FHNet_Mapping_Result_Out_t   *mappingRet);

P_FHNET_DS_VLAN_HOOK			 fhnet_ds_vlan_action_hook;
P_FHNET_L3_DS_VLAN_HOOK 		 fhnet_L3_ds_vlan_action_hook;
P_FHNET_US_VLAN_MAPPING_HOOK	 fhnet_us_vlan_action_mapping_hook;
P_FHNET_L3_US_VLAN_MAPPING_HOOK  fhnet_L3_us_vlan_action_mapping_hook;
#endif/*TCSUPPORT_COMPILE*/

/*************************************************************************************
				Function API Define
*****************************************************************************************/
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_FWC_ENV)
int fhnet_ds_vlan_action_callback(P_FHNET_DS_VLAN_HOOK func)
{
	rcu_assign_pointer(fhnet_ds_vlan_action_hook,func);
}
int fhnet_cpu_pkt_ds_vlan_action_callback(P_FHNET_L3_DS_VLAN_HOOK func)
{
	printk("%s,%d\n",__FUNCTION__,__LINE__);
	rcu_assign_pointer(fhnet_L3_ds_vlan_action_hook,func);
}
int fhnet_us_vlan_action_mapping_callback(P_FHNET_US_VLAN_MAPPING_HOOK func)
{
	rcu_assign_pointer(fhnet_us_vlan_action_mapping_hook,func);
}
int fhnet_cpu_pkt_us_vlan_action_mapping_callback(P_FHNET_US_VLAN_MAPPING_HOOK func)
{
	printk("%s,%d\n",__FUNCTION__,__LINE__);
	rcu_assign_pointer(fhnet_L3_us_vlan_action_mapping_hook,func);
}

EXPORT_SYMBOL(fhnet_ds_vlan_action_callback);
EXPORT_SYMBOL(fhnet_cpu_pkt_ds_vlan_action_callback);
EXPORT_SYMBOL(fhnet_us_vlan_action_mapping_callback);
EXPORT_SYMBOL(fhnet_cpu_pkt_us_vlan_action_mapping_callback);


static int fhnet_us_vlan_action_mapping_debug(FHNet_Mapping_Vlan_Para_In_t *pktInfo, 
											   FHNet_Mapping_Result_Out_t	*mappingRet)
{
	*mappingRet=gMappingRet;
	PON_MSG(MSG_ERR, "fhnet_us_vlan_action_mapping_debug gemport %d queueId %d dropFlag %d \n",
		mappingRet->gemportId,mappingRet->queueId,mappingRet->dropFlag);
	return 0;
}
static int fhnet_l3_us_vlan_action_mapping_debug(FHNet_L3Pkt_Mapping_Vlan_Para_In_t *pktInfo, 
											   FHNet_Mapping_Result_Out_t	*mappingRet)
{
	*mappingRet=gMappingRet;
	PON_MSG(MSG_ERR, "fhnet_l3_us_vlan_action_mapping_debug gemport %d queueId %d dropFlag %d \n",
		mappingRet->gemportId,mappingRet->queueId,mappingRet->dropFlag);
	return 0;
}

static int fhnet_ds_vlan_action (FHNet_Mapping_Vlan_Para_In_t *pktInfo, 
						  unsigned char *dropFlag)
{
	*dropFlag = gFhDrop;
	return 0;
}

static int fhnet_l3_ds_vlan_action (FHNet_Mapping_Vlan_Para_In_t *pktInfo, 
						  unsigned char *dropFlag)
{
	*dropFlag = gFhDrop;
	return 0;
}

void fhnet_set_vlan_mapping_hook(void)
{
	static P_FHNET_US_VLAN_MAPPING_HOOK 	us_vlan_mapping_hook = NULL;
	static P_FHNET_L3_US_VLAN_MAPPING_HOOK	l3_us_vlan_mapping_hook = NULL;
	static P_FHNET_DS_VLAN_HOOK 			ds_vlan_hook = NULL;
	static P_FHNET_L3_DS_VLAN_HOOK			l3_ds_vlan_hook = NULL; 
	
	if(fhNetMappingDebug)
	{
		if(fhnet_L3_us_vlan_action_mapping_hook != fhnet_l3_us_vlan_action_mapping_debug)
		{
			us_vlan_mapping_hook	= fhnet_us_vlan_action_mapping_hook;
			l3_us_vlan_mapping_hook = fhnet_L3_us_vlan_action_mapping_hook;
			ds_vlan_hook			= fhnet_ds_vlan_action_hook;
			l3_ds_vlan_hook 		= fhnet_L3_ds_vlan_action_hook; 		
		}
		rcu_assign_pointer(fhnet_L3_us_vlan_action_mapping_hook,fhnet_l3_us_vlan_action_mapping_debug);
		rcu_assign_pointer(fhnet_us_vlan_action_mapping_hook,fhnet_us_vlan_action_mapping_debug);
		rcu_assign_pointer(fhnet_ds_vlan_action_hook,fhnet_ds_vlan_action);
		rcu_assign_pointer(fhnet_L3_ds_vlan_action_hook,fhnet_l3_ds_vlan_action);		
	}
	else
	{
		rcu_assign_pointer(fhnet_L3_us_vlan_action_mapping_hook,l3_us_vlan_mapping_hook);
		rcu_assign_pointer(fhnet_us_vlan_action_mapping_hook,us_vlan_mapping_hook); 
		rcu_assign_pointer(fhnet_ds_vlan_action_hook,ds_vlan_hook);
		rcu_assign_pointer(fhnet_L3_ds_vlan_action_hook,l3_ds_vlan_hook);			
	}
}

static int fhnet_get_inport_from_skb(struct sk_buff *skb)
{
	/* to do, inport may from lan, wifi and usb, so port number should be get from skb */
	unchar port = ETH_GET_LAN_PORT(skb);
	
	if(port >= E_ETH_PORT_0 && port <= E_ETH_PORT_3)
	{
		PON_MSG(MSG_ERR,"port from skb->mark is Lan %d",port);
	}
	else if(port >= E_WIFI_PORT_0 && port <= E_WIFI_PORT_3)
	{
		port = port - E_WIFI_PORT_0 + ECNT_PORT_DEVICE_RA0;
		PON_MSG(MSG_ERR,"port from skb->mark is Wifi %d",port);
	}
#if 0 /* todo add usb port  */
	if(port < ETH_SKB_MARK_PORT_MAX_USB)
	{
		port = port - E_WIFI_PORT_3 - 1 + ECNT_PORT_DEVICE_USB;
		PON_MSG(MSG_ERR,"port from skb->mark is Usb %d",port);	
	}
#endif
	else if(port == E_WAN_PORT)
	{
		port = ECNT_PORT_DEVICE_PON0;
		PON_MSG(MSG_ERR,"port from skb->mark is wan %d",port);			
	}
	else
	{
		PON_MSG(MSG_ERR,"port from skb->mark is unknow %d",port);	
	}
	
	return port;
}

static int fhnet_get_dev_by_skb(struct sk_buff *skb,struct net_device **dev)
{
	
	struct smux_api_data_s smux_data = {0};
	SMUX_Dev_By_Mac indate = {0};
	if(skb == NULL || dev == NULL)
	{
		printk("input para is null, skb %x dev %x\n",skb,dev);
		return RETURN_RET_NORMAL_FAIL;
	}
	skb_reset_mac_header(skb);
	
	indate.skb = skb;
	indate.dev = dev;
	
	smux_data.api_type = SMUX_API_TYPE_GET;
	smux_data.cmd_id   = SMUX_GET_DEV_BY_SRC_MAC;
	smux_data.smux_dev_by_mac	   = &indate ;	

	if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_SMUX, ECNT_SMUX_API, (struct ecnt_data * )&smux_data) ){
		printk("ECNT_HOOK_ERROR occur with cmd_id:0x%x\n", smux_data.cmd_id);
		return RETURN_RET_NORMAL_FAIL;
	}	

	if(smux_data.ret != SMUX_ECNT_HOOK_PROCESS_SUCESS)
	{
		PON_MSG(MSG_ERR, "fhnet_get_dev_by_skb Fail \n");
		return RETURN_RET_NORMAL_FAIL;
	}
	return RETURN_RET_SUCCESS;
}


static int fhnet_check_l3_packet(struct sk_buff *skb, struct net_device ** dev)
{
	if(1 == skb->bridge_flag ){
		return 0;
	}

	if(dev != NULL){
		fhnet_get_dev_by_skb(skb, dev);
	}
	
	/* todo change skb->mark defination, bit 14 is bridge_flag */
	if(FHWC_IS_MARKED_ROUTE_PKT(skb) ){
		return 1;
	}

	return 0;
}

static int fhnet_get_port_from_dev(struct net_device *dev, unchar *uniPort)
{
	int name_len =0 ;
	if(NULL == dev)
	{
		printk("fenet_get_port_from_dev dev = %x is NULL\n",dev);
		return RETURN_RET_NORMAL_FAIL;
	}
	name_len = strlen(dev->name);
	if(name_len <= 0)
	{
		printk("dev name len is 0 \n");
		return RETURN_RET_SUCCESS;
	}
	
	switch(dev->name[0])
	{
		case 'e':
			*uniPort = atoi_temp(&dev->name[name_len -1]);
			break;

		case 'r':
			*uniPort = atoi_temp(&dev->name[name_len -1]) + ECNT_PORT_DEVICE_RA0;
			break;

		case 'u':
			*uniPort = atoi_temp(&dev->name[name_len -1]) + ECNT_PORT_DEVICE_USB;
			break;

		default:
			return RETURN_RET_NORMAL_FAIL;
	}
	PON_MSG(MSG_ERR,"dev name is %s len is %d uniport is %d\n",dev->name,name_len,*uniPort);
	return RETURN_RET_SUCCESS;
}

static int fhnet_gwan_vlan_mapping_proc(struct sk_buff *skb)
{
	FHNet_Mapping_Vlan_Para_In_t packetInfo;
	FHNet_L3Pkt_Mapping_Vlan_Para_In_t L3packetInfo;
	FHNet_Mapping_Result_Out_t mappingRet;
	struct net_device *dev = NULL;
	int ret = 0;

	if(NULL == skb)
	{
		printk("ERR %s input para is NULL \n",__FUNCTION__);
		return RETURN_RET_NORMAL_FAIL;
	}

	memset(&packetInfo, 0, sizeof(packetInfo));
	memset(&L3packetInfo, 0, sizeof(L3packetInfo));
	memset(&mappingRet, 0, sizeof(mappingRet));

	if(gpPonSysData->debugLevel & MSG_ERR) 
	{
		printk("====before fh mapping====\n");
		__dump_skb(skb, skb->len) ;
	} 

	if(fhnet_check_l3_packet(skb, &dev))
	{
		if(dev == NULL) 
		{
			PON_MSG(MSG_ERR, "fhnet_get_dev_by_skb dev is NULL \n");
			return RETURN_RET_NORMAL_FAIL;
		}
		L3packetInfo.skb = skb;
		strcpy(L3packetInfo.interfaceName,dev->name);
		PON_MSG(MSG_ERR, "Us L3packetInfo.interfaceName %s hook fh L3 vlan mapping\n",L3packetInfo.interfaceName);
		if(fhnet_L3_us_vlan_action_mapping_hook != NULL)
		{
			ret = fhnet_L3_us_vlan_action_mapping_hook(&L3packetInfo,&mappingRet);
			if(ret != 0 || mappingRet.dropFlag == FHNET_IS_DROP_TRUE)
			{
				if(gpPonSysData->debugLevel & MSG_ERR) 
				{
					printk("ERR fhnet_L3_us_vlan_action_mapping_hook proc ret %d, dropFlag %d \n",ret,mappingRet.dropFlag);
					printk("fhnet_L3_gwan_vlan_mapping_proc drop packet\n");
					__dump_skb(skb, skb->len) ;
				} 
				return RETURN_RET_NORMAL_FAIL;
			}
		}
		else
		{
			PON_MSG(MSG_ERR, "fhnet_L3_us_vlan_action_mapping_hook is NULL return\n");
		}
	}
	else
	{
		packetInfo.skb = skb;
		packetInfo.inPort = fhnet_get_inport_from_skb(skb);
		packetInfo.outPort = ECNT_PORT_DEVICE_PON0;
		PON_MSG(MSG_ERR, "US L2 skb->mark %x inport %d, outport %d, hook fh vlan mapping\n",
		skb->mark, packetInfo.inPort, packetInfo.outPort);
		if(fhnet_us_vlan_action_mapping_hook != NULL)
		{
			ret = fhnet_us_vlan_action_mapping_hook(&packetInfo,&mappingRet);
			if(ret != 0 || mappingRet.dropFlag == FHNET_IS_DROP_TRUE)
			{
				if(gpPonSysData->debugLevel & MSG_ERR) 
				{
					printk("ERR fhnet_us_vlan_action_mapping_hook proc ret %d, dropFlag %d \n",ret,mappingRet.dropFlag);
					printk("fhnet_gwan_vlan_mapping_proc drop packet\n");
					__dump_skb(skb, skb->len) ;
				} 
				return RETURN_RET_NORMAL_FAIL;
			}
		}
		else
		{
			PON_MSG(MSG_ERR, "fhnet_us_vlan_action_mapping_hook is NULL return\n");
		}
	}
	PON_MSG(MSG_ERR, "mapping result gem %d, allocid %d, queue %d, dropflag %d\n",
		mappingRet.gemportId,mappingRet.allocId,mappingRet.queueId, mappingRet.dropFlag);
	skb->gem_port = mappingRet.gemportId;//mappingRet.gemportId;
	skb->mark = skb->mark |((mappingRet.queueId & 0x07) << 11); //((mappingRet.queueId & 0x07) << 10);	
	
	return RETURN_RET_SUCCESS;
}


int fhnet_ds_vlan_proc(struct sk_buff *skb, struct net_device *dev)
{
	FHNet_Mapping_Vlan_Para_In_t paraIn;
	FHNet_L3Pkt_Mapping_Vlan_Para_In_t L3paraIn;
	unsigned char inport = 0;
	unsigned char dropFlag = 0;
	struct net_device *l3dev = NULL;
	int ret = 0;
	unchar outPort = 0;
	PWAN_NetPriv_T * pNetPriv;
	
	if(NULL == skb || NULL == dev)
	{
		printk("fhnet_ds_vlan_proc input pointer skb = %x or dev =%x is null\n",skb,dev);
		return RETURN_RET_NORMAL_FAIL;
	}
	
	if(0 == strcmp(skb->dev->name, "omci") ){
		PON_MSG(MSG_ERR,"======pkt from omci, bypass!!========\n");
		return RETURN_RET_SUCCESS;
	}
	
	ret = fhnet_get_port_from_dev(dev, &outPort);
	if(ret == RETURN_RET_NORMAL_FAIL)
	{
		PON_MSG(MSG_ERR,"fhnet_get_port_from_dev return err\n");
		return RETURN_RET_SUCCESS;
	}

	inport = fhnet_get_inport_from_skb(skb);

	memset(&paraIn, 0, sizeof(paraIn));
	memset(&L3paraIn, 0, sizeof(L3paraIn));

	/*if packet not from cpu, call fhnet_ds_vlan_action_hook*/
	if(!fhnet_check_l3_packet(skb, NULL))
	{	
		paraIn.skb = skb;
		paraIn.outPort = outPort;
		paraIn.inPort =  inport;/* inport will be wan if skb->mark port is exist*/
		PON_MSG(MSG_ERR,"L2 DS fhnet_ds_vlan_action_hook dev %s, input outPort %d, inPort %d \n",dev->name,paraIn.outPort, paraIn.inPort);
		if(fhnet_ds_vlan_action_hook != NULL)
		{
			ret = fhnet_ds_vlan_action_hook(&paraIn, &dropFlag);
			if(ret !=RETURN_RET_SUCCESS || dropFlag == FHNET_IS_DROP_TRUE)
			{
				if(gpPonSysData->debugLevel & MSG_ERR) 
				{
					printk("fhnet_ds_vlan_action_hook fail ret %d, dropFlag %d\n", ret, dropFlag);
					printk("fhnet_ds_vlan_action_hook drop packet\n");
					__dump_skb(skb, skb->len) ;
				}
				return RETURN_RET_NORMAL_FAIL;
			}
		}
		else
		{
			PON_MSG(MSG_ERR, "fhnet_ds_vlan_action_hook dev is NULL return\n");
		}

	}
	else/* call another function to process cpu packet */
	{
		if(dev == NULL) 
		{
			PON_MSG(MSG_ERR, "fhnet_get_dev_by_skb is NULL \n");
			return RETURN_RET_NORMAL_FAIL;
		}
		L3paraIn.skb = skb;
		strcpy(L3paraIn.interfaceName,dev->name);
		PON_MSG(MSG_ERR, "DS L3packetInfo.interfaceName %s hook fh L3 ds vlan action\n",L3paraIn.interfaceName);
		if(fhnet_L3_ds_vlan_action_hook != NULL)
		{
			ret = fhnet_L3_ds_vlan_action_hook(&L3paraIn,&dropFlag);
			if(ret != 0 || dropFlag == FHNET_IS_DROP_TRUE)
			{
				if(gpPonSysData->debugLevel & MSG_ERR) 
				{
					printk("ERR fhnet_L3_ds_vlan_action_hook proc ret %d, dropFlag %d \n",ret,dropFlag);
					printk("fhnet_L3_ds_vlan_action_hook drop packet\n");
					__dump_skb(skb, skb->len) ;
				} 
				return RETURN_RET_NORMAL_FAIL;
			}
		}
		else
		{
			PON_MSG(MSG_ERR, "fhnet_L3_ds_vlan_action_hook is NULL return\n");
		}
	}

	return RETURN_RET_SUCCESS;
}

static int ecnt_FhNetDsVlanAction_hook(struct ecnt_data * int_data)
{

	struct net_data_s * net_data = (struct net_data_s *)int_data;
	struct sk_buff * skb = NULL;
	struct net_device * dev = NULL;
	int ret = 0;
	
	if(net_data == NULL)
	{
		printk("%s para is null,line %d\n",__FUNCTION__,__LINE__);
		return ECNT_CONTINUE;
	}
	
	skb = skb_unshare(*(net_data->pskb), GFP_ATOMIC );
	if(skb == NULL)
	{
		printk("%s para is null,line %d\n",__FUNCTION__,__LINE__);
		return ECNT_CONTINUE;
	}

	*(net_data->pskb) = skb;	
	dev = skb->dev;
	
	ret = fhnet_ds_vlan_proc(skb, dev);
	if(ret != 0)
	{
		PON_MSG(MSG_ERR,"fhnet_ds_vlan_proc process fail return drop\n");
		return ECNT_RETURN_DROP;
	}
	PON_MSG(MSG_ERR,"ecnt_FhNetDsVlanAction_hook return %d\n",ECNT_CONTINUE);
	return ECNT_CONTINUE;
   
}

struct ecnt_hook_ops ecnt_FhNetDsVlanAction_op = {
	.name = "ecnt_checkTrafficWan2Wan_hook",
	.is_execute = 1,
	.hookfn = ecnt_FhNetDsVlanAction_hook,
	.maintype = ECNT_NET_CORE_DEV,
	.subtype = ECNT_DEV_QUE_XMIT,
	.priority = 1
};
#endif/*TCSUPPORT_COMPILE*/

#if defined(TCSUPPORT_VOIP) && defined(RTP_RX_SHORTCUT)
/*______________________________________________________________________________
**function name: checkUdpPacketIsRtp
**
**description:
*    check whether the udp packet is rtp or not
**parameters:
*    char *cp, int flag
**global:
*    none
**return:
*    int ret
**call:
*    none
**revision:
*     1.shelven.lu 20150331
**____________________________________________________________________________*/

int checkUdpPacketIsRtp(unsigned char *cp, int flag)
{ 

	int i=0;
	unsigned short srcPortNum=0;
#define MAX_SESSION_NUM 6
	unsigned int *rtp_src_port=NULL;
	int ret=0;
	
	cp += 11;
	srcPortNum = *((u16*)cp);
	if(recv_rtp_src_port_get_hook){
		rtp_src_port=recv_rtp_src_port_get_hook();			
		
		for(i=0; i<MAX_SESSION_NUM; i++)
		{
			if(srcPortNum ==  rtp_src_port[i])
			{
				ret= flag;/*1: dhcp mode, 2: pppoe mode, recv udp packet src port match, is RTP  packet*/
				break;
			}
		}
	}
#undef MAX_SESSION_NUM
return ret;
	
}

/*______________________________________________________________________________
**function name: isRtpPacket
**
**description:
*    check the packet is rtp or not
**parameters:
*    struct sk_buff *skb, unsigned char *vlanLayer, unsigned short *vlanId
**global:
*    none
**return:
*    int ret
**call:
*    checkUdpPacketIsRtp
**revision:
*     1.shelven.lu 20150331
**____________________________________________________________________________*/

int isRtpPacket(struct sk_buff *skb, unsigned char *vlanLayer, unsigned short *vlanId)
{
	unsigned char *cp = skb->data;	
	unsigned char tagNum = 0;
	unsigned short tpid = 0;
	unsigned short etherType= 0;
	int ret=0;
	cp += 12;

	while((tpid =((*cp << 8) + *(cp + 1))) == 0x8100 ||(tpid == 0x88a8)||(tpid == 0x9100))
	{
		tagNum ++;
		if(tagNum == 1)
			*vlanId = *(u16*)(cp+2);
		cp += 4;
	}

	*vlanLayer = tagNum;	
	etherType = *(u16*)cp;	
	/* check ftp test packet */
	if (etherType == htons(ETH_P_IP)) {/* IP */
		cp = skb->data;
		cp += (23+(tagNum<<2));
		if (*cp == 0x11) { /*0x11 for udp ,0x6 for tcp*/
			ret= checkUdpPacketIsRtp(cp,DHCP_MODE);
		}
	}
	else if(etherType == htons(ETH_P_PPP_SES))
	{
		/* pppoe session */
		cp = skb->data;
		cp += (20+(tagNum<<2));
		if(*(u16*)cp == 0x0021)
		{
			/* pppoe session ip protocol */
			cp += 11;
			if (*cp == 0x11) {  /*0x11 for udp ,0x6 for tcp*/
				ret= checkUdpPacketIsRtp(cp,PPPOE_MODE);
			}
		}
	}

	return ret;
}

#endif

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON_GDV20) || defined(TCSUPPORT_IS_CF_PON)
/****************************************************************************
**function name
	 isTR143TestPkt
**description:
	check is tr143 test pakcets
**return 
	0: not, other: ip type  1: IP, 2:PPP, 3:dslite+IP, 4:dslite+PPP 
**parameter:
	skb: the packet information
	vlanLayer: vlan layer counts
	vlanId: vlan id
****************************************************************************/
int isTR143TestPkt(struct sk_buff *skb
, unsigned char *vlanLayer
, unsigned short *vlanId)
{
	unsigned char *cp = skb->data;	
	unsigned char tagNum = 0;
	unsigned short tpid = 0;
	unsigned short etherType= 0;
	unsigned short srcPortNum = 0;
	unsigned char *cp_ds = NULL;


	if ( 0 == tr143_test_enable )
		return 0;

	cp += 12;
	while((tpid =((*cp << 8) + *(cp + 1))) == 0x8100 ||(tpid == 0x88a8)||(tpid == 0x9100))
	{
		tagNum ++;
		if(tagNum == 1)
			*vlanId = *(u16*)(cp+2);
		cp += 4;
	}	
	*vlanLayer = tagNum;	
	etherType = *(u16*)cp;	
	/* check ftp test packet */
	if (etherType == htons(ETH_P_IP)) {/* IP */
		cp = skb->data;
		cp += (23+tagNum*4);
		
		if (*cp == 0x06) { /* tcp */
				cp += 11;
				srcPortNum = *((u16*)cp);
				if ( tr143_test_port == srcPortNum )
					return 1;
		}
	}
	else if (etherType == htons(ETH_P_IPV6)) /* IP6 */
	{
		cp = skb->data;
		cp += (20 + tagNum * 4);
		if ( 0x04 == *cp ) /* IPIP */
		{
			cp += 43;
			if ( 0x06 == *cp ) /* tcp */
			{
				cp += 11;
				srcPortNum = *((u16*)cp);
			
				if ( tr143_test_port == srcPortNum )
					return 3;
			}
		}
	}
	else if(etherType == htons(ETH_P_PPP_SES))
	{
		/* pppoe session */
		cp = skb->data;
		cp += (20+tagNum*4);
		if(*(u16*)cp == 0x0021)
		{
			/* pppoe session ip protocol */
			cp += 11;
			if (*cp == 0x06) { /* tcp */
				cp += 11;
				srcPortNum = *((u16*)cp);

				if ( tr143_test_port == srcPortNum )
					return 2;
			}
		}
		else if(*(u16*)cp == 0x0057) /* IPv6 */
		{
			cp_ds = cp;
			cp_ds += 8;
			if ( 0x04 == *cp_ds ) /* IPIP */
			{
				cp += 51;
				if ( 0x06 == *cp ) /* tcp */
				{
					cp += 11;
					srcPortNum = *((u16*)cp);

					if ( tr143_test_port == srcPortNum )
						return 4;
				}
			}
		}
	}

	return 0;
}
#endif/*TCSUPPORT_COMPILE*/

#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
int isFtpTestPacket(struct sk_buff *skb, unsigned char *vlanLayer, unsigned short *vlanId)
{
	unsigned char *cp = skb->data;	
	unsigned char tagNum = 0;
	unsigned short tpid = 0;
	unsigned short etherType= 0;
	unsigned short srcPortNum = 0;
	cp += 12;
	while((tpid =((*cp << 8) + *(cp + 1))) == 0x8100 ||(tpid == 0x88a8)||(tpid == 0x9100))
	{
		tagNum ++;
		if(tagNum == 1)
			*vlanId = *(u16*)(cp+2);
		cp += 4;
	}	
	*vlanLayer = tagNum;	
	etherType = *(u16*)cp;	
	/* check ftp test packet */
	if (etherType == htons(ETH_P_IP)) {/* IP */
		cp = skb->data;
		cp += (23+tagNum*4);
		
		if (*cp == 0x06) { /* tcp */
				int i=0;
			
				cp += 11;
				srcPortNum = *((u16*)cp);
				
				for(i=0; i<FTP_PORT_NUM; i++)
				{
					if(srcPortNum ==  test_ftp_port[i])
					{
						return 1;// is ftp test packet, tcp dst port match.
					}
				}

		}
	}
	else if(etherType == htons(ETH_P_PPP_SES))
	{
		/* pppoe session */
		cp = skb->data;
		cp += (20+tagNum*4);
		if(*(u16*)cp == 0x0021)
		{
			/* pppoe session ip protocol */
			cp += 11;
			if (*cp == 0x06) { /* tcp */
				int i=0;
			
				cp += 11;
				srcPortNum = *((u16*)cp);
				
				for(i=0; i<FTP_PORT_NUM; i++)
				{
					if(srcPortNum ==  test_ftp_port[i])
					{
						return 2;//pppoe mode, is ftp test packet, tcp dst port match.
					}
				}
			}
		}
	}

	return 0;
}
#endif


/*********************************************************************************************************************
				XPON WAN Internet LED Control
**********************************************************************************************************************/
#if defined(TCSUPPORT_XPON_LED)
static void pwan_net_timer(TIMER_FUN_PAAM data)
{
	unsigned long rx_pkts_diff ,tx_pkts_diff;
	unsigned long rx_pkts,tx_pkts;

	if(TRUE == gpPonSysData->ponMacPhyReset) {
		goto restart_timer;
	}

	if ((gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) ||
		(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON)){

#ifdef TCSUPPORT_RA_HWNAT
	if (internet_hwnat_timer_switch) {		
		rx_pkts = pwanStats.rx_pkts; 	
		tx_pkts = pwanStats.tx_pkts; 		

		FE_API_GET_ETH_FRAME_CNT(FE_GDM_SEL_RX, (uint *)&(pwanStats.rx_pkts));
		FE_API_GET_ETH_FRAME_CNT(FE_GDM_SEL_TX, (uint *)&(pwanStats.tx_pkts));

		rx_pkts_diff = pwanStats.rx_pkts- rx_pkts;
		tx_pkts_diff = pwanStats.tx_pkts- tx_pkts;

		if ((rx_pkts_diff > internet_hwnat_pktnum) ||
			(tx_pkts_diff > internet_hwnat_pktnum)) {
			//printk("\r\npwan_net_timer light led,internet_hwnat_pktnum=%d",internet_hwnat_pktnum);
			/*for interner traffic led*/
#if !defined(TCSUPPORT_C9_ROST_LED)
#if 0		//TODO: control internet
			
			if(internet_led_on && alarm_led_status != ALARM_LED_FLICKER) {//IP connected and IP traffic is passing
				change_internet_led_status(ALARM_LED_FLICKER);
				alarm_led_status = ALARM_LED_FLICKER;
			} 
			else {
				if(!internet_trying_led_on && alarm_led_status != ALARM_LED_OFF) {
					change_internet_led_status(ALARM_LED_OFF);
					alarm_led_status = ALARM_LED_OFF;
				}
			}			
#endif			
#endif			
		}
	}
#endif
	}

restart_timer:
	/* Schedule for the next time */
	gpWanPriv->pwan_timer.expires = jiffies + msecs_to_jiffies(250);
  	//add_timer(&gpWanPriv->pwan_timer); heredbg
  	mod_timer(&gpWanPriv->pwan_timer, (jiffies + msecs_to_jiffies(250)));
}
#endif

void pwan_net_open_internet_led_control(struct net_device *dev)
{
#if defined(TCSUPPORT_XPON_LED)
	/* Schedule timer */
	if(dev == NULL){
		return;
	}
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)	
	if(strcmp(dev->name,CONST_XMCI_DEV_NAME) == 0)
#else/*TCSUPPORT_COMPILE*/
	if(strcmp(dev->name, "pon") == 0)
#endif/*TCSUPPORT_COMPILE*/	
	{
		GPON_CREATE_TIMER(&gpWanPriv->pwan_timer,pwan_net_timer,(jiffies + msecs_to_jiffies(250)));
		add_timer(&gpWanPriv->pwan_timer);
		memset(&pwanStats, 0, sizeof(pwanStats));
	}
#endif

}

void pwan_net_del_internet_led_control(struct net_device *dev)
{
#if defined(TCSUPPORT_XPON_LED)
	if(dev == NULL){
		return;
	}
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)	
	if((strcmp(dev->name, CONST_XMCI_DEV_NAME) == 0) && (dev->flags & IFF_UP))
#else/*TCSUPPORT_COMPILE*/
	if((strcmp(dev->name, "pon") == 0) && (dev->flags & IFF_UP))
#endif/*TCSUPPORT_COMPILE*/	
	{
		del_timer_sync(&gpWanPriv->pwan_timer);
	}
#endif

}
void xpon_wan_tx_internet_led_control(uint8_t netIdx)
{
#if/*TCSUPPORT_COMPILE*/ !defined(TCSUPPORT_C9_ROST_LED)
#if defined(TR068_LED)
	if(netIdx == PWAN_IF_DATA){
		/*for interner traffic led*/
		if(internet_led_on && alarm_led_status != ALARM_LED_FLICKER) //IP connected and IP traffic is passing
		{
			change_internet_led_status(ALARM_LED_FLICKER);
			alarm_led_status = ALARM_LED_FLICKER;
		} 
		else
		{
			if(!internet_trying_led_on && alarm_led_status != ALARM_LED_OFF) 
			{
				change_internet_led_status(ALARM_LED_OFF);
				alarm_led_status = ALARM_LED_OFF;
			}
		}
	}
#endif
#endif/*TCSUPPORT_COMPILE*/

}

void xpon_wan_rx_internet_led_control(uint8_t netIdx)
{
#if defined(TR068_LED)
#if !defined(TCSUPPORT_C9_ROST_LED)
	
	if((netIdx == PWAN_IF_DATA) 
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HYBIRD)
		|| (netIdx == PWAN_IF_WAN0)
		|| (netIdx == PWAN_IF_GPON0)
#endif/*TCSUPPORT_COMPILE*/
	)
	{
		/*for interner traffic led*/
		if(internet_led_on && alarm_led_status != ALARM_LED_FLICKER) //IP connected and IP traffic is passing
		{
			change_internet_led_status(ALARM_LED_FLICKER);
			alarm_led_status = ALARM_LED_FLICKER;
		} 
		else
		{
			if(!internet_trying_led_on && alarm_led_status != ALARM_LED_OFF) 
			{
				change_internet_led_status(ALARM_LED_OFF);
				alarm_led_status = ALARM_LED_OFF;
			}
		}
	}
#endif
#endif
}

/*********************************************************************************************************************
				XPON WAN SKB VLAN, GEMPORT MAP Handle
**********************************************************************************************************************/

int gwan_tx_vlan_gem_map_handler_customer(struct sk_buff *skb, uint8_t netIdx)
{
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
	if(PWAN_IF_DATA == netIdx) {
		/* proc vlan trans before gem port mapping */
		if (0 != FH_VLAN_TX_TRANS(skb)){
			XPON_DROP_PRINT;
			return -1;
		}
	}
#endif/*TCSUPPORT_COMPILE*/
	
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_FWC_ENV)
	if(0 != fhnet_gwan_vlan_mapping_proc(skb))
	{
		XPON_DROP_PRINT;
		return -1;
	}
#endif/*TCSUPPORT_COMPILE*/
		
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
	if(PWAN_IF_DATA == netIdx) {
		/* proc wlan before add to HW_NAT */
		FH_VLAN_TX_PROC(skb);
	}
#endif/*TCSUPPORT_COMPILE*/
	return 0;
}
int ewan_tx_vlan_handler_customer(struct sk_buff *skb, uint8_t netIdx)
{
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
	if(PWAN_IF_DATA == netIdx)
	{
		/* proc wlan before add to HW_NAT */
		if (0 != fb_pwan_tx_vlan_trans_proc(skb)){
			PON_MSG(MSG_ERR, "vlan trans error\n") ;
			return -1;
		}
		fb_pwan_tx_vlan_proc(skb);
	}
#endif/*TCSUPPORT_COMPILE*/	
	return 0;
}

int xpon_wan_tx_vlan_handle_customer(struct sk_buff *skb, uint8_t netIdx)
{
	int ret = ECNT_HOOK_ERROR;
#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	if((PWAN_IF_DATA == netIdx ) && sw_pktqosEnqueue){
		if(sw_pktqosEnqueue(skb) == 1)
			return ECNT_RETURN;
			
    #if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_HWNAT_OFFLOAD)
		if((skb->qosEnque & 0x02) == 0){
			skb->qosEnque |= (skb->mark& 0x000000f0);
		}
		if((skb->qosEnque & 0x02) == 0x02){
			skb->mark |= (skb->qosEnque & 0xf0);
		}				
    #endif/*TCSUPPORT_COMPILE*/
	}
	ret = ECNT_CONTINUE;
#endif
	
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
	FH_VLAN_OPERATION(skb);
	FH_DSCP_OPERATION(skb);
	ret = ECNT_CONTINUE;
#endif/*TCSUPPORT_COMPILE*/
	return ret;
}
/*ret = -1 : goto drop*/
int xpon_wan_rx_vlan_handle_customer(struct sk_buff *skb, uint8_t netIdx)
{
	int ret = 0;
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
	if(PWAN_IF_DATA == netIdx){
		if(0 != FH_VLAN_FILTER(skb)){
			XPON_DROP_PRINT;
			ret = -1;
		}
/* rx data packet do vlan proc after Add to HW_NAT */
		
		ret = FH_VLAN_RX_PROC(skb);
		if(-1 == ret){
			printk("fb_pwan_rx_vlan_proc  fail free skb, return\n");
			XPON_DROP_PRINT;
			ret = -1;
		}
		
		FH_VLAN_PARSER(skb);
	}
#endif/*TCSUPPORT_COMPILE*/

	return ret;
}

/*********************************************************************************************************************
				XPON WAN SKB QoS, Special Packet Handle
**********************************************************************************************************************/
/*this function may not be used*/
int xpon_wan_rx_rtp_handler(struct sk_buff *skb)
{
#if (defined(TCSUPPORT_VOIP) && defined(RTP_RX_SHORTCUT)) || (defined(TCSUPPORT_CPU_PERFORMANCE_TEST)) || defined(TCSUPPORT_CT_PON_GDV20) || defined(TCSUPPORT_IS_CF_PON)	
	unsigned char vlanLayer=0;
	unsigned short vlanId=0;
#endif
#if (defined(TCSUPPORT_VOIP) && defined(RTP_RX_SHORTCUT)) || (defined(TCSUPPORT_CPU_PERFORMANCE_TEST))
	int isRtpFlag = 0; /*1: ip mode rtp  packet 2: pppoe mode rtp  packet*/
#endif
#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
	int ftpTestFlag = 0; // 1: ip mode ftp test packet 2: pppoe mode ftp test packet
#endif
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON_GDV20) || defined(TCSUPPORT_IS_CF_PON)
	int isTR143Test = 0;
#endif/*TCSUPPORT_COMPILE*/

	
#if defined(TCSUPPORT_VOIP) && defined(RTP_RX_SHORTCUT)
	isRtpFlag = isRtpPacket(skb, &vlanLayer, &vlanId);

#endif	
#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
	if(isRtpFlag == 0)
	{
		ftpTestFlag = isFtpTestPacket(skb, &vlanLayer, &vlanId);
	}
#endif			
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON_GDV20) || defined(TCSUPPORT_IS_CF_PON)
	if ( tr143_test_enable )
		isTR143Test = isTR143TestPkt(skb, &vlanLayer, &vlanId);
#endif/*TCSUPPORT_COMPILE*/

#if defined(TCSUPPORT_VOIP) && defined(RTP_RX_SHORTCUT)
    if(isRtpFlag)
    {
        rtpRxShortCut(skb, vlanLayer, isRtpFlag);
    }
    else
#endif
#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
    if(ftpTestFlag)
    {
        ftpRxShortCut(skb, vlanLayer, vlanId, ftpTestFlag);
    }
    else
#endif
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON_GDV20) || defined(TCSUPPORT_IS_CF_PON)
    if ( isTR143Test
        && 0 == tr143RxShortCut(tr143_test_enable, skb, vlanLayer
        , tr143_test_iface, isTR143Test
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_IS_CF_PON)
, recv_iface
#endif/*TCSUPPORT_COMPILE*/
) )
    {
        isTR143Test = 0;
    }
#endif/*TCSUPPORT_COMPILE*/

	return 0;
}

/*ret != 0 : goto success_ret*/
int xpon_wan_rx_l2_ppp_mcast_shortcut(struct sk_buff *skb)
{
	int isShortCut = 0;
#if defined(TCSUPPORT_L2_PPP_MCAST_SHORTCUT)
	if(pppoeBridgeMultiShortCut == 1)
	{
		isShortCut = pppoe_bridge_multicast_handle(skb);
	}
#endif
	return isShortCut;
}
/*ret = -1 : goto drop*/
int xpon_wan_rx_traffic_classify(struct sk_buff *skb, uint8_t netIdx)
{
	int ret = 0;
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_FWC_TRAFFIC_CLASSIFY) || defined(TCSUPPORT_CMCCV2)
	unsigned char traffic_classify_flag = E_ECNT_FORWARD;
	if(PWAN_IF_DATA == netIdx)
	{
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CMCCV2)
		skb->mark2 &= (~(QUEUE_MARK_MASK));
		skb->mark2 |= QUEUE_FROM_WAN;
#else/*TCSUPPORT_COMPILE*/
		skb->mark &= (~(PORT_MASK << PORT_MASK_OFFSET));
		skb->mark |= ((E_WAN_PORT & PORT_MASK) << PORT_MASK_OFFSET);
#endif/*TCSUPPORT_COMPILE*/
		ECNT_TRAFFIC_CLASSIFY_HOOK(skb, &traffic_classify_flag);
		if(E_ECNT_DROP == traffic_classify_flag)
		{
#if defined(TCSUPPORT_RA_HWNAT) && defined(TCSUPPORT_RA_HWNAT_ENHANCE_HOOK)
			if(ra_sw_nat_hook_drop_packet)
			{
				ra_sw_nat_hook_drop_packet(skb);
			}
#endif
			ret = -1;
		}

	}
#endif/*TCSUPPORT_COMPILE*/
	return ret;
}
/******************************************************************************************************************
					proc create function
********************************************************************************************************************/
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
static ssize_t pon_hw_qos_enable_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char val_string[8];
	//char* endpo = NULL; 
	long ret = 0;
		
	if (count > sizeof(val_string) - 1)
	{
		ret = -EFAULT;
		return ret;
	}
	memset(val_string,0,sizeof(val_string));
	if (count > sizeof(val_string) - 1)
		return -EINVAL;
	
	if (copy_from_user(val_string, buf, count))
	{
		ret = -EFAULT;
		return ret;
	}
	val_string[count] = '\0';

	sscanf(val_string, "%u", &hw_qos_enable);
	
	//hw_qos_enable = simple_strtol(val_string,&endpo,10);

	if(hw_qos_enable)
	{
		gpWanPriv->devCfg.flags.isQosUp = 1;
		//todo: not implement yet

	}
	else
	{
		gpWanPriv->devCfg.flags.isQosUp = 0;
		//todo: not implement yet

	}
	
	return count;
}

static ssize_t pon_hw_qos_enable_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	printk("%d\n",hw_qos_enable);
	
	return 0;
}
#else
static int pon_hw_qos_enable_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[8];
	//char* endpo = NULL; 
	long ret = 0;
		
	if (count > sizeof(val_string) - 1)
	{
		ret = -EFAULT;
		return ret;
	}
	memset(val_string,0,sizeof(val_string));
	if (count > sizeof(val_string) - 1)
		return -EINVAL;
	
	if (copy_from_user(val_string, buffer, count))
	{
		ret = -EFAULT;
		return ret;
	}
	val_string[count] = '\0';

	sscanf(val_string, "%u", &hw_qos_enable);
	
	//hw_qos_enable = simple_strtol(val_string,&endpo,10);

	if(hw_qos_enable)
	{
		gpWanPriv->devCfg.flags.isQosUp = 1;
		//todo: not implement yet

	}
	else
	{
		gpWanPriv->devCfg.flags.isQosUp = 0;
		//todo: not implement yet

	}
	
	return count;
}

static int pon_hw_qos_enable_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	printk("%d\n",hw_qos_enable);
	
	return 0;
}
#endif

#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
static int pon_trafficShapingSet_write_proc(
		struct file *file, const char *buffer, 
		unsigned long count, void *data
)
{
	printk("\npon_trafficShapingSet_write_proc EN7521 is not support\n");
	
	return 0;
}

static int pon_trafficShapingSet_read_proc(
		char *buf, char **start, off_t off, int count,
        int *eof, void *data
)
{
	printk("\n pon_trafficShapingSet_read_proc EN7521 is not support\n");
	
	return 0;
}
#if defined (QOS_REMARKING) || defined (TCSUPPORT_HW_QOS)
static int pon_qoswrr_read_proc(
		char *page, char **start, off_t off,
		int count, int *eof, void *data
)
{
	/* qos_wrr_info[0]: 0:SP, 1:WRR 
	 * qos_wrr_info[1]: txq3 weight for WRR
	 * qos_wrr_info[0]: txq2 weight for WRR
	 * qos_wrr_info[0]: txq1 weight for WRR
	 * qos_wrr_info[0]: txq0 weight for WRR 
	 */
	printk("%d %d %d %d %d\n", *qos_wrr_info, 
		*(qos_wrr_info + 1), *(qos_wrr_info + 2), 
		*(qos_wrr_info + 3), *(qos_wrr_info + 4));
		
	return 0;
}


/* when setting QoS via webpage, cfg_manager will
 * "echo ..... > /proc/tc3162/ptm_qoswrr" to do
 * HW QDMA QoS config */
static int pon_qoswrr_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data
)
{
	int len, i;
	char get_buf[33];
	int max_wrr_val = 0;
	int path, txq;
	QDMA_TxQosScheduler_T txQos;
	int ret;

	if(count > 32)
		len = 32;
	else
		len = count;
		
	/* use the copy_from_user function to copy 
	 * buffer data to our get_buf */
	if(copy_from_user(get_buf, buffer, len))
		return -EFAULT;
		
	/* zero terminate get_buf */
	get_buf[len]='\0';


	/* qos_wrr_info[0]: 0:SP, 1:WRR 
	 * qos_wrr_info[1]: txq3 weight for WRR
	 * qos_wrr_info[2]: txq2 weight for WRR
	 * qos_wrr_info[3]: txq1 weight for WRR
	 * qos_wrr_info[4]: txq0 weight for WRR */
	if(sscanf(get_buf, "%d %d %d %d %d", qos_wrr_info, 
			(qos_wrr_info+1), (qos_wrr_info+2), 
			(qos_wrr_info+3), (qos_wrr_info+4)) != 5)
		return count;


	/* find the txq with max qos wrr weight */
	for (i = 0; i < 4; i++) {
		if (max_wrr_val < qos_wrr_info[i + 1]) {
			max_wrr_val = qos_wrr_info[i + 1];
			maxPrio = 3 - i;
		}
	}


	/* Strict Priority */
	if(*qos_wrr_info == 0) 
	{
		/* set QDMA as Strict Priority Mode,
	 	 * so that p7> ... >p0 for all paths */
		for (path = 0; path < PATH_NUM; path++)
		{
			memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
			txQos.channel = path;
			txQos.qosType = QDMA_TXQOS_TYPE_SP;

			ret = QDMA_API_SET_TX_QOS(ECNT_QDMA_WAN, &txQos);
			if (ret)
			{
				printk("\nFAILED(%s): qdma strict priority setting for path%d\n", __FUNCTION__, path);
				return -1;
			}
		}

		printk("\nSP setting for txq0~3 is done\n\n");

	}
	/* Weighted Round Robin */
	else 
	{
		/* set QDMA's SP & WRR priority registers,
	 	 * so that p7>p6>p5>p4> p3:p2:p1:p0 for path 0~7 && 
		 * p3:p2:p1:p0 == qos_wrr_info[1]:qos_wrr_info[2]:
		 * 				  qos_wrr_info[3]:qos_wrr_info[4] */
		for (path = 0; path < PATH_NUM; path++)
		{
			memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
			txQos.channel = path;
			txQos.qosType = QDMA_TXQOS_TYPE_SPWRR4;
			for(txq = 0 ; txq < 4; txq++)
				txQos.queue[txq].weight = qos_wrr_info[4-txq];

			ret = QDMA_API_SET_TX_QOS(ECNT_QDMA_WAN, &txQos);
			if (ret)
			{
				printk("\nFAILED(%s): qdma wrr setting for path%d\n" , __FUNCTION__, path);
				return -1;
			}
		}

		//take path0's WRR setting as an example to show
		memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
		txQos.channel = 0;
		QDMA_API_GET_TX_QOS(ECNT_QDMA_WAN, &txQos);
		printk("\nWRR setting for txq0~3 is done:\n" 
		"(Type:%d) Q3:%d, Q2:%d, Q1:%d, Q0:%d\n\n",
					txQos.qosType, 
					txQos.queue[3].weight,
					txQos.queue[2].weight,
					txQos.queue[1].weight,
					txQos.queue[0].weight);
	}
		
	return len;
}
#endif


#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
int isDefaultQueue(struct sk_buff *skb)
{
	int queue;
	queue = skb->mark & QOS_FILTER_MARK;
		
	if(queue == 0){
				return 1;
	}
	
	if(gQueueMask & (1 << ((queue >> 4) -1 ))){ 	//default queue
		return 0;
	}else{
		return 1;
	}
}

int isWeigt0(struct sk_buff *skb)
{
	int queue;
	
	queue = (skb->mark & QOS_FILTER_MARK) >> 4;

	if(queue < 1 || queue >4){	//wrr queue is from 1 to 4
		return 0;
	}

	if(qos_wrr_info[queue]){
		return 0;
	}else{				//weight is 0
		return 1;
	}	
}

#endif
#ifdef TCSUPPORT_QOS
/* when setting QoS via webpage, cfg_manager will
 * "echo ..... > /proc/tc3162/pon_tcqos_disc" to do
 * tcqos_disc config */
static int pon_tcqos_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data
)
{
	char qos_disc[11];
	int len;

	if (count > 10) {
		len = 10;
	}
	else {
		len = count;
	}
	memset(qos_disc, 0, sizeof(qos_disc));
	
	if(copy_from_user(qos_disc, buffer, len - 1))
		return -EFAULT;

	qos_disc[len] = '\0';

#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	isNeedHWQoS = 1;
#endif

	if (!strcmp(qos_disc, "HWWRR")) {
		qosFlag = QOS_HW_WRR;
		printk("\nqos discipline is HW WRR.\n\n");
	}
	else if (!strcmp(qos_disc, "HWPQ")) {
		qosFlag = QOS_HW_PQ;
		printk("\nqos discipline is HW PQ.\n\n");
	}
#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	else if (!strcmp(qos_disc, "HWCAR")) {
		qosFlag = QOS_HW_CAR;
		printk("\nqos discipline is HW CAR.\n\n");
	}
#endif
	else {
		qosFlag = NULLQOS;
		printk("\nqos discipline is disabled.\n\n");
	}

	return len;
}
#endif

#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
static int pon_tcqos_read_proc(
			char *page, char **start, off_t off,
			int count, int *eof, void *data
)
{
	if(qosFlag == QOS_HW_WRR){
		printk("\nqos discipline is HW WRR.\n\n");
	}else if(qosFlag == QOS_HW_PQ){
		printk("\nqos discipline is HW PQ.\n\n");
	}else if(qosFlag == QOS_HW_CAR){
		printk("\nqos discipline is HW CAR.\n\n");
	}else {
		printk("\nqos discipline is disabled.\n\n");
	}

	printk("\n isNeedHWQoS =%d.\n\n",isNeedHWQoS);

	return 0;
}



static int pon_tcqos_queuemask_read_proc(
			char *page, char **start, off_t off,
			int count, int *eof, void *data
)
{
	printk("\nqos gQueueMask is 0x%x\n",gQueueMask);		
	return 0;
}


static int pon_tcqos_queuemask_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data
)
{
	char valString[4];
	int val = 0;
	
	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EINVAL;

	valString[count] = '\0';

	sscanf(valString, "%d", &val);

	if (val)
	{
		gQueueMask = val;
		printk("\nqos gQueueMask is 0x%x\n",gQueueMask);
	}

	return count;

}
#endif
#endif

#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
static int pon_qos_stop_read_proc(
		char *page, char **start, off_t off,
		int count, int *eof, void *data
){
	printk("%d\n",qos_stop_enable);
	return 0;
}

static int pon_qos_stop_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char get_buf[9];
	int len;

	if (count > 8)
		len = 8;
	else 
		len = count;
	
	memset(get_buf, 0, sizeof(get_buf));
	
	if(copy_from_user(get_buf, buffer, len - 1))
		return -EFAULT;

	get_buf[len] = '\0';

	sscanf(get_buf, "%c", &qos_stop_enable);
	
	return len;
	
}

static int pon_qos_stop_num_read_proc(
		char *page, char **start, off_t off,
		int count, int *eof, void *data
){
	printk("%d\n",qos_stop_num);
	return 0;
}

static int pon_qos_stop_num_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char get_buf[9];
	int len;

	if (count > 8)
		len = 8;
	else 
		len = count;
	
	memset(get_buf, 0, sizeof(get_buf));
	
	if(copy_from_user(get_buf, buffer, len - 1))
		return -EFAULT;

	get_buf[len] = '\0';

	sscanf(get_buf, "%d", &qos_stop_num);
	
	return len;
	
}

#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
extern int xmcs_set_qos_policer_creat(struct XMCS_QosPolicerCreat_S *pQosPolicer);
extern int xmcs_set_qos_policer_delete(struct XMCS_QosPolicerDelete_S *pQosPolicerDelete);
extern int xmcs_set_qos_overall_ratelimit(struct XMCS_OverallRatelimitConfig_S *pOverallRatelimit);
static int qos_creat_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char valString[16];
	struct XMCS_QosPolicerCreat_S PolicerCreat;
	
	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d %d", &PolicerCreat.policer_id, &PolicerCreat.cir);

    xmcs_set_qos_policer_creat(&PolicerCreat);

	return  count;
}

static int qos_delete_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char valString[4];
	struct XMCS_QosPolicerDelete_S PolicerDelete;
	
	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d", &PolicerDelete.policer_id);

    xmcs_set_qos_policer_delete(&PolicerDelete);

	return  count;
}

static int qos_totalbandwidth_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char valString[16];
	struct  XMCS_OverallRatelimitConfig_S OverallRatelimit;
	
	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d", &OverallRatelimit.bandwidth);

    xmcs_set_qos_overall_ratelimit(&OverallRatelimit);

	return  count;
}
#endif
#endif

#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
static int pon_add_ftp_port_read_proc(
		char *page, char **start, off_t off,
		int count, int *eof, void *data
){
	int i=0;
	for(i=0; i<FTP_PORT_NUM; i++)
	{
		printk("test ftp port %d: %d\n", i, test_ftp_port[i]);
	}

	return 0;
}

static int pon_add_ftp_port_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char get_buf[8];
	int len;
	int i=0;
	unsigned int add_port = 0;

	if (count > 8)
		len = 8;
	else 
		len = count;
	
	memset(get_buf, 0, sizeof(get_buf));
	
	if(copy_from_user(get_buf, buffer, len - 1))
		return -EFAULT;

	get_buf[len] = '\0';
	sscanf(get_buf, "%d", &add_port);

	if(add_port == 0)
	{
		for(i=0; i<FTP_PORT_NUM; i++)
		{
			test_ftp_port[i] = 0; 
		}
	}
	else
	{
	
		for(i=0; i<FTP_PORT_NUM; i++)
		{
			if(test_ftp_port[i] == 0)
			{
				test_ftp_port[i] = add_port;
			
				printk("open ftp port: %d\n", test_ftp_port[i]);		
				break;
			}
		}
	}
	
	return len;
	
}

static int pon_del_ftp_port_read_proc(
		char *page, char **start, off_t off,
		int count, int *eof, void *data
){
	int i=0;
	for(i=0; i<FTP_PORT_NUM; i++)
	{
		printk("test ftp port %d: %d\n", i, test_ftp_port[i]);
	}

	return 0;
}

static int pon_del_ftp_port_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char get_buf[8];
	int len;
	int i=0;
	unsigned int del_port = 0;
	

	if (count > 8)
		len = 8;
	else 
		len = count;
	
	memset(get_buf, 0, sizeof(get_buf));
	
	if(copy_from_user(get_buf, buffer, len - 1))
		return -EFAULT;

	get_buf[len] = '\0';
	sscanf(get_buf, "%d", &del_port);

	for(i=0; i<FTP_PORT_NUM; i++)
	{
		if(test_ftp_port[i] == del_port)
		{
			test_ftp_port[i] = 0;
			break;
		}
	}
	printk("close ftp port: %d\n", del_port);
	
	return len;
	
}

#endif


#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON_GDV20) || defined(TCSUPPORT_IS_CF_PON)
/****************************************************************************
**function name
	 tr143_test_write_proc
**description:
	tr143 test write proc
**return 
	int
**parameter:
****************************************************************************/
static int tr143_test_write_proc(struct file *file, const char *buffer, 
	unsigned long count, void *data)
{	
	char buff[64] = {0};
	__be32 len = 0;
	__be32 v1 = 0, v2 = 0;
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_IS_CF_PON)
	char iface[32] = {0};
#else/*TCSUPPORT_COMPILE*/
	__be32 v3 = 0;
#endif/*TCSUPPORT_COMPILE*/

	if ( count >= sizeof(buff) )
		len = sizeof(buff) - 1;
	else
		len = count;

	memset(buff, 0, sizeof(buff));
	if ( copy_from_user(buff, buffer, len) )
		return -EFAULT;

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_IS_CF_PON)
	sscanf(buff, "%u %u %s", &v1, &v2, iface);
	tr143_test_enable = v1;
	tr143_test_port = v2;
	snprintf(recv_iface, sizeof(recv_iface) - 1, "%s", iface);
#else/*TCSUPPORT_COMPILE*/
	sscanf(buff, "%u %u %u", &v1, &v2, &v3);
	tr143_test_enable = v1;
	tr143_test_iface = v2;
	tr143_test_port = v3;
#endif/*TCSUPPORT_COMPILE*/

	return len;
}
/****************************************************************************
**function name
	 tr143_test_read_proc
**description:
	tr143 test read proc
**return 
	int
**parameter:
****************************************************************************/
static int tr143_test_read_proc(char *buf, char **start, off_t off,
	int count,int *eof, void *data)
{
	int len = 0;

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_IS_CF_PON)
	len = sprintf(buf, "enable:%u iface:%s port:%u\n", tr143_test_enable
								, recv_iface
								, tr143_test_port);
#else/*TCSUPPORT_COMPILE*/
	len = sprintf(buf, "enable:%u iface:%u port:%u\n", tr143_test_enable
								, tr143_test_iface
								, tr143_test_port);
#endif/*TCSUPPORT_COMPILE*/

	*start = buf + off;
	if (len < off + count)
		*eof = 1;
	len -= off;
	if (len > count)
		len = count ;
	if (len <0)
		len = 0;

	return len;
}
#endif/*TCSUPPORT_COMPILE*/

#if defined(TCSUPPORT_L2_PPP_MCAST_SHORTCUT)
static int pppoe_multicast_shortcut_read_proc(
		char *page, char **start, off_t off,
		int count, int *eof, void *data
){
	printk("pppoe bridge multicast short cut: %d\n",pppoeBridgeMultiShortCut);
	return 0;
}

static int pppoe_multicast_shortcut_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data)
{
	char get_buf[8];
	int len;

	if (count > 8)
		len = 8;
	else 
		len = count;
	
	memset(get_buf, 0, sizeof(get_buf));
	
	if(copy_from_user(get_buf, buffer, len - 1))
		return -EFAULT;

	get_buf[len] = '\0';

	sscanf(get_buf, "%d", &pppoeBridgeMultiShortCut);
	
	return len;
	
}
#endif

struct proc_dir_entry *customer_proc_dir = NULL;

int customer_proc_create(void)
{
#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
	struct proc_dir_entry *ftpTestProc=NULL;
#endif
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON_GDV20) || defined(TCSUPPORT_IS_CF_PON)
	struct proc_dir_entry *tr143Proc=NULL;
#endif/*TCSUPPORT_COMPILE*/
	struct proc_dir_entry *ponProc;

#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
	ponProc = proc_create("qos/pon_hw_qos_enable", 0, NULL, &proc_hw_qos_enable_ops);
	if (!ponProc)
	{
		printk("\nFAILED: create proc for pon_hw_qos_enable\n");
		return -ENOMEM;
	}
#else
    if (customer_proc_dir){
        return 0;
    }

	customer_proc_dir = proc_mkdir("qos", NULL);
    if (NULL == customer_proc_dir){
        return -1;
    }
	ponProc = create_proc_entry("pon_hw_qos_enable", 0, customer_proc_dir);
	if (!ponProc)
	{
		printk("\nFAILED: create proc for pon_hw_qos_enable\n");
		return -ENOMEM;
	}
	ponProc->write_proc = pon_hw_qos_enable_write_proc;
	ponProc->read_proc = pon_hw_qos_enable_read_proc;
#endif
#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)		
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc = create_proc_entry("qos/pon_trafficShapingSet", 0, NULL);
#else 	
	ponProc = create_proc_entry("tc3162/pon_trafficShapingSet", 0, NULL);
#endif 
	if (!ponProc)
	{
		printk("\nFAILED: create proc for pon_trafficShapingSet\n");
		return -ENOMEM;
	}
	ponProc->write_proc = pon_trafficShapingSet_write_proc;
	ponProc->read_proc = pon_trafficShapingSet_read_proc;
#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc = create_proc_entry("qos/pon_qoswrr", 0, NULL);
#else
	ponProc = create_proc_entry("tc3162/pon_qoswrr", 0, NULL);
#endif		
	if (!ponProc)
	{
		printk("\nFAILED: create proc for pon_qoswrr\n");
		return -ENOMEM;
	}
	ponProc->read_proc = pon_qoswrr_read_proc;
	ponProc->write_proc = pon_qoswrr_write_proc;
#endif

#ifdef TCSUPPORT_QOS
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc = create_proc_entry("qos/pon_tcqos_disc", 0, NULL);
#else 
	ponProc = create_proc_entry("tc3162/pon_tcqos_disc", 0, NULL);
#endif 	
	if (!ponProc)
	{
		printk("\nFAILED: create proc for pon_tcqos_disc\n");
		return -ENOMEM;
	}
	ponProc->write_proc = pon_tcqos_write_proc; 		
#endif

#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc->read_proc = pon_tcqos_read_proc;
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc = create_proc_entry("qos/pon_tcqos_queuemask", 0, NULL);
#else 		
	ponProc = create_proc_entry("tc3162/pon_tcqos_queuemask", 0, NULL);
#endif 	
	if (!ponProc)
	{
		printk("\nFAILED: create proc for pon_tcqos_queuemask\n");
		return -ENOMEM;
	}

	ponProc->read_proc = pon_tcqos_queuemask_read_proc;
	ponProc->write_proc = pon_tcqos_queuemask_write_proc;


#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc = create_proc_entry("qos/pon_tcqos_dbg", 0, NULL);
#else
	ponProc = create_proc_entry("tc3162/pon_tcqos_dbg", 0, NULL);
#endif

	if (!ponProc)
	{
		printk("\nFAILED: create proc for pon_tcqos_dbg\n");
		return -ENOMEM;
	}
#endif
#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc = create_proc_entry("qos/qos_stop_enable", 0, NULL);
#else
	ponProc = create_proc_entry("tc3162/qos_stop_enable", 0, NULL);
#endif 	
	if(!ponProc){
		printk("\nFAILED: create proc for qos_stop_enable\n");
		return -ENOMEM;
	}
	ponProc->read_proc = pon_qos_stop_read_proc;
	ponProc->write_proc = pon_qos_stop_write_proc;
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	ponProc = create_proc_entry("qos/qos_stop_num", 0, NULL);
#else 
	ponProc = create_proc_entry("tc3162/qos_stop_num", 0, NULL);
#endif 
	if(!ponProc){
		printk("\nFAILED: create proc for qos_stop_num\n");
		return -ENOMEM;
	}
	ponProc->read_proc = pon_qos_stop_num_read_proc;
	ponProc->write_proc = pon_qos_stop_num_write_proc;
#endif

#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
		ponProc = create_proc_entry("qos/qos_creat", 0, NULL);
		if (!ponProc)
		{
			printk("\nFAILED: create proc for qos_creat\n");
			return -ENOMEM;
		}
		ponProc->write_proc = qos_creat_write_proc; 	

		ponProc = create_proc_entry("qos/qos_delete", 0, NULL);
		if (!ponProc)
		{
			printk("\nFAILED: create proc for qos_delete\n");
			return -ENOMEM;
		}
		ponProc->write_proc = qos_delete_write_proc;	

		ponProc = create_proc_entry("qos/qos_totalbandwidth", 0, NULL);
		if (!ponProc)
		{
			printk("\nFAILED: create proc for qos_totalbandwidth\n");
			return -ENOMEM;
		}
		ponProc->write_proc = qos_totalbandwidth_write_proc; 
#endif

#endif

#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
	ftpTestProc = create_proc_entry("tc3162/add_ftp_port", 0, NULL);
	if(!ftpTestProc){
		printk("\nFAILED: create proc for add_ftp_port\n");
		return -ENOMEM;
	}
	ftpTestProc->read_proc = pon_add_ftp_port_read_proc;
	ftpTestProc->write_proc = pon_add_ftp_port_write_proc;

	ftpTestProc = create_proc_entry("tc3162/del_ftp_port", 0, NULL);
	if(!ftpTestProc){
		printk("\nFAILED: create proc for del_ftp_port\n");
		return -ENOMEM;
	}
	ftpTestProc->read_proc = pon_del_ftp_port_read_proc;
	ftpTestProc->write_proc = pon_del_ftp_port_write_proc;	
#endif

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON_GDV20) || defined(TCSUPPORT_IS_CF_PON)
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_IS_CF_PON)
	tr143Proc = create_proc_entry("tc3162/shortcut_test", 0, NULL);
#else/*TCSUPPORT_COMPILE*/
	tr143Proc = create_proc_entry("tc3162/tr143_test", 0, NULL);
#endif/*TCSUPPORT_COMPILE*/
	if ( !tr143Proc )
	{
		printk("\nFAILED: create proc for tr143_test\n");
		return -ENOMEM;
	}
	tr143Proc->read_proc = tr143_test_read_proc;
	tr143Proc->write_proc = tr143_test_write_proc;
#endif/*TCSUPPORT_COMPILE*/

#if defined(TCSUPPORT_L2_PPP_MCAST_SHORTCUT)
	ponProc = create_proc_entry("tc3162/pppoemultishortcut", 0, NULL);

	if(!ponProc){
		printk("\nFAILED: create proc for pppoemultishortcut\n");
		return -ENOMEM;
	}
	ponProc->read_proc = pppoe_multicast_shortcut_read_proc;
	ponProc->write_proc = pppoe_multicast_shortcut_write_proc;
#endif

	return 0;
}

void customer_proc_remove(void)
{
    if (customer_proc_dir)
    {
        remove_proc_entry("pon_hw_qos_enable", customer_proc_dir);
#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
        remove_proc_entry("qos/pon_trafficShapingSet",NULL);
#else
        remove_proc_entry("tc3162/pon_trafficShapingSet",NULL);
#endif

#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
        remove_proc_entry("qos/pon_qoswrr",NULL);
#else
        remove_proc_entry("tc3162/pon_qoswrr",NULL);
#endif
#endif

#ifdef TCSUPPORT_QOS
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
        remove_proc_entry("qos/pon_tcqos_disc",NULL);
#else
        remove_proc_entry("tc3162/pon_tcqos_disc",NULL);
#endif
#endif

#if defined(TCSUPPORT_CT_HWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
        remove_proc_entry("qos/pon_tcqos_queuemask",NULL);
#else
        remove_proc_entry("tc3162/pon_tcqos_queuemask",NULL);
#endif

#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
        remove_proc_entry("qos/pon_tcqos_dbg",NULL);
#else
        remove_proc_entry("tc3162/pon_tcqos_dbg",NULL);
#endif
#endif

#if defined(TCSUPPORT_CT_SWQOS) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
        remove_proc_entry("qos/qos_stop_enable",NULL);
#else
        remove_proc_entry("tc3162/qos_stop_enable",NULL);
#endif
#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
        remove_proc_entry("qos/qos_stop_num",NULL);
#else
        remove_proc_entry("tc3162/qos_stop_num",NULL);
#endif
#endif

#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
        remove_proc_entry("qos/qos_creat",NULL);
        remove_proc_entry("qos/qos_delete",NULL);
        remove_proc_entry("qos/qos_totalbandwidth",NULL);
#endif
#endif

#if defined(TCSUPPORT_CPU_PERFORMANCE_TEST)
        remove_proc_entry("tc3162/add_ftp_port",NULL);
        remove_proc_entry("tc3162/del_ftp_port",NULL);
#endif
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON_GDV20) || defined(TCSUPPORT_IS_FH_PON)
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_IS_FH_PON)
        remove_proc_entry("tc3162/shortcut_test",NULL);
#else/*TCSUPPORT_COMPILE*/
        remove_proc_entry("tc3162/tr143_test",NULL);
#endif/*TCSUPPORT_COMPILE*/
#endif/*TCSUPPORT_COMPILE*/
#if defined(TCSUPPORT_L2_PPP_MCAST_SHORTCUT)
        remove_proc_entry("tc3162/pppoemultishortcut",NULL);
#endif
    }
    remove_proc_entry("qos",NULL);
}



