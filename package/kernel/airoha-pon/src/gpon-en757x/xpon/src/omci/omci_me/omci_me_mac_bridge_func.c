/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	omci_me_mac_bridge_func.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	wayne.lee	2012/7/28	Create
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
//#include "../../../version/tcversion.h"
#include "omci.h"
#include "omci_me.h"
#include "omci_types.h"
#include "omci_general_func.h"
#include "omci_me_mac_bridge.h"
#include "omci_me_multicast.h"
#include "omci_me_vlan.h"

#ifdef TCSUPPORT_MT7530_SWITCH_API
#include "../../../../ethcmd/libmii.h"
#include "../../../../lib/mtkswitch_api.h"
#endif
//#include "../../public/bridge-utils-1.0.6/libbridge/libbridge.h"
#include "omci_me_gemport_mapping.h"
#include "omci_me_ani_management.h"

/* global variable */
uint16 validAniPortFlag[MAX_OMCI_ANI_PORT_NUM][MAX_OMCI_ANI_PORT_VALID_NUM] = {{0,OMCI_INVALID_MAC_BRIDGE_PORT_FLAG}};//9.3.4 instance id , valid or invalid ANI interface
mb_port_info_ptr gMBPortInfo_ptr = NULL;
extern pthread_mutex_t omciHandlePonIfc;


/*******************************************************************************************************************************
PON API  function 

********************************************************************************************************************************/

#ifndef TCSUPPORT_MT7530_SWITCH_API
/*******************************************************************************************************************************
switch API  function (switch)

********************************************************************************************************************************/
int macMT7530GetPortBrgInd(uint8 portId, uint8 *ind){
//	printf("\r\n macMT7530GetPortBrgInd API, portId=%d",portId);
	return 0;
}
int macMT7530SetPortBrgInd(uint8 portId, uint8 ind){
//	printf("\r\n macMT7530SetPortBrgInd API, portId=%d, ind=%d",portId, ind);
	return 0;
}

int macMT7530GetDiscardUnknownMacInd(uint8 portId){
//	printf("\r\n macMT7530GetDiscardUnknownMacInd API, portId=%d",portId);
	return 0;
}
int macMT7530SetDiscardUnknownMacInd(uint8 portId, uint8 ind){
//	printf("\r\n macMT7530SetDiscardUnknownMacInd API, portId=%d, ind=%d",portId, ind);
	return 0;
}

int macMT7530GetPortMac(uint8 portId, uint8 * portMac){
//	printf("\r\n macMT7530GetPortMac API, portId=%d",portId);
	
	return 0;
}

int macMT7530GetPortStatus(uint8 portId){
//	printf("\r\n macMT7530GetPortStatus API, portId=%d",portId);
	return OMCI_MAC_BRIDGE_PORT_STATE_6;//stp_off
}
#endif

#ifndef TCSUPPORT_PON_MAC_FILTER

/*******************************************************************************************************************************
MAC filter API  function (Reid)

********************************************************************************************************************************/
/*
Interface:
         0:reserved
         11~14:lan port1~4
         21~24:wlan port 1~4
         30:usb
         40:reserved
         51~58:nas11_0~nas11_7
*/

void displayMAcFilterRule(pon_mac_filter_rule * rule){
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n displayMAcFilterRule API->");
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n rule.filter_type=0x%02x",rule->filter_type);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n rule.mac_type=0x%02x",rule->mac_type);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n rule.ethertype=0x%02x",rule->ethertype);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n rule.startMac =0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x",
		rule->start_mac[0],rule->start_mac[1],rule->start_mac[2],rule->start_mac[3],rule->start_mac[4],rule->start_mac[5]);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n rule.end_mac =0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x",
		rule->end_mac[0],rule->end_mac[1],rule->end_mac[2],rule->end_mac[3],rule->end_mac[4],rule->end_mac[5]);
	
	return;
}
//int pon_add_mac_filter_rule(__u8 mac_interface, pon_mac_filter_rule rule);
int pon_add_mac_filter_rule(__u8 ifindex,__u8 type,pon_mac_filter_rule * rule){//type: 0->9.3.6  1->9.3.7	
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n pon_add_mac_filter_rule API-> interface=0x%02x", ifindex);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n pon_add_mac_filter_rule API-> type=0x%02x", type);
//	displayMAcFilterRule(rule);
	return 0;
}

//int pon_del_mac_filter_rule(__u8 mac_interface,  __u8 index)
int pon_del_mac_filter_rule(__u8 ifindex, __u8 type,__u8 index){

//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n pon_del_mac_filter_rule API-> interface=0x%02x", ifindex);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n pon_del_mac_filter_rule API-> type=0x%02x", type);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n pon_del_mac_filter_rule API-> index=0x%02x", index);
	return 0;
}

// int pon_clean_mac_filter_rule(__u8 mac_interface){
int pon_clean_mac_filter_rule(__u8 ifindex,__u8 type){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n pon_clean_mac_filter_rule API-> interface=0x%02x", ifindex);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n pon_clean_mac_filter_rule API-> type=0x%02x", type);
	return 0;
 }

// int pon_get_mac_filter_rule(__u8 mac_interface, pon_mac_filter_rule * rule){
int pon_get_mac_filter_rule(__u8 ifindex,__u8 type,pon_mac_filter_rule * rule){

//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n pon_get_mac_filter_rule API-> mac_interface=0x%02x, type=0x%02x", ifindex, type);
	return 0;
 }
#endif
#if 0
/*******************************************************************************************************************************

9.2.12 traffic descriptor  API  (Kenny)

********************************************************************************************************************************/
/*******************************************************************************************
**function name
	setBoundTD
**description:
	set bound traffic descriptor to limit the trafic
 **retrun :
 	-1:	failure
 	0: 	success
**parameter:
  	portId: lan port id or wan port id
  	ifcType: 0:lan interface, 1: wan interface
  	tdInstId: 9.2.12 traffic descriptor instance id
  	dir: 1:inbount, 2:outbound
********************************************************************************************/
int setBoundTD(IN uint8 portId, IN uint8 ifcType, IN uint16 tdInstId, IN uint8 dir){
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\n setBoundTD API");
	return 0;
}

/*******************************************************************************************
**function name
	cleanBoundTD
**description:
	clean bound configuration from  traffic descriptor
 **retrun :
 	-1:	failure
 	0: 	success
**parameter:
  	portId: lan port id or wan port id
  	ifcType: 0:lan interface, 1: wan interface
  	tdInstId: 9.2.12 traffic descriptor instance id
  	dir: 1:inbount, 2:outbound
********************************************************************************************/
int cleanBoundTD(IN uint8 portId, IN uint8 ifcType, IN uint16 tdInstId, IN uint8 dir){
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\n cleanOutBoundTD API");
	return 0;
}
#endif

#define PON_DEV "/dev/pon"

static int XMCS_IOCTL_SDI(int cmd, void *arg)
{
    int fd = -1;
  
    fd = open(PON_DEV, O_RDWR);
    if (fd < 0)
    {
        tcdbg_printf("Couldn't open %s, fd = %d.\n", PON_DEV, fd);
        return -1;
    }

    if (ioctl(fd, cmd, arg) < 0) 
    {
        close(fd);
        tcdbg_printf("ioctl error!\n");
    }

    close(fd);

    return 0;
}


/*******************************************************************************************************************************
general function

********************************************************************************************************************************/
/*******************************************************************************************
**function name
	handlePonIfc
**description:
	add or delete ANI interface, assign or unassign gemport to ANI interface
 **retrun :
	0:	success
	-1: failure
**parameter:
	ifcHandleType: action type
	index: ANI interface number(0~31)
	gemPortList: gemport list in this ANI interface
	validGemPortNum: valid gemport num
	instId: 9.3.4 instance id	
********************************************************************************************/

int handlePonIfc(uint8 ifcHandleType, uint8 index, uint16 * gemPortList, uint8 validGemPortNum, uint16 instId){
	int ret = -1;
	int i = 0;
	mb_port_info_ptr info_ptr = NULL;
	struct XMCS_GemPortAssign_S gponGemAssign ;
	pthread_mutex_lock(&omciHandlePonIfc);
	
	if((index>=MAX_OMCI_ANI_PORT_NUM) || (validGemPortNum > MAX_MAC_BRIDGE_PORT_GEM_PORT_NUM)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n handlePonIfc parameter error!");
		goto end;
	}

	switch(ifcHandleType){
			
		case GPON_HANDLE_PON_IFC_MODIFY:
		case GPON_HANDLE_PON_IFC_UNASSIGN:
			gponGemAssign.entryNum = 0 ;
			for(i=0 ; i < validGemPortNum ; i++) {
				if(gemPortList[i]< 4096 && index < 32) {
					gponGemAssign.gemPort[gponGemAssign.entryNum].id = gemPortList[i] ;
					gponGemAssign.entryNum++ ;
				}
			}
			if((ret = XMCS_IOCTL_SDI(IF_IOS_GEMPORT_UNASSIGN, &gponGemAssign)) != 0) {
				tcdbg_printf("in %s:%d ioctl IF_IOS_GEMPORT_UNASSIGN failed!\n", __FUNCTION__, __LINE__);
			}
		
			if(GPON_HANDLE_PON_IFC_UNASSIGN == ifcHandleType){
				break;
			} // else mean modify, fall through to create
		
		case GPON_HANDLE_PON_IFC_CREATE:
    		gponGemAssign.entryNum = 0 ;
    		for(i=0 ; i < validGemPortNum ; i++) {
    			if(gemPortList[i]< 4096 && index < MAX_OMCI_ANI_PORT_NUM) {
    				gponGemAssign.gemPort[gponGemAssign.entryNum].id = gemPortList[i] ;
    				gponGemAssign.gemPort[gponGemAssign.entryNum].ani = index ;
    				gponGemAssign.entryNum++ ;
    			}
    		}
    		if((ret = XMCS_IOCTL_SDI(IF_IOS_GEMPORT_ASSIGN, &gponGemAssign)) != 0) {
    			tcdbg_printf("in %s:%d ioctl IF_IOS_GEMPORT_ASSIGN failed!\n", __FUNCTION__, __LINE__);
			}else{
                tcapi_set("XPON_Common","GponBindIfc", "Yes");
            }

			break;

		case GPON_HANDLE_PON_IFC_DEL:
			info_ptr = findMBPortInfo(instId);
			gponGemAssign.entryNum = 0 ;
			if((info_ptr != NULL) 
				&& (info_ptr->type == OMCI_MAC_BRIDGE_PORT_WAN_IFC)){
				for(i=0; i<MAX_MAC_BRIDGE_PORT_GEM_PORT_NUM; i++){
					if(info_ptr->gemlist[i][0] == OMCI_VALID_MB_PORT_GEM_PORT){
						gponGemAssign.gemPort[gponGemAssign.entryNum].id = info_ptr->gemlist[i][1] ;
						gponGemAssign.entryNum++ ;		
					}
				}
			}
    		if((ret = XMCS_IOCTL_SDI(IF_IOS_GEMPORT_UNASSIGN, &gponGemAssign)) != 0) {
    			tcdbg_printf("in %s:%d ioctl IF_IOS_GEMPORT_UNASSIGN failed!\n", __FUNCTION__, __LINE__);
   		    }
			break;
		default:
    		tcdbg_printf("in %s:%d unexpected error!\n", __FUNCTION__, __LINE__);
            return -1;
	}	

	/*when clear all bind rule, the gpon bind flag need been set to No*/
	if(ifcHandleType == GPON_HANDLE_PON_IFC_UNASSIGN || ifcHandleType == GPON_HANDLE_PON_IFC_DEL){
		for(i = 0; i<MAX_OMCI_ANI_PORT_NUM; i++){
			if((validAniPortFlag[i][1] == OMCI_VALID_MAC_BRIDGE_PORT_FLAG)
				&& (i != index)){
				break;
			}
		}
		if(i == MAX_OMCI_ANI_PORT_NUM){
			tcapi_set("XPON_Common","GponBindIfc", "No");
			tcapi_set("XPON_Common","trafficStatus", "down");
		}
	}

	ret = 0;
	
end:
	pthread_mutex_unlock(&omciHandlePonIfc);
	return ret;

}

void getOmciEtherAddr(unsigned char *mac_addr)
{
	char macaddr[32] = {0};
	int i;
	char *macptr = NULL;
	char *macmove = NULL;
	
	char cmd[32]={0};
	char tmpBuf[512]={0};
	char *pValue=NULL;
	FILE * fp=NULL;
	char macInfo[2][18];
	char temp[]="/tmp/test.conf";

	memset(macInfo,0, sizeof(macInfo));	
	sprintf(cmd,"ifconfig eth0 > %s", temp);
	system(cmd);	
	fp=fopen(temp, "r");
	if(fp==NULL){
		return;
	}
	else{
		while (fgets (tmpBuf, sizeof(tmpBuf), fp) != NULL){
			/*krammer add*/
			if((pValue=strstr(tmpBuf,"HWaddr"))){
			       /*tmpBuf format: HWaddr 00:AA:BB:01:23:45*/
				sscanf(pValue,"%s %s", macInfo[0],macInfo[1]);
                        }
		}
	}
	fclose(fp);
	if(strlen(macInfo[1])>0){
		strcpy(macaddr, macInfo[1]);
	}	
	unlink(temp);
	
	macptr = macmove = macaddr;
	
	for(i = 0; i < 6; i++)
	{
		if((macmove = strchr(macptr,':')) == NULL)
			break;
		*macmove = '\0';
		mac_addr[i] = (unsigned char)strtoul(macptr,NULL,16);
		macptr = macmove + 1;
	}
	mac_addr[i] = (unsigned char)strtoul(macptr,NULL,16);
	return;
}


/*******************************************************************************************
**function name
	char2Hex
**description:
	convert character arr to HEX
 **retrun :
 	-1:	failure
 	other: hex value
**parameter:
  	arr: source character
********************************************************************************************/
int char2Hex(char arr){
	uint8 a;

	if(arr>='0' && arr <= '9')
		a = arr-'0';
	else if(arr>='a' && arr <= 'f')
		a = arr-'a'+10;
	else if(arr>='A' && arr <= 'F')
		a = arr-'A'+10;
	else 
		return -1;
	
	return a;
}
/*******************************************************************************************
**function name
	getMacBridgePortCnt
**description:
	It records the number of ports linked to this bridge
 **retrun :
 	0:	success
 	-1:	failure
**parameter:
  	instId: instance id in mac bridge service profile ME.
  	portCnt: port numbers.
********************************************************************************************/
int getMacBridgePortCnt(uint16 instanceId, uint8 * portCnt){
	uint8 bridgePortCnt = 0;
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciInst_p instPtr = NULL;	
	omciMeInst_ptr meInstPtr = NULL;
	uint16 bridgeInstId = 0;
	uint8 * val = NULL;

	tmpOmciManageEntity_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
	if(tmpOmciManageEntity_p == NULL){
		bridgePortCnt = 0;
		goto end;
	}
	if((instPtr = &(tmpOmciManageEntity_p->omciInst)) == NULL){
		bridgePortCnt = 0;
		goto end;
	}	
	if((meInstPtr = instPtr->omciMeInstList) == NULL){
		bridgePortCnt = 0;
		goto end;
	}
	
	while(meInstPtr != NULL){
		val = meInstPtr->attributeVlaue_ptr;
		if(val != NULL){
			val = val+2;/*jump to bridge id pointer attriubte*/
			bridgeInstId = get16(val);
			if(bridgeInstId == instanceId){
				bridgePortCnt++;
			}
		}		
		meInstPtr = meInstPtr->next;
	}

	*portCnt = bridgePortCnt;
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMacBridgePortCnt->*portCnt = 0x%0x",*portCnt);
end:
	return 0;
}
/*******************************************************************************************
**function name
	getMacBridgePortInstType
**description:
	get interface type  from mac bridge port configuration data
 **retrun :
 	0:	lan interface type
 	1:	wan interface type
 	-1:	failure
**parameter:
  	instId: instance id in mac bridge port configuration data ME.
  	ifcType: 1: WAN multicast tp, 2: wan gem tp, 3: wan dot1p.
********************************************************************************************/
int getMacBridgePortInstType(uint16 instId, uint8 * ifcType){
	omciMeInst_t *meInst_p = NULL;
	omciManageEntity_t *me_p = NULL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 type = 0;
	int instType = -1;

	if(ifcType == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMacBridgePortInstType->ifcType == NULL, fail");
		goto end;
	}
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMacBridgePortInstType->instId = %d",instId);
	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMacBridgePortInstType->omciGetMeByClassId fail");
		goto end;
	}
	
	meInst_p = omciGetInstanceByMeId(me_p , instId);	
	if(meInst_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMacBridgePortInstType->omciGetInstanceByMeId fail");
		goto end;
	}
	
	attributeValuePtr = (uint8 *)omciGetInstAttriByName(meInst_p , me_p->omciAttriDescriptList[3].attriName, &length);
	if(attributeValuePtr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMacBridgePortInstType->omciGetInstAttriByName fail");
		goto end;
	}
	type = *attributeValuePtr;
	switch(type){
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_1:
			*ifcType = OMCI_MAC_BRIDGE_PORT_PPTP_UNI;
			instType = OMCI_MAC_BRIDGE_PORT_LAN_IFC;
			break;
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_4:
#ifdef TCSUPPORT_PON_IP_HOST
			*ifcType = OMCI_MAC_BRIDGE_PORT_IP_CONFIG_HOST;
			instType = OMCI_MAC_BRIDGE_PORT_LAN_IFC;
			break;
#endif
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_11:
			instType = OMCI_MAC_BRIDGE_PORT_LAN_IFC;
			break;
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_3:
			*ifcType = OMCI_MAC_BRIDGE_PORT_DOT1P;
			instType = OMCI_MAC_BRIDGE_PORT_WAN_IFC;
			break;
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_5:
			*ifcType = OMCI_MAC_BRIDGE_PORT_GEM_TP;
			instType = OMCI_MAC_BRIDGE_PORT_WAN_IFC;
			break;
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_6:
			*ifcType = OMCI_MAC_BRIDGE_PORT_MULTICAST_TP;
			instType = OMCI_MAC_BRIDGE_PORT_WAN_IFC;
			break;
		case 2:
		case 7:
		case 8:
		case 9:
		case 10:
		case 12:
		default:
			break;
	}
end:
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMacBridgePortInstType->instType=0x%0x",instType);
	return instType;
}

/*******************************************************************************************
**function name
	getMacBridgeLanPortId
**description:
	get lan port id from 9.3.4 mac bridge port configuration data
 **retrun :
 	0:	success
 	-1:	failure
**parameter:
  	instId: instance id in 9.3.4 mac bridge port configuration data ME.
  	lanPortId: lan port id (0~3)
********************************************************************************************/
int getMacBridgeLanPortId(uint16 instId, uint8 *lanPortId){
	omciMeInst_t *meInst_p = NULL;
	omciManageEntity_t *me_p = NULL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	int ret = -1;
	uint8 portId = 0;
	uint8 tpType = 0;

	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMacBridgeLanPortId->omciGetMeByClassId fail");
		ret  = -1;
		goto end;
	}
	meInst_p = omciGetInstanceByMeId(me_p , instId);
	if(meInst_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMacBridgeLanPortId->omciGetInstanceByMeId fail");
		ret  = -1;
		goto end;
	}
	
	/*find tp type = Physical path termination point Ethernet UNI*/
	attributeValuePtr = (uint8 *)omciGetInstAttriByName(meInst_p , me_p->omciAttriDescriptList[3].attriName, &length);
	if(attributeValuePtr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMacBridgeLanPortId->omciGetInstAttriByName[3] fail");
		ret  = -1;
		goto end;
	}
	tpType = *attributeValuePtr;

	/*get lan port id from TP pointer attribute*/
	attributeValuePtr = (uint8 *)omciGetInstAttriByName(meInst_p , me_p->omciAttriDescriptList[4].attriName, &length);
	if(attributeValuePtr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMacBridgeLanPortId->omciGetInstAttriByName fail");
		ret  = -1;
		goto end;
	}
	switch(tpType){
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_1://pptp UNI
			portId = *(attributeValuePtr+1) - 1;	
			if(portId >= OMCI_LAN_PORT_NUM){
				ret = -1;
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMacBridgeLanPortId->portId>=4  fail, portId=%d", portId);
				goto end;
			}			
			*lanPortId = portId;
			ret = 0;
			break;
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_4://ip host config data
#ifdef TCSUPPORT_PON_IP_HOST
			portId = *(attributeValuePtr+1);			
			*lanPortId = portId + IP_HOST_CONFIG_PORT_BASIC;
			ret = 0;
			break;
#endif
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_11://virtual ethernet interface point
			*lanPortId = 0;
	ret = 0;
			break;
		default:
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMacBridgeLanPortId->TP type isn't LAN port");
			ret = -1;
			break;
	}
	
end:
	return ret;
	
}

/*******************************************************************************************
**function name
	getMacBridgeWanPortId
**description:
	get wan port id from mac bridge port configuration data
 **retrun :
 	0:	success
 	-1:	failure
**parameter:
  	instId: instance id in mac bridge port configuration data ME.
  	wanPortId: wan port id (0~31)
********************************************************************************************/
int getMacBridgeWanPortId(uint16 instId, uint8 *wanPortId){
	omciMeInst_t *meInst_p = NULL;
	omciManageEntity_t *me_p = NULL;
//	uint8 *attributeValuePtr = NULL;
//	uint16 length = 0;
	int ret = -1;
	uint8 portId = 0;

	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMacBridgeLanPortId->omciGetMeByClassId fail");
		ret  = -1;
		goto end;
	}
	meInst_p = omciGetInstanceByMeId(me_p , instId);
	if(meInst_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMacBridgeLanPortId->omciGetInstanceByMeId fail");
		ret  = -1;
		goto end;
	}
	
	portId = (uint8)meInst_p->deviceId;
	*wanPortId = portId;
	
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMacBridgeWanPortId->Wan port id=0x%0x",*wanPortId);	
	
	ret = 0;
	
end:
	return ret;
	
}

/*******************************************************************************************
**function name
	getMacBridgePortIfcByInst
**description:
	get mac bridge port interface name via  the instance id of MAC bridge port configuration data ME
 **retrun :
 	0:	success
 	-1:	failure
**parameter:
	instanceId: me instance id
  	ifcName: return interface name, eg:eth0.1
********************************************************************************************/
int getMacBridgePortIfcByInst(IN uint16 instanceId, OUT char * ifcName){
	int ifaceType = 0; //0:UNI, 1:ANI	
	uint8 portId = 0;//UNI/ANI port id
	int ret = -1;
	uint8 ifcType = 0;

	if(ifcName == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMacBridgePortIfcByInst->parameter is error, failure");
		return -1;			
	}	

	/*get interface type*/
	ifaceType = getMacBridgePortInstType(instanceId,&ifcType);
	/*only support getting bridge table information in UNI or ANI port*/
	if(ifaceType == OMCI_MAC_BRIDGE_PORT_LAN_IFC){//UNI port
		/*get lan  portid*/
		ret = getMacBridgeLanPortId(instanceId, &portId);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->getMacBridgeLanPortId failure");
			ret = -1;
			goto end;
		}
		sprintf(ifcName, "%s%d", OMCI_LAN_INTERFACE_PREFIX_STR, (portId+OMCI_MAC_BRIDGE_PORT_LAN_PORT_BASIC)); //eth0.1~eth0.4
	}else if(ifaceType == OMCI_MAC_BRIDGE_PORT_WAN_IFC){//ANI port
		/*get Wan  portid*/
		ret = getMacBridgeWanPortId(instanceId, &portId);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->getMacBridgeWanPortId failure");
			ret = -1;
			goto end;
		}
		sprintf(ifcName, "%s%d", OMCI_PON_INTERFACE_PREFIX_STR, (portId+OMCI_MAC_BRIDGE_PORT_PON_PORT_BASIC));// nas13~
	}else{//no UNI/ANI port 
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->ifaceType =%d failure",ifaceType);
			ret = -1;
			goto end;
	}

	ret = 0;
	
end:
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->ifaceType=%d",ifaceType);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->portId=%d",portId);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->ifcName=%s",ifcName);	
	return ret;
}

/*******************************************************************************************
**function name
	getPortFromMacBridgeByInstId
**description:
	get mac bridge port interface type and port id via  the instance id of MAC bridge port configuration data ME
 **retrun :
 	0:	success
 	-1:	failure
**parameter:
	portId: return port id (lan port id:0~3, WAN port id: 0~xx)
	ifaceType: return interface type(0:LAN interface, 1:Wan interface)
	instanceId: MAC bridge port configuration data ME instance id
********************************************************************************************/
int getPortFromMacBridgeByInstId(OUT uint8 *portId, OUT uint8 * ifaceType, IN uint16 instId){
	int ret = -1;
	uint8 port = 0;//UNI/ANI port id	
	int type = 0; // 0:UNI interface, 1:ANI interface
	uint8 ifcType = 0;

	if(portId == NULL ||ifaceType == NULL ){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getPortFromMacBridgeByInstId->parameter is error, failure");
		goto end;		
	}	

	/*get interface type*/
	type = getMacBridgePortInstType(instId, &ifcType);
	/*only support getting bridge table information in UNI or ANI port*/
	if(type == OMCI_MAC_BRIDGE_PORT_LAN_IFC){//UNI port
		/*get lan  portid*/
		ret = getMacBridgeLanPortId(instId, &port);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getPortFromMacBridgeByInstId->getMacBridgeLanPortId failure");
			ret = -1;
			goto end;
		}
	}else if(type == OMCI_MAC_BRIDGE_PORT_WAN_IFC){//ANI port
		/*get Wan  portid*/
		ret = getMacBridgeWanPortId(instId, &port);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getPortFromMacBridgeByInstId->getMacBridgeWanPortId failure");
			ret = -1;
			goto end;
		}
	}else{//no UNI/ANI port 
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getPortFromMacBridgeByInstId->type =%d failure",type);
		ret = -1;
		goto end;
	}

	*portId = port;
	*ifaceType = type;
	ret = 0;
	
end:
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getPortFromMacBridgeByInstId->type=%d",type);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getPortFromMacBridgeByInstId->port=%d",port);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getPortFromMacBridgeByInstId->ret=0x%02x",ret);
	
	return ret;
}

