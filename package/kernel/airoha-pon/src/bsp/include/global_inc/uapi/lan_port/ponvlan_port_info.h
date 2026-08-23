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

#ifndef __UAPI_PONVLAN_PORT_INFO_H__
#define __UAPI_PONVLAN_PORT_INFO_H__

/**
* \file  ponvlan_port_info.h
* \brief This file is ponvlan port info header file that will be exported for others to use.
* \author Lei.Zhang
* \date     2020-09-24
* \version  A001 
* \copyright EcoNet Inc                                                              
*/

/*********************************MICRO CONSTANT************************************/

/* port index start */
#define PONVLAN_PORT_OFFSET_DEFAULT		1		//for default vlan rule in oam
#define PONVLAN_PORT_OFFSET_DEFGPON		2		// for default vlan rule in omci
#define PONVLAN_PORT_OFFSET_VP			3		// for VEIP
#define PONVLAN_PORT_OFFSET_IPHOST		4		// for IPHost port
#define PONVLAN_PORT_OFFSET_ETH			10		
#define PONVLAN_PORT_START_ETH			11		// for start No. of ether port
#define PONVLAN_PORT_OFFSET_WLAN		50		// for WLAN port
#define PONVLAN_PORT_START_WLAN			51		// for start No. of WLAN port
#define PONVLAN_PORT_OFFSET_WLANAC		90		// for WLAN AC port
#define PONVLAN_PORT_START_WLANAC		91		// for start No. of WLAN AC port
#define PONVLAN_PORT_OFFSET_USB			130		// for USB port
#define PONVLAN_PORT_START_USB			131		// for start No. of USB port
#define PONVLAN_PORT_HWNAT_CLEAN        255


/*********************************inline      function*************************************/


#endif

