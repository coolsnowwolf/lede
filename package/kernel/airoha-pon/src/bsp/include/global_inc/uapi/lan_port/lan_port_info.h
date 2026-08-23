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

#ifndef __UAPI_LAN_PORT_INFO_H__
#define __UAPI_LAN_PORT_INFO_H__

/**
* \file  lan_port_info.h 
* \brief This file is lan port info header file that will be exported for others to use.
* \author Lei.Zhang
* \date     2020-09-24
* \version  A001 
* \copyright EcoNet Inc                                                              
*/

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#include <stdio.h>
#endif

/*************************Notice**************************************** 
#define MAX_LAN_PORT_NUM     ((sizeof(bl_map_data)) / (sizeof((bl_map_data)[0])))
**********************************************************************/
#define     MAX_LAN_PORT_NUM                    (26)
/*****************itf name format********************/
#define     S_ETHER_ITF_NAME_FORMAT             "eth0"

#define     M_ETHER_ITF_NAME_FORMAT             "eth0."

#define     WLAN_ITF_NAME_FORMAT                "ra"
#define     WLAN_ITF_NAME_SIZE                  (3)

#define     WLAN_AC_ITF_NAME_FORMAT             "rai"
#define     WLAN_AC_ITF_NAME_SIZE               (4)

#define     WLAN_APCLI_ITF_NAME_FORMAT          "apcli"
#define     WLAN_AC_APCLI_ITF_NAME_FORMAT       "apclii"
#define     USB_ITF_NAME_FORMAT                 "usb"

#define     CT_ETHER_WAN_ITF_NAME                  "nas10"
#define     CMCC_ETHER_WAN_ITF_NAME                  "nas10"
#define     UNION_ETHER_WAN_ITF_NAME               "nas0_0"
#define     SISM_HOST_AEWAN_ITF_NAME               "ae_wan"

/******************Notice special itf name**************/
#define    	ETHER_ITF_NAME_1 					"eth0.1"
#define    	ETHER_ITF_NAME_2 					"eth0.2"
#define     WLAN_ITF_NAME_4               		"ra3"
#define     VETH_ITF_NAME_1               		"veth1"


/*********************mark id **********************/
#define     SSID1_PORT_MASK                     (0x9)
#define     SSID2_PORT_MASK                     0xA
#define     SSID3_PORT_MASK                     0xB
#define     SSID4_PORT_MASK                     0xC
#define     SSID5_PORT_MASK                     0xD
#define     SSID6_PORT_MASK                     0xE
#define     SSID7_PORT_MASK                     0xF
#define     SSID8_PORT_MASK                     0x10

#define     SSIDAC1_PORT_MASK                   0x11
#define     SSIDAC2_PORT_MASK                   0x12
#define     SSIDAC3_PORT_MASK                   0x13
#define     SSIDAC4_PORT_MASK                   0x14
#define     SSIDAC5_PORT_MASK                   0x15
#define     SSIDAC6_PORT_MASK                   0x16
#define     SSIDAC7_PORT_MASK                   0x17
#define     SSIDAC8_PORT_MASK                   0x18

/************************support various class port number info****************************/
#define     MAX_ECNT_10G_ETHER_PORT_NUM         (4)

#if defined(TCSUPPORT_XSI_ENABLE) && defined(TCSUPPORT_MULTI_SWITCH_EXT)
#define     MAX_ECNT_1GETHER_PORT_NUM           (16)
#elif defined(TCSUPPORT_MULTI_SWITCH_EXT)
#define     MAX_ECNT_1GETHER_PORT_NUM           (8)
#else
#define     MAX_ECNT_1GETHER_PORT_NUM           (4)
#endif
#define     MAX_ECNT_ETHER_PORT_NUM             (MAX_ECNT_1GETHER_PORT_NUM + MAX_ECNT_10G_ETHER_PORT_NUM)
#define     MAX_ECNT_ETHER_PORT_ITF_NUM         (8)


#if defined(TCSUPPORT_MULTI_USER_ITF) 
#define     MAX_ECNT_WALN_PORT_NUM              (8)
#define     MAX_ECNT_WALNAC_PORT_NUM            (8)
#else
#define     MAX_ECNT_WALN_PORT_NUM              (4)
#define     MAX_ECNT_WALNAC_PORT_NUM            (4)
#endif
#define     MAX_ECNT_WLAN_PORT_ITF_NUM     	    (8)
#define     MAX_ECNT_WLANAC_PORT_ITF_NUM        (8)


#define     MAX_ECNT_WALN_USE_PORT_NUM          (4)
#define     MAX_ECNT_LAN_USE_PORT_NUM           (4)

#define     MAX_ECNT_USB_PORT_NUM               (1)
#define     MAX_ECNT_WDS_PORT_NUM               (1)
#define     ECNT_WLANAC_PORT_OFFSET             (MAX_ECNT_ETHER_PORT_NUM + MAX_ECNT_WALN_PORT_NUM)

