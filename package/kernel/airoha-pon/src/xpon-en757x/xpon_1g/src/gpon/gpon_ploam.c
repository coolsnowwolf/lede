/*
* File Name: gpon_ploam.c
* Description: PLOAM message related function for GPON
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/random.h>

#include "common/drv_global.h"
#include "common/phy_if_wrapper.h"
#include "gpon/gpon.h"

#include "xmcs/xmcs_sdi.h"
#include "gpon/gpon_power_management.h"
#include "common/xpon_daemon.h"
#include "gpon/gpon_qos.h"

#ifdef TCSUPPORT_CPU_EN7521
#include <ecnt_hook/ecnt_hook_fe.h>
#endif 

#ifdef TCSUPPORT_AUTOBENCH
#include <linux/proc_fs.h>
#endif
#include <linux/time.h>

#if defined(TCSUPPORT_XPON_LED)
#include <asm/tc3162/ledcetrl.h>
#endif
#include "common/xpon_led.h"


extern int deallocate_flag;
extern Omci_Oam_Monitor_t * gpMonitor;
extern int typeB_delay_g;

/******************************************************************************
******************************************************************************/
void gpon_init_qdma_tx_buff(void){
    QDMA_TxBufCtrl_T txBufCtrl;

	txBufCtrl.mode = QDMA_ENABLE;
	txBufCtrl.chnThreshold = 4;
	txBufCtrl.totalThreshold = 0xe0;

	QDMA_API_SET_TXBUF_THRESHOLD(ECNT_QDMA_WAN, &txBufCtrl);

    return;
}

void ploam_eqd_adjustment(uint newEqd) 
{
	REG_G_EQD gponEqd ;
	int deltaEqd = newEqd - gpGponPriv->gponCfg.eqd ;
	int K, A, B, a, intByteDelay ;
#ifdef TCSUPPORT_CPU_EN7521
	REG_DBG_TX_SYNC_OFFSET txSyncOffset;
#else
	REG_DBG_PROBE_LOW32 gponDebugProbe ;
#endif	
#ifdef TCSUPPORT_CUC
    if(gpGponPriv->gponCfg.dis_ranging_in_o5) {
        if(deltaEqd > 16 || deltaEqd < -16) {
            PON_MSG(MSG_OAM, "Delta Eqd is bigger than 16!\n");
            return;
        }
    }
#endif
	if(gpGponPriv->gponCfg.eqdO5Offset == 0) {
		gpGponPriv->gponCfg.eqd = newEqd ;
	} else {
		/* For O5 EqD offset test */
		if(gpGponPriv->gponCfg.flags.eqdOffsetFlag == GPON_EQD_OFFSET_FLAG_SUBTRACT) {
			deltaEqd -= gpGponPriv->gponCfg.eqdO5Offset ;
		} else {
			deltaEqd += gpGponPriv->gponCfg.eqdO5Offset ;
		}
	}

#ifdef TCSUPPORT_CPU_EN7521
	/* Get MAC internal byte delay*/
	txSyncOffset.Raw = IO_GREG(DBG_TX_SYNC_OFFSET) ;
	intByteDelay = txSyncOffset.Bits.dbg_tx_sync_offset ;
#else
	/* Get MAC internal byte delay from debug probe */				
	IO_SREG(DBG_PROBE_CTRL, 0x0000000C) ;
	gponDebugProbe.Raw = IO_GREG(DBG_PROBE_LOW32) ;
	intByteDelay = (gponDebugProbe.Raw>>23) & 3 ;
#endif
	
	if(deltaEqd > 0) {
		K = deltaEqd + gpGponPriv->gponCfg.bitDelay ;
		A = K >> 3 ;
		B = K & 7 ;
		//a = (((intByteDelay+(A&3)) <= 3) ? 0 : 8) + (A>>2) - (A&3) ;
		a=(((intByteDelay + (A&3)) <= 3) ? 0 : 8) + (A - (A&3)) - (A&3);
		gpGponPriv->gponCfg.byteDelay += (a<<3) ;
		gpGponPriv->gponCfg.bitDelay = B ;
	} else if(deltaEqd < 0) {
		K = 0 - deltaEqd - gpGponPriv->gponCfg.bitDelay ;
		A = (K>0) ? ((K>>3)+((K&7)?1:0)) : 0 ;
		B = (A<<3) - K ; 
		//a = ((intByteDelay >= (A&3)) ? 0 : 8) + (A>>2) - (A&3) ;
		a=((intByteDelay >= (A&3)) ? 0 : 8) + (A - (A&3)) - (A&3);
		gpGponPriv->gponCfg.byteDelay -= (a<<3) ;
		gpGponPriv->gponCfg.bitDelay = B ;
	} else {
		return ;
	}

	if(gpGponPriv->gponCfg.eqdO5Offset != 0) {
		/* For O5 EqD offset test */
		PON_MSG(MSG_OAM, "PLOAM: main path EqD in O5, %x, deltaEqd = %x.\n", gpGponPriv->gponCfg.eqd, deltaEqd) ;
		gpGponPriv->gponCfg.eqdO5Offset = 0;
	}
	PON_MSG(MSG_OAM|MSG_ACT|MSG_TYPEB, "PLOAM: Setting the main path EqD in O5, ByteDelay:%x, BitDelay:%x. (A:%x, B:%x, byte_dly:%x, a:%x)\n", gpGponPriv->gponCfg.byteDelay, gpGponPriv->gponCfg.bitDelay, A, B, intByteDelay, a) ;
	
	/* Setting the EqD to MAC register */
	gponEqd.Raw = IO_GREG(G_EQD) ;
	gponEqd.Bits.eqd = gpGponPriv->gponCfg.byteDelay ;
	IO_SREG(G_EQD, gponEqd.Raw) ;
	XPON_PHY_SET_BIT_DELAY(gpGponPriv->gponCfg.bitDelay);
}

/******************************************************************************
******************************************************************************/
int ploam_parser_down_message(PLOAM_RAW_General_T *pGenPloamMsg)
{
	int ret = -EFAULT ;
	uint msgId = pGenPloamMsg->raw.msg_id ;
	ploam_recv_handler_t ploam_recv_handler ;
	
	if(msgId>=PLOAM_DOWN_MAX_TYPE || gpGponPriv->ploamRecvHandler[msgId]==NULL) {
		PON_MSG((MSG_ERR|MSG_OAM), "PLOAM: receive unknow message.(%.8X %.8X %.8X)\n", htonl(pGenPloamMsg->value[0]), htonl(pGenPloamMsg->value[1]),htonl(pGenPloamMsg->value[2])) ;
		gpon_set_alarmBit(MEM_INDEX);
		return -EFAULT ;
	}
	
	ploam_recv_handler = gpGponPriv->ploamRecvHandler[msgId] ;
	ret = ploam_recv_handler(pGenPloamMsg) ;

	return ret ;
}


