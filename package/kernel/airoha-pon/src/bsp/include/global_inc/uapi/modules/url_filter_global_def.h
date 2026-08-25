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

#ifndef _UAPI_URL_FILTER_GLOBAL_DEF_H_
#define _UAPI_URL_FILTER_GLOBAL_DEF_H_

/**
* \file  url_filter_global_def.h  
* \brief This file is url filter global def header file that will be exported for others to use.
* \author Lei.Zhang
* \date     2020-09-24
* \version  A001 
* \copyright EcoNet Inc                                                              
*/

/************************************************************************
*                  I N C L U D E S
*************************************************************************/

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
************************************************************************/
typedef enum
{
	E_URL_ADD = 1,
	E_URL_REM_ALL,
	E_URL_REM_GLOBAL_ALL,
	E_URL_REM_BASEMAC_ALL,
	E_DNS_ADD,
	E_DNS_REM_ALL,
	E_DNS_REM_GLOBAL_ALL,
	E_DNS_REM_BASEMAC_ALL,
	E_URL_CMD_MAX
}URLFITER_CMD_TYPE;

#define URL_TYPE_HOST	1
#define URL_TYPE_PATH	2
#define URL_LEN	32
#define MAX_MAC_LENTH		260

typedef struct _url_info
{
	unsigned char index;
	unsigned char type;		/*host or path*/
	char host[URL_LEN];
	char path[URL_LEN];
	unsigned int blockedtimes;
	char MacAddr[MAX_MAC_LENTH];
}url_info;

#define URL_FILTER_IOC_MAGIC 	'h'
#define URL_FILTER_SET_HASHDATA _IOW(URL_FILTER_IOC_MAGIC,6,struct _urlfilter_ioctl_data_s)
#define URL_FILTER_GET_INFO _IOW(URL_FILTER_IOC_MAGIC,7,struct _urlfilter_ioctl_info_s)
#define URL_FILTER_SET_LANIP _IOW(URL_FILTER_IOC_MAGIC,8,struct _dnsfilter_ioctl_lanip_s)

#define URL_FILTER_IOC_SET_ENABLE _IOW(URL_FILTER_IOC_MAGIC,0,unsigned char)
#define URL_FILTER_IOC_SET_MODE _IOW(URL_FILTER_IOC_MAGIC,1,unsigned char)
#define URL_FILTER_SET_URL _IOW(URL_FILTER_IOC_MAGIC,2,struct _url_info)

/************************************************************************
*                  M A C R O S
************************************************************************/

/************************************************************************
*                  D A T A   T Y P E S
************************************************************************/
typedef struct _urlfilter_obj_data_s
{
	int cmd_type;
	int index;
	int mode; /* 0:black, 1:white. */
	int match_all;
	int action;/* 0:drop, 1:response lanip 2:response name error. */
	int port;
	int times; /* blocktimes */
	char domain[256];
}urlfilter_obj_data;

typedef struct _urlfilter_ioctl_data_s
{
	int cmd_type;
	int urlfilter_obj_cnt;
	int clearbtflag;
	urlfilter_obj_data *obj;
	char mac_addr[20];
}urlfilter_ioctl_data;

typedef struct _urlfilter_ioctl_info_s
{
	int index;
	unsigned long blocked_times;
}urlfilter_ioctl_info;

typedef struct _dnsfilter_ioctl_lanip_s
{
	char lanip[16];
	char lanip6[48];
}dnsfilter_ioctl_lanip_t;

typedef struct _dns_urlfilter_data_s
{
	int cmd_type;
	int index;
	int mode; /* 0:black, 1:white. */
	int match_all;
	int action;/* 0:drop, 1:response lanip 2:response name error. */
	int times; /* blocktimes */
	char domain[256];
}dns_urlfilter_data;

typedef struct _dns_urlfilter_info_s
{
	int index;
	unsigned long blocked_times;
}dns_urlfilter_info;

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
************************************************************************/


/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
************************************************************************/


/************************************************************************
*                  P U B L I C   D A T A
************************************************************************/




#endif
