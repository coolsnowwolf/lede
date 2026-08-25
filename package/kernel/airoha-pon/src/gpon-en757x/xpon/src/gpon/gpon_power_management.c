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

/************************************************************************
*                  I N C L U D E S
************************************************************************/
#include <linux/proc_fs.h>

#include "common/drv_global.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
************************************************************************/

/************************************************************************
*                  M A C R O S
************************************************************************/

/************************************************************************
*                  D A T A	 T Y P E S
************************************************************************/

/************************************************************************
*                  E X T E R N A L	 D A T A   D E C L A R A T I O N S
************************************************************************/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
************************************************************************/

/************************************************************************
*                  P U B L I C   D A T A
************************************************************************/

/************************************************************************
*                  P R I V A T E   D A T A
************************************************************************/


/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
************************************************************************/

#ifdef TCSUPPORT_CPU_EN7521
int gpon_power_management_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;

	REG_SLEEP_CNT sleepCnt;

	sleepCnt.Raw = IO_GREG(SLEEP_CNT) ;

	index += sprintf(buf + index, "sleep count:%.8X\n", sleepCnt.Bits.sleep_cnt);
	CHK_BUF();

	*eof = 1;

done:
	*start = buf + (off - begin);
	index -= (off - begin);
	
	if (index < 0) {
		index = 0;
	}
	if (index > count) {
		index = count;
	}

	return index;
}

int gpon_power_management_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[64], cmd[32];
	int value, ret ;
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL ;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT ;

	sscanf(val_string, "%s %x", cmd, &value) ;

	if(!strcmp(cmd, "sleep_count")) {
		ret = modify_sleep_count(value);
		printk("Modify Sleep count:%.8X %s.\n", value, ((ret == 0) ? "success" : "fail"));
	} else if(!strcmp(cmd, "usage")) {
		printk("sleep_count <count (HEX)> \n");
	}

	return count ;
}

/******************************************************************************
 Descriptor:	It's used to modify sleep counter(BFB643A8).
 Input Args:	count: sleep counter and its unit is 125us.
 Ret Value:		none.
******************************************************************************/
int modify_sleep_count(uint count)
{
	REG_SLEEP_CNT sleepCnt;
	REG_SLEEP_GLB_CFG sleepGlbCfg;

	/* set sleep count */
	gpGponPriv->gponCfg.sleep_count = count;
	sleepCnt.Raw = IO_GREG(SLEEP_CNT);
	sleepCnt.Bits.sleep_cnt = count ;
	IO_SREG(SLEEP_CNT, sleepCnt.Raw) ;

	/* reload sleep count */
	sleepGlbCfg.Raw = IO_GREG(SLEEP_GLB_CFG);
	sleepGlbCfg.Bits.reload_sleep_cnt = 1;
	IO_SREG(SLEEP_GLB_CFG, sleepGlbCfg.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	power management initialization function.
 Input Args:	none.
 Ret Value:		none.
******************************************************************************/
int gpon_power_management_init(void)
{
	/* set default sleep count */
	modify_sleep_count(gpGponPriv->gponCfg.sleep_count);
	
	return 0;
}

/******************************************************************************
 Descriptor:	power management deinitialization function.
 Input Args:	none.
 Ret Value:		none.
******************************************************************************/
void xpon_power_management_deinit(void)
{
}

#endif