/******************************************************************************
******************************************************************************/
static int ploam_send_passwd_msg(unchar *passwd, unchar len)
{
	PLOAM_RAW_Password_T pdMsg ;
		
	memset(&pdMsg, 0, sizeof(PLOAM_RAW_Password_T)) ;
	pdMsg.raw.dest_id = GPON_ONU_ID ;
	pdMsg.raw.msg_id = PLOAM_UP_MSG_PASSWORD_MESSAGE ;
	memcpy(pdMsg.raw.passwd, passwd,  len) ;

	/* Set the MAC registedr to send PLOAM message */
	PON_MSG(MSG_OAM, "PLOAM: Send password message.(%.8X %.8X %.8X)\n", htonl(pdMsg.value[0]), htonl(pdMsg.value[1]), htonl(pdMsg.value[2])) ; 
	gponDevSendPloamMsg((PLOAM_RAW_General_T *)&pdMsg, 3) ;
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int ploam_send_dying_gasp(void)
{
	PLOAM_RAW_Dying_Gasp_T dgMsg ;
	gp_dying_info->dying_flag		  = TRUE;
	memset(&dgMsg, 0, sizeof(PLOAM_RAW_Dying_Gasp_T)) ;
	dgMsg.raw.dest_id = GPON_ONU_ID ;
	dgMsg.raw.msg_id = PLOAM_UP_MSG_DYING_GASP ;
	
	/* Set the MAC registedr to send PLOAM message */
	PON_MSG(MSG_OAM, "PLOAM: Send dying gasp message.(%.8X %.8X %.8X)\n", htonl(dgMsg.value[0]), htonl(dgMsg.value[1]), htonl(dgMsg.value[2])) ; 
	gponDevSendPloamMsg((PLOAM_RAW_General_T *)&dgMsg, 3) ;

	return 0 ;
}


/******************************************************************************
******************************************************************************/
static int ploam_send_encryption_key(unchar key_idx, unchar *key)
{
	PLOAM_RAW_Encryption_Key_T encryMsg ;
	int i ;

	memset(&encryMsg, 0, sizeof(PLOAM_RAW_Encryption_Key_T)) ;
	encryMsg.raw.dest_id = GPON_ONU_ID ;
	encryMsg.raw.msg_id = PLOAM_UP_MSG_ENCRYPTION_KEY ;
	for(i=0 ; i<2 ; i++) {
		encryMsg.raw.key_idx = key_idx ;
		encryMsg.raw.frag_idx = i ;
		memcpy(encryMsg.raw.key, key+(i*8),  8) ;
		
		/* Set the MAC registedr to send PLOAM message */
		PON_MSG(MSG_OAM, "PLOAM: Send encryption key message.(%.8X %.8X %.8X)\n", htonl(encryMsg.value[0]), htonl(encryMsg.value[1]), htonl(encryMsg.value[2])) ; 
		gponDevSendPloamMsg((PLOAM_RAW_General_T *)&encryMsg, 3) ;
	}
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int ploam_send_pee_msg(void)
{
	PLOAM_RAW_PEE_T peeMsg ;
	
	memset(&peeMsg, 0, sizeof(PLOAM_RAW_PEE_T)) ;
	peeMsg.raw.dest_id = GPON_ONU_ID ;
	peeMsg.raw.msg_id = PLOAM_UP_MSG_PEE_MESSAGE ;

	/* Set the MAC registedr to send PLOAM message */
	PON_MSG(MSG_OAM, "PLOAM: Send PEE message.(%.8X %.8X %.8X)\n", htonl(peeMsg.value[0]), htonl(peeMsg.value[1]), htonl(peeMsg.value[2])) ; 
	gponDevSendPloamMsg((PLOAM_RAW_General_T *)&peeMsg, 1) ;

	return 0 ;
}

/******************************************************************************
******************************************************************************/
int ploam_send_pst_msg(unchar line_num, unchar k1_ctl, unchar k2_ctl)
{
	PLOAM_RAW_PST_T pstMsg ;
	
	memset(&pstMsg, 0, sizeof(PLOAM_RAW_PST_T)) ;
	pstMsg.raw.dest_id = GPON_ONU_ID ;
	pstMsg.raw.msg_id = PLOAM_UP_MSG_PST_MESSAGE ;
	pstMsg.raw.line_num = line_num ;
	pstMsg.raw.k1_ctrl = k1_ctl ;
	pstMsg.raw.k2_ctrl = k2_ctl ;

	/* Set the MAC registedr to send PLOAM message */
	PON_MSG(MSG_OAM, "PLOAM: Send PST message.(%.8X %.8X %.8X)\n", htonl(pstMsg.value[0]), htonl(pstMsg.value[1]), htonl(pstMsg.value[2])) ; 
	gponDevSendPloamMsg((PLOAM_RAW_General_T *)&pstMsg, 1) ;

	return 0 ;
}

/******************************************************************************
******************************************************************************/
int ploam_send_rei_msg(uint err_count, unchar *seq_num_p)
{
	PLOAM_RAW_REI_T reiMsg ;
	
	memset(&reiMsg, 0, sizeof(PLOAM_RAW_REI_T)) ;
	reiMsg.raw.dest_id = GPON_ONU_ID ;
	reiMsg.raw.msg_id = PLOAM_UP_MSG_REI_MESSAGE ;
	reiMsg.raw.counter[0] = err_count>>24 ;
	reiMsg.raw.counter[1] = err_count>>16 ;
	reiMsg.raw.counter[2] = err_count>>8 ;
	reiMsg.raw.counter[3] = err_count ;
	reiMsg.raw.seq_num = *seq_num_p ;

	/* Set the MAC registedr to send PLOAM message */
	PON_MSG(MSG_OAM, "PLOAM: Send REI message.(%.8X %.8X %.8X)\n", htonl(reiMsg.value[0]), htonl(reiMsg.value[1]), htonl(reiMsg.value[2])) ; 
	gponDevSendPloamMsg((PLOAM_RAW_General_T *)&reiMsg, 1) ;
	
	*seq_num_p = (*seq_num_p+1) & 0xF ;

	return 0 ;
}


/******************************************************************************
******************************************************************************/
static int ploam_send_acknowledge_msg(unchar dm_id, unchar *msg)
{
	PLOAM_RAW_Acknowledge_T ackMsg ;

	PON_MSG(MSG_OAM, "PLOAM: Send acknowledge PLOAM message.\n") ;
	
	ackMsg.raw.dest_id = GPON_ONU_ID ;
	ackMsg.raw.msg_id = PLOAM_UP_MSG_ACKNOWLEDGE ;
	ackMsg.raw.dm_id = dm_id ;
	memcpy(ackMsg.raw.dm_byte, msg, 9) ;
	
	/* Set the MAC registedr to send PLOAM message */
	PON_MSG(MSG_OAM, "PLOAM: Send acknowledge message.(%.8X %.8X %.8X)\n", htonl(ackMsg.value[0]), htonl(ackMsg.value[1]), htonl(ackMsg.value[2])) ; 
	gponDevSendPloamMsg((PLOAM_RAW_General_T *)&ackMsg, CONFIG_SEND_ACK_PLOAMU) ;
	
	return 0 ;
}

/******************************************************************************
 Descriptor:	To send sleep_request PLOAM.
 Input Args:	sleepMode: 0: Doze.
                                  1: Sleep.
                                  2: WSleep.
 Ret Value:		0: Success.
******************************************************************************/
int ploam_send_sleep_request_msg(GPON_PLOAMu_SLEEP_MODE_t sleepMode)
{
	PLOAM_RAW_Sleep_Request_T sleepRequestMsg ;
	
	memset(&sleepRequestMsg, 0, sizeof(PLOAM_RAW_Sleep_Request_T)) ;
	sleepRequestMsg.raw.dest_id = GPON_ONU_ID ;
	sleepRequestMsg.raw.msg_id = PLOAM_UP_MSG_SLEEP_REQUEST_MESSAGE ;
	sleepRequestMsg.raw.sleep_mode = sleepMode ;

	/* Set the MAC registedr to send PLOAM message */
	PON_MSG(MSG_OAM, "PLOAM: Send Sleep_Request message.(%.8X %.8X %.8X)\n", htonl(sleepRequestMsg.value[0]), htonl(sleepRequestMsg.value[1]), htonl(sleepRequestMsg.value[2])) ; 
	gponDevSendPloamMsg((PLOAM_RAW_General_T *)&sleepRequestMsg, 1) ;
	
	return 0 ;
}

#define PHY_TX_EN_PATTERN (0xAA)

/******************************************************************************
******************************************************************************/
#ifndef TCSUPPORT_CPU_EN7523
void modify_mac_internal_delay(void)
{
    UINT32 raw = 0;
    UINT32 rx_dly = 0;
    REG_DBG_DLY dbgDly;
    
    IO_SREG(DBG_PROBE_CTRL, 0xf) ;
    raw = IO_GREG(DBG_PROBE_HIGH32);
    rx_dly = (raw & 0x00fff000)>>12;
    PON_MSG(MSG_OAM|MSG_ACT, "set [%x] = 0xf, get [%x]= %x, rx delay = %x\n", (__u32)DBG_PROBE_CTRL, (__u32)DBG_PROBE_HIGH32, raw, rx_dly) ;

    dbgDly.Raw = IO_GREG(DBG_DLY);
    raw = dbgDly.Raw;
    dbgDly.Bits.phy_rx_dly_sel = 1;
    dbgDly.Bits.fix_phy_rx_dly = rx_dly/2;
    // if set onuResponseTime > 0x577 = 36000ns, OMCI Report 35999ns;
    // if set onuResponseTime <= 0x577 = 36000ns, OMCI Report Real reponse time by api function get from driver register;
    if((gpGponPriv->gponCfg.onuResponseTime - 0x577) > 0)
        dbgDly.Bits.fix_phy_rx_dly += 4*(gpGponPriv->gponCfg.onuResponseTime - 0x577);      //why 4, because laser(2*) and   omci report (2*)
    PON_MSG(MSG_OAM|MSG_ACT, "get [%x] = %x, set [%x]= %x\n", (__u32)DBG_DLY, raw, (__u32)DBG_DLY, dbgDly.Raw) ;
    IO_SREG(DBG_DLY, dbgDly.Raw) ;
}
#endif
static int ploam_recv_upstream_overhead(PLOAM_RAW_Upstream_Overhead_T *pUpOverheadMsg)
{
	REG_G_PLOu_GUARD_BIT gponGuardBit ;
	REG_G_PLOu_PRMBL_TYPE1_2 gponPrmblType ;
	REG_G_PRE_ASSIGNED_DLY gponPreAssignDly ;
    PHY_GPON_Delimiter_Guard_t pat = {0} ;
    uint phyRegData;
	PHY_GponPreb_T pon_preb;
	PPHY_GponPreb_T p_pon_preb = & pon_preb;

	PON_MSG(MSG_OAM, "PLOAM: Rx Upstream_Overhead.(%.8X %.8X %.8X),state O%d\n", 
            htonl(pUpOverheadMsg->value[0]), 
            htonl(pUpOverheadMsg->value[1]), 
            htonl(pUpOverheadMsg->value[2]),
            GPON_CURR_STATE) ; 
	//PON_MSG(MSG_OAM, "PLOAM: Current State O%d\n", GPON_CURR_STATE) ; 

	if(pUpOverheadMsg->raw.dest_id != PLOAM_BROADCAST_ADDR) {
		PON_MSG((MSG_OAM|MSG_ERR), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pUpOverheadMsg->raw.dest_id) ;
		return -1 ;
	}

	if(GPON_CURR_STATE == GPON_STATE_O2) {

		/*enable transceiver power*/
		if (!gpGponPriv->emergencystate && gpPonSysData->ponRogueStatus != PON_STATUS_ROGUE)
        	XPON_PHY_TX_ENABLE();
		
		p_pon_preb->mask = PHY_GUARD_BIT_NUM_EN |  PHY_PRE_T1_NUM_EN | PHY_PRE_T2_NUM_EN | PHY_PRE_T3_PAT_EN;
		p_pon_preb->guard_bit_num = gpGponPriv->gponCfg.phy_guard_bit_num;
		p_pon_preb->preamble_t1_num = pUpOverheadMsg->raw.t2_pbits;
		p_pon_preb->preamble_t2_num = pUpOverheadMsg->raw.t1_pbits;
		p_pon_preb->preamble_t3_pat = pUpOverheadMsg->raw.t3_pbits;
        XPON_PHY_SET_API(PON_SET_PHY_GPON_PREAMBLE, p_pon_preb);
		gpGponPriv->gponCfg.preamble_t3_pat = pUpOverheadMsg->raw.t3_pbits;
		phyRegData = ((pUpOverheadMsg->raw.delimiter[0]<<16) | (pUpOverheadMsg->raw.delimiter[1]<<8) | (pUpOverheadMsg->raw.delimiter[2]<<0)) ;

        pat.delimiter  = phyRegData;
        pat.guard_time = PHY_TX_EN_PATTERN;
        XPON_PHY_SET_API(PON_SET_PHY_GPON_DELIMITER_GUARD, &pat);

		/* Setting the MAC register */
		gponGuardBit.Raw = IO_GREG(G_PLOu_GUARD_BIT) ;
		gponGuardBit.Bits.guard_bit = gpGponPriv->gponCfg.phy_guard_bit_num;
		IO_SREG(G_PLOu_GUARD_BIT, gponGuardBit.Raw) ;
        gpGponPriv->gponCfg.gponGuardBit.Raw = gponGuardBit.Raw;

		gponPrmblType.Raw = IO_GREG(G_PLOu_PRMBL_TYPE1_2) ;
		gponPrmblType.Bits.prmb1_bit = pUpOverheadMsg->raw.t1_pbits ;
		gponPrmblType.Bits.prmb2_bit = pUpOverheadMsg->raw.t2_pbits ;
		IO_SREG(G_PLOu_PRMBL_TYPE1_2, gponPrmblType.Raw) ;
        gpGponPriv->gponCfg.gponPrmblType.Raw = gponPrmblType.Raw;

		gponPreAssignDly.Raw = IO_GREG(G_PRE_ASSIGNED_DLY) ;
		gponPreAssignDly.Bits.pre_dly_en = pUpOverheadMsg->raw.delay_mode ;
		gponPreAssignDly.Bits.pre_dly = ((pUpOverheadMsg->raw.delay_time[0]<<8) | pUpOverheadMsg->raw.delay_time[1]) ;
		IO_SREG(G_PRE_ASSIGNED_DLY, gponPreAssignDly.Raw) ;
        gpGponPriv->gponCfg.gponPreAssignDly.Raw = gponPreAssignDly.Raw;

        gpGponPriv->gponCfg.gponDelmBit.Raw  = IO_GREG(G_PLOu_DELM_BIT);
		gpGponPriv->gponCfg.tx_power = pUpOverheadMsg->raw.tx_power;
		
#ifndef TCSUPPORT_CPU_EN7523
        modify_mac_internal_delay();
#endif

		/* Change the current state */
		gpon_act_change_gpon_state(GPON_STATE_O3) ;
	}
    

	return 0 ;
}


/******************************************************************************
******************************************************************************/
static int ploam_recv_assign_onu_id(PLOAM_RAW_Assign_OnuID_T *pAssignOnuIdMsg)
{
	REG_G_ONU_ID gponOnuId ;
	
	PON_MSG(MSG_OAM, "PLOAM: Receive Assign_ONU_ID message.(%.8X %.8X %.8X)\n", htonl(pAssignOnuIdMsg->value[0]), htonl(pAssignOnuIdMsg->value[1]), htonl(pAssignOnuIdMsg->value[2])) ; 

	if(gpPhyData->working_mode == PHY_GPON_SYM_CONFIG){
		PON_MSG(MSG_OAM, "PLOAM: Receive Assign_ONU_ID message,dest id is 0x%.8X.\n", pAssignOnuIdMsg->raw.dest_id)
		if((pAssignOnuIdMsg->raw.dest_id != PLOAM_BROADCAST_ADDR)&&(pAssignOnuIdMsg->raw.dest_id != PLOAM_SYM_MODE_BROADCAST_ADDR)) {
			PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pAssignOnuIdMsg->raw.dest_id) ;
			return -1 ;
		}
	} else {
		if(pAssignOnuIdMsg->raw.dest_id != PLOAM_BROADCAST_ADDR) {
			PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pAssignOnuIdMsg->raw.dest_id) ;
			return -1 ;
		}
	}	
	/* Compare the serial number of system and Assign_ONU-ID PLOAM message */
	if(memcmp(gpGponPriv->gponCfg.sn, pAssignOnuIdMsg->raw.sn, 8)) {
		PON_MSG((MSG_ERR|MSG_OAM), "The serial number of PLOAM message is incorrect\n") ;
		return -1 ;
	}

	if(GPON_CURR_STATE == GPON_STATE_O3) {
		/* Change the current state */
		/* modify for CIG IOT testing */
		gpon_act_change_gpon_state(GPON_STATE_O4) ;

		/* Setting the ONU ID to MAC register */
		gponOnuId.Raw = IO_GREG(G_ONU_ID) ;
		gponOnuId.Bits.onu_id_vld = 1 ;
		gponOnuId.Bits.onu_id = pAssignOnuIdMsg->raw.onu_id ;
		IO_SREG(G_ONU_ID, gponOnuId.Raw) ;

		/* Store the latest ONU ID (GPON_ONU_ID)*/
		gpGponPriv->gponCfg.onu_id = pAssignOnuIdMsg->raw.onu_id ;
		gpWanPriv->gpon.allocId[0] = gpGponPriv->gponCfg.onu_id;
		gpWanPriv->activeChannelNum = 1;

        /*initial qdma tx buff*/
        gpon_init_qdma_tx_buff();
	}
	
	return 0 ;
}

