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
*				   I N C L U D E S
*************************************************************************
*/
	

#include "xpon_fh.h"
#include "phy_if_wrapper.h"
#include "ecnt_hook/ecnt_hook.h"
#include "ecnt_hook/ecnt_hook_pon_customer.h"
#include "xpon_driver_global.h"


/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


/************************************************************************
*				   M A C R O S
*************************************************************************
*/

#define	VLAN_REMARKING		0x0FFF00
#define	PRI_REMARKING		0xE00000
#define	PRI_STORE			0xE
#define	PRI_REMARK_ENABLE	0x100000

/*skb->mark bit26~bit31*/
#define	DSCP_MASK			0xFC000000
#define	DSCP_REMARK_ENABLE	0x2000000

#define	FB_SPECIAL_VLAN	4093
#define	setVID(x) 		((x) & 0x0FFF)
#define	getVID(x)		((x) & 0x0FFF)
#define	VLAN_HEAD_LEN	4


/************************************************************************
*				   D A T A	 T Y P E S
*************************************************************************
*/



/************************************************************************
*				   E X T E R N A L	 D A T A   D E C L A R A T I O N S
*************************************************************************
*/


/************************************************************************
*				   F U N C T I O N	 D E C L A R A T I O N S
*************************************************************************
*/

/* calbacks. */
void skb_vlan_parser(struct sk_buff *skb);

int xmcs_add_vlan_to_list(unsigned short vlan1, unsigned short vlan2);
int xmcs_add_vlan_to_list(unsigned short vlan1, unsigned short vlan2);
int xmcs_del_vlan_from_list(unsigned short vlan1, unsigned short vlan2);
int xmcs_en_vlan_up_trans(unsigned short upTransEn);
void showTransVlanInfo(void);

int xmcs_get_omcc_info(struct XMCS_OMCC_Info_S *ptOmccInfo);
static int xmcs_set_i2c_ctrl_write(struct XMCS_PHY_I2cCtrl_S *pI2cCtrl);
static int xmcs_get_i2c_ctrl_read(struct XMCS_PHY_I2cCtrl_S *pI2cCtrl);
static int gpon_fh_debug_write_proc(struct file *file, const char *buffer, unsigned long count, void *data);
static int gpon_fh_debug_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
static int xpon_fh_proc_init(void);
int xpon_fh_proc_dest(void);
void phy_event_notify(XMCS_EventType_t type, uint id);
uint xmcs_event_check_valid(XMCS_EventType_t type, uint id, uint value);
int  xpon_set_emergence_state(unchar emergencystate);
int xpon_check_emergence_state(void);
unsigned char fh_tx_power_handle(unchar opt);
int  xpon_set_onu_id(ushort onuId);
int  xpon_set_omcc_id(uint omccId);
void set_phy_event_handler(int stateNotifyType);
static int register_callbacks(void);
long xpon_fh_ioctl(struct file *filp, uint cmd, ulong arg);
int xpon_fh_init(void);
void xpon_fh_destroy(void);

extern UINT16 SIF_X_Write(UINT8 u1CHannelID,UINT16 u2ClkDiv,UINT8 u1DevAddr,UINT8 u1WordAddrNum,UINT32 u4WordAddr,UINT8 * pu1Buf,UINT16 u2ByteCnt);
extern UINT16 SIF_X_Read(UINT8 u1CHannelID,UINT16 u2ClkDiv,UINT8 u1DevAddr,UINT8 u1WordAddrNum,UINT32 u4WordAddr,UINT8 * pu1Buf,UINT16 u2ByteCnt);

/************************************************************************
*				   P U B L I C	 D A T A
*************************************************************************
*/
	
struct file_operations fh_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl = xpon_fh_ioctl,
} ;


struct cdev *pPonFhDev ;

uint eponTxPowerFlag = 0;
unchar g_emergencystate = 0;
struct XMCS_OMCC_Info_S		omccInfo = {0};
static struct proc_dir_entry *gpon_fh_proc_dir = NULL;

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/


/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/


/************************************************************************
*                  I O C T L   D E F I N I T I O N S
*************************************************************************
*/