int getPortFromMacBridgeSkipUNIFilterRuleByInstId(OUT uint8 * skip_filter_flag, IN uint16 instId)
{  
        omciMeInst_t *meInst_p = NULL;
        omciManageEntity_t *me_p = NULL;
        uint8 *attributeValuePtr = NULL;
        uint16 length = 0;
        uint8 type = 0;
        int instType = -1;
	    int ret = -1;
        
        
        if(NULL == skip_filter_flag)
        {
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getPortFromMacBridgeSkipUNIFilterRuleByInstId->is_veip_port == NULL, fail");
            goto end;
        }

        //2--hgu modes
        /*SFU mode don't need filter pptp ether uni port vlan filter rule*/
        if(2 != omci.omciCap.systemCap.onuType)
        {
            *skip_filter_flag = 0;
            ret = 0;
            goto end;
        } 

        me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
        if(NULL == me_p)
        {
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getPortFromMacBridgeSkipUNIFilterRuleByInstId->omciGetMeByClassId fail");
            goto end;
        }
        
        meInst_p = omciGetInstanceByMeId(me_p , instId);    
        if(NULL == meInst_p)
        {
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getPortFromMacBridgeSkipUNIFilterRuleByInstId->omciGetInstanceByMeId fail");
            goto end;
        }
        
        attributeValuePtr = (uint8 *)omciGetInstAttriByName(meInst_p , me_p->omciAttriDescriptList[3].attriName, &length);
        if(NULL == attributeValuePtr)
        {
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMacBridgePortInstType->omciGetInstAttriByName fail");
            goto end;
        }
        type = *attributeValuePtr;
        switch(type)
        {
            case OMCI_MAC_BRIDGE_PORT_TP_TYPE_1:
            {
                *skip_filter_flag = 1;
                break;
            }
            default :
            {
                *skip_filter_flag = 0;
                break;
            }
        }
        
        ret = 0;
end:
    return ret;
}

/*******************************************************************************************
**function name
	setMacBridgePortInfoToRomfile
**description:
	set mac bridge port information to romfile(interface type(0/1), port list)
 **retrun :
 	-1:	fail
 	0:	success
**parameter:
	instanceId:	9.3.4 instance id
********************************************************************************************/
int setMacBridgePortInfoToRomfile(IN uint16 instanceId){
	int ret = -1;
	int i = 0;
	uint16 gemPortList[MAX_MAC_BRIDGE_PORT_GEM_PORT_NUM] = {0};
	uint8 validGemPortNum = 0;
	uint8 ifaceType = 0; //0:UNI, 1:ANI	
	uint8 portId = 0;//UNI/ANI port id
	char attrInfo[MAX_BUFFER_SIZE] = {0};
	char attrName[32] = {0};
	char tempStr[8] = {0};

	/*get attribute in PMGR_MAC_BRIDGE_PORT_ENTRY  node*/
	sprintf(attrName, "%s%x", PMGR_ATTR_MACBRIDGEPORT_PREFIX, instanceId);

	/*get interface type and port id*/
	if(getPortFromMacBridgeByInstId(&portId, &ifaceType, instanceId) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortInfoToRomfile->getPortFromMacBridgeByInstId, instanceId=%d, fail",instanceId);
		goto end;;
	}

	/*don't support interface type*/
	if((ifaceType != OMCI_MAC_BRIDGE_PORT_LAN_IFC)
		&& (ifaceType != OMCI_MAC_BRIDGE_PORT_WAN_IFC)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortInfoToRomfile->getPortFromMacBridgeByInstId, instanceId=%d, ifaceType=%d, fail",instanceId,ifaceType);
		goto end;
	}

	/*get attribute information*/
	if(ifaceType == OMCI_MAC_BRIDGE_PORT_LAN_IFC){//UNI port
		sprintf(attrInfo,"%x %x", ifaceType,portId );
	}else if(ifaceType == OMCI_MAC_BRIDGE_PORT_WAN_IFC){//ANI port
		if(getGemPortByMACBridgePortInst(instanceId, gemPortList, &validGemPortNum) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortInfoToRomfile->getGemPortByMACBridgePortInst,, instanceId=%d, fail",instanceId);
			goto end;
		}			
//		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMacBridgePortInfoToRomfile->pmmgrTcapiGet, instanceId=%d, validGemPortNum=%d",instanceId,validGemPortNum);

		sprintf(attrInfo,"%x", ifaceType);
		for(i = 0; i<validGemPortNum; i++){
			sprintf(tempStr," %x", gemPortList[i]);
			strcat(attrInfo, tempStr);			
		}		
	}

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMacBridgePortInfoToRomfile->attrName=%s, attrInfo=%s \n",attrName,attrInfo);
	tcapi_set(PMGR_MAC_BRIDGE_PORT_ENTRY, attrName, attrInfo);
	ret = 0;
	
end:
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMacBridgePortInfoToRomfile->ret=0x%02x",ret);
	return ret;

}

/*******************************************************************************************
**function name
	updateGemPortInfoToRomfile
**description:
	set ALL GEM port information to romfile()
 **retrun :
 	-1:	fail
 	0:	success
**parameter:
	NULL
********************************************************************************************/
int updateGemPortInfoToRomfile(void){
#ifdef TCSUPPORT_OMCI_ALCATEL
	uint8 attrName[32] = {0};
	uint8 tempStr[8] = {0};
	uint8 attrInfo[256] = {0};
	uint16 length = 0;
	uint8 *attributeValuePtr = NULL, *attributeValuePtr2 = NULL;
	omciManageEntity_t *tmpOmciManageEntity_p = NULL, *tmpOmciManageEntity2_p = NULL;
	omciInst_t *inst_p = NULL, *inst2_p = NULL;
	omciMeInst_t *meInst_p = NULL, *meInst2_p = NULL;
	uint16 attriMask = 0;
	uint16 portID = 0;
	uint16 gemPortCTPPtr = 0, gemPortCTPId = 0;

	sprintf(attrName, "%s", PMGR_ATTR_TOTALGEMPORT);
	sprintf(attrInfo, "\0");

	tmpOmciManageEntity_p = omciGetMeByClassId(OMCI_CLASS_ID_GEM_INTERWORK_TP);
	if(tmpOmciManageEntity_p == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "updateGemPortInfoToRomfile:---tmpOmciManageEntity_p == NULL\n");
		return -1;
	}
	
	inst_p = &(tmpOmciManageEntity_p->omciInst);
	meInst_p = inst_p->omciMeInstList;
	
	while(meInst_p != NULL)
	{
		attriMask = 1<<15;
		attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);
		gemPortCTPPtr = get16(attributeValuePtr);
		
		tmpOmciManageEntity2_p = omciGetMeByClassId(OMCI_CLASS_ID_GEM_PORT_CTP);
		if(tmpOmciManageEntity2_p == NULL)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "updateGemPortInfoToRomfile:---tmpOmciManageEntity_p == NULL\n");
			return -1;
		}
		
		inst2_p = &(tmpOmciManageEntity2_p->omciInst);
		meInst2_p = inst2_p->omciMeInstList;
		
		while(meInst2_p != NULL)
		{
			gemPortCTPId= get16(meInst2_p->attributeVlaue_ptr);

			if(gemPortCTPId == gemPortCTPPtr) {
				attriMask = 1<<15;
				attributeValuePtr = omciGetInstAttriByMask(meInst2_p, attriMask, &length);
				portID = get16(attributeValuePtr);
				
				sprintf(tempStr,"%x ", portID);
				strcat(attrInfo, tempStr);
				break;
			}
			meInst2_p = meInst2_p->next;
		}
		
		meInst_p = meInst_p->next;
	}

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateGemPortInfoToRomfile->attrName=%s, attrInfo=%s \n",attrName,attrInfo);
	tcapi_set(GPON_OMCI_ENTRY, attrName, attrInfo);
	if(tcapi_commit(GPON_OMCI_ENTRY) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "updateGemPortInfoToRomfile: tcapi_commit GPON_OMCI_ENTRY fail !\n");
		return -1;
	}
#endif
	return 0;
}

#if 0
/*******************************************************************************************
**function name
	getPortInfoByTraficDesInst
**description:
	get port information about inbound traffic descriptor
 **retrun :
 	0:	success
 	-1:	failure
**parameter:
	portId: return port id (lan port id:0~3, WAN port id: 0~xx)
	ifaceType: return interface type(0:LAN interface, 1:Wan interface)
	instanceId: traffic descriptor ME instance id
	type: 
		0:inbound
		1:outbound
		2:inbound and outbound
********************************************************************************************/
int getPortInfoByTraficDesInst(OUT uint8 *portId, OUT uint8 * ifaceType, OUT uint8 type, IN uint16 instId){
	int ret = -1;
	omciInst_t *omciInst_p;
	omciMeInst_t *meInst_p = NULL;
	omciManageEntity_t *me_p = NULL;
	uint8 inBoundFindFlag = OMCI_NO_FIND_RULE;
	uint8 outBoundFindFlag = OMCI_NO_FIND_RULE;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 tempInstId = 0;
	uint16 currInstId = 0;


	if(portId == NULL ||ifaceType == NULL ){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getPortInfoByTraficDesInst->parameter is error, failure");
		goto end;		
	}

	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getPortInfoByTraficDesInst->omciGetMeByClassId fail");
		goto end;
	}

	omciInst_p = &(me_p->omciInst);	
	/* find the instance and exec it */
	if(omciInst_p->omciMeInstList == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getPortInfoByTraficDesInst->omciMeInstList is NULL!");
		goto end;
	}

	meInst_p = omciInst_p->omciMeInstList;
	if(meInst_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getPortInfoByTraficDesInst->meInst_p is NULL!");
		goto end;
	}

	while(meInst_p != NULL){
		/*get Inbound TD pointer attribute value*/
		attributeValuePtr = (uint8 *)omciGetInstAttriByName(meInst_p , me_p->omciAttriDescriptList[12].attriName, &length);
		if(attributeValuePtr != NULL){
			tempInstId = get16(attributeValuePtr);		
			if(tempInstId == instId){
				findFlag = OMCI_FIND_RULE;
				currInstId = get16(meInst_p->attributeVlaue_ptr);
				break;
			}
		}
		meInst_p = meInst_p->next;
	}

	if(findFlag != OMCI_FIND_RULE){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getPortInfoByTraficDesInst->not find, fail!");
		goto end;
	}
	if(getPortFromMacBridgeByInstId(portId, ifaceType, currInstId) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getPortInfoByTraficDesInst->getPortFromMacBridgeByInstId, fail!");
		goto end;
	}
	
	ret = 0;
	
end:

	return ret;
}

#endif
/*******************************************************************************************
**function name
	getGemPortByMACBridgePortInst
**description:
	according mac bridge port config data instance id, get gem port list
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
	instanceId:	9.3.4 instance id
	gemPort :	gemp port list
  	validGemPortNum :	valid gem port num.
********************************************************************************************/
int getGemPortByMACBridgePortInst(IN uint16 instanceId, OUT uint16 * gemPort, OUT uint8 * validGemPortNum){
	omciMeInst_t *meInst_p = NULL;
	omciManageEntity_t *me_p = NULL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 tpType = 0;
	uint16 tpPointer = 0;	
	int ret = -1;
	int i = 0, j = 0; 
	uint8 attrIndex = 0;
	uint8 currGemPortNum = 0;
	uint8 findFlag = OMCI_NO_FIND_RULE;
	uint16 tempGemPortId = 0;

	if(gemPort == NULL ||validGemPortNum == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortByMACBridgePortInst->parameters is error,  fail");
		goto end;
	}
	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortByMACBridgePortInst->omciGetMeByClassId 1,fail");
		ret = -1;
		goto end;
	}
	
	meInst_p = omciGetInstanceByMeId(me_p , instanceId);
	if(meInst_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortByMACBridgePortInst->omciGetInstanceByMeId 1,fail");
		ret = -1;
		goto end;
	}
			 
	/*get TP type attribute value*/
	attributeValuePtr = (uint8 *)omciGetInstAttriByName(meInst_p , me_p->omciAttriDescriptList[3].attriName, &length);
	if(attributeValuePtr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortByMACBridgePortInst->omciGetInstAttriByName fail");
		ret = -1;
		goto end;
	}
	tpType = *attributeValuePtr;
	
	/*get TP pointer attribute value*/
	attributeValuePtr = (uint8 *)omciGetInstAttriByName(meInst_p , me_p->omciAttriDescriptList[4].attriName, &length);
	if(attributeValuePtr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortByMACBridgePortInst->omciGetInstAttriByName fail");
		ret  = -1;
		goto end;
	}
	tpPointer = get16(attributeValuePtr);

	switch(tpType){		
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_3:// get gem port id  from 802.1p ME
			me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_VLAN_802_1P);
			if(me_p == NULL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortByMACBridgePortInst->802.1p->omciGetMeByClassId 2,fail");
				ret = -1;
				goto end;
			}			
			meInst_p = omciGetInstanceByMeId(me_p , tpPointer);
			if(meInst_p == NULL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortByMACBridgePortInst->802.1p->omciGetInstanceByMeId 2,fail");
				ret = -1;
				goto end;
			}

			for(i = 0; i<MAX_OMCI_PBIT_NUM; i++){
				attrIndex = i+2;
				/*get interwork TP pointer for P-bit priority i*/
				attributeValuePtr = (uint8 *)omciGetInstAttriByName(meInst_p , me_p->omciAttriDescriptList[attrIndex].attriName, &length);
				if(attributeValuePtr == NULL){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortByMACBridgePortInst->802.1p->omciGetInstAttriByName(9.3.10), i=%d, fail", i);
					ret  = -1;
					goto end;
				}
				tpPointer = get16(attributeValuePtr);
				if(tpPointer == OMCI_DOT1P_INTERWORK_TP_DISCARD){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getGemPortByMACBridgePortInst->802.1p->tpPointer == 0xff, i=%d", i);
					continue;
				}
				/*get a gem port id */
				ret = getGemPortIdFromGemInterTP(OMCI_CLASS_ID_GEM_INTERWORK_TP, tpPointer, &tempGemPortId);
				if(ret != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortByMACBridgePortInst->802.1p->getGemPortIdFromGemInterTP,tpPointer=0x%02x ,fail",tpPointer);
					ret = -1;
					goto end;
				}
				/*find gem port in gemPort array*/
				findFlag = OMCI_NO_FIND_RULE;
				for(j = 0; j<currGemPortNum; j++){
					if(*(gemPort+j) == tempGemPortId){
						findFlag = OMCI_FIND_RULE;
						break;
					}	
				}
				/*get new gem port id*/
				if(findFlag == OMCI_FIND_RULE){
					continue;
				}
				*(gemPort+currGemPortNum) = tempGemPortId;
				currGemPortNum++;
				if(currGemPortNum > MAX_MAC_BRIDGE_PORT_GEM_PORT_NUM){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortByMACBridgePortInst->gem port num overflow, currGemPortNum = %d, fail",currGemPortNum);
					ret = -1;
					goto end;
				}
			}
			
			break;
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_5://get gem port id form GEM interworking termination point ME
			/*get a gem port id */
			ret = getGemPortIdFromGemInterTP(OMCI_CLASS_ID_GEM_INTERWORK_TP, tpPointer, &tempGemPortId);
			if(ret != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortByMACBridgePortInst->GEM TP->getGemPortIdFromGemInterTP,tpPointer=0x%02x ,fail",tpPointer);
				ret = -1;
				goto end;
			}
			*gemPort = tempGemPortId;
			currGemPortNum = 1;
			break;
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_6://get gem port id form multicast gem interworking termination point ME
			/*get a gem port id */
			ret = getGemPortIdFromGemInterTP(OMCI_CLASS_ID_MULTICAST_GEM_INTERWORK_TP, tpPointer, &tempGemPortId);
			if(ret != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortByMACBridgePortInst->Multicast TP->getGemPortIdFromGemInterTP,tpPointer=0x%02x ,fail",tpPointer);
				ret = -1;
				goto end;
			}
			*gemPort = tempGemPortId;
			currGemPortNum = 1;
			break;
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_1:
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_4:
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_11:
		default:
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getGemPortByMACBridgePortInst->tpType=0x%02x",tpType);
			ret = -1;
			goto end;
	}

	if(currGemPortNum == 0){// not find
		ret = -1;
		goto end;
	}
	*validGemPortNum = currGemPortNum;
	ret = 0;
end:
	return ret;
}
/*******************************************************************************************
**function name
	findMacBridgeFirstStr
**description:
	find the first string in row buffer,
	find the remain buffer in row buffer.
 **retrun :
 	0:	success
 	-1:	failure
**parameter:
	rowBuf: row buffer
	fromLen: from rowBuf+fromLen
	firstStr: first words string
********************************************************************************************/

int findMacBridgeFirstStr(char * rowBuf, uint32 * fromLen, char * firstStr){
	char *rowPtr = NULL;	
	char buf[MAX_MAC_BRIDGE_RECORD_BUF] = {0};
	int len = 0;
	uint32 currLen = 0;

	if((rowBuf == NULL) || (firstStr == NULL)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n findMacBridgeFirstStr  fail ");
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	currLen = *fromLen;

//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n findMacBridgeFirstStr->fromLen =%d",*fromLen);
	
	rowPtr = rowBuf+currLen;
	/*strip the prefix space*/
	while(*rowPtr != '\0'){
		if((*rowPtr != ' ') && (*rowPtr != '\t'))
			break;
		currLen++;
		rowPtr++;
	}
	/*get first Str*/
	len = 0;
	while(*rowPtr != '\0'){		
		if((*rowPtr == ' ') || (*rowPtr == '\t')){	
			break;
		}
		if(len >=  MAX_MAC_BRIDGE_RECORD_BUF){
			goto failure;
		}
		buf[len++] = *rowPtr;
		currLen++;
		rowPtr++;
	}
	buf[MAX_MAC_BRIDGE_RECORD_BUF-1] = '\0';
	strcpy(firstStr, buf);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n findMacBridgeFirstStr->firstStr =%s",firstStr);
	* fromLen = currLen;
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n findMacBridgeFirstStr->currLen =%d",currLen);
	
	return 0;
failure:
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n findMacBridgeFirstStr->len =%d",len);

	return -1;

}



/*******************************************************************************************
**function name
	getMacFormMacStr
**description:
	get 6 bytes mac addres form mac string
 **retrun :
 	0:	success
 	-1:	failure
**parameter:
	delimiter: strip char
	macStr: mac string 
	mac: 6 bytes mac address
********************************************************************************************/
int getMacFormMacStr(char delimiter, char * macStr, uint8 * mac){
	char * tempPtr = NULL;
	uint8 tempMac[OMCI_MAC_LENGTH] = {0};
	uint8 temp = 0;
	int i = 0;
	int k = 0;
	
	if((macStr == NULL) || (mac == NULL)){
		return -1;
	}
	if(strlen(macStr) != 17){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMacFormMacStr->macStr len = %d error",strlen(macStr));
		return -1;
	}
	memset(tempMac, 0, sizeof(tempMac));
	
	tempPtr = macStr;
	
	while(*tempPtr != '\0'){
		if(*tempPtr != delimiter){
			temp = char2Hex(*tempPtr);
			if(temp == -1){
				return -1;
			}
			if((i%2) == 0){
				tempMac[k] = (temp<<4);
			}else{
				tempMac[k] |= temp;
				k++;
			}
			i++;
		}
		tempPtr++;
	}
	if(k == 0){
		return -1;
	}
	memcpy(mac, tempMac, 6);
	return 0;	
}


/*******************************************************************************************
**function name
	getSearchFilterMacFormFilterTbl
**description:
	find mac address in mac filter table(only filter type)
 **retrun :
 	0:	success
 	-1:	failure
**parameter:
	instanceId: instance id
	mac: find mac address
	find: return find result
********************************************************************************************/
int getSearchFilterMacFormFilterTbl(IN uint16 instanceId,IN  uint8 *mac, OUT uint8 *find, OUT uint8 * filterFlag){
	omciMeInst_t *meInst_p = NULL;
	omciManageEntity_t *me_p = NULL;
	omciTableAttriValue_t *tableConent_ptr = NULL; //table content pointer
	uint8 *attributeValuePtr = NULL;	
	int i = 0;
	uint8 attributeNum =0;
	uint8 macStart[6] = {0};
//	uint8 macEnd[6] = {0};
	uint16 length = 0;
	uint8 *tmp_ptr = NULL;
	/*  	filterType: 
		Bit 	Name			Setting
		1	filter/forward		0:forward,1:filter
		2					0:dest mac, 1:src mac
		3~6	reserved
		7~8	add/remove		00:remove this entry, 01:addthis entry, 10:clear entire table
  	*/	
	uint8 filterType = 0;
	uint8 findFlag = OMCI_NO_FIND_RULE;
	int ret = 0;

	if(mac == NULL || find == NULL ||filterFlag == NULL ){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getSearchFilterMacFormFilterTbl->parameter NULL, fail");
		return -1;
	}

	/*find mac filter in  9.3.6 mac filter table*/
	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_TABLE_DATA);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getSearchFilterMacFormFilterTbl->omciGetMeByClassId fail");
		ret = -1;
		goto end;
	}
	meInst_p = omciGetInstanceByMeId(me_p , instanceId);
	if(meInst_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getSearchFilterMacFormFilterTbl->omciGetInstanceByMeId fail");
		ret = -1;
		goto end;
	}

	tableConent_ptr = omciGetTableValueByIndex(meInst_p, omciAttriDescriptListMBPortFilterData[1].attriIndex);
	if(tableConent_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getSearchFilterMacFormFilterTbl->tmp_ptr is NULL");
		ret = -1;
		goto end;		
	}
	/*search record in 9.3.6 mac filter table*/
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getSearchFilterMacFormFilterTbl->search 9.3.6 start");
	
	tmp_ptr = tableConent_ptr->tableValue_p;
	length = MAC_FILTER_RECORD_LENGTH;
	while(tmp_ptr != NULL){
		tmp_ptr += 1;//jump index, get filter type
		filterType  = *tmp_ptr;
		tmp_ptr += 1;//jump filter type, get mac
		if(((filterType & MAC_FILTER_TYPE_80_VALID_BITS) == MAC_FILTER_TYPE_80)
			&& (memcmp(mac,tmp_ptr,OMCI_MAC_LENGTH) == 0)){
			findFlag = OMCI_FIND_RULE;//find
			*filterFlag = (filterType & 0x01);

			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getSearchFilterMacFormFilterTbl->findFlag = %d, filterType =%d", findFlag, filterType);
			
			ret = 0;
			goto end;
		}
		length += MAC_FILTER_RECORD_LENGTH;
		if(length >= tableConent_ptr->tableSize){
			break;
		}
		tmp_ptr += OMCI_MAC_LENGTH;
	}
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getSearchFilterMacFormFilterTbl->search 9.3.6 end");	
	/*find mac filter in  9.3.7 mac prefilter table*/
	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_PREASSIGN_DATA);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getSearchFilterMacFormFilterTbl->omciGetMeByClassId fail");
		ret = -1;
		goto end;
	}
	meInst_p = omciGetInstanceByMeId(me_p , instanceId);
	if(meInst_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getSearchFilterMacFormFilterTbl->omciGetInstanceByMeId fail");
		ret = -1;
		goto end;
	}

	attributeNum = omciGetAttriNumOfMe(me_p);
	i = 1;
	length = 0;
	while(i < attributeNum){
		attributeValuePtr = omciGetInstAttriByName(meInst_p, me_p->omciAttriDescriptList[i].attriName, &length);
		if(attributeValuePtr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getSearchFilterMacFormFilterTbl->omciGetInstAttriByName fail");
			ret = -1;
			goto end;
		}
		*filterFlag = *attributeValuePtr;
		switch(i){
			case 1://01.00.5e.00.00.00.00--01.00.5e.7f.ff.ff
				macStart[0] = 0x01;
				macStart[1] = 0x00;
				macStart[2] = 0x5e;
				macStart[3] = 0x00;
				macStart[4] = 0x00;
				macStart[5] = 0x00;
				if((memcmp(macStart, mac, 3) == 0)
					&&(mac[3]<=0x7f)){
					findFlag = OMCI_FIND_RULE;//find
					ret = 0;
					goto end;
				}
				break;
			case 2://33.33.00.00.00.00.00--33.33.ff.ff.ff.ff
				if((mac[0] == 0x33)
					&&(mac[1] == 0x33)){
					findFlag = OMCI_FIND_RULE;//find
					ret = 0;
					goto end;
				}						
				break;
			case 5:
				/*09:00:1b:ff:ff:ff*/
				macStart[0] = 0x09;
				macStart[1] = 0x0;
				macStart[2] = 0x1b;
				macStart[3] = 0xff;
				macStart[4] = 0xff;
				macStart[5] = 0xff;					
				if(memcmp(macStart, mac, 6) == 0){
					findFlag = OMCI_FIND_RULE;//find
					ret = 0;
					goto end;
				}

				/*09:00:4e:00:00:02*/
				macStart[0] = 0x09;
				macStart[1] = 0x0;
				macStart[2] = 0x4e;
				macStart[3] = 0x00;
				macStart[4] = 0x00;
				macStart[5] = 0x02;
				if(memcmp(macStart, mac, 6) == 0){
					findFlag = OMCI_FIND_RULE;//find
					ret = 0;
					goto end;
				}
				break;
			case 6://0x03, 0x00, 0x00, 0x00, 0x00, 0x01
				macStart[0] = 0x03;
				macStart[1] = 0x0;
				macStart[2] = 0x0;
				macStart[3] = 0x0;
				macStart[4] = 0x0;
				macStart[5] = 0x01;
				if(memcmp(macStart, mac, 6) == 0){
					findFlag = OMCI_FIND_RULE;//find
					ret = 0;
					goto end;
				}
				break;
			case 7://09:00:07:00:00:00~ 09:00:07:00:00:fc, 09:00:07:ff:ff:ff
				macStart[0] = 0x09;
				macStart[1] = 0x0;
				macStart[2] = 0x07;
				macStart[3] = 0x0;
				macStart[4] = 0x0;
				macStart[5] = 0x0;
				if((memcmp(macStart, mac, 5) == 0)
					&&(mac[5] <= 0xfc)){
					findFlag = OMCI_FIND_RULE;//find
					ret = 0;
					goto end;
				}
				macStart[0] = 0x09;
				macStart[1] = 0x0;
				macStart[2] = 0x07;
				macStart[3] = 0xff;
				macStart[4] = 0xff;
				macStart[5] = 0xff;
				if(memcmp(macStart, mac, 6) == 0){
					findFlag = OMCI_FIND_RULE;//find
					ret = 0;
					goto end;
				}
				break;
			case 8://01:80:c2:00:00:00~ 01:80:c2:00:00:ff
				macStart[0] = 0x01;
				macStart[1] = 0x80;
				macStart[2] = 0xc2;
				macStart[3] = 0x0;
				macStart[4] = 0x0;
				macStart[5] = 0x0;
				if(memcmp(macStart, mac, 5) == 0){
					findFlag = OMCI_FIND_RULE;//find
					ret = 0;
					goto end;
				}
				break;
			case 3:
			case 4:					
			case 9:
			case 10:
				//nothing					
				break;					
			default:
				ret = -1;
				goto end;
		}

		i++;
	}
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getSearchFilterMacFormFilterTbl->search 9.3.7 end");	

	ret = 0;
