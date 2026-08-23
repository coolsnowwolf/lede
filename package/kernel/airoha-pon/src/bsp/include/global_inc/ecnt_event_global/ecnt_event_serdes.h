/***************************************************************
Copyright Statement:

This software/firmware and related documentation (AIROHA Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to AIROHA Limited (AIROHA) and/or 
its licensors. Without the prior written permission of AIROHA and/or its licensors, 
any reproduction, modification, use or disclosure of AIROHA Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

AIROHA Limited AIROHA. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (AIROHA SOFTWARE) RECEIVED FROM AIROHA 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES AIROHA PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE AIROHA SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE.

AIROHA SHALL NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
AIROHA'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AIROHA 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT AIROHA'S SOLE OPTION, TO 
REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO AIROHA FOR SUCH 
AIROHA SOFTWARE.
***************************************************************/

#ifndef __ECNT_EVENT_SERDES_H_
#define __ECNT_EVENT_SERDES_H_

/************************************************************************
*               I N C L U D E S
************************************************************************/
#include <linux/types.h>
#include <uapi/ecnt_event_global/ecnt_event_system.h>
#ifdef TCSUPPORT_BOARD_SELECT
#include <linux/limits.h>
#endif

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
************************************************************************/
#define SERDES_CFG_MAJOR					(267)


#ifdef TCSUPPORT_BOARD_SELECT
#define BOOTARGS_SRDS_PORT_CNT_STR					("srdsPortCnt")
#define BOOTARGS_SRDS_PARM_PER_PORT_STR				("srdsParmCntPerPort")
#define BOOTARGS_SRDS_IF_NAME_ALL_COMP_STR			("srdsIfNameAllComp")
#define BOOTARGS_ETHER_TYPE_NAME_ALL_COMP_STR		("srdsEtherTypeAllComp")
#define BOOTARGS_ETHER_PHY_NAME_ALL_COMP_STR		("srdsEtherPhyTypeAllComp")
#define BOOTARGS_SRDS_PORT_COMBO_STR				("srdsPortCombo")
#define BOOTARGS_SRDS_PORT_NAME_ALL_COMP_STR		("srdsPortNameAllComp")
#define SRDS_PORT_NAME_PREFIX_STR					("Serdes-")
#define SRDS_ETHER_TYPE_PREFIX_STR					("Ether-")
#define SERDES_ETHER_PHY_STS_ERROR					(-1)
#define SERDES_IF_STS_ERROR							(U8_MAX)
#else
#ifdef TCSUPPORT_CPU_EN7581
#define BOOTARGS_SERDES_PON_SEL_STR			("serdes_pon")
#define BOOTARGS_SERDES_ETHER_SEL_STR		("serdes_ethernet")
#define BOOTARGS_SERDES_WIFI1_SEL_STR		("serdes_wifi1")
#define BOOTARGS_SERDES_WIFI2_SEL_STR		("serdes_wifi2")
#define BOOTARGS_SERDES_USB1_SEL_STR		("serdes_usb1")
#define BOOTARGS_SERDES_USB2_SEL_STR		("serdes_usb2")
#define SERDES_PORT_CNT						(6)
#elif defined(TCSUPPORT_CPU_AN7552)
#define BOOTARGS_SERDES_PON_SEL_STR			("serdes_pon")
#define SERDES_PORT_CNT						(1)
#elif defined(TCSUPPORT_CPU_EN7523)
#define BOOTARGS_SERDES_SEL_STR				("serdes_sel")
#define SERDES_PORT_CNT						(4)
#else
#define SERDES_PORT_CNT						(1)
#endif
#endif
/************************************************************************
*               M A C R O S
************************************************************************/

/************************************************************************
*               D A T A   T Y P E S
************************************************************************/

/************************************************************************
*               D A T A   D E C L A R A T I O N S
************************************************************************/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************/
#ifdef TCSUPPORT_BOARD_SELECT
uint8_t get_serdes_port_count(void);
ECNT_EVENT_SYSTEM_SERDES_ETHER_TYPE_SEL_t get_serdes_ether_type_sel(uint8_t port_idx);
uint8_t get_serdes_interface_sel(uint8_t port_idx);
ECNT_EVENT_SYSTEM_SERDES_PHY_SEL_t get_serdes_phy_sel(uint8_t port_idx);
const char *get_serdes_port_name(uint8_t port_idx);
const char *get_serdes_interface_name(uint8_t port_idx, uint8_t serdes_if);
#else
int get_serdes_port_count(void);
const char *get_serdes_port_name(ECNT_EVENT_SYSTEM_SERDES_SEL_t port_idx);
const char *get_serdes_interface_name(ECNT_EVENT_SYSTEM_SERDES_SEL_t port_idx, uint8_t serdes_if);
ECNT_EVENT_SYSTEM_SERDES_ETHER_TYPE_SEL_t get_serdes_ether_type_sel(ECNT_EVENT_SYSTEM_SERDES_SEL_t port_idx);
uint8_t get_serdes_interface_sel(ECNT_EVENT_SYSTEM_SERDES_SEL_t port_idx);
#endif
#endif/* __ECNT_EVENT_SERDES_H_ */

