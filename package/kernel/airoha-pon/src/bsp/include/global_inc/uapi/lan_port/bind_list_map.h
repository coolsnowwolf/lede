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

#ifndef __UAPI_BIND_LIST_MAP_H__
#define __UAPI_BIND_LIST_MAP_H__

/**
* \file  bind_list_map.h 
* \brief This file is bind list map header file that will be exported for others to use.
* \author Lei.Zhang
* \date     2020-09-24
* \version  A001 
* \copyright EcoNet Inc                                                              
*/

struct bindlist_map
{
    char *bindif;
    char *realif;
    char  bit;
    char  mark;
};

/* support four lan port and four wlan port */
static const struct bindlist_map bl_map_data[] = 
{
    {"LAN1",        "eth0.1",    0x0,       0x1},
    {"LAN2",        "eth0.2",    0x1,       0x2},
    {"LAN3",        "eth0.3",    0x2,       0x3},
    {"LAN4",        "eth0.4",    0x3,       0x4},
    {"LAN5",        "eth0.5",    0x4,       0x5},
    {"LAN6",        "eth0.6",    0x5,       0x6},
    {"LAN7",        "eth0.7",    0x6,       0x7},
    {"LAN8",        "eth0.8",    0x7,       0x8},

    {"SSID1",       "ra0",       0x8,       0x9},
    {"SSID2",       "ra1",       0x9,       0xA},
    {"SSID3",       "ra2",       0xA,       0xB},
    {"SSID4",       "ra3",       0xB,       0xC},
    {"SSID5",       "ra4",       0xC,       0xD},
    {"SSID6",       "ra5",       0xD,       0xE},
    {"SSID7",       "ra6",       0xE,       0xF},
    {"SSID8",       "ra7",       0xF,       0x10},

    {"SSIDAC1",     "rai0",      0x10,      0x11},
    {"SSIDAC2",     "rai1",      0x11,      0x12},
    {"SSIDAC3",     "rai2",      0x12,      0x13},
    {"SSIDAC4",     "rai3",      0x13,      0x14},
    {"SSIDAC5",     "rai4",      0x14,      0x15},
    {"SSIDAC6",     "rai5",      0x15,      0x16},
    {"SSIDAC7",     "rai6",      0x16,      0x17},
    {"SSIDAC8",     "rai7",      0x17,      0x18},

    {"USB",         "usb0",      0x18,      0x19},
    {"WDS",         "wds0",      0x19,      0x1A},
};

static inline int mark_convert_bind_itf_name(int mark, char* bind_itf_name, int size)
{
	int i = 0;
	memset(bind_itf_name, 0, size);
	for(i = 0; i < sizeof(bl_map_data) / sizeof(bl_map_data[0]); i++)
	{
		if(bl_map_data[i].mark == mark)
		{
			snprintf(bind_itf_name, size, bl_map_data[i].bindif);
			return 0;
		}
	}
	
	return -1;
}

static inline int mark_convert_real_itf_name(int mark, char* real_itf_name, int size)
{
	int i = 0;
	memset(real_itf_name, 0, size);
	for(i = 0; i < sizeof(bl_map_data) / sizeof(bl_map_data[0]); i++)
	{
		if(bl_map_data[i].mark == mark)
		{
			snprintf(real_itf_name, size, bl_map_data[i].realif);
			return 0;
		}
	}
	
	return -1;
}

static inline int bit_convert_bind_itf_name(int bit, char* bind_itf_name, int size)
{
	int i = 0;
	memset(bind_itf_name, 0, size);
	for(i = 0; i < sizeof(bl_map_data) / sizeof(bl_map_data[0]); i++)
	{
		if(bl_map_data[i].bit == bit)
		{
			snprintf(bind_itf_name, size, bl_map_data[i].bindif);
			return 0;
		}
	}
	
	return -1;
}

static inline int bit_convert_real_itf_name(int bit, char* real_itf_name, int size)
{
	int i = 0;
	memset(real_itf_name, 0, size);
	for(i = 0; i < sizeof(bl_map_data) / sizeof(bl_map_data[0]); i++)
	{
		if(bl_map_data[i].bit == bit)
		{
			snprintf(real_itf_name, size, bl_map_data[i].realif);
			return 0;
		}
	}
	
	return -1;
}


#endif