#ifdef TCSUPPORT_AUTOBENCH
int gpon_slt_test =0;
EXPORT_SYMBOL(gpon_slt_test);
#endif
#ifdef CONFIG_USE_FOR_TEST
uint eqdDbgFlag = 0;
uint EqdCounter = 0;
uint EqdArray[EQD_ARRAY_MAX_NUM] = {0};
#endif /* CONFIG_USE_FOR_TEST */

/******************************************************************************
******************************************************************************/
static int ploam_recv_ranging_time(PLOAM_RAW_Ranging_Time_T *pRangingMsg)
{
    REG_G_EQD gponEqd ;
    REG_G_GBL_CFG gponGlbCfg ;
    uint newEqd ;
	REG_DBG_GRP_0 dbgGrp0Status;
	REG_DBG_GRP_1 dbgGrp1Status;
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
	REG_G_MBI_MPI_STOP gponDelStop ;
#endif	
#ifdef TCSUPPORT_CPU_EN7521
	REG_G_AES_KEY_SWITCH_BY_SW aesKeySwitchBySw;
	memset(&aesKeySwitchBySw, 0, sizeof(REG_G_AES_KEY_SWITCH_BY_SW));
#endif
    
    PON_MSG(MSG_OAM, "PLOAM: Receive Ranging_Time message.(%.8X %.8X %.8X)\n", htonl(pRangingMsg->value[0]), htonl(pRangingMsg->value[1]), htonl(pRangingMsg->value[2])) ; 

    if(pRangingMsg->raw.dest_id != GPON_ONU_ID) {
        PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pRangingMsg->raw.dest_id) ;
        return -1 ;
    }

    if((GPON_CURR_STATE == GPON_STATE_O4) || (GPON_CURR_STATE == GPON_STATE_O5)) {
        /* Setting the EqD to MAC register */
        if(pRangingMsg->raw.eqd_type == 0) {
            newEqd = ((pRangingMsg->raw.delay[0]<<24) | (pRangingMsg->raw.delay[1]<<16) | (pRangingMsg->raw.delay[2]<<8) | (pRangingMsg->raw.delay[3])) ;   
            PON_MSG((MSG_OAM|MSG_EQD|MSG_ACT), "PLOAM: Receive the main path EqD, %x.\n", newEqd) ;
#ifdef CONFIG_USE_FOR_TEST
			if(eqdDbgFlag){
				EqdArray[EqdCounter] = newEqd;	
				EqdCounter++;
				if(EqdCounter == EQD_ARRAY_MAX_NUM)
					eqdDbgFlag = 0;
			}
#endif /* CONFIG_USE_FOR_TEST */
            if(GPON_CURR_STATE == GPON_STATE_O4) {
                gpGponPriv->gponCfg.eqd = newEqd ; /* record the eqd value before adjust the offset */
        
                /* Adjust the eqd value for eqd debug */
                if(gpGponPriv->gponCfg.eqdO4Offset) {
                    if(gpGponPriv->gponCfg.flags.eqdOffsetFlag == GPON_EQD_OFFSET_FLAG_SUBTRACT) {
                        newEqd -= gpGponPriv->gponCfg.eqdO4Offset ;
                        PON_MSG(MSG_OAM, "PLOAM: Adjust the main path EqD in O4, Offset:%x.\n", gpGponPriv->gponCfg.eqdO4Offset) ;
                    } else {
                        newEqd += gpGponPriv->gponCfg.eqdO4Offset ;
                        PON_MSG(MSG_OAM, "PLOAM: Adjust the main path EqD in O4, Offset:%x.\n", gpGponPriv->gponCfg.eqdO4Offset) ;
                    }
                }
                
                gpGponPriv->gponCfg.byteDelay = newEqd & BYTE_DELAY_MASK ;
                gpGponPriv->gponCfg.bitDelay = newEqd & BIT_DELAY_MASK ;
                PON_MSG(MSG_OAM|MSG_ACT|MSG_TYPEB, "PLOAM: Setting the main path EqD in O4, ByteDelay:%x, BitDelay:%x.\n", gpGponPriv->gponCfg.byteDelay, gpGponPriv->gponCfg.bitDelay) ;
                /* Setting the EqD to MAC register */
                gponEqd.Raw = IO_GREG(G_EQD) ;
                gponEqd.Bits.eqd = gpGponPriv->gponCfg.byteDelay ;
                IO_SREG(G_EQD, gponEqd.Raw) ;

                XPON_PHY_SET_BIT_DELAY(gpGponPriv->gponCfg.bitDelay);
                
            } else if(GPON_CURR_STATE == GPON_STATE_O5) {
                ploam_eqd_adjustment(newEqd) ;
            }
        } else if (pRangingMsg->raw.eqd_type == 1) {
            PON_MSG(MSG_OAM, "PLOAM: Setting the protection path EqD.\n") ;
        }
    }

    if(GPON_CURR_STATE == GPON_STATE_O4) {
        /* Enable the MAC tx FEC */
        gponGlbCfg.Raw = IO_GREG(G_GBL_CFG) ;
        gponGlbCfg.Bits.us_fec_en = 1 ;
        IO_SREG(G_GBL_CFG, gponGlbCfg.Raw) ;
        /* Change the current state */
        gpon_act_change_gpon_state(GPON_STATE_O5) ;
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
		/* START DEL GEM RX Interface */
		gponDelStop.Raw = IO_GREG(G_MBI_MPI_STOP) ;
		gponDelStop.Bits.del_rx_stop = XPON_DISABLE ;
		IO_SREG(G_MBI_MPI_STOP, gponDelStop.Raw) ;		
#endif		
        if (!gpGponPriv->typeBOnGoing){
#ifndef TCSUPPORT_CPU_EN7521
			enable_cpu_us_omci_oam_traffic() ;
            disable_cpu_us_data_traffic()    ;
#endif
	        start_omci_oam_monitor();
		}
#ifdef TCSUPPORT_CPU_EN7521
		else
		{
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
			if((1 == gpon_check_gem_encrypt()) || GPON_OMCC_GEM_ENCRYPTION)
			{
				gponDevSetShadowKey(&gpGponPriv->pGponRecovery->recoveraesKey);
				gponDevSetKeySwithTime(0) ;
				gponDevSetEncryptKey(gpGponPriv->gponCfg.key) ;
			}
			/*if 7523,for type B,we will trun on the xpon_led_on here*/
			change_pon_led_status(ALARM_LED_ON);	
#else
		    if(0 != gpon_recover_create_allocId())
    		{
    			PON_MSG(MSG_INT, "GPON IRQ: gpon_recover_create_allocId fail.\n") ;
    		}
			/*if not 7523,for type B,we will trun on the xpon_led_on here*/
    		if(0 != gpon_recover_create_gemport())
    		{
    			PON_MSG(MSG_INT, "GPON IRQ: gpon_recover_create_gemport fail.\n") ;
    		}
			if((1 == gpon_check_gem_encrypt()) || GPON_OMCC_GEM_ENCRYPTION)
			{
    			gponDevSetShadowKey(&gpGponPriv->pGponRecovery->recoveraesKey);
    			aesKeySwitchBySw.Bits.aes_key_switch_by_sw_cmd = GPON_AES_KEY_SWITCH_BY_SW_CMD_SET;
    			IO_SREG(G_AES_KEY_SWITCH_BY_SW, aesKeySwitchBySw.Raw) ;
				aesKeySwitchBySw.Raw = IO_GREG(G_AES_KEY_SWITCH_BY_SW) ;
				if(aesKeySwitchBySw.Bits.aes_key_switch_by_sw_done != GPON_AES_KEY_SWITCH_BY_SW_DONE)
				{
					PON_MSG(MSG_OAM, "PLOAM: aes key switch set by sw fail\n") ;
				}
				else
				{
					gponDevSetEncryptKey(gpGponPriv->gponCfg.key) ;
				}
			}
#endif
		}
		enable_cpu_us_traffic(); /* enable cpu traffic */
#endif
        /* Change the current state */
		gpGponPriv->typeBOnGoing = 0;


		/*clear the origin data of GRP0 and GRP1*/
		dbgGrp0Status.Raw = 0xFFFFFFFF;
		dbgGrp1Status.Raw = 0xFFFFFFFF;
		IO_SREG(DBG_GRP_0, dbgGrp0Status.Raw) ;
		IO_SREG(DBG_GRP_1, dbgGrp1Status.Raw) ;

#ifdef TCSUPPORT_AUTOBENCH
		if(gpon_slt_test!=1){
	        printk("gpon O5 pass\n");
	        gpon_slt_test = 1;
		}
#endif
        
        /* report GPON event message */
        xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_ACTIVATE, 0) ;
    }
    
    return 0 ;
}