static inline int getVlanType(unsigned short TPID)
{
	switch(TPID)
	{
		case ETH_P_8021Q:
			return ETH_P_8021Q;
		case 0x88a8:
			return ETH_P_8021Q;
		case 0x9100:
			return ETH_P_8021Q;
		case 0xAABB:	  //only for test
			return ETH_P_8021Q;
		default:
			return -1;
	}
}


/*______________________________________________________________________________
**	function name
**		xpon_fh_ioctl
**	description:
**		xpon fh ioctl function.
**	parameters:
**		filp:
**		cmd:
**		arg:
**	global:
**		None
**	return:
**		0: scuess
**____________________________________________________________________________  
*/
long xpon_fh_ioctl(struct file *filp, uint cmd, ulong arg) 
{
	int ret = -EINVAL ; 
	
	if(_IOC_DIR(cmd) == _IOC_WRITE){
		switch(cmd) {	
			case PHY_IOS_I2C_CTRL_WRITE :
				ret = xmcs_set_i2c_ctrl_write((struct XMCS_PHY_I2cCtrl_S *)arg) ;
				break ;
			default:
				printk("No such I/O command, cmd: %x\n", cmd) ;
				break ;		
		}
	}else if(_IOC_DIR(cmd) == _IOC_READ){
		switch(cmd) {
			case IF_IOG_OMCC_INFO : 
				ret = xmcs_get_omcc_info((struct XMCS_OMCC_Info_S *)arg) ;
				break ;
			case PHY_IOG_I2C_CTRL_READ :
				ret = xmcs_get_i2c_ctrl_read((struct XMCS_PHY_I2cCtrl_S *)arg) ;
				break ;
			default:
				printk("No such I/O command, cmd: %x\n", cmd) ;
				break ;
		} 
	}
	
	return ret ;
}
/*______________________________________________________________________________
**	function name
**		xpon_fh_destroy
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
void xpon_fh_destroy(void) 
{	
	dev_t devNo ;
		
	if(pPonFhDev != NULL) {
		cdev_del(pPonFhDev) ;
		pPonFhDev = NULL ;
	}
	
	devNo = MKDEV(COSNT_XPON_FH_MAJOR_NUN, 0) ;
	unregister_chrdev_region(devNo, 1) ;

	printk("PON FH destroy successful\n") ;
}


/*______________________________________________________________________________
**	function name
**		xpon_fh_init
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
int xpon_fh_init(void)
{
	int ret = 0 ;
	dev_t devNo ;
	
	devNo = MKDEV(COSNT_XPON_FH_MAJOR_NUN, 0) ;
	ret = register_chrdev_region(devNo, 1, CONST_XPON_FH_DEV_NAME) ;
	if(ret < 0) {
		printk("Get the PON FH device number (register_chrdev_region()) failed\n") ;
		return ret ;
	}
	
	pPonFhDev = cdev_alloc() ;
	if(pPonFhDev == NULL) {
		printk("Alloc character device (cdev_alloc()) failed\n") ;
		ret = -ENOMEM ;
		goto out ;
	}
	
	cdev_init(pPonFhDev, &fh_fops) ;
	pPonFhDev->owner = THIS_MODULE ;
	pPonFhDev->ops = &fh_fops ;
	ret = cdev_add(pPonFhDev, devNo, 1) ;
	if(ret < 0) {
		printk( "PON FH driver register failed\n") ;
		goto out ;
	}
	printk("PON FH initialization successful\n") ;

	return ret ; 

out:
	xpon_fh_destroy() ;
	return ret ;
}



/***************************************************************
***************************************************************/
int xmcs_get_omcc_info(struct XMCS_OMCC_Info_S *ptOmccInfo)
{
	if(NULL == ptOmccInfo){
	    return -EINVAL ;
	}

	ptOmccInfo->allocId   = omccInfo.allocId;
	ptOmccInfo->gemportid = omccInfo.gemportid;

	return 0 ;
}
static int xmcs_set_i2c_ctrl_write(struct XMCS_PHY_I2cCtrl_S *pI2cCtrl) 
{
	unsigned char * ptr = NULL;
	unsigned char * ptrread = NULL;
	int ret = 0;
	int i = 0;
	unsigned int u4WordAddr = 0;
	if(NULL == pI2cCtrl){
		printk("input param error!\n");
		return -EINVAL;
	}

	if((pI2cCtrl->u2ByteCnt == 0) || (pI2cCtrl->u1WordAddrNum > 3)){
		printk("[%s] byte cnt or addr num error", __FUNCTION__);
		return -1;
	}
	ptr = (unsigned char *)kmalloc(pI2cCtrl->u2ByteCnt, GFP_KERNEL);
	if(ptr == NULL){
		printk("[%s] kmalloc fail!\n", __FUNCTION__);
		return -1;
	}
	for(i=0; i<pI2cCtrl->u2ByteCnt; i++){
		ptr[i] = pI2cCtrl->pu1Buf[i];
	}
	ret = SIF_X_Write(pI2cCtrl->u1CHannelID, pI2cCtrl->u2ClkDiv, pI2cCtrl->u1DevAddr, 
		pI2cCtrl->u1WordAddrNum, pI2cCtrl->u4WordAddr, ptr, pI2cCtrl->u2ByteCnt);
	if(ret < 0){
		printk("[%s] write error, code=%d", __FUNCTION__, ret);
		kfree(ptr);
		return -1;
	}

	ptrread = (unsigned char *)kmalloc(pI2cCtrl->u2ByteCnt, GFP_KERNEL);
	if(ptrread == NULL){
		printk("[%s] write verify read kmalloc fail!\n", __FUNCTION__);
		kfree(ptr);
		return -1;
	}
	ret = SIF_X_Read(pI2cCtrl->u1CHannelID, pI2cCtrl->u2ClkDiv, pI2cCtrl->u1DevAddr, 
		pI2cCtrl->u1WordAddrNum, pI2cCtrl->u4WordAddr, ptrread, pI2cCtrl->u2ByteCnt);
	if(ret < 0){
		printk("[%s] write verify read error, error code=%d", __FUNCTION__, ret);
		kfree(ptr);
		kfree(ptrread);
		return -1;
	}
	u4WordAddr = (unsigned int)pI2cCtrl->u4WordAddr;
	for(i=0; i<pI2cCtrl->u2ByteCnt; i++){
		if((i!=0) && ((i%8) == 0)){
			u4WordAddr = u4WordAddr + 4;
		}
		if(ptrread[i] != ptr[i])
			printk("\r\n xmcs_set_i2c_ctrl_write write verify read error!,dev 0x%x, Addr 0x%x, index %d, wirte 0x%x, read 0x%x\n ",
			pI2cCtrl->u1DevAddr, pI2cCtrl->u4WordAddr, i, ptr[i], ptrread[i]);
	}
	
	if(ptrread)
		kfree(ptrread);
	if(ptr)
		kfree(ptr);
	
	return 0;
}