end:
	*find = findFlag;
	return ret;
}


/*******************************************************************************************
**function name
	updateMACBridgePortInfoByAssociaAttr
**description:
	1.update multicast rule and set 9.3.4 tp type or tp pointer attribute.
	2.update PM instance in PM mechanism
	3.update device id of the mac bridge ANI port
	4.update gem port mapping
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	value :	set value
  	meInstantPtr: 		instance pointer
  	omciAttribute:		attribute pointer
  	flag:				create  or set
  	attrType:		 	tp type attribute or tp pointer attribute
********************************************************************************************/
int updateMACBridgePortInfoByAssociaAttr(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag, uint8 attrType){
	int ret = -1;
	int result = 0;
	uint8 findMulticastInstFlag = 0;//0:no find, 1 find
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	uint16 instanceId = 0;
	uint8 type = 0;
	uint8 oldPortType = 0;
	uint8 newPortType = 0;

	uint8 *attrValue_ptr = NULL;
	uint16 mbInstId = 0; 
	uint8 oldIfcType = 0;
	uint8 newIfcType = 0;
	uint8 portId = 0;
	uint8 macAddrLimitFromMBSP = 0;
	uint8 macAddrLimitFromMBPCD = 0;

	if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr-> parameter is eror, fail");
		goto end;
	}

	switch(attrType){
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_ATTR:
//			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateMACBridgePortInfoByAssociaAttr-> TP Type start");
			break;
		case OMCI_MAC_BRIDGE_PORT_TP_POINTE_ATTR:
//			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateMACBridgePortInfoByAssociaAttr-> TP Pointer start");
			break;
		default:
			goto end;
			break;
	}
	if((flag != OMCI_CREATE_ACTION_SET_VAL) && (flag != OMCI_SET_ACTION_SET_VAL)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr-> flag=%d, fail", flag);
		goto end;
	}
	
	/*calloc memory*/
	attrValue_ptr =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_ATTR_LEN);
	if(attrValue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr-> val calloc error ");
		ret = -1;
		goto end;
	}
	
	/*get instance id*/
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);

	/*get mac bridge service profile instance id*/
	ret = getAttributeValueByInstId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA, instanceId, 1, attrValue_ptr, 2);
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateMACBridgePortInfoByAssociaAttr-> getAttributeValueByInstId  fail ");
		ret = -1;
		goto end;
	}
	mbInstId = get16(attrValue_ptr);

#if 0		
	/*clean old  port information*/
	if(flag == OMCI_SET_ACTION_SET_VAL){
		/*1. clean multicast rule */
		/*1.1 find 9.3.28 multicast subscriber config info instance by 9.3.4 mac bridge port config data ME instance*/
		if(findMulticastInstByMacBrPortInst(instanceId) == 0){
			findMulticastInstFlag = OMCI_FIND_MULTICAST_BY_MAC_BRIDGE_PORT;
			type = (OMCI_MULTICAST_DYNAMIC_ACL_TYPE 
					| OMCI_MULTICAST_STATIC_ACL_TYPE 
					| OMCI_MULTICAST_MAX_RATE_TYPE
					| OMCI_MULTICAST_VER_TYPE
					| OMCI_MULTICAST_FUNC_TYPE
					| OMCI_MULTICAST_FAST_LEAVE_TYPE
					| OMCI_MULTICAST_UP_TCI_TYPE
					| OMCI_MULTICAST_TAG_CTRL_TYPE) ;
		}

		/*1.2 clean multicast rule by mac bridge port config data ME*/
		if(findMulticastInstFlag == OMCI_FIND_MULTICAST_BY_MAC_BRIDGE_PORT){
			if(clearRealMulticastRuleByMeType(instanceId, OMCI_MULTICAST_CONFIG_INFO_ME_TYPE_0, type) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr-> clearRealMulticastRuleByMeType fail");
			}
		}

		/*2. clean PM instance in PM mechanism*/
		/*2.1 clean 9.3.9 PM instance in PM mechanism*/
		if (pmmgrTcapiSet(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_PMHD, instanceId, PMMGR_DELETE, 0, NULL) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateMACBridgePortInfoByAssociaAttr-> pmmgrTcapiSet,9.3.9 PMMGR_DELETE,  fail");
		}
		/*2.2 clean 9.3.30 PM instance in PM mechanism*/
		if (pmmgrTcapiSet(OMCI_ME_CLASS_ID_ETHERNET_FRAME_UP_PMHD, instanceId, PMMGR_DELETE, 0, NULL) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateMACBridgePortInfoByAssociaAttr-> pmmgrTcapiSet,9.3.30 PMMGR_DELETE,  fail");
		}
		/*2.3 clean 9.3.31 PM instance in PM mechanism*/
		if (pmmgrTcapiSet(OMCI_ME_CLASS_ID_ETHERNET_FRAME_DOWN_PMHD, instanceId, PMMGR_DELETE, 0, NULL) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateMACBridgePortInfoByAssociaAttr-> pmmgrTcapiSet,9.3.31 PMMGR_DELETE,  fail");
		}
		/*3.update device id of the mac bridge ANI port*/
		/*3.1get mac bridge port interface type*/
		result = getMacBridgePortInstType(instanceId, &oldIfcType);
		if(result == OMCI_MAC_BRIDGE_PORT_WAN_IFC){
			oldPortType = OMCI_MAC_BRIDGE_PORT_WAN_IFC;
		}else if(result == OMCI_MAC_BRIDGE_PORT_LAN_IFC){
			oldPortType = OMCI_MAC_BRIDGE_PORT_LAN_IFC;
			
			/*set user isolate function to default value in this lan port*/
			if(oldIfcType == OMCI_MAC_BRIDGE_PORT_PPTP_UNI){
				if(getMacBridgeLanPortId(instanceId, &portId) == 0){					
					if(pon_set_user_group(portId, OMCI_MBP_UNI_ISOLATE_DEFAULT_VAL) != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciMACBridgePortConDataDeleteAction-> pon_set_user_group, fail");
					}
				}else{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr-> getMacBridgeLanPortId,old lan port id, fail");
				}
			}
		}else{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr-> getMacBridgePortInstType,oldPortType, fail");
			goto end;
		}


	}
#endif
	
	/*update value for TP type  or TP pointer attribute*/
	if(setGeneralValue(value, meInstantPtr , omciAttribute, flag) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr-> setGeneralValue fail");
		goto end;
	}

	/*get new mac bridge port interface type*/
	result = getMacBridgePortInstType(instanceId,&newIfcType);
	if(result == OMCI_MAC_BRIDGE_PORT_WAN_IFC){
		newPortType = OMCI_MAC_BRIDGE_PORT_WAN_IFC;
	}else if(result == OMCI_MAC_BRIDGE_PORT_LAN_IFC){
		newPortType = OMCI_MAC_BRIDGE_PORT_LAN_IFC;
	}else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr-> getMacBridgePortInstType,newPortType, fail");
		goto end;
	}
	
	/*update some information when create mac bridge port instance*/
	if((flag == OMCI_CREATE_ACTION_SET_VAL) && (attrType == OMCI_MAC_BRIDGE_PORT_TP_POINTE_ATTR)){
		/*3.update device id of the mac bridge ANI port*/
		if(newPortType == OMCI_MAC_BRIDGE_PORT_WAN_IFC){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateMACBridgePortInfoByAssociaAttr->1 newPortType=%d",newPortType);
			if(setMacBridgePortDeviceId(instanceId, OMCI_ADD_MAC_BRIDGE_ANI_PORT) == OMCI_MAC_BRIDGE_PORT_FAIL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr-> setMacBridgePortDeviceId, add1, fail");
			}
		//added by lidong	
			if (newIfcType == OMCI_MAC_BRIDGE_PORT_MULTICAST_TP)
			{
				//nothing
			}else
			/*4.update gem port mapping*/
			/*4.1 add gem port mapping rule by ani port*/
			if(addGemPortMappingRuleByAniPort(instanceId) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr->create-> addGemPortMappingRuleByAniPort, fail");
			}
		}else{//newPortType = OMCI_MAC_BRIDGE_PORT_LAN_IFC
			/*add gem port rule according to the 9.3.4 point to the PPTP UNI*/
			if(addGemPortRuleByMbPortUNI(instanceId) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr->create-> addGemPortRuleByMbPortUNI, fail");
			}
		}
		
	}

	/*update mac bridge port information for PM in romfile*/
	if((flag == OMCI_CREATE_ACTION_SET_VAL) && (attrType == OMCI_MAC_BRIDGE_PORT_TP_TYPE_ATTR)){
		//nothing
	}else{	
		if(setMacBridgePortInfoToRomfile(instanceId) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateMACBridgePortInfoByAssociaAttr-> setMacBridgePortInfoToRomfile, fail");
		}

		/*set user isolate function to related group  in this lan port*/
		if((newPortType == OMCI_MAC_BRIDGE_PORT_LAN_IFC) \
            && (newIfcType == OMCI_MAC_BRIDGE_PORT_PPTP_UNI) \
            && (pSystemCap->onuType==SFU)){
			if(getMacBridgeLanPortId(instanceId, &portId) == 0){
#ifdef TCSUPPORT_PON_USER_ISOLATION				
				if(userIsolateApi(OMCI_USER_ISOLATION_SET_GROUP, portId, mbInstId) != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciMACBridgePortConDataDeleteAction-> userIsolateApi,new lan port id, fail");
				}
#endif
			}else{
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr-> getMacBridgeLanPortId,new lan port id, fail");
			}
		}
	}


#if 0
	/*update new port information*/
	if(flag == OMCI_SET_ACTION_SET_VAL){//update port information
		/*1. update multicast information*/
		/*1.1 set multicast rule by mac bridge port cofig data ME*/
		if(findMulticastInstFlag == OMCI_FIND_MULTICAST_BY_MAC_BRIDGE_PORT){
			if(setRealMulticastRuleByMeType(instanceId, OMCI_MULTICAST_CONFIG_INFO_ME_TYPE_0, type) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr-> setRealMulticastRuleByMeType fail");
			}
		}
		/*2. update PM instance in PM mechanism*/
		/*2.1 update 9.3.9 PM instance in PM mechanism*/
		if (pmmgrTcapiSet(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_PMHD, instanceId, PMMGR_CREATE, 0, NULL) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateMACBridgePortInfoByAssociaAttr-> pmmgrTcapiSet,9.3.9 PMMGR_CREATE,  fail");
		}
		/*2.2 update 9.3.30 PM instance in PM mechanism*/
		if (pmmgrTcapiSet(OMCI_ME_CLASS_ID_ETHERNET_FRAME_UP_PMHD, instanceId, PMMGR_CREATE, 0, NULL) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateMACBridgePortInfoByAssociaAttr-> pmmgrTcapiSet,9.3.30 PMMGR_CREATE,  fail");
		}
		/*2.3 update 9.3.31 PM instance in PM mechanism*/
		if (pmmgrTcapiSet(OMCI_ME_CLASS_ID_ETHERNET_FRAME_DOWN_PMHD, instanceId, PMMGR_CREATE, 0, NULL) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateMACBridgePortInfoByAssociaAttr-> pmmgrTcapiSet,9.3.31 PMMGR_CREATE,  fail");
		}		

//		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateMACBridgePortInfoByAssociaAttr->2 oldPortType=%d, newPortType=%d",oldPortType,newPortType);
		if((oldPortType == OMCI_MAC_BRIDGE_PORT_LAN_IFC)
			&& (newPortType == OMCI_MAC_BRIDGE_PORT_WAN_IFC)){//LAN->WAN
			/*3.update device id of the mac bridge ANI port*/
			/*3.1.add  device id of the mac bridge ANI port*/
			if(setMacBridgePortDeviceId(instanceId, OMCI_ADD_MAC_BRIDGE_ANI_PORT) == OMCI_MAC_BRIDGE_PORT_FAIL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr->LAN->WAN-> setMacBridgePortDeviceId, add2, fail");
			}

			/*4.update gem port mapping*/
			/*4.1 add gem port mapping rule by ani port*/
			if(addGemPortMappingRuleByAniPort(instanceId) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n updateMACBridgePortInfoByAssociaAttr->LAN->WAN-> addGemPortMappingRuleByAniPort, fail");
			}
		}else if((oldPortType == OMCI_MAC_BRIDGE_PORT_WAN_IFC)
			&& (newPortType == OMCI_MAC_BRIDGE_PORT_LAN_IFC)){//WAN->LAN
			/*4.2 delete gem port mapping rule by mac bridge port*/
			if(delGemPortMappingRuleByMbp(instanceId) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr->WAN->LAN-> delGemPortMappingRuleByMbp, fail");
			}
			
			/*3.2.delete  device id of the mac bridge ANI port*/
			if(setMacBridgePortDeviceId(instanceId, OMCI_DEL_MAC_BRIDGE_ANI_PORT) == OMCI_MAC_BRIDGE_PORT_FAIL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr->WAN->LAN-> setMacBridgePortDeviceId, del, fail");
			}
		}else if((oldPortType == OMCI_MAC_BRIDGE_PORT_WAN_IFC)
			&& (newPortType == OMCI_MAC_BRIDGE_PORT_WAN_IFC)){//WAN->WAN
			
			if(attrType == OMCI_MAC_BRIDGE_PORT_TP_POINTE_ATTR){
				/*4.3.1 modify gem port mapping rule by mac bridge port */
				if(delGemPortMappingRuleByMbp(instanceId) != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr->WAN->LAN-> delGemPortMappingRuleByMbp, fail");
				}
				/*3.3 modify  device id of the mac bridge ANI port*/
				if(setMacBridgePortDeviceId(instanceId, OMCI_MOD_MAC_BRIDGE_ANI_PORT) == OMCI_MAC_BRIDGE_PORT_FAIL){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr->LAN->WAN-> setMacBridgePortDeviceId, add2, fail");
				}
				/*4.3.2 modify gem port mapping rule by mac bridge port */
				if(addGemPortMappingRuleByAniPort(instanceId) != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateMACBridgePortInfoByAssociaAttr->LAN->WAN-> addGemPortMappingRuleByAniPort, fail");
				}
			}

		}else{
			//nothing
		}
	}
#endif
	
	ret = 0;
	
end:
	if(attrValue_ptr != NULL){
		free(attrValue_ptr);
	}
	return ret;
}
/*******************************************************************************************
**function name
	addGemPortRuleByMbPortUNI
**description:
	add gem port rule, according to the 9.3.4 point to the PPTP UNI or IP host config data for CUC
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	instId :	9.3.4 UNI instance id
********************************************************************************************/
int addGemPortRuleByMbPortUNI(uint16 instId){
	int ret = -1;
	uint8 tpType = 0;
	uint16 mbpInstId = 0;//9.3.1 mac bridge service profile instance id
	omciManageEntity_ptr me_p = NULL;	
	omciInst_t *omciInst_p = NULL ;
	omciMeInst_t *inst_p = NULL;
	omciMeInst_t *meInst_p = NULL;	
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 tempMbpInstId = 0;
	uint8 tempTpType = 0;
	uint16 tempTpPointer = 0;
	uint16 tempInstId = 0;

	uint8 userPort = 0;
	uint32 classIdInstId = 0;
	uint32 meIdArr[MAX_GEMPORT_VALID_MEID_NUM] = {0};
	uint8 validNum  = 0;

	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortRuleByMbPortUNI->omciGetMeByClassId fail");
		goto end;
	}

	/*1.get some information from current instance */	
	meInst_p = omciGetInstanceByMeId(me_p , instId);
	if(meInst_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortRuleByMbPortUNI->omciGetInstanceByMeId fail");
		goto end;
	}
	/*1.1 get tp type attribute value*/
	attributeValuePtr = (uint8 *)omciGetInstAttriByName(meInst_p , me_p->omciAttriDescriptList[3].attriName, &length);
	if(attributeValuePtr == NULL ){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortRuleByMbPortUNI-> getAttributeValueByInstId tp type fail,[%d]", __LINE__);
		goto end;
	}
	tpType = *attributeValuePtr;

	/*1.2 get tp pointer attribute value*/
	attributeValuePtr = (uint8 *)omciGetInstAttriByName(meInst_p , me_p->omciAttriDescriptList[4].attriName, &length);
	if(attributeValuePtr == NULL ){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortRuleByMbPortUNI-> getAttributeValueByInstId tp pointer fail,[%d]", __LINE__);
		goto end;
	}	
	/*1.3 get user port and classIdInstId from current instance*/
#ifdef TCSUPPORT_PON_IP_HOST
	if(tpType == OMCI_MAC_BRIDGE_PORT_TP_TYPE_4)
		userPort = *(attributeValuePtr+1); // ip host config data should == 0
	else
#endif
	userPort = *(attributeValuePtr+1) -1;  // pptp eth uni port 1 ~ 4

	classIdInstId = (OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA << 16);
	classIdInstId |= instId;
	
	/*1.4 get mac bridge id(9.3.1) pointer attribute value*/
	attributeValuePtr = (uint8 *)omciGetInstAttriByName(meInst_p , me_p->omciAttriDescriptList[1].attriName, &length);
	if(attributeValuePtr == NULL ){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortRuleByMbPortUNI-> getAttributeValueByInstId Bridge id pointer fail,[%d]", __LINE__);
		goto end;
	}	
	mbpInstId = get16(attributeValuePtr);

	/*2. only handle tp type =1*/
	if(tpType != OMCI_MAC_BRIDGE_PORT_TP_TYPE_1
#ifdef TCSUPPORT_PON_IP_HOST
		&& tpType != OMCI_MAC_BRIDGE_PORT_TP_TYPE_4
#endif
	){
		ret = 0;
		goto end;
	}
#ifdef TCSUPPORT_PON_IP_HOST
	else if(tpType == OMCI_MAC_BRIDGE_PORT_TP_TYPE_4)
		userPort += IP_HOST_CONFIG_PORT_BASIC;		
#endif

	
	/*3.traversal all 9.3.4 instance*/
	omciInst_p = &(me_p->omciInst);
	if(omciInst_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortRuleByMbPortUNI-> omciInst_p == NULL fail,[%d]", __LINE__);
		goto end;
	}
	inst_p = omciInst_p->omciMeInstList;
	while(inst_p != NULL){
		/*3.1 get 9.3.4 instance id*/
		tempInstId = get16(inst_p->attributeVlaue_ptr);
		
		/*3.2 get mac bridge id(9.3.1) pointer attribute value*/
		attributeValuePtr = (uint8 *)omciGetInstAttriByName(inst_p , me_p->omciAttriDescriptList[1].attriName, &length);//Bridge id pointer attribute
		if(attributeValuePtr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortRuleByMbPortUNI-> omciGetInstAttriByName, Bridge id pointerinstid=0x%02x,fail",tempInstId);
			goto end;
		}
		tempMbpInstId = get16(attributeValuePtr);
		
		/*3.3 jump different mac bridge with current mac bridge */
		if(tempMbpInstId != mbpInstId){
			inst_p = inst_p->next;
			continue;
		}
	
		/*3.4 get 9.3.4 tp type attribute value*/
		attributeValuePtr = (uint8 *)omciGetInstAttriByName(inst_p , me_p->omciAttriDescriptList[3].attriName, &length);//tp type attribute
		if(attributeValuePtr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortRuleByMbPortUNI-> omciGetInstAttriByName, tp typeinstid=0x%02x,fail",tempInstId);
			goto end;
		}
		tempTpType= *attributeValuePtr;

		/*3.5 get 9.3.4 tp pointer attribute value*/
		attributeValuePtr = (uint8 *)omciGetInstAttriByName(inst_p , me_p->omciAttriDescriptList[4].attriName, &length);//tp pointer attribute
		if(attributeValuePtr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortRuleByMbPortUNI-> omciGetInstAttriByName, tp pointer , instid=0x%02x,fail",tempInstId);
			goto end;
		}
		tempTpPointer= get16(attributeValuePtr);
		
		/*3.6 add gem port rule by user port */
		switch(tempTpType){
			case OMCI_MAC_BRIDGE_PORT_TP_TYPE_3:
			case OMCI_MAC_BRIDGE_PORT_TP_TYPE_5:
//			case OMCI_MAC_BRIDGE_PORT_TP_TYPE_6: //multicast gemport should not be included 
				meIdArr[0] = (OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA << 16);
				meIdArr[0] |=  tempInstId;
				meIdArr[1] = (OMCI_ME_CLASS_ID_VLAN_TAG_FILTER_DATA << 16);
				meIdArr[1] |=  tempInstId;
				validNum = 2;
				if(tempTpType == OMCI_MAC_BRIDGE_PORT_TP_TYPE_3){
					meIdArr[2] = (OMCI_ME_CLASS_ID_VLAN_802_1P << 16);
					meIdArr[2] |=  tempTpPointer;
					validNum = 3;
				}
				if(addOmciGemPortMappingRuleByUni(userPort, classIdInstId, meIdArr, validNum) != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addGemPortRuleByMbPortUNI-> addOmciGemPortMappingRuleByUni, instid=0x%02x,fail",tempInstId);
				}
				break;
			default:
				break;				
		}
		inst_p = inst_p->next;
	}
	ret = 0;
end:
	return ret;
}
	
/*******************************************************************************************
**function name
	updateGemMappingRuleByMbPortUNI
**description:
	update gemport rule, accroding the dot1p or vlan filter changed.
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	mapType :	1:according to the vlan filter, 2 according to  the dot 1p
  	instId: dot1p(9.3.10)  or vlan filter(9.3.11) ME instance id
********************************************************************************************/
int updateGemMappingRuleByMbPortUNI(uint8 mapType, uint16 instId){
	int ret = -1;
//	uint8 tpType = 0;
	uint16 mbpInstId = 0;//9.3.1 mac bridge service profile instance id
	omciManageEntity_ptr me_p = NULL;	
	omciInst_t *omciInst_p = NULL ;
	omciMeInst_t *inst_p = NULL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 tempMbpInstId = 0;
	uint8 tempTpType = 0;
	uint16 tempTpPointer = 0;
	uint16 tempInstId = 0;
	uint32 classIdInstId = 0;
	uint8 findMb = OMCI_NO_FIND_RULE;
	uint16 delMatchFlag = 0;
	omciGemPortMapping_ptr mapRule_ptr = NULL;

	/*1.init*/
	if((mapType != OMCI_UPDATE_UNI_MAPPING_BY_VLAN_FILTER_ME)
//		&&(mapType != OMCI_UPDATE_UNI_MAPPING_BY_USERPORT_ME)
		&&(mapType != OMCI_UPDATE_UNI_MAPPING_BY_DOT1P_ME)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMbPortUNI->mapType=%d, fail",mapType);
		goto end;
	}
	/*malloc and set value for new port mapping*/		
	mapRule_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
	if(mapRule_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMbPortUNI->calloc newRule_ptr  fail");
		goto end;
	}
	
	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMbPortUNI->omciGetMeByClassId fail");
		goto end;
	}

	omciInst_p = &(me_p->omciInst);
	if((omciInst_p == NULL) || (omciInst_p->omciMeInstList == NULL)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMbPortUNI-> 9.3.4 instance is NULL, fail,[%d]", __LINE__);
		goto end;
	}
	inst_p = omciInst_p->omciMeInstList;

	/*2.find current mac bridge service profile*/
	findMb = OMCI_NO_FIND_RULE;
	while(inst_p != NULL){		
		/*2.1 get 9.3.4 instance id*/
		tempInstId = get16(inst_p->attributeVlaue_ptr);
		
		/*2.2 get mac bridge id(9.3.1) pointer attribute value*/
		attributeValuePtr = (uint8 *)omciGetInstAttriByName(inst_p , me_p->omciAttriDescriptList[1].attriName, &length);//Bridge id pointer attribute
		if(attributeValuePtr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMbPortUNI-> omciGetInstAttriByName, Bridge id pointerinstid=0x%02x,fail,[%d]",tempInstId, __LINE__);
			goto end;
		}
		mbpInstId = get16(attributeValuePtr);

		/*2.3 get 9.3.4 tp type attribute value*/
		attributeValuePtr = (uint8 *)omciGetInstAttriByName(inst_p , me_p->omciAttriDescriptList[3].attriName, &length);//tp type attribute
		if(attributeValuePtr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMbPortUNI-> omciGetInstAttriByName, tp type,instid=0x%02x,fail,[%d]",tempInstId, __LINE__);
			goto end;
		}
		tempTpType= *attributeValuePtr;

		/*2.4 get 9.3.4 tp pointer attribute value*/
		attributeValuePtr = (uint8 *)omciGetInstAttriByName(inst_p , me_p->omciAttriDescriptList[4].attriName, &length);//tp pointer attribute
		if(attributeValuePtr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMbPortUNI-> omciGetInstAttriByName, tp pointer , instid=0x%02x,fail,[%d]",tempInstId,__LINE__);
			goto end;
		}
		tempTpPointer= get16(attributeValuePtr);
		switch(mapType){
			case OMCI_UPDATE_UNI_MAPPING_BY_VLAN_FILTER_ME:
				if((tempInstId == instId)//ANI side
					&& ((tempTpType == OMCI_MAC_BRIDGE_PORT_TP_TYPE_3)
						|| (tempTpType == OMCI_MAC_BRIDGE_PORT_TP_TYPE_5)
						|| (tempTpType == OMCI_MAC_BRIDGE_PORT_TP_TYPE_6))){
					findMb = OMCI_FIND_RULE;
				}
				break;
			case OMCI_UPDATE_UNI_MAPPING_BY_DOT1P_ME:
				if((tempTpType == OMCI_MAC_BRIDGE_PORT_TP_TYPE_3)
					&& (tempTpPointer == instId)){
					findMb = OMCI_FIND_RULE;
				}
				break;
			
//			case OMCI_UPDATE_UNI_MAPPING_BY_USERPORT_ME:
			default:
				break;
		}
		if(findMb == OMCI_FIND_RULE)
			break;
		inst_p = inst_p->next;
	}

	if(findMb == OMCI_NO_FIND_RULE){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMbPortUNI-> findMb == OMCI_NO_FIND_RULE,fail,[%d]",__LINE__);
		goto end;
	}
	
	/*3.delete old gemport rule and add new rule by user port*/
	inst_p = omciInst_p->omciMeInstList;
	delMatchFlag = OMCI_GEMPORT_MAPPING_CLASSID_INSTID;
	while(inst_p != NULL){		
		/*3.1 get 9.3.4 instance id*/
		tempInstId = get16(inst_p->attributeVlaue_ptr);
		
		/*3.2 get mac bridge id(9.3.1) pointer attribute value*/
		attributeValuePtr = (uint8 *)omciGetInstAttriByName(inst_p , me_p->omciAttriDescriptList[1].attriName, &length);//Bridge id pointer attribute
		if(attributeValuePtr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMbPortUNI-> omciGetInstAttriByName, Bridge id pointer2,instid=0x%02x,fail,[%d]",tempInstId, __LINE__);
			goto end;
		}
		tempMbpInstId = get16(attributeValuePtr);
		if(mbpInstId != tempMbpInstId){
			inst_p = inst_p->next;
			continue;
		}
		/*3.3 get 9.3.4 tp type attribute value and find UNI instance*/
		attributeValuePtr = (uint8 *)omciGetInstAttriByName(inst_p , me_p->omciAttriDescriptList[3].attriName, &length);//tp type attribute
		if(attributeValuePtr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMbPortUNI-> omciGetInstAttriByName, tp type2,instid=0x%02x,fail,[%d]",tempInstId, __LINE__);
			goto end;
		}
		tempTpType= *attributeValuePtr;
		if(tempTpType != OMCI_MAC_BRIDGE_PORT_TP_TYPE_1
#ifdef TCSUPPORT_PON_IP_HOST
			&& tempTpType != OMCI_MAC_BRIDGE_PORT_TP_TYPE_4		
#endif
		){
			inst_p = inst_p->next;
			continue;
		}
		/*3.4 get ME id(class id +instance id)*/
		classIdInstId = (OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA<<16);
		classIdInstId |= tempInstId;
		mapRule_ptr->classIdInstId = classIdInstId;
		/*3.5 delete old gemport rule by user port*/
		if(delOmciGemPortMappingRuleByMatchFlag(delMatchFlag, mapRule_ptr) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMbPortUNI->delOmciGemPortMappingRuleByMatchFlag, fail");
		}
		/*3.6 add new gemport rule by user port*/
		if(addGemPortRuleByMbPortUNI(tempInstId) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateGemMappingRuleByMbPortUNI->addGemPortRuleByMbPortUNI, fail");
		}
		inst_p = inst_p->next;
	}

	ret = 0;