/******************************************************************************
******************************************************************************/
#ifdef TCSUPPORT_VNPTT
	int ignore_deactive_flag = 0;
#endif
static int ploam_recv_deactivate_onu(PLOAM_RAW_Deactivate_OnuID_T *pDeactMsg)
{
	uint spf;

	gponDevGetSuperframe(&spf);
	PON_MSG(MSG_OAM, "PLOAM: Receive Deactivate_OnuID message.(%.8X %.8X %.8X), SPF:0x%08x\n", htonl(pDeactMsg->value[0]), htonl(pDeactMsg->value[1]), htonl(pDeactMsg->value[2]), spf) ; 
	PON_MSG(MSG_ERR, "one bit err cnt:0x%08x\n", IO_GREG(DBG_GEM_HEC_ONE_ERR_CNT)) ; 
	PON_MSG(MSG_ERR, "two bits err cnt:0x%08x\n", IO_GREG(DBG_GEM_HEC_TWO_ERR_CNT)) ; 
	PON_MSG(MSG_ERR, "uncorrectable err cnt:0x%08x\n", IO_GREG(DBG_GEM_HEC_UC_ERR_CNT)) ; 
        
	if((pDeactMsg->raw.dest_id != GPON_ONU_ID) && (pDeactMsg->raw.dest_id != PLOAM_BROADCAST_ADDR)) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pDeactMsg->raw.dest_id) ;
		return -1 ;
	}
#ifdef TCSUPPORT_VNPTT
	if(ignore_deactive_flag)
		return 0;
#endif			
	if(GPON_CURR_STATE==GPON_STATE_O4 || GPON_CURR_STATE==GPON_STATE_O5 || 
	   GPON_CURR_STATE==GPON_STATE_O6) {
		
        xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVNET_GPON_DEACTIVATE, 0) ;
		gpon_set_alarmBit(DACT_INDEX);
        stop_omci_oam_monitor() ;
        