/***************************************************************
***************************************************************/
static int xmcs_get_i2c_ctrl_read(struct XMCS_PHY_I2cCtrl_S *pI2cCtrl) 
{
	unsigned char * ptr = NULL;
	int ret = 0;
	int i = 0;
	unsigned int u4WordAddr = 0;
	if(NULL == pI2cCtrl){
		printk("input param error!\n");
		return -EINVAL;
	}

	ptr = (unsigned char *)kmalloc(pI2cCtrl->u2ByteCnt, GFP_KERNEL);
	if(ptr == NULL){
		printk("[%s] kmalloc fail!\n", __FUNCTION__);
		return -1;
	}
	ret = SIF_X_Read(pI2cCtrl->u1CHannelID, pI2cCtrl->u2ClkDiv, pI2cCtrl->u1DevAddr, 
		pI2cCtrl->u1WordAddrNum, pI2cCtrl->u4WordAddr, ptr, pI2cCtrl->u2ByteCnt);
	if(ret < 0){
		printk("[%s] read error, error code=%d", __FUNCTION__, ret);
		kfree(ptr);
		return -1;
	}
	u4WordAddr = (unsigned int)pI2cCtrl->u4WordAddr;
	for(i=0; i<pI2cCtrl->u2ByteCnt; i++){
		if((i!=0) && ((i%8) == 0)){
			u4WordAddr = u4WordAddr + 4;
		}

		pI2cCtrl->pu1Buf[i] = ptr[i];
	}
	if(ptr){
		kfree(ptr);
		printk("[%s]bbb\n", __FUNCTION__);
	}
	return 0 ;
}
static int gpon_fh_debug_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[64] = {0};

	if (count > sizeof(val_string) - 1)
		return -EINVAL ;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT ;

	return count ;
}
static int gpon_fh_debug_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;
	return index ;
}