end:
	if(mapRule_ptr != NULL){
		free(mapRule_ptr);
	}
	return ret;
}

/*******************************************************************************************
**function name
	getMacFilterRulePortIdByInst
**description:
	get mac filter rule port id
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	instId :	9.3.4 instance id
  	macFilterRulePortId: return vlan rule port id
********************************************************************************************/
int getMacFilterRulePortIdByInst(uint16 instId, uint8 * macFilterRulePortId){
	int ret = -1;
	int ifaceType = 0;
	uint8 portId = 0;
	uint8 portBasic = 0;
	uint8 ifcType = 0;

	if(macFilterRulePortId == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMacFilterRulePortIdByInst-> parameter is NULL, fail");
		ret = -1;
		goto end;
	}
	
	/*get interface type*/
	ifaceType = getMacBridgePortInstType(instId,&ifcType);
	
	/*get ANI/UNI port id*/
	if(ifaceType == OMCI_MAC_BRIDGE_PORT_LAN_IFC){//lan, UNI port
		ret = getMacBridgeLanPortId(instId, &portId);
		if(ret != 0){
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMacFilterRulePortIdByInst-> not find lan port, fail");
			goto end;
		}
		portBasic = OMCI_MAC_BRIDGE_FILTER_LAN_PORT_BASIC;
	}else if(ifaceType == OMCI_MAC_BRIDGE_PORT_WAN_IFC){//wan , ANI port
		ret = getMacBridgeWanPortId(instId, &portId);
		if(ret != 0){
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMacFilterRulePortIdByInst-> not find wan port, fail");
			goto end;
		}
		portBasic = OMCI_MAC_BRIDGE_FILTER_WAN_PORT_BASIC;
	}else{
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMacFilterRulePortIdByInst-> getMacBridgePortInstType, fail");
		goto end;
	}
	*macFilterRulePortId = portBasic +portId;
	
	ret = 0;
end:
	return ret;
}

/*******************************************************************************************
**function name
	setMacFilterRuleByType
**description:
	set mac filter rule to specified port
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
	type:	9.3.6 or 9.3.7 mac filter type
	macFilterEntry_ptr:	when type is 9.3.6(MAC_FILTER_TABLE_ENTRY), this value length is 8 bytes.
					when type is 9.3.7, this value is 0(forward) or 1(filter)
  	macFilterRulePortId :	mac filter rule port Id( interface)
********************************************************************************************/
int setMacFilterRuleByType(IN uint8 type,  IN uint8 * macFilterEntry_ptr,  IN  uint8 macFilterRulePortId){
	int ret = -1;
	pon_mac_filter_rule macFilterRule;
	pon_mac_filter_rule_p macFilterRule_ptr = NULL;
	pon_mac_filter_rule macFilterRule2;
	pon_mac_filter_rule_p macFilterRule2_ptr = NULL;
	pon_mac_filter_rule macFilterRule3;
	pon_mac_filter_rule_p macFilterRule3_ptr = NULL;	
	pon_mac_filter_rule macFilterRule4;
	pon_mac_filter_rule_p macFilterRule4_ptr = NULL;
	uint8 filterType = 0;
	uint8 * temp_ptr = NULL;
	uint8 macFilterType = OMCI_MAC_FILTER_TYPE;

	if(macFilterEntry_ptr == NULL){
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMacFilterRuleByType-> macFilterEntry_ptr is NULL, fail");
		goto end;
	}	
	macFilterRule_ptr = &macFilterRule;
	memset(macFilterRule_ptr, 0, sizeof(pon_mac_filter_rule));
	
	switch(type){
		case MAC_FILTER_TABLE_ENTRY://mac filter table
			/*handle filter type*/
			temp_ptr = macFilterEntry_ptr+1;			
			filterType = *temp_ptr;
			macFilterRule_ptr->filter_type = (filterType&0x01);
			if((filterType&0x02) == 0x02){
				macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_SRC;
			}else{
				macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			}
			macFilterRule_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			
			/*handle mac*/
			temp_ptr = macFilterEntry_ptr+2;
			memcpy(macFilterRule_ptr->start_mac, temp_ptr, 6);
			memcpy(macFilterRule_ptr->end_mac, temp_ptr, 6);
			break;
		case MAC_FILTER_IPV4_MULTICAST:	
			macFilterRule_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule_ptr->start_mac[0] = 0x01;
			macFilterRule_ptr->start_mac[1] = 0x00;
			macFilterRule_ptr->start_mac[2] = 0x5e;
			macFilterRule_ptr->start_mac[3] = 0x00;
			macFilterRule_ptr->start_mac[4] = 0x00;
			macFilterRule_ptr->start_mac[5] = 0x00;

			macFilterRule_ptr->end_mac[0] = 0x01;
			macFilterRule_ptr->end_mac[1] = 0x00;
			macFilterRule_ptr->end_mac[2] = 0x5e;
			macFilterRule_ptr->end_mac[3] = 0x7f;
			macFilterRule_ptr->end_mac[4] = 0xff;
			macFilterRule_ptr->end_mac[5] = 0xff;
			break;
		case MAC_FILTER_IPV6_MULTICAST:
			macFilterRule_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule_ptr->start_mac[0] = 0x33;
			macFilterRule_ptr->start_mac[1] = 0x33;
			macFilterRule_ptr->start_mac[2] = 0x00;
			macFilterRule_ptr->start_mac[3] = 0x00;
			macFilterRule_ptr->start_mac[4] = 0x00;
			macFilterRule_ptr->start_mac[5] = 0x00;

			macFilterRule_ptr->end_mac[0] = 0x33;
			macFilterRule_ptr->end_mac[1] = 0x33;
			macFilterRule_ptr->end_mac[2] = 0xff;
			macFilterRule_ptr->end_mac[3] = 0xff;
			macFilterRule_ptr->end_mac[4] = 0xff;
			macFilterRule_ptr->end_mac[5] = 0xff;
			break;
		case MAC_FILTER_IPV4_BROADCAST:
			macFilterRule_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0800;
			macFilterRule_ptr->start_mac[0] = 0xff;
			macFilterRule_ptr->start_mac[1] = 0xff;
			macFilterRule_ptr->start_mac[2] = 0xff;
			macFilterRule_ptr->start_mac[3] = 0xff;
			macFilterRule_ptr->start_mac[4] = 0xff;
			macFilterRule_ptr->start_mac[5] = 0xff;

			macFilterRule_ptr->end_mac[0] = 0xff;
			macFilterRule_ptr->end_mac[1] = 0xff;
			macFilterRule_ptr->end_mac[2] = 0xff;
			macFilterRule_ptr->end_mac[3] = 0xff;
			macFilterRule_ptr->end_mac[4] = 0xff;
			macFilterRule_ptr->end_mac[5] = 0xff;
			break;
		case MAC_FILTER_RARP:
			macFilterRule_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_8035;
			macFilterRule_ptr->start_mac[0] = 0xff;
			macFilterRule_ptr->start_mac[1] = 0xff;
			macFilterRule_ptr->start_mac[2] = 0xff;
			macFilterRule_ptr->start_mac[3] = 0xff;
			macFilterRule_ptr->start_mac[4] = 0xff;
			macFilterRule_ptr->start_mac[5] = 0xff;

			macFilterRule_ptr->end_mac[0] = 0xff;
			macFilterRule_ptr->end_mac[1] = 0xff;
			macFilterRule_ptr->end_mac[2] = 0xff;
			macFilterRule_ptr->end_mac[3] = 0xff;
			macFilterRule_ptr->end_mac[4] = 0xff;
			macFilterRule_ptr->end_mac[5] = 0xff;
			break;
		case MAC_FILTER_IPX:
			/*first*/
			macFilterRule_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_8137;
			macFilterRule_ptr->start_mac[0] = 0xff;
			macFilterRule_ptr->start_mac[1] = 0xff;
			macFilterRule_ptr->start_mac[2] = 0xff;
			macFilterRule_ptr->start_mac[3] = 0xff;
			macFilterRule_ptr->start_mac[4] = 0xff;
			macFilterRule_ptr->start_mac[5] = 0xff;

			macFilterRule_ptr->end_mac[0] = 0xff;
			macFilterRule_ptr->end_mac[1] = 0xff;
			macFilterRule_ptr->end_mac[2] = 0xff;
			macFilterRule_ptr->end_mac[3] = 0xff;
			macFilterRule_ptr->end_mac[4] = 0xff;
			macFilterRule_ptr->end_mac[5] = 0xff;
			/*second*/
			macFilterRule2_ptr = &macFilterRule2;
			memset(macFilterRule2_ptr, 0, sizeof(macFilterRule2));
			macFilterRule2_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule2_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule2_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule2_ptr->start_mac[0] = 0x09;
			macFilterRule2_ptr->start_mac[1] = 0x00;
			macFilterRule2_ptr->start_mac[2] = 0x1b;
			macFilterRule2_ptr->start_mac[3] = 0xff;
			macFilterRule2_ptr->start_mac[4] = 0xff;
			macFilterRule2_ptr->start_mac[5] = 0xff;

			macFilterRule2_ptr->end_mac[0] = 0x09;
			macFilterRule2_ptr->end_mac[1] = 0x00;
			macFilterRule2_ptr->end_mac[2] = 0x1b;
			macFilterRule2_ptr->end_mac[3] = 0xff;
			macFilterRule2_ptr->end_mac[4] = 0xff;
			macFilterRule2_ptr->end_mac[5] = 0xff;			
			/*third*/
			macFilterRule3_ptr = &macFilterRule3;
			memset(macFilterRule3_ptr, 0, sizeof(macFilterRule3));
			macFilterRule3_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule3_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule3_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule3_ptr->start_mac[0] = 0x09;
			macFilterRule3_ptr->start_mac[1] = 0x00;
			macFilterRule3_ptr->start_mac[2] = 0x4e;
			macFilterRule3_ptr->start_mac[3] = 0x00;
			macFilterRule3_ptr->start_mac[4] = 0x00;
			macFilterRule3_ptr->start_mac[5] = 0x02;

			macFilterRule3_ptr->end_mac[0] = 0x09;
			macFilterRule3_ptr->end_mac[1] = 0x00;
			macFilterRule3_ptr->end_mac[2] = 0x4e;
			macFilterRule3_ptr->end_mac[3] = 0x00;
			macFilterRule3_ptr->end_mac[4] = 0x00;
			macFilterRule3_ptr->end_mac[5] = 0x02;
			break;
		case MAC_FILTER_NETBEUL:
			macFilterRule_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule_ptr->start_mac[0] = 0x03;
			macFilterRule_ptr->start_mac[1] = 0x00;
			macFilterRule_ptr->start_mac[2] = 0x00;
			macFilterRule_ptr->start_mac[3] = 0x00;
			macFilterRule_ptr->start_mac[4] = 0x00;
			macFilterRule_ptr->start_mac[5] = 0x01;

			macFilterRule_ptr->end_mac[0] = 0x03;
			macFilterRule_ptr->end_mac[1] = 0x00;
			macFilterRule_ptr->end_mac[2] = 0x00;
			macFilterRule_ptr->end_mac[3] = 0x00;
			macFilterRule_ptr->end_mac[4] = 0x00;
			macFilterRule_ptr->end_mac[5] = 0x01;
			break;
		case MAC_FILTER_AARP:
			/*first*/
			macFilterRule_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_809b;
			macFilterRule_ptr->start_mac[0] = 0xff;
			macFilterRule_ptr->start_mac[1] = 0xff;
			macFilterRule_ptr->start_mac[2] = 0xff;
			macFilterRule_ptr->start_mac[3] = 0xff;
			macFilterRule_ptr->start_mac[4] = 0xff;
			macFilterRule_ptr->start_mac[5] = 0xff;

			macFilterRule_ptr->end_mac[0] = 0xff;
			macFilterRule_ptr->end_mac[1] = 0xff;
			macFilterRule_ptr->end_mac[2] = 0xff;
			macFilterRule_ptr->end_mac[3] = 0xff;
			macFilterRule_ptr->end_mac[4] = 0xff;
			macFilterRule_ptr->end_mac[5] = 0xff;
			/*second*/
			macFilterRule2_ptr = &macFilterRule2;
			memset(macFilterRule2_ptr, 0, sizeof(macFilterRule2));
			macFilterRule2_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule2_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule2_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_80f3;
			macFilterRule2_ptr->start_mac[0] = 0xff;
			macFilterRule2_ptr->start_mac[1] = 0xff;
			macFilterRule2_ptr->start_mac[2] = 0xff;
			macFilterRule2_ptr->start_mac[3] = 0xff;
			macFilterRule2_ptr->start_mac[4] = 0xff;
			macFilterRule2_ptr->start_mac[5] = 0xff;

			macFilterRule2_ptr->end_mac[0] = 0xff;
			macFilterRule2_ptr->end_mac[1] = 0xff;
			macFilterRule2_ptr->end_mac[2] = 0xff;
			macFilterRule2_ptr->end_mac[3] = 0xff;
			macFilterRule2_ptr->end_mac[4] = 0xff;
			macFilterRule2_ptr->end_mac[5] = 0xff;			
			/*third*/
			macFilterRule3_ptr = &macFilterRule3;
			memset(macFilterRule3_ptr, 0, sizeof(macFilterRule3));
			macFilterRule3_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule3_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule3_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule3_ptr->start_mac[0] = 0x09;
			macFilterRule3_ptr->start_mac[1] = 0x00;
			macFilterRule3_ptr->start_mac[2] = 0x07;
			macFilterRule3_ptr->start_mac[3] = 0x00;
			macFilterRule3_ptr->start_mac[4] = 0x00;
			macFilterRule3_ptr->start_mac[5] = 0x00;

			macFilterRule3_ptr->end_mac[0] = 0x09;
			macFilterRule3_ptr->end_mac[1] = 0x00;
			macFilterRule3_ptr->end_mac[2] = 0x07;
			macFilterRule3_ptr->end_mac[3] = 0x00;
			macFilterRule3_ptr->end_mac[4] = 0x00;
			macFilterRule3_ptr->end_mac[5] = 0xfc;
			/*fourth*/
			macFilterRule4_ptr = &macFilterRule4;
			memset(macFilterRule4_ptr, 0, sizeof(macFilterRule4));
			macFilterRule4_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule4_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule4_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule4_ptr->start_mac[0] = 0x09;
			macFilterRule4_ptr->start_mac[1] = 0x00;
			macFilterRule4_ptr->start_mac[2] = 0x07;
			macFilterRule4_ptr->start_mac[3] = 0xff;
			macFilterRule4_ptr->start_mac[4] = 0xff;
			macFilterRule4_ptr->start_mac[5] = 0xff;

			macFilterRule4_ptr->end_mac[0] = 0x09;
			macFilterRule4_ptr->end_mac[1] = 0x00;
			macFilterRule4_ptr->end_mac[2] = 0x07;
			macFilterRule4_ptr->end_mac[3] = 0xff;
			macFilterRule4_ptr->end_mac[4] = 0xff;
			macFilterRule4_ptr->end_mac[5] = 0xff;
			break;
		case MAC_FILTER_MANAGE_INFO:
			macFilterRule_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule_ptr->start_mac[0] = 0x01;
			macFilterRule_ptr->start_mac[1] = 0x80;
			macFilterRule_ptr->start_mac[2] = 0xc2;
			macFilterRule_ptr->start_mac[3] = 0x00;
			macFilterRule_ptr->start_mac[4] = 0x00;
			macFilterRule_ptr->start_mac[5] = 0x00;

			macFilterRule_ptr->end_mac[0] = 0x01;
			macFilterRule_ptr->end_mac[1] = 0x80;
			macFilterRule_ptr->end_mac[2] = 0xc2;
			macFilterRule_ptr->end_mac[3] = 0x00;
			macFilterRule_ptr->end_mac[4] = 0x00;
			macFilterRule_ptr->end_mac[5] = 0xff;
			break;
		case MAC_FILTER_ARP:
			macFilterRule_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0806;
			macFilterRule_ptr->start_mac[0] = 0xff;
			macFilterRule_ptr->start_mac[1] = 0xff;
			macFilterRule_ptr->start_mac[2] = 0xff;
			macFilterRule_ptr->start_mac[3] = 0xff;
			macFilterRule_ptr->start_mac[4] = 0xff;
			macFilterRule_ptr->start_mac[5] = 0xff;

			macFilterRule_ptr->end_mac[0] = 0xff;
			macFilterRule_ptr->end_mac[1] = 0xff;
			macFilterRule_ptr->end_mac[2] = 0xff;
			macFilterRule_ptr->end_mac[3] = 0xff;
			macFilterRule_ptr->end_mac[4] = 0xff;
			macFilterRule_ptr->end_mac[5] = 0xff;
			break;
		case MAC_FILTER_PPPOE_BROADCAST:
			macFilterRule_ptr->filter_type = *macFilterEntry_ptr;
			macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			macFilterRule_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_8863;
			macFilterRule_ptr->start_mac[0] = 0xff;
			macFilterRule_ptr->start_mac[1] = 0xff;
			macFilterRule_ptr->start_mac[2] = 0xff;
			macFilterRule_ptr->start_mac[3] = 0xff;
			macFilterRule_ptr->start_mac[4] = 0xff;
			macFilterRule_ptr->start_mac[5] = 0xff;

			macFilterRule_ptr->end_mac[0] = 0xff;
			macFilterRule_ptr->end_mac[1] = 0xff;
			macFilterRule_ptr->end_mac[2] = 0xff;
			macFilterRule_ptr->end_mac[3] = 0xff;
			macFilterRule_ptr->end_mac[4] = 0xff;
			macFilterRule_ptr->end_mac[5] = 0xff;
			break;			
		default:
			ret = -1;
			goto end;
	}

	/*set mac filter type(0:9.3.6, 1:9.3.7)*/
	if(type == MAC_FILTER_TABLE_ENTRY){
		macFilterType = OMCI_MAC_FILTER_TYPE;
	}else{
		macFilterType = OMCI_PKT_FILTER_TYPE;
	}

//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMacFilterRuleByType-> pon_add_mac_filter_rule->[%d]",__LINE__);

	if(macFilterRule_ptr != NULL){
		ret = pon_add_mac_filter_rule(macFilterRulePortId,macFilterType, macFilterRule_ptr);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacFilterRuleByType-> pon_add_mac_filter_rule->macFilterRule, fail");
		}
	}
	if(macFilterRule2_ptr != NULL){
		ret = pon_add_mac_filter_rule(macFilterRulePortId,macFilterType, macFilterRule2_ptr);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacFilterRuleByType-> pon_add_mac_filter_rule->macFilterRule2, fail");
		}
	}
	if(macFilterRule3_ptr != NULL){
		ret = pon_add_mac_filter_rule(macFilterRulePortId,macFilterType, macFilterRule3_ptr);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacFilterRuleByType-> pon_add_mac_filter_rule->macFilterRule3, fail");
		}
	}
	if(macFilterRule4_ptr != NULL){
		ret = pon_add_mac_filter_rule(macFilterRulePortId,macFilterType, macFilterRule4_ptr);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacFilterRuleByType-> pon_add_mac_filter_rule->macFilterRule4, fail");
		}
	}
	
	ret = 0;
end:
	return ret;

}

/*******************************************************************************************
**function name
	addMacFilterRuleByInst
**description:
	search 9.3.6 or 9.3.7 instance, set mac filter rule to specified port.
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
	classId:	9.3.6 or 9.3.7 class id
	instId :	instance id
  	macFilterRulePortId :	vlan rule port Id
********************************************************************************************/
int addMacFilterRuleByInst(IN uint16 classId, IN uint16 instId, IN uint8 macFilterRulePortId){
	int ret = -1;
//	uint8 currMacFilterRulePortId = 0;
//	uint8 currAttrVal = OMCI_MAC_FILTER_ETHERTYPE_DISABLED;
	pon_mac_filter_rule macFilterRule;
	pon_mac_filter_rule_p macFilterRule_ptr = NULL;
	uint8 attrValue[MAC_FILTER_RECORD_LENGTH] = {0};
	int index = 0;
	omciTableAttriValue_t *tableConent_ptr = NULL;
	omciManageEntity_t *me_p=NULL;
	omciMeInst_t * meInst_p = NULL;	
	uint8 *tableBuffer = NULL;
	uint8 *temp_ptr  = NULL;
	uint32 tableSize = 0;
	uint32 currLength = 0;

	macFilterRule_ptr = &macFilterRule;
	memset(macFilterRule_ptr, 0, sizeof(pon_mac_filter_rule));
	if(classId == OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_TABLE_DATA){//set 9.3.6 mac filter rule
		me_p = omciGetMeByClassId(classId);
		if(me_p ==  NULL){
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addMacFilterRuleByInst->omciGetMeByClassId, fail");
			goto end;
		}
		
		meInst_p = omciGetInstanceByMeId(me_p , instId);
		if(meInst_p ==  NULL){
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addMacFilterRuleByInst->omciGetInstanceByMeId, fail");
			goto end;
		}

		tableConent_ptr = omciGetTableValueByIndex(meInst_p, me_p->omciAttriDescriptList[1].attriIndex);	
		if(tableConent_ptr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addMacFilterRuleByInst->tableConent_ptr is NULL, fail");
			ret = -1;
			goto end;
		}
		
		/*calculate the table size*/
		tableSize = tableConent_ptr->tableSize;
		if(tableSize == 0){
			ret = 0;
			goto end;
		}
		
		/*get table content*/
		tableBuffer = tableConent_ptr->tableValue_p;
		if(tableBuffer == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addMacFilterRuleByInst->tableBuffer is NULL, fail");
			ret = -1;
			goto end;
		}

		/*set mac filter rule by every entry */
		currLength = MAC_FILTER_RECORD_LENGTH;
		while(currLength <= tableSize){
			temp_ptr = tableBuffer+(currLength-MAC_FILTER_RECORD_LENGTH);
			memcpy(attrValue, temp_ptr, MAC_FILTER_RECORD_LENGTH);
			
			ret = setMacFilterRuleByType(MAC_FILTER_TABLE_ENTRY, attrValue, macFilterRulePortId);
			if(ret != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addMacFilterRuleByMacFilterPortId-> setMacFilterRuleByType->MAC_FILTER_TABLE_ENTRY, fail");
				ret = -1;
				goto end;
			}
			currLength += MAC_FILTER_RECORD_LENGTH;
		}
		

	}else if(classId == OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_PREASSIGN_DATA){//set 9.3.7 mac filter rule
		for(index = MAC_FILTER_IPV4_MULTICAST; index<=MAC_FILTER_PPPOE_BROADCAST; index++){
			ret = getAttributeValueByInstId(classId, instId, index, attrValue, 1);
			if(ret != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addMacFilterRuleByMacFilterPortId-> getAttributeValueByInstId->index=%d, fail", index);
				ret = -1;
				goto end;
			}
			if(attrValue[0] == OMCI_MAC_FILTER_RULE_TYPE_DISCARD){ //filter
				ret = setMacFilterRuleByType(index, attrValue, macFilterRulePortId);
				if(ret != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addMacFilterRuleByMacFilterPortId-> setMacFilterRuleByType->index=%d, fail", index);
					ret = -1;
					goto end;
				}
			}
		}
		
	}else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addMacFilterRuleByMacFilterPortId-> classId = 0x%02x, fail", classId);
		ret = -1;
		goto end;
	}
	
	ret = 0;
end:
	if(ret != 0){
		ret = -1;
	}
	return ret;
}
/*******************************************************************************************
**function name
	findMacFilterRuleByRuleInfo
**description:
	find mac filter Rule by mac filter rule information
 **retrun :
 	-1:	not find
 	other:	return find rule index(0~xx)
**parameter:
	macFilterRule_ptr:	mac filter rule
  	macFilterRulePortId :	vlan rule port Id
  	macFilterType: 0:9.3.6, 1:9.3.7
********************************************************************************************/
int findMacFilterRuleByRuleInfo(IN pon_mac_filter_rule_p macFilterRule_ptr, IN uint8 macFilterRulePortId,  IN uint8 macFilterType){
	int findFlag = -1;
	pon_mac_filter_rule_p portMacFilterRule_ptr = NULL;
	pon_mac_filter_rule_p currMacFilterRule_ptr = NULL;
	int currMaxNum = 0;
	int i = 0;
	uint8 entryLen = 0;

	entryLen = sizeof(pon_mac_filter_rule);
	/*calloc space for saving mac filter rule in this port*/
	portMacFilterRule_ptr = (pon_mac_filter_rule_p)calloc(MAX_OMCI_MAC_FILTER_RULE_ONE_PORT , entryLen);
	if(portMacFilterRule_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n findMacFilterRuleByRuleInfo-> calloc, fail");
		findFlag = -1;
		goto end;
	}
	/*get all mac filter rule in this port*/
	currMaxNum = pon_get_mac_filter_rule(macFilterRulePortId,macFilterType, portMacFilterRule_ptr);
	if(currMaxNum == -1){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n findMacFilterRuleByRuleInfo-> pon_get_mac_filter_rule, fail");
		findFlag = -1;
		goto end;
	}
	if(currMaxNum == 0){
		findFlag = -1;
		goto end;
	}
	/*find mac filter rule*/
	currMacFilterRule_ptr = portMacFilterRule_ptr;
	for(i=0; i<currMaxNum; i++){
		if(memcmp(currMacFilterRule_ptr, macFilterRule_ptr, entryLen) == 0){//find
			findFlag = i;
			break;
		}
		currMacFilterRule_ptr++;
	}
	
end:
	if(portMacFilterRule_ptr != NULL){
		free(portMacFilterRule_ptr);
	}

	return findFlag;
}


