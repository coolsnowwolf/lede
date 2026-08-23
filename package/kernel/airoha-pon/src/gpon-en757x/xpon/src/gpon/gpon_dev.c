/*
* File Name: gpon_dev.c
* Description: GPON device configuration
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/delay.h>

#include "common/drv_global.h"
#include "gpon/gpon_dev.h"
#include "common/phy_if_wrapper.h"

#ifdef TCSUPPORT_CPU_EN7521
#include <ecnt_hook/ecnt_hook_fe.h>
#endif

void select_xpon_wan(Xpon_Phy_Mode_t mode)
{
	uint reg;
	
	reg = IO_GREG(0xBFB00070);

	switch(mode)
	{
		case PHY_GPON_CONFIG:
#ifdef TCSUPPORT_CPU_EN7521
		/* Set the SCU register to select the WAN mode (GPON) */
		reg &= ~(0x7<<0);
		reg |= ((0x00 & 0x7) << 0);
#else
		/* Set the SCU register to select the WAN mode (GPON) */
		reg &= ~(0x3<<0);
		reg |= ((0x00 & 0x3) << 0);
#endif
			break;

		case PHY_EPON_CONFIG:
#ifdef TCSUPPORT_CPU_EN7521
			/* Set the SCU register to select the WAN mode (EPON) */
			reg &= ~(0x7<<0);
			reg |= ((0x01 & 0x7) << 0);
#else
			/* Set the SCU register to select the WAN mode (EPON) */
			reg &= ~(0x3<<0);
			reg |= ((0x01 & 0x3) << 0);
#endif
			break;

		default:
			PON_MSG(MSG_ERR, "WAN Configuration Error. mode:%d\n", mode);
			return;
	}
	
	IO_SREG(0xBFB00070, reg) ;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetPloamMsg(PLOAM_RAW_General_T *pPloamMsg)
{
	REG_G_PLOAMd_FIFO_STS gponPloamStatus ;
	int i ;
	
	gponPloamStatus.Raw = IO_GREG(G_PLOAMd_FIFO_STS) ;
	if(gponPloamStatus.Bits.ploamd_fifo_used == 0) {
		goto ret ;
	} else if(gponPloamStatus.Bits.ploamd_fifo_used >= 3) {
		for(i=0 ; i<3 ; i++) {
			pPloamMsg->value[i] = IO_GREG(G_PLOAMd_RDATA) ;
		}
	} else {
		return -EFAULT ;
	}
	
ret:
	return gponPloamStatus.Bits.ploamd_fifo_used ;
}

