#include <linux/module.h>
#include <linux/kernel.h>

#include "epon/epon.h"
#include "epon/epon_reg.h"
#include "epon/epon_act.h"
#include "epon/epon_dev.h"
#include "common/xpon_global.h"


/*______________________________________________________________________________
**	function name
**		get_onu_mac_address
**	description:
**		get onu mac from flash
**	parameters:
**		macAddr
**	global:
**		flash_base
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/


int get_onu_mac_address(__u8 *macAddr){

	char flashMacAddr[6];
	int i;
	
	for (i=0; i<6; i++) {
		flashMacAddr[i] = READ_FLASH_BYTE(flash_base + 0xff48 + i);
	}

	if( (flashMacAddr[0] == 0) && (flashMacAddr[1] == 0) && (flashMacAddr[2] == 0) &&
	    (flashMacAddr[3] == 0) && (flashMacAddr[4] == 0) && (flashMacAddr[5] == 0) )
		printk(KERN_INFO "The MAC address in flash is null!\n");	    
	else    
  		memcpy(macAddr, flashMacAddr, 6);  	
	return 0;
}

/*______________________________________________________________________________
**	function name
**		epon_check_mac_cfg
**	description:
**		check epon mac cfg
**	parameters:
**		None
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/


int epon_check_mac_cfg(void)
{
	REG_e_mac_addr_cfg eponMacAddrCfgReg;
	__u32 timers = 0;

	while(timers < 100){
		eponMacAddrCfgReg.Raw = READ_REG_WORD(e_mac_addr_cfg);
		if(eponMacAddrCfgReg.Bits.mac_addr_rwcmd_done == 0){
			break;
		}
		timers++;
	}

	if(timers == 100){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "ERR: eponMacSetMacAddr timers == 100");
		return -1;
	}
	return 0;
}

/*______________________________________________________________________________
**	function name
**		epon_set_llid_regs_mac_address
**	description:
**		set mac addr to reg
**	parameters:
**		addr_index
**		llid_index
**		addr_val
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_set_llid_regs_mac_address(__u8 addr_index,__u8 llid_index, __u32 addr_val)
{
	REG_e_mac_addr_cfg eponMacAddrCfgReg ;
    REG_e_mac_addr_value mac_addr;

    eponMacAddrCfgReg.Raw = 0;
    mac_addr.Raw = 0;

	mac_addr.Bits.mac_addr_value =  addr_val;
	WRITE_REG_WORD(e_mac_addr_value , mac_addr.Raw);
    
	EPON_MAC_ADDR_CFG_OPERATE(eponMacAddrCfgReg,addr_index,llid_index,EPON_REGS_WRITE);
	//check done bit is ready
	EPON_CHECK_MAC_ADDR_CFG_DONE;

    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_get_llid_regs_mac_address
**	description:
**		get mac addr from reg
**	parameters:
**		addr_index
**		llid_index
**		addr_val
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_get_llid_regs_mac_address(__u8 addr_index,__u8 llid_index, __u32 *addr_val)
{
	REG_e_mac_addr_cfg eponMacAddrCfgReg;

	eponMacAddrCfgReg.Raw = 0;

	EPON_MAC_ADDR_CFG_OPERATE(eponMacAddrCfgReg,addr_index,llid_index,EPON_REGS_READ);
    
	EPON_CHECK_MAC_ADDR_CFG_DONE;
    
    *addr_val = READ_REG_WORD(e_mac_addr_value);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_set_mac_address
**	description:
**		set mac addr to mac temp
**	parameters:
**		llidEntry_p
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		epon_get32
**		epon_get16
**		epon_set_llid_regs_mac_address
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_set_mac_address(EPON_LLID_INFO_T *llidEntry_p)
{
    __u32 mac_addr_low = 0;
    __u32 mac_addr_high  = 0;
    int ret = 0;

	DEBUG_MSG_EPON(MSG_LEVEL_TRACE,"enter epon_set_mac_address");

	if(llidEntry_p == NULL){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERR: eponMacSetMacAddr llidEntry_p == NULL");
		return EPON_INPUT_POINT_NULL;
	}

	EPON_CHECK_MAC_ADDR_CFG_DONE;

    mac_addr_low  = epon_get32(llidEntry_p->macAddr+2);
    mac_addr_high = epon_get16(llidEntry_p->macAddr);

    ret = epon_set_llid_regs_mac_address(EPON_ADDR_REG_LOW,llidEntry_p->llidIndex,mac_addr_low);
    ret |= epon_set_llid_regs_mac_address(EPON_ADDR_REG_HIGH,llidEntry_p->llidIndex,mac_addr_high);
    if(ret != EPON_SUCCESS)
    {
    	DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: %s fail",__FUNCTION__);
        return EPON_NORMAL_ERROR;
    }
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG," epon_set_mac_address %x%x success",mac_addr_high,mac_addr_low);
	return EPON_SUCCESS;
}

/*______________________________________________________________________________
**	function name
**		epon_get_mac_address
**	description:
**		get mac addr from mac temp
**	parameters:
**		llidEntry_p
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		epon_get_llid_regs_mac_address
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_get_mac_address(EPON_LLID_INFO_T *llidEntry_p)
{
	int ret = 0;
    __u32 mac_addr_low = 0;
    __u32 mac_addr_high  = 0;

	DEBUG_MSG_EPON(MSG_LEVEL_TRACE,"enter epon_set_mac_address");

	if(llidEntry_p == NULL){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERR: eponMacSetMacAddr llidEntry_p == NULL");
		return EPON_INPUT_POINT_NULL;
	}

	EPON_CHECK_MAC_ADDR_CFG_DONE;

    ret = epon_get_llid_regs_mac_address(EPON_ADDR_REG_LOW,llidEntry_p->llidIndex,&mac_addr_low);
    ret |= epon_get_llid_regs_mac_address(EPON_ADDR_REG_HIGH,llidEntry_p->llidIndex,&mac_addr_high);
    if(ret != EPON_SUCCESS)
    {
    	DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: %s epon_get_llid_regs_mac_address fail",__FUNCTION__);
        return EPON_NORMAL_ERROR;
    }
	printk("eponMacSetMacAddr get 1 macLow == %8X", mac_addr_low);
    printk("eponMacSetMacAddr get 2 macHigh == %8X", mac_addr_high);
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacSetMacAddr get 1 macLow == %8X", mac_addr_low);
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacSetMacAddr get 2 macHigh == %8X", mac_addr_high);
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"MAC Addr in HW is %x:%x:%x:%x:%x:%x",
        (mac_addr_high>>8)&0xff, mac_addr_high&0xff, (mac_addr_low>>24)&0xff,
        (mac_addr_low>>16)&0xff,(mac_addr_low>>8)&0xff,(mac_addr_low)&0xff);
	
	return EPON_SUCCESS;
}

/*______________________________________________________________________________
**	function name
**		epon_dev_set_tx_burst_mode
**	description:
**		set tx brust mode
**	parameters:
**		mode
**	global:
**		None
**	return:
**		None
**	call:
**		IO_CBITS
**          IO_SBITS
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_dev_set_tx_burst_mode(__u8 mode)
{
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s begin",__FUNCTION__);
	if ( isEPONFWID )
	{
		if (TRUE == mode)
		{
			/* set burst mode */
			IO_CBITS(PHY_CSR_DUMMY_REG_RX,(1<<21));	
            DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s set burst mode",__FUNCTION__);
		}
		else if (FALSE == mode)
		{
			/* set continue mode */
			IO_SBITS(PHY_CSR_DUMMY_REG_RX,(1<<21));	
            DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s set continue mode",__FUNCTION__);
		}
	}	
	return;
}
/*______________________________________________________________________________
**	function name
**		epon_dev_interrupt_mask_enable
**	description:
**		enable interrupt
**	parameters:
**		None
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_dev_interrupt_mask_enable(void)
{
	__u32 raw = 0;
	__u32 m_raw = 0;
	WRITE_REG_WORD(e_int_sts3 , 0xffffffff);//clear int before enable int
	m_raw = READ_REG_WORD(e_int_en3);
	WRITE_REG_WORD(e_int_en2 , 0x1fff);/*enable int2 for mac error*/
	WRITE_REG_WORD(e_int_status , 0xffffffff);//clear int before enable int
	raw = READ_REG_WORD(e_int_en);
	

	EPON_SET_BIT(raw,RPT_OVERINTVL_INT);
    EPON_SET_BIT(raw,MPCP_TIMEOUT_INT);
    EPON_SET_BIT(raw,TIMEDRFT_INT);
    EPON_SET_BIT(raw,TOD_1PPS_INT);
    EPON_SET_BIT(raw,TOD_UPDT_INT);
    EPON_SET_BIT(raw,GNT_BUF_OVRRUN_INT);
    
	EPON_SET_BIT(raw,SEND_REGISTER_ACK_INT);
	EPON_SET_BIT(raw,SEND_REGISTER_REQ_INT);
	EPON_SET_BIT(raw,RCV_DSCVRY_GATE_INT);

	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_INT_ENABLE,NULL,&raw);

    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s  raw is %x ",__FUNCTION__,raw);
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s  m_raw is %x",__FUNCTION__,m_raw);
    WRITE_REG_WORD(e_int_en , raw);
	
}
/*______________________________________________________________________________
**	function name
**		epon_dev_check_mpi_mbi_stop_polling
**	description:
**		check mac mbi/mpi stop or not
**	parameters:
**		None
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_dev_check_mpi_mbi_stop_polling(void)
{
    int cnt = POIING_TX_STOP_STAS_NUM;
    while(cnt > 0)
    {
        if(UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_CHECK_MAC_TX_MBI_STOP, NULL, NULL)
			&& UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_CHECK_MAC_TX_MPI_STOP, NULL, NULL)) 
		{
            return;
        }else {
            cnt--;
        }
    }
    return;
}

/*______________________________________________________________________________
**	function name
**		epon_dev_tx_rx_disable
**	description:
**		disable mac tx rx
**	parameters:
**		None
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		epon_dev_check_mpi_mbi_stop_polling
**	revision:
**		v1.0
**____________________________________________________________________________*/