#define 	MAX_BSSID_NUM 						(8)
#define 	MAX_DUALBAND_BSSID_NUM 				(MAX_ECNT_WALN_PORT_NUM + MAX_ECNT_WALNAC_PORT_NUM)
/********************************dev offset *******************************************/
/*WLAN_DEV_OFFSET: 5*/
#define     WLAN_DEV_OFFSET                     ((MAX_ECNT_ETHER_PORT_ITF_NUM) + 1)
/*WLAN_AC_DEV_OFFSET: 13*/
#define     WLAN_AC_DEV_OFFSET                  ((WLAN_DEV_OFFSET) + (MAX_ECNT_WLAN_PORT_ITF_NUM))
/*USB_DEV_OFFSET:        21*/
#define     USB_DEV_OFFSET                      ((WLAN_AC_DEV_OFFSET) + (MAX_ECNT_WLANAC_PORT_ITF_NUM))
/*WDS_DEV_OFFSET:      22*/
#define     WDS_DEV_OFFSET                      ((USB_DEV_OFFSET) + (MAX_ECNT_USB_PORT_NUM))
/*********************************itf mask********************************************/
/*****such as ETHER_ITF_MASK = (2 ^ N - 1) > MAX_ECNT_ETHER_PORT_NUM*****/
#define     ETHER_ITF_MASK                      (0xf)
/***********************************************************************************/
#if defined(TCSUPPORT_MULTI_USER_ITF) || defined(TCSUPPORT_MULTI_SWITCH_EXT)
/*skb->mark 27~31bit*/
#define     DEV_OFFSET                          (27)
#define     LAN_ITF_MASK                        (0x1f)
#else
#define     DEV_OFFSET                          (28)
#define     LAN_ITF_MASK                        (0xf)
#endif
#define     LANIF_MASK                          ((LAN_ITF_MASK) << (DEV_OFFSET))

#define     GET_ETHER_ITF_MARK(x)               (((x) >> DEV_OFFSET) & (ETHER_ITF_MASK))

#define     GET_LAN_ITF_MARK(x)                 (((x) >> DEV_OFFSET) & (LAN_ITF_MASK))
#define     XFI_LAN_DEV_OFFSET                  (11)
#define     XFI_LAN_ITF_MASK                    (0x7)
#define     GET_XFI_LAN_ITF_MARK(x)             (((x) >> XFI_LAN_DEV_OFFSET) & (XFI_LAN_ITF_MASK))
/*********************************inline      function*************************************/
/*macro define compatible*/
#if 0
static inline int devname_convert_port(char *devname)
{
    int portnum = -1;

    if ((M_ETHER_ITF_NAME_SIZE == strlen(devname)) \
        && (NULL != strstr(devname, M_ETHER_ITF_NAME_FORMAT))) 
    {
        portnum = devname[M_ETHER_ITF_NAME_SIZE - 1] - '0';
    }
    else if ((WLAN_AC_ITF_NAME_SIZE == strlen(devname)) \
        && (NULL != strstr(devname, WLAN_AC_ITF_NAME_FORMAT)))
    {
        portnum = devname[WLAN_AC_ITF_NAME_SIZE - 1] - '0' + WLAN_AC_DEV_OFFSET;
    }
    else if ((WLAN_ITF_NAME_SIZE == strlen(devname)) \
        && (NULL != strstr(devname, WLAN_ITF_NAME_FORMAT)))
    {
        portnum = devname[WLAN_ITF_NAME_SIZE - 1] - '0' + WLAN_DEV_OFFSET;
    }

    return portnum;
}
#else
static inline int devname_convert_port(char *devname)
{
    int portnum = -1;

    if (NULL != strstr(devname, M_ETHER_ITF_NAME_FORMAT)) 
    {
    	sscanf(devname, M_ETHER_ITF_NAME_FORMAT"%d", &portnum);
		if(portnum < 1 || portnum > MAX_ECNT_ETHER_PORT_NUM)
		{
			portnum = -1;
		}
    }
    else if (NULL != strstr(devname, WLAN_AC_ITF_NAME_FORMAT))
    {
    	sscanf(devname, WLAN_AC_ITF_NAME_FORMAT"%d", &portnum);
		if(portnum >= 0 && portnum < MAX_ECNT_WALNAC_PORT_NUM)
		{
			portnum = portnum + WLAN_AC_DEV_OFFSET;
		}
		else{
			portnum = -1;
		}
    }
    else if (NULL != strstr(devname, WLAN_ITF_NAME_FORMAT))
    {
    	sscanf(devname, WLAN_ITF_NAME_FORMAT"%d", &portnum);
		if(portnum >= 0 && portnum < MAX_ECNT_WALN_PORT_NUM)
		{
			portnum = portnum + WLAN_DEV_OFFSET;
		}
		else{
			portnum = -1;
		}
    }

    return portnum;
}
#endif


/*macro define compatible*/
#if 0
static inline int offset_convert_devname(int offset, char* dev_name, int size)
{
	
	memset(dev_name, 0, size);
	if(MAX_ECNT_ETHER_PORT_NUM >= offset)
	{
		snprintf(dev_name, size, "%s%d", M_ETHER_ITF_NAME_FORMAT, offset);
	}
	else if((WLAN_DEV_OFFSET <= offset) && (WLAN_AC_DEV_OFFSET > offset))
	{
		snprintf(dev_name, size, "%s%d", WLAN_ITF_NAME_FORMAT, (offset-WLAN_DEV_OFFSET));
	}
	else if((WLAN_AC_DEV_OFFSET <= offset) && (USB_DEV_OFFSET > offset))
	{
		snprintf(dev_name, size, "%s%d", WLAN_AC_ITF_NAME_FORMAT, (offset-WLAN_AC_DEV_OFFSET));
	}
	else
	{
		return -1;
	}
	
	return 0;
}
#else
static inline int offset_convert_devname(int offset, char* dev_name, int size)
{
	
	memset(dev_name, 0, size);
	if(8 >= offset)
	{
		snprintf(dev_name, size, "%s%d", M_ETHER_ITF_NAME_FORMAT, offset);
	}
	else if((9 <= offset) && (17 > offset))
	{
		snprintf(dev_name, size, "%s%d", WLAN_ITF_NAME_FORMAT, (offset - 9) );
	}
	else if((17 <= offset) && (25 > offset))
	{
		snprintf(dev_name, size, "%s%d", WLAN_AC_ITF_NAME_FORMAT, (offset - 17) );
	}
	else
	{
		return -1;
	}
	
	return 0;
}
#endif

#endif