static int xpon_fh_proc_init(void)
{
	struct proc_dir_entry *temp_proc = NULL;
	if(gpon_fh_proc_dir){
		return 0;
	}

	gpon_fh_proc_dir=proc_mkdir("gpon_fh", NULL);
	if(NULL == gpon_fh_proc_dir){
		return -1;
	}

	temp_proc = create_proc_entry("debug", 0, gpon_fh_proc_dir) ;
	if(NULL == temp_proc) {
		return -1;
	}
	temp_proc->read_proc = gpon_fh_debug_read_proc;
	temp_proc->write_proc = gpon_fh_debug_write_proc;
	return 0;
}

int xpon_fh_proc_dest(void){
	remove_proc_entry("debug", gpon_fh_proc_dir);
	remove_proc_entry("gpon_fh", NULL);
	
	return 0;
}

/************************************************************************
*                  I N T E R N A  L   D E F I N I T I O N S
*************************************************************************
*/

/************************************************************************
*                  C A L L B A C K S   D E F I N I T I O N S
*************************************************************************
*/
/*______________________________________________________________________________
**	function name
**		phy_event_notify
**	description:
**		callback for phy_event_notify.
**	parameters:
**		state: PHY state
**		emerg_state: emergence state.
**	global:
**		none
**	return:
**		void
**	call:
**		ECNT_HOOK
**	revision:
**		v1.0
**____________________________________________________________________________*/

void phy_event_notify(XMCS_EventType_t type, uint id)
{
	if(type == XMCS_EVENT_TYPE_GPON ){
		if((id == XMCS_EVENT_GPON_LOS)){
			if(xpon_check_emergence_state() == FALSE){
				ECNT_XPON_CUSTOMER_PHY_EVENT_HANDLER(PON_SET_PHY_NOTIFY_EVENT, PHY_XPON_STATE_NOTIFY_LOS);
			}
		}else if((id == XMCS_EVENT_GPON_DYING_GASP)){
				ECNT_XPON_CUSTOMER_PHY_EVENT_HANDLER(PON_SET_PHY_NOTIFY_EVENT, PHY_XPON_STATE_NOTIFY_DYINGGASP);
		}else if((id == XMCS_EVENT_GPON_PHY_READY)){
			if(xpon_check_emergence_state() == FALSE){
				ECNT_XPON_CUSTOMER_PHY_EVENT_HANDLER(PON_SET_PHY_NOTIFY_EVENT, PHY_XPON_STATE_NOTIFY_SD);
			}
		}else{
			printk("can not support the event id!\n");
			return;
		}
	}else if(type == XMCS_EVENT_TYPE_EPON){
		if((id == XMCS_EVENT_EPON_LOS)){
			if(xpon_check_emergence_state() == FALSE){
				ECNT_XPON_CUSTOMER_PHY_EVENT_HANDLER(PON_SET_PHY_NOTIFY_EVENT, PHY_XPON_STATE_NOTIFY_LOS);
			}
		}else if((id == XMCS_EVENT_EPON_DYING_GASP)){
				ECNT_XPON_CUSTOMER_PHY_EVENT_HANDLER(PON_SET_PHY_NOTIFY_EVENT, PHY_XPON_STATE_NOTIFY_DYINGGASP);
		}else if((id == XMCS_EVENT_EPON_PHY_READY)){
			if(xpon_check_emergence_state() == FALSE){
				ECNT_XPON_CUSTOMER_PHY_EVENT_HANDLER(PON_SET_PHY_NOTIFY_EVENT, PHY_XPON_STATE_NOTIFY_SD);
			}
		}else{
			printk("can not support the event id!\n");
			return;
		}
	}else{
		printk("can not support the event type!\n");
		return;
	}
}
uint xmcs_event_check_valid(XMCS_EventType_t type, uint id, uint value)
{
	if(xpon_check_emergence_state() == TRUE){
		if((XMCS_EVENT_TYPE_GPON == type)
		&& (XMCS_EVENT_GPON_STATE_CHANGE == id)
		&& (GPON_STATE_O7 == value)){	/* GPON in emergence state just report O7 */
			printk("Now Gpon in emergence state just report O7!\n") ;
		}else if(XMCS_EVENT_TYPE_EPON == type){	/* EPON in tx power off ,just report id replace by XMCS_EVENT_EPON_TX_POWER_OFF */
			printk("Now Gpon in emergence state or Epon in Tx Power Off return!\n") ;
			if(XMCS_EVENT_EPON_DEREGISTER != id)
				return FALSE;
			else
				return TRUE;
		}else{
			printk("Now is in emergence state just return!\n");
			return FALSE;
		}
	}
	return TRUE;
}