#ifdef TCSUPPORT_CPU_EN7521
		XPON_DPRINT_MSG("call gpon disable.\n");
		gpon_disable();
#else
        gpon_disable_with_option(GPON_DEV_RESET_WITH_FE_RESET);
#endif
    }
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
static int ploam_recv_disable_serial_number(PLOAM_RAW_Disable_SN_T *pDisSnMsg)
{
	PON_MSG(MSG_OAM, "PLOAM: Receive Disable_SN message.(%.8X %.8X %.8X)\n", htonl(pDisSnMsg->value[0]), htonl(pDisSnMsg->value[1]), htonl(pDisSnMsg->value[2])) ; 

	if(pDisSnMsg->raw.dest_id != PLOAM_BROADCAST_ADDR) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pDisSnMsg->raw.dest_id) ;
		return -1 ;
	}
	
	if(GPON_CURR_STATE == GPON_STATE_O7) {
		if((pDisSnMsg->raw.mode == PLOAM_DISABLE_PARTICIPATE_ALL) || 
		  ((pDisSnMsg->raw.mode == PLOAM_DISABLE_PARTICIPATE) && !memcmp(gpGponPriv->gponCfg.sn, pDisSnMsg->raw.sn, 8))) {		

			gpGponPriv->emergencystate = 0;
            tasklet_hi_schedule(&gpGponPriv->gpon_dev_reset);
	        xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_ENABLE, 0) ;
		}
		gpon_clear_alarmBit(DIS_INDEX);
	} else if(GPON_CURR_STATE != GPON_STATE_O1) {
		if((pDisSnMsg->raw.mode == PLOAM_DISABLE_DENIED_ALL) || 
          ((pDisSnMsg->raw.mode == PLOAM_DISABLE_DENIED) && !memcmp(gpGponPriv->gponCfg.sn, pDisSnMsg->raw.sn, 8))) {
			/* Change the current state */
			gponDevMbiStop(XPON_ENABLE);
			tasklet_hi_schedule(&gpGponPriv->gpon_channel_retire);

			gpon_act_change_gpon_state(GPON_STATE_O7) ;
            XPON_PHY_TX_DISABLE();
            #ifdef TCSUPPORT_GOOGLE_FIBER
            XPON_PHY_TX_VCC_DISABLE();
            #endif
	        xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_DISABLE, 0) ;
			gpGponPriv->emergencystate = 1;
			gpon_set_alarmBit(DIS_INDEX);
			gpon_record_emergence_info(GPON_DISABLE_SN_REPORT_O7);
			gpon_record_emergence_info(GPON_DISABLE_SN_SET_EMERGNCE_STATE);
		}
	}
	
	return 0 ;
}

void gpon_dev_and_phy_reset(unsigned long arg)
{
	gpon_disable_with_option(GPON_DEV_RESET_WITH_O7_RESET);
	gpon_act_change_gpon_state(GPON_STATE_O2) ;
#ifdef TCSUPPORT_GOOGLE_FIBER
	XPON_PHY_TX_VCC_ENABLE();
#endif
	XPON_PHY_TX_ENABLE();
}

/******************************************************************************
******************************************************************************/
static int ploam_recv_encrypt_port_id(PLOAM_RAW_Encrypted_PortID_T *pEncryMsg)
{
	ushort port_id ;

	PON_MSG(MSG_OAM, "PLOAM: Receive Encrypted_PortID message.(%.8X %.8X %.8X)\n", htonl(pEncryMsg->value[0]), htonl(pEncryMsg->value[1]), htonl(pEncryMsg->value[2])) ; 

	if(pEncryMsg->raw.dest_id != GPON_ONU_ID) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pEncryMsg->raw.dest_id) ;
		return -1 ;
	}
	
	port_id = ((pEncryMsg->raw.port_id_m<<4) | (pEncryMsg->raw.port_id_l)) ;
				
	/* Check the Port-ID is valid or not */
	/* In some case, the OLT could send the Encrypted_PortID PLOAM 
	   before creating GEM port via OMCI */

	/* only record the GEM port encry info here*/
	gpWanPriv->gpon.gemIdToIndex[port_id] &= GPON_GEM_IDX_MASK ;
	gpWanPriv->gpon.gemIdToIndex[port_id] |= ((pEncryMsg->raw.encrypt==0b11)?GPON_GEM_ENCRY_MASK:0) ;
	gpon_recovery_set_encrytion(port_id, (pEncryMsg->raw.encrypt==0b11)?1:0);
	if(gwan_is_gemport_valid(port_id) == 1) 		
		gwan_config_gemport(port_id, ENUM_CFG_ENCRYPTION, ((pEncryMsg->raw.encrypt==0b11)?1:0)) ;
#if 0
	if(gwan_is_gemport_valid(port_id) != 1) {
		gwan_create_new_gemport(port_id, GPON_UNKNOWN_CHANNEL, ((pEncryMsg->raw.encrypt==0b11)?1:0)) ;
	} else {
		gwan_config_gemport(port_id, ENUM_CFG_ENCRYPTION, ((pEncryMsg->raw.encrypt==0b11)?1:0)) ;
	}
#endif


	/* Send acknowledge PLOAM message */
	ploam_send_acknowledge_msg(pEncryMsg->raw.msg_id, (unchar *)pEncryMsg) ;
	
	return 0 ;
}


/******************************************************************************
******************************************************************************/
static int ploam_recv_request_passwd(PLOAM_RAW_Request_Password_T *pReqPasswdMsg)
{
	PON_MSG(MSG_OAM, "PLOAM: Receive Request_Password message.(%.8X %.8X %.8X)\n", htonl(pReqPasswdMsg->value[0]), htonl(pReqPasswdMsg->value[1]), htonl(pReqPasswdMsg->value[2])) ; 

	if(pReqPasswdMsg->raw.dest_id != GPON_ONU_ID) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pReqPasswdMsg->raw.dest_id) ;
		return -1 ;
	}
	
	if(GPON_CURR_STATE == GPON_STATE_O5) {
		/* three Password PLOAM messages */
		ploam_send_passwd_msg(gpGponPriv->gponCfg.passwd, 10) ;
	}
	memset(&gpGponPriv->prePloamMsg, 0, sizeof(PLOAM_RAW_General_T));
	return 0 ;
}

/******************************************************************************
******************************************************************************/
static int ploam_recv_assign_alloc_id(PLOAM_RAW_Assign_AllocID_T *pAssAllocIdMsg)
{
	ushort allocId ;
	
	PON_MSG(MSG_OAM, "PLOAM: Receive Assign_AllocID message.(%.8X %.8X %.8X)\n", htonl(pAssAllocIdMsg->value[0]), htonl(pAssAllocIdMsg->value[1]), htonl(pAssAllocIdMsg->value[2])) ; 

	if(pAssAllocIdMsg->raw.dest_id != GPON_ONU_ID) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pAssAllocIdMsg->raw.dest_id) ;
		return -1 ;
	}
	
	if(GPON_CURR_STATE == GPON_STATE_O5) {	
		allocId = ((pAssAllocIdMsg->raw.alloc_id_m<<4) | (pAssAllocIdMsg->raw.alloc_id_l)) ;

		if(allocId != gpGponPriv->gponCfg.onu_id) {
			/* Setting the MAC register, set the Alloc-ID valid */
			if(pAssAllocIdMsg->raw.type == 0x01 /*Allocate in GEM*/) 
			{
				if(FALSE == gwanCheckAllocIdExist(allocId))
				{
					if (0 ==gwan_create_new_tcont(allocId)){
						gpWanPriv->activeChannelNum ++;
						if (gpWanPriv->activeChannelNum > GPON_TCONT_MAX_NUM) {
							printk("AllocId has exceed the max num %d\n", GPON_TCONT_MAX_NUM);
							gpWanPriv->activeChannelNum = GPON_TCONT_MAX_NUM;
						}
						if(gpMonitor->run_state == MONITOR_DONE){
							gpon_init_qdma_tx_buff();
							GPON_START_TIMER(gpWanPriv->gpon.setQdmaTxBuffTimer,GPON_SET_QDMA_TX_BUFF_TIMER) ;
						}
					}
				}
				/* report GPON event message */
				xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_TCONT_ALLOCED, allocId) ;
			} else if(pAssAllocIdMsg->raw.type == 0xFF /*Deallocate*/) {
				if ( 0 == gwan_remove_tcont(allocId)){
					gpWanPriv->activeChannelNum --;
					if (gpWanPriv->activeChannelNum < 1)
						gpWanPriv->activeChannelNum = 1;
					//xpon_reset_qdma_tx_buf();  OSBNB00090348
					deallocate_flag = 1;
				}
			}
		}
		PON_MSG(MSG_OAM, "PLOAM: OLT Assign ALLOC_ID %d\n", allocId) ;
	}
	
	/* Send acknowledge PLOAM message */
	ploam_send_acknowledge_msg(pAssAllocIdMsg->raw.msg_id, (unchar *)pAssAllocIdMsg) ;	

	return 0 ;
}

