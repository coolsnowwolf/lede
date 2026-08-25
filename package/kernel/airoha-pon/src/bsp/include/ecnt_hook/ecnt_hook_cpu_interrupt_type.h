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
#ifndef _ECNT_HOOK_CPU_INTERRUPT_TYPE_H_
#define _ECNT_HOOK_CPU_INTERRUPT_TYPE_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define IRQ_STRING_LEN_MAX  16

#define IRQ_NAME_UART           "uart"
#define IRQ_NAME_DRAM_PROTECT   "dram_protect"
#define IRQ_NAME_TIMER0         "timer0"
#define IRQ_NAME_TIMER1         "timer1"
#define IRQ_NAME_TIMER2         "timer2"
#define IRQ_NAME_WATCHDOG       "watchdog"
#define IRQ_NAME_GPIO           "gpio"
#define IRQ_NAME_PCM1           "pcm1"
#define IRQ_NAME_GDMA           "gdma"
#define IRQ_NAME_GIGA_SWITCH    "giga_switch"
#define IRQ_NAME_UART2          "uart2"
#define IRQ_NAME_USB            "usb"
#define IRQ_NAME_DYING_GASP     "dying_gasp"
#define IRQ_NAME_DMT            "dmt"
#define IRQ_NAME_QDMA_LAN0      "qdma_lan0"
#define IRQ_NAME_QDMA_LAN1      "qdma_lan1"
#define IRQ_NAME_QDMA_LAN2      "qdma_lan2"
#define IRQ_NAME_QDMA_LAN3      "qdma_lan3"
#define IRQ_NAME_QDMA_WAN0      "qdma_wan0"
#define IRQ_NAME_QDMA_WAN1      "qdma_wan1"
#define IRQ_NAME_QDMA_WAN2      "qdma_wan2"
#define IRQ_NAME_QDMA_WAN3      "qdma_wan3"
#define IRQ_NAME_PCIE0          "pcie0"
#define IRQ_NAME_PCIE1          "pcie1"
#define IRQ_NAME_PCIE_ERR       "pcie_err"
#define IRQ_NAME_XPON_MAC       "xpon_mac"
#define IRQ_NAME_XPON_PHY       "xpon_phy"
#define IRQ_NAME_CRYPTO         "crypto"
#define IRQ_NAME_PBUS_TIMEOUT   "pbus_timeout"
#define IRQ_NAME_PCM2           "pcm2"
#define IRQ_NAME_SPI            "spi"
#define IRQ_NAME_USB2           "usb2"
#define IRQ_NAME_FE_ERR         "fe_err"
#define IRQ_NAME_UART3          "uart3"
#define IRQ_NAME_UART4          "uart4"
#define IRQ_NAME_UART5          "uart5"
#define IRQ_NAME_HSDMA          "hsdma"
#define IRQ_NAME_XSI_MAC        "xsi_mac"
#define IRQ_NAME_XSI_PHY        "xsi_phy"
#define IRQ_NAME_WOE0           "woe0"
#define IRQ_NAME_WOE1           "woe1"
#define IRQ_NAME_WDMA0_P0       "wdma0_p0"
#define IRQ_NAME_WDMA0_P1       "wdma0_p1"
#define IRQ_NAME_WDMA0_WOE      "wdma0_woe"
#define IRQ_NAME_WDMA1_P0       "wdma1_p0"
#define IRQ_NAME_WDMA1_P1       "wdma1_p1"
#define IRQ_NAME_WDMA1_WOE      "wdma1_woe"
#ifdef TCSUPPORT_CPU_EN7528
#define IRQ_NAME_RBUS_TOUT      "rbus_tout"
#else
#define IRQ_NAME_EFUSE_ERR0     "efuse_err0"
#endif
#define IRQ_NAME_EFUSE_ERR1     "efuse_err1"

/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/
	typedef enum {
		CPU_INTERRUPT_GET_IRQNUM = 0,
        CPU_INTERRUPT_SHOW_INTERRUPTS,
        CPU_INTERRUPT_CHECK_INTRNAME,

		CPU_INTERRUPT_FUNCTION_MAX_NUM
	} CPU_Interrupt_HookFunctionID_t ;

	
	struct ecnt_cpu_interrupt_data
	{
		CPU_Interrupt_HookFunctionID_t function_id;
		int retValue;
		int irqNum;
        char irqString[IRQ_STRING_LEN_MAX];
	};

/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#endif /* _ECNT_HOOK_CPU_INTERRUPT_TYPE_H_ */

