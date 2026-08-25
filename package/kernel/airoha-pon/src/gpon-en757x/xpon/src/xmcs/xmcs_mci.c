/*
* File Name: xmcs_mci.c
* Description: Management control Interface for xPON Management 
*			  Control Subsystem
*
******************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/init.h>

#include "common/drv_global.h"
#include "xmcs/xmcs_mci.h"
#include "xmcs/xmcs_sdi.h"

static inline int should_iotcl_wait(void)
{
#ifndef TCSUPPORT_CPU_EN7521
	if(atomic_read(&fe_reset_flag) ) {
		return TRUE;
	}
#endif

    if(TRUE == gpPonSysData->ponMacPhyReset){
        return TRUE;
    }

    return FALSE;
}

/***************************************************************
***************************************************************/
long pon_mci_ioctl(struct file *filp, uint cmd, ulong arg) 
{
	int ret = -EINVAL ;

    while(TRUE == should_iotcl_wait() ){
        msleep(100) ;
    }

#ifdef TCSUPPORT_WAN_GPON
	if(_IOC_TYPE(cmd) == GPON_MAGIC) {
		ret = gpon_cmd_proc(cmd, arg) ;
	} else 
#endif /* TCSUPPORT_WAN_GPON */
#ifdef TCSUPPORT_WAN_EPON
	if(_IOC_TYPE(cmd) == EPON_MAGIC) {
		ret = epon_cmd_proc(cmd, arg) ;
	} else 
#endif /* TCSUPPORT_WAN_EPON */
	if(_IOC_TYPE(cmd) == INTERFACE_MAGIC) {
		ret = if_cmd_proc(cmd, arg) ;
	} else if(_IOC_TYPE(cmd) == FDET_MAGIC) {
		ret = fdet_cmd_proc(cmd, arg) ;
	} else if(_IOC_TYPE(cmd) == PHY_MAGIC) {
		ret = phy_cmd_proc(cmd, arg) ;
	} 
	
	return ret ;
}

/***************************************************************
***************************************************************/
struct file_operations xmci_fops = {
	.owner 			= THIS_MODULE,
	.unlocked_ioctl	= pon_mci_ioctl,
} ;


/***************************************************************
***************************************************************/
void pon_mci_destroy(void) 
{   
	dev_t devNo ;
		
	if(gpMcsPriv->pPonMciDev != NULL) {
		cdev_del(gpMcsPriv->pPonMciDev) ;
		gpMcsPriv->pPonMciDev = NULL ;
	}
	
	devNo = MKDEV(COSNT_XMCI_MAJOR_NUN, 0) ;
	unregister_chrdev_region(devNo, 1) ;
	
	PON_MSG(MSG_TRACE, "PON MCI destroy successful\n") ;
}

/***************************************************************
***************************************************************/
int pon_mci_init(void)
{
    int ret = 0 ;
    dev_t devNo ;
    
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

    return 0 ; 

out:
    pon_mci_destroy() ;
    return ret ;
}