/******************************************************************************
******************************************************************************/
static int ploam_recv_popup_msg(PLOAM_RAW_Popup_T *pPopupMsg)
{
    if(gpPhyData->phy_link_status  == PHY_LINK_STATUS_LOS)
    {        
	    PON_MSG(MSG_TYPEB, "PLOAM:PHY STATUS LOS, Ignore Receive Popup message.(%.8X %.8X %.8X)\n", htonl(pPopupMsg->value[0]), htonl(pPopupMsg->value[1]), htonl(pPopupMsg->value[2])) ;
        return 0;
    }
    else
    {
	    PON_MSG(MSG_OAM|MSG_TYPEB, "PLOAM: Receive Popup message.(%.8X %.8X %.8X)\n", htonl(pPopupMsg->value[0]), htonl(pPopupMsg->value[1]), htonl(pPopupMsg->value[2])) ;
	}

	if((pPopupMsg->raw.dest_id != GPON_ONU_ID) && (pPopupMsg->raw.dest_id != PLOAM_BROADCAST_ADDR)) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pPopupMsg->raw.dest_id) ;
		return -1 ;
	}

	if(GPON_CURR_STATE == GPON_STATE_O6) {
		mdelay(typeB_delay_g);
#ifndef TCSUPPORT_CPU_EN7523
		modify_mac_internal_delay();
#endif
        gpGponPriv->typeBOnGoing = 1;
		if(pPopupMsg->raw.dest_id == PLOAM_BROADCAST_ADDR) {
			/* Change the current state */
			gpon_act_change_gpon_state(GPON_STATE_O4) ;
		} else {
			/* Change the current state */
			gpon_act_change_gpon_state(GPON_STATE_O5) ;
		}
		gpon_set_cpu_load_version(SET_LIMIT);
	} 
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
static int ploam_recv_request_key(PLOAM_RAW_Request_Key_T *pReqKeyMsg)
{
	PON_MSG(MSG_OAM, "PLOAM: Receive Request_Key message.(%.8X %.8X %.8X)\n", htonl(pReqKeyMsg->value[0]), htonl(pReqKeyMsg->value[1]), htonl(pReqKeyMsg->value[2])) ; 

	if(pReqKeyMsg->raw.dest_id != GPON_ONU_ID) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pReqKeyMsg->raw.dest_id) ;
		return -1 ;
	}
		
	if(GPON_CURR_STATE == GPON_STATE_O5) {
		/* Generate a random number for encryption key */
		get_random_bytes(gpGponPriv->gponCfg.key, GPON_ENCRYPT_KEY_LENS) ;
		gpGponPriv->gponCfg.keyIdx = (gpGponPriv->gponCfg.keyIdx)?0:1 ;
		
		/* Set the new encryption to device */
		gponDevSetEncryptKey(gpGponPriv->gponCfg.key) ;
		
		/* Send three encryption key PLOAM messages */
		ploam_send_encryption_key(gpGponPriv->gponCfg.keyIdx, gpGponPriv->gponCfg.key) ;
		
		/* Set the flag to indicate the key request in activation state */
		gpGponPriv->gponCfg.flags.isRequestKey = 1 ;
	}
	
	return 0 ;
}


/******************************************************************************
******************************************************************************/
static int ploam_recv_config_port_id(PLOAM_RAW_Configure_PortID_T *pConfigPortIdMsg)
{
	REG_G_OMCI_ID gponOmciId ;
	ushort port_id ;
	
	PON_MSG(MSG_OAM, "PLOAM: Receive Configure_PortID message.(%.8X %.8X %.8X)\n", htonl(pConfigPortIdMsg->value[0]), htonl(pConfigPortIdMsg->value[1]), htonl(pConfigPortIdMsg->value[2])) ; 

	if(pConfigPortIdMsg->raw.dest_id != GPON_ONU_ID) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pConfigPortIdMsg->raw.dest_id) ;
		return -1 ;
	}

	if(GPON_CURR_STATE == GPON_STATE_O5) {
		port_id = ((pConfigPortIdMsg->raw.port_id_m<<4) | (pConfigPortIdMsg->raw.port_id_l)) ;
		
		if(pConfigPortIdMsg->raw.activate == 1 /*activate*/) {
			gpGponPriv->gponCfg.omcc = port_id ;
			
			/* Setting the MAC registger */
			/* activate the port id of OMCI channel */
			gponOmciId.Raw = IO_GREG(G_OMCI_ID) ;
			gponOmciId.Bits.omci_port_id_vld = 1 ;
			gponOmciId.Bits.omci_gpid = port_id ;
			IO_SREG(G_OMCI_ID, gponOmciId.Raw) ;

			if(0 != gwan_create_new_tcont(GPON_ONU_ID)){
				PON_MSG((MSG_ERR|MSG_OAM), "gwan_create_new_tcont fail.\n") ;
			}

			if(gwan_is_gemport_valid(port_id)  != 1){
				gwan_create_new_gemport(port_id, 0, XPON_DISABLE,GPON_ONU_ID) ;
			}
			
			/* reload the AES key to MAC */
			if(!gpGponPriv->gponCfg.flags.isRequestKey) {
				gponDevSetKeySwithTime(0) ;
				gpGponPriv->gponCfg.flags.isRequestKey = 1 ;
			}
			/* report GPON event message */
			xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_TCONT_ALLOCED, GPON_ONU_ID) ;
		} else {
			gpGponPriv->gponCfg.omcc = GPON_UNASSIGN_GEM_ID ;
			gpGponPriv->gponCfg.omccGemEnCry = 0;
			/* Setting the MAC registger */
			/* deactivate the port id of OMCI channel */
			gponOmciId.Raw = IO_GREG(G_OMCI_ID) ;
			gponOmciId.Bits.omci_port_id_vld = 0 ;
			IO_SREG(G_OMCI_ID, gponOmciId.Raw) ;

			gwan_remove_gemport(port_id) ;
			if(0 != gwan_remove_tcont(GPON_ONU_ID)){
				PON_MSG((MSG_ERR|MSG_OAM), "gwan_remove_tcont fail.\n") ;
			}
		}
	}
	
	/* Send acknowledge PLOAM message */
	ploam_send_acknowledge_msg(pConfigPortIdMsg->raw.msg_id, (unchar *)pConfigPortIdMsg) ;
	
	return 0 ;
}


/******************************************************************************
******************************************************************************/
static int ploam_recv_pee_msg(PLOAM_RAW_PEE_T *pPeeMsg)
{
	PON_MSG(MSG_OAM, "PLOAM: Receive PEE message.(%.8X %.8X %.8X)\n", htonl(pPeeMsg->value[0]), htonl(pPeeMsg->value[1]), htonl(pPeeMsg->value[2])) ; 

	if(pPeeMsg->raw.dest_id != PLOAM_BROADCAST_ADDR) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pPeeMsg->raw.dest_id) ;
		return -1 ;
	}
	
	//Setting the system alarm status
	//alert the PEE alarm
	gpon_set_alarmBit(PEE_INDEX);
	return 0 ;
}


/******************************************************************************
******************************************************************************/
static int ploam_recv_cpl_msg(PLOAM_RAW_CPL_T *pCplMsg)
{
	PON_MSG(MSG_OAM, "PLOAM: Receive CPL message.(%.8X %.8X %.8X)\n", htonl(pCplMsg->value[0]), htonl(pCplMsg->value[1]), htonl(pCplMsg->value[2])) ; 

	if((pCplMsg->raw.dest_id!=PLOAM_BROADCAST_ADDR) && (pCplMsg->raw.dest_id!=GPON_ONU_ID)) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pCplMsg->raw.dest_id) ;
		return -1 ;
	}
		
	if(GPON_CURR_STATE==GPON_STATE_O4 || GPON_CURR_STATE==GPON_STATE_O5) {
		if(pCplMsg->raw.power_level == 0b10 /*Increase*/) {
			//Setting the PHY register
			//increase the transmitted power
		} else if(pCplMsg->raw.power_level == 0b10 /*Decrease*/) {
			//Setting the PHY register
			//decrease the transmitted power
		}
	}
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
static int ploam_recv_pst_msg(PLOAM_RAW_PST_T *pPstMsg)
{
	PON_MSG(MSG_OAM, "PLOAM: Receive PST message.(%.8X %.8X %.8X)\n", htonl(pPstMsg->value[0]), htonl(pPstMsg->value[1]), htonl(pPstMsg->value[2])) ; 

	if((pPstMsg->raw.dest_id!=PLOAM_BROADCAST_ADDR) && (pPstMsg->raw.dest_id!=GPON_ONU_ID)) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pPstMsg->raw.dest_id) ;
		return -1 ;
	}
	
	//Active upstream APS command
	//gpon_active_aps_command
	gpon_set_alarmBit(MIS_INDEX);
	return 0 ;
}

