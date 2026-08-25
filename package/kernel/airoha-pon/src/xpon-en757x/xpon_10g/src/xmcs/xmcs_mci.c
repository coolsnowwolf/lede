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

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include "common/xpon_global.h"
#include "xmcs/xmcs_mci.h"
#include "xmcs/xmcs_epon.h"

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
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
long pon_mci_ioctl(struct file *filp, uint cmd, ulong arg);
/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
struct file_operations xmci_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl = pon_mci_ioctl,
#ifdef TCSUPPORT_CPU_ARMV8_64
	.compat_ioctl	= pon_mci_ioctl,
#endif
} ;
/***************************************************************
***************************************************************/
/*______________________________________________________________________________
**	function name
**		pon_mci_ioctl
**	description:
**		PON MCI ioctl function.
**	parameters:
**		filp:
**		cmd:
**		arg:
**	global:
**		None
**	return:
**		0: scuess
**	call:
**		gpon_10g_cmd_proc
**		epon_cmd_proc
**		if_cmd_proc
**		fdet_cmd_proc
**		gpon_cmd_proc
**		phy_cmd_proc
**		PON_MSG
**	revision:
**		v1.0
**____________________________________________________________________________*/
long pon_mci_ioctl(struct file *filp, uint cmd, ulong arg) 
{
	int ret = -EINVAL ; 

	if(_IOC_TYPE(cmd) == GPON_10G_MAGIC) {
		ret = gpon_10g_cmd_proc(cmd, arg) ;
	}else if(_IOC_TYPE(cmd) == EPON_MAGIC) {
		ret = epon_cmd_proc(cmd, arg) ;
	} else if(_IOC_TYPE(cmd) == INTERFACE_MAGIC) {
		ret = if_cmd_proc(cmd, arg) ;
	}else if(_IOC_TYPE(cmd) == FDET_MAGIC) {
		ret = fdet_cmd_proc(cmd, arg) ;
	}else if(_IOC_TYPE(cmd) == GPON_MAGIC) {
	    ret = gpon_cmd_proc(cmd, arg) ;
	}else if(_IOC_TYPE(cmd) == FDET_MAGIC) {
		ret = 0;
	}else if(_IOC_TYPE(cmd) == PHY_MAGIC) {
		ret = phy_cmd_proc(cmd, arg) ;
	}else if(_IOC_TYPE(cmd) == RDKB_MAGIC){
		ret = rdkb_10g_cmd_proc(cmd, arg) ;
	}else {
	    PON_MSG(MSG_ERR, "pon mci ioctl fail\n") ;
    }
	return ret ;
}
/*______________________________________________________________________________
**	function name
**		xpon_mci_destroy
**	description:
**		Destory the mci(pon) interface.
**	parameters:
**		None
**	global:
**		gpMcsPriv
**	return:
**		0: scuess
**	call:
**		PON_MSG
**	revision:
**		v1.0
**____________________________________________________________________________*/
void xpon_mci_destroy(void) 
{	
	dev_t devNo;
		
	if(gpMcsPriv->pPonMciDev != NULL) {
		cdev_del(gpMcsPriv->pPonMciDev) ;
		gpMcsPriv->pPonMciDev = NULL ;
	}
	
	devNo = MKDEV(COSNT_XMCI_MAJOR_NUN, 0) ;
	unregister_chrdev_region(devNo, 1) ;
	
	PON_MSG(MSG_TRACE, "PON MCI destroy successful\n") ;
}
/*______________________________________________________________________________
**	function name
**		xpon_mci_init
**	description:
**		Create  the mci(pon) interface.
**	parameters:
**		None
**	global:
**		gpMcsPriv
**	return:
**		0: scuess
**		-x: failure
**	call:
**		PON_MSG
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xpon_mci_init(void)
{
	int ret = 0;
	dev_t devNo;
	
	devNo = MKDEV(COSNT_XMCI_MAJOR_NUN, 0) ;
	
	ret = register_chrdev_region(devNo, 1, CONST_XMCI_DEV_NAME) ;
	if(ret < 0) {
		PON_MSG(MSG_ERR, "Get the PON MCI device number (register_chrdev_region()) failed\n") ;
		return ret ;
	}
	
	gpMcsPriv->pPonMciDev = cdev_alloc() ;
	if(gpMcsPriv->pPonMciDev == NULL) {
		PON_MSG(MSG_ERR, "Alloc character device (cdev_alloc()) failed\n") ;
		ret = -ENOMEM ;
		goto out ;
	}
	
	cdev_init(gpMcsPriv->pPonMciDev, &xmci_fops) ;
	gpMcsPriv->pPonMciDev->owner = THIS_MODULE ;
	gpMcsPriv->pPonMciDev->ops = &xmci_fops ;
	ret = cdev_add(gpMcsPriv->pPonMciDev, devNo, 1) ;
	if(ret < 0) {
		PON_MSG(MSG_ERR, "PON MCI driver register failed\n") ;
		goto out ;
	} 

    /* init XMCS wait queue */
    init_waitqueue_head(&gpMcsPriv->xmcsWaitQueue) ;
    memset(&gpMcsPriv->xmcsEventStatus, 0, sizeof(struct XMCS_PonEventStatus_S)) ;
    gpMcsPriv->ctrlFlag.report_init_O1= 0;
    
    spin_lock_init(&gpMcsPriv->fdetLock) ;

    PON_MSG(MSG_TRACE, "PON MCI initialization successful\n") ;

	return ret ; 

out:
	xpon_mci_destroy() ;
	return ret ;
}
