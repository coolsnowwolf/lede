/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _FE_API_H
#define _FE_API_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <ecnt_hook/ecnt_hook_fe.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*               M A C R O S
*************************************************************************
*/
#define TC_DTC (1 << 5)
#define TOS_DTC (1 << 4)
#define VLAN_DTC (0xF)

#ifdef SUPPORT_SERDES_PON
#define PON_WAN_VALID 		((get_serdes_ether_type_sel(ECNT_EVENT_SERDES_SEL_PON) == ECNT_EVENT_SERDES_ETHER_TYPE_WAN) ? 1 : 0)
#else
#define PON_WAN_VALID 0
#endif

#ifdef SUPPORT_SERDES_USB
#define USB_WAN_VALID 		((get_serdes_ether_type_sel(ECNT_EVENT_SERDES_SEL_USB1) == ECNT_EVENT_SERDES_ETHER_TYPE_WAN) ? 1 : 0)
#else
#define USB_WAN_VALID 0
#endif

#ifdef SUPPORT_SERDES_PCIE0
#define PCIE0_WAN_VALID		((get_serdes_ether_type_sel(ECNT_EVENT_SERDES_SEL_WIFI1) == ECNT_EVENT_SERDES_ETHER_TYPE_WAN) ? 1 : 0)
#else
#define PCIE0_WAN_VALID 0
#endif

#ifdef SUPPORT_SERDES_PCIE1
#define PCIE1_WAN_VALID		((get_serdes_ether_type_sel(ECNT_EVENT_SERDES_SEL_WIFI2) == ECNT_EVENT_SERDES_ETHER_TYPE_WAN) ? 1 : 0)			
#else
#define PCIE1_WAN_VALID 0
#endif

#ifdef SUPPORT_SERDES_ETHER
#define ETH_WAN_VALID 		((get_serdes_ether_type_sel(ECNT_EVENT_SERDES_SEL_ETHER) == ECNT_EVENT_SERDES_ETHER_TYPE_WAN) ? 1 : 0)
#else
#define ETH_WAN_VALID 0
#endif

#define WAN_VALID                 PON_WAN_VALID || USB_WAN_VALID || PCIE0_WAN_VALID || PCIE1_WAN_VALID || ETH_WAN_VALID

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/	
unsigned int fe_reg_read(unsigned int reg_offset);
void fe_reg_write(unsigned int reg_offset, unsigned int value);
void fe_reg_modify_bits(unsigned int reg_offset, unsigned int Data, unsigned int Offset, unsigned int Len);
ecnt_ret_val ecnt_fe_api_hook(struct ecnt_data *in_data);
int fe_get_ok_cnt(int  gdm_sel, int txrx_sel);
int fe_do_core_reset_without_qdma(void);
int fe_do_core_reset_with_qdma(void);
int fe_core_soft_reset(void);
#ifdef TCSUPPORT_CPU_EN7580
int mbi_hang_unlock_by_aging(FE_Gdma_Sel_t idx, uint aging_sel);
int mbi_hang_unlock_by_terminate(FE_Gdma_Sel_t idx);
#endif

int  feChecConfigDone(uint reg, uint doneBit);

#endif /* _FE_API_H */