/******************************************************************************
******************************************************************************/
static int ploam_recv_ber_interval(PLOAM_RAW_BER_Interval_T *pBerMsg)
{
	uint interval ;
	
	PON_MSG(MSG_OAM, "PLOAM: Receive BER_Interval message.(%.8X %.8X %.8X)\n", htonl(pBerMsg->value[0]), htonl(pBerMsg->value[1]), htonl(pBerMsg->value[2])) ; 

	if((pBerMsg->raw.dest_id!=PLOAM_BROADCAST_ADDR) && (pBerMsg->raw.dest_id!=GPON_ONU_ID)) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pBerMsg->raw.dest_id) ;
		return -1 ;
	}
		
	if(GPON_CURR_STATE == GPON_STATE_O5) {
		interval = ((pBerMsg->raw.interval[0]<<24) | (pBerMsg->raw.interval[1]<<16) | (pBerMsg->raw.interval[2]<<8) | (pBerMsg->raw.interval[3])) ;
		gpGponPriv->gponCfg.berInterval = interval>>3 ; /* translation the frame number to ms */
		if(gpGponPriv->gponCfg.berInterval) {
			GPON_START_TIMER(gpGponPriv->ber_timer,gpGponPriv->gponCfg.berInterval ) ;
			XPON_PHY_COUNTER_CLEAR(PHY_BIP_CNT_CLR);
		}
	}
	
	/* Send acknowledge PLOAM message */
	ploam_send_acknowledge_msg(pBerMsg->raw.msg_id, (unchar *)pBerMsg) ;
	
	return 0 ;	
}

/******************************************************************************
******************************************************************************/
static int ploam_recv_key_switch_time(PLOAM_RAW_Key_Switching_Time_T *pKeySwitchMsg)
{
	uint counter ;
	
	PON_MSG(MSG_OAM, "PLOAM: Receive Key_Switching_Time message.(%.8X %.8X %.8X)\n", htonl(pKeySwitchMsg->value[0]), htonl(pKeySwitchMsg->value[1]), htonl(pKeySwitchMsg->value[2])) ; 

	if(pKeySwitchMsg->raw.dest_id != GPON_ONU_ID) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pKeySwitchMsg->raw.dest_id) ;
		return -1 ;
	}
		
	if(GPON_CURR_STATE == GPON_STATE_O5) {
		counter = ((pKeySwitchMsg->raw.counter[0]<<24) | (pKeySwitchMsg->raw.counter[1]<<16) | (pKeySwitchMsg->raw.counter[2]<<8) | (pKeySwitchMsg->raw.counter[3])) ;
		
		/* Setting the MAC register */
		/* set the super counter to start key switching */
		gponDevSetKeySwithTime(counter) ;
	}
		
	/* Send acknowledge PLOAM message */
	ploam_send_acknowledge_msg(pKeySwitchMsg->raw.msg_id, (unchar *)pKeySwitchMsg) ;
	
	return 0 ;	
}

/******************************************************************************
******************************************************************************/
static int ploam_recv_extended_burst_length(PLOAM_RAW_Extended_Burst_Length_T *pExtBurstLenMsg)
{
    REG_G_PLOu_PRMBL_TYPE3 gponT3Prmbl ;
    REG_G_PLOu_GUARD_BIT gponBuardBit ;
    REG_G_PLOu_PRMBL_TYPE1_2 gponPrmblType ;
    REG_G_PLOu_DELM_BIT gponDelmBit ;
    uint overhead, o3T3Preamble, o5T3Preamble ;
    int t3Offset = gpGponPriv->gponCfg.t3PreambleOffset ;
    PHY_GponPreb_T pon_preb;
    PPHY_GponPreb_T p_pon_preb = & pon_preb;
    
    PON_MSG(MSG_OAM, "PLOAM: Receive Extended_Burst_Length message.(%.8X %.8X %.8X)\n", htonl(pExtBurstLenMsg->value[0]), htonl(pExtBurstLenMsg->value[1]), htonl(pExtBurstLenMsg->value[2])) ; 

    if(pExtBurstLenMsg->raw.dest_id != PLOAM_BROADCAST_ADDR) {
        PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pExtBurstLenMsg->raw.dest_id) ;
        return -1 ;
    }
        
    if(GPON_CURR_STATE == GPON_STATE_O3) {
        /* Check the guard bit, t1, t2, t3 preamble and delimitation bit > 128 byte or not */
        gponBuardBit.Raw = IO_GREG(G_PLOu_GUARD_BIT) ;
        gponPrmblType.Raw = IO_GREG(G_PLOu_PRMBL_TYPE1_2) ;
        gponDelmBit.Raw = IO_GREG(G_PLOu_DELM_BIT) ;
        
        overhead = (gponBuardBit.Bits.guard_bit + gponPrmblType.Bits.prmb1_bit + gponPrmblType.Bits.prmb2_bit + gponDelmBit.Bits.delm_bit) >> 3 ;
        
        if(gpGponPriv->gponCfg.flags.preambleFlag == GPON_BURST_MODE_OVERHEAD_LEN_MORE_THAN_128)
        {
            o3T3Preamble = pExtBurstLenMsg->raw.o3_t3_preamble;
            o5T3Preamble = pExtBurstLenMsg->raw.o5_t3_preamble;
        }
        else
        {
            o3T3Preamble = ((overhead+pExtBurstLenMsg->raw.o3_t3_preamble)>128) ? (128-overhead) : pExtBurstLenMsg->raw.o3_t3_preamble ;
            o5T3Preamble = ((overhead+pExtBurstLenMsg->raw.o5_t3_preamble)>128) ? (128-overhead) : pExtBurstLenMsg->raw.o5_t3_preamble ;
        }
            
        
        p_pon_preb->mask = PHY_T3_O4_PRE_EN | PHY_T3_O5_PRE_EN | PHY_EXT_BUR_MODE_EN | PHY_OPER_RANG_EN;
        p_pon_preb->t3_O4_preamble = o3T3Preamble-t3Offset;
        p_pon_preb->t3_O5_preamble = o5T3Preamble;
        p_pon_preb->extend_burst_mode = 1;
        p_pon_preb->oper_ranged_st = 0x2;
        if(gpGponPriv->gponCfg.flags.preambleFlag == GPON_BURST_MODE_OVERHEAD_LEN_MORE_THAN_128)
        {
        #if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
            p_pon_preb->mask |= PHY_EXTB_LENG_SEL_EN;
            p_pon_preb->extb_length_sel = 1;
        #endif
            PON_MSG(MSG_OAM, "PLOAM: Setting the O3/O4 type 3 preamble. preambleFlag:%s, o3Overhead:%d, O5Overhead:%d\n", 
                (gpGponPriv->gponCfg.flags.preambleFlag) ? "Enable" : "Disable", (overhead+pExtBurstLenMsg->raw.o3_t3_preamble), (overhead+pExtBurstLenMsg->raw.o5_t3_preamble)) ;
        }
        XPON_PHY_SET_API(PON_SET_PHY_GPON_EXTEND_PREAMBLE, p_pon_preb);
        /* Setting the MAC register */
        gponT3Prmbl.Raw = IO_GREG(G_PLOu_PRMBL_TYPE3) ;
        gponT3Prmbl.Bits.ebl_en = 1 ;
        gponT3Prmbl.Bits.ext_prmb3_o5_num = o5T3Preamble ;
        gponT3Prmbl.Bits.ext_prmb3_o3_o4_num = (o3T3Preamble-t3Offset) ;
        IO_SREG(G_PLOu_PRMBL_TYPE3, gponT3Prmbl.Raw) ;
        gpGponPriv->gponCfg.gponT3Prmbl.Raw = gponT3Prmbl.Raw;
        
        PON_MSG(MSG_OAM, "PLOAM: Setting the O3/O4 type 3 preamble. Overflow: %d, Rec. Value: %d, Set Value: %d\n", ((overhead+pExtBurstLenMsg->raw.o3_t3_preamble)>128) ? 1 : 0, pExtBurstLenMsg->raw.o3_t3_preamble, o3T3Preamble-t3Offset) ;
    }
    return 0 ;  
}