unsigned char fh_tx_power_handle(unchar opt)
{
	static uint tx_power_last = 0;
	if(PHY_EPON_CONFIG == XPON_PHY_GET(PON_GET_PHY_MODE)){
		eponTxPowerFlag = opt;
		if(opt == XPON_PHY_TX_POWER_OFF){
			ECNT_API_XPON_EVENT_REPORT(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_TX_POWER_OFF);
			tx_power_last = opt;
		}else if(opt == XPON_PHY_TX_POWER_ON){
			if (XPON_PHY_GET(PON_GET_PHY_LOS_STATUS) == PHY_LOS_HAPPEN){
				if(xpon_check_emergence_state() == FALSE){
					ECNT_XPON_CUSTOMER_PHY_EVENT_HANDLER(PON_SET_PHY_NOTIFY_EVENT, PHY_XPON_STATE_NOTIFY_LOS);
				}
				ECNT_API_XPON_EVENT_REPORT(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_LOS) ;
			}
			
			if(opt != tx_power_last){
				ECNT_API_XPON_RESET();
				tx_power_last = opt;
			}
		}else{
			printk("Error operation %u\n", opt);
		}
	}else if(PHY_GPON_CONFIG == XPON_PHY_GET(PON_GET_PHY_MODE)){
		printk("phy set tx power %d ,GPON do nothing\n", opt);
	}
	return 0;
}

int xpon_set_emergence_state(unchar emergencystate)
{
	g_emergencystate = emergencystate;

	return 0;
}
int xpon_check_emergence_state(void)
{
	if(PHY_GPON_CONFIG == XPON_PHY_GET(PON_GET_PHY_MODE)){
		if(g_emergencystate == XPON_ENABLE){
			printk("Now Gpon in emergence_state return TRUE\n") ;
			return TRUE;
		}
		else
			return FALSE;
	}else if(PHY_EPON_CONFIG == XPON_PHY_GET(PON_GET_PHY_MODE)){
		if(eponTxPowerFlag == XPON_PHY_TX_POWER_OFF){
			printk("Now Epon in tx power off return TRUE\n") ;
			return TRUE;
		}
		else
			return FALSE;
	}
	return FALSE;
}

int  xpon_set_onu_id(ushort onuId)
{
	omccInfo.allocId = onuId;

	return 0;
}
int  xpon_set_omcc_id(uint omccId)
{
	omccInfo.gemportid = omccId;

	return 0;
}

void set_phy_event_handler(int stateNotifyType)
{
	printk("phy_xpon_state_notify_handler not implement!\n");
	return;
}