int epon_dev_tx_rx_disable(void)
{
	__u32 Raw = READ_REG_WORD(e_glb_cfg);
	
	Raw |= (RXMBI_STOP|TXMBI_STOP|TXMPI_STOP|RXMPI_STOP);
	WRITE_REG_WORD(e_glb_cfg,Raw);
    epon_dev_check_mpi_mbi_stop_polling();
	return 0;
}

/*______________________________________________________________________________
**	function name
**		epon_dev_set_hw_discv_states
**	description:
**		set hw status
**	parameters:
**		llid_index
**		state
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_dev_set_hw_discv_states(__u8 llid_index,__u8 state)
{
	REG_e_llid0_dscvry_sts sdcvSts;	
	if(llid_index < 32) 
	{
		sdcvSts.Raw = READ_REG_WORD(g_epon_llid_dscv_stat[llid_index]);
		sdcvSts.Bits.llid0_dscvry_sts = state;
		WRITE_REG_WORD(g_epon_llid_dscv_stat[llid_index], sdcvSts.Raw); 
	}else {
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "ERR: %s fail llidIndex=%d", __FUNCTION__, llid_index);
		return;
	}
	      
}
/*______________________________________________________________________________
**	function name
**		epon_dev_send_mpcp_msg
**	description:
**		send mpcp msg
**	parameters:
**		send_msg_info
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_dev_send_mpcp_msg(MPCP_SEND_INFO_T *send_msg_info)
{
	REG_e_llid_dscvry_ctrl dscvCtrl;
 	dscvCtrl.Raw = 0;

	dscvCtrl.Bits.tx_mpcp_llid_idx = send_msg_info->llid_index;
	dscvCtrl.Bits.rgstr_req_flg = send_msg_info->register_req_flag;
    dscvCtrl.Bits.rgstr_ack_flg	= send_msg_info->register_ack_flag;
	dscvCtrl.Bits.mpcp_cmd = send_msg_info->msg_type;
	WRITE_REG_WORD(e_llid_dscvry_ctrl, dscvCtrl.Raw); 
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: %s end  llid %d, ack flag %d mpcp_cmd %d value is %x",
        __FUNCTION__,send_msg_info->llid_index,dscvCtrl.Bits.rgstr_ack_flg,dscvCtrl.Bits.mpcp_cmd,dscvCtrl.Raw);   
}

