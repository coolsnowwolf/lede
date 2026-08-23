/***************************************************************
Copyright Statement:

This software/firmware and related documentation (��AIROHA Software��) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to AIROHA Limited (��AIROHA��) and/or 
its licensors. Without the prior written permission of AIROHA and/or its licensors, 
any reproduction, modification, use or disclosure of AIROHA Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

AIROHA Limited  AIROHA. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (��AIROHA SOFTWARE��) RECEIVED FROM AIROHA 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ��AS IS�� 
BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES AIROHA PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE AIROHA SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER��S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE.

AIROHA SHALL NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES 
MADE TO RECEIVER��S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
AIROHA'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AIROHA 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT AIROHA'S SOLE OPTION, TO 
REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO AIROHA FOR SUCH 
AIROHA SOFTWARE.
***************************************************************/

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <asm/tc3162/tc3162.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <ecnt_event_global/ecnt_event_board.h>

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*                  STATIC VARIABLE DECLARATIONS
*************************************************************************
*/
static char rfb_no;
static RFB_ID_t rfb_id;
/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

static int __init early_rfb_no(char *p)
{

	int ret;

	ret = sscanf(p, "%d", &rfb_no);
	if(ret != 1) {
		printk("parse %s error:%d\n", BOOTARGS_RFB_NO_STR, ret);
		return -1;
	}

	return 0;
}
early_param(BOOTARGS_RFB_NO_STR, early_rfb_no);


static int __init early_rfb_id1(char *p)
{
	int ret;

	ret = sscanf(p, "%02lx", &rfb_id.id1);
	if(ret != 1) {
		printk("parse %s error:%d\n", BOOTARGS_RFB_ID1_STR, ret);
		return -1;
	}

	return 0;
}
early_param(BOOTARGS_RFB_ID1_STR, early_rfb_id1);

static int __init early_rfb_id2(char *p)
{

	int ret;

	ret = sscanf(p, "%02lx", &rfb_id.id2);
	if(ret != 1) {
		printk("parse %s error:%d\n", BOOTARGS_RFB_ID2_STR, ret);
		return -1;
	}

	return 0;
}
early_param(BOOTARGS_RFB_ID2_STR, early_rfb_id2);

void get_rfb_id(RFB_ID_t *_rfb_id)
{
	_rfb_id->id1 = rfb_id.id1;
	_rfb_id->id2 = rfb_id.id2;
}
EXPORT_SYMBOL(get_rfb_id);

long boardCfgIoctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	boardCfgIoctl_t __boardCfg;
	boardCfgIoctl_t *userCfg = (boardCfgIoctl_t *)arg;
	RFB_ID_t rfb_id;
	
	if (_IOC_TYPE(cmd) != BOARD_CFG_IOC_MAGIC) {
		return -ENOTTY;
	}

	if ((_IOC_DIR(cmd) & _IOC_READ) || 
		(_IOC_DIR(cmd) & _IOC_WRITE)) {
		ret = !access_ok( (void __user *)userCfg, _IOC_SIZE(cmd));
		if (ret) {
			printk("_IOC_READ or _IOC_WRITE check error.\n");
			return -EFAULT;
		}
	}

	if(copy_from_user(&__boardCfg, userCfg, sizeof(boardCfgIoctl_t))) {
		printk("copy_from_user failed\n");
		return -EFAULT;
	}

	//printk("__boardCfg.port_idx:%d\n", __boardCfg.port_idx);
	//printk("__boardCfg.rfb_no:%d\n", __boardCfg.rfb_no);

	switch(cmd)
	{
		case BOARD_CFG_IOCTL_GET_RFB_NO:
			get_rfb_id(&rfb_id);
			ret = copy_to_user((void __user *)&userCfg->rfb_id, &rfb_id, sizeof(RFB_ID_t));
			if(ret) {
				printk("copy rfb id error.\n");
			}
			
			//printk("BOARD_CFG_IOCTL_GET_RFB_NO env_rfb_no:%d\n", env_rfb_no);
			break;
		default:
			printk("unknown cmd:%d.\n", cmd);
			return -EFAULT;
	}
	return ret;
}

static struct file_operations boardCfgFops = {
	.owner =			THIS_MODULE,
	.write =			NULL,
	.read =				NULL,
	.unlocked_ioctl =	boardCfgIoctl,	
#ifdef TCSUPPORT_CPU_ARMV8_64
	.compat_ioctl	= 	boardCfgIoctl,
#endif
	.open =				NULL,
	.release =			NULL,
};

static int ECNT_BOARD_CFG_PROBE(void)
{
	int ret = 0;
	
	ret = register_chrdev(BOARD_CFG_MAJOR, "board_cfg", &boardCfgFops);
	if (ret < 0) {
		printk("board cfg create ioctl fail\n");
		return ret;
    }

	return 0;
}

subsys_initcall(ECNT_BOARD_CFG_PROBE);

