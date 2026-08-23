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

#ifndef _GLOBAL_DNS_HOST_H
#define _GLOBAL_DNS_HOST_H

/**
* \file  global_dnshost.h  
* \brief This file is global dnshost header file that will be exported for others to use.
* \author Lei.Zhang
* \date     2020-09-24
* \version  A001 
* \copyright EcoNet Inc                                                              
*/

#include <netinet/in.h>

#define MAXLEN_DOMAIN 256
#define TRAFFIC_APP_MAX 5
#define NO_ACT 0
#define ADD_ACT 1
#define DEL_ACT 2
#define ADD_V4_IP 3
#define ADD_V6_IP 4
#define SEARCH_V4_IP 5
#define SEARCH_V6_IP 6
#define SHOW_TOTAL_HASHMAP 7

#define TRAFFIC_APP_MAX 5

#define BIT_DNS     0
#define BIT_DETAIL  1
#define BIT_MIRROR  2
#define BIT_MONITOR 3
#define BIT_QOS     4
#define BIT_FORWARD 5

#define MAX_NODE_NAME_SIZE 34

struct traffic_total{	
	char *key;/*hostname*/
	int bitflag; /*00001:trafficdetail 00010:trafficmirror 00100:trafficmonitor 01000:trafficqos 10000:trafficfarward*/
	struct in_addr addr4;
	struct in6_addr addr6;
	time_t now;
	int istimeout;
	char domain_name[MAXLEN_DOMAIN];
};

struct traffic_app{	
	char *key;/*hostname*/
};
typedef struct
traffic2host_msg{
	unsigned int bitflag; /*1:trafficdetail  2:trafficmirror 3:trafficmonitor  4:trafficqos 5:trafficforward 0:dnsmasq*/
	unsigned int action; /*1:add 2:del 3:insert v4ip 4:insert v6ip  5:need ipv4 addr 6:need ipv6 addr 0:no act*/
	unsigned int exists_flag; /*0:not exists 1:exists  to tell dnsmasq*/
	union {
	struct in_addr addr4;
	struct in6_addr addr6;
	} addr;
	char domain_name[MAXLEN_DOMAIN];
	unsigned long cttl;
} traffic2host_msg_t;

#define GET_BIT_IN_BYTE(idx, byte)	((1<<(idx)) & (byte))
#define REVERSEBIT(x,y) (x)^=(1<<(y))
#define MAX_TRAFFIC_CLIENT	5
#define TRAFFICAPP_PATH "/tmp/trafficapp_sock"

#endif