/******************************************************************************
******************************************************************************/
static int ploam_recv_pon_id_msg(PLOAM_RAW_PonID_T *pPonIdMsg)
{
	PON_MSG(MSG_OAM, "PLOAM: Receive PonID message.(%.8X %.8X %.8X)\n", htonl(pPonIdMsg->value[0]), htonl(pPonIdMsg->value[1]), htonl(pPonIdMsg->value[2])) ; 

	if(pPonIdMsg->raw.dest_id != PLOAM_BROADCAST_ADDR) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pPonIdMsg->raw.dest_id) ;
		return -1 ;
	}
	
	//Get the PON identifier value
	//store the PON ID information if necessary
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
static int ploam_recv_swift_popup(PLOAM_RAW_Swift_Popup_T *pSwiftPopupMsg)
{
	PON_MSG(MSG_OAM, "PLOAM: Receive Swift_Popup message.(%.8X %.8X %.8X)\n", htonl(pSwiftPopupMsg->value[0]), htonl(pSwiftPopupMsg->value[1]), htonl(pSwiftPopupMsg->value[2])) ; 

	if(pSwiftPopupMsg->raw.dest_id != PLOAM_BROADCAST_ADDR) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pSwiftPopupMsg->raw.dest_id) ;
		return -1 ;
	}
	
	if(GPON_CURR_STATE == GPON_STATE_O6) {
		/* Change the current state */
		gpon_act_change_gpon_state(GPON_STATE_O5) ;
	} 
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
static int ploam_recv_ranging_adjustment(PLOAM_RAW_Ranging_Adjustment_T *pRangingAdjustMsg)
{
	uint eqd_offset ;
	
	PON_MSG(MSG_OAM, "PLOAM: Receive Ranging_Adjustment message.(%.8X %.8X %.8X)\n", htonl(pRangingAdjustMsg->value[0]), htonl(pRangingAdjustMsg->value[1]), htonl(pRangingAdjustMsg->value[2])) ; 

	if((pRangingAdjustMsg->raw.dest_id!=GPON_ONU_ID) && (pRangingAdjustMsg->raw.dest_id!=PLOAM_BROADCAST_ADDR)) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pRangingAdjustMsg->raw.dest_id) ;
		return -1 ;
	}
		
	if(GPON_CURR_STATE == GPON_STATE_O5) {
		eqd_offset = ((pRangingAdjustMsg->raw.eqd_value[0]<<24) | (pRangingAdjustMsg->raw.eqd_value[1]<<16) | (pRangingAdjustMsg->raw.eqd_value[2]<<8) | (pRangingAdjustMsg->raw.eqd_value[3])) ;
		
		if(eqd_offset != 0) {
			if(pRangingAdjustMsg->raw.s_bit == 0b0) {
				/* increase eqd_offset value to the eqd */
				ploam_eqd_adjustment(gpGponPriv->gponCfg.eqd + eqd_offset) ;
			} else {
				/* decrease eqd_offset value to the eqd */
				ploam_eqd_adjustment(gpGponPriv->gponCfg.eqd - eqd_offset) ;
			}
		}
	}
				
	/* Send acknowledge PLOAM message */
	ploam_send_acknowledge_msg(pRangingAdjustMsg->raw.msg_id, (unchar *)pRangingAdjustMsg) ;
	
	return 0 ;
}

/******************************************************************************
 Descriptor:	To receive sleep_allow PLOAM.
 Input Args:	pSleepAllow: sleep_allow PLOAM structure.
 Ret Value:		0: Success.
               	    -1: The PLOAM destination ID is neither ONU ID nor broadcast.
******************************************************************************/
static int ploam_recv_Sleep_Allow(PLOAM_RAW_Sleep_Allow_T *pSleepAllow)
{	
	PON_MSG(MSG_OAM, "PLOAM: Receive Sleep_Allow message.(%.8X %.8X %.8X)\n", htonl(pSleepAllow->value[0]), htonl(pSleepAllow->value[1]), htonl(pSleepAllow->value[2])) ; 

	if((pSleepAllow->raw.dest_id!=GPON_ONU_ID) && (pSleepAllow->raw.dest_id!=PLOAM_BROADCAST_ADDR)) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", pSleepAllow->raw.dest_id) ;
		return -1 ;
	}

	return 0 ;
}

#if !defined(TCSUPPORT_CPU_EN7523) && !defined(TCSUPPORT_CPU_EN7581)
#define MAX_MISMATCH_CNT 20
int mismatchCnt=0;
uint64_t preHwRxCnt=0;
uint64_t preSwRxCnt=0;
void checkGemRxLoss(unsigned long arg)
{
	uint64_t hwRxCnt=0;
	uint64_t swRxCnt=0;
	int gemIdx=0;
	gponDevGetGemPortCounter(GPON_OMCC_ID, GEMPORT_RX_FRAME_CNT, &hwRxCnt, 0) ;
	gemIdx = (gpWanPriv->gpon.gemIdToIndex[GPON_OMCC_ID] & GPON_GEM_IDX_MASK);
	swRxCnt = gpWanPriv->gpon.gemPort[gemIdx].stats.rx_packets ;
	PON_MSG((MSG_INT|MSG_ERR), "Gem Port:%d, SW Rx:%llu, HW Rx:%llu, SW preRx:%llu, HW preRx:%llu, mismatchCnt:%d\n",
						GPON_OMCC_ID,swRxCnt,hwRxCnt,preSwRxCnt,preHwRxCnt,mismatchCnt);
	if((hwRxCnt-preHwRxCnt>0) && (swRxCnt-preSwRxCnt<hwRxCnt-preHwRxCnt)){
		mismatchCnt++;
	}
	preHwRxCnt=hwRxCnt;
	preSwRxCnt=swRxCnt;
	if(mismatchCnt>MAX_MISMATCH_CNT){
		mismatchCnt=0;
		gpon_dev_and_phy_reset(0);
		printk("OMCI HW/SW RX mismatch, MAC Reset\n");
	}
}
#endif
/******************************************************************************
******************************************************************************/
int ploam_init(void)
{
	memset(gpGponPriv->ploamRecvHandler, 0, sizeof(ploam_recv_handler_t)*PLOAM_DOWN_MAX_TYPE) ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_UPSTREAM_OVERHEAD] 		= (ploam_recv_handler_t)ploam_recv_upstream_overhead ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_ASSIGN_ONUID] 			= (ploam_recv_handler_t)ploam_recv_assign_onu_id ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_RANGING_TIME] 			= (ploam_recv_handler_t)ploam_recv_ranging_time ;	
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_DEACTIVATE_ONUID] 		= (ploam_recv_handler_t)ploam_recv_deactivate_onu ;	
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_DISABLE_SERIAL_NUM] 	= (ploam_recv_handler_t)ploam_recv_disable_serial_number ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_ENCRYPTED_PORTID] 		= (ploam_recv_handler_t)ploam_recv_encrypt_port_id ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_REQUEST_PASSWORD] 		= (ploam_recv_handler_t)ploam_recv_request_passwd ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_ASSIGN_ALLOCID] 		= (ploam_recv_handler_t)ploam_recv_assign_alloc_id ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_POPUP_MESSAGE] 			= (ploam_recv_handler_t)ploam_recv_popup_msg ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_REQUEST_KEY] 			= (ploam_recv_handler_t)ploam_recv_request_key ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_CONFIG_PORTID] 			= (ploam_recv_handler_t)ploam_recv_config_port_id ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_PEE_MESSAGE] 			= (ploam_recv_handler_t)ploam_recv_pee_msg ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_CPL_MESSAGE] 			= (ploam_recv_handler_t)ploam_recv_cpl_msg ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_PST_MESSAGE] 			= (ploam_recv_handler_t)ploam_recv_pst_msg ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_BER_INTERVAL] 			= (ploam_recv_handler_t)ploam_recv_ber_interval ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_KEY_SWITCHING_TIME] 	= (ploam_recv_handler_t)ploam_recv_key_switch_time ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_EXTENDED_BURST_LENGTH] 	= (ploam_recv_handler_t)ploam_recv_extended_burst_length ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_PONID_MESSAGE] 			= (ploam_recv_handler_t)ploam_recv_pon_id_msg ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_SWIFT_POPUP] 			= (ploam_recv_handler_t)ploam_recv_swift_popup ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_RANGING_ADJUSTMENT] 	= (ploam_recv_handler_t)ploam_recv_ranging_adjustment ; 
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_SLEEP_ALLOW_MESSAGE] 	= (ploam_recv_handler_t)ploam_recv_Sleep_Allow ; 
	
	memset(&gpGponPriv->prePloamMsg, 0, sizeof(PLOAM_RAW_General_T)) ;

	GPON_CREATE_TIMER(&gpGponPriv->ber_timer,gpon_ber_interval_expires,gpGponPriv->gponCfg.berInterval);/* the default value is 0 */
	GPON_CREATE_TIMER(&gpGponPriv->silence_timer,gpon_silence_interval_expires,gpGponPriv->gponCfg.silenceInterval);/* the default value is 70000 */

	tasklet_init(&gpGponPriv->gpon_dev_reset, gpon_dev_and_phy_reset, 0);
	tasklet_init(&gpGponPriv->gpon_channel_retire, channel_retire_func, 0);
#if !defined(TCSUPPORT_CPU_EN7523) && !defined(TCSUPPORT_CPU_EN7581)
    tasklet_init(&gpGponPriv->gpon_check_rx_loss, checkGemRxLoss, 0);
#endif
	atomic_set(&gpGponPriv->retire_flag, 1);
	
	return 0 ;
}

