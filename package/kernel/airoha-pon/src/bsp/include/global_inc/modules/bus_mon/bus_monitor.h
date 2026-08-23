#ifndef __LINUX_BUS_MONITOR_H__
#define __LINUX_BUS_MONITOR_H__

/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°Airoha Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to Airoha Limited (¡°Airoha¡±) and/or 
its licensors. Without the prior written permission of Airoha and/or its licensors, 
any reproduction, modification, use or disclosure of Airoha Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

Airoha Limited. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°AIROHA SOFTWARE¡±) RECEIVED FROM AIROHA 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES AIROHA PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE AIROHA SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE.

AIROHA SHALL NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
AIROHA'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AIROHA 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT AIROHA'S SOLE OPTION, TO 
REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO AIROHA FOR SUCH 
AIROHA SOFTWARE.
***************************************************************/

#define REG_SIZE	32


typedef struct {
	unsigned int mon_idx;		
	unsigned int master_id;
	unsigned int  addr_max;
	unsigned int  addr_min;
	int wr_sel;
} pbus_mon_t;
	
typedef struct {
	unsigned long long mon_acc_cnt;
	unsigned long long mon_acc_prd;
	unsigned int mon_acc_max;
	unsigned long long mon_acc_total;
} pbus_mon_cnt_t;

typedef struct {
	unsigned int mon_idx;		
	unsigned int master_id;
} rbus_mon_t;

typedef struct {
	unsigned int mon_idx;
	unsigned long long mon_vld_cnt;
	unsigned long long mon_idle_cnt;
} rbus_mon_cnt_t;

typedef union
{
	pbus_mon_cnt_t pbus_cnt;
	rbus_mon_cnt_t rbus_cnt;
}mon_cnt_t;


enum HOST_PBUS_PORT
{
	P_CPU,
	P_GDMA,
	P_SPI,
	P_CRYPTO,
	P_WOE0,
	P_WOE1,
	P_NPU,
	P_THERM,
	P_I2C2RBUS,
	P_EMMC,
	P_NPU_PCIE0,
	P_NPU_PCIE1,
	P_NPU_PCIE2
};

enum HOST_RBUS_PORT
{
	R_PCM,
	R_GDMA,
	R_USB_P0,
	R_PCIE1,
	R_DFD,
	R_SPI,
	R_EMMC,
	R_CRYPTO,
	R_XPON,
	R_USB_P1,
	R_PCI2,
	R_WDMA1,
	R_WOE0,
	R_WOE1,
	R_WDMA0,
	R_HSDMA,
	R_FEPPE,
	R_FEPDMA,
	R_QDMA,
	R_FETDMA,
	R_NPU,
	R_PCIE0,
	R_CPU
};

#endif