/*******************************************************************************************
**function name
	findMBPortInfo
**description:
	find mac bridge port information form list
 **retrun :
 	NULL:	not find
 	information node:	success
**parameter:
	instId:	9.3.4 instance id
********************************************************************************************/
mb_port_info_ptr findMBPortInfo(uint16 instId){
	mb_port_info_ptr info_ptr = NULL;

	info_ptr = gMBPortInfo_ptr;
	while(info_ptr != NULL){
		if(info_ptr->instId == instId){
//			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n findMBPortInfo->find!");
			return info_ptr;
		}
		info_ptr = info_ptr->next;
	}

	return NULL;	
}

/*******************************************************************************************
**function name
	addMBPortInfo
**description:
	add or modify mac bridge port information in list
 **retrun :
 	-1:	fail
 	0:	success
**parameter:
	info_ptr:	mac bridge port information
********************************************************************************************/
int addMBPortInfo(IN mb_port_info_ptr  info_ptr){
	int ret = -1;
	mb_port_info_ptr new_info_ptr = NULL;
	mb_port_info_ptr temp_ptr = NULL;

	if(info_ptr == NULL)
		goto end;

	/*find mac port information form list*/
	temp_ptr = findMBPortInfo(info_ptr->instId);
	
	/*set new mac bridge port inromation*/
	
	if(temp_ptr != NULL){//find , modify
		info_ptr->next = temp_ptr->next;
		memcpy(temp_ptr, info_ptr, sizeof(mb_port_info_t));
		ret = 0;
		goto end;
	}
	
	/*no find , create new mac bridge port node*/	
	new_info_ptr = (mb_port_info_ptr)calloc(1 , sizeof(mb_port_info_t));
	if(new_info_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addMBPortInfo->calloc failure");
		goto end;
	}
	memcpy(new_info_ptr, info_ptr, sizeof(mb_port_info_t));
	new_info_ptr->next = NULL;

	/*add new node to list*/
	if(gMBPortInfo_ptr == NULL){
		gMBPortInfo_ptr = new_info_ptr;
		ret = 0;
		goto end;
	}

	temp_ptr = gMBPortInfo_ptr;
	while(temp_ptr->next != NULL){
		temp_ptr = temp_ptr->next;
	}
	temp_ptr->next = new_info_ptr;
	ret = 0;
end:
	if(ret != 0){
		if(new_info_ptr != NULL)
			free(new_info_ptr);
//		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addMBPortInfo->fail");
		ret = -1;	
	}else{
//		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addMBPortInfo->success");

	}
//	displayMBPortInfo();//LHS DBG
	return ret;
	
}
/*******************************************************************************************
**function name
	delMBPortInfo
**description:
	del mac bridge port information node from list
 **retrun :
 	-1:	fail
 	0:	success
**parameter:
	instId:	9.3.4 instnace id
********************************************************************************************/
int delMBPortInfo(IN uint16 instId){
	int ret = -1;
	mb_port_info_ptr info_ptr = NULL;
	mb_port_info_ptr pre_ptr = NULL;

	info_ptr = gMBPortInfo_ptr;
	pre_ptr = gMBPortInfo_ptr;
	while(info_ptr != NULL){
		if(info_ptr->instId == instId){
			if(info_ptr == gMBPortInfo_ptr){
				gMBPortInfo_ptr = info_ptr->next;
			}else{
				pre_ptr->next = info_ptr->next;
			}
			free(info_ptr);
//			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n delMBPortInfo->find del success");
			break;
		}
		pre_ptr = info_ptr;
		info_ptr = info_ptr->next;
	}

	ret = 0;
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n delMBPortInfo->find or no find success");
//	displayMBPortInfo();//LHS DBG
	return ret;
}
/*******************************************************************************************
**function name
	clearMBPortInfo
**description:
	clear mac bridge port information list
 **retrun :
 	-1:	fail
 	0:	success
**parameter:
********************************************************************************************/
int clearMBPortInfo(void){
	mb_port_info_ptr info_ptr = NULL;
	mb_port_info_ptr temp_ptr = NULL;

	info_ptr = gMBPortInfo_ptr;
	while(info_ptr != NULL){
		temp_ptr = info_ptr->next;
		free(info_ptr);
		info_ptr = temp_ptr;
	}

	gMBPortInfo_ptr = NULL;
	return 0;
}

/*******************************************************************************************
**function name
	displayMBPortInfo
**description:
	display all mac bridge port information node from list
 **retrun :
 	-1:	fail
 	0:	success
**parameter:
********************************************************************************************/
void displayMBPortInfo(){
	mb_port_info_ptr info_ptr = NULL;
	int i = 0;
	
	info_ptr = gMBPortInfo_ptr;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n displayMBPortInfo->");
	while(info_ptr != NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n info_ptr->instId = 0x%02x",info_ptr->instId);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n info_ptr->type = 0x%02x",info_ptr->type);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n info_ptr->reserve = 0x%02x",info_ptr->reserve);
		for(i=0; i<MAX_MAC_BRIDGE_PORT_GEM_PORT_NUM; i++){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n info_ptr->gemlist[%d][0]=0x%02x,info_ptr->gemlist[%d][1]=0x%02x",i,info_ptr->gemlist[i][0],i, info_ptr->gemlist[i][1]);
		}
		info_ptr = info_ptr->next;
	}
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n");

	return;
}

/*******************************************************************************************
**function name
	
**description:
	clear mac bridge global infomation
 **retrun :
	0:	success
	-1: failure
**parameter:
********************************************************************************************/
int clearOmciMBInfo(void){
	int i = 0;

	/*clear global validAniPortFlag variable*/
	for(i=0;i<MAX_OMCI_ANI_PORT_NUM;i++){
		validAniPortFlag[i][0] = 0;
		validAniPortFlag[i][1] = OMCI_INVALID_MAC_BRIDGE_PORT_FLAG;
	}
	/*clear global omci mac bridge port information*/
	clearMBPortInfo();

	return 0;	
}

/*******************************************************************************************
**function name
	getFreeOmciAniPortId
**description:
	get free omci ani port id
 **retrun :
 	1:	fail
 	0:	success
**parameter:
	omcAniPortId:	return ani port id
********************************************************************************************/
int getFreeOmciAniPortId(OUT uint16 * omcAniPortId){
	int i = 0;
	int ret = OMCI_MAC_BRIDGE_PORT_FAIL;

	for(i=0; i<MAX_OMCI_ANI_PORT_NUM; i++){
		if(validAniPortFlag[i][OMCI_ANI_PORT_VALID_INDEX] == OMCI_INVALID_MAC_BRIDGE_PORT_FLAG){
			*omcAniPortId = i;
			ret = OMCI_MAC_BRIDGE_PORT_SUCCESS;
			break;
		} 
	}
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getFreeOmciAniPortId-->ret = 0x%02x", ret);
	return ret;
}
/*******************************************************************************************
**function name
	findOmciAniPortId
**description:
	find omci ani port id
 **retrun :
 	1:	fail
 	0:	success
**parameter:
	instId:	9.3.4 instance id
	findFlag: 0:no find, 1:find
	omcAniPortId:find ani port id
********************************************************************************************/
int findOmciAniPortId(uint16 instId, uint8 *findFlag, uint16 * omcAniPortId ){
	int i = 0;
	int ret = OMCI_MAC_BRIDGE_PORT_FAIL;

	if(findFlag == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n findFlag==NULL");
		goto end;
	}

	*findFlag = OMCI_NO_FIND_RULE;
	
	for(i=0; i<MAX_OMCI_ANI_PORT_NUM; i++){
		if((validAniPortFlag[i][OMCI_ANI_PORT_INST_ID_INDEX] == instId)
			&& (validAniPortFlag[i][OMCI_ANI_PORT_VALID_INDEX] == OMCI_VALID_MAC_BRIDGE_PORT_FLAG)){
			*findFlag = OMCI_FIND_RULE;
			*omcAniPortId = i;
			break;
		} 
	}
	
	ret = OMCI_MAC_BRIDGE_PORT_SUCCESS;	
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n findOmciAniPortId-->ret = 0x%02x", ret);
	return ret;
}

/*******************************************************************************************
**function name
	modifyMacBridgePortDeviceId
**description:
	set  device id of mac bridge ANI port
 **retrun :
 	1:	fail
 	0:	success
**parameter:
	instId: 9.3.4 instance id in ANI 
	type: 0:add device id, 1:del device id, 2:modify
********************************************************************************************/
int setMacBridgePortDeviceId(IN uint16 instId, IN uint8 type){
	omciMeInst_t *meInst_p = NULL;
	omciManageEntity_t *me_p = NULL;
	int ret = OMCI_MAC_BRIDGE_PORT_FAIL;
	uint16 aniPortId = 0;
	int i = 0;
	uint8 findFlag = OMCI_NO_FIND_RULE;
	uint16 gemPortList[MAX_MAC_BRIDGE_PORT_GEM_PORT_NUM] = {0};
	uint16 oldGemPortList[MAX_MAC_BRIDGE_PORT_GEM_PORT_NUM] = {0};
	uint16 oldValidGemPortNum = 0;
	uint8 validGemPortNum = 0;
	mb_port_info_ptr mbPortInfo_ptr = NULL;
	mb_port_info_ptr tmpInfo_ptr = NULL;
//	char cmd[128] = {0};

#if 0
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMacBridgePortDeviceId->Start, instId=0x%02x, type=0x%02x",instId, type);
//	for(i=0; i<MAX_OMCI_ANI_PORT_NUM; i++){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMacBridgePortDeviceId->old->validAniPortFlag[%d][0]=0x%02x",i, validAniPortFlag[i][0]);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMacBridgePortDeviceId->old->validAniPortFlag[%d][1]=0x%02x",i, validAniPortFlag[i][1]);

//	}
#endif	
	if((type != OMCI_ADD_MAC_BRIDGE_ANI_PORT) 
		&& (type != OMCI_DEL_MAC_BRIDGE_ANI_PORT)
		&& (type != OMCI_MOD_MAC_BRIDGE_ANI_PORT)
		&& (type != OMCI_RECFG_MAC_BRIDGE_ANI_PORT)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortDeviceId->type=0x%02x,  fail", type);
		goto end;
	}
	
	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortDeviceId->omciGetMeByClassId fail");
		goto end;
	}
	
	meInst_p = omciGetInstanceByMeId(me_p , instId);	
	if(meInst_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortDeviceId->omciGetInstanceByMeId fail");
		goto end;
	}

	if(findOmciAniPortId(instId, &findFlag, &aniPortId) == OMCI_MAC_BRIDGE_PORT_FAIL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMacBridgePortDeviceId-->findOmciAniPortId, fail");
		goto end;
	}

	/*calloc memory for storing mac bridge port information*/
 
	mbPortInfo_ptr = (mb_port_info_ptr)calloc(1 , sizeof(mb_port_info_t));
	if(mbPortInfo_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortDeviceId->calloc failure");
		goto end;
	}
	memset(mbPortInfo_ptr,0, sizeof(mb_port_info_t));
	
	if((type == OMCI_MOD_MAC_BRIDGE_ANI_PORT)
		|| (type == OMCI_ADD_MAC_BRIDGE_ANI_PORT)
		|| (type == OMCI_RECFG_MAC_BRIDGE_ANI_PORT)){
		
		/*get gem port id*/
		if(getGemPortByMACBridgePortInst(instId, gemPortList, &validGemPortNum) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMacBridgePortDeviceId->getGemPortByMACBridgePortInst,instId=%d, fail",instId);
		}
		/*set value to mac bridge port information node */
		mbPortInfo_ptr->instId = instId;
		mbPortInfo_ptr->type = OMCI_MAC_BRIDGE_PORT_WAN_IFC;
		mbPortInfo_ptr->reserve = 0;
		for(i=0; i<validGemPortNum; i++){
			mbPortInfo_ptr->gemlist[i][0] = OMCI_VALID_MB_PORT_GEM_PORT;
			mbPortInfo_ptr->gemlist[i][1] = gemPortList[i];
		}

		if(findFlag == OMCI_NO_FIND_RULE){//add
			if(type == OMCI_ADD_MAC_BRIDGE_ANI_PORT){
				if(getFreeOmciAniPortId(&aniPortId) == OMCI_MAC_BRIDGE_PORT_FAIL){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortDeviceId-->getFreeOmciAniPortId,type=0x%02x, fail", type);
					goto end;
				}
				/*add mac bridge port information node to list*/
				if(addMBPortInfo(mbPortInfo_ptr) != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortDeviceId-->addMBPortInfo,9.3.4 instance id=0x%02x, fail", instId);
					goto end;
				}
				meInst_p->deviceId = aniPortId;
				validAniPortFlag[aniPortId][OMCI_ANI_PORT_INST_ID_INDEX] = instId;
				validAniPortFlag[aniPortId][OMCI_ANI_PORT_VALID_INDEX] = OMCI_VALID_MAC_BRIDGE_PORT_FLAG;
				/*create PON interface API*/

				if(handlePonIfc(GPON_HANDLE_PON_IFC_CREATE,aniPortId,gemPortList,validGemPortNum,instId) != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortDeviceId-->handlePonIfc create,type=0x%02x, fail", type);
				}
			}


		}else{//find and modify
			if(type != OMCI_RECFG_MAC_BRIDGE_ANI_PORT){
				/*unassign gem port */
				tmpInfo_ptr = findMBPortInfo(instId);
				if((tmpInfo_ptr != NULL) 
					&& (tmpInfo_ptr->type == OMCI_MAC_BRIDGE_PORT_WAN_IFC)){
					for(i=0; i<MAX_MAC_BRIDGE_PORT_GEM_PORT_NUM; i++){
						if(tmpInfo_ptr->gemlist[i][0] == OMCI_VALID_MB_PORT_GEM_PORT){
	//						sprintf(cmd,"%s gpon set gem_unassign %d",OMCI_PONMGR_CMD,tmpInfo_ptr->gemlist[i][1]);
	//						system(cmd);
							oldGemPortList[oldValidGemPortNum] = tmpInfo_ptr->gemlist[i][1];
							oldValidGemPortNum++;
						}
					}

				}			
				if(oldValidGemPortNum > 0){
					if(handlePonIfc(GPON_HANDLE_PON_IFC_UNASSIGN,aniPortId,oldGemPortList,oldValidGemPortNum,instId) != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortDeviceId-->handlePonIfc unassign gemport,type=0x%02x, fail", type);
					}
				}
			}
			
			/*modify mac bridge port information node to list*/
			if(addMBPortInfo(mbPortInfo_ptr) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortDeviceId-->addMBPortInfo(modify),9.3.4 instance id=0x%02x, fail", instId);
				goto end;
			}
			if(type != OMCI_RECFG_MAC_BRIDGE_ANI_PORT){
			/*modify PON interface API*/			
				if(handlePonIfc(GPON_HANDLE_PON_IFC_CREATE,aniPortId,gemPortList,validGemPortNum,instId) != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortDeviceId-->handlePonIfc modify,type=0x%02x, fail", type);
				}
			}
			else{
			if(handlePonIfc(GPON_HANDLE_PON_IFC_MODIFY,aniPortId,gemPortList,validGemPortNum,instId) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortDeviceId-->handlePonIfc modify,type=0x%02x, fail", type);
			}
		}
		}
	}else if(type == OMCI_DEL_MAC_BRIDGE_ANI_PORT){
		/*delete PON interface API*/
		if(findFlag == OMCI_FIND_RULE){
			if(validAniPortFlag[aniPortId][OMCI_ANI_PORT_VALID_INDEX] == OMCI_VALID_MAC_BRIDGE_PORT_FLAG){
				if(handlePonIfc(GPON_HANDLE_PON_IFC_DEL,aniPortId,NULL,0,instId) != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortDeviceId-->handlePonIfc del,type=0x%02x, fail", type);
				}
				/*modify mac bridge port information node to list*/
				if(delMBPortInfo(instId) != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMacBridgePortDeviceId-->delMBPortInfo,9.3.4 instance id=0x%02x, fail", instId);
				}
			}
			validAniPortFlag[aniPortId][OMCI_ANI_PORT_INST_ID_INDEX] = 0;
			validAniPortFlag[aniPortId][OMCI_ANI_PORT_VALID_INDEX] = OMCI_INVALID_MAC_BRIDGE_PORT_FLAG;
			meInst_p->deviceId = 0;
		}
	}
	ret = OMCI_MAC_BRIDGE_PORT_SUCCESS;
end:

	if(mbPortInfo_ptr != NULL){
		free(mbPortInfo_ptr);
	}
#if 0
	for(i=0; i<MAX_OMCI_ANI_PORT_NUM; i++){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMacBridgePortDeviceId->old->validAniPortFlag[%d][0]=0x%02x",i, validAniPortFlag[i][0]);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMacBridgePortDeviceId->old->validAniPortFlag[%d][1]=0x%02x",i, validAniPortFlag[i][1]);
	}

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMacBridgePortDeviceId-->ret=0x%02x",ret);
#endif
	return ret;
}

/*******************************************************************************************
**function name
	omciRecfgIfc
**description:
	reset mapping rule between ANI interface and gemport.
 **retrun :
 	-1:	fail
 	0:	success
**parameter:
	none
********************************************************************************************/

int omciRecfgIfcTimeOut(void){
	int ret = OMCI_MAC_BRIDGE_PORT_FAIL;
	int i = 0;

	for(i = 0; i<MAX_OMCI_ANI_PORT_NUM; i++){
		if(validAniPortFlag[i][OMCI_ANI_PORT_VALID_INDEX] == OMCI_VALID_MAC_BRIDGE_PORT_FLAG){
			if(setMacBridgePortDeviceId(validAniPortFlag[i][OMCI_ANI_PORT_INST_ID_INDEX], OMCI_RECFG_MAC_BRIDGE_ANI_PORT) == OMCI_MAC_BRIDGE_PORT_FAIL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciRecfgIfc->validAniPortFlag[%d][0]=0x%02x, fail[%d]",i, validAniPortFlag[i][OMCI_ANI_PORT_INST_ID_INDEX], __LINE__);
			}
		}

	}

	ret = OMCI_MAC_BRIDGE_PORT_SUCCESS;
	return ret;	
}


int omciRecfgIfc(void)
{
    /* use time function ; because trap handler may 
    block OMCI RX handler while doing this action */
    OMCI_TRACE("[%s][%d] start timer for omciRecfgIfc\r\n",__FUNCTION__,__LINE__);
    if (timerStartS(500 , omciRecfgIfcTimeOut, NULL) < 0 )
    {
        OMCI_ERROR("omci delay  omciRecfgIfc fail!");
        return -1;
    }
    return 0;
}


/*******************************************************************************************
**function name
	updateAniInfoBy8021p
**description:
	undate ANI port some information according to the 9.3.10 ME
 **retrun :
 	-1:	fail
 	0:	success
**parameter:
	instId: 9.3.10 instance id
********************************************************************************************/
int updateAniInfoBy8021p(uint16 instId){
	omciInst_t *omciInst_p ;	
	omciMeInst_t *meInst_p = NULL;
	omciManageEntity_t *me_p = NULL;
	int ret = OMCI_MAC_BRIDGE_PORT_FAIL;
//	int i = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	uint8 tpType= 0;
	uint16 tpPointer = 0;
	uint16 macPortInstId = 0;
	
	/* find the instance and exec it */
	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateAniInfoBy8021p->omciGetMeByClassId fail");
		goto end;
	}	
	omciInst_p = &(me_p->omciInst);	
	if((omciInst_p == NULL) || (omciInst_p->omciMeInstList == NULL)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n updateAniInfoBy8021p omciMeInstList == NULL");
		goto end;
	}
	meInst_p = omciInst_p->omciMeInstList;
	while(meInst_p != NULL){
		/*get instance id in mac bridge port ME*/
		macPortInstId = get16(meInst_p->attributeVlaue_ptr);
		
		/* get TP Type attribute in this instance*/
		attributeValuePtr = (uint8 *)omciGetInstAttriByName(meInst_p , me_p->omciAttriDescriptList[3].attriName, &length);
		if(attributeValuePtr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateAniInfoBy8021p->omciGetInstAttriByName tp type, failure");
			goto end;
		}
		tpType = *attributeValuePtr;
		
		if(tpType == OMCI_MAC_BRIDGE_PORT_TP_TYPE_3){
			/*get TP pointer attribute in this instance*/			
			attributeValuePtr = (uint8 *)omciGetInstAttriByName(meInst_p , me_p->omciAttriDescriptList[4].attriName, &length);
			if(attributeValuePtr == NULL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateAniInfoBy8021p->omciGetInstAttriByName tp pointer, failure");
				goto end;
			}
			tpPointer = get16(attributeValuePtr);
			/*update ANI information*/
			if(tpPointer == instId){
				/*update ANi port and ANI interface information*/
				if(setMacBridgePortDeviceId(macPortInstId, OMCI_MOD_MAC_BRIDGE_ANI_PORT) == OMCI_MAC_BRIDGE_PORT_FAIL){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateAniInfoBy8021p->setMacBridgePortDeviceId, failure");
				}
				/*update mac bridge port information in OMCI_Entry node*/
				if(setMacBridgePortInfoToRomfile(macPortInstId) != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateAniInfoBy8021p->setMacBridgePortInfoToRomfile, failure");
				}

			}
		}
		meInst_p = meInst_p->next;
	}

	ret = OMCI_MAC_BRIDGE_PORT_SUCCESS;
end:
	return ret;

}

/*******************************************************************************************
**function name
	uint16totimeval
**description:
	convert uint16 value to time value, reference strtotimeval function  in brctl_cmd.c
 **retrun :
 	-1:	fail
 	0:	success
**parameter:
	tv: time value pointer
	time: uint16 seconce value
********************************************************************************************/
static int uint16totimeval(struct timeval *tv, uint16 time)
{
	double secs;

	secs = (double)time;
	tv->tv_sec = secs;
	tv->tv_usec = 1000000 * (secs - tv->tv_sec);
	return 0;
}

/*******************************************************************************************************************************
start
9.3.1 MAC bridge Service Profile ME

********************************************************************************************************************************/

/*init 9.3.1 MAC Bridge Service Profile ME*/
int omciMeInitForMACBridgeServiceProfile(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	return 0;
}

int setMBSPSpanningTreeIndValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	uint8 spanTreeInd = 0;
	int	ret = -1;
	uint16 mask = 0;
	uint16 instanceId = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	instanceId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	mask = OMCI_THREAD_BR_PRO_STP;

 	spanTreeInd = *((uint8*)value);
	if((spanTreeInd != 0) && (spanTreeInd != 1)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBSPSpanningTreeIndValue-> para error!, [%d]",__LINE__);
		goto end;
	}
	
	if(setGeneralValue(value, meInstantPtr , omciAttribute, flag) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBSPSpanningTreeIndValue-> setGeneralValue fail!, [%d]",__LINE__);
		goto end;
	}
	if(flag == OMCI_SET_ACTION_SET_VAL){
		if(omciBrProVal(instanceId, mask) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBSPSpanningTreeIndValue-> omciBrProVal fail!, [%d]",__LINE__);
		}
	}
	ret = 0;
end:
	return ret;
}

int setMBSPLearningIndValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	uint8 learningInd = 0;
	int	ret = 0;
//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBSPLearningIndValue start");	
	learningInd = *((uint8*)value);
	if(learningInd != 1){/*only support set enabled, and it is always 1.*/
		goto failure;
	}

	return setGeneralValue(value, meInstantPtr , omciAttribute, flag);

failure:
	ret = -1;
	return ret;
}
int32 getMBSPPortBridingIndValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	uint8 val  = 0;
	uint8 mask[MAX_OMCI_LAN_PORT_NUM] = {0};
//	int i = 0;

	if(((macMT7530GetPortBrgInd(OMCI_MAC_BRIDGE_LAN_PORT1, &mask[0]) == 0)	&& ((mask[0] & MAX_OMCI_LAN_PORT_MASK)== MAX_OMCI_LAN_PORT_MASK))
		&& ((macMT7530GetPortBrgInd(OMCI_MAC_BRIDGE_LAN_PORT2, &mask[1]) == 0)	&& ((mask[1] & MAX_OMCI_LAN_PORT_MASK)== MAX_OMCI_LAN_PORT_MASK))
		&& ((macMT7530GetPortBrgInd(OMCI_MAC_BRIDGE_LAN_PORT3, &mask[2]) == 0)	&& ((mask[2] & MAX_OMCI_LAN_PORT_MASK)== MAX_OMCI_LAN_PORT_MASK))
		&& ((macMT7530GetPortBrgInd(OMCI_MAC_BRIDGE_LAN_PORT4, &mask[3]) == 0)	&& ((mask[3] & MAX_OMCI_LAN_PORT_MASK)== MAX_OMCI_LAN_PORT_MASK))){
		val = 1;
	}else{
		val = 0;
	}
#if 0
	for(i = 0; i<MAX_OMCI_LAN_PORT_NUM; i++){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMBSPPortBridingIndValue mask[%d]= 0x02x", i, mask[i]);	
	}
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMBSPPortBridingIndValue val = %d", val);
#endif

	return getTheValue(value, (char *)&val, 1, omciAttribute);	

}

int setMBSPPortBridingIndValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	uint8 portBridgingInd = 0;
	int	ret = 0;
	uint8 mask[MAX_OMCI_LAN_PORT_NUM] = {0};
	int i = 0;

	if(omci.omciCap.systemCap.onuType != 2) //2--hgu modes
	{
	portBridgingInd = *((uint8*)value);
	switch(portBridgingInd){
		case 0:
			for(i = 0; i<MAX_OMCI_LAN_PORT_NUM; i++){
				mask[i] = (OMCI_LAN_PORT1_OTHER_MASK | (1<<i));
			}
			break;
		case 1:
			for(i = 0; i<MAX_OMCI_LAN_PORT_NUM; i++){
				mask[i] = MAX_OMCI_LAN_PORT_DEFAULT_MASK;
			}
			break;
		default:
			goto failure;
			break;
	
	}

	for(i = 0; i<MAX_OMCI_LAN_PORT_NUM; i++){
		if(macMT7530SetPortBrgInd(i, mask[i]) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBSPPortBridingIndValue->macMT7530SetPortBrgInd mask[%d]= 0x%02x, fail[%d]",i, mask[i],__LINE__);
		}
	}
	}

	ret = 0;
	return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
failure:
	ret = -1;
	return ret;
}

int setMBSPPriorityValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	uint16 priority = 0;
	uint8 * temp = NULL;
	int	ret = -1;
	uint16 mask = 0;
	uint16 instanceId = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	instanceId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	mask = OMCI_THREAD_BR_PRO_PRI;

 	temp = (uint8 *)value;
	priority = get16(temp);

	if(setGeneralValue(value, meInstantPtr , omciAttribute, flag) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBSPPriorityValue-> setGeneralValue fail!, [%d]",__LINE__);
		goto end;
	}
	if(flag == OMCI_SET_ACTION_SET_VAL){
		if(omciBrProVal(instanceId, mask) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBSPPriorityValue-> omciBrProVal fail!, [%d]",__LINE__);
		}
	}
	ret = 0;
end:
	return ret;
}

int setMBSPMaxAgeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	uint16	maxAge = 0;
	uint8 * temp = NULL;
	int	ret = -1;
	uint16 mask = 0;
	uint16 instanceId = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	instanceId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	mask = OMCI_THREAD_BR_PRO_MAXAGE;
	
	temp = (uint8 *)value;
	maxAge = get16(temp);

	if(maxAge & 0xff00){
		maxAge = (maxAge>>8);
	}
	if((maxAge <MIN_STP_MAX_AGE)
		||(maxAge >MAX_STP_MAX_AGE)){
		goto end;
	}
	if(setGeneralValue(value, meInstantPtr , omciAttribute, flag) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBSPMaxAgeValue-> setGeneralValue fail!, [%d]",__LINE__);
		goto end;
	}
	if(flag == OMCI_SET_ACTION_SET_VAL){
		if(omciBrProVal(instanceId, mask) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBSPMaxAgeValue-> omciBrProVal fail!, [%d]",__LINE__);
		}
	}
	ret = 0;
end:
	return ret;
}

int setMBSPHelloTimeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	uint16	helloTime = 0;
	uint8 * temp = NULL;
	int	ret = -1;
	uint16 mask = 0;
	uint16 instanceId = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	instanceId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	mask = OMCI_THREAD_BR_PRO_HELLO;
		
	temp = (uint8 *)value;
	helloTime = get16(temp);

	if(helloTime & 0xff00){
		helloTime = (helloTime>>8);
	}
	if((helloTime <MIN_STP_HELLO_TIME)
		||(helloTime >MAX_STP_HELLO_TIME)){
		goto end;
	}

	if(setGeneralValue(value, meInstantPtr , omciAttribute, flag) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBSPHelloTimeValue-> setGeneralValue fail!, [%d]",__LINE__);
		goto end;
	}
	if(flag == OMCI_SET_ACTION_SET_VAL){
		if(omciBrProVal(instanceId, mask) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBSPHelloTimeValue-> omciBrProVal fail!, [%d]",__LINE__);
		}
	}
	ret = 0;
end:
	return ret;
}

int setMBSPForwardDelayValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	uint16	forwardDelay = 0;
	uint8 * temp = NULL;
	int	ret = -1;
	uint16 mask = 0;
	uint16 instanceId = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	instanceId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	switch(flag){
		case OMCI_SET_ACTION_SET_VAL:
			mask = OMCI_THREAD_BR_PRO_FORWARD;
			break;
		case OMCI_CREATE_ACTION_SET_VAL:
			mask = (OMCI_THREAD_BR_PRO_STP|OMCI_THREAD_BR_PRO_PRI|OMCI_THREAD_BR_PRO_MAXAGE|OMCI_THREAD_BR_PRO_HELLO|OMCI_THREAD_BR_PRO_FORWARD);
			break;
		default:
			goto end;			
	}	
	
	temp = (uint8 *)value;
	forwardDelay = get16(temp);	

	if(forwardDelay & 0xff00){
		forwardDelay = (forwardDelay>>8);
	}

	if((forwardDelay <MIN_STP_FORWARD_DELAY)
		||(forwardDelay >MAX_STP_FORWARD_DELAY)){
		goto end;
	}
	
	if(setGeneralValue(value, meInstantPtr , omciAttribute, flag) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBSPHelloTimeValue-> setGeneralValue fail!, [%d]",__LINE__);
		goto end;
	}
	if(omciBrProVal(instanceId, mask) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBSPHelloTimeValue-> omciBrProVal fail!, [%d]",__LINE__);
	}
	ret = 0;
end:
	return ret;
}

/*The boolean value true specifies that MAC frames with unknown destination addresses be discarded
   The value false specifies that such frames be forwarded to all allowed ports.
*/
int32 getMBSPUnKnownMacAddrDiscardValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	uint8 val  = 0;
//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMBSPUnKnownMacAddrDiscardValue start");		
	if((macMT7530GetDiscardUnknownMacInd(OMCI_MAC_BRIDGE_LAN_PORT1) == OMCI_MAC_BRIDGE_ENABLE_DISCARD)
		&& (macMT7530GetDiscardUnknownMacInd(OMCI_MAC_BRIDGE_LAN_PORT2) == OMCI_MAC_BRIDGE_ENABLE_DISCARD)
		&& (macMT7530GetDiscardUnknownMacInd(OMCI_MAC_BRIDGE_LAN_PORT3) == OMCI_MAC_BRIDGE_ENABLE_DISCARD)
		&& (macMT7530GetDiscardUnknownMacInd(OMCI_MAC_BRIDGE_LAN_PORT4) == OMCI_MAC_BRIDGE_ENABLE_DISCARD)
		){
		val = 1;
	}else{
		val = 0;
	}	
	return getTheValue(value, (char *)&val, 1, omciAttribute);

}
int setMBSPUnKnownMacAddrDiscardValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	uint8 unKnownMacAddrDiscard = 0;
	uint8 ind = 0;
	int	ret = 0;
//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBSPUnKnownMacAddrDiscardValue start");	
	unKnownMacAddrDiscard = *((uint8*)value);
	if(unKnownMacAddrDiscard == 0){
		ind = OMCI_MAC_BRIDGE_DISABLE_DISCARD;
	}else if(unKnownMacAddrDiscard == 1){
		ind = OMCI_MAC_BRIDGE_ENABLE_DISCARD;
	}else{
		goto failure;
	}
	
	if((macMT7530SetDiscardUnknownMacInd(OMCI_MAC_BRIDGE_LAN_PORT1, ind) == 0)
		&& (macMT7530SetDiscardUnknownMacInd(OMCI_MAC_BRIDGE_LAN_PORT2, ind) == 0)
		&& (macMT7530SetDiscardUnknownMacInd(OMCI_MAC_BRIDGE_LAN_PORT3, ind) == 0)
		&& (macMT7530SetDiscardUnknownMacInd(OMCI_MAC_BRIDGE_LAN_PORT4, ind) == 0)
		){
		ret = 0;
	}else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBSPUnKnownMacAddrDiscardValue->macMT7530SetDiscardUnknownMacInd fail[%d]",__LINE__);
	}
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBSPUnKnownMacAddrDiscardValue->unKnownMacAddrDiscard =  0x%0x", unKnownMacAddrDiscard);

	ret = 0;
	return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
failure:
	ret = -1;
	return ret;
}

int setMBSPMacAddrLimitValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	uint8 ind = 0;
	int	ret = 0;
	uint16 mbspInstanceId = 0;
	uint16 mbpcdInstanceId = 0;
	uint16 mbpcdAssMbspInstanceID = 0;
	uint8 ifaceType = 0; //0:UNI, 1:ANI
	uint8 portId = 0;
	uint8 *attributeValuePtr = NULL;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint8 macAddrLimitFromMBPCD = 0,macAddrLimitFromMBSP = 0;
	#ifdef TCSUPPORT_BRIDGE_MAC_LIMIT
	char cmd[64];
	#endif

	if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL){
		OMCI_ERROR("setMBSPMacAddrLimitValue-> parameter is null, fail\n");
		goto failure;
	}
	macAddrLimitFromMBSP =  *((uint8*)value);

	OMCI_ERROR("setMBSPMacAddrLimitValue start macAddrLimit=%d\n",macAddrLimitFromMBSP); 
    #ifdef TCSUPPORT_BRIDGE_MAC_LIMIT
	if(HGU != pSystemCap->onuType)
	{
		/* set mac limit total */
		sprintf(cmd,"echo %d > /proc/br_fdb/mac_limit_total",macAddrLimitFromMBSP);
		system(cmd);
		system("/userfs/bin/hw_nat -!");
        OMCI_TRACE("%s set mac bridge service profile learning depth %d\n",__FUNCTION__,macAddrLimitFromMBSP);	
	}
	#endif
	ret = 0;
	return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
failure:
	ret = -1;
	return ret;
}


/*******************************************************************************************************************************
start
9.3.2 MAC bridge configuration data ME

********************************************************************************************************************************/
/*init 9.3.2 MAC Bridge Configuration Data ME*/
int omciMeInitForMACBridgeConData(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciInternalCreateActionForMACBridge;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	return 0;
}
int32 getMBCDBridgeMACAddrValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	uint8 macAddr[6] = {0};
	
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n 9.3.2-->getMBCDBridgeMACAddrValue start");		
	getOmciEtherAddr(macAddr);
	return getTheValue(value, (char *)macAddr, 6, omciAttribute);
}

int32 getMBCDBridgePriorityValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	uint8 priority[2] = {0}; 
	uint16 instanceId = 0;
	omciMeInst_t * meInst = (omciMeInst_t *)meInstantPtr;
	int ret = 0;

	/* same with 9.3.1*/
	instanceId = get16(meInst->attributeVlaue_ptr);
	
	ret =  getAttributeValueByInstId(OMCI_ME_CLASS_ID_MAC_BRIDGE_SERVICE_PROFILE, instanceId,4, priority, 2);
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMBCDBridgePriorityValue->getAttributeValueByInstId fail");
		return ret;
	}
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMBCDBridgePriorityValue->priority[0]=0x%0x, priority[1]=0x%0x \r\n",priority[0], priority[1]);
	
	return getTheValue(value, (char *)priority, 2, omciAttribute);
}

int32 getMBCDDesignatedRootValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	uint8 designatedRoot[8] = {0};
	uint8 macAddr[6] = {0};
	uint8 priority[2] = {0}; 
	uint16 instanceId = 0;
	omciMeInst_t * meInst = (omciMeInst_t *)meInstantPtr;
	int ret = -1;
//	int i = 0;

	/* priorty  same with 9.3.1*/	
	instanceId = get16(meInst->attributeVlaue_ptr);
	ret =  getAttributeValueByInstId(OMCI_ME_CLASS_ID_MAC_BRIDGE_SERVICE_PROFILE, instanceId,4, priority, 2);	
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMBCDDesignatedRootValue->getAttributeValueByInstId fail");
		ret = -1;
		goto end;
	}
	memcpy(designatedRoot, priority, 2);
	
	getOmciEtherAddr(macAddr);
	memcpy(designatedRoot+2, macAddr, 6);

	ret = 0;
end:
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n 9.3.2-->getMBCDDesignatedRootValue->getAttributeValueByInstId, ret=0x%02x", ret);
	if(ret == 0){
		return getTheValue(value, (char *)designatedRoot, 8, omciAttribute);
	}else{
		ret = -1;
		return ret;
	}
}

int32 getMBCDRootPathCostValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	uint8 temp[4] = {0,0,0,0};/*this bridge is root bridge, so return 0*/

//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMBCDRootPathCostValue->temp[0]=%d,temp[1]=%d, temp[2]=%d, temp[3]=%d", temp[0],temp[1],temp[2],temp[3]);
	
	return getTheValue(value, (char *)temp, 4, omciAttribute);

}

int32 getMBCDBridgePortCountValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret;
	uint16 instanceId = 0;
	uint8 portCnt = 0;
	omciMeInst_t * meInst = (omciMeInst_t *)meInstantPtr;
	
	
	instanceId = get16(meInst->attributeVlaue_ptr);	
	ret = getMacBridgePortCnt(instanceId, &portCnt);
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n 9.3.2-->getMBCDBridgePortCountValue->getMacBridgePortCnt fail");
		goto failure;
	}

//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMBCDBridgePortCountValue->portCnt=0x%0x",portCnt);
	
	return getTheValue(value, (char *)&portCnt, 1, omciAttribute);
failure:
	return ret;
}

int32 getMBCDRootPortNumValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
//	uint8 temp[2] = {0,0};/*this bridge is root bridge, so return 0*/
	uint16 temp = 0;

//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMBCDRootPortNumValue->temp[0]=%d,temp[1]=%d, temp[2]=%d, temp[3]=%d", temp[0],temp[1],temp[2],temp[3]);
	
	return getTheValue(value, (char *)&temp, 2, omciAttribute);
}

/*******************************************************************************************************************************
start
9.3.3 MAC bridge performance monitoring history data

********************************************************************************************************************************/

/*init 9.3.3 MAC bridge performance monitoring history data ME*/
int omciMeInitForMACBridgePMHD(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_CURR_DATA] = omciGetCurrentDataAction;	

	return 0;
}

int32 setMBPMHDThresholdDataIdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag){
//	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	//int ret = 0;
//	uint32 thresholdData = 0;
//	uint16 thresholdMeId = 0;
	uint16 length = 0;
	uint8 *attributeValuePtr = NULL;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);
#if 0 //because the function don't support , so the threshold doesn't support also.
	thresholdMeId = get16(attributeValuePtr);

	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 1, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 1, tempBuffer);
	if (ret != 0)
		return -1;
#endif
	return 0;
}

/*******************************************************************************************************************************
start
9.3.4 MAC bridge port configuration data

********************************************************************************************************************************/

/*init 9.3.4 MAC bridge port configuration data ME*/
int omciMeInitForMACBridgePortConData(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciMACBridgePortConDataDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;

	return 0;
}

int omciMACBridgePortConDataDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t *  omciPayLoad, uint8 msgSrc){
	int ret = -1;
	uint16 instId = 0;
	int result = 0;
	uint16 outTpPointer = 0;
	uint16 inTpPointer = 0;
	uint8 * attrValue_ptr = NULL;
	int portType = 0;
	uint8 portId = 0;
	uint8 ifcType = 0;

	if(meInstant_ptr == NULL || omciPayLoad == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciMACBridgePortConDataDeleteAction-> parameter is NULL, fail");
		goto end;
	}
	
	/*get instance id*/	
	instId = get16((uint8*)&(omciPayLoad->meId)+2);

	attrValue_ptr =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_ATTR_LEN);
	if(attrValue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciMACBridgePortConDataDeleteAction-> attrValue_ptr == NULL calloc error ");
		goto end;
	}		

	result = getMacBridgePortInstType(instId,&ifcType);
	/*delete gem port mapping rule by mac bridge port*/
	if(delGemPortMappingRuleByMbp(instId) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciMACBridgePortConDataDeleteAction-> delGemPortMappingRuleByMbp, fail");
	}
	if(result == OMCI_MAC_BRIDGE_PORT_WAN_IFC){
		
		/*delete ANI mac bridge port*/	
		if(setMacBridgePortDeviceId(instId, OMCI_DEL_MAC_BRIDGE_ANI_PORT) == OMCI_MAC_BRIDGE_PORT_FAIL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciMACBridgePortConDataDeleteAction->setMacBridgePortDeviceId, del, fail");
 		}
	}

	/*clean trafic descriptor*/
	/*get 9.3.4  mac bridge port config data ME outbound TD pointerattribute */
	if(getAttributeValueByInstId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA, instId, 11, attrValue_ptr, 2) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciMACBridgePortConDataDeleteAction-> getAttributeValueByInstId outbound fail ");
		goto end;
	}
	outTpPointer = get16(attrValue_ptr);
	
	/*get 9.3.4  mac bridge port config data ME inbound TD pointerattribute */
	if(getAttributeValueByInstId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA, instId, 12, attrValue_ptr, 2) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciMACBridgePortConDataDeleteAction-> getAttributeValueByInstId inbound fail ");
		goto end;
	}
	inTpPointer = get16(attrValue_ptr);

	/*get interface type*/
	portType = getMacBridgePortInstType(instId,&ifcType);
	if(portType == OMCI_MAC_BRIDGE_PORT_LAN_IFC){//UNI
		/*get lan port id*/
		if(getMacBridgeLanPortId(instId, &portId) == 0){
			cleanBoundTD(portId, portType, outTpPointer, OMCI_OUT_BOUND);
			cleanBoundTD(portId, portType, inTpPointer,OMCI_IN_BOUND);

			/*set user isolate function to default value in this lan port*/
		if((ifcType == OMCI_MAC_BRIDGE_PORT_PPTP_UNI) \
                && (pSystemCap->onuType==SFU)){
#ifdef TCSUPPORT_PON_USER_ISOLATION				
				if(userIsolateApi(OMCI_USER_ISOLATION_SET_GROUP, portId, OMCI_MBP_UNI_ISOLATE_DEFAULT_VAL) != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciMACBridgePortConDataDeleteAction-> userIsolateApi,new lan port id to default, fail");
				}
#endif
			}
		}
	}else if(portType == OMCI_MAC_BRIDGE_PORT_WAN_IFC){//don't support wan interface, because there isn't this configurated method in ZTE C300 OLT.
#if 0
		if(getMacBridgeWanPortId(instId, &portId) == 0){
			cleanBoundTD(portId, portType, outTpPointer, OMCI_OUT_BOUND);
			cleanBoundTD(portId, portType, inTpPointer,OMCI_IN_BOUND);
		}
#endif
	}else{
		//nothing
	}	
	
	ret = 0;
end:
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciMACBridgePortConDataDeleteAction-> ret=0x%02x",ret);
	if(attrValue_ptr != NULL)
		free(attrValue_ptr);
	
	if(ret == 0){
		return omciDeleteAction(meClassId , (omciMeInst_t *)meInstant_ptr, omciPayLoad, msgSrc);
	}

	return ret;
}
int setMBPCDTPTypeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = -1;
	uint8 tpType = 0;

	if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL){
		ret = -1;
		goto end;
	}

	tpType = *(uint8 *)value;
	switch(tpType){
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_1:
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_3:
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_4:
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_5:
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_6:
		case OMCI_MAC_BRIDGE_PORT_TP_TYPE_11:
			break;
		default:// others not support
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPCDTPTypeValue-> value is error, tpType=, fail",tpType);
			ret = -1;
			goto end;
	}
	
	ret = 0;
end:
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBPCDTPTypeValue-> ret= 0x%02x",ret);
	if(ret == 0){
		return updateMACBridgePortInfoByAssociaAttr(value, meInstantPtr, omciAttribute, flag, OMCI_MAC_BRIDGE_PORT_TP_TYPE_ATTR);
	}else{
		return -1;
	}
}

int setMBPCDTPPointerValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	
	return updateMACBridgePortInfoByAssociaAttr(value, meInstantPtr, omciAttribute, flag, OMCI_MAC_BRIDGE_PORT_TP_POINTE_ATTR);
}


int32 getMBPCDportMacAddressValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	uint8 portMacAddr[6] = {0,0,0,0,0,0};
	uint16 instanceId = 0;
	omciMeInst_t * meInst = (omciMeInst_t *)meInstantPtr;
	int type = 0;
	int ret = -1;
	uint8 lanPortId = 0;	
	uint8 ifcType = 0;

	instanceId = get16(meInst->attributeVlaue_ptr);

	/*get interface type*/
	type = getMacBridgePortInstType(instanceId, &ifcType);
	if(type == OMCI_MAC_BRIDGE_PORT_LAN_IFC){
		/*get lan port id*/
		ret = getMacBridgeLanPortId(instanceId, &lanPortId);
		if(ret != 0){
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMBPCDportMacAddressValue->getMacBridgeLanPortId, fail");
			goto end;
		}
		/*switch API*/
		ret = macMT7530GetPortMac(lanPortId, portMacAddr);
		if(ret != 0){
//			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMBPCDportMacAddressValue->macMT7530GetPortMac, fail[%d]",__LINE__);
		}
	}else if(type == OMCI_MAC_BRIDGE_PORT_WAN_IFC){
		/*wan interface Port mac address, CPE mac address*/
		getOmciEtherAddr(portMacAddr);
	}else{
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMBPCDportMacAddressValue->getMacBridgePortInstType, fail");
		goto end;
	}
	
	ret = 0;
end:
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMBPCDportMacAddressValue->ret =0x%02x",ret);
	if(ret == 0){
		return getTheValue(value, (char *)portMacAddr, 6, omciAttribute);
	}else{
		return -1;
	}
	
}

int setMBPCDTDPointerValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag, uint8 attrIndex){
	uint16 instanceId = 0;
	omciMeInst_t * meInst = (omciMeInst_t *)meInstantPtr;
	int type = 0;
	int ret = 0;
	uint8 portId = 0;
	uint16 oldTDInstId = 0;
	uint16 newTDInstId = 0;
	uint8 *attrValue_ptr = NULL;
	uint8 ifcType = 0;

	if((attrIndex != OMCI_MAC_BRIDGE_PORT_CFG_DATA_ATTR_11) 
		&& (attrIndex != OMCI_MAC_BRIDGE_PORT_CFG_DATA_ATTR_12)){

		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPCDoutTDPointerValue-> attrIndex error, fail ");
		ret = -1;
		goto end;
	}
	
	attrValue_ptr =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_ATTR_LEN);
	if(attrValue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPCDoutTDPointerValue-> val calloc error ");
		ret = -1;
		goto end;
	}

	instanceId = get16(meInst->attributeVlaue_ptr);

	/*get 9.3.4  mac bridge port config data ME outbound TD pointer or inbound TD pointer attribute */
	ret = getAttributeValueByInstId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA, instanceId, attrIndex, attrValue_ptr, 2);
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setRealMulticastRuleByMeType-> getAttributeValueByInstId  fail ");
		ret = -1;
		goto end;
	}
	
	oldTDInstId = get16(attrValue_ptr);
	newTDInstId = get16((uint8 *)value);
	
	/*get interface type*/
	type = getMacBridgePortInstType(instanceId, &ifcType);
	if(type == OMCI_MAC_BRIDGE_PORT_LAN_IFC){//UNI
		/*get lan port id*/
		ret = getMacBridgeLanPortId(instanceId, &portId);
		if(ret != 0){
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPCDoutTDPointerValue-> getMacBridgeLanPortId fail!");
			goto end;
		}

		/* clear old outbound trafic descriptor by calling API*/
		if(flag != OMCI_CREATE_ACTION_SET_VAL){
			if(attrIndex == OMCI_MAC_BRIDGE_PORT_CFG_DATA_ATTR_11){
				ret = cleanBoundTD(portId, type, oldTDInstId, OMCI_OUT_BOUND);
			}else if(attrIndex == OMCI_MAC_BRIDGE_PORT_CFG_DATA_ATTR_12){
				ret = cleanBoundTD(portId, type, oldTDInstId, OMCI_IN_BOUND);	
			}
			if(ret != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPCDoutTDPointerValue-> cleanOutBoundTD or cleanInBoundTD fail , attrIndex =%d", attrIndex);
			}
		}	
		
		/* set outbound trafic descriptor by calling API*/
		if(attrIndex == OMCI_MAC_BRIDGE_PORT_CFG_DATA_ATTR_11){
			ret = setBoundTD(portId, type, newTDInstId, OMCI_OUT_BOUND);
		}else if(attrIndex == OMCI_MAC_BRIDGE_PORT_CFG_DATA_ATTR_12){
			ret = setBoundTD(portId, type, newTDInstId, OMCI_IN_BOUND);
		}
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPCDoutTDPointerValue-> setOutBoundTD or setInBoundTD fail , attrIndex =%d", attrIndex);
		}
	}else if(type== OMCI_MAC_BRIDGE_PORT_WAN_IFC){//don't support wan interface, because there isn't this configurated method in ZTE C300 OLT.
#if 0		
		ret = getMacBridgeWanPortId(instanceId, &portId);
		if(ret != 0){
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPCDoutTDPointerValue-> getMacBridgeWanPortId fail!");
			goto end;
		}
#endif
	}else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPCDoutTDPointerValue-> getMacBridgePortInstType fail!");
		ret = -1;
		goto end;
	}

	ret = 0;
end:
	if(attrValue_ptr != NULL){
		free(attrValue_ptr);
	}
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBPCDoutTDPointerValue->ret =0x%02x",ret);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return -1;
	}

}

int setMBPCDoutTDPointerValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	return setMBPCDTDPointerValue(value, meInstantPtr, omciAttribute, flag, OMCI_MAC_BRIDGE_PORT_CFG_DATA_ATTR_11);
}

int setMBPCDinTDPointerValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	return setMBPCDTDPointerValue(value, meInstantPtr, omciAttribute, flag, OMCI_MAC_BRIDGE_PORT_CFG_DATA_ATTR_12);
}
int setMBPCDMacAddrLimitValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	//return setMBPCDTDPointerValue(value, meInstantPtr, omciAttribute, flag, OMCI_MAC_BRIDGE_PORT_CFG_DATA_ATTR_12);
	uint8 macAddrLimit = 0;
	uint8 ind = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = -1;
	int i = 0;
	uint16 currInstId = 0;
	uint8 ifaceType = 0; //0:UNI, 1:ANI
	uint8 portId = 0;
	#ifdef TCSUPPORT_BRIDGE_MAC_LIMIT
	char cmd[64];
	uint8 portId_temp = 0;
	#endif

	if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPCDMacAddrLimitValue-> parameter is null, fail");
		goto end;
	}

	macAddrLimit = *((uint8*)value);
	OMCI_TRACE("%s port cfg data flag %d\n",__FUNCTION__,flag);

	/*2. get  instance id*/
	currInstId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	/*3. get port and interface type */
	if(getPortFromMacBridgeByInstId(&portId, &ifaceType, currInstId) != 0){
		OMCI_ERROR("\r\n setMBPCDMacAddrLimitValue----->getPortFromMacBridgeByInstId, fail");
		ret = -1;
		goto end;
	}
	OMCI_TRACE( "\r\n setMBPCDMacAddrLimitValue----->uint8 port=0x%02x", portId);
    
	if((OMCI_MAC_BRIDGE_PORT_LAN_IFC == ifaceType) && (HGU != pSystemCap->onuType))
    {
    	OMCI_TRACE("%s  set port %d mac limit %d interface type %d\n",__FUNCTION__,portId,macAddrLimit,ifaceType);
		#ifdef TCSUPPORT_BRIDGE_MAC_LIMIT
		portId_temp = portId+3;/*confirm from DASAN:olt lan port no from 0~3,onu br0 lan port no from 3~6*/
		sprintf(cmd,"echo %d-%d > /proc/br_fdb/mac_limit_by_port",portId_temp,macAddrLimit);
		system(cmd);

		system("/userfs/bin/hw_nat -!");
		#else
		if (0 != macMT7530SetPortMacLimit(portId, macAddrLimit)){
			OMCI_ERROR("\r\n setMBPCDMacAddrLimitValue->macMT7530SetPortMacLimit failure");
		}
		#endif
	}
    
