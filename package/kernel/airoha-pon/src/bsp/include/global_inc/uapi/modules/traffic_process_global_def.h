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

#ifndef _UAPI_TRAFFIC_PROCESS_GLOBAL_DEF_H_
#define _UAPI_TRAFFIC_PROCESS_GLOBAL_DEF_H_

/**
* \file  traffic_process_global_def.h  
* \brief This file is traffic process global def header file that will be exported for others to use.
* \author Lei.Zhang
* \date     2020-09-24
* \version  A001 
* \copyright EcoNet Inc                                                              
*/

/* for TRAFFIC PROCESS. */
#define CMD_TRAFFIC_GET_COUNT		(1)
#define CMD_TRAFFIC_GET_DATA		(2)

#define TRAFFIC_MODULE_NAME         "traffic_process"
#define TRAFFIC_MAJOR             	(251)
#define TRAFFIC_MAX_FLOWNUM_RULE 	255
#define TRAFFIC_MAX_DOMAIN_LEN		256
#define PPEMGR_TYPE_UPSTREAM 0
#define PPEMGR_TYPE_DWSTREAM 1
#define PPEMGR_TYPE_BOTH 2	

#define PPEMGR_SIGN_TRAFFIC_DETAIL 0
#define PPEMGR_SIGN_TRAFFIC_MIRROR 1
#define PPEMGR_SIGN_TRAFFIC_MONITOR 2

#define TRAFFIC_DETAIL_PATH 				"/etc/TrafficDetail.data"
#define TRAFFIC_MIRROR_PATH 				"/etc/TrafficMirror.data"
#define TRAFFIC_MONITOR_PATH				"/etc/TrafficMonitor.data"

typedef struct _ecnt_ip_addr_ {
  union {
	struct in_addr addr4;
	struct in6_addr addr6;
  } addr;
}ecnt_ip_addr;

typedef struct _ecnt_sock_addr_ {
  union {
	struct sockaddr_in sock_dst;
	struct sockaddr_in6 sock6_dst;
  } sock;
}ecnt_sock_addr;

typedef struct _traffic_monitor_data_s
{
	ecnt_ip_addr ecnt_addr;
	int port;
}traffic_monitor_data;

typedef struct _traffic_mirror_data_s
{
	char ipaddress[40];
	ecnt_sock_addr mirror_dst;
	int to_ai_family;
	int mirror_isloop;
	void *socket;
}traffic_mirror_data;

typedef struct _traffic_mirror_install_data_s
{
	int sockfd_loop;
	int sockfd_wan;
	int sockfd_loopv6;
	int sockfd_wanv6;
	int sockfd_lanv6;
}traffic_mirror_install_data;

typedef enum
{
	E_TRAFFIC_MONITOR		= 1,
	E_TRAFFIC_MIRROR,
	E_TRAFFIC_MAX			= 4
}TRAFFIC_TYPE;

typedef enum
{
	E_CMD_INSTALL 			= 1,
	E_CMD_UNINSTALL,
	E_CMD_MONITOR_CNT,
	E_CMD_MONITOR_DATE,
	E_CMD_ADD_MIRROR_RULE,
	E_CMD_DEL_MIRROR_RULE
}TRAFFIC_CMD_TYPE;

typedef struct _traffic_ioctl_data_s
{
	int traffic_type;
	int data_len;
	void *data;
}traffic_ioctl_data;

typedef struct _monitor_file_node_info_
{
	char destAddr[TRAFFIC_MAX_DOMAIN_LEN];
}monitor_file_node_info_t, *pt_monitor_file_node_info;

typedef struct _monitor_file_bundle_info_
{
	char bundle_name[TRAFFIC_MAX_DOMAIN_LEN];
	int node_count;
	monitor_file_node_info_t node[TRAFFIC_MAX_FLOWNUM_RULE];
}monitor_file_bundle_info_t, *pt_monitor_file_bundle_info;

typedef struct _monitor_file_info_
{
	unsigned int monitor_timeout;
	char wan_ipv4[20];
	char wan_ipv6[40];
	char wan_name[65];
	char wan_name6[65];
	monitor_file_bundle_info_t bundle[8];
}monitor_file_info_t, *pt_monitor_file_info;

typedef struct _mirror_file_node_info_
{
	char remoteAddress[TRAFFIC_MAX_DOMAIN_LEN];
	char remotePort[20];
	char direction[12];
	char protocol[12];
	char hostMAC[20];
	char mirrorToIP[64];
	char mirrorToPort[12];
	int entrynum;
}mirror_file_node_info_t, *pt_mirror_file_node;

typedef struct _mirror_file_info_
{
	int node_count;
	mirror_file_node_info_t node[TRAFFIC_MAX_FLOWNUM_RULE];
}mirror_file_info_t, *pt_mirror_file_info;

typedef struct _detail_file_node_info_
{
	unsigned int ruleinst;
	char remoteAddress[TRAFFIC_MAX_DOMAIN_LEN];
	char remotePort[20];
	char direction[12];
	char hostMAC[20];
	char methodList[TRAFFIC_MAX_DOMAIN_LEN];
	char statuscodeList[TRAFFIC_MAX_DOMAIN_LEN];
	char headerList[TRAFFIC_MAX_DOMAIN_LEN];
	char bundle_name[TRAFFIC_MAX_DOMAIN_LEN];
}detail_file_node_info_t, *pt_detail_file_node_info;

typedef struct _detail_file_info_
{
	int node_count;
	detail_file_node_info_t node[TRAFFIC_MAX_FLOWNUM_RULE];
}detail_file_info_t, *pt_detail_file_info;

#endif