/*****************************************************************************
******************************************************************************/
int gponDevSendPloamMsg(PLOAM_RAW_General_T *pPloamMsg, uint times)
{
	int i, j ;

    if(gp_dying_info->dying_flag == TRUE && pPloamMsg->raw.msg_id != PLOAM_UP_MSG_DYING_GASP)
        return 0;

	for(i=0 ; i<times ; i++) {
		for(j=0 ; j<3 ; j++) {
			IO_SREG(G_PLOAMu_WDATA, pPloamMsg->value[j]) ;
		}
        
        if(pPloamMsg->raw.msg_id == PLOAM_UP_MSG_DYING_GASP)
        {
	        if(gp_dying_info->opt_type == GPON_DG_DELAY_US)
	            delay1us(gp_dying_info->delay_time);   
	        
	        if(gp_dying_info->opt_type == GPON_DG_SEND_ONEBYONE)
	            return 0;
        }        
	}
    
	if(gp_dying_info->dying_flag == TRUE && pPloamMsg->raw.msg_id == PLOAM_UP_MSG_DYING_GASP) 
        gp_dying_info->dying_flag = FALSE;
    
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevSet1PPSInt(XPON_Mode_t mode)
{
	int flag = (mode==XPON_ENABLE)?1:0 ;
	REG_G_INT_ENABLE gponIntEnable ;
	REG_G_INT_STATUS gponIntStatus ;
	
	gponIntStatus.Raw = IO_GREG(G_INT_STATUS) ;
	gponIntStatus.Bits.tod_1pps_int = flag ;
	IO_SREG(G_INT_STATUS, gponIntStatus.Raw) ;

	gponIntEnable.Raw = IO_GREG(G_INT_ENABLE) ;
	gponIntEnable.Bits.tod_1pps_int_en = flag ;
	IO_SREG(G_INT_ENABLE, gponIntEnable.Raw) ;

	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevSetNewTod(uint superframe, uint sec, uint nanosec) 
{
	REG_G_TOD_CFG gponTodCfg ;
	REG_G_NEW_TOD_SEC_L32 gponTodSec ;
	REG_G_NEW_TOD_NANO_SEC gponTodNano ;
	float n1310 = 1.4677;
	float n1490 = 1.4682;
	float onuResponseTime = 35000;
	float up = 1.24416;
	uint delta = 0;
	
	
	PON_MSG(MSG_DBG,"Before gponDevSetNewTod sec = %x, nanosec = %x\n", sec, nanosec);
	delta =(uint) (((gpGponPriv->gponCfg.eqd /up) + onuResponseTime) * n1490/(n1310+n1490));

	if (delta > nanosec)
	{
		sec = sec -1;
		nanosec = ~(delta - nanosec) + 1;
	}
	else
	{
		nanosec = nanosec - delta;
	}

	PON_MSG(MSG_DBG,"gponDevSetNewTod:  delta = %d, eqd = %d\n", delta, gpGponPriv->gponCfg.eqd);	
	PON_MSG(MSG_DBG,"After gponDevSetNewTod: sec = %x, nanosec = %x\n", sec, nanosec);
	
	gponTodCfg.Bits.tod_spf_cnt = superframe ;
	gponTodSec.Bits.new_tod_sec_l32 = sec ;
	gponTodNano.Bits.new_tod_nano_sec = nanosec ;
	IO_SREG(G_TOD_CFG, gponTodCfg.Raw) ;
	IO_SREG(G_NEW_TOD_SEC_L32, gponTodSec.Raw) ;
	IO_SREG(G_NEW_TOD_NANO_SEC, gponTodNano.Raw) ;
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetCurrentTod(uint *pSec, uint *pNanosec)
{
	REG_G_CUR_TOD_SEC_L32 gponTodSec ;
	REG_G_CUR_TOD_NANO_SEC gponTodNano ;

	gponTodSec.Raw = IO_GREG(G_CUR_TOD_SEC_L32) ;
	gponTodNano.Raw = IO_GREG(G_CUR_TOD_NANO_SEC) ;
	*pSec = gponTodSec.Bits.cur_tod_sec_l32 ;
	*pNanosec = gponTodNano.Bits.cur_tod_nano_sec ;
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetNewTod(uint *pNewSec, uint *pNewNanoSec)
{
	REG_G_NEW_TOD_SEC_L32 gponNewTodSec ;
	REG_G_NEW_TOD_NANO_SEC gponNewTodNano ;

	gponNewTodSec.Raw = IO_GREG(G_NEW_TOD_SEC_L32) ;
	gponNewTodNano.Raw = IO_GREG(G_NEW_TOD_NANO_SEC) ;
	*pNewSec = gponNewTodSec.Bits.new_tod_sec_l32 ;
	*pNewNanoSec = gponNewTodNano.Bits.new_tod_nano_sec ;
	
	return 0 ;
}


/*****************************************************************************
******************************************************************************/
int gponDevGetTodSwitchTime(uint *spf)
{
	REG_G_TOD_CFG gponTodCfg ;

	gponTodCfg.Raw = IO_GREG(G_TOD_CFG) ;
	*spf = gponTodCfg.Bits.tod_spf_cnt ;
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevSetSerialNumber(unchar *sn)
{
	REG_G_VENDOR_ID gponVendor ;
	REG_G_VS_SN gponSn ;
	
	gponVendor.Bits.vendor_id = (sn[0]<<24)|(sn[1]<<16)|(sn[2]<<8)|(sn[3]) ;
	IO_SREG(G_VENDOR_ID, gponVendor.Raw) ;
	
	gponSn.Bits.vs_sn = (sn[4]<<24)|(sn[5]<<16)|(sn[6]<<8)|(sn[7]) ;
	IO_SREG(G_VS_SN, gponSn.Raw) ;
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
void gponDevResetGemInfo(void) 
{
	int i, RETRY = 3 ;
	REG_G_GEM_PORT_CFG gponGemPortCfg ;
	REG_G_GEM_PORT_STS gponGemPortStatus ;
	
	for(i=0 ; i<GPON_MAX_GEM_ID ; i++) {
		gponGemPortCfg.Raw = 0 ;
		gponGemPortCfg.Bits.gpid_cmd = 1 ; //Write Command
		gponGemPortCfg.Bits.gpid_vld =  0 ;
		gponGemPortCfg.Bits.gpid_encrypt = 0 ;
		gponGemPortCfg.Bits.gem_port_id = i ;
		IO_SREG(G_GEM_PORT_CFG, gponGemPortCfg.Raw) ;
		
		RETRY = 3 ;
		while(RETRY--) {
			gponGemPortStatus.Raw = IO_GREG(G_GEM_PORT_STS) ;
			if(gponGemPortStatus.Bits.gpid_cmd_done) {
				break ;
			} 
			mdelay(1) ;
		}
	}

	return ;
}
/*****************************************************************************
******************************************************************************/
int gponDevGetSuperframe(uint *counter)
{
#ifdef TCSUPPORT_CPU_EN7521
	REG_DBG_DS_SPF_CNT dsSpfCnt;

	dsSpfCnt.Raw = IO_GREG(DBG_DS_SPF_CNT);
	*counter = dsSpfCnt.Bits.dbg_ds_spf_cnt;
#else
	REG_DBG_PROBE_CTRL 	gponDbgProbeCtrl ;    
	REG_DBG_PROBE_HIGH32    gponDbgProbeHigh; 
    	REG_DBG_PROBE_LOW32 	gponDbgProbeLow;

	IO_SREG(DBG_PROBE_CTRL, 0x500015) ;

	gponDbgProbeHigh.Raw = IO_GREG(DBG_PROBE_HIGH32);
	gponDbgProbeLow.Raw = IO_GREG(DBG_PROBE_LOW32);

	PON_MSG(MSG_DBG, "gponDevGetSuperframe: gponDbgProbeHigh[4244] = %x, gponDbgProbeLow[4248] = %x\n", gponDbgProbeHigh.Raw, gponDbgProbeLow.Raw );

	 *counter = ((gponDbgProbeLow.Raw & 0xffff0000) >> 16) | ((gponDbgProbeHigh.Raw & 0x3ffff) << 16);
#endif
	
	 PON_MSG(MSG_DBG, "gponDevGetSuperframe: super frame counter = %x\n", *counter);
	return 0 ;
}


/*****************************************************************************
******************************************************************************/
int gponDevGetGemInfo(ushort gemPortId, unchar *pValid, unchar *pEncrypted)
{
	int RETRY = 3 ;
	REG_G_GEM_PORT_CFG gponGemPortCfg ;
	REG_G_GEM_PORT_STS gponGemPortStatus ;
	
	if(gemPortId & ~0xFFF) {
		return -EFAULT ;
	}
	
	gponGemPortCfg.Bits.gpid_cmd = 0 ; //Read Command
	gponGemPortCfg.Bits.gem_port_id = gemPortId ;
	IO_SREG(G_GEM_PORT_CFG, gponGemPortCfg.Raw) ;
	
	while(RETRY--) {
		gponGemPortStatus.Raw = IO_GREG(G_GEM_PORT_STS) ;
		if(gponGemPortStatus.Bits.gpid_cmd_done) {
			*pValid = (gponGemPortStatus.Bits.gpid_vld_sts) ? XPON_ENABLE : XPON_DISABLE ;
			*pEncrypted = (gponGemPortStatus.Bits.gpid_encrypt_sts) ? XPON_ENABLE : XPON_DISABLE ;
			
			return 0 ;
		} 
		mdelay(1) ;
	}

	return -ETIME ;
}

/******************************************************************************
 Descriptor:	dump gemport info.
 Input Args:	none.
 Ret Value:		0: Success.
             -EFAULT: Get gemInfo error.
******************************************************************************/
int gponDevDumpGemInfo(void)
{
	unchar valid, encrypt ;
	int i, ret ;
	
	for(i=0 ; i < GPON_MAX_GEM_ID ; i++) {
		ret = gponDevGetGemInfo(i, &valid, &encrypt) ;
		if(ret != 0) {
			printk("Get GEM port ID:%d failed\n", i) ;	
			return -EFAULT ;
		} else {
			printk("GEM ID:%04d, Valid:%s,\tEncryption:%s\n", i, 
								(valid == XPON_ENABLE) 	 ? "Valid" : "Invalid",
								(encrypt == XPON_ENABLE) ? "Encrypted" : "Not encrypted") ;
		}
	}

	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevSetGemInfo(ushort gemPortId, unchar isValid, unchar isEncrypted)
{
	int RETRY = 3000 ;
	REG_G_GEM_PORT_CFG gponGemPortCfg ;
	REG_G_GEM_PORT_STS gponGemPortStatus ;
	
	if(gemPortId & ~0xFFF) {
		return -EFAULT ;	
	}
	
	gponGemPortCfg.Raw = 0 ;
	gponGemPortCfg.Bits.gpid_cmd = 1 ; //Write Command
	gponGemPortCfg.Bits.gpid_vld = (isValid==XPON_ENABLE) ? 1 : 0 ;
	gponGemPortCfg.Bits.gpid_encrypt = (isEncrypted==XPON_ENABLE) ? 1 : 0 ;
	gponGemPortCfg.Bits.gem_port_id = gemPortId ;
	IO_SREG(G_GEM_PORT_CFG, gponGemPortCfg.Raw) ;
	
	while(RETRY--) {
		gponGemPortStatus.Raw = IO_GREG(G_GEM_PORT_STS) ;
		if(gponGemPortStatus.Bits.gpid_cmd_done) {
			return 0 ;
		} 
		udelay(1) ;
	}

	return -ETIME ;
}

/*****************************************************************************
******************************************************************************/
int gponDevGemMibTablesInit(void)
{
	int RETRY = 3 ;
	REG_G_MIB_TBL_INIT gponMibTblInit ;
	REG_G_GPIDX_TBL_INIT gponGPIdxTblInit ;
	
	gponMibTblInit.Raw = 0 ;
	gponMibTblInit.Bits.mib_tbl_init_start = 1 ;
	IO_SREG(G_MIB_TBL_INIT, gponMibTblInit.Raw) ;

	gponGPIdxTblInit.Raw = 0 ;
	gponGPIdxTblInit.Bits.gpidx_tbl_init_start = 1 ;
	IO_SREG(G_GPIDX_TBL_INIT, gponGPIdxTblInit.Raw) ;
	
	while(RETRY--) {
		gponMibTblInit.Raw = IO_GREG(G_MIB_TBL_INIT) ;
		gponGPIdxTblInit.Raw = IO_GREG(G_GPIDX_TBL_INIT) ;
		if(gponMibTblInit.Bits.mib_tbl_init_done && gponGPIdxTblInit.Bits.gpidx_tbl_init_done) {
			return 0 ;
		} 
		mdelay(1) ;
	}

	return -ETIME ;
}

/*****************************************************************************
******************************************************************************/
int gponDevUpdateGemMibIdxTable(unchar addr, ushort data)
{
	int RETRY = 3 ;
	REG_G_GPIDX_TBL_CTRL gponGPIdxTblCtrl ;
	REG_G_GPIDX_TBL_STS gponGPIdxTblStatus ;
	
	gponGPIdxTblCtrl.Raw = 0 ;
	gponGPIdxTblCtrl.Bits.gpidx_cmd = 1 ; //Write Command
	gponGPIdxTblCtrl.Bits.gpidx_tbl_addr = addr ;
	gponGPIdxTblCtrl.Bits.gpidx_tbl_wdata = data ;
	IO_SREG(G_GPIDX_TBL_CTRL, gponGPIdxTblCtrl.Raw) ;
	
	while(RETRY--) {
		gponGPIdxTblStatus.Raw = IO_GREG(G_GPIDX_TBL_STS) ;
		if(gponGPIdxTblStatus.Bits.gpidx_cmd_done) {
			return 0 ;
		} 
		udelay(1) ;
	}

	return -ETIME ;
}

/******************************************************************************
 Description: it's used to get HW gem port's rx/tx frmae/payload counter 
 Input Args : gemPortId [in]: gem port id
              type      [in]: type of statistics
              pHData   [out]: high 32 bit of result
              pLData   [out]: low 32 bit of result
              readClear [in]: boolean, specify whether opration is read-clear
 Ret Value: success: 0
            fail   : non-zero value
******************************************************************************/
static int __gponDevGetGemPortCounter(ushort gemPortId, GPON_GEMPORT_STATS_TYPE_t type, 
                    uint *pHData, uint *pLData, unchar readClear)
{
	int RETRY = 3 ;
	REG_G_MIB_CTRL_STS gponGemMibCtrl ;
	
	if(gemPortId & ~0xFFF) {
		return -EFAULT ;	
	}

	gponGemMibCtrl.Raw = 0 ;
	gponGemMibCtrl.Bits.mib_cmd = 0 ; /* Read Command */
	gponGemMibCtrl.Bits.mib_read_clr_en = readClear ;
	gponGemMibCtrl.Bits.mib_type = type ;
	gponGemMibCtrl.Bits.mib_gpid = gemPortId ;
	IO_SREG(G_MIB_CTRL_STS, gponGemMibCtrl.Raw) ;
	
	while(RETRY--) {
		gponGemMibCtrl.Raw = IO_GREG(G_MIB_CTRL_STS) ;
		if(gponGemMibCtrl.Bits.mib_cmd_done) {
			*pHData = IO_GREG(G_MIB_RDATA_H32) ;
			*pLData = IO_GREG(G_MIB_RDATA_L32) ;
			return 0 ;
		} 
		mdelay(1) ;
	}

	return -ETIME ;
}


/*****************************************************************************
******************************************************************************/
int gponDevClearGemPortCounter(ushort gemPortId)
{
    GPON_GEMPORT_STATS_TYPE_t pHData, pLData;
    
    if (0 !=  __gponDevGetGemPortCounter(gemPortId, GEMPORT_RX_FRAME_CNT,   &pHData, &pLData, 1) )
        return -1;

    if (0 !=  __gponDevGetGemPortCounter(gemPortId, GEMPORT_RX_PL_BYTE_CNT, &pHData, &pLData, 1) )
        return -1;

    if (0 !=  __gponDevGetGemPortCounter(gemPortId, GEMPORT_TX_FRAME_CNT,   &pHData, &pLData, 1) )
        return -1;

    if (0 !=  __gponDevGetGemPortCounter(gemPortId, GEMPORT_TX_PL_BYTE_CNT, &pHData, &pLData, 1) )
        return -1;

    return 0;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetGemPortCounter(ushort gemPortId, GPON_GEMPORT_STATS_TYPE_t type, uint *pHData, uint *pLData)
{
    return __gponDevGetGemPortCounter(gemPortId, type, pHData, pLData, 0);
}

/*****************************************************************************
******************************************************************************/
void gponDevResetAllocId(void) 
{
	REG_G_TCONT_ID_0_1 gponTContId ;
	
	gponTContId.Raw = 0 ;
	gponTContId.Bits.t_cont0_id = 0xFF ;
	gponTContId.Bits.t_cont1_id = 0xFF ;
	
	IO_SREG(G_TCONT_ID_0_1, gponTContId.Raw) ;
	IO_SREG(G_TCONT_ID_2_3, gponTContId.Raw) ;
	IO_SREG(G_TCONT_ID_4_5, gponTContId.Raw) ;
	IO_SREG(G_TCONT_ID_6_7, gponTContId.Raw) ;
	IO_SREG(G_TCONT_ID_8_9, gponTContId.Raw) ;
	IO_SREG(G_TCONT_ID_10_11, gponTContId.Raw) ;
	IO_SREG(G_TCONT_ID_12_13, gponTContId.Raw) ;
	IO_SREG(G_TCONT_ID_14_15, gponTContId.Raw) ;
}

/*****************************************************************************
******************************************************************************/
int gponDevIsChannelValid(unchar channel)
{
	REG_G_TCONT_ID_0_1 gponTCont0 ;
	REG_G_TCONT_ID_2_3 gponTCont2 ;
	REG_G_TCONT_ID_4_5 gponTCont4 ;
	REG_G_TCONT_ID_6_7 gponTCont6 ;
	REG_G_TCONT_ID_8_9 gponTCont8 ;
	REG_G_TCONT_ID_10_11 gponTCont10 ;
	REG_G_TCONT_ID_12_13 gponTCont12 ;
	REG_G_TCONT_ID_14_15 gponTCont14 ;

	switch(channel) {
		case 0:
			gponTCont0.Raw = IO_GREG(G_TCONT_ID_0_1) ;
			return gponTCont0.Bits.t_cont0_vld ;
		case 1:
			gponTCont0.Raw = IO_GREG(G_TCONT_ID_0_1) ;
			return gponTCont0.Bits.t_cont1_vld ;
		case 2:
			gponTCont2.Raw = IO_GREG(G_TCONT_ID_2_3) ;
			return gponTCont2.Bits.t_cont2_vld ;
		case 3:
			gponTCont2.Raw = IO_GREG(G_TCONT_ID_2_3) ;
			return gponTCont2.Bits.t_cont3_vld ;
		case 4:
			gponTCont4.Raw = IO_GREG(G_TCONT_ID_4_5) ;
			return gponTCont4.Bits.t_cont4_vld ;
		case 5:
			gponTCont4.Raw = IO_GREG(G_TCONT_ID_4_5) ;
			return gponTCont4.Bits.t_cont5_vld ;
		case 6:
			gponTCont6.Raw = IO_GREG(G_TCONT_ID_6_7) ;
			return gponTCont6.Bits.t_cont6_vld ;
		case 7:
			gponTCont6.Raw = IO_GREG(G_TCONT_ID_6_7) ;
			return gponTCont6.Bits.t_cont7_vld ;
		case 8:
			gponTCont8.Raw = IO_GREG(G_TCONT_ID_8_9) ;
			return gponTCont8.Bits.t_cont8_vld ;
		case 9:
			gponTCont6.Raw = IO_GREG(G_TCONT_ID_6_7) ;
			return gponTCont8.Bits.t_cont9_vld ;
		case 10:
			gponTCont10.Raw = IO_GREG(G_TCONT_ID_10_11) ;
			return gponTCont10.Bits.t_cont10_vld ;
		case 11:
			gponTCont6.Raw = IO_GREG(G_TCONT_ID_6_7) ;
			return gponTCont10.Bits.t_cont11_vld ;
		case 12:
			gponTCont12.Raw = IO_GREG(G_TCONT_ID_12_13) ;
			return gponTCont12.Bits.t_cont12_vld ;
		case 13:
			gponTCont6.Raw = IO_GREG(G_TCONT_ID_6_7) ;
			return gponTCont12.Bits.t_cont13_vld ;
		case 14:
			gponTCont14.Raw = IO_GREG(G_TCONT_ID_14_15) ;
			return gponTCont14.Bits.t_cont14_vld ;
		case 15:
			gponTCont6.Raw = IO_GREG(G_TCONT_ID_6_7) ;
			return gponTCont14.Bits.t_cont15_vld ;
	}
	
	return 0 ;
} 

#ifdef TCSUPPORT_CPU_EN7521
/******************************************************************************
 Descriptor:	enable/disable sniffer mode. if enable, config sniffer to lan port.
 Input Args:	sniffer: sniffer mode config.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevEnableSniffMode(XPON_Mode_t enable)
{
	DBG_GTC_ETH_EXTR_SET_dbg_gtc_eth_extr_en(DBG_GTC_ETH_EXTR, enable);
}

int gponDevSetSniffMode(GPON_DEV_SNIFFER_MODE_T *sniffer)
{
	REG_DBG_GTC_ETH_EXTR			dbg_gtc_eth_extr;
	REG_DBG_DS_GTC_EXTR_ETH_HDR		ds_gtc_extr_eth_hdr;
	REG_DBG_US_GTC_EXTR_ETH_HDR		us_gtc_extr_eth_hdr;
	REG_SNIFF_TX_DA_SA				sniff_tx_da_sa;
	REG_SNIFF_RX_DA_SA				sniff_rx_da_sa;
	REG_SNIFF_RX_TX_TPID			sniff_rx_tx_tpid;

	if(sniffer->enable != XPON_DISABLE && sniffer->enable != XPON_ENABLE) {
		return -1;
	}

	/* enable & packet padding */
	dbg_gtc_eth_extr.Raw = IO_GREG(DBG_GTC_ETH_EXTR) ;

	/* set enable */
	dbg_gtc_eth_extr.Bits.dbg_gtc_eth_extr_en =			sniffer->enable;
	
	if(sniffer->enable == XPON_ENABLE) {
		if(sniffer->lan_port != SNIFFER_MODE_LAN0 && sniffer->lan_port != SNIFFER_MODE_LAN1 &&
		   sniffer->lan_port != SNIFFER_MODE_LAN2 && sniffer->lan_port != SNIFFER_MODE_LAN3) {
		   return -1;
		}

		/* Set tx */
		sniff_tx_da_sa.Bits.tx_us_da_h16 =					sniffer->tx_da;
		sniff_tx_da_sa.Bits.tx_us_sa_h16 =					sniffer->tx_sa;
		IO_SREG(SNIFF_TX_DA_SA, sniff_tx_da_sa.Raw) ;
		us_gtc_extr_eth_hdr.Bits.dbg_extr_tx_us_eth_type =	sniffer->tx_ethertype ;
		us_gtc_extr_eth_hdr.Bits.dbg_extr_tx_us_sp =		sniffer->tx_vid;
		IO_SREG(DBG_US_GTC_EXTR_ETH_HDR, us_gtc_extr_eth_hdr.Raw) ;
		/* Set rx */
		sniff_rx_da_sa.Bits.rx_ds_da_h16 =					sniffer->rx_da;
		sniff_rx_da_sa.Bits.rx_ds_sa_h16 =					sniffer->rx_sa;
		IO_SREG(SNIFF_RX_DA_SA, sniff_rx_da_sa.Raw) ;
		ds_gtc_extr_eth_hdr.Bits.dbg_extr_rx_ds_eth_type =	sniffer->rx_ethertype;
		ds_gtc_extr_eth_hdr.Bits.dbg_extr_rx_ds_sp =		sniffer->rx_vid;
		IO_SREG(DBG_DS_GTC_EXTR_ETH_HDR, ds_gtc_extr_eth_hdr.Raw) ;
		
		/* Set TPID(special tag) */
		sniff_rx_tx_tpid.Bits.sniff_rx_tpid =				sniffer->rx_tpid;
		sniff_rx_tx_tpid.Bits.sniff_tx_tpid =				sniffer->tx_tpid;
		IO_SREG(SNIFF_RX_TX_TPID, sniff_rx_tx_tpid.Raw) ;

		/* set packet padding */
		dbg_gtc_eth_extr.Bits.sniff_pkt_padding_en =		sniffer->packet_padding;
	}

	/* set enable */
	IO_SREG(DBG_GTC_ETH_EXTR, dbg_gtc_eth_extr.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	get sniffer mode config.
 Input Args:	sniffer: used to store sniffer mode config.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevGetSniffMode(GPON_DEV_SNIFFER_MODE_T *sniffer)
{
	REG_DBG_GTC_ETH_EXTR			dbg_gtc_eth_extr;
	REG_DBG_DS_GTC_EXTR_ETH_HDR		ds_gtc_extr_eth_hdr;
	REG_DBG_US_GTC_EXTR_ETH_HDR		us_gtc_extr_eth_hdr;
	REG_SNIFF_TX_DA_SA				sniff_tx_da_sa;
	REG_SNIFF_RX_DA_SA				sniff_rx_da_sa;
	REG_SNIFF_RX_TX_TPID			sniff_rx_tx_tpid;

	dbg_gtc_eth_extr.Raw =		IO_GREG(DBG_GTC_ETH_EXTR) ;
	sniffer->enable =			dbg_gtc_eth_extr.Bits.dbg_gtc_eth_extr_en;
	sniffer->packet_padding =	dbg_gtc_eth_extr.Bits.sniff_pkt_padding_en;
	
	/* Get tx */
	sniff_tx_da_sa.Raw =		IO_GREG(SNIFF_TX_DA_SA) ;
	sniffer->tx_da =			sniff_tx_da_sa.Bits.tx_us_da_h16;
	sniffer->tx_sa =			sniff_tx_da_sa.Bits.tx_us_sa_h16;
	us_gtc_extr_eth_hdr.Raw =	IO_GREG(DBG_US_GTC_EXTR_ETH_HDR) ;
	sniffer->tx_ethertype =		us_gtc_extr_eth_hdr.Bits.dbg_extr_tx_us_eth_type;
	sniffer->tx_vid =			us_gtc_extr_eth_hdr.Bits.dbg_extr_tx_us_sp;
		
	/* Get rx */
	sniff_rx_da_sa.Raw =		IO_GREG(SNIFF_RX_DA_SA) ;
	sniffer->rx_da =			sniff_rx_da_sa.Bits.rx_ds_da_h16;
	sniffer->rx_sa =			sniff_rx_da_sa.Bits.rx_ds_sa_h16;
	ds_gtc_extr_eth_hdr.Raw =	IO_GREG(DBG_DS_GTC_EXTR_ETH_HDR) ;
	sniffer->rx_ethertype =		ds_gtc_extr_eth_hdr.Bits.dbg_extr_rx_ds_eth_type;
	sniffer->rx_vid =			ds_gtc_extr_eth_hdr.Bits.dbg_extr_rx_ds_sp;
			
	/* Get TPID(special tag) */
	sniff_rx_tx_tpid.Raw =		IO_GREG(SNIFF_RX_TX_TPID) ;
	sniffer->rx_tpid =			sniff_rx_tx_tpid.Bits.sniff_rx_tpid;
	sniffer->tx_tpid =			sniff_rx_tx_tpid.Bits.sniff_tx_tpid;
	sniffer->lan_port =			sniffer->tx_tpid;

	return 0;
}

/******************************************************************************
 Descriptor:	Immediately switch AES key from shadow key to active key.
 Input Args:	none.
 Ret Value: GPON_AES_KEY_SWITCH_BY_SW_DONE_PROCESSING: Hardware has something 
                                                                                               error.
                GPON_AES_KEY_SWITCH_BY_SW_DONE:                     success.
******************************************************************************/
GPON_AES_KEY_SWITCH_BY_SW_DONE_t gponDevSetAesKeySwitchBySw(void)
{
	REG_G_AES_KEY_SWITCH_BY_SW aesKeySwitchBySw;
	int RETRY = 3000 ;

	aesKeySwitchBySw.Bits.aes_key_switch_by_sw_cmd = GPON_AES_KEY_SWITCH_BY_SW_CMD_SET;
	IO_SREG(G_AES_KEY_SWITCH_BY_SW, aesKeySwitchBySw.Raw) ;
	
	while(RETRY--) {
		aesKeySwitchBySw.Raw = IO_GREG(G_AES_KEY_SWITCH_BY_SW) ;
		if(aesKeySwitchBySw.Bits.aes_key_switch_by_sw_done == GPON_AES_KEY_SWITCH_BY_SW_DONE) {
			return GPON_AES_KEY_SWITCH_BY_SW_DONE;
		} 
		udelay(1) ;
	}

	PON_MSG(MSG_ERR, "[%s:%s]\n  Fail to set AES key switch by sw.\n", strrchr(__FILE__, '/') + 1, __func__) ;
	return GPON_AES_KEY_SWITCH_BY_SW_DONE_PROCESSING;
}

/******************************************************************************
 Descriptor:	GPON MAC reset exclude reg module.
 Input Args:	none.
 Ret Value: none.
******************************************************************************/
int gponDevSwReset(void)
{
	REG_DBG_GPON_MAC_SET dbg_gpon_mac_set;
	unsigned long flag;

	spin_lock_irqsave(&gpGponPriv->swReset_lock, flag) ;
	/* GPON MAC SW reset */
	dbg_gpon_mac_set.Raw = IO_GREG(DBG_GPON_MAC_SET);
	dbg_gpon_mac_set.Bits.gpon_mac_sw_rst_n = GPON_MAC_SW_RESET_ENABLE;
	IO_SREG(DBG_GPON_MAC_SET, dbg_gpon_mac_set.Raw) ;
	udelay(GPON_MAC_SW_RESET_INTERVAL_US) ;
	dbg_gpon_mac_set.Bits.gpon_mac_sw_rst_n = GPON_MAC_SW_RESET_DISABLE;
	IO_SREG(DBG_GPON_MAC_SET, dbg_gpon_mac_set.Raw) ;
	spin_unlock_irqrestore(&gpGponPriv->swReset_lock, flag) ;

	return 0;
}

/******************************************************************************
 Descriptor:	GPON MAC resync.
 Input Args:	none.
 Ret Value: none.
******************************************************************************/
int gponDevSwResync(void)
{
	REG_DBG_SW_RESYNC swResync;

	swResync.Bits.dbg_sw_resync_en = GPON_SW_RESYNC_ENABLE;
	swResync.Bits.dbg_sw_resync = GPON_SW_RESYNC;
	IO_SREG(DBG_SW_RESYNC, swResync.Raw) ;
	
	udelay(GPON_MAC_SW_RESYNC_INTERVAL_US) ;
	
	swResync.Bits.dbg_sw_resync_en = GPON_SW_RESYNC_DISABLE;
	swResync.Bits.dbg_sw_resync = GPON_SW_NOT_RESYNC;
	IO_SREG(DBG_SW_RESYNC, swResync.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Set DBA backdoor enable.
 Input Args:	enable: enable/disable DBA backdoor.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetDbaBackdoor(XPON_Mode_t enable)
{
	REG_DBG_DBA_BACK_DOOR		dba_back_door; 		// 4340

	if(enable != XPON_DISABLE && enable != XPON_ENABLE) {
		return -1;
	}

	dba_back_door.Raw = IO_GREG(DBG_DBA_BACK_DOOR) ;
	dba_back_door.Bits.dbg_sw_rpt_dba_en = enable;
	IO_SREG(DBG_DBA_BACK_DOOR, dba_back_door.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Set DBA backdoor total buf size.
 Input Args:	total_buf_size: DBA backdoor total buf size.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetDbaBackdoorTotal(uint total_buf_size)
{
	REG_DBG_DBA_TOTAL_BUF_SIZE	dba_total_buf_size; // 4344

	dba_total_buf_size.Raw = IO_GREG(DBG_DBA_TOTAL_BUF_SIZE) ;
	dba_total_buf_size.Bits.dbg_dba_total_buf_size = total_buf_size;
	IO_SREG(DBG_DBA_TOTAL_BUF_SIZE, dba_total_buf_size.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Set DBA backdoor green size.
 Input Args:	total_buf_size: DBA backdoor green size.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetDbaBackdoorGreen(uint green_size)
{
	REG_DBG_DBA_GREEN_SIZE		dba_green_size; 	// 4348

	dba_green_size.Raw = IO_GREG(DBG_DBA_GREEN_SIZE) ;
	dba_green_size.Bits.dbg_dba_green_size = green_size;
	IO_SREG(DBG_DBA_GREEN_SIZE, dba_green_size.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Set DBA backdoor yellow size.
 Input Args:	total_buf_size: DBA backdoor yellow size.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetDbaBackdoorYellow(uint yellow_size)
{
	REG_DBG_DBA_YELLOW_SIZE		dba_yellow_size; 	//434C

	dba_yellow_size.Raw = IO_GREG(DBG_DBA_YELLOW_SIZE) ;
	dba_yellow_size.Bits.dbg_dba_yellow_size = yellow_size;
	IO_SREG(DBG_DBA_YELLOW_SIZE, dba_yellow_size.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Get DBA backdoor configure.
 Input Args:	dba_backdoor: used to stored DBA backdoor configure.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevGetDbaBackdoor(GPON_DEV_DBA_BACKDOOR_T *dba_backdoor)
{
	REG_DBG_DBA_BACK_DOOR		dba_back_door; 		// 4340
	REG_DBG_DBA_TOTAL_BUF_SIZE	dba_total_buf_size; // 4344
	REG_DBG_DBA_GREEN_SIZE		dba_green_size; 	// 4348
	REG_DBG_DBA_YELLOW_SIZE		dba_yellow_size; 	//434C

	dba_back_door.Raw = IO_GREG(DBG_DBA_BACK_DOOR) ;
	dba_total_buf_size.Raw = IO_GREG(DBG_DBA_TOTAL_BUF_SIZE) ;
	dba_green_size.Raw = IO_GREG(DBG_DBA_GREEN_SIZE) ;
	dba_yellow_size.Raw = IO_GREG(DBG_DBA_YELLOW_SIZE) ;

	dba_backdoor->enable =					dba_back_door.Bits.dbg_sw_rpt_dba_en;
	dba_backdoor->dba_backdoor_total_buf =	dba_total_buf_size.Bits.dbg_dba_total_buf_size;
	dba_backdoor->dba_backdoor_green_buf =	dba_green_size.Bits.dbg_dba_green_size;
	dba_backdoor->dba_backdoor_yellow_buf =	dba_yellow_size.Bits.dbg_dba_yellow_size;

	return 0;
}

/******************************************************************************
 Descriptor:	DBA slight modify.
 Input Args:	enable: enable/disable DBA slight modify.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetDbaSlightModify(XPON_Mode_t enable)
{
	REG_DBG_DBA_BACK_DOOR			dba_back_door; // 4340

	if(enable != XPON_DISABLE && enable != XPON_ENABLE) {
		return -1;
	}

	dba_back_door.Raw = IO_GREG(DBG_DBA_BACK_DOOR) ;
	dba_back_door.Bits.dbg_slight_modify_en = enable;
	IO_SREG(DBG_DBA_BACK_DOOR, dba_back_door.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Set DBA slight modify total size.
 Input Args:	yellow_size: used to set DBA slight modify total size.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetDbaSlightModifyTotal(ushort total_size)
{
	REG_DBG_SLIGHT_MODIFY_SIZE_2	slight_modify_size_2; // 4354

	slight_modify_size_2.Raw = IO_GREG(DBG_SLIGHT_MODIFY_SIZE_2) ;
	slight_modify_size_2.dbg_total_slight_modify_size = total_size;
	IO_SREG(DBG_SLIGHT_MODIFY_SIZE_2, slight_modify_size_2.Raw) ;

	return 0;
}


/******************************************************************************
 Descriptor:	Set DBA slight modify green size.
 Input Args:	yellow_size: used to set DBA slight modify green size.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetDbaSlightModifyGreen(ushort green_size)
{
	REG_DBG_SLIGHT_MODIFY_SIZE_1	slight_modify_size_1; // 4350

	slight_modify_size_1.Raw = IO_GREG(DBG_SLIGHT_MODIFY_SIZE_1) ;
	slight_modify_size_1.dbg_green_slight_modify_size = green_size;
	IO_SREG(DBG_SLIGHT_MODIFY_SIZE_1, slight_modify_size_1.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Set DBA slight modify yellow size.
 Input Args:	yellow_size: used to set DBA slight modify yellow size.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetDbaSlightModifyYellow(ushort yellow_size)
{
	REG_DBG_SLIGHT_MODIFY_SIZE_1	slight_modify_size_1; // 4350

	slight_modify_size_1.Raw = IO_GREG(DBG_SLIGHT_MODIFY_SIZE_1) ;
	slight_modify_size_1.dbg_yellow_slight_modify_size = yellow_size;
	IO_SREG(DBG_SLIGHT_MODIFY_SIZE_1, slight_modify_size_1.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Get DBA slight modify configure.
 Input Args:	dba_slightModify: used to store DBA slight modify configure.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevGetDbaSlightModify(GPON_DEV_SLIGHT_MODIFY_T *dba_slight_modify)
{
	REG_DBG_DBA_BACK_DOOR			dba_back_door; // 4340
	REG_DBG_SLIGHT_MODIFY_SIZE_1	slight_modify_size_1; // 4350
	REG_DBG_SLIGHT_MODIFY_SIZE_2	slight_modify_size_2; // 4354

	dba_back_door.Raw = IO_GREG(DBG_DBA_BACK_DOOR) ;
	slight_modify_size_1.Raw = IO_GREG(DBG_SLIGHT_MODIFY_SIZE_1) ;
	slight_modify_size_2.Raw = IO_GREG(DBG_SLIGHT_MODIFY_SIZE_2) ;

	dba_slight_modify->enable =							dba_back_door.Bits.dbg_slight_modify_en;
	dba_slight_modify->dba_slight_modify_total_buf =	slight_modify_size_2.dbg_total_slight_modify_size;
	dba_slight_modify->dba_slight_modify_green_buf =	slight_modify_size_1.dbg_green_slight_modify_size;
	dba_slight_modify->dba_slight_modify_yellow_buf =	slight_modify_size_1.dbg_yellow_slight_modify_size;

	return 0;
}

/******************************************************************************
 Descriptor:	DBA shift modify.
 Input Args:	enable: enable/disable DBA shift modify.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetDbaShiftModify(XPON_Mode_t enable)
{
	REG_DBG_DBA_BACK_DOOR			dba_back_door; // 4340

	if(enable != XPON_DISABLE && enable != XPON_ENABLE) {
		return -1;
	}

	dba_back_door.Raw = IO_GREG(DBG_DBA_BACK_DOOR) ;
	dba_back_door.Bits.dbg_shift_modify_en = enable;
	IO_SREG(DBG_DBA_BACK_DOOR, dba_back_door.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Set DBA shift modify total size.
 Input Args:	yellow_size: used to set DBA shift modify total size.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetDbaShiftModifyTotal(struct XMCS_DBAShiftMod_S *gponShiftTotal)
{
	REG_DBG_SLIGHT_MODIFY_SIZE_2	slight_modify_size_2; // 4354

	slight_modify_size_2.Raw = IO_GREG(DBG_SLIGHT_MODIFY_SIZE_2) ;
	slight_modify_size_2.dbg_total_shift_modify_direction = gponShiftTotal->direction;
	slight_modify_size_2.dbg_total_shift_modify_size = gponShiftTotal->size;
	IO_SREG(DBG_SLIGHT_MODIFY_SIZE_2, slight_modify_size_2.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Set DBA shift modify green size.
 Input Args:	yellow_size: used to set DBA shift modify green size.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetDbaShiftModifyGreen(struct XMCS_DBAShiftMod_S *gponShiftGreen)
{
	REG_DBG_SLIGHT_MODIFY_SIZE_1	slight_modify_size_1; // 4350

	slight_modify_size_1.Raw = IO_GREG(DBG_SLIGHT_MODIFY_SIZE_1) ;
	slight_modify_size_1.green_shift_dir = gponShiftGreen->direction;
	slight_modify_size_1.green_shift_size = gponShiftGreen->size;
	IO_SREG(DBG_SLIGHT_MODIFY_SIZE_1, slight_modify_size_1.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Set DBA shift modify yellow size.
 Input Args:	yellow_size: used to set DBA shift modify yellow size.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetDbaShiftModifyYellow(struct XMCS_DBAShiftMod_S *gponShiftYellow)
{
	REG_DBG_SLIGHT_MODIFY_SIZE_1	slight_modify_size_1; // 4350

	slight_modify_size_1.Raw = IO_GREG(DBG_SLIGHT_MODIFY_SIZE_1) ;
	slight_modify_size_1.yellow_shift_dir = gponShiftYellow->direction;
	slight_modify_size_1.yellow_shift_size = gponShiftYellow->size;
	IO_SREG(DBG_SLIGHT_MODIFY_SIZE_1, slight_modify_size_1.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Set BWMAP stop time interrupt bit BFB6400C[30].
 Input Args:	enable: XPON_ENABLE:  set BWMAP stop time interrupt bit.
 					XPON_DISABLE: clear BWMAP stop time interrupt bit.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetBwmStopTimeInt(XPON_Mode_t enable)
{
	REG_G_INT_ENABLE g_int_enable ;
	REG_G_INT_STATUS g_int_status ;

	if(enable != XPON_DISABLE && enable != XPON_ENABLE) {
		return -1;
	}

	g_int_enable.Raw = IO_GREG(G_INT_ENABLE) ;
	
	if(enable == XPON_ENABLE) {
		/* clear BwmStopTimeInt status */
		g_int_status.Raw = IO_GREG(G_INT_STATUS) ;
		g_int_status.Bits.bwm_stop_time_err_int = 1 ;
		IO_SREG(G_INT_STATUS, g_int_status.Raw) ;
	} 

	/* enable */
	g_int_enable.Bits.bwm_stop_time_err_int_en = enable;
	IO_SREG(G_INT_ENABLE, g_int_enable.Raw) ;

	return 0 ;
}

/******************************************************************************
 Descriptor:	Set enable/disable tx data length aligned multiple of 4
 Input Args:	enable: enable/disable tx data length aligned multiple of 4.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetTx4bytesAlign(XPON_Mode_t enable)
{
	REG_DBG_GPON_MAC_SET dbg_gpon_mac_set;

	if(enable != XPON_DISABLE && enable != XPON_ENABLE) {
		return -1;
	}

	dbg_gpon_mac_set.Raw = IO_GREG(DBG_GPON_MAC_SET) ;
	dbg_gpon_mac_set.Bits.asb_tx_eth_4byte_align_en = enable;

	/* enable */
	IO_SREG(DBG_GPON_MAC_SET, dbg_gpon_mac_set.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Get status of tx data length aligned multiple of 4
 Input Args:	enable: status of tx data length aligned multiple of 4.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevGetTx4bytesAlign(XPON_Mode_t *enable)
{
	REG_DBG_GPON_MAC_SET dbg_gpon_mac_set;

	dbg_gpon_mac_set.Raw = IO_GREG(DBG_GPON_MAC_SET) ;
	*enable = dbg_gpon_mac_set.Bits.asb_tx_eth_4byte_align_en;

	return 0;
}

/******************************************************************************
 Descriptor:	Get tx sync offset
 Input Args:	tx_sync: status of tx_sync.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevGetTxSyncOffset(unchar *tx_sync)
{
	REG_DBG_TX_SYNC_OFFSET tx_sync_offset;

	tx_sync_offset.Raw = IO_GREG(DBG_TX_SYNC_OFFSET) ;
	*tx_sync = tx_sync_offset.Bits.dbg_tx_sync_offset;

	return 0;
}

/******************************************************************************
 Descriptor:	Get o3 and o4 PLOAM control status
 Input Args:	sel: o3 and o4 PLOAM control status.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevGetO3O4PloamCtrl(GPON_SW_HW_SELECT_T *sel)
{
	REG_O3_O4_PLOAMU_CTRL o3_o4_ploamu_ctrl;

	o3_o4_ploamu_ctrl.Raw = IO_GREG(O3_O4_PLOAMU_CTRL) ;
	*sel = (o3_o4_ploamu_ctrl.Bits.o3_o4_ploamu_ctrl == 0) ? GPON_HW : GPON_SW;

	return 0;
}

/******************************************************************************
 Descriptor:	Set o3 and o4 PLOAM control status
 Input Args:	sel: Set o3 and o4 PLOAM control.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetO3O4PloamCtrl(GPON_SW_HW_SELECT_T sel)
{
	REG_O3_O4_PLOAMU_CTRL o3_o4_ploamu_ctrl;

	o3_o4_ploamu_ctrl.Raw = IO_GREG(O3_O4_PLOAMU_CTRL) ;
	o3_o4_ploamu_ctrl.Bits.o3_o4_ploamu_ctrl = (sel == GPON_HW) ? 0 : 1;
	IO_SREG(O3_O4_PLOAMU_CTRL, o3_o4_ploamu_ctrl.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Set tcont info between tcont16 to tcont31 for EN7521.
 Input Args:	isValid:        tcont valid or invalid.
                 tcont_index: tcont index from 16 to 31.
                 allocId:        tcont alloc ID.
 Ret Value:	GPON_TCONT_CMD_SUCCESS: Success.
               	GPON_TCONT_CMD_FAIL:        HW has something error.
******************************************************************************/
static GPON_TCONT_CMD_RESULT_t gponDevSetTCont16_31(GPON_TCONT_t isValid, int tcont_index, ushort allocId)
{
	REG_G_TCONT_ID_16_31_CFG gponTcontCfg;
	REG_G_TCONT_ID_16_31_STS gponTcontSts;
	
	int RETRY = 3000;

	if(tcont_index < 16 || tcont_index > 31)
		return GPON_TCONT_CMD_FAIL;
	
	/* setting cfg */
	gponTcontCfg.Bits.tcont_cmd = GPON_TCONT_WRITE;
	gponTcontCfg.Bits.wr_tcont_id_vld = isValid;
	gponTcontCfg.Bits.tcont_id_index = tcont_index - 16;
	gponTcontCfg.Bits.wr_tcont_id = allocId;
	IO_SREG(G_TCONT_ID_16_31_CFG, gponTcontCfg.Raw) ;

	/* Wait for done */
	while(RETRY--) {
		gponTcontSts.Raw = IO_GREG(G_TCONT_ID_16_31_STS) ;
		if(gponTcontSts.Bits.tcont_cmd_done) {
			return GPON_TCONT_CMD_SUCCESS;
		} 
		udelay(1) ;
	}

	PON_MSG(MSG_ERR, "[%s:%s]\n  Fail to set tcont:%d.", strrchr(__FILE__, '/') + 1, __func__, tcont_index) ;
	return GPON_TCONT_CMD_FAIL;
}

/******************************************************************************
 Descriptor:	Get tcont info between tcont16 to tcont31 for EN7521.
 Input Args:	isValid:        tcont valid or invalid.
                 tcont_index: tcont index from 16 to 31.
                 allocId:        tcont alloc ID.
                 gponTcontSts:        BFB64184 register structure.
 Ret Value:	GPON_TCONT_CMD_SUCCESS: Success.
               	GPON_TCONT_CMD_FAIL:        HW has something error.
******************************************************************************/
static GPON_TCONT_CMD_RESULT_t gponDevGetTCont16_31(GPON_TCONT_t *isValid, int tcont_index, ushort *allocId)
{
	REG_G_TCONT_ID_16_31_CFG gponTcontCfg;
	REG_G_TCONT_ID_16_31_STS gponTcontSts;
	
	int RETRY = 3000;

	if(tcont_index < 16 || tcont_index > 31)
		return GPON_TCONT_CMD_FAIL;
	
	/* setting cfg */
	gponTcontCfg.Bits.tcont_cmd = GPON_TCONT_READ;
	gponTcontCfg.Bits.tcont_id_index = tcont_index - 16;
	IO_SREG(G_TCONT_ID_16_31_CFG, gponTcontCfg.Raw) ;

	/* Wait for done */
	while(RETRY--) {
		gponTcontSts.Raw = IO_GREG(G_TCONT_ID_16_31_STS) ;
		if(gponTcontSts.Bits.tcont_cmd_done) {
			*isValid = gponTcontSts.Bits.rd_tcont_id_vld;
			*allocId = gponTcontSts.Bits.rd_tcont_id;
			return GPON_TCONT_CMD_SUCCESS;
		} 
		udelay(1) ;
	}

	PON_MSG(MSG_ERR, "[%s:%s]\n  Fail to get tcont:%d.", strrchr(__FILE__, '/') + 1, __func__, tcont_index) ;
	return GPON_TCONT_CMD_FAIL;
}

/******************************************************************************
 Descriptor:	Set enable/disable filter Upstream Overhead PLOAM at O5 state.
 Input Args:	XPON_ENABLE or XPON_DISABLE.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetFilterUpstreamOverheadPLOAM(XPON_Mode_t enable)
{
	REG_DBG_PLOAMD_FILTER_IN_O5 ploamd_filter_in_o5;

	ploamd_filter_in_o5.Raw = IO_GREG(DBG_PLOAMD_FILTER_IN_O5) ;
	ploamd_filter_in_o5.Bits.dbg_us_overhead_filter_in_o5_en = (enable == XPON_ENABLE) ? 1 : 0;
	IO_SREG(DBG_PLOAMD_FILTER_IN_O5, ploamd_filter_in_o5.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Get status of filter Upstream Overhead PLOAM at O5 state.
 Input Args:	XPON_ENABLE or XPON_DISABLE.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevGetFilterUpstreamOverheadPLOAM(XPON_Mode_t *enable)
{
	REG_DBG_PLOAMD_FILTER_IN_O5 ploamd_filter_in_o5;

	ploamd_filter_in_o5.Raw = IO_GREG(DBG_PLOAMD_FILTER_IN_O5) ;
	*enable = ploamd_filter_in_o5.Bits.dbg_us_overhead_filter_in_o5_en;

	return 0;
}

/******************************************************************************
 Descriptor:	Set enable/disable filter extented burst length PLOAM at O5 state.
 Input Args:	enable: XPON_ENABLE or XPON_DISABLE.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetFilterExtBurstLengthPLOAM(XPON_Mode_t enable)
{
	REG_DBG_PLOAMD_FILTER_IN_O5 ploamd_filter_in_o5;

	ploamd_filter_in_o5.Raw = IO_GREG(DBG_PLOAMD_FILTER_IN_O5) ;
	ploamd_filter_in_o5.Bits.dbg_ext_bst_len_filter_in_o5_en = (enable == XPON_ENABLE) ? 1 : 0;
	IO_SREG(DBG_PLOAMD_FILTER_IN_O5, ploamd_filter_in_o5.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Get status of filter extented burst length PLOAM at O5 state.
 Input Args:	enable: XPON_ENABLE or XPON_DISABLE.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevGetFilterExtBurstLengthPLOAM(XPON_Mode_t *enable)
{
	REG_DBG_PLOAMD_FILTER_IN_O5 ploamd_filter_in_o5;

	ploamd_filter_in_o5.Raw = IO_GREG(DBG_PLOAMD_FILTER_IN_O5) ;
	*enable = ploamd_filter_in_o5.Bits.dbg_ext_bst_len_filter_in_o5_en;

	return 0;
}

#endif

#ifdef TCSUPPORT_CPU_EN7521
/******************************************************************************
 Descriptor:	Set the high width of a 1pps pluse.
 Input Args:	width: The high width of a 1pps pluse. The unit is microsecond.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSet1ppsHighWidth(uint width)
{
	REG_G_TOD_CLK_PERIOD tod_clk_period;  //40e4
	REG_TOD_1PPS_WD_CTRL tod_1pps_wd_ctrl; //43c8

	if (!(isEN7526c || isEN751627) )
		return -1;

	/* width can not fill 1pps */
	if(width >= 1000000)
		return -1;

	/* change to nanosecond */
	width *= 1000;

	tod_clk_period.Raw = IO_GREG(G_TOD_CLK_PERIOD) ;

	tod_1pps_wd_ctrl.Bits.tod_1pps_width_ctrl = width / tod_clk_period.Bits.tod_period;
	IO_SREG(TOD_1PPS_WD_CTRL, tod_1pps_wd_ctrl.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Get the high width of a 1pps pluse.
 Input Args:	width: The high width of a 1pps pluse. The unit is microsecond.
 Ret Value:		0: Success.
******************************************************************************/
int gponDevGet1ppsHighWidth(uint *width)
{
	REG_G_TOD_CLK_PERIOD tod_clk_period;  //40e4
	REG_TOD_1PPS_WD_CTRL tod_1pps_wd_ctrl; //43c8

	if (!(isEN7526c || isEN751627) )
		return 0;

	tod_clk_period.Raw = IO_GREG(G_TOD_CLK_PERIOD) ;
	tod_1pps_wd_ctrl.Raw = IO_GREG(TOD_1PPS_WD_CTRL) ;

	*width = tod_1pps_wd_ctrl.Bits.tod_1pps_width_ctrl * tod_clk_period.Bits.tod_period;

	/* change to milisecond */
	*width /= 1000;
	
	return 0;
}

/******************************************************************************
 Descriptor:	Set the the upstream send PLOAMu behavior.
 Input Args:	mode: The upstream send PLOAMu behavior.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetSendPloamuWaitMode(GPON_DEV_SEND_PLOAMU_WAIT_MODE_T mode)
{
	REG_O3_O4_PLOAMU_CTRL ploamu_ctrl;  //43c4

	if (!(isEN7526c || isEN751627) )
		return 0;

	ploamu_ctrl.Raw = IO_GREG(O3_O4_PLOAMU_CTRL) ;
	ploamu_ctrl.Bits.ploamu_ind_ctrl = mode;
	IO_SREG(PLOAMU_CTRL, ploamu_ctrl.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	Get the the upstream send PLOAMu behavior.
 Input Args:	mode: The upstream send PLOAMu behavior.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevGetSendPloamuWaitMode(GPON_DEV_SEND_PLOAMU_WAIT_MODE_T *mode)
{
	REG_O3_O4_PLOAMU_CTRL ploamu_ctrl;  //43c4

	if (!(isEN7526c || isEN751627) )
		return 0;

	ploamu_ctrl.Raw = IO_GREG(O3_O4_PLOAMU_CTRL) ;
	*mode = (GPON_DEV_SEND_PLOAMU_WAIT_MODE_T)ploamu_ctrl.Bits.ploamu_ind_ctrl;

	return 0;
}
#endif

/******************************************************************************
 Descriptor:	Set the period of ToD clock, in unit of nano second.
 Input Args:	period: in unit of nano second.
 Ret Value: 0: success
******************************************************************************/
int gponDevSetTodClkPeriod(ushort period)
{
	REG_G_TOD_CLK_PERIOD todClkPeroid;

	todClkPeroid.Raw = IO_GREG(G_TOD_CLK_PERIOD);
	todClkPeroid.Bits.tod_period = period;
	IO_SREG(G_TOD_CLK_PERIOD, todClkPeroid.Raw);

	return 0;
}


/******************************************************************************
 Descriptor:	Set interrupt mask.
 Input Args:	mask: used to set interrupt mask.
 Ret Value: none.
******************************************************************************/
int gponDevSetIntMask(uint mask)
{
	IO_SREG(G_INT_ENABLE, mask);

	return 0;
}

/******************************************************************************
 Descriptor:	Get interrupt mask.
 Input Args:	mask: used to stored interrupt mask.
 Ret Value: none.
******************************************************************************/
int gponDevGetIntMask(uint *mask)
{
	REG_G_INT_ENABLE int_enable;

	int_enable.Raw = IO_GREG(G_INT_ENABLE);
	*mask = int_enable.Raw;

	return 0;
}

/******************************************************************************
 Descriptor:	Set tcont info between tcont0 to tcont15.
 Input Args:	isValid:        tcont valid or invalid.
                 tcont_index: tcont index from 0 to 15.
                 allocId:        tcont alloc ID.
 Ret Value:	GPON_TCONT_CMD_SUCCESS: Success.
               	GPON_TCONT_CMD_FAIL:        HW has something error.
******************************************************************************/
static GPON_TCONT_CMD_RESULT_t gponDevSetTCont0_15(GPON_TCONT_t isValid, int tcont_index, ushort allocId)
{
	REG_G_TCONT_ID_0_1 gponTCont0 ;
	REG_G_TCONT_ID_2_3 gponTCont2 ;
	REG_G_TCONT_ID_4_5 gponTCont4 ;
	REG_G_TCONT_ID_6_7 gponTCont6 ;
	REG_G_TCONT_ID_8_9 gponTCont8 ;
	REG_G_TCONT_ID_10_11 gponTCont10 ;
	REG_G_TCONT_ID_12_13 gponTCont12 ;
	REG_G_TCONT_ID_14_15 gponTCont14 ;

	if(tcont_index < 0 || tcont_index > 15) {
		return GPON_TCONT_CMD_FAIL;
	}

	switch(tcont_index) {
		case 0:
			gponTCont0.Raw = IO_GREG(G_TCONT_ID_0_1) ;
			gponTCont0.Bits.t_cont0_vld = isValid ;
			gponTCont0.Bits.t_cont0_id = allocId ;
			IO_SREG(G_TCONT_ID_0_1, gponTCont0.Raw) ;
			break;
		case 1:
			gponTCont0.Raw = IO_GREG(G_TCONT_ID_0_1) ;
			gponTCont0.Bits.t_cont1_vld = isValid ;
			gponTCont0.Bits.t_cont1_id = allocId ;
			IO_SREG(G_TCONT_ID_0_1, gponTCont0.Raw) ;
			break;
		case 2:
			gponTCont2.Raw = IO_GREG(G_TCONT_ID_2_3) ;
			gponTCont2.Bits.t_cont2_vld = isValid ;
			gponTCont2.Bits.t_cont2_id = allocId ;
			IO_SREG(G_TCONT_ID_2_3, gponTCont2.Raw) ;
			break;
		case 3:
			gponTCont2.Raw = IO_GREG(G_TCONT_ID_2_3) ;
			gponTCont2.Bits.t_cont3_vld = isValid ;
			gponTCont2.Bits.t_cont3_id = allocId ;
			IO_SREG(G_TCONT_ID_2_3, gponTCont2.Raw) ;
			break;
		case 4:
			gponTCont4.Raw = IO_GREG(G_TCONT_ID_4_5) ;
			gponTCont4.Bits.t_cont4_vld = isValid ;
			gponTCont4.Bits.t_cont4_id = allocId ;
			IO_SREG(G_TCONT_ID_4_5, gponTCont4.Raw) ;
			break;
		case 5:
			gponTCont4.Raw = IO_GREG(G_TCONT_ID_4_5) ;
			gponTCont4.Bits.t_cont5_vld = isValid ;
			gponTCont4.Bits.t_cont5_id = allocId ;
			IO_SREG(G_TCONT_ID_4_5, gponTCont4.Raw) ;
			break;
		case 6:
			gponTCont6.Raw = IO_GREG(G_TCONT_ID_6_7) ;
			gponTCont6.Bits.t_cont6_vld = isValid ;
			gponTCont6.Bits.t_cont6_id = allocId ;
			IO_SREG(G_TCONT_ID_6_7, gponTCont6.Raw) ;
			break;
		case 7:
			gponTCont6.Raw = IO_GREG(G_TCONT_ID_6_7) ;
			gponTCont6.Bits.t_cont7_vld = isValid ;
			gponTCont6.Bits.t_cont7_id = allocId ;
			IO_SREG(G_TCONT_ID_6_7, gponTCont6.Raw) ;
			break;
		case 8:
			gponTCont8.Raw = IO_GREG(G_TCONT_ID_8_9) ;
			gponTCont8.Bits.t_cont8_vld = isValid ;
			gponTCont8.Bits.t_cont8_id = allocId ;
			IO_SREG(G_TCONT_ID_8_9, gponTCont8.Raw) ;
			break;
		case 9:
			gponTCont8.Raw = IO_GREG(G_TCONT_ID_8_9) ;
			gponTCont8.Bits.t_cont9_vld = isValid ;
			gponTCont8.Bits.t_cont9_id = allocId ;
			IO_SREG(G_TCONT_ID_8_9, gponTCont8.Raw) ;
			break;
		case 10:
			gponTCont10.Raw = IO_GREG(G_TCONT_ID_10_11) ;
			gponTCont10.Bits.t_cont10_vld = isValid ;
			gponTCont10.Bits.t_cont10_id = allocId ;
			IO_SREG(G_TCONT_ID_10_11, gponTCont10.Raw) ;
			break;
		case 11:
			gponTCont10.Raw = IO_GREG(G_TCONT_ID_10_11) ;
			gponTCont10.Bits.t_cont11_vld = isValid ;
			gponTCont10.Bits.t_cont11_id = allocId ;
			IO_SREG(G_TCONT_ID_10_11, gponTCont10.Raw) ;
			break;
		case 12:
			gponTCont12.Raw = IO_GREG(G_TCONT_ID_12_13) ;
			gponTCont12.Bits.t_cont12_vld = isValid ;
			gponTCont12.Bits.t_cont12_id = allocId ;
			IO_SREG(G_TCONT_ID_12_13, gponTCont12.Raw) ;
			break;
		case 13:
			gponTCont12.Raw = IO_GREG(G_TCONT_ID_12_13) ;
			gponTCont12.Bits.t_cont13_vld = isValid ;
			gponTCont12.Bits.t_cont13_id = allocId ;
			IO_SREG(G_TCONT_ID_12_13, gponTCont12.Raw) ;
			break;
		case 14:
			gponTCont14.Raw = IO_GREG(G_TCONT_ID_14_15) ;
			gponTCont14.Bits.t_cont14_vld = isValid ;
			gponTCont14.Bits.t_cont14_id = allocId ;
			IO_SREG(G_TCONT_ID_14_15, gponTCont14.Raw) ;
			break;
		case 15:
			gponTCont14.Raw = IO_GREG(G_TCONT_ID_14_15) ;
			gponTCont14.Bits.t_cont15_vld = isValid ;
			gponTCont14.Bits.t_cont15_id = allocId ;
			IO_SREG(G_TCONT_ID_14_15, gponTCont14.Raw) ;
			break;
	}

	return GPON_TCONT_CMD_SUCCESS;
}

/******************************************************************************
 Descriptor:	Get tcont info between tcont0 to tcont15.
 Input Args:	isValid:        tcont valid or invalid.
                 tcont_index: tcont index from 0 to 15.
                 allocId:        tcont alloc ID.
 Ret Value:	GPON_TCONT_CMD_SUCCESS: Success.
               	GPON_TCONT_CMD_FAIL:        HW has something error.
******************************************************************************/
static GPON_TCONT_CMD_RESULT_t gponDevGetTCont0_15(GPON_TCONT_t *isValid, int tcont_index, ushort *allocId)
{
	REG_G_TCONT_ID_0_1 gponTCont0 ;
	REG_G_TCONT_ID_2_3 gponTCont2 ;
	REG_G_TCONT_ID_4_5 gponTCont4 ;
	REG_G_TCONT_ID_6_7 gponTCont6 ;
	REG_G_TCONT_ID_8_9 gponTCont8 ;
	REG_G_TCONT_ID_10_11 gponTCont10 ;
	REG_G_TCONT_ID_12_13 gponTCont12 ;
	REG_G_TCONT_ID_14_15 gponTCont14 ;

	if(tcont_index < 0 || tcont_index > 15) {
		return GPON_TCONT_CMD_FAIL;
	}

	switch(tcont_index) {
		case 0:
			gponTCont0.Raw = IO_GREG(G_TCONT_ID_0_1) ;
			*isValid = gponTCont0.Bits.t_cont0_vld ;
			*allocId = gponTCont0.Bits.t_cont0_id;
			break;
		case 1:
			gponTCont0.Raw = IO_GREG(G_TCONT_ID_0_1) ;
			*isValid = gponTCont0.Bits.t_cont1_vld ;
			*allocId = gponTCont0.Bits.t_cont1_id;
			break;
		case 2:
			gponTCont2.Raw = IO_GREG(G_TCONT_ID_2_3) ;
			*isValid = gponTCont2.Bits.t_cont2_vld ;
			*allocId = gponTCont2.Bits.t_cont2_id;
			break;
		case 3:
			gponTCont2.Raw = IO_GREG(G_TCONT_ID_2_3) ;
			*isValid = gponTCont2.Bits.t_cont3_vld ;
			*allocId = gponTCont2.Bits.t_cont3_id;
			break;
		case 4:
			gponTCont4.Raw = IO_GREG(G_TCONT_ID_4_5) ;
			*isValid = gponTCont4.Bits.t_cont4_vld ;
			*allocId = gponTCont4.Bits.t_cont4_id;
			break;
		case 5:
			gponTCont4.Raw = IO_GREG(G_TCONT_ID_4_5) ;
			*isValid = gponTCont4.Bits.t_cont5_vld ;
			*allocId = gponTCont4.Bits.t_cont5_id;
			break;
		case 6:
			gponTCont6.Raw = IO_GREG(G_TCONT_ID_6_7) ;
			*isValid = gponTCont6.Bits.t_cont6_vld ;
			*allocId = gponTCont6.Bits.t_cont6_id;
			break;
		case 7:
			gponTCont6.Raw = IO_GREG(G_TCONT_ID_6_7) ;
			*isValid = gponTCont6.Bits.t_cont7_vld ;
			*allocId = gponTCont6.Bits.t_cont7_id;
			break;
		case 8:
			gponTCont8.Raw = IO_GREG(G_TCONT_ID_8_9) ;
			*isValid = gponTCont8.Bits.t_cont8_vld ;
			*allocId = gponTCont8.Bits.t_cont8_id;
			break;
		case 9:
			gponTCont8.Raw = IO_GREG(G_TCONT_ID_8_9) ;
			*isValid = gponTCont8.Bits.t_cont9_vld ;
			*allocId = gponTCont8.Bits.t_cont9_id;
			break;
		case 10:
			gponTCont10.Raw = IO_GREG(G_TCONT_ID_10_11) ;
			*isValid = gponTCont10.Bits.t_cont10_vld ;
			*allocId = gponTCont10.Bits.t_cont10_id;
			break;
		case 11:
			gponTCont10.Raw = IO_GREG(G_TCONT_ID_10_11) ;
			*isValid = gponTCont10.Bits.t_cont11_vld ;
			*allocId = gponTCont10.Bits.t_cont11_id;
			break;
		case 12:
			gponTCont12.Raw = IO_GREG(G_TCONT_ID_12_13) ;
			*isValid = gponTCont12.Bits.t_cont12_vld ;
			*allocId = gponTCont12.Bits.t_cont12_id;
			break;
		case 13:
			gponTCont12.Raw = IO_GREG(G_TCONT_ID_12_13) ;
			*isValid = gponTCont12.Bits.t_cont13_vld ;
			*allocId = gponTCont12.Bits.t_cont13_id;
			break;
		case 14:
			gponTCont14.Raw = IO_GREG(G_TCONT_ID_14_15) ;
			*isValid = gponTCont14.Bits.t_cont14_vld ;
			*allocId = gponTCont14.Bits.t_cont14_id;
			break;
		case 15:
			gponTCont14.Raw = IO_GREG(G_TCONT_ID_14_15) ;
			*isValid = gponTCont14.Bits.t_cont15_vld ;
			*allocId = gponTCont14.Bits.t_cont15_id;
			break;
	}

	return GPON_TCONT_CMD_SUCCESS;
}

/******************************************************************************
 Descriptor:	Set tcont info.
 Input Args:	tcont_index: tcont index from 0 to (CONFIG_GPON_MAX_TCONT - 1).
                 tcont valid or invalid.
                 allocId:        tcont alloc ID.
 Ret Value:	GPON_TCONT_CMD_SUCCESS: Success.
               	GPON_TCONT_CMD_FAIL:        HW has something error.
******************************************************************************/
GPON_TCONT_CMD_RESULT_t gponDevSetTCont(GPON_TCONT_t isValid, int tcont_index, ushort allocId)
{
	GPON_TCONT_CMD_RESULT_t retVal = GPON_TCONT_CMD_FAIL;
	
	if(tcont_index < 0 || tcont_index >= CONFIG_GPON_MAX_TCONT) {
		return GPON_TCONT_CMD_FAIL;
	}

	if(tcont_index < 16) {
		retVal = gponDevSetTCont0_15(isValid, tcont_index, allocId);
	} else {
#ifdef TCSUPPORT_CPU_EN7521
		retVal = gponDevSetTCont16_31(isValid, tcont_index, allocId);
#endif
	}

	return retVal;
}

/******************************************************************************
 Descriptor:	Get tcont info.
 Input Args:	tcont_index: tcont index from 0 to (CONFIG_GPON_MAX_TCONT - 1).
                 tcont valid or invalid.
                 allocId:        tcont alloc ID.
 Ret Value:	GPON_TCONT_CMD_SUCCESS: Success.
               	GPON_TCONT_CMD_FAIL:        HW has something error.
******************************************************************************/
GPON_TCONT_CMD_RESULT_t gponDevGetTCont(GPON_TCONT_t *isValid, int tcont_index, ushort *allocId)
{
	GPON_TCONT_CMD_RESULT_t retVal = GPON_TCONT_CMD_FAIL;
	
	if(tcont_index < 0 || tcont_index >= CONFIG_GPON_MAX_TCONT) {
		return GPON_TCONT_CMD_FAIL;
	}

	if(tcont_index < 16) {
		retVal = gponDevGetTCont0_15(isValid, tcont_index, allocId);
	} else {
#ifdef TCSUPPORT_CPU_EN7521
		retVal = gponDevGetTCont16_31(isValid, tcont_index, allocId);
#endif
	}

	return retVal;
}

/*****************************************************************************
******************************************************************************/
int gponDevDisableTCont(ushort allocId)
{
	int i;
	GPON_TCONT_t valid;
	ushort id;
	GPON_TCONT_CMD_RESULT_t result;

	for(i = 0 ; i < CONFIG_GPON_MAX_TCONT ; i++) {
		/* read tcont status */
		result = gponDevGetTCont(&valid, i, &id);
		if(result == GPON_TCONT_CMD_SUCCESS) {
			if(id == allocId) {
				/* match allocId */
				result = gponDevSetTCont(GPON_TCONT_INVALID, i, 0xFF);
				if(result == GPON_TCONT_CMD_SUCCESS) {
					return (i) ;
				} else {
					return -EFAULT ;
				}
			}
		} else {
			return -EFAULT ;
		}
	}

	return -EFAULT ;
} 

/*****************************************************************************
******************************************************************************/
int gponDevEnableTCont(ushort allocId)
{
	int i;
	GPON_TCONT_t valid;
	ushort id;
	GPON_TCONT_CMD_RESULT_t result;
	
	if(allocId == GPON_ONU_ID) {
		return 0 ;
	} 
	
	/* Disable the alloc ID if the alloc ID is exist */
	gponDevDisableTCont(allocId) ;

	/* Because the TCONT 0 is an shadow of ONU ID ,
	 * So the i start value is 1.
	 */
	for(i = 1 ; i < CONFIG_GPON_MAX_TCONT ; i++) {
		/* read tcont status */
		result = gponDevGetTCont(&valid, i, &id);
		if(result == GPON_TCONT_CMD_SUCCESS) {
			if(valid == GPON_TCONT_INVALID) {
				result = gponDevSetTCont(GPON_TCONT_VALID, i, allocId);
				if(result == GPON_TCONT_CMD_SUCCESS) {
					return (i) ;
				} else {
					return -EFAULT ;
				}
			}
		} else {
			return -EFAULT ;
		}
	}

	return -EFAULT ;
} 

/******************************************************************************
 Descriptor:	dump GPON MAC control status register.
 Input Args:	none.
 Ret Value:		0: Success.
             -EFAULT: Get gemInfo error.
******************************************************************************/
int gponDevDumpCsr(void)
{	
	printk("0x4000: ONU ID                                                        :0x%.8x\n", IO_GREG(G_ONU_ID)) ;
	printk("0x4004: ONU GLOBAL CONFIGURATION REGISTER                             :0x%.8x\n", IO_GREG(G_GBL_CFG)) ;
	printk("0x4008: GPON MAC INTERRUPTS STATUS                                    :0x%.8x\n", IO_GREG(G_INT_STATUS)) ;
	printk("0x400C: GPON MAC INTERRUPTS ENABLE                                    :0x%.8x\n", IO_GREG(G_INT_ENABLE)) ;

	printk("0x4020: The Alloc-ID of T-CONT0 and T-CONT1                           :0x%.8x\n", IO_GREG(G_TCONT_ID_0_1)) ;
	printk("0x4024: The Alloc-ID of T-CONT2 and T-CONT3                           :0x%.8x\n", IO_GREG(G_TCONT_ID_2_3)) ;
	printk("0x4028: The Alloc-ID of T-CONT4 and T-CONT5                           :0x%.8x\n", IO_GREG(G_TCONT_ID_4_5)) ;
	printk("0x402C: The Alloc-ID of T-CONT6 and T-CONT7                           :0x%.8x\n", IO_GREG(G_TCONT_ID_6_7)) ;
	printk("0x4030: The Alloc-ID of T-CONT8 and T-CONT9                           :0x%.8x\n", IO_GREG(G_TCONT_ID_8_9)) ;
	printk("0x4034: The Alloc-ID of T-CONT10 and T-CONT11                         :0x%.8x\n", IO_GREG(G_TCONT_ID_10_11)) ;
	printk("0x4038: The Alloc-ID of T-CONT12 and T-CONT13                         :0x%.8x\n", IO_GREG(G_TCONT_ID_12_13)) ;
	printk("0x403C: The Alloc-ID of T-CONT14 and T-CONT15                         :0x%.8x\n", IO_GREG(G_TCONT_ID_14_15)) ;
	//printk("0x4040: GEM PORT ID COFIGURATION                                      :0x%.8x\n", IO_GREG(G_GEM_PORT_CFG)) ;
	//printk("0x4044: GEM PORT ID STATUS                                            :0x%.8x\n", IO_GREG(G_GEM_PORT_STS)) ;
	printk("0x4048: OMCI GEM PORT ID                                              :0x%.8x\n", IO_GREG(G_OMCI_ID)) ;
	printk("0x404C: GEM TABLE INITIALIZATION CONTROL                              :0x%.8x\n", IO_GREG(G_GEM_TBL_INIT)) ;
	printk("0x4050: PLOAMu FIFO STATUS                                            :0x%.8x\n", IO_GREG(G_PLOAMu_FIFO_STS)) ;
	//printk("0x4054: WRITE DATA for PLOAMu FIFO                                    :0x%.8x\n", IO_GREG(G_PLOAMu_WDATA)) ;
	printk("0x4058: PLOAMd FIFO STATUS                                            :0x%.8x\n", IO_GREG(G_PLOAMd_FIFO_STS)) ;
	//printk("0x405C: READ DATA from PLOAMd FIFO                                    :0x%.8x\n", IO_GREG(G_PLOAMd_RDATA)) ;
	printk("0x4060: SUPER FRAME COUNTER FOR AES KEY CHANGE                        :0x%.8x\n", IO_GREG(G_AES_CFG)) ;
	printk("0x4064: ACTIVE AES KEY0                                               :0x%.8x\n", IO_GREG(G_AES_ACTIVE_KEY0)) ;
	printk("0x4068: ACTIVE AES KEY1                                               :0x%.8x\n", IO_GREG(G_AES_ACTIVE_KEY1)) ;
	printk("0x406C: ACTIVE AES KEY2                                               :0x%.8x\n", IO_GREG(G_AES_ACTIVE_KEY2)) ;
	printk("0x4070: ACTIVE AES KEY3                                               :0x%.8x\n", IO_GREG(G_AES_ACTIVE_KEY3)) ;
	printk("0x4074: AES KEY in SHADOW REGISTER0                                   :0x%.8x\n", IO_GREG(G_AES_SHADOW_KEY0)) ;
	printk("0x4078: AES KEY in SHADOW REGISTER1                                   :0x%.8x\n", IO_GREG(G_AES_SHADOW_KEY1)) ;
	printk("0x407C: AES KEY in SHADOW REGISTER2                                   :0x%.8x\n", IO_GREG(G_AES_SHADOW_KEY2)) ;
	printk("0x4080: AES KEY in SHADOW REGISTER3                                   :0x%.8x\n", IO_GREG(G_AES_SHADOW_KEY3)) ;
#ifdef TCSUPPORT_CPU_EN7521
	printk("0x4084: AES KEY SWITCH BY SOFTWARE                                    :0x%.8x\n", IO_GREG(G_AES_KEY_SWITCH_BY_SW)) ;
#endif

	printk("0x4090: PHYSICAL LAYER OVERHEAD TIME                                  :0x%.8x\n", IO_GREG(G_PLOu_OVERHEAD)) ;
	printk("0x4094: NUMBER OF GUARD BITS                                          :0x%.8x\n", IO_GREG(G_PLOu_GUARD_BIT)) ;
	printk("0x4098: NUMBER OF TYPE1 and TYPE2 PREAMBLE BITS                       :0x%.8x\n", IO_GREG(G_PLOu_PRMBL_TYPE1_2)) ;
	printk("0x409C: NUNMBER OF TYPE3                                              :0x%.8x\n", IO_GREG(G_PLOu_PRMBL_TYPE3)) ;
	printk("0x40A0: NUMBER OF DELIMETER BITS                                      :0x%.8x\n", IO_GREG(G_PLOu_DELM_BIT)) ;
	printk("0x40A4: PRE-ASSIGNED DELAY                                            :0x%.8x\n", IO_GREG(G_PRE_ASSIGNED_DLY)) ;
	printk("0x40A8: MAIN PATH EQUALIZATION DELAY                                  :0x%.8x\n", IO_GREG(G_EQD)) ;
	printk("0x40AC: ONU ACTIVATION PARAMETER                                      :0x%.8x\n", IO_GREG(G_RSP_TIME)) ;
	printk("0x40B0: Vendor ID                                                     :0x%.8x\n", IO_GREG(G_VENDOR_ID)) ;
	printk("0x40B4: Vendor-specific serial number                                 :0x%.8x\n", IO_GREG(G_VS_SN)) ;
	printk("0x40B8: SN_ONU MESSAGE INFORMATION                                    :0x%.8x\n", IO_GREG(G_SN_MSG_CFG)) ;
	printk("0x40BC: ONU STATE IN ACTIVATION                                       :0x%.8x\n", IO_GREG(G_ACTIVATION_ST)) ;

	printk("0x40D0: SUPER FRAME COUNTER FOR TIME OF DAY CONFIGURATION             :0x%.8x\n", IO_GREG(G_TOD_CFG)) ;
	printk("0x40D4: NEW ToD SECOND LOW 32 BITS                                    :0x%.8x\n", IO_GREG(G_NEW_TOD_SEC_L32)) ;
	printk("0x40D8: NEW ToD NANO SECOND                                           :0x%.8x\n", IO_GREG(G_NEW_TOD_NANO_SEC)) ;
	printk("0x40DC: CURRENT ToD SECOND LOW 32 BITS                                :0x%.8x\n", IO_GREG(G_CUR_TOD_SEC_L32)) ;
	printk("0x40E0: CURRENT ToD NANO SECOND                                       :0x%.8x\n", IO_GREG(G_CUR_TOD_NANO_SEC)) ;
	printk("0x40E4: TOD CLOCK PERIOD                                              :0x%.8x\n", IO_GREG(G_TOD_CLK_PERIOD)) ;

	printk("0x4100: TX FCS TABLE INITIALIZATION CONTROL                           :0x%.8x\n", IO_GREG(G_TX_FCS_TBL_INIT)) ;

	printk("0x4120: MIB CONTROL AND STATUS                                        :0x%.8x\n", IO_GREG(G_MIB_CTRL_STS)) ;
	printk("0x4124: LOW 32 BITS of MIB READ DATA                                  :0x%.8x\n", IO_GREG(G_MIB_RDATA_L32)) ;
	printk("0x4128: HIGH 32 BITS of MIB READ DATA                                 :0x%.8x\n", IO_GREG(G_MIB_RDATA_H32)) ;
	printk("0x412C: LOW 32 BITS of MIB WRITE DATA                                 :0x%.8x\n", IO_GREG(G_MIB_WDATA_L32)) ;
	printk("0x4130: HIGH 32 BITS of MIB WRITE DATA                                :0x%.8x\n", IO_GREG(G_MIB_WDATA_H32)) ;
	printk("0x4134: MIB TABLE INITIALIZATION CONTROL                              :0x%.8x\n", IO_GREG(G_MIB_TBL_INIT)) ;

	printk("0x4140: GEM PORT INDEX TABLE ACCESS CONTROL FOR MIB                   :0x%.8x\n", IO_GREG(G_GPIDX_TBL_CTRL)) ;
	printk("0x4144: GEM PORT INDEX TABLE ACCESS STATUS                            :0x%.8x\n", IO_GREG(G_GPIDX_TBL_STS)) ;
	printk("0x4148: GEM PORT INDEX TABLE INITIALIZATION CONTROL                   :0x%.8x\n", IO_GREG(G_GPIDX_TBL_INIT)) ;

	printk("0x4160: MAC AND BM INTERFACE STOP                                     :0x%.8x\n", IO_GREG(G_MBI_STOP)) ;

	//printk("0x4180: THE CFG OF Alloc-ID of T-CONT16 to T-CONT31                   :0x%.8x\n", IO_GREG(G_TCONT_ID_16_31_CFG)) ;
	//printk("0x4184: THE STS OF Alloc-ID of T-CONT16 to T-CONT31                   :0x%.8x\n", IO_GREG(G_TCONT_ID_16_31_STS)) ;

	printk("0x4200: FLEXIBLE ONU CAPABILITY SETTING                               :0x%.8x\n", IO_GREG(DBG_CAP_SETTING)) ;
	printk("0x4204: UPSTREAM PREFETCH DATA OFFSET                                 :0x%.8x\n", IO_GREG(DBG_US_PREF_OFS)) ;
	printk("0x4208: INTERNAL DELAY                                                :0x%.8x\n", IO_GREG(DBG_DLY)) ;
	printk("0x420C: THRESHOLD of INSERT IDLE GEM                                  :0x%.8x\n", IO_GREG(DBG_IDLE_GEM_THLD)) ;
	printk("0x4210: UPSTREAM NO MESSAGE 0                                         :0x%.8x\n", IO_GREG(DBG_US_NO_MSG0)) ;
	printk("0x4214: UPSTREAM NO MESSAGE 1                                         :0x%.8x\n", IO_GREG(DBG_US_NO_MSG1)) ;
	printk("0x4218: UPSTREAM NO MESSAGE 2                                         :0x%.8x\n", IO_GREG(DBG_US_NO_MSG2)) ;
	printk("0x421C: UPSTREAM DYING GASP CONTROL                                   :0x%.8x\n", IO_GREG(DBG_US_DYING_GASP_CTRL)) ;
	printk("0x4220: BW MAP FITLER CONTROL                                         :0x%.8x\n", IO_GREG(DBG_BWM_FILTER_CTRL)) ;
#ifdef TCSUPPORT_CPU_EN7521
	printk("0x4224: BW MAP SFIFO STATUS                                           :0x%.8x\n", IO_GREG(DBG_BWM_SFIFO_STS)) ;
#else
	printk("0x4224: BW MAP SFIFO STATUS                                           :0x%.8x\n", IO_GREG(DBG_BWM_FIFO_STS)) ;
#endif
	printk("0x4228: DEBUG SIGNAL GROUP 0                                          :0x%.8x\n", IO_GREG(DBG_GRP_0)) ;
	printk("0x422C: DEBUG SIGNAL GROUP 1                                          :0x%.8x\n", IO_GREG(DBG_GRP_1)) ;
	printk("0x4230: THE MAXIMUM USED ENTRIES OF TX FIFO                           :0x%.8x\n", IO_GREG(DBG_TXFIFO_MAX_USED)) ;
	printk("0x4234: THE RECEIVED BWM GRANT STATUS                                 :0x%.8x\n", IO_GREG(DBG_BWM_GNT_STS)) ;

	//printk("0x4240: PROBE PIN CONTROL REGISTER                                    :0x%.8x\n", IO_GREG(DBG_PROBE_CTRL)) ;
	//printk("0x4244: PROBE PIN HIGH 32 BITS                                        :0x%.8x\n", IO_GREG(DBG_PROBE_HIGH32)) ;
	//printk("0x4248: PROBE PIN LOW 32 BITS                                         :0x%.8x\n", IO_GREG(DBG_PROBE_LOW32)) ;

#ifdef TCSUPPORT_CPU_EN7521
	printk("0x4250: BW MAP BFIFO STATUS                                           :0x%.8x\n", IO_GREG(DBG_BWM_BFIFO_STS)) ;
#endif

	printk("0x4260: ERROR CONTROL                                                 :0x%.8x\n", IO_GREG(DBG_ERR_CTRL)) ;
	printk("0x4264: SOFTWARE RDI CONTROL                                          :0x%.8x\n", IO_GREG(DBG_SW_RDI_CTRL)) ;

	printk("0x4300: RX GEM COUNTER                                                :0x%.8x\n", IO_GREG(DBG_RX_GEM_CNT)) ;
	printk("0x4304: RX ETHERNET FRAME WITH ERROR CRC COUNTER                      :0x%.8x\n", IO_GREG(DBG_RX_CRC_ERR_CNT)) ;
	printk("0x4308: RX GTC COUNTER                                                :0x%.8x\n", IO_GREG(DBG_RX_GTC_CNT)) ;
	printk("0x430C: TX GEM COUNTER                                                :0x%.8x\n", IO_GREG(DBG_TX_GEM_CNT)) ;
	printk("0x4310: TX BST COUNTER                                                :0x%.8x\n", IO_GREG(DBG_TX_BST_CNT)) ;

	printk("0x4330: RX GEM HEADER ONE BIT ERROR COUNTER                           :0x%.8x\n", IO_GREG(DBG_GEM_HEC_ONE_ERR_CNT)) ;
	printk("0x4334: RX GEM HEADER TWO BIT ERROR COUNTER                           :0x%.8x\n", IO_GREG(DBG_GEM_HEC_TWO_ERR_CNT)) ;
	printk("0x4338: RX GEM HEADER UNCORRECTABLE ERROR COUNTER                     :0x%.8x\n", IO_GREG(DBG_GEM_HEC_UC_ERR_CNT)) ;

#ifdef TCSUPPORT_CPU_EN7521
	printk("0x4340: DBA_BACK_DOOR_FOR_SW                                          :0x%.8x\n", IO_GREG(DBG_DBA_BACK_DOOR)) ;
	printk("0x4344: DBA_TOTAL_BUF_SIZE                                            :0x%.8x\n", IO_GREG(DBG_DBA_TOTAL_BUF_SIZE)) ;
	printk("0x4348: DBA_GREEN_SIZE                                                :0x%.8x\n", IO_GREG(DBG_DBA_GREEN_SIZE)) ;
	printk("0x434C: DBA_YELLOW_SIZE                                               :0x%.8x\n", IO_GREG(DBG_DBA_YELLOW_SIZE)) ;
	printk("0x4350: DBG_SLIGHT_MODIFY_SIZE_1                                      :0x%.8x\n", IO_GREG(DBG_SLIGHT_MODIFY_SIZE_1)) ;
	printk("0x4354: DBG_SLIGHT_MODIFY_SIZE_2                                      :0x%.8x\n", IO_GREG(DBG_SLIGHT_MODIFY_SIZE_2)) ;
 	printk("0x4358: DS_SPF_CNT                                                    :0x%.8x\n", IO_GREG(DBG_DS_SPF_CNT)) ;
 	printk("0x435C: TX_SYNC_OFFSET                                                :0x%.8x\n", IO_GREG(DBG_TX_SYNC_OFFSET)) ;
	printk("0x4360: PLOAMD_FILTER_IN_O5                                           :0x%.8x\n", IO_GREG(DBG_PLOAMD_FILTER_IN_O5)) ;
	printk("0x4364: SW_RESYNC                                                     :0x%.8x\n", IO_GREG(DBG_SW_RESYNC)) ;
	printk("0x4368: GTC_ETH_EXTR                                                  :0x%.8x\n", IO_GREG(DBG_GTC_ETH_EXTR)) ;
	printk("0x436C: DS_GTC_EXTR_ETH_HDR                                           :0x%.8x\n", IO_GREG(DBG_DS_GTC_EXTR_ETH_HDR)) ;
	printk("0x4370: US_GTC_EXTR_ETH_HDR                                           :0x%.8x\n", IO_GREG(DBG_US_GTC_EXTR_ETH_HDR)) ;
	printk("0x4374: DS_GTC_EXTR_ETH_CNT                                           :0x%.8x\n", IO_GREG(DBG_DS_GTC_EXTR_ETH_CNT)) ;
	printk("0x4378: US_GTC_EXTR_ETH_CNT                                           :0x%.8x\n", IO_GREG(DBG_US_GTC_EXTR_ETH_CNT)) ;
	printk("0x437C: SNIFF_GTC_INVLD_GEM_BYTE                                      :0x%.8x\n", IO_GREG(SNIFF_GTC_GTC_INVLD_GEM_BYTE)) ;
	printk("0x4380: SNIFF_DBG_REG                                                 :0x%.8x\n", IO_GREG(DBG_SNIFF_DBG_REG)) ;
	printk("0x4384: SNIFF_US_INFO_FIFO                                            :0x%.8x\n", IO_GREG(DBG_SNIFF_US_INFO_FIFO)) ;
	printk("0x4388: Sniff mode Up stream header transmit time window of stop time :0x%.8x\n", IO_GREG(SNIFF_US_HDR_WIDO)) ;
	printk("0x438C: Sniff TX DA SA adderess                                       :0x%.8x\n", IO_GREG(SNIFF_TX_DA_SA)) ;
	printk("0x4390: Sniff RX DA SA adderess                                       :0x%.8x\n", IO_GREG(SNIFF_RX_DA_SA)) ;
	printk("0x4394: Sniff TX RX PID                                               :0x%.8x\n", IO_GREG(SNIFF_TX_RX_PID)) ;
	printk("0x4398: Sniff mode GEM port counter enable                            :0x%.8x\n", IO_GREG(SNIFF_CNT_EN)) ;
	printk("0x439C: Sniff RX TX TPID                                              :0x%.8x\n", IO_GREG(SNIFF_RX_TX_TPID)) ;
	printk("0x43A0: GPON_MAC_SET                                                  :0x%.8x\n", IO_GREG(DBG_GPON_MAC_SET)) ;
	printk("0x43A4: power management sleep mode configuration                     :0x%.8x\n", IO_GREG(SLEEP_GLB_CFG)) ;
	printk("0x43A8: Power management sleep counter                                :0x%.8x\n", IO_GREG(SLEEP_CNT)) ;
	
	printk("0x43C0: Tx sync option                                                :0x%.8x\n", IO_GREG(TX_SYNC_OPT)) ;
	printk("0x43C4: O3 and O4 US PLOAM CONTROL                                    :0x%.8x\n", IO_GREG(O3_O4_PLOAMU_CTRL)) ;
#endif

#ifdef TCSUPPORT_CPU_EN7521
	if (isEN7526c || isEN751627) 
		printk("0x43C8: TOD 1PPS PLUSE WIDTH CONTROL								  :0x%.8x\n", IO_GREG(TOD_1PPS_WD_CTRL)) ;
#endif

	return 0;
}

/******************************************************************************
******************************************************************************/
int gponDevDumpTcontInfo(void)
{
	int i;
	GPON_TCONT_t valid;
	ushort id;
	GPON_TCONT_CMD_RESULT_t result;

	for(i = 0 ; i < CONFIG_GPON_MAX_TCONT ; i++) {
		/* read tcont status */
		result = gponDevGetTCont(&valid, i, &id);
		if(result == GPON_TCONT_CMD_SUCCESS) {
			printk("TCONT%d ID:%d, Valid:%s\n", i, id, 
						(valid == GPON_TCONT_VALID) ? "Valid" : "Invalid");
		} else {
			return -1;
		}
	}

	return 0 ;
}

/*****************************************************************************
******************************************************************************/

#ifndef TCSUPPORT_CPU_EN7521
extern void gpon_reset_dbg_delay(void);
#endif

#ifndef TCSUPPORT_CPU_EN7521
void setBwmFilterCtrl(void)
{
	uint write_data = 0 ;
	uint read_data = 0 ;
	/* set DBG_BWM_FILTER_CTRL bit 17 */
	read_data = IO_GREG(DBG_BWM_FILTER_CTRL) ;
	write_data = read_data & (~(1 << 17));
	IO_SREG(DBG_BWM_FILTER_CTRL, write_data) ;		
}
#endif

void gpon_dev_init_reset(void)
{
	gponDevMbiStop(XPON_ENABLE) ;

    if(XPON_ROUGE_STATE_TRUE == gpPonSysData->rogue_state)
    {   /* only reset mac */
        IO_SBITS(SCU_RESET_REG, SCU_GPON_MAC_RESET) ; 
        mdelay(1) ;
        IO_CBITS(SCU_RESET_REG, SCU_GPON_MAC_RESET) ;
    } 
    else 
    {   /* Reset GPON MAC & PHY*/
        IO_SBITS(SCU_RESET_REG, SCU_GPON_MAC_RESET) ; 
    #ifdef CONFIG_USE_MT7520_ASIC       
        IO_CBITS(TOP_TEST_MISC0_CTRL, SCU_GPON_PHY_RESET) ;
    #endif
        mdelay(1) ;
        IO_CBITS(SCU_RESET_REG, SCU_GPON_MAC_RESET) ;
    #ifdef CONFIG_USE_MT7520_ASIC
        IO_SBITS(TOP_TEST_MISC0_CTRL, SCU_GPON_PHY_RESET) ; 
    #endif      
        /*Init PHY */
        if(XPON_PHY_SET(PON_SET_PHY_DEV_INIT) != 0){
            dump_stack();
            panic("%s:phy device initialization failed.\n", __FUNCTION__);
        }

        XPON_PHY_SET(PON_SET_PHY_TRANS_MODEL_SETTING);
    }
    
#ifndef TCSUPPORT_CPU_EN7521
    if (gpGponPriv->gponCfg.rstDbgDly){
        gpon_reset_dbg_delay();
    }

	/* OSBNB00047232 */
    setBwmFilterCtrl();  /* OSBNB00047232 */
#endif    
    gponDevMbiStop(XPON_DISABLE) ;   /* START GPON/PSE MBI Interface */
}

void gpon_dev_reset_GPON_MAC(void)
{
	unsigned long flags;
	spin_lock_irqsave(&gpPonSysData->event_lock, flags) ;
	/* Reset GPON MAC */
	IO_SBITS(SCU_RESET_REG, SCU_GPON_MAC_RESET) ; 
	udelay(1) ;
	/* Release GPON MAC */
	IO_CBITS(SCU_RESET_REG, SCU_GPON_MAC_RESET) ;
	spin_unlock_irqrestore(&gpPonSysData->event_lock, flags) ;

    /* disable MBI tx & rx, GDM2 tx & rx, CDM2 rx*/
    gponDevMbiStop(XPON_ENABLE) ;
}

/*****************************************************************************
******************************************************************************/
int gponDevDeactiveOnu(void)
{
	REG_G_ONU_ID gponOnuId ;
	REG_G_GBL_CFG gponGlbCfg ;
	
	/* Setting the ONU ID to MAC register */
	gponOnuId.Raw = IO_GREG(G_ONU_ID) ;
	gponOnuId.Bits.onu_id_vld = 0 ;
	gponOnuId.Bits.onu_id = GPON_UNASSIGN_ONU_ID ;
	IO_SREG(G_ONU_ID, gponOnuId.Raw) ;

	/* Disable the MAC tx FEC */
	gponGlbCfg.Raw = IO_GREG(G_GBL_CFG) ;
	gponGlbCfg.Bits.us_fec_en = 0 ;
	IO_SREG(G_GBL_CFG, gponGlbCfg.Raw) ;
	
	/* Reset the GEM Port */
	gponDevResetGemInfo() ;
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevSetEncryptKey(unchar *aesKey)
{
	REG_G_AES_SHADOW_KEY0 gponAesKey0 ;
	REG_G_AES_SHADOW_KEY1 gponAesKey1 ;
	REG_G_AES_SHADOW_KEY2 gponAesKey2 ;
	REG_G_AES_SHADOW_KEY3 gponAesKey3 ;
	
	gponAesKey3.Bits.aes_shadow_key3 = (aesKey[0]<<24)|(aesKey[1]<<16)|(aesKey[2]<<8)|(aesKey[3]) ;
	gponAesKey2.Bits.aes_shadow_key2 = (aesKey[4]<<24)|(aesKey[5]<<16)|(aesKey[6]<<8)|(aesKey[7]) ;
	gponAesKey1.Bits.aes_shadow_key1 = (aesKey[8]<<24)|(aesKey[9]<<16)|(aesKey[10]<<8)|(aesKey[11]) ;
	gponAesKey0.Bits.aes_shadow_key0 = (aesKey[12]<<24)|(aesKey[13]<<16)|(aesKey[14]<<8)|(aesKey[15]) ;
	
	IO_SREG(G_AES_SHADOW_KEY0, gponAesKey0.Raw) ;
	IO_SREG(G_AES_SHADOW_KEY1, gponAesKey1.Raw) ;
	IO_SREG(G_AES_SHADOW_KEY2, gponAesKey2.Raw) ;
	IO_SREG(G_AES_SHADOW_KEY3, gponAesKey3.Raw) ;

	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetEncryptKey(GPON_DEV_ENCRYPT_KEY_INFO_T *aesKey)
{
    REG_G_AES_CFG aesCfgReg;
    aesCfgReg.Raw = IO_GREG(G_AES_CFG);
    aesKey->aesSpf = aesCfgReg.Bits.aes_spf_cnt;

	aesKey->shadowKey[0] = IO_GREG(G_AES_SHADOW_KEY0) ;
	aesKey->shadowKey[1] = IO_GREG(G_AES_SHADOW_KEY1) ;
	aesKey->shadowKey[2] = IO_GREG(G_AES_SHADOW_KEY2) ;
	aesKey->shadowKey[3] = IO_GREG(G_AES_SHADOW_KEY3) ;

	aesKey->activeKey[0] = IO_GREG(G_AES_ACTIVE_KEY0) ;
	aesKey->activeKey[1] = IO_GREG(G_AES_ACTIVE_KEY1) ;
	aesKey->activeKey[2] = IO_GREG(G_AES_ACTIVE_KEY2) ;
	aesKey->activeKey[3] = IO_GREG(G_AES_ACTIVE_KEY3) ;

	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevSetKeySwithTime(uint counter)
{
#ifdef TCSUPPORT_CPU_EN7521	
	GPON_AES_KEY_SWITCH_BY_SW_DONE_t aesSwitchKeyResult;
#endif
	REG_G_AES_CFG gponAesCfg ;
	
	if(!counter) {
		/* Set the debug probe to get current superframe counter */
#ifdef TCSUPPORT_CPU_EN7521
		/* switch key by SW */
		aesSwitchKeyResult = gponDevSetAesKeySwitchBySw();

		if(aesSwitchKeyResult == GPON_AES_KEY_SWITCH_BY_SW_DONE_PROCESSING) {
			/* There is something wrong with set AES key switch by SW */
			PON_MSG(MSG_ERR, "There is something wrong with set AES key switch by SW\n");
		}

		return 0;
#else
		/* Set the debug probe to get current superframe counter */
		gponDevGetSuperframe(&counter);
		/* switch key after 0.5 second */
		counter += 4000 ;
#endif		
	}

	gponAesCfg.Raw = IO_GREG(G_AES_CFG) ;
	gponAesCfg.Bits.aes_spf_cnt = counter ;
	IO_SREG(G_AES_CFG, gponAesCfg.Raw) ;
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevSetDBABlockSize(ushort blockSize)
{
	unchar setValue = 0, seed, i ;
	REG_G_GBL_CFG gponGlbCfg ;
	
	if(blockSize >= 2048) {
		setValue = 0x80 ;
	} else {
		seed = (2048/blockSize) + (((2048%blockSize)>=(blockSize>>1)) ? 1 : 0) ;
		for(i=0 ; i<8 ; i++) {
			setValue += (!((~seed)&(1<<i))) ? (1<<(7-i)) : 0 ;
		}
	}
	
	/* Set GPON BlockSize */
    gpGponPriv->gponCfg.sr_blk_size = setValue;
	gponGlbCfg.Raw = IO_GREG(G_GBL_CFG) ;
	gponGlbCfg.Bits.sr_blk_size = setValue ;
	IO_SREG(G_GBL_CFG, gponGlbCfg.Raw) ;

	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetDBABlockSize(ushort *blockSize)
{
	REG_G_GBL_CFG gponGlbCfg ;
	
	gponGlbCfg.Raw = IO_GREG(G_GBL_CFG) ;
	*blockSize = gponGlbCfg.Bits.sr_blk_size ;

	return 0 ;
}


/*****************************************************************************
******************************************************************************/
int gponDevHardwareDyingGasp(GPON_SW_HW_SELECT_T mode)
{
	REG_DBG_US_DYING_GASP_CTRL gponDGCtrl ;
	
	gponDGCtrl.Raw = IO_GREG(DBG_US_DYING_GASP_CTRL) ;
	gponDGCtrl.Bits.hw_dying_gasp_en = mode ;	
	IO_SREG(DBG_US_DYING_GASP_CTRL, gponDGCtrl.Raw) ;
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetDyingGaspMode(GPON_SW_HW_SELECT_T *mode)
{
	REG_DBG_US_DYING_GASP_CTRL gponDGCtrl ;
	
	gponDGCtrl.Raw = IO_GREG(DBG_US_DYING_GASP_CTRL) ;
	*mode = gponDGCtrl.Bits.hw_dying_gasp_en ;
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevSetDyingGaspNum(uint num)
{
	REG_DBG_US_DYING_GASP_CTRL gponDGCtrl ;
	
	gponDGCtrl.Raw = IO_GREG(DBG_US_DYING_GASP_CTRL) ;
	gponDGCtrl.Bits.dying_gasp_num = num ;
	IO_SREG(DBG_US_DYING_GASP_CTRL, gponDGCtrl.Raw) ;
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetDyingGaspNum(uint *num)
{
	REG_DBG_US_DYING_GASP_CTRL gponDGCtrl ;
	
	gponDGCtrl.Raw = IO_GREG(DBG_US_DYING_GASP_CTRL) ;
	*num = gponDGCtrl.Bits.dying_gasp_num ;
	
	return 0 ;
}


/*****************************************************************************
******************************************************************************/
int gponDevSetIdleGemThreshold(ushort idle_gem_thld)
{
	REG_DBG_IDLE_GEM_THLD	dbg_idle_gem_thld;

	dbg_idle_gem_thld.Raw = IO_GREG(DBG_IDLE_GEM_THLD) ;
	dbg_idle_gem_thld.Bits.idle_gem_thld = idle_gem_thld;
	IO_SREG(DBG_IDLE_GEM_THLD, dbg_idle_gem_thld.Raw) ;
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetIdleGemThreshold(ushort *idle_gem_thld)
{
	REG_DBG_IDLE_GEM_THLD	dbg_idle_gem_thld;
	
	dbg_idle_gem_thld.Raw = IO_GREG(DBG_IDLE_GEM_THLD) ;
	*idle_gem_thld = dbg_idle_gem_thld.Bits.idle_gem_thld;
	
	return 0 ;
}


/*****************************************************************************
******************************************************************************/
int gponDevSetCounterType(GPON_COUNTER_TYPE_t type)
{
	REG_DBG_CAP_SETTING gponCapSet ;
	
	gponCapSet.Raw = IO_GREG(DBG_CAP_SETTING) ;
	gponCapSet.Bits.mib_frame_type = type ;	
	IO_SREG(DBG_CAP_SETTING, gponCapSet.Raw) ;
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetCounterType(GPON_COUNTER_TYPE_t *type)
{
	REG_DBG_CAP_SETTING gponCapSet ;
	
	gponCapSet.Raw = IO_GREG(DBG_CAP_SETTING) ;
	*type = gponCapSet.Bits.mib_frame_type ;	
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevSetResponseTime(ushort time)
{
	REG_G_RSP_TIME gponRspTime ;
	
	gponRspTime.Raw = IO_GREG(G_RSP_TIME) ;
	gponRspTime.Bits.tresp = time ;	
	IO_SREG(G_RSP_TIME, gponRspTime.Raw) ;
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetResponseTime(ushort *time)
{
	REG_G_RSP_TIME gponRspTime ;
	
	gponRspTime.Raw = IO_GREG(G_RSP_TIME) ;
	*time = gponRspTime.Bits.tresp ;
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevSetInternalDelayFineTune(unchar delay)
{
	REG_DBG_DLY dbgDly;

	dbgDly.Raw = IO_GREG(DBG_DLY);
	dbgDly.Bits.fine_int_dly = delay;
	IO_SREG(DBG_DLY, dbgDly.Raw);
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetInternalDelayFineTune(unchar *delay)
{
	REG_DBG_DLY dbgDly;

	dbgDly.Raw = IO_GREG(DBG_DLY);
	*delay = dbgDly.Bits.fine_int_dly;
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevClearSwCounter(void)
{
	int i;
	
	for(i = 0 ; i < CONFIG_GPON_MAX_GEMPORT ; i++) {
		if(gpWanPriv->gpon.gemPort[i].info.valid) {
			memset(&gpWanPriv->gpon.gemPort[i].stats, 0, sizeof(struct net_device_stats)) ;
		}
	}
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevClearHwCounter(void)
{
	int idx = 0 ;
	int i, j;
	int gemIdx ;
	
	gponDevGemMibTablesInit() ;
	
	gponDevUpdateGemMibIdxTable(0, 0) ;
	for(j = 1, i = 0 ; i < GPON_MAX_GEM_ID ; i++) {
		gemIdx = gpWanPriv->gpon.gemIdToIndex[i] ;
		if(gpWanPriv->gpon.gemPort[gemIdx].info.valid) {
			idx++ ;
		}
		if((i&0xF)==0xF && j!=256) {
			gponDevUpdateGemMibIdxTable(j, idx) ;
			j++ ;
		}
	}
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int feDevGdm2Cdm2Stop(XPON_Mode_t mode)
{	
#ifdef TCSUPPORT_CPU_EN7521
	int i;
	GPON_TCONT_t valid;
	ushort id;
	GPON_TCONT_CMD_RESULT_t result;
#else
	uint regAddr = 0xBFB5152C ;
	uint pseChannelMode ;
#endif

	if((mode != XPON_DISABLE) && (mode != XPON_ENABLE)) {
		return -1;
	}

#ifdef TCSUPPORT_CPU_EN7521
	if(mode == XPON_ENABLE) {
		/* tx: disable all GDM2 tx channel */
		for(i = 0; i < CONFIG_GPON_MAX_TCONT; i++) {
			FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_DISABLE);
		}
		/* rx: disable all GDM2 rx channel */
		for(i = 0; i < 16; i++) {
			FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX, i, FE_DISABLE);
		}
		/* disable CDM2 rx DMA */
		for(i = 0 ; i < CONFIG_GPON_MAX_TCONT ; i++) {
			FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, i, FE_DISABLE);
		}
	} else {
		/* rx: enable 2 rx channel */
		for(i = 0; i < 2; i++) {
			FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX, i, FE_ENABLE);
		}
		/* tx: tx channel is enabled by OMCI */
	}
#else
	if(mode == XPON_ENABLE) {
		/* disable all GDM2 tx channel */
		pseChannelMode = 0 ;
		/* MT7520 has no CDM2 rx DMA register */
	} else {
		/* rx: enable 2 rx channel */
		pseChannelMode = 0x00030000 ;
		/* tx: tx channel is enabled by OMCI */
	}
	IO_SREG(regAddr, pseChannelMode) ;
#endif

	return 0;
}

/*****************************************************************************
******************************************************************************/
int gponDevMbiStop(XPON_Mode_t mode)
{
	REG_G_MBI_STOP gponMbiStop ;

	if((mode != XPON_DISABLE) && (mode != XPON_ENABLE)) {
		return -1;
	}
		
	/* STOP/START MBI Interface */
	gponMbiStop.Raw = IO_GREG(G_MBI_STOP) ;
	/* 0: not stop 
	 *  1: stop
	 */
	gponMbiStop.Bits.mbi_rx_stop = (mode == XPON_DISABLE) ? 0 : 1 ;
	gponMbiStop.Bits.mbi_tx_stop = (mode == XPON_DISABLE) ? 0 : 1 ;
	IO_SREG(G_MBI_STOP, gponMbiStop.Raw) ;

	/* wait for MBI interface finish */
	mdelay(1);

	feDevGdm2Cdm2Stop(mode);

	/* wait for GDM2 finish */
	mdelay(1);
	
	return 0 ;
}

static void gpon_INT_init(void)
{
	REG_G_INT_ENABLE gponIntEnable ;
	
	/* Set the INT mask */
	gponIntEnable.Raw = 0 ;
#ifdef TCSUPPORT_CPU_EN7521	
	gponIntEnable.Bits.bwm_stop_time_err_int_en  = 1;
	gponIntEnable.Bits.lwi_int_en				 = 1;
	gponIntEnable.Bits.fwi_int_en                = 1;
    gponIntEnable.Bits.onu_us_fec_chg_int_en     = 1;
    gponIntEnable.Bits.olt_ds_fec_chg_int_en     = 1;
    gponIntEnable.Bits.o5_eqd_adj_done_int_en    = 1;
    gponIntEnable.Bits.sfifo_full_int_en         = 1;
    gponIntEnable.Bits.bfifo_full_int_en         = 1;
    gponIntEnable.Bits.rx_gem_intlv_err_int_en   = 1;
#endif
	gponIntEnable.Bits.rx_eof_err_int_en =			1 ;
	gponIntEnable.Bits.tx_late_start_int_en =		1 ;
	gponIntEnable.Bits.bst_sgl_diff_int_en =		1 ;
	gponIntEnable.Bits.fifo_err_int_en =			1 ;
	gponIntEnable.Bits.rx_err_int_en =				1 ;
	gponIntEnable.Bits.dying_gasp_send_int_en =		1 ;
	gponIntEnable.Bits.tod_1pps_int_en =			0 ;
	gponIntEnable.Bits.tod_update_done_int_en =		1 ;
	gponIntEnable.Bits.aes_key_switch_done_int_en =	1 ;
	gponIntEnable.Bits.los_gem_del_int_en =			1 ;
	gponIntEnable.Bits.sn_req_crs_int_en =			1 ;
	gponIntEnable.Bits.sn_onu_send_o4_int_en =		1 ;
	gponIntEnable.Bits.ranging_req_recv_int_en =	1 ;
	gponIntEnable.Bits.sn_onu_send_o3_int_en =		1 ;
	gponIntEnable.Bits.sn_req_recv_int_en =			1 ;
	gponIntEnable.Bits.ploamu_send_int_en =			0 ;
	gponIntEnable.Bits.ploamd_recv_int_en =			1 ;

	/* Clear the INT status and enable the INT */
	IO_SREG(G_INT_STATUS, 0xFFFFFFFF) ;
	IO_SREG(G_INT_ENABLE, gponIntEnable.Raw) ;

}

/*****************************************************************************
******************************************************************************/

int gpon_dev_init(void)
{
    int  ret = 0;

	select_xpon_wan(PHY_GPON_CONFIG);
	
	//Set the MAC register
    gpon_INT_init();

	if (gpPonSysData->dyingGaspData.isGponHwFlag) {
		gponDevHardwareDyingGasp(GPON_HW) ;
	} else {
		gponDevHardwareDyingGasp(GPON_SW) ;
	}

	/* Set idle gem threshold */
	gponDevSetIdleGemThreshold(gpGponPriv->gponCfg.idle_gem_thld);

	/* Set counter type */
	gponDevSetCounterType(gpGponPriv->gponCfg.counter_type);
	
#ifdef TCSUPPORT_CPU_EN7521
	/* Set DBA block size */
	if (isEN7526c || isEN751627) 
		gponDevSetDBABlockSize(GPON_DEFAULT_DBA_BLOCK_SIZE);

	/* set default sleep count */
	modify_sleep_count(gpGponPriv->gponCfg.sleep_count);
	/* set ToD clock period */
	gponDevSetTodClkPeriod(0x0A);
#else /* TCSUPPORT_CPU_EN7521 */
	/* OSBNB00047232 */
	setBwmFilterCtrl();
#endif /* TCSUPPORT_CPU_EN7521 */

	return 0 ;
}

#ifndef TCSUPPORT_CPU_EN7521
static void save_tconts(void)
{
    GPON_TCONT_T * pTconts = & gpGponPriv->gponCfg.tconts;
    pTconts->gponTCont0.Raw = IO_GREG(G_TCONT_ID_0_1) ;
    pTconts->gponTCont2.Raw = IO_GREG(G_TCONT_ID_2_3) ;
    pTconts->gponTCont4.Raw = IO_GREG(G_TCONT_ID_4_5) ;
    pTconts->gponTCont6.Raw = IO_GREG(G_TCONT_ID_6_7) ;
    pTconts->gponTCont8.Raw = IO_GREG(G_TCONT_ID_8_9) ;
    pTconts->gponTCont10.Raw = IO_GREG(G_TCONT_ID_10_11) ;
    pTconts->gponTCont12.Raw = IO_GREG(G_TCONT_ID_12_13) ;
    pTconts->gponTCont14.Raw = IO_GREG(G_TCONT_ID_14_15) ;
}

static inline void save_gpon_dev_cfg(void)
{
    save_tconts();
}

static void restore_tconts(void)
{
    PON_MSG(MSG_INT, "Enter \"restore_tconts\".\n");
    GPON_TCONT_T * pTconts = & gpGponPriv->gponCfg.tconts;
    IO_SREG(G_TCONT_ID_0_1, pTconts->gponTCont0.Raw);
    IO_SREG(G_TCONT_ID_2_3, pTconts->gponTCont2.Raw);
    IO_SREG(G_TCONT_ID_4_5, pTconts->gponTCont4.Raw);
    IO_SREG(G_TCONT_ID_6_7, pTconts->gponTCont6.Raw);
    IO_SREG(G_TCONT_ID_8_9, pTconts->gponTCont8.Raw);
    IO_SREG(G_TCONT_ID_10_11, pTconts->gponTCont10.Raw);
    IO_SREG(G_TCONT_ID_12_13, pTconts->gponTCont12.Raw);
    IO_SREG(G_TCONT_ID_14_15, pTconts->gponTCont14.Raw);
    PON_MSG(MSG_INT, "Exit \"restore_tconts\".\n");
}

static inline void restore_gemports(void)
{
    ushort i = 0;
    ushort portID = 0;
    unchar isEncrypted = 0;
    PON_MSG(MSG_INT, "Enter \"restore_gemports\".\n");
	for(i=0 ; i< CONFIG_GPON_MAX_GEMPORT ; i++)
    {
		if(!gpWanPriv->gpon.gemPort[i].info.valid){
            continue;
		}

		portID = gpWanPriv->gpon.gemPort[i].info.portId ;
        isEncrypted = (gpWanPriv->gpon.gemPort[i].info.rxEncrypt ? XPON_ENABLE : XPON_DISABLE);
		
		gponDevSetGemInfo(portID, XPON_ENABLE, isEncrypted) ;
	}
    mod_timer(&gpWanPriv->gpon.gemMibTimer, (jiffies + (2*HZ))) ;
    PON_MSG(MSG_INT, "Exit \"restore_gemports\".\n");
}

static uint gIrqMaskValue = 0;
static uint gSnMsgValue = 0;

static void __gpon_dev_typeB_reset(void)
{	
    uint write_data = 0 ;
	uint read_data = 0 ;

    PON_MSG(MSG_INT, "Enter into \"__gpon_dev_typeB_reset\".\n");

    XPON_PHY_TX_DISABLE();

    if (gpGponPriv->gponCfg.macPhyReset & GPON_MAC_RESET_BIT)
    {
        gIrqMaskValue = IO_GREG(G_INT_ENABLE) ;
        save_gpon_dev_cfg();
    	gponDevMbiStop(XPON_ENABLE) ; /* STOP GPON/PSE MBI Interface */
        udelay(1) ;
    	IO_SBITS(SCU_RESET_REG, SCU_GPON_MAC_RESET) ;
        udelay(1) ;
        IO_CBITS(SCU_RESET_REG, SCU_GPON_MAC_RESET) ;
        gponDevMbiStop(XPON_DISABLE) ;	/* START GPON/PSE MBI Interface */
    }
    
    if (gpGponPriv->gponCfg.macPhyReset &  PHY_ANALOG_RESET_BIT)
    {
        XPON_PHY_FW_READY_DISABLE();
#ifdef CONFIG_USE_MT7520_ASIC
    	IO_CBITS(TOP_TEST_MISC0_CTRL, SCU_GPON_PHY_RESET) ;
#endif
	    udelay(1) ;
#ifdef CONFIG_USE_MT7520_ASIC
	    IO_SBITS(TOP_TEST_MISC0_CTRL, SCU_GPON_PHY_RESET) ; 
#endif
    	if(XPON_PHY_SET(PON_SET_PHY_DEV_INIT) != 0) /*Init PHY */
        { 
    		printk("%s:phy device initialization failed.\n", __FUNCTION__);
    	}
        /* slect transceiver model */
    	XPON_PHY_SET(PON_SET_PHY_TRANS_MODEL_SETTING);
        XPON_PHY_FW_READY_ENABLE(); /*enable FW ready*/
    }

    if (gpGponPriv->gponCfg.macPhyReset & (PHY_DIGITAL_RESET_BIT | PHY_ANALOG_RESET_BIT))
    {
    	/* set to burst mode*/
    	read_data = IO_GPHYREG(PHY_CSR_PHYSET3) ;
    	write_data = read_data & PHY_TX_EPON_CONT_MODE;
    	IO_SPHYREG(PHY_CSR_PHYSET3,write_data);
        
    	/* change to GPON mode*/
    	read_data = IO_GPHYREG(PHY_CSR_PHYSET10) ;
#ifdef CONFIG_USE_MT7520_ASIC
    	write_data = (read_data | PHY_GPON_MODE);
#else
    	write_data = (read_data | PHY_GPON_MODE) | PHY_GPON_DEMASK;
#endif
        IO_SPHYREG(PHY_CSR_PHYSET10, write_data) ;

        /*reset phy*/
		read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
		write_data = read_data | (PHY_PLL_RST) |(PHY_COUNT_RST);
		IO_SPHYREG(PHY_CSR_PHYSET3, write_data) ;
		udelay(1);
		IO_SPHYREG(PHY_CSR_PHYSET3, read_data) ;
		
		#ifdef CONFIG_USE_A60901
		IO_SPHYREG(PHY_CSR_ANASET10, 0x211A0202) ;//cdr_bir
		#endif /* CONFIG_USE_A60901 */
    }
    
    XPON_PHY_TX_ENABLE();

    PON_MSG(MSG_INT, "Exit \"__gpon_dev_typeB_reset\".\n");
}

/* regs restore is done here */
static void gpon_dev_typeB_restore(void)
{
    REG_G_ONU_ID gponOnuId ;
    REG_G_GBL_CFG gponGlbCfg ;
    REG_G_OMCI_ID gponOmciId ;

    if (gpGponPriv->gponCfg.macPhyReset & (PHY_DIGITAL_RESET_BIT | PHY_ANALOG_RESET_BIT))
    {
        XPON_PHY_SET(PON_SET_PHY_RESTORE_PREAMBLE_AND_GUARD_BIT);
    }

    if (gpGponPriv->gponCfg.macPhyReset & GPON_MAC_RESET_BIT)
    {
    	/* Setting the ONU ID to MAC register */
    	gponOnuId.Raw = IO_GREG(G_ONU_ID) ;
    	gponOnuId.Bits.onu_id_vld = 1 ;
    	gponOnuId.Bits.onu_id = gpGponPriv->gponCfg.onu_id ;
    	IO_SREG(G_ONU_ID, gponOnuId.Raw) ;

        /* Set GPON BlockSize */
        gponGlbCfg.Raw = IO_GREG(G_GBL_CFG) ;
        gponGlbCfg.Bits.sr_blk_size = gpGponPriv->gponCfg.sr_blk_size ;
        IO_SREG(G_GBL_CFG, gponGlbCfg.Raw) ;
    	
    	/* activate the port id of OMCI channel */
    	gponOmciId.Raw = IO_GREG(G_OMCI_ID) ;
    	gponOmciId.Bits.omci_port_id_vld = 1 ;
    	gponOmciId.Bits.omci_gpid = gpGponPriv->gponCfg.omcc ;
    	IO_SREG(G_OMCI_ID, gponOmciId.Raw) ;

        restore_tconts();
        restore_gemports();

        IO_SREG(G_PLOu_GUARD_BIT, gpGponPriv->gponCfg.gponGuardBit.Raw) ;
        IO_SREG(G_PLOu_PRMBL_TYPE1_2, gpGponPriv->gponCfg.gponPrmblType.Raw) ;
        IO_SREG(G_PLOu_PRMBL_TYPE3, gpGponPriv->gponCfg.gponT3Prmbl.Raw) ;
        IO_SREG(G_PLOu_DELM_BIT, gpGponPriv->gponCfg.gponDelmBit.Raw) ;
        IO_SREG(G_PRE_ASSIGNED_DLY, gpGponPriv->gponCfg.gponPreAssignDly.Raw) ;
        IO_SREG(G_RSP_TIME, gpGponPriv->gponCfg.onuResponseTime) ;

        gponDevSetSerialNumber(gpGponPriv->gponCfg.sn) ;

        /* resume GPON MAC */
    	IO_SREG(G_INT_STATUS, 0xFFFFFFFF) ;
    	IO_SREG(G_INT_ENABLE, gIrqMaskValue) ;
        
    	gSnMsgValue = IO_GREG(G_SN_MSG_CFG);
    	gSnMsgValue |= (2<<16);
    	IO_SREG(G_SN_MSG_CFG, gSnMsgValue);
    }
}

void gpon_dev_typeB_reset(void)
{
    PON_MSG(MSG_INT, "Enter into \"gpon_dev_typeB_reset\".\n");
    __gpon_dev_typeB_reset();
    gpon_dev_typeB_restore();
    PON_MSG(MSG_INT, "Exit \"gpon_dev_typeB_reset\".\n");
}
#endif /* TCSUPPORT_CPU_EN7521 */

/*****************************************************************************
******************************************************************************/
int gponDevCheckTContReg(ushort channelId, ushort allocId)
{
	REG_G_TCONT_ID_0_1 gponTCont ;
	uint TcontReg = 0;

	if(channelId >= CONFIG_GPON_MAX_TCONT || allocId >= GPON_MAX_ALLOC_ID)
	{
		printk("ERROR: Input Para wrong, channelId: %d  allocId: %d\n",channelId,allocId);
		return FALSE;
	}
	
	/* get tcont register by channelID */
	TcontReg = G_TCONT_ID_0_1 + (channelId / 2);

	/* check allocID ang valid flag write */
	gponTCont.Raw = IO_GREG(TcontReg) ;
	if(0 == (channelId % 2))
	{
		if(gponTCont.Bits.t_cont0_vld == 1 && gponTCont.Bits.t_cont0_id == allocId)
		{
			//printk("AllocId %d matched in channel %d\n",allocId,channelId);
			return TRUE;
		}
		else
		{
			printk("ERROR: Tcont info in reg is wrong,channelId %d, valid %d, alloc %d\n",
				channelId,gponTCont.Bits.t_cont0_vld,gponTCont.Bits.t_cont0_id);
			return FALSE;
		}
	}
	else
	{
		if(gponTCont.Bits.t_cont1_vld == 1 && gponTCont.Bits.t_cont1_id == allocId)
		{
			//printk("AllocId %d matched in channel %d\n",allocId,channelId);
			return TRUE;
		}
		else
		{
			printk("ERROR: Tcont info in reg is wrong,channelId %d, valid %d, alloc %d\n",
				channelId,gponTCont.Bits.t_cont1_vld,gponTCont.Bits.t_cont1_id);
			return FALSE;
		}
	}

	return TRUE;
}