static int register_callbacks(void)
{
	int ret = 0;
	XPON_CALLBACK_Info_t info;

	ECNT_XPON_CUSTOMER_CALLBACK_RESET();

	info.callback_id = ECNT_XPON_CALLBACK_MULTICAST_RX_DATA;
	info.callback_fn = NULL;	/* this function is from customer. */
	ret = ECNT_XPON_CUSTOMER_CALLBACK_REGISTER(&info);
	if(ret == ECNT_HOOK_ERROR){
		printk("callback ECNT_XPON_CALLBACK_MULTICAST_RX_DATA register failed!\n");
	}
	
	info.callback_id = ECNT_XPON_CALLBACK_PHY_EVENT;
	info.callback_fn = phy_event_notify;
	ret = ECNT_XPON_CUSTOMER_CALLBACK_REGISTER(&info);
	if(ECNT_HOOK_ERROR == ret){
		printk("callback ECNT_XPON_CALLBACK_PHY_EVENT register failed\n");
	}
	
	info.callback_id = ECNT_XPON_CALLBACK_XMCS_CHECK_EVENT_VALID;
	info.callback_fn = xmcs_event_check_valid;
	ret = ECNT_XPON_CUSTOMER_CALLBACK_REGISTER(&info);
	if(ECNT_HOOK_ERROR == ret){
		printk("callback ECNT_XPON_CALLBACK_XMCS_CHECK_EVENT_VALID register failed\n");
	}
	info.callback_id = ECNT_XPON_CALLBACK_SET_EMERGENCE_STATE;
	info.callback_fn = xpon_set_emergence_state;
	ret = ECNT_XPON_CUSTOMER_CALLBACK_REGISTER(&info);
	if(ECNT_HOOK_ERROR == ret){
		printk("callback ECNT_XPON_CALLBACK_SET_EMERGENCE_STATE register failed\n");
	}
	info.callback_id = ECNT_XPON_CALLBACK_FH_TX_POWER_HANDLE;
	info.callback_fn = fh_tx_power_handle;
	ret = ECNT_XPON_CUSTOMER_CALLBACK_REGISTER(&info);
	if(ECNT_HOOK_ERROR == ret){
		printk("callback ECNT_XPON_CALLBACK_FH_TX_POWER_HANDLE register failed\n");
	}
	
	info.callback_id = ECNT_XPON_CALLBACK_SET_ONU_ID;
	info.callback_fn = xpon_set_omcc_id;
	ret = ECNT_XPON_CUSTOMER_CALLBACK_REGISTER(&info);
	if(ECNT_HOOK_ERROR == ret){
		printk("callback ECNT_XPON_CALLBACK_SET_ONU_ID register failed\n");
	}

	info.callback_id = ECNT_XPON_CALLBACK_SET_OMCC_ID;
	info.callback_fn = xpon_set_onu_id;
	ret = ECNT_XPON_CUSTOMER_CALLBACK_REGISTER(&info);
	if(ECNT_HOOK_ERROR == ret){
		printk("callback ECNT_XPON_CALLBACK_SET_OMCC_ID register failed\n");
	}

	info.callback_id = ECNT_XPON_CALLBACK_SET_PHY_EVENT_HANDLER;
	info.callback_fn = set_phy_event_handler;
	ret = ECNT_XPON_CUSTOMER_CALLBACK_REGISTER(&info);
	if(ECNT_HOOK_ERROR == ret){
		printk("callback ECNT_XPON_CALLBACK_NOTIFY_HANDLER register failed\n");
	}
	
	return ret;
}

/******************************************************************************
******************************************************************************/
static void __exit xpon_customer_module_cleanup(void) 
{
	printk("xpon_customer_module_clean up!\n");
	ECNT_XPON_CUSTOMER_CALLBACK_RESET();
	xpon_fh_proc_dest();
	xpon_fh_destroy();
	return;
}

/******************************************************************************
******************************************************************************/
static int __init xpon_customer_module_init(void)
{
	printk("xpon_customer_module_init\n");
	register_callbacks();
	
	xpon_fh_init();
	xpon_fh_proc_init();
	return 0 ;
}


module_init(xpon_customer_module_init);
module_exit(xpon_customer_module_cleanup);