ret = 0;
	return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
end:	
	ret = -1;
	return ret;	
}


/*******************************************************************************************************************************
start
9.3.5 MAC bridge port designation data

********************************************************************************************************************************/

/*init  9.3.5 MAC bridge port designation data ME*/
int omciMeInitForMACBridgePortDesignData(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciInternalCreateActionForMACBridge;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;

	return 0;
}
int omciInternalCreateActionForMACBridge(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	uint16 classId = 0;
	uint16 instanceId = 0;
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
//	uint8 * attributeValuePtr = NULL;
//	uint16 length = 0;
	int ret = -1;

	if(msgSrc != INTERNAL_MSG){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciInternalCreateActionForMACBridge: fail, can be only created by ONU!");
		goto end;
	}

	/* get instance id for new instance*/
	classId = get16((uint8*)&(omciPayLoad->meId));
	instanceId = get16((uint8*)&(omciPayLoad->meId)+2);
	
	/*create instace*/
	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	if(tmpOmciManageEntity_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciInternalCreateActionForMACBridge omciGetMeByClassId->failure");
		goto end;
	}

	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	if(tmpomciMeInst_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciInternalCreateActionForMACBridge omciAllocInstance->failure");
		goto end;
	}
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	/*set instance id  to new instace*/
	put16(tmpomciMeInst_p->attributeVlaue_ptr, instanceId);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateActionForMACBridge: classId= 0x%02x, instanceId = 0x%02x\n",classId, instanceId);

	ret = 0;	
end: 
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciInternalCreateActionForMACBridge: ret = 0x%02x",ret);
	
	if(ret != 0){
		if(tmpomciMeInst_p != NULL){
			omciDelInstance(tmpOmciManageEntity_p, tmpomciMeInst_p);
		}
	}
	return ret;
}

int32 getMBPDDPortStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	uint8 portState = 0;/*0:disabled, 1:listening, 2:learning, 3:forwarding, 4:blocking, 5:linkdown, 6 stp_off*/
	uint16 instanceId = 0;
	omciMeInst_t * meInst = (omciMeInst_t *)meInstantPtr;
	uint8 val = 0;
	int type = 0;
	int ret = -1;
	uint8 lanPortId = 0;
	uint8 ifcType = 0;

	val = *(uint8 *)value;
	instanceId = get16(meInst->attributeVlaue_ptr);

	/*get interface type*/
	type = getMacBridgePortInstType(instanceId, &ifcType);
	
	if(type == OMCI_MAC_BRIDGE_PORT_LAN_IFC){
		ret = getMacBridgeLanPortId(instanceId, &lanPortId);
		if(ret != 0){
			ret = -1;
			goto end;
		}
		/* lan interface switch API*/
		ret = macMT7530GetPortStatus(lanPortId);
		if(ret == -1){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMBPDDPortStateValue->macMT7530GetPortStatus, fail[%d]",__LINE__);
			portState = OMCI_MAC_BRIDGE_PORT_STATE_6;//stp_off
		}else{
			portState = (uint8)ret;
		}
		
	}else if(type== OMCI_MAC_BRIDGE_PORT_WAN_IFC){
		/* wan interface port status*/
		portState = OMCI_MAC_BRIDGE_PORT_STATE_6;//stp_off
	}else{
		ret = -1;
		goto end;
	}
	
	ret = 0;
end:
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMBPDDPortStateValue-> ret !=0, fail!");
		portState = OMCI_MAC_BRIDGE_PORT_STATE_6;
	}
	return getTheValue(value, (char *)&portState, 1, omciAttribute);
}
/*******************************************************************************************************************************
start
9.3.6 mac bridge port filter table table

********************************************************************************************************************************/

int omciMeInitForMACBridgePortFilterData(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciInternalCreateActionForMACBridge;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciMACBridgePortFilterDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	return 0;
}



int omciMACBridgePortFilterDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc){
	int ret = -1;
	uint16 instId = 0;
	uint8 macFilterRulePortId = 0;
//	int ifaceType = 0;
//	uint8 portId = 0;

	if(omciPayLoad == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciMACBridgePortFilterDeleteAction-> parameter is NULL, fail");
		ret = -1;
		goto end;
	}
	
	/*get instance id*/	
	instId = get16((uint8*)&(omciPayLoad->meId)+2);

	/*1.get mac filter rule port id*/
	ret = getMacFilterRulePortIdByInst(instId, &macFilterRulePortId);
	if(ret != 0){
		ret = 0;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciMACBridgePortFilterDeleteAction-> getMacFilterRulePortIdByInst, not find mac filter rule port id");
		goto end;
	}
	
	/*2. clean all mac filter rule for this port*/	
	ret = pon_clean_mac_filter_rule(macFilterRulePortId, OMCI_MAC_FILTER_TYPE);	
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciMACBridgePortFilterDeleteAction-> pon_clean_mac_filter_rule, fail");
	}

#if 0
	/*3. add 9.3.7 mac filter rule to this port*/	
	ret = addMacFilterRuleByInst(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_PREASSIGN_DATA, instId, macFilterRulePortId);
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciMACBridgePortFilterDeleteAction-> addMacFilterRuleByMacFilterPortId, fail");
	}
#endif
	ret = 0;
end:
	if(ret == 0){
		return omciDeleteAction(meClassId , (omciMeInst_t *)meInstant_ptr, omciPayLoad, msgSrc);
	}else{
		return -1;
	}
}


int32 getMBPFTmacFilterTableValue(char *value, uint8 *meInstantPtr,  omciAttriDescript_ptr omciAttribute, uint8 flag){
	omciTableAttriValue_t *tableConent_ptr = NULL;
	omciManageEntity_t *me_p=NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;	
	char *tableBuffer = NULL;
	uint32 tableSize = 0;
	uint16 instanceId = 0;
	int ret = -1;

	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_TABLE_DATA);
	if(me_p ==  NULL){
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMBPFTmacFilterTableValue->omciGetMeByClassId, fail");
		goto end;
	}

	tableConent_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, me_p->omciAttriDescriptList[1].attriIndex);	
	if(tableConent_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMBPFTmacFilterTableValue->tableConent_ptr is NULL, fail");
		ret = -1;
		goto end;
	}
	
	/*1.calculate the table size*/
	tableSize = tableConent_ptr->tableSize;
	if(tableSize == 0){
		ret = 0;
		goto end;
	}
	
	/*2.get table content*/
	tableBuffer = (char *)tableConent_ptr->tableValue_p;
	if(tableBuffer == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMBPFTmacFilterTableValue->tableBuffer is NULL, fail");
		ret = -1;
		goto end;
	}

	/*3. add the table to omci archtecture*/
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	ret = omciAddTableContent(tmpomciMeInst_ptr->classId, instanceId, (uint8*)tableBuffer, tableSize ,omciAttribute->attriName);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMBPFTmacFilterTableValue->omciAddTableContent fail, fail");
		ret = -1;
		goto end;
	}	
	
end:
	if(ret != 0){
		tableSize = 0;
	}
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMBPFTmacFilterTableValue->tableSize = %d",tableSize);
	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
	
}

int setMBPFTmacFilterTableValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	omciTableAttriValue_t *tableConent_ptr = NULL; //table content pointer
	uint8 *tmpValue_ptr = NULL;
	uint8 *tmp_ptr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	uint8 index = 0;//record index in table
	/*  	filterType: 
  		Bit 	Name			Setting
  		1	filter/forward		0:forward,1:filter
  		2					0:dest mac, 1:src mac
  		3~6	reserved
  		7~8	add/remove		00:remove this entry, 01:addthis entry, 10:clear entire table
  	*/	
	uint8 filterType = 0;
	uint8 tempFilterType = 0;
	uint8 mac[6] = {0};
//	uint8 macMask[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	uint8 action = 0;//0:remove this entry, 2:add this entry, 1: clear table
	uint16 instanceId = 0;//instance id
	uint32 tableSize = 0;
	uint32 length = 0;
	uint8 findFlag = OMCI_NO_FIND_RULE;//0:not find, 1:find
	int ret = -1;
	omciManageEntity_t *me_p=NULL;
	pon_mac_filter_rule macFilterRule;
	pon_mac_filter_rule_p macFilterRule_ptr = NULL;
	uint8 tempU8 = 0;
	uint8 macFilterRulePortId= 0;
	int delIndex = 0;
	uint8 tableFilterType = OMCI_MAC_BRIDGE_PORT_MAC_FILTER_TBL_TYPE_DEFAULT;

	
	if((tmpomciMeInst_ptr == NULL) || (value == NULL) || (omciAttribute == NULL)){
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFTmacFilterTableValue->parameter is error, fail");
		goto end;
	}
	
	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_TABLE_DATA);
	if(me_p ==  NULL){
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFTmacFilterTableValue->omciGetMeByClassId, fail");
		goto end;
	}

	macFilterRule_ptr = &macFilterRule;
	memset(macFilterRule_ptr, 0, sizeof(pon_mac_filter_rule));
	
	/*get instance id*/
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);

	/*get mac filter rule port id*/
	ret = getMacFilterRulePortIdByInst(instanceId, &macFilterRulePortId);
	if(ret != 0){
		ret = 0;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFTmacFilterTableValue-> getMacFilterRulePortIdByInst, not find mac filter rule port id");
		goto end;
	}	
	
	/*parse value*/
	index = *(uint8*)value;
	filterType = *(uint8 *)(value+1);
	memcpy(mac, (uint8 *)(value+2), 6);
	
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setMBPFTmacFilterTableValue_>index=0x%2x,filterType=0x%02x,mac[0]=0x%02x,mac[1]=0x%02x,mac[2]=0x%02x,mac[3]=0x%02x,mac[4]=0x%02x,mac[5]=0x%02x",
//		index, filterType, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	/*get action(add/del/clear)*/
	action = (filterType >> 6);
	if((action != OMCI_MAC_BRIDGE_PORT_MAC_FILTER_DEL_ENTRY)
		&& (action != OMCI_MAC_BRIDGE_PORT_MAC_FILTER_ADD_ENTRY)
		&& (action != OMCI_MAC_BRIDGE_PORT_MAC_FILTER_CLEAR_ENTRY)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFTmacFilterTableValue->action = %d, fail", action);
		ret = -1;
		goto end;
	}
	if((action == OMCI_MAC_BRIDGE_PORT_MAC_FILTER_DEL_ENTRY) 
		|| (action ==OMCI_MAC_BRIDGE_PORT_MAC_FILTER_ADD_ENTRY)){// del or add
		
		/*get current table content*/
		tableConent_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, me_p->omciAttriDescriptList[1].attriIndex);
		if(tableConent_ptr == NULL){
			if(action == OMCI_MAC_BRIDGE_PORT_MAC_FILTER_DEL_ENTRY){//del
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFTmacFilterTableValue->del table is NULL, fail");
				ret = -1;
				goto end;
			}
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBPFTmacFilterTableValue->tableConent_ptr is NULL and add first record");
			findFlag = OMCI_NO_FIND_RULE;
			tableSize = 0;
			tmp_ptr = NULL;
		}else{
			tableSize = tableConent_ptr->tableSize;
			tmp_ptr = tableConent_ptr->tableValue_p;
		}
		/*1. malloc new space*/
		tmpValue_ptr = calloc(1 , tableSize+MAC_FILTER_RECORD_LENGTH);
		if(tmpValue_ptr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBPFTmacFilterTableValue->tmpValue_ptr calloc failure");
			ret = -1;
			goto end;		
		}
						
		/*2.search record*/
		length = MAC_FILTER_RECORD_LENGTH;
		findFlag = OMCI_NO_FIND_RULE;
		while(tmp_ptr != NULL){
			if(index == *tmp_ptr){
				findFlag = OMCI_FIND_RULE;//find
				
				/*get mac filter rule information*/
				tempU8 = *(tmp_ptr+1); //filter type				
				memset(macFilterRule_ptr, 0, sizeof(macFilterRule));
				macFilterRule_ptr->filter_type = (tempU8&0x01);
				if((tempU8&0x02) == 0x02){
					macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_SRC;
				}else{
					macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
				}
				macFilterRule_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
				memcpy(macFilterRule_ptr->start_mac, (tmp_ptr+2), 6);
				memcpy(macFilterRule_ptr->end_mac, (tmp_ptr+2), 6);
				
				break;
			}
			length += MAC_FILTER_RECORD_LENGTH;
			if(length > tableConent_ptr->tableSize){//no find
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBPFTmacFilterTableValue->length > tableConent_ptr->tableSize");
				break;
			}
			tmp_ptr += MAC_FILTER_RECORD_LENGTH;
		}
		/*get real length*/
		length -= MAC_FILTER_RECORD_LENGTH;
		
		/*3. get valid content in table, del find mac filter rule*/
		if(findFlag == OMCI_FIND_RULE){
			/*real delete a rule by MAC filter API*/
			delIndex = findMacFilterRuleByRuleInfo(macFilterRule_ptr, macFilterRulePortId, OMCI_MAC_FILTER_TYPE);
			if(delIndex != -1){//find
				ret = pon_del_mac_filter_rule(macFilterRulePortId,OMCI_MAC_FILTER_TYPE, delIndex);
				if(ret != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFTmacFilterTableValue->pon_del_mac_filter_rule ,failure");
				}
			}
			
			/*if find, then delete this record in  table(instance)*/			
			memcpy(tmpValue_ptr,tableConent_ptr->tableValue_p, length);
			memcpy(tmpValue_ptr+length, tableConent_ptr->tableValue_p+length+MAC_FILTER_RECORD_LENGTH, tableSize-length-MAC_FILTER_RECORD_LENGTH);
			tableSize -= MAC_FILTER_RECORD_LENGTH; //update current table size			
		}else{
			if(action == OMCI_MAC_BRIDGE_PORT_MAC_FILTER_DEL_ENTRY){//when delete entry, if this entry is not find, then return -1
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFTmacFilterTableValue->delete record and no find ,so failure");
				ret = -1;
				goto end;
			}
			
			if(tableSize > 0){
				memcpy(tmpValue_ptr,tableConent_ptr->tableValue_p,tableSize);
			}
		}

		/*4.execute real add action*/		
		if(action == OMCI_MAC_BRIDGE_PORT_MAC_FILTER_ADD_ENTRY){//add new entry to this table 
			/*support max table size is 17 entry*/
			if(tableSize >= (MAX_OMCI_MAC_FILTER_9_3_6_RULE_ONE_PORT *MAC_FILTER_RECORD_LENGTH)){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFTmacFilterTableValue->tablesize = %d, overflow, fail", tableSize);
				ret = -1;
				goto end;
			}
			/*real add a rule by MAC filter API*/
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBPFTmacFilterTableValue->add new entry!");

			/*get mac filter rule information*/			
			memset(macFilterRule_ptr, 0, sizeof(macFilterRule));
			macFilterRule_ptr->filter_type = (filterType&0x01);
			if((filterType&0x02) == 0x02){
				macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_SRC;
			}else{
				macFilterRule_ptr->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;
			}
			macFilterRule_ptr->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			memcpy(macFilterRule_ptr->start_mac, mac, 6);
			memcpy(macFilterRule_ptr->end_mac, mac, 6);

			/*get current table filter type*/
			if(tableSize == 0){
				tableFilterType = OMCI_MAC_BRIDGE_PORT_MAC_FILTER_TBL_TYPE_DEFAULT;
			}else{
				tempFilterType = *(tmpValue_ptr+1);
				tableFilterType = (tempFilterType&0x01);
			}

			if((tableFilterType != OMCI_MAC_BRIDGE_PORT_MAC_FILTER_TBL_TYPE_DEFAULT)
				&& (macFilterRule_ptr->filter_type != tableFilterType)){//It is not allow when forward and filter actions are mixed.
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFTmacFilterTableValue->It is not allow when forward and filter actions are mixed, fail");
				ret = -1;
				goto end;	
			}
			/*add mac filter rule*/			
			ret = pon_add_mac_filter_rule(macFilterRulePortId,OMCI_MAC_FILTER_TYPE, macFilterRule_ptr);
			if(ret != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFTmacFilterTableValue->pon_add_mac_filter_rule, fail");
				ret = -1;
				goto end;	
			}
			
			/*add entry to instance*/
			memcpy(tmpValue_ptr+tableSize, value, MAC_FILTER_RECORD_LENGTH);		
			tableSize += MAC_FILTER_RECORD_LENGTH;
		}else{
			//nothing
		}
		
		/*5. update table content in instance table list*/
		if(tableConent_ptr != NULL){//del table content  in instance table list
			ret = omciDelTableValueOfInst(tmpomciMeInst_ptr, me_p->omciAttriDescriptList[1].attriIndex);
			if(ret != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBPFTmacFilterTableValue->omciDelTableValueOfInst failure");
				ret = -1;
				goto end;
			}
		}
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBPFTmacFilterTableValue->tableSize = %d",tableSize);
		
		if(tableSize == 0){//current table is empty
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBPFTmacFilterTableValue->current table is empty");
			ret = 0;
			goto end;
		}
		tableConent_ptr = omciAddTableValueToInst(tmpomciMeInst_ptr, me_p->omciAttriDescriptList[1].attriIndex, tableSize, tmpValue_ptr);
		if(tableConent_ptr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBPFTmacFilterTableValue->omciAddTableValueToInst failure");
			ret = -1;
			goto end;	
		}
	}else if(action == OMCI_MAC_BRIDGE_PORT_MAC_FILTER_CLEAR_ENTRY){//clear this table
		/*1. get table content*/
		tableConent_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, me_p->omciAttriDescriptList[1].attriIndex);
		if(tableConent_ptr == NULL){
			ret = 0;
			goto end;	
		}
		tmp_ptr = tableConent_ptr->tableValue_p;
		
		/*2. real clean record*/
			/*clean all mac filter rule for this port*/	
		ret = pon_clean_mac_filter_rule(macFilterRulePortId, OMCI_MAC_FILTER_TYPE);	
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBPFTmacFilterTableValue-> pon_clean_mac_filter_rule, fail");
			ret = -1;
			goto end;
		}
#if 0
		/*add 9.3.7 mac filter rule to this port*/	
		ret = addMacFilterRuleByInst(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_PREASSIGN_DATA, instanceId, macFilterRulePortId);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciMACBridgePortFilterDeleteAction-> addMacFilterRuleByMacFilterPortId, fail");
		}
#endif
		/*3.clean table in table list*/
		ret = omciDelTableValueOfInst(tmpomciMeInst_ptr, me_p->omciAttriDescriptList[1].attriIndex);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBPFTmacFilterTableValue->omciDelTableValueOfInst failure");
			ret = -1;
			goto end;
		}
		
	}
	ret = 0;
end:
	if(tmpValue_ptr != NULL){
		free(tmpValue_ptr);
	}
	return ret;
}

/*******************************************************************************************************************************
start
9.3.7 mac bridge port filter preassign table

********************************************************************************************************************************/
int omciMeInitForMACBridgePortFilterPreData(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciInternalCreateActionForMACBridge;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciMACBridgePortFilterPreDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}
int omciMACBridgePortFilterPreDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc){
	int ret = -1;
	uint16 instId = 0;
	uint8 macFilterRulePortId = 0;
//	int ifaceType = 0;
//	uint8 portId = 0;

	if(omciPayLoad == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciMACBridgePortFilterPreDeleteAction-> parameter is NULL, fail");
		ret = -1;
		goto end;
	}
	
	/*get instance id*/	
	instId = get16((uint8*)&(omciPayLoad->meId)+2);

	/*1.get mac filter rule port id*/
	ret = getMacFilterRulePortIdByInst(instId, &macFilterRulePortId);
	if(ret != 0){
		ret = 0;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciMACBridgePortFilterPreDeleteAction-> getMacFilterRulePortIdByInst, not find mac filter rule port id");
		goto end;
	}
	
	/*2. clean all mac filter rule for this port*/	
	ret = pon_clean_mac_filter_rule(macFilterRulePortId,OMCI_PKT_FILTER_TYPE);	
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciMACBridgePortFilterPreDeleteAction-> pon_clean_mac_filter_rule, fail");
	}
#if 0
	/*3. add 9.3.6 mac filter rule to this port*/	
	ret = addMacFilterRuleByInst(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_FILTER_TABLE_DATA, instId, macFilterRulePortId);
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciMACBridgePortFilterPreDeleteAction-> addMacFilterRuleByMacFilterPortId, fail");
	}
#endif
	ret = 0;
end:
	if(ret == 0){
		return omciDeleteAction(meClassId , (omciMeInst_t *)meInstant_ptr, omciPayLoad, msgSrc);
	}else{
		return -1;
	}
}

int setMBPFPTFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag, uint8 type){
	uint8 val= 0;
	uint16 instanceId = 0;
	int ret = -1;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	pon_mac_filter_rule macFilterRule[MAX_OMCI_MAC_BRIDGE_FILTER_SIMULTANEOUS_RULE_NUM];
	pon_mac_filter_rule_p macFilterRule_ptr[MAX_OMCI_MAC_BRIDGE_FILTER_SIMULTANEOUS_RULE_NUM] = {NULL, NULL, NULL, NULL};
	uint8 validRuleNum = 0;
//	uint8 filterType = 0;
	uint8 macFilterRulePortId = 0;
	int delIndex = -1;
	int i = 0;
	uint8 entryNum = 0;

	if(value == NULL || meInstantPtr ==NULL || omciAttribute == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFPTFilterValue-> parameter is NULL,type = %d, fail", type);
		return -1;
	}

	/*init*/
	entryNum = sizeof(pon_mac_filter_rule);
	for(i = 0; i<MAX_OMCI_MAC_BRIDGE_FILTER_SIMULTANEOUS_RULE_NUM; i++){
		macFilterRule_ptr[i] = &macFilterRule[i];
		memset(macFilterRule_ptr[i], 0, entryNum);
		macFilterRule_ptr[i]->filter_type = OMCI_MAC_FILTER_RULE_TYPE_DISCARD;
		macFilterRule_ptr[i]->mac_type = OMCI_MAC_FILTER_RULE_MAC_TYPE_DST;		
	}
	
	val = *(uint8 *)value;
	if((val != 0) && (val != 1)){
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMBPFPTFilterValue-> val = %d,type = %d,  fail",val, type);
		goto end;
	}
	
	/*get instance id*/
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	
	/*1.get mac filter rule port id*/
	ret = getMacFilterRulePortIdByInst(instanceId, &macFilterRulePortId);
	if(ret != 0){
		ret = 0;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFPTFilterValue-> getMacFilterRulePortIdByInst, not find mac filter rule port id, type = %d",type);
		goto end;
	}
	/*2.get mac filter rule information*/
	switch(type){
		case MAC_FILTER_IPV4_MULTICAST:	
			macFilterRule_ptr[0]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule_ptr[0]->start_mac[0] = 0x01;
			macFilterRule_ptr[0]->start_mac[1] = 0x00;
			macFilterRule_ptr[0]->start_mac[2] = 0x5e;
			macFilterRule_ptr[0]->start_mac[3] = 0x00;
			macFilterRule_ptr[0]->start_mac[4] = 0x00;
			macFilterRule_ptr[0]->start_mac[5] = 0x00;

			macFilterRule_ptr[0]->end_mac[0] = 0x01;
			macFilterRule_ptr[0]->end_mac[1] = 0x00;
			macFilterRule_ptr[0]->end_mac[2] = 0x5e;
			macFilterRule_ptr[0]->end_mac[3] = 0x7f;
			macFilterRule_ptr[0]->end_mac[4] = 0xff;
			macFilterRule_ptr[0]->end_mac[5] = 0xff;
			validRuleNum = 1;
			break;
		case MAC_FILTER_IPV6_MULTICAST:
			macFilterRule_ptr[0]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule_ptr[0]->start_mac[0] = 0x33;
			macFilterRule_ptr[0]->start_mac[1] = 0x33;
			macFilterRule_ptr[0]->start_mac[2] = 0x00;
			macFilterRule_ptr[0]->start_mac[3] = 0x00;
			macFilterRule_ptr[0]->start_mac[4] = 0x00;
			macFilterRule_ptr[0]->start_mac[5] = 0x00;

			macFilterRule_ptr[0]->end_mac[0] = 0x33;
			macFilterRule_ptr[0]->end_mac[1] = 0x33;
			macFilterRule_ptr[0]->end_mac[2] = 0xff;
			macFilterRule_ptr[0]->end_mac[3] = 0xff;
			macFilterRule_ptr[0]->end_mac[4] = 0xff;
			macFilterRule_ptr[0]->end_mac[5] = 0xff;
			validRuleNum = 1;
			break;
		case MAC_FILTER_IPV4_BROADCAST:
			macFilterRule_ptr[0]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0800;
			macFilterRule_ptr[0]->start_mac[0] = 0xff;
			macFilterRule_ptr[0]->start_mac[1] = 0xff;
			macFilterRule_ptr[0]->start_mac[2] = 0xff;
			macFilterRule_ptr[0]->start_mac[3] = 0xff;
			macFilterRule_ptr[0]->start_mac[4] = 0xff;
			macFilterRule_ptr[0]->start_mac[5] = 0xff;

			macFilterRule_ptr[0]->end_mac[0] = 0xff;
			macFilterRule_ptr[0]->end_mac[1] = 0xff;
			macFilterRule_ptr[0]->end_mac[2] = 0xff;
			macFilterRule_ptr[0]->end_mac[3] = 0xff;
			macFilterRule_ptr[0]->end_mac[4] = 0xff;
			macFilterRule_ptr[0]->end_mac[5] = 0xff;
			validRuleNum = 1;
			break;
		case MAC_FILTER_RARP:
			macFilterRule_ptr[0]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_8035;
			macFilterRule_ptr[0]->start_mac[0] = 0xff;
			macFilterRule_ptr[0]->start_mac[1] = 0xff;
			macFilterRule_ptr[0]->start_mac[2] = 0xff;
			macFilterRule_ptr[0]->start_mac[3] = 0xff;
			macFilterRule_ptr[0]->start_mac[4] = 0xff;
			macFilterRule_ptr[0]->start_mac[5] = 0xff;

			macFilterRule_ptr[0]->end_mac[0] = 0xff;
			macFilterRule_ptr[0]->end_mac[1] = 0xff;
			macFilterRule_ptr[0]->end_mac[2] = 0xff;
			macFilterRule_ptr[0]->end_mac[3] = 0xff;
			macFilterRule_ptr[0]->end_mac[4] = 0xff;
			macFilterRule_ptr[0]->end_mac[5] = 0xff;
			validRuleNum = 1;
			break;
		case MAC_FILTER_IPX:
			/*first*/
			macFilterRule_ptr[0]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_8137;
			macFilterRule_ptr[0]->start_mac[0] = 0xff;
			macFilterRule_ptr[0]->start_mac[1] = 0xff;
			macFilterRule_ptr[0]->start_mac[2] = 0xff;
			macFilterRule_ptr[0]->start_mac[3] = 0xff;
			macFilterRule_ptr[0]->start_mac[4] = 0xff;
			macFilterRule_ptr[0]->start_mac[5] = 0xff;

			macFilterRule_ptr[0]->end_mac[0] = 0xff;
			macFilterRule_ptr[0]->end_mac[1] = 0xff;
			macFilterRule_ptr[0]->end_mac[2] = 0xff;
			macFilterRule_ptr[0]->end_mac[3] = 0xff;
			macFilterRule_ptr[0]->end_mac[4] = 0xff;
			macFilterRule_ptr[0]->end_mac[5] = 0xff;
			/*second*/
			macFilterRule_ptr[1]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule_ptr[1]->start_mac[0] = 0x09;
			macFilterRule_ptr[1]->start_mac[1] = 0x00;
			macFilterRule_ptr[1]->start_mac[2] = 0x1b;
			macFilterRule_ptr[1]->start_mac[3] = 0xff;
			macFilterRule_ptr[1]->start_mac[4] = 0xff;
			macFilterRule_ptr[1]->start_mac[5] = 0xff;

			macFilterRule_ptr[1]->end_mac[0] = 0x09;
			macFilterRule_ptr[1]->end_mac[1] = 0x00;
			macFilterRule_ptr[1]->end_mac[2] = 0x1b;
			macFilterRule_ptr[1]->end_mac[3] = 0xff;
			macFilterRule_ptr[1]->end_mac[4] = 0xff;
			macFilterRule_ptr[1]->end_mac[5] = 0xff;			
			/*third*/
			macFilterRule_ptr[2]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule_ptr[2]->start_mac[0] = 0x09;
			macFilterRule_ptr[2]->start_mac[1] = 0x00;
			macFilterRule_ptr[2]->start_mac[2] = 0x4e;
			macFilterRule_ptr[2]->start_mac[3] = 0x00;
			macFilterRule_ptr[2]->start_mac[4] = 0x00;
			macFilterRule_ptr[2]->start_mac[5] = 0x02;

			macFilterRule_ptr[2]->end_mac[0] = 0x09;
			macFilterRule_ptr[2]->end_mac[1] = 0x00;
			macFilterRule_ptr[2]->end_mac[2] = 0x4e;
			macFilterRule_ptr[2]->end_mac[3] = 0x00;
			macFilterRule_ptr[2]->end_mac[4] = 0x00;
			macFilterRule_ptr[2]->end_mac[5] = 0x02;
			validRuleNum = 3;
			break;
		case MAC_FILTER_NETBEUL:
			macFilterRule_ptr[0]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule_ptr[0]->start_mac[0] = 0x03;
			macFilterRule_ptr[0]->start_mac[1] = 0x00;
			macFilterRule_ptr[0]->start_mac[2] = 0x00;
			macFilterRule_ptr[0]->start_mac[3] = 0x00;
			macFilterRule_ptr[0]->start_mac[4] = 0x00;
			macFilterRule_ptr[0]->start_mac[5] = 0x01;

			macFilterRule_ptr[0]->end_mac[0] = 0x03;
			macFilterRule_ptr[0]->end_mac[1] = 0x00;
			macFilterRule_ptr[0]->end_mac[2] = 0x00;
			macFilterRule_ptr[0]->end_mac[3] = 0x00;
			macFilterRule_ptr[0]->end_mac[4] = 0x00;
			macFilterRule_ptr[0]->end_mac[5] = 0x01;
			validRuleNum = 1;
			break;
		case MAC_FILTER_AARP:
			/*first*/
			macFilterRule_ptr[0]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_809b;
			macFilterRule_ptr[0]->start_mac[0] = 0xff;
			macFilterRule_ptr[0]->start_mac[1] = 0xff;
			macFilterRule_ptr[0]->start_mac[2] = 0xff;
			macFilterRule_ptr[0]->start_mac[3] = 0xff;
			macFilterRule_ptr[0]->start_mac[4] = 0xff;
			macFilterRule_ptr[0]->start_mac[5] = 0xff;

			macFilterRule_ptr[0]->end_mac[0] = 0xff;
			macFilterRule_ptr[0]->end_mac[1] = 0xff;
			macFilterRule_ptr[0]->end_mac[2] = 0xff;
			macFilterRule_ptr[0]->end_mac[3] = 0xff;
			macFilterRule_ptr[0]->end_mac[4] = 0xff;
			macFilterRule_ptr[0]->end_mac[5] = 0xff;
			/*second*/
			macFilterRule_ptr[1]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_80f3;
			macFilterRule_ptr[1]->start_mac[0] = 0xff;
			macFilterRule_ptr[1]->start_mac[1] = 0xff;
			macFilterRule_ptr[1]->start_mac[2] = 0xff;
			macFilterRule_ptr[1]->start_mac[3] = 0xff;
			macFilterRule_ptr[1]->start_mac[4] = 0xff;
			macFilterRule_ptr[1]->start_mac[5] = 0xff;

			macFilterRule_ptr[1]->end_mac[0] = 0xff;
			macFilterRule_ptr[1]->end_mac[1] = 0xff;
			macFilterRule_ptr[1]->end_mac[2] = 0xff;
			macFilterRule_ptr[1]->end_mac[3] = 0xff;
			macFilterRule_ptr[1]->end_mac[4] = 0xff;
			macFilterRule_ptr[1]->end_mac[5] = 0xff;			
			/*third*/
			macFilterRule_ptr[2]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule_ptr[2]->start_mac[0] = 0x09;
			macFilterRule_ptr[2]->start_mac[1] = 0x00;
			macFilterRule_ptr[2]->start_mac[2] = 0x07;
			macFilterRule_ptr[2]->start_mac[3] = 0x00;
			macFilterRule_ptr[2]->start_mac[4] = 0x00;
			macFilterRule_ptr[2]->start_mac[5] = 0x00;

			macFilterRule_ptr[2]->end_mac[0] = 0x09;
			macFilterRule_ptr[2]->end_mac[1] = 0x00;
			macFilterRule_ptr[2]->end_mac[2] = 0x07;
			macFilterRule_ptr[2]->end_mac[3] = 0x00;
			macFilterRule_ptr[2]->end_mac[4] = 0x00;
			macFilterRule_ptr[2]->end_mac[5] = 0xfc;
			/*fourth*/
			macFilterRule_ptr[3]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule_ptr[3]->start_mac[0] = 0x09;
			macFilterRule_ptr[3]->start_mac[1] = 0x00;
			macFilterRule_ptr[3]->start_mac[2] = 0x07;
			macFilterRule_ptr[3]->start_mac[3] = 0xff;
			macFilterRule_ptr[3]->start_mac[4] = 0xff;
			macFilterRule_ptr[3]->start_mac[5] = 0xff;

			macFilterRule_ptr[3]->end_mac[0] = 0x09;
			macFilterRule_ptr[3]->end_mac[1] = 0x00;
			macFilterRule_ptr[3]->end_mac[2] = 0x07;
			macFilterRule_ptr[3]->end_mac[3] = 0xff;
			macFilterRule_ptr[3]->end_mac[4] = 0xff;
			macFilterRule_ptr[3]->end_mac[5] = 0xff;
			validRuleNum = 4;
			break;
		case MAC_FILTER_MANAGE_INFO:
			macFilterRule_ptr[0]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0;
			macFilterRule_ptr[0]->start_mac[0] = 0x01;
			macFilterRule_ptr[0]->start_mac[1] = 0x80;
			macFilterRule_ptr[0]->start_mac[2] = 0xc2;
			macFilterRule_ptr[0]->start_mac[3] = 0x00;
			macFilterRule_ptr[0]->start_mac[4] = 0x00;
			macFilterRule_ptr[0]->start_mac[5] = 0x00;

			macFilterRule_ptr[0]->end_mac[0] = 0x01;
			macFilterRule_ptr[0]->end_mac[1] = 0x80;
			macFilterRule_ptr[0]->end_mac[2] = 0xc2;
			macFilterRule_ptr[0]->end_mac[3] = 0x00;
			macFilterRule_ptr[0]->end_mac[4] = 0x00;
			macFilterRule_ptr[0]->end_mac[5] = 0xff;
			validRuleNum = 1;
			break;
		case MAC_FILTER_ARP:
			macFilterRule_ptr[0]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_0806;
			macFilterRule_ptr[0]->start_mac[0] = 0xff;
			macFilterRule_ptr[0]->start_mac[1] = 0xff;
			macFilterRule_ptr[0]->start_mac[2] = 0xff;
			macFilterRule_ptr[0]->start_mac[3] = 0xff;
			macFilterRule_ptr[0]->start_mac[4] = 0xff;
			macFilterRule_ptr[0]->start_mac[5] = 0xff;

			macFilterRule_ptr[0]->end_mac[0] = 0xff;
			macFilterRule_ptr[0]->end_mac[1] = 0xff;
			macFilterRule_ptr[0]->end_mac[2] = 0xff;
			macFilterRule_ptr[0]->end_mac[3] = 0xff;
			macFilterRule_ptr[0]->end_mac[4] = 0xff;
			macFilterRule_ptr[0]->end_mac[5] = 0xff;
			validRuleNum = 1;
			break;
		case MAC_FILTER_PPPOE_BROADCAST:
			macFilterRule_ptr[0]->ethertype = OMCI_MAC_FILTER_RULE_ETHERTYPE_8863;
			macFilterRule_ptr[0]->start_mac[0] = 0xff;
			macFilterRule_ptr[0]->start_mac[1] = 0xff;
			macFilterRule_ptr[0]->start_mac[2] = 0xff;
			macFilterRule_ptr[0]->start_mac[3] = 0xff;
			macFilterRule_ptr[0]->start_mac[4] = 0xff;
			macFilterRule_ptr[0]->start_mac[5] = 0xff;

			macFilterRule_ptr[0]->end_mac[0] = 0xff;
			macFilterRule_ptr[0]->end_mac[1] = 0xff;
			macFilterRule_ptr[0]->end_mac[2] = 0xff;
			macFilterRule_ptr[0]->end_mac[3] = 0xff;
			macFilterRule_ptr[0]->end_mac[4] = 0xff;
			macFilterRule_ptr[0]->end_mac[5] = 0xff;
			validRuleNum = 1;
			break;
		case MAC_FILTER_TABLE_ENTRY://don't support 9.3.6
		default:
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFPTFilterValue-> type=%d, fail", type);
			goto end;
	}
	
	/*3. set mac filter rule*/
	for(i = 0; i<validRuleNum; i++){
		if(val == 0){ //del mac filter rule
			delIndex = findMacFilterRuleByRuleInfo(macFilterRule_ptr[i], macFilterRulePortId, OMCI_PKT_FILTER_TYPE);
			if(delIndex != -1){
				ret = pon_del_mac_filter_rule(macFilterRulePortId,OMCI_PKT_FILTER_TYPE, delIndex);
				if(ret != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFPTFilterValue-> pon_del_mac_filter_rule,type=%d, i=%d, fail", type, i);
				}
			}
			
		}else if(val == 1){ //add mac filter rule
			ret = pon_add_mac_filter_rule(macFilterRulePortId,OMCI_PKT_FILTER_TYPE, macFilterRule_ptr[i]);
			if(ret != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPFPTFilterValue-> pon_add_mac_filter_rule,type=%d, i=%d, fail", type, i);
			}
		}else{
			//nothing
		}
	}
	
	ret = 0;
end:
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return -1;
	}
}

int setMBPFPTipv4MulticastFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	return setMBPFPTFilterValue(value, meInstantPtr,omciAttribute,flag,MAC_FILTER_IPV4_MULTICAST);
}

int setMBPFPTipv6MulticastFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	return setMBPFPTFilterValue(value, meInstantPtr,omciAttribute,flag,MAC_FILTER_IPV6_MULTICAST);
}
int setMBPFPTipv4BroadcastFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	return setMBPFPTFilterValue(value, meInstantPtr,omciAttribute,flag,MAC_FILTER_IPV4_BROADCAST);
}
int setMBPFPTRARPFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	return setMBPFPTFilterValue(value, meInstantPtr,omciAttribute,flag,MAC_FILTER_RARP);
}
int setMBPFPTIPXFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	return setMBPFPTFilterValue(value, meInstantPtr,omciAttribute,flag,MAC_FILTER_IPX);
}
int setMBPFPTNetBEULFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	return setMBPFPTFilterValue(value, meInstantPtr,omciAttribute,flag,MAC_FILTER_NETBEUL);
}
int setMBPFPTappleTalkFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	return setMBPFPTFilterValue(value, meInstantPtr,omciAttribute,flag,MAC_FILTER_AARP);
}
int setMBPFPTmanageInfoFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	return setMBPFPTFilterValue(value, meInstantPtr,omciAttribute,flag,MAC_FILTER_MANAGE_INFO);
}
int setMBPFPTARPFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	return setMBPFPTFilterValue(value, meInstantPtr,omciAttribute,flag,MAC_FILTER_ARP);
}
int setMBPFPTPPPoEBroadcastFilterValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	return setMBPFPTFilterValue(value, meInstantPtr,omciAttribute,flag,MAC_FILTER_PPPOE_BROADCAST);
}
/*******************************************************************************************************************************
start
9.3.8 mac bridge port bridge table data

********************************************************************************************************************************/

int omciMeInitForMACBridgePortData(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciInternalCreateActionForMACBridge;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;
	
	return 0;
}


int32 getBPBTDBridgeTableValue(char *value, uint8 *meInstantPtr,  omciAttriDescript_ptr omciAttribute, uint8 flag){
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	char buffer[120] = {0};//a row string from file
	char *bufferPtr = buffer;	
	uint8 *tableBuffer = NULL;
	char *tempPtr = NULL;
	char tempStr[MAX_MAC_BRIDGE_RECORD_BUF] = {0};
	char macStr[MAX_MAC_BRIDGE_RECORD_BUF] = {0}; //get mac string from row string
	uint8 mac[OMCI_MAC_LENGTH] = {0};
//	int i = 0;
	uint16 ageTime = 0;
	/*recordHeadInfo for 9.3.8
	   Bit 	name			settng
	   1		filter/forward		0:forward,1:filter
	   2		reserved			0
	   3		dynamic/static	0:this entry is statically assigned, 1:this entry is dynamically learned
	   4		reserved 			0
	   16..5	age				age in seconds(1..4095)
	*/
	uint16 recordHeadInfo = 0;
	FILE *fp = NULL;
	uint32 tableSize = 0; //return table size
	uint32 length = 0;
	uint16 instanceId = 0;//instance id
	char ifcName[MAX_OMCI_IFC_NAME_LEN] = {0};
	uint8 findFlag = OMCI_NO_FIND_RULE;
	uint8 filterFlag = OMCI_MAC_FILTER_FLAG_0; //0:forward, 1:filter
	int ret = 0;
	uint32 fromLen = 0;

	/*get instance id*/
	instanceId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
 	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->instanceId = 0x%0x",instanceId);


	/*get interface name*/
	ret = getMacBridgePortIfcByInst(instanceId, ifcName);
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->getMacBridgePortIfcByInst, fail");
		goto failure;
	}
	
	/*write bridge table infomation to file*/
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "%s showportmacs br0>/tmp/omci/brctlshowportmacs.txt",OMCI_BRCTL_CMD);
	system(buffer);

	fp = fopen("/tmp/omci/brctlshowportmacs.txt", "r");
	if(fp == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->fp open is failure");
		goto failure;
	}
	
	/*1.calculate the table size*/
	fseek(fp, 0, SEEK_END);
	tableSize = ftell(fp);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->tableSize = %d", tableSize);

	/*2.malloc memory for table*/
	tableBuffer = (uint8 *)calloc(1 , tableSize);
	if(tableBuffer == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getBPBTDBridgeTableValue->calloc failure");
		fclose(fp);
		goto failure;
	}
	
	/*3. get mac table information in this port*/
	memset(buffer, 0, sizeof(buffer));
	memset(tempStr, 0, sizeof(tempStr));
	memset(tableBuffer, 0, tableSize);	

	fseek(fp, 0, SEEK_SET);
	fgets(buffer, sizeof(buffer), fp);//jump title row
	//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->buffer1=%s",buffer);

	length = 0;//reset length to 0 , in order to the table content
	while(fgets(buffer, sizeof(buffer), fp)){
		//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n brctlshow->buffer=%s",buffer);
		fromLen = 0;
		/*3.1.get interface from this line*/
		memset(tempStr, 0, sizeof(tempStr));
		ret = findMacBridgeFirstStr(buffer, &fromLen, tempStr);//tempStr include port id and interface name , eg:1(nas0)
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->findMacBridgeFirstStr fail 1");
			continue;
		}

//		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->tempStr(interface)=%s",tempStr);
		if(strcmp(tempStr, ifcName) != 0){
			continue;
		}
		
		/*3.2.get  mac address from this line*/
		memset(macStr, 0, sizeof(macStr));
		ret = findMacBridgeFirstStr(bufferPtr, &fromLen, macStr);			
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->findMacBridgeFirstStr fail 2");
			continue;
		}
				
		/*3.3.get local? from this line*/
		memset(tempStr, 0, sizeof(tempStr));
		ret = findMacBridgeFirstStr(bufferPtr, &fromLen, tempStr);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->findMacBridgeFirstStr fail 3");
			continue;
		}
		if(strcmp(tempStr,"no") !=0){
			continue;
		}
		/*3.4.get age time string from this line*/
		memset(tempStr, 0, sizeof(tempStr));
		ret = findMacBridgeFirstStr(bufferPtr, &fromLen, tempStr);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->findMacBridgeFirstStr fail 4");
			continue;
		}
		ageTime = atoi(tempStr);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->ageTime = 0x%02x",ageTime);
		
		/*3.5.strip ":" from macStr, get mac address*/
		ret =  getMacFormMacStr(':', macStr, mac);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->findMacBridgeFirstStr fail 5");
			continue;
		}
		
		/*3.6.search mac address in mac bridge filter table(9.3.6 and 9.3.7)*/				
		findFlag = OMCI_NO_FIND_RULE;
		filterFlag = OMCI_MAC_FILTER_FLAG_0;
		ret =  getSearchFilterMacFormFilterTbl(instanceId, mac, &findFlag, &filterFlag);

		/*3.7 add a record in table*/
		recordHeadInfo = 0;
		recordHeadInfo = (ageTime<<4);//set age time to record information 				
		recordHeadInfo |= (1<<2);//set dynamic mac type to record information
		if((findFlag == OMCI_FIND_RULE) && (filterFlag == OMCI_MAC_FILTER_FLAG_1)){//set filter type to record information
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->getSearchFilterMacFormFilterTbl find item");
			recordHeadInfo |= (1<<0);
		}else{//forward
			//nothing
		}
		tempPtr = (char *)tableBuffer + length;
		put16((uint8 *)tempPtr, recordHeadInfo);
//		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->mac = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		memcpy((tempPtr+2), mac, OMCI_MAC_LENGTH);
		length += MAC_FILTER_RECORD_LENGTH;
	}
	fclose(fp);		
	
	/*4. add the table to omci archtecture*/
	if(length == 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->length == 0");
		goto failure;
	}
	ret = omciAddTableContent(tmpomciMeInst_p->classId, instanceId,tableBuffer, length ,omciAttribute->attriName);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->omciAddTableContent fail\n");
		goto failure;
	}

//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getBPBTDBridgeTableValue->length=%d",length);
	if (tableBuffer != NULL){
		free(tableBuffer);
	}
	return getTheValue(value, (char*)&length, 4, omciAttribute);
 
failure:
	length = 0;
	if (tableBuffer != NULL){
		free(tableBuffer);
	}
	return getTheValue(value, (char*)&length, 4, omciAttribute);
}

/*******************************************************************************************************************************
start
9.3.9 mac bridge port performance monitoring history data

********************************************************************************************************************************/

int omciMeInitForMACBridgePortPMHD(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_CURR_DATA] = omciGetCurrentDataAction;	
	
	return 0;
}
int setThresholdDataForMACBridgePort(IN uint16 currClassId, IN uint16 currDeviceId, IN uint16 thresholdInstId){
	int ret = -1;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	uint32 thresholdData = 0;	

	/*1. set Delay exceeded discard cnt attribute*/
	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdInstId, 1, &thresholdData);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setThresholdDataForMACBridgePort-> getThresholdDataValue 1, fail!");
		ret = -1;
		goto end;
	}
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet(currClassId, currDeviceId, PMMGR_THRESHOLD, 2, tempBuffer);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setThresholdDataForMACBridgePort-> pmgrTcapiSe 1, fail!");
		ret = -1;
		goto end;
	}
	/*2. set MTU exceeded discard cnt attribute*/
	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdInstId, 2, &thresholdData);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setThresholdDataForMACBridgePort-> getThresholdDataValue 2, fail!");
		ret = -1;
		goto end;
	}
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet(currClassId, currDeviceId, PMMGR_THRESHOLD, 3, tempBuffer);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setThresholdDataForMACBridgePort-> pmgrTcapiSe 2, fail!");
		ret = -1;
		goto end;
	}
	/*3. set received and discard cnt attribute*/
	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdInstId, 3, &thresholdData);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setThresholdDataForMACBridgePort-> getThresholdDataValue 3, fail!");
		ret = -1;
		goto end;
	}
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet(currClassId, currDeviceId, PMMGR_THRESHOLD, 5, tempBuffer);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setThresholdDataForMACBridgePort-> pmgrTcapiSe 3, fail!");
		ret = -1;
		goto end;
	}	

	ret = 0;
end:
	return ret;

}
int32 setMBPPMHDThresholdDataValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag){

	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;

	uint16 thresholdMeId = 0;
	uint16 length = 0;
	uint8 *attributeValuePtr = NULL;
//	int i = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPPMHDThresholdDataValue-> attributeValuePtr == NULL, fail!");
		return -1;
	}
	memcpy(attributeValuePtr, value, length);

	thresholdMeId = get16(attributeValuePtr);
	if((thresholdMeId != 0) && (thresholdMeId != 0xffff)){

		ret = setThresholdDataForMACBridgePort(tmpomciMeInst_p->classId,tmpomciMeInst_p->deviceId, thresholdMeId);
		if (ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMBPPMHDThresholdDataValue-> setThresholdDataForMACBridgePort, fail!");
			return -1;
		}
	}
	return 0;
}

/*******************************************************************************************************************************
start
9.3.30 ethernet frame performance monitoring history data upstream

********************************************************************************************************************************/

int omciMeInitForEthFrameUpPMHD(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_CURR_DATA] = omciGetCurrentDataAction;	
	
	return 0;
}

int setThresholdDataForEthFrame(IN uint16 currClassId, IN uint16 currDeviceId, IN uint16 thresholdInstId){
	int ret = -1;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	uint32 thresholdData = 0;

	/*1. set drop event attribute*/
	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdInstId, 1, &thresholdData);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setThresholdDataForEthFrame-> getThresholdDataValue 1, fail!");
		ret = -1;
		goto end;
	}
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet(currClassId, currDeviceId, PMMGR_THRESHOLD, 1, tempBuffer);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setThresholdDataForEthFrame-> pmgrTcapiSe 1, fail!");
		ret = -1;
		goto end;
	}
	/*2. set CRC error packets attribute*/
	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdInstId, 2, &thresholdData);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setThresholdDataForEthFrame-> getThresholdDataValue 2, fail!");
		ret = -1;
		goto end;
	}
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet(currClassId, currDeviceId, PMMGR_THRESHOLD, 6, tempBuffer);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setThresholdDataForEthFrame-> pmgrTcapiSe 2, fail!");
		ret = -1;
		goto end;
	}
	/*3. set undersize packets attribute*/
	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdInstId, 3, &thresholdData);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setThresholdDataForEthFrame-> getThresholdDataValue 3, fail!");
		ret = -1;
		goto end;
	}
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet(currClassId, currDeviceId, PMMGR_THRESHOLD, 7, tempBuffer);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setThresholdDataForEthFrame-> pmgrTcapiSe 3, fail!");
		ret = -1;
		goto end;
	}

	/*3. set oversize packets attribute*/
	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdInstId, 4, &thresholdData);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setThresholdDataForEthFrame-> getThresholdDataValue 4, fail!");
		ret = -1;
		goto end;
	}
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet(currClassId, currDeviceId, PMMGR_THRESHOLD, 8, tempBuffer);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setThresholdDataForEthFrame-> pmgrTcapiSe 4, fail!");
		ret = -1;
		goto end;
	}	

	ret = 0;
end:
	return ret;

}
int32 setEFPMHDThresholdDataValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;

	uint16 thresholdMeId = 0;
	uint16 length = 0;
	uint8 *attributeValuePtr = NULL;
//	int i = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setEFPMHDThresholdDataValue-> attributeValuePtr == NULL, fail!");
		return -1;
	}
	memcpy(attributeValuePtr, value, length);

	thresholdMeId = get16(attributeValuePtr);

	if((thresholdMeId != 0) && (thresholdMeId != 0xFFFF)){
		ret = setThresholdDataForEthFrame(tmpomciMeInst_p->classId,tmpomciMeInst_p->deviceId, thresholdMeId);
		if (ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setEFPMHDThresholdDataValue-> setThresholdDataForEthFrame, fail!");
			return -1;
		}
	}
	return 0;
}

/*******************************************************************************************************************************
start
9.3.31 ethernet frame performance monitoring history data downstream

********************************************************************************************************************************/

int omciMeInitForEthFrameDownPMHD(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_CURR_DATA] = omciGetCurrentDataAction;	
	
	return 0;
}
