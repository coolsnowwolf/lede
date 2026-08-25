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
#include <linux/kernel.h>
#include <linux/delay.h>
#include "common/xpon_global.h"
#include "common/phy_if_wrapper.h"
#include "gpon/gpon_security.h"

#ifdef TCSUPPORT_CPU_EN7581
#include <ecnt_hook/ecnt_hook_tod_gen.h>
#endif


/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                  M A C R O S
*************************************************************************
*/
#define CHECK_RETRY_TIMES 100

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

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/
extern uint gponTCLODSEvent;
extern uint gponTCRestoreLODSEvent;
extern uint gponTCReactivLODSEvent;
extern uint hw_gem_config;
extern int omciMicErrSwCnt;

#ifdef TCSUPPORT_CPU_ARMV8_64
extern struct device* get_xpon_dev(void);
#endif

#if defined(TCSUPPORT_CPU_EN7581)
extern int gpon_tod_adjust;
#endif


/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

/*****************************************************************************
******************************************************************************/
int gponDevGetPloamMsg(PLOAM_RAW_General_T *pPloamMsg)
{
	REG_PLOAMd_FIFO_STS gponPloamStatus;	
	int i = 0;
	
	gponPloamStatus.Raw = IO_GREG(PLOAMd_FIFO_STS);
	if(gponPloamStatus.Bits.ploamd_fifo_used == 0) {
		goto ret;
	} else if(gponPloamStatus.Bits.ploamd_fifo_used >= PLOAM_DOWN_MSG_LENGTH) {
		for(i=0; i<PLOAM_DOWN_MSG_LENGTH; i++) {
#ifdef __BIG_ENDIAN
			pPloamMsg->value[i] = IO_GREG(PLOAMd_RDATA) ;
#else
			pPloamMsg->value[i] = ntohl(IO_GREG(PLOAMd_RDATA)) ;
#endif
		}
	} else {
		return -EFAULT ;
	}
	
ret:
	return gponPloamStatus.Bits.ploamd_fifo_used ;
}

/*****************************************************************************
******************************************************************************/
void gponDevSendPloamMsg(PLOAM_RAW_General_T *pPloamMsg)
{
	int i=0;
	
	for(i=0 ; i< PLOAM_UP_MSG_LENGTH ; i++) {
#ifdef __BIG_ENDIAN	
		IO_SREG(PLOAMu_WDATA, pPloamMsg->value[i]) ;
#else
		IO_SREG(PLOAMu_WDATA, htonl(pPloamMsg->value[i])) ;
#endif
	}
	
	gpGponPriv->ploamMsgcounter.txPloamMsgCnt++;
}
/*****************************************************************************
******************************************************************************/
int feDevGdm2Cdm2Stop(XPON_RESET_MODE_t mode)
{	
	int i=0;
	if((mode != XPON_RESET_HOLD_ON) && (mode != XPON_RESET_RELEASE)) {
		return -1;
	}

	if(mode == XPON_RESET_HOLD_ON) {
		/* tx: disable all GDM2 tx channel */
		for(i = 0; i < CONFIG_GPON_10G_MAX_TCONT; i++) {
			FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_DISABLE);
		}
		/* rx: disable all GDM2 rx channel */
		for(i = 0; i < CONFIG_GPON_10G_MAX_TCONT; i++) {
			FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX, i, FE_DISABLE);
		}
		/* disable CDM2 rx DMA */
		for(i = 0 ; i < CONFIG_GPON_10G_MAX_TCONT ; i++) {
			FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, i, FE_DISABLE);
		}     
	} else {
		if(GPON_CURR_STATE == GPON_10G_STATE_O8){
			/* tx: enable all GDM2 tx channel */
			for(i = 0; i < CONFIG_GPON_10G_MAX_TCONT; i++) {
				FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_ENABLE);
			}
			/* rx: enable all GDM2 rx channel */
			for(i = 0; i < CONFIG_GPON_10G_MAX_TCONT; i++) {
				FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX, i, FE_ENABLE);
			}
			/* enable CDM2 rx DMA */
			for(i = 0 ; i < CONFIG_GPON_10G_MAX_TCONT ; i++) {
				FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, i, FE_ENABLE);
			}
		}
		else{
			/* rx: enable 2 rx channel */
			for(i = 0; i < 2; i++) {
				FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX, i, FE_ENABLE);
			}
	        FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, 0, FE_ENABLE);
	        /* Enable CDM2 Rx DMA */
	        FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, 0, FE_ENABLE);
			/* tx: tx channel is enabled by OMCI */
		}
	}

	return 0;
}

/*****************************************************************************
******************************************************************************/
int gponDevTxMbiStop(XPON_RESET_MODE_t mode)
{
	   REG_MBI_MPI_STOP gponMbiMpiStop ;
	   int RETRY = 3000 ;

	   if((mode != XPON_RESET_HOLD_ON) && (mode != XPON_RESET_RELEASE)) {
			   return -1;
	   }

	   /* STOP/START MBI Interface ; 1:stop; 0:start*/
	   gponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;
	   gponMbiMpiStop.Bits.mbi_tx_stop = (mode == XPON_RESET_HOLD_ON) ? 1 : 0;
	   IO_SREG(MBI_MPI_STOP, gponMbiMpiStop.Raw) ;

		/* wait for MPI interface finish */
	   if(mode == XPON_RESET_HOLD_ON){
		  while(RETRY--) {
				  gponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;
				  if(gponMbiMpiStop.Bits.mbi_tx_stop_done) {
						 break;
				  }
		  }
	   }

	   return 0;
}

int gponDevMbiStop(XPON_RESET_MODE_t mode, XPON_GDM2CDM2_STOP_FLAG_t flag)
{
	REG_MBI_MPI_STOP gponMbiMpiStop ;
    int RETRY = 3000 ;

	if((mode != XPON_RESET_HOLD_ON) && (mode != XPON_RESET_RELEASE)) {
		return -1;
	}

	/* STOP/START MBI Interface ; 1:stop; 0:start*/
	gponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;    

    gponMbiMpiStop.Bits.mbi_rx_stop = (mode == XPON_RESET_HOLD_ON) ? 1 : 0;
    gponMbiMpiStop.Bits.mbi_tx_stop = (mode == XPON_RESET_HOLD_ON) ? 1 : 0; 
	IO_SREG(MBI_MPI_STOP, gponMbiMpiStop.Raw) ;
    /* wait for MPI interface finish */
    if(mode == XPON_RESET_HOLD_ON){
       while(RETRY--) {
           gponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;
           if((gponMbiMpiStop.Bits.mbi_rx_stop_done)&&(gponMbiMpiStop.Bits.mbi_tx_stop_done)) {               
              break;
           }
       }       
    }
	if(flag == XPON_WITH_GDM2CDM2_STOP)
		feDevGdm2Cdm2Stop(mode);
	
	return 0 ;
}

int gponDevMpiTxStop(XPON_RESET_MODE_t mode)
{
	REG_MBI_MPI_STOP gponMbiMpiStop ;
    int RETRY = 3000 ;

	if((mode != XPON_RESET_HOLD_ON) && (mode != XPON_RESET_RELEASE)) {
		return -1;
	}		
    /* STOP/START MPI Interface ; 1:stop; 0:start*/
	gponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;
    
    gponMbiMpiStop.Bits.mpi_tx_stop =  (mode == XPON_RESET_HOLD_ON) ? 1 : 0;
	IO_SREG(MBI_MPI_STOP, gponMbiMpiStop.Raw) ;

    /* wait for MPI interface finish */
    if(mode == XPON_RESET_HOLD_ON){
    	while(RETRY--) {
    		gponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;
    		if(gponMbiMpiStop.Bits.mpi_tx_stop_done) {
    			break  ;
    		}
    	}
    }
	return 0 ;
}

int gponDevMpiRxStop(XPON_RESET_MODE_t mode)
{
	REG_MBI_MPI_STOP gponMbiMpiStop ;
    int RETRY = 3000 ;

	if((mode != XPON_RESET_HOLD_ON) && (mode != XPON_RESET_RELEASE)) {
		return -1;
	}		
    /* STOP/START MPI Interface ; 1:stop; 0:start*/
	gponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;
    
    gponMbiMpiStop.Bits.mpi_rx_stop =  (mode == XPON_RESET_HOLD_ON) ? 1 : 0;
	IO_SREG(MBI_MPI_STOP, gponMbiMpiStop.Raw) ;

    /* wait for MPI interface finish */
    if(mode == XPON_RESET_HOLD_ON){
    	while(RETRY--) {
    		gponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;
    		if(gponMbiMpiStop.Bits.mpi_rx_stop_done) {
    			break  ;
    		}
    	}
    }
	return 0 ;
}


int gponDevMpiStop(XPON_RESET_MODE_t mode)
{
	REG_MBI_MPI_STOP gponMbiMpiStop ;
    int RETRY = 3000 ;

	if((mode != XPON_RESET_HOLD_ON) && (mode != XPON_RESET_RELEASE)) {
		return -1;
	}		
    /* STOP/START MPI Interface ; 1:stop; 0:start*/
	gponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;
    
    gponMbiMpiStop.Bits.mpi_rx_stop =  (mode == XPON_RESET_HOLD_ON) ? 1 : 0;        
    gponMbiMpiStop.Bits.mpi_tx_stop =  (mode == XPON_RESET_HOLD_ON) ? 1 : 0;
	IO_SREG(MBI_MPI_STOP, gponMbiMpiStop.Raw) ;

    /* wait for MPI interface finish */
    if(mode == XPON_RESET_HOLD_ON){
    	while(RETRY--) {
    		gponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;
    		if((gponMbiMpiStop.Bits.mpi_rx_stop_done)&&(gponMbiMpiStop.Bits.mpi_tx_stop_done)) {
    			break  ;
    		}
    	}
    }
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
void gponGetNGPonMode(uint *mode)
{
	*mode = IO_GREG(0xbfb00070);
}
/*****************************************************************************
******************************************************************************/
void gponDevSetRspTime(ushort time)
{
	REG_RSP_TIME gponRspTime ;
	
	gponRspTime.Raw = IO_GREG(RSP_TIME) ;
	gponRspTime.Bits.tresp = time ;
	IO_SREG(RSP_TIME, gponRspTime.Raw) ;
}
/*****************************************************************************
******************************************************************************/
void gponDevGetRspTime(ushort *time)
{
	REG_RSP_TIME gponRspTime ;
	
	gponRspTime.Raw = IO_GREG(RSP_TIME) ;
	*time = gponRspTime.Bits.tresp ;
}
/*****************************************************************************
******************************************************************************/
int gponDevSetSerialNumber(unchar *sn)
{
	REG_VENDOR_ID gponVendor ;
	REG_VS_SN gponSn ;
	
	gponVendor.Bits.vendor_id = (sn[0]<<24)|(sn[1]<<16)|(sn[2]<<8)|(sn[3]) ;
	IO_SREG(VENDOR_ID, gponVendor.Raw) ;
	
	gponSn.Bits.vs_sn = (sn[4]<<24)|(sn[5]<<16)|(sn[6]<<8)|(sn[7]) ;
	IO_SREG(VS_SN, gponSn.Raw) ;
	
	return 0 ;
}
/*****************************************************************************
******************************************************************************/
void gponDevSetOnuId(uint onuId, unchar valid)
{
	REG_ONU_ID gponOnuId ;
	
	gponOnuId.Raw = IO_GREG(ONU_ID) ;
	gponOnuId.Bits.onu_id_vld = valid ;
	gponOnuId.Bits.onu_id = onuId ;
	IO_SREG(ONU_ID, gponOnuId.Raw) ;
	
}

/*****************************************************************************
******************************************************************************/
void gponDevSetRegId(unchar *regId)
{
	REG_RGS_ID3_0  gponRegId3_0 ;
	REG_RGS_ID7_4  gponRegId7_4 ;
	REG_RGS_ID11_8  gponRegId11_8 ;
	REG_RGS_ID15_12  gponRegId15_12 ;
	REG_RGS_ID19_16  gponRegId19_16 ;
	REG_RGS_ID23_20  gponRegId23_20 ;
	REG_RGS_ID27_24  gponRegId27_24 ;
	REG_RGS_ID31_28  gponRegId31_28 ;
	REG_RGS_ID35_32  gponRegId35_32 ;

	gponRegId3_0.Bits.rgs_id3_0 = (regId[32]<<24)|(regId[33]<<16)|(regId[34]<<8)|(regId[35]) ;
	IO_SREG(RGS_ID3_0, gponRegId3_0.Raw) ;
	gponRegId7_4.Bits.rgs_id7_4 = (regId[28]<<24)|(regId[29]<<16)|(regId[30]<<8)|(regId[31]) ;
	IO_SREG(RGS_ID7_4, gponRegId7_4.Raw) ;
	gponRegId11_8.Bits.rgs_id11_8 = (regId[24]<<24)|(regId[25]<<16)|(regId[26]<<8)|(regId[27]) ;
	IO_SREG(RGS_ID11_8, gponRegId11_8.Raw) ;
	gponRegId15_12.Bits.rgs_id15_12 = (regId[20]<<24)|(regId[21]<<16)|(regId[22]<<8)|(regId[23]) ;
	IO_SREG(RGS_ID15_12, gponRegId15_12.Raw) ;
	gponRegId19_16.Bits.rgs_id19_16 = (regId[16]<<24)|(regId[17]<<16)|(regId[18]<<8)|(regId[19]) ;
	IO_SREG(RGS_ID19_16, gponRegId19_16.Raw) ;
	gponRegId23_20.Bits.rgs_id23_20 = (regId[12]<<24)|(regId[13]<<16)|(regId[14]<<8)|(regId[15]) ;
	IO_SREG(RGS_ID23_20, gponRegId23_20.Raw) ;	
	gponRegId27_24.Bits.rgs_id27_24 = (regId[8]<<24)|(regId[9]<<16)|(regId[10]<<8)|(regId[11]) ;
	IO_SREG(RGS_ID27_24, gponRegId27_24.Raw) ;
	gponRegId31_28.Bits.rgs_id31_28 = (regId[4]<<24)|(regId[5]<<16)|(regId[6]<<8)|(regId[7]) ;
	IO_SREG(RGS_ID31_28, gponRegId31_28.Raw) ;
	gponRegId35_32.Bits.rgs_id35_32 = (regId[0]<<24)|(regId[1]<<16)|(regId[2]<<8)|(regId[3]) ;
	IO_SREG(RGS_ID35_32, gponRegId35_32.Raw) ;
    
}
/*****************************************************************************
******************************************************************************/
void gponDevSetProfileInfo(unchar profIndex, uint profLens, unchar version)
{
	REG_US_PROF_VLD gponProfValid;
	REG_US_PROF_PSBu_LEN_0_1 gponProfLen01;    
	REG_US_PROF_PSBu_LEN_2_3 gponProfLen23;

    if(profIndex >3){        
		PON_MSG((MSG_ERR), "Wrong profile index:%d \n",profIndex) ;
        return;
    }
    
	gponProfValid.Raw = IO_GREG(US_PROF_VLD) ;
	gponProfLen01.Raw = IO_GREG(US_PROF_PSBu_LEN_0_1) ;	
    gponProfLen23.Raw = IO_GREG(US_PROF_PSBu_LEN_2_3) ;
	if(profIndex ==0){
		gponProfValid.Bits.us_prof0_vld = 1 ;
		gponProfValid.Bits.us_prof0_vsn = version ;
		gponProfLen01.Bits.us_psbu_len_prof0 = profLens ;
	}else if(profIndex ==1){
		gponProfValid.Bits.us_prof1_vld = 1 ;
		gponProfValid.Bits.us_prof1_vsn = version ;
		gponProfLen01.Bits.us_psbu_len_prof1 = profLens ;
	}else if(profIndex ==2){
		gponProfValid.Bits.us_prof2_vld = 1 ;
		gponProfValid.Bits.us_prof2_vsn = version ;
		gponProfLen23.Bits.us_psbu_len_prof2 = profLens ;
	}else{
		gponProfValid.Bits.us_prof3_vld = 1 ;
		gponProfValid.Bits.us_prof3_vsn = version ;
		gponProfLen23.Bits.us_psbu_len_prof3 = profLens ;
    }
	IO_SREG(US_PROF_VLD, gponProfValid.Raw) ;
	IO_SREG(US_PROF_PSBu_LEN_0_1, gponProfLen01.Raw) ;
    IO_SREG(US_PROF_PSBu_LEN_2_3, gponProfLen23.Raw) ;

}

/*****************************************************************************
******************************************************************************/
void gponDevSetPonTag(unchar *ponTag)
{
	REG_PON_TAG_0 gponTag0 ;
	REG_PON_TAG_1 gponTag1 ;

	/*set the pon tag to mac*/

	gponTag0.Raw = IO_GREG(PON_TAG_0);
	gponTag0.Bits.pon_tag_0 =  (ponTag[4]<<24)|(ponTag[5]<<16)|(ponTag[6]<<8)|(ponTag[7]) ;
	IO_SREG(PON_TAG_0, gponTag0.Raw) ;
	
	gponTag1.Raw = IO_GREG(PON_TAG_1) ;
	gponTag1.Bits.pon_tag_1 = (ponTag[0]<<24)|(ponTag[1]<<16)|(ponTag[2]<<8)|(ponTag[3]);
	IO_SREG(PON_TAG_1, gponTag1.Raw) ;
	
}
/******************************************************************************
 Descriptor:	Set tcont info .
 Input Args:	isValid:          tcont valid or invalid.
                 	tcont_index: tcont index .
                 	allocId:          tcont alloc ID.
 Ret Value:	GPON_TCONT_CMD_SUCCESS: Success.
               	GPON_TCONT_CMD_FAIL:        HW has something error.
******************************************************************************/
GPON_TCONT_CMD_RESULT_t gponDevSetTCont(GPON_TCONT_t isValid, int tcont_index, ushort allocId)
{
	REG_TCONT_ID_CFG gponTcontCfg ;
	REG_TCONT_ID_STS gponTcontSts ;
	
	int RETRY = 3000 ;

	if(tcont_index < 0 || tcont_index > 31) {
		return GPON_TCONT_CMD_FAIL;
	}
	/* setting cfg */
	gponTcontCfg.Bits.tcont_cmd = GPON_TCONT_WRITE ;
	gponTcontCfg.Bits.wr_tcont_id_vld = isValid ;
	gponTcontCfg.Bits.tcont_id_index = tcont_index ;
	gponTcontCfg.Bits.wr_tcont_id = allocId ;
	IO_SREG(TCONT_ID_CFG, gponTcontCfg.Raw) ;

	/* Wait for done */
	while(RETRY--) {
		gponTcontSts.Raw = IO_GREG(TCONT_ID_STS) ;
		if(gponTcontSts.Bits.tcont_cmd_done) {
			return GPON_TCONT_CMD_SUCCESS;
		} 
		udelay(1) ;
	}

	PON_MSG(MSG_ERR, "[%s:%s]\n  Fail to set tcont:%d.", strrchr(__FILE__, '/') + 1, __func__, tcont_index) ;
	return GPON_TCONT_CMD_FAIL;
}
/******************************************************************************
 Descriptor:	Get tcont info .
 Input Args:	isValid:          tcont valid or invalid.
                 	tcont_index: tcont index.
                 	allocId:          tcont alloc ID.
 Ret Value:	GPON_TCONT_CMD_SUCCESS: Success.
               	GPON_TCONT_CMD_FAIL:        HW has something error.
******************************************************************************/
static GPON_TCONT_CMD_RESULT_t gponDevGetTCont(GPON_TCONT_t *isValid, int tcont_index, ushort *allocId)
{
	REG_TCONT_ID_CFG gponTcontCfg;
	REG_TCONT_ID_STS gponTcontSts;
	
	int RETRY = 3000;

	if(tcont_index < 0 || tcont_index >= CONFIG_GPON_10G_MAX_TCONT) {
		return GPON_TCONT_CMD_FAIL;
	}
	
	/* setting cfg */
	gponTcontCfg.Bits.tcont_cmd = GPON_TCONT_READ;
	gponTcontCfg.Bits.tcont_id_index = tcont_index ;
	IO_SREG(TCONT_ID_CFG, gponTcontCfg.Raw) ;

	/* Wait for done */
	while(RETRY--) {
		gponTcontSts.Raw = IO_GREG(TCONT_ID_STS) ;
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
/*****************************************************************************
Descriptor:	disable tcont alloc ID.
 Input Args:	allocId:          tcont alloc ID.
 Ret Value:	: .
               	:   
******************************************************************************/
int gponDevDisableTCont(ushort allocId)
{
	int i=0;
	GPON_TCONT_t valid;
	ushort id =0;
	GPON_TCONT_CMD_RESULT_t result;

	for(i = 0 ; i < CONFIG_GPON_10G_MAX_TCONT ; i++) {
		/* read tcont status */
		result = gponDevGetTCont(&valid, i, &id);
		if(result == GPON_TCONT_CMD_SUCCESS) {
			if(id == allocId) {
				/* match allocId */
				result = gponDevSetTCont(GPON_TCONT_INVALID, i, 0x3FF);
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
Descriptor:	enable  tcont alloc ID.
 Input Args:	allocId:          tcont alloc ID.
 Ret Value:	: .
               	: .
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
	for(i = 1 ; i < CONFIG_GPON_10G_MAX_TCONT ; i++) {
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
******************************************************************************/
int gponDevDumpTcontInfo(void)
{
	int i=0;
	GPON_TCONT_t valid;
	ushort id=0;
	GPON_TCONT_CMD_RESULT_t result;

	for(i = 1 ; i < CONFIG_GPON_10G_MAX_TCONT ; i++) {
		/* read tcont status */
		result = gponDevGetTCont(&valid, i, &id);
        if(result == GPON_TCONT_CMD_SUCCESS) {
			printk("TCONT%d ID:%d, Valid:%s\n", i, id, 
						(valid == GPON_TCONT_VALID) ? "Valid" : "Invalid");
		}else {
			return -1;
		}         
	}    
	return 0 ;
}
/*****************************************************************************
******************************************************************************/
int gponDevAssignNewAllocId(unsigned long data){

	AllocId_Config_t * gponConfig = (AllocId_Config_t *)data ;

	if(gponConfig->allocIdType == PLOAM_ALLOC_ID_ASSIGN){
		if (0 ==gwan_create_new_tcont((ushort)(gponConfig->allocId))){ 
			gpWanPriv->activeChannelNum ++ ;
			if (gpWanPriv->activeChannelNum > CONFIG_GPON_10G_MAX_TCONT) {				
				PON_MSG((MSG_ERR|MSG_OAM), "AllocId has exceed the max num %d\n", CONFIG_GPON_10G_MAX_TCONT) ;
				gpWanPriv->activeChannelNum = CONFIG_GPON_10G_MAX_TCONT ;
			}
			xpon_reset_qdma_tx_buf();
            /* report GPON event message */
			xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_TCONT_ALLOCED, gponConfig->allocId) ;
			return 0;
		}else{
			PON_MSG((MSG_ERR|MSG_OAM), "Allocate alloc id fail\n") ;			
			return -1;
		}
	}else if(gponConfig->allocIdType == PLOAM_ALLOC_ID_DEALLOCATE){
		if ( 0 == gwan_remove_tcont((ushort)(gponConfig->allocId))){  
			gpWanPriv->activeChannelNum -- ;
			if (gpWanPriv->activeChannelNum < 1)
				gpWanPriv->activeChannelNum = 1 ;
			//xpon_reset_qdma_tx_buf(); 
		}
	}else{
		PON_MSG((MSG_ERR|MSG_OAM), "wrong alloc id config type\n") ;
	}
	return 0;
}
/*****************************************************************************
******************************************************************************/
void gponDevResetGemInfo(void) 
{
	int i, RETRY = RETRY_TIMES ;
	REG_GEM_PORT_CFG   gponGemPortCfg ;
	REG_GEM_PORT_STS   gponGemPortStatus ;
	
	for(i=0 ; i<GPON_10G_MAX_GEM_ID ; i++) {
		gponGemPortCfg.Raw = 0 ;
		gponGemPortCfg.Bits.gpid_cmd = 1 ; /*Write Command*/
		gponGemPortCfg.Bits.gpid_vld =  0 ;
		gponGemPortCfg.Bits.gpid_type = 1 ; /*unicast type*/
		gponGemPortCfg.Bits.gpid_us_encrypt = 0 ;
		gponGemPortCfg.Bits.gem_port_id = i ;
		IO_SREG(GEM_PORT_CFG, gponGemPortCfg.Raw) ;
		
		RETRY = 3 ;
		while(RETRY--) {
			gponGemPortStatus.Raw = IO_GREG(GEM_PORT_STS) ;
			if(gponGemPortStatus.Bits.gpid_cmd_done) {
				break ;
			}
            udelay(1);
		}        
	}    
	PON_MSG(MSG_ERR, "%s %d failed to reset gem info.\n",__FUNCTION__,__LINE__) ;
	return ;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetGemInfo(ushort gemPortId, unchar *pValid, unchar *pType,unchar *pEncrypted)
{
	int RETRY = RETRY_TIMES ;
	REG_GEM_PORT_CFG gponGemPortCfg ;
	REG_GEM_PORT_STS gponGemPortStatus ;
	
	if(gemPortId == 0xFFFF) {
		return -EFAULT ;
	}
	
	gponGemPortCfg.Bits.gpid_cmd = 0 ; /*Read Command*/
	gponGemPortCfg.Bits.gem_port_id = gemPortId ;
	IO_SREG(GEM_PORT_CFG, gponGemPortCfg.Raw) ;

	while(RETRY--) {
		gponGemPortStatus.Raw = IO_GREG(GEM_PORT_STS) ;
		if(gponGemPortStatus.Bits.gpid_cmd_done) {
			*pValid = (gponGemPortStatus.Bits.gpid_rd_sts & 0x4) ? XPON_ENABLE : XPON_DISABLE ;
			*pType = (gponGemPortStatus.Bits.gpid_rd_sts & 0x2) ? XPON_DISABLE : XPON_ENABLE ;  /*unicast or boradcast type*/
			*pEncrypted = (gponGemPortStatus.Bits.gpid_rd_sts & 0x1) ? XPON_ENABLE : XPON_DISABLE ;
			
			return 0 ;
		} 
		udelay(1) ;
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
	unchar valid=0,type=0,encrypt=0 ;
	int i = 0, ret = 0;
	
	for(i=0 ; i <= GPON_10G_MAX_VALID_GEM_ID ; i++) {
    	if(i%200==0){
            msleep(5);
        }    
		ret = gponDevGetGemInfo(i, &valid, &type, &encrypt) ;
		if(ret != 0) {
			printk("Get GEM port ID:%d failed\n", i) ;	
			return -EFAULT ;
		} else {
			 if(valid ==XPON_ENABLE){
				printk("GEM ID:%06d, Valid,\tType:%s,\tEncryption:%s\n", i, 
								(type == GPON_UNICAST_GEM) 	 ? "unicast" : "broadcast" ,
								(encrypt == XPON_ENABLE && (gpGponPriv->gemUpAESMode != UPAES_MODE_NONE)) ? "Encrypted" : "Not encrypted") ;
			}
		}
	}

	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevSetGemInfoNoCheck(ushort gemPortId, unchar isValid, unchar isType,unchar isEncrypted)
{
	int RETRY = RETRY_TIMES ;
	REG_GEM_PORT_CFG gponGemPortCfg ;
	REG_GEM_PORT_STS gponGemPortStatus ;
	
	if(gemPortId == 0xFFFF) {
		return -EFAULT ;	
	}
	gponGemPortCfg.Raw = 0 ;
	gponGemPortCfg.Bits.gpid_cmd = 1 ; /*Write Command*/
	gponGemPortCfg.Bits.gpid_vld = (isValid==XPON_ENABLE) ? 1 : 0 ;
	gponGemPortCfg.Bits.gpid_type = (isType==XPON_ENABLE) ? 0 : 1 ;  /*for 10GPON , 0 is broadcast ,1 is unicast*/
	gponGemPortCfg.Bits.gpid_us_encrypt = (isEncrypted==XPON_ENABLE) ? 1 : 0 ; 
	gponGemPortCfg.Bits.gem_port_id = gemPortId ;
	IO_SREG(GEM_PORT_CFG, gponGemPortCfg.Raw) ;
    
	while(RETRY--) {
		gponGemPortStatus.Raw = IO_GREG(GEM_PORT_STS) ;
		if(gponGemPortStatus.Bits.gpid_cmd_done) {
			return 0 ;
		} 
		udelay(1) ;
	}
	PON_MSG(MSG_ERR, "%s %d failed to set gem info.\n",__FUNCTION__,__LINE__) ;
	return -ETIME ;
}

#if defined(TCSUPPORT_CPU_EN7580) /*EN7581 Will fix this creating gemport bug.*/
/*****************************************************************************
******************************************************************************/
int gponDevCheckGemExist(ushort gemPortId, unchar isType,unchar isEncrypted)
{
	unchar valid=0,type=0,encrypt=0 ;
	int ret = 0;

	ret = gponDevGetGemInfo(gemPortId, &valid, &type, &encrypt) ;
	if(ret == 0 && valid ==XPON_ENABLE && type == isType && encrypt == isEncrypted){
		PON_MSG(MSG_TRACE, "GEM ID:%d, exist\n", gemPortId) ;
		return 0 ;
	}
	PON_MSG(MSG_TRACE, "GEM ID:%d, not exist\n", gemPortId) ;
	return -EFAULT ;
}
int gponDevGetSwGemInfo(ushort gemId, GEM_INFO_T *pSwGemInfo)
{
	ushort gemIndex=0 ;

	if(pSwGemInfo == NULL)
		return -EFAULT ;
	pSwGemInfo->gemId = gemId;
	gemIndex = (gpWanPriv->gpon.gemIdToIndex[gemId] & GPON_GEM_IDX_MASK);
	if(GPON_GEM_IDX_MASK == gemIndex){
		pSwGemInfo->valid = 0;
	}else {
		pSwGemInfo->valid = gpWanPriv->gpon.gemPort[gemIndex].info.valid;
		pSwGemInfo->encrypt = (gpWanPriv->gpon.gemPort[gemIndex].info.txEncrypt) ? XPON_ENABLE : XPON_DISABLE ;
		if(gpWanPriv->gpon.gemPort[gemIndex].info.channel >= GPON_MULTICAST_CHANNEL) {
			pSwGemInfo->type = GPON_MULTICAST_GEM ;
		} else {
			pSwGemInfo->type = GPON_UNICAST_GEM ;
		}
	}
	return 0;
}
int gponDevcheckGemInfo(GEM_INFO_T *pSwGemInfo, GEM_INFO_T *pRdGemInfo)
{
	unchar valid=0,type=0,encrypt=0 ;
	int ret = 0;

	if(pSwGemInfo == NULL || pRdGemInfo == NULL)
		return -EFAULT ;
		
	ret = gponDevGetGemInfo(pSwGemInfo->gemId, &valid, &type, &encrypt) ;
	if(ret != 0 || valid != pSwGemInfo->valid ||
		((valid == pSwGemInfo->valid)&&(valid == XPON_ENABLE)&&(type != pSwGemInfo->type || encrypt != pSwGemInfo->encrypt))){
		PON_MSG(MSG_TRACE, "SW info:GEM ID:%06d, Valid:%d,\tType:%s,\tEncryption:%s\n", pSwGemInfo->gemId, pSwGemInfo->valid,
						(pSwGemInfo->type == XPON_ENABLE)	? "broadcast" : "unicast" ,
						(pSwGemInfo->encrypt == XPON_ENABLE) ? "Encrypted" : "Not encrypted") ;
	
		PON_MSG(MSG_TRACE, "HW info:GEM ID:%06d, Valid:%d,\tType:%s,\tEncryption:%s\n", pSwGemInfo->gemId, valid,
						(type == XPON_ENABLE)	 ? "broadcast" : "unicast" ,
						(encrypt == XPON_ENABLE) ? "Encrypted" : "Not encrypted") ;
		PON_MSG(MSG_TRACE, "[%s] [%d] Get GEM port ID:%d info mismatch\n",__FUNCTION__,__LINE__,pSwGemInfo->gemId) ;
		/*Record the mismatch gemport.*/
		memcpy(pRdGemInfo, pSwGemInfo, sizeof(GEM_INFO_T));
		return 0;
	}

	PON_MSG(MSG_TRACE, "[%s] [%d] Get GEM port ID:%d info match\n",__FUNCTION__,__LINE__,pSwGemInfo->gemId) ;
	return -EFAULT ;
}

/*****************************************************************************
******************************************************************************/
int gponDevCheckArrayGemInfo(ushort gemPortId)
{
	int RETRY = CHECK_RETRY_TIMES ;
	GEM_INFO_T swGemInfo[32];
	GEM_INFO_T recordGemInfo[32];
	int i = 0, n = 0;
	ushort gemId = 0;

	PON_MSG(MSG_TRACE, "[%s] [%d] ..... enter .....\n",__FUNCTION__,__LINE__) ;
	memset(swGemInfo, 0, sizeof(swGemInfo)) ;
	memset(recordGemInfo, 0, sizeof(recordGemInfo)) ;

	/*Step 1: Check relevant 32 gemports!*/
	gemId = (gemPortId & 0xFFE0); /*as 0~31,32~63,...,1024~1053,...,65504~65535*/
	for(i=0; i<32; i++){
		swGemInfo[i].gemId = gemId;
		if(gemId == 0xFFFF)
		{
			break;
		}
		gponDevGetSwGemInfo(gemId, &swGemInfo[i]);
		if(gponDevcheckGemInfo(&swGemInfo[i], &recordGemInfo[n]) == 0){
			n ++;
		}
		gemId ++;
	}
	/*All relevant 32 gemports match*/
	if(n == 0){
		PON_MSG(MSG_TRACE, "[%s] [%d] ..... success .....\n",__FUNCTION__,__LINE__) ;
		return 0 ;
	}

	while(RETRY--){
		PON_MSG(MSG_TRACE, "[%s] [%d] some gemports mismatch, start retry %d times\n",__FUNCTION__,__LINE__,(CHECK_RETRY_TIMES-RETRY)) ;
		/*Step 2: Reconfig these mismatch gemports!*/
		for(i=0; i<n; i++){
			gponDevSetGemInfoNoCheck(recordGemInfo[i].gemId, recordGemInfo[i].valid, recordGemInfo[i].type, recordGemInfo[i].encrypt);
		}
		/*Clear num.*/
		n = 0;
		/*Step 3: Check relevant 32 gemports again!*/
		for(i=0; i<32; i++){
			if(swGemInfo[i].gemId == 0xFFFF)
			{
				break;
			}
			if(gponDevcheckGemInfo(&swGemInfo[i], &recordGemInfo[n]) == 0){
				n ++;
			}
		}
		/*All relevant 32 gemports match*/
		if(n == 0){
			printk("[%s] [%d] ..... Retry %d times success .....\n",__FUNCTION__,__LINE__,(CHECK_RETRY_TIMES-RETRY)) ;
			return 0 ;
		}
	}
	printk("[%s] [%d] .....Retry %d times fail .....\n",__FUNCTION__,__LINE__,CHECK_RETRY_TIMES) ;
	return -EFAULT ;
}

/*****************************************************************************
******************************************************************************/
int gponDevCheckGemInfoInvalid(ushort gemPortId)
{
	GEM_INFO_T swGemInfo;
	GEM_INFO_T recordGemInfo;
	int i = 0;
	int j = 0;
	ushort gemId = 0;
	int ret = 0;

	PON_MSG(MSG_TRACE, "[%s] [%d] ..... enter .....\n",__FUNCTION__,__LINE__);
	memset(&swGemInfo, 0, sizeof(GEM_INFO_T));
	memset(&recordGemInfo, 0, sizeof(GEM_INFO_T));

	/*Step 1: set relevant 256 gemports!*/
	gemId = (gemPortId & 0xFFE0); /*as 0~31,32~63,...,1024~1053,...,65504~65535*/
	
	for(i = 0; i < 256; i++)
	{
		swGemInfo.gemId = gemId;
		
		if(gemId == 0xFFFF)
		{
			break;
		}
		
		gponDevGetSwGemInfo(gemId, &swGemInfo);
		ret = gponDevSetGemInfo(gemId, swGemInfo.valid, swGemInfo.type, (swGemInfo.encrypt)?XPON_ENABLE:XPON_DISABLE);
		if(0 != ret)
		{
			udelay(50);
			PON_MSG(MSG_TRACE, "[%s] [%d] gpon dev set gem info fail\n",__FUNCTION__,__LINE__);
		}

		/*Step 2: Check relevant 32 gemports!*/
		swGemInfo.gemId = (gemId & 0xFFE0); /*as 0~31,32~63,...,1024~1053,...,65504~65535*/
		for(j = 0; j < 32; j++)
		{	
			if(swGemInfo.gemId == 0xFFFF)
			{
				break;				
			}
			
			gponDevGetSwGemInfo(swGemInfo.gemId, &swGemInfo);			
			gponDevcheckGemInfo(&swGemInfo, &recordGemInfo);
			swGemInfo.gemId++;
		}
		
		gemId++;
	}

	return -EFAULT;;
}
#endif
/*****************************************************************************
******************************************************************************/
int gponDevSetGemInfo(ushort gemPortId, unchar isValid, unchar isType,unchar isEncrypted)
{
	int ret = 0;

	if(gemPortId == 0xFFFF) {
		return -EFAULT ;
	}

	PON_MSG(MSG_TRACE, "[%s] [%d] enter to set gem (%u) info, valid(%u).\n",__FUNCTION__,__LINE__,gemPortId,isValid) ;
	ret = gponDevCheckGemExist(gemPortId, isType, isEncrypted);
	if(0 == ret){
		PON_MSG(MSG_TRACE, "[%s] [%d] gem (%u) already exists.\n",__FUNCTION__,__LINE__,gemPortId) ;
		return 0;
	}
	ret = gponDevSetGemInfoNoCheck(gemPortId,isValid,isType,isEncrypted);
	if(0 != ret){
		PON_MSG(MSG_ERR, "[%s] [%d] failed to set gem (%u) info.\n",__FUNCTION__,__LINE__,gemPortId) ;
		return -ETIME ;
	}
#if defined(TCSUPPORT_CPU_EN7580) /*EN7581 Will fix this creating gemport bug.*/
	if (!hw_gem_config) {
		ret = gponDevCheckArrayGemInfo(gemPortId);
		if(0 != ret){
			PON_MSG(MSG_ERR, "[%s] [%d] failed to set gem (%u) info.\n",__FUNCTION__,__LINE__,gemPortId) ;
			return -ETIME ;
		}
	}
#endif
	PON_MSG(MSG_TRACE, "[%s] [%d] create gem (%u) success .\n",__FUNCTION__,__LINE__,gemPortId) ;
	return 0;
}

/*****************************************************************************
******************************************************************************/
int gponDevSetErrMicPloamDrop(XPON_Mode_t mode)
{
	REG_EPDROP_EN gponMicDrop ;
	
	if(mode != XPON_DISABLE && mode != XPON_ENABLE) {
		return -1;
	}
	gponMicDrop.Raw = IO_GREG(EPDROP_EN) ;
	gponMicDrop.Bits.errploamd_drop_en = mode ;
	IO_SREG(EPDROP_EN, gponMicDrop.Raw) ;

	return 0;
}

/*****************************************************************************
******************************************************************************/
void gponDevGetErrMicPloamDrop(XPON_Mode_t *mode)
{
	*mode = IO_GREG(EPDROP_EN) ;
}

/*****************************************************************************
******************************************************************************/
int gponDevSetDyingGaspMode(GPON_10G_DYING_GASP_MODE_T *arg)
{
	REG_US_DYING_GASP_CTRL gponDGCtrl ;
	
	if(arg->dyingGaspCtrl != XGPON_SW && arg->dyingGaspCtrl != XGPON_HW) {
		return -1;
	}
	gponDGCtrl.Raw = IO_GREG(US_DYING_GASP_CTRL) ;
    gponDGCtrl.Bits.hw_dying_gasp_en = arg->dyingGaspCtrl;    
    gponDGCtrl.Bits.dying_gasp_send_num = arg->dyingGaspNum;
	IO_SREG(US_DYING_GASP_CTRL, gponDGCtrl.Raw) ;
    if(arg->dyingGaspCtrl == XGPON_SW){
        gponDGCtrl.Bits.sw_dying_gasp_trig = XPON_ENABLE ;
	IO_SREG(US_DYING_GASP_CTRL, gponDGCtrl.Raw) ;
    }    

	return 0;
}
/*****************************************************************************
******************************************************************************/
int gponDevGetDyingGaspMode(GPON_10G_DYING_GASP_MODE_T *arg)
{
	REG_US_DYING_GASP_CTRL gponDGCtrl ;

	if(arg == NULL)
		return -1;
	
	gponDGCtrl.Raw = IO_GREG(US_DYING_GASP_CTRL) ;
    arg->dyingGaspCtrl = gponDGCtrl.Bits.hw_dying_gasp_en ;
    arg->dyingGaspNum = gponDGCtrl.Bits.dying_gasp_send_num ;
    arg->hwDGStatus = gponDGCtrl.Bits.hw_dying_gasp_sts ;
	
	return 0;
}

/*****************************************************************************
Descriptor:    set unicast data aes encryption first key
Input Args:    none.
Ret Value:     none.
******************************************************************************/
void gponDevSetAesUcKey0(unchar *key){
	
	REG_AES_UC_IDX0_KEY0 unicastKey0 ;
	REG_AES_UC_IDX0_KEY1 unicastKey1 ;
	REG_AES_UC_IDX0_KEY2 unicastKey2 ;
	REG_AES_UC_IDX0_KEY3 unicastKey3 ;
	
	unicastKey0.Bits.aes_uc_idx0_key0 = (key[12]<<24)|(key[13]<<16)|(key[14]<<8)|(key[15]) ;
	IO_SREG(AES_UC_IDX0_KEY0, unicastKey0.Raw) ;
	unicastKey1.Bits.aes_uc_idx0_key1 = (key[8]<<24)|(key[9]<<16)|(key[10]<<8)|(key[11]) ;
	IO_SREG(AES_UC_IDX0_KEY1, unicastKey1.Raw) ;
	unicastKey2.Bits.aes_uc_idx0_key2 = (key[4]<<24)|(key[5]<<16)|(key[6]<<8)|(key[7]) ;
	IO_SREG(AES_UC_IDX0_KEY2, unicastKey2.Raw) ;
	unicastKey3.Bits.aes_uc_idx0_key3 = (key[0]<<24)|(key[1]<<16)|(key[2]<<8)|(key[3]) ;
	IO_SREG(AES_UC_IDX0_KEY3, unicastKey3.Raw) ;
}
/*****************************************************************************
Descriptor:    clear unicast data aes encryption first key
Input Args:    none.
Ret Value:     none.
******************************************************************************/
void gponDevClearAesUcKey0(void){
	
	REG_AES_UC_IDX0_KEY0 unicastKey0 ;
	REG_AES_UC_IDX0_KEY1 unicastKey1 ;
	REG_AES_UC_IDX0_KEY2 unicastKey2 ;
	REG_AES_UC_IDX0_KEY3 unicastKey3 ;
	
	unicastKey0.Bits.aes_uc_idx0_key0 = 0;
	IO_SREG(AES_UC_IDX0_KEY0, unicastKey0.Raw) ;
	unicastKey1.Bits.aes_uc_idx0_key1 = 0 ;
	IO_SREG(AES_UC_IDX0_KEY1, unicastKey1.Raw) ;
	unicastKey2.Bits.aes_uc_idx0_key2 = 0 ;
	IO_SREG(AES_UC_IDX0_KEY2, unicastKey2.Raw) ;
	unicastKey3.Bits.aes_uc_idx0_key3 = 0 ;
	IO_SREG(AES_UC_IDX0_KEY3, unicastKey3.Raw) ;
}

/*****************************************************************************
Descriptor:    set unicast data aes encryption second key
Input Args:    none.
Ret Value:     none.
******************************************************************************/
void gponDevSetAesUcKey1(unchar *key){

	REG_AES_UC_IDX1_KEY0 unicastKey0 ;
	REG_AES_UC_IDX1_KEY1 unicastKey1 ;
	REG_AES_UC_IDX1_KEY2 unicastKey2 ;
	REG_AES_UC_IDX1_KEY3 unicastKey3 ;
	
	unicastKey0.Bits.aes_uc_idx1_key0 = (key[12]<<24)|(key[13]<<16)|(key[14]<<8)|(key[15]) ;
	IO_SREG(AES_UC_IDX1_KEY0, unicastKey0.Raw) ;
	unicastKey1.Bits.aes_uc_idx1_key1 = (key[8]<<24)|(key[9]<<16)|(key[10]<<8)|(key[11]) ;
	IO_SREG(AES_UC_IDX1_KEY1, unicastKey1.Raw) ;
	unicastKey2.Bits.aes_uc_idx1_key2 = (key[4]<<24)|(key[5]<<16)|(key[6]<<8)|(key[7]) ;
	IO_SREG(AES_UC_IDX1_KEY2, unicastKey2.Raw) ;
	unicastKey3.Bits.aes_uc_idx1_key3 = (key[0]<<24)|(key[1]<<16)|(key[2]<<8)|(key[3]) ;
	IO_SREG(AES_UC_IDX1_KEY3, unicastKey3.Raw) ;
}
/*****************************************************************************
Descriptor:    clear unicast data aes encryption second key
Input Args:    none.
Ret Value:     none.
******************************************************************************/
void gponDevClearAesUcKey1(void){

	REG_AES_UC_IDX1_KEY0 unicastKey0 ;
	REG_AES_UC_IDX1_KEY1 unicastKey1 ;
	REG_AES_UC_IDX1_KEY2 unicastKey2 ;
	REG_AES_UC_IDX1_KEY3 unicastKey3 ;
	
	unicastKey0.Bits.aes_uc_idx1_key0 = 0 ;
	IO_SREG(AES_UC_IDX1_KEY0, unicastKey0.Raw) ;
	unicastKey1.Bits.aes_uc_idx1_key1 = 0;
	IO_SREG(AES_UC_IDX1_KEY1, unicastKey1.Raw) ;
	unicastKey2.Bits.aes_uc_idx1_key2 = 0 ;
	IO_SREG(AES_UC_IDX1_KEY2, unicastKey2.Raw) ;
	unicastKey3.Bits.aes_uc_idx1_key3 = 0 ;
	IO_SREG(AES_UC_IDX1_KEY3, unicastKey3.Raw) ;
}
/*****************************************************************************
Descriptor:    set broadcast data aes encryption first key
Input Args:    none.
Ret Value:     none.
******************************************************************************/
void gponDevSetAesBcKey0(unchar *key){
	
	REG_AES_BC_IDX0_KEY0 broadcastKey0 ;
	REG_AES_BC_IDX0_KEY1 broadcastKey1 ;
	REG_AES_BC_IDX0_KEY2 broadcastKey2 ;
	REG_AES_BC_IDX0_KEY3 broadcastKey3 ;

	broadcastKey0.Bits.aes_bc_idx0_key0 = (key[12]<<24)|(key[13]<<16)|(key[14]<<8)|(key[15]) ;
	IO_SREG(AES_BC_IDX0_KEY0, broadcastKey0.Raw) ;
	broadcastKey1.Bits.aes_bc_idx0_key1 = (key[8]<<24)|(key[9]<<16)|(key[10]<<8)|(key[11]) ;
	IO_SREG(AES_BC_IDX0_KEY1, broadcastKey1.Raw) ;
	broadcastKey2.Bits.aes_bc_idx0_key2 = (key[4]<<24)|(key[5]<<16)|(key[6]<<8)|(key[7]) ;
	IO_SREG(AES_BC_IDX0_KEY2, broadcastKey2.Raw) ;
	broadcastKey3.Bits.aes_bc_idx0_key3 = (key[0]<<24)|(key[1]<<16)|(key[2]<<8)|(key[3]) ;
	IO_SREG(AES_BC_IDX0_KEY3, broadcastKey3.Raw) ;

}
/*****************************************************************************
Descriptor:    clean broadcast data aes encryption first key
Input Args:    none.
Ret Value:     none.
******************************************************************************/
void gponDevCleanAesBcKey0(void){
	
	REG_AES_BC_IDX0_KEY0 broadcastKey0 ;
	REG_AES_BC_IDX0_KEY1 broadcastKey1 ;
	REG_AES_BC_IDX0_KEY2 broadcastKey2 ;
	REG_AES_BC_IDX0_KEY3 broadcastKey3 ;

	broadcastKey0.Bits.aes_bc_idx0_key0 = 0;
	IO_SREG(AES_BC_IDX0_KEY0, broadcastKey0.Raw) ;
	broadcastKey1.Bits.aes_bc_idx0_key1 = 0;
	IO_SREG(AES_BC_IDX0_KEY1, broadcastKey1.Raw) ;
	broadcastKey2.Bits.aes_bc_idx0_key2 = 0;
	IO_SREG(AES_BC_IDX0_KEY2, broadcastKey2.Raw) ;
	broadcastKey3.Bits.aes_bc_idx0_key3 = 0;
	IO_SREG(AES_BC_IDX0_KEY3, broadcastKey3.Raw) ;
}
/*****************************************************************************
Descriptor:    set broadcast data aes encryption second key
Input Args:    none.
Ret Value:     none.
******************************************************************************/
void gponDevSetAesBcKey1(unchar *key){
	
	REG_AES_BC_IDX1_KEY0 broadcastKey0 ;
	REG_AES_BC_IDX1_KEY1 broadcastKey1 ;
	REG_AES_BC_IDX1_KEY2 broadcastKey2 ;
	REG_AES_BC_IDX1_KEY3 broadcastKey3 ;
	
	broadcastKey0.Bits.aes_bc_idx1_key0 = (key[12]<<24)|(key[13]<<16)|(key[14]<<8)|(key[15]) ;
	IO_SREG(AES_BC_IDX1_KEY0, broadcastKey0.Raw) ;
	broadcastKey1.Bits.aes_bc_idx1_key1 = (key[8]<<24)|(key[9]<<16)|(key[10]<<8)|(key[11]) ;
	IO_SREG(AES_BC_IDX1_KEY1, broadcastKey1.Raw) ;
	broadcastKey2.Bits.aes_bc_idx1_key2 = (key[4]<<24)|(key[5]<<16)|(key[6]<<8)|(key[7]) ;
	IO_SREG(AES_BC_IDX1_KEY2, broadcastKey2.Raw) ;
	broadcastKey3.Bits.aes_bc_idx1_key3 = (key[0]<<24)|(key[1]<<16)|(key[2]<<8)|(key[3]) ;
	IO_SREG(AES_BC_IDX1_KEY3, broadcastKey3.Raw) ;
	
}
/*****************************************************************************
Descriptor:    clean broadcast data aes encryption second key
Input Args:    none.
Ret Value:     none.
******************************************************************************/
void gponDevCleanAesBcKey1(void){
	
	REG_AES_BC_IDX1_KEY0 broadcastKey0 ;
	REG_AES_BC_IDX1_KEY1 broadcastKey1 ;
	REG_AES_BC_IDX1_KEY2 broadcastKey2 ;
	REG_AES_BC_IDX1_KEY3 broadcastKey3 ;
	
	broadcastKey0.Bits.aes_bc_idx1_key0 = 0;
	IO_SREG(AES_BC_IDX1_KEY0, broadcastKey0.Raw) ;
	broadcastKey1.Bits.aes_bc_idx1_key1 = 0;
	IO_SREG(AES_BC_IDX1_KEY1, broadcastKey1.Raw) ;
	broadcastKey2.Bits.aes_bc_idx1_key2 = 0;
	IO_SREG(AES_BC_IDX1_KEY2, broadcastKey2.Raw) ;
	broadcastKey3.Bits.aes_bc_idx1_key3 = 0;
	IO_SREG(AES_BC_IDX1_KEY3, broadcastKey3.Raw) ;
}
/*****************************************************************************
Descriptor:    set the AES CMAC engine to calculate keys
Input Args:    the key type of MSK,SK,OMCI_IK,PLOAM_IK,KEK
Ret Value:     GPON_KEY_GEN_SUCCESS:success, GPON_KEY_GEN_FAIL:fail.
******************************************************************************/
int gponDevKeyGenStart(uint keyType){

	REG_INT_STATUS intStatus ;
	REG_KEY_GEN gponKeyGen ;
	int RETRY = RETRY_TIMES ;

	/*Clear the interrupt status*/
	IO_SREG(INT_STATUS, GPON_KEY_CAL_DONE_INT) ;
	
	gponKeyGen.Raw = keyType ;	
	IO_SREG(KEY_GEN, gponKeyGen.Raw) ;
	
	/* Wait for done */
	while(RETRY--) {
		intStatus.Raw = IO_GREG(INT_STATUS) ;		
		if(intStatus.Bits.key_cal_done_int) {
			IO_SREG(INT_STATUS, GPON_KEY_CAL_DONE_INT) ;
			return GPON_KEY_GEN_SUCCESS ;
		}
		udelay(1) ;
	}	
	return GPON_KEY_GEN_FAIL ;
}
/*****************************************************************************
Descriptor:    set the MSK value
Input Args:    pointer of MSK
Ret Value:     none.
******************************************************************************/
void gponDevSetMSK(unchar *key){
	
	REG_MSK_0 msk0 ;
	REG_MSK_1 msk1 ;
	REG_MSK_2 msk2 ;
	REG_MSK_3 msk3 ;	

	msk0.Bits.msk0 = (key[12]<<24)|(key[13]<<16)|(key[14]<<8)|(key[15]) ;
	IO_SREG(MSK_0, msk0.Raw) ;
	msk1.Bits.msk1 = (key[8]<<24)|(key[9]<<16)|(key[10]<<8)|(key[11]) ;
	IO_SREG(MSK_1, msk1.Raw) ;
	msk2.Bits.msk2 = (key[4]<<24)|(key[5]<<16)|(key[6]<<8)|(key[7]) ;
	IO_SREG(MSK_2, msk2.Raw) ;
	msk3.Bits.msk3 = (key[0]<<24)|(key[1]<<16)|(key[2]<<8)|(key[3]) ;
	IO_SREG(MSK_3, msk3.Raw) ;
}
/*****************************************************************************
Descriptor:    get the AES CMAC calculated MSK value
Input Args:    pointer of key
Ret Value:     none.
******************************************************************************/
void gponDevGetRegMSK(uint *key){

#ifdef __BIG_ENDIAN
	key[0] = IO_GREG(REGMSK_3) ;
	key[1] = IO_GREG(REGMSK_2) ;
	key[2] = IO_GREG(REGMSK_1) ;
	key[3] = IO_GREG(REGMSK_0) ;
#else
    key[0] = ntohl(IO_GREG(REGMSK_3)) ;
	key[1] = ntohl(IO_GREG(REGMSK_2)) ;
	key[2] = ntohl(IO_GREG(REGMSK_1));
	key[3] = ntohl(IO_GREG(REGMSK_0)) ;
#endif	
}
/*****************************************************************************
Descriptor:    get the AES CMAC calculated SK value
Input Args:    pointer of key
Ret Value:     none.
******************************************************************************/
void gponDevGetSK(uint *key){

	key[0] = IO_GREG(SK_3) ;
	key[1] = IO_GREG(SK_2) ;
	key[2] = IO_GREG(SK_1) ;
	key[3] = IO_GREG(SK_0) ;
	
	PON_MSG(MSG_SECUR, "Hw Gen sk: %.8x %.8x %.8x %.8x\n",key[0],key[1],key[2],key[3]);
}
/*****************************************************************************
Descriptor:    get the OMCI_IK,PLOAM_IK,KEK after AES CMAC calculated done
Input Args:    pointer of key
Ret Value:     none.
******************************************************************************/
void gponDevGetHwGenKey(uint *key){

#ifdef __BIG_ENDIAN
	key[0] = IO_GREG(HW_GENK_3) ;
	key[1] = IO_GREG(HW_GENK_2) ;
	key[2] = IO_GREG(HW_GENK_1) ;
	key[3] = IO_GREG(HW_GENK_0) ; 
#else
    key[0] = ntohl(IO_GREG(HW_GENK_3)) ;
	key[1] = ntohl(IO_GREG(HW_GENK_2)) ;
	key[2] = ntohl(IO_GREG(HW_GENK_1)) ;
	key[3] = ntohl(IO_GREG(HW_GENK_0)) ;
#endif
	PON_MSG(MSG_SECUR, "Hw Gen key: %.8x %.8x %.8x %.8x\n",key[0],key[1],key[2],key[3]);
}
/*****************************************************************************
Descriptor:    set the first PLOAM_IK
Input Args:    pointer of key
Ret Value:     none.
******************************************************************************/
void gponDevSetPloamIk0(unchar *key){
	
	REG_PIK0_0 ploamIk0 ;
	REG_PIK0_1 ploamIk1 ;
	REG_PIK0_2 ploamIk2 ;
	REG_PIK0_3 ploamIk3 ;

	ploamIk0.Bits.pik0_0 = (key[12]<<24)|(key[13]<<16)|(key[14]<<8)|(key[15]) ;
	IO_SREG(PIK0_0, ploamIk0.Raw) ;
	ploamIk1.Bits.pik0_1 = (key[8]<<24)|(key[9]<<16)|(key[10]<<8)|(key[11]) ;
	IO_SREG(PIK0_1, ploamIk1.Raw) ;
	ploamIk2.Bits.pik0_2 = (key[4]<<24)|(key[5]<<16)|(key[6]<<8)|(key[7]) ;
	IO_SREG(PIK0_2, ploamIk2.Raw) ;
	ploamIk3.Bits.pik0_3 = (key[0]<<24)|(key[1]<<16)|(key[2]<<8)|(key[3]) ;
	IO_SREG(PIK0_3, ploamIk3.Raw) ;
}
/*****************************************************************************
Descriptor:    set the second PLOAM_IK
Input Args:    pointer of key
Ret Value:     none.
******************************************************************************/
void gponDevSetPloamIk1(unchar *key){
	
	REG_PIK1_0 ploamIk0 ;
	REG_PIK1_1 ploamIk1 ;
	REG_PIK1_2 ploamIk2 ;
	REG_PIK1_3 ploamIk3 ;

	ploamIk0.Bits.pik1_0 = (key[12]<<24)|(key[13]<<16)|(key[14]<<8)|(key[15]) ;
	IO_SREG(PIK1_0, ploamIk0.Raw) ;
	ploamIk1.Bits.pik1_1 = (key[8]<<24)|(key[9]<<16)|(key[10]<<8)|(key[11]) ;
	IO_SREG(PIK1_1, ploamIk1.Raw) ;
	ploamIk2.Bits.pik1_2 = (key[4]<<24)|(key[5]<<16)|(key[6]<<8)|(key[7]) ;
	IO_SREG(PIK1_2, ploamIk2.Raw) ;
	ploamIk3.Bits.pik1_3 = (key[0]<<24)|(key[1]<<16)|(key[2]<<8)|(key[3]) ;
	IO_SREG(PIK1_3, ploamIk3.Raw) ;
}
/*****************************************************************************
Descriptor:    set the first OMCI_IK
Input Args:    pointer of key
Ret Value:     none.
******************************************************************************/
void gponDevSetOmciIk0(unchar *key){
	
	REG_OIK0_0 omciIk0 ;
	REG_OIK0_1 omciIk1 ;
	REG_OIK0_2 omciIk2 ;
	REG_OIK0_3 omciIk3 ;
	
	omciIk0.Bits.oik0_0 = (key[12]<<24)|(key[13]<<16)|(key[14]<<8)|(key[15]) ;
	IO_SREG(OIK0_0, omciIk0.Raw) ;
	omciIk1.Bits.oik0_1 = (key[8]<<24)|(key[9]<<16)|(key[10]<<8)|(key[11]) ;
	IO_SREG(OIK0_1, omciIk1.Raw) ;
	omciIk2.Bits.oik0_2 = (key[4]<<24)|(key[5]<<16)|(key[6]<<8)|(key[7]) ;
	IO_SREG(OIK0_2, omciIk2.Raw) ;
	omciIk3.Bits.oik0_3 = (key[0]<<24)|(key[1]<<16)|(key[2]<<8)|(key[3]) ;
	IO_SREG(OIK0_3, omciIk3.Raw) ;
}
/*****************************************************************************
Descriptor:    set the second OMCI_IK
Input Args:    pointer of key
Ret Value:     none.
******************************************************************************/
void gponDevSetOmciIk1(unchar *key){
	
	REG_OIK1_0 omciIk0 ;
	REG_OIK1_1 omciIk1 ;
	REG_OIK1_2 omciIk2 ;
	REG_OIK1_3 omciIk3 ;

	omciIk0.Bits.oik1_0 = (key[12]<<24)|(key[13]<<16)|(key[14]<<8)|(key[15]) ;
	IO_SREG(OIK1_0, omciIk0.Raw) ;
	omciIk1.Bits.oik1_1 = (key[8]<<24)|(key[9]<<16)|(key[10]<<8)|(key[11]) ;
	IO_SREG(OIK1_1, omciIk1.Raw) ;
	omciIk2.Bits.oik1_2 = (key[4]<<24)|(key[5]<<16)|(key[6]<<8)|(key[7]) ;
	IO_SREG(OIK1_2, omciIk2.Raw) ;
	omciIk3.Bits.oik1_3 = (key[0]<<24)|(key[1]<<16)|(key[2]<<8)|(key[3]) ;
	IO_SREG(OIK1_3, omciIk3.Raw) ;
	
}
/*****************************************************************************
Descriptor:    set the first KEK
Input Args:    pointer of key
Ret Value:     none.
******************************************************************************/
void gponDevSetKEK0(unchar *key){
	
	REG_KEK0_0 kek0 ;
	REG_KEK0_1 kek1 ;
	REG_KEK0_2 kek2 ;
	REG_KEK0_3 kek3 ;

	kek0.Bits.kek0_0 = (key[12]<<24)|(key[13]<<16)|(key[14]<<8)|(key[15]) ;
	IO_SREG(KEK0_0, kek0.Raw) ;
	kek1.Bits.kek0_1 = (key[8]<<24)|(key[9]<<16)|(key[10]<<8)|(key[11]) ;
	IO_SREG(KEK0_1, kek1.Raw) ;
	kek2.Bits.kek0_2 = (key[4]<<24)|(key[5]<<16)|(key[6]<<8)|(key[7]) ;
	IO_SREG(KEK0_2, kek2.Raw) ;
	kek3.Bits.kek0_3 = (key[0]<<24)|(key[1]<<16)|(key[2]<<8)|(key[3]) ;
	IO_SREG(KEK0_3, kek3.Raw) ;
	
}
/*****************************************************************************
Descriptor:    set the second KEK
Input Args:    pointer of key
Ret Value:     none.
******************************************************************************/
void gponDevSetKEK1(unchar *key){
	
	REG_KEK1_0 kek0 ;
	REG_KEK1_1 kek1 ;
	REG_KEK1_2 kek2 ;
	REG_KEK1_3 kek3 ;

	kek0.Bits.kek1_0 = (key[12]<<24)|(key[13]<<16)|(key[14]<<8)|(key[15]) ;
	IO_SREG(KEK1_0, kek0.Raw) ;
	kek1.Bits.kek1_1 = (key[8]<<24)|(key[9]<<16)|(key[10]<<8)|(key[11]) ;
	IO_SREG(KEK1_1, kek1.Raw) ;
	kek2.Bits.kek1_2 = (key[4]<<24)|(key[5]<<16)|(key[6]<<8)|(key[7]) ;
	IO_SREG(KEK1_2, kek2.Raw) ;
	kek3.Bits.kek1_3 = (key[0]<<24)|(key[1]<<16)|(key[2]<<8)|(key[3]) ;
	IO_SREG(KEK1_3, kek3.Raw) ;
	
}
/*****************************************************************************
Descriptor:    set the PLOAM_IK index
Input Args:    key index
Ret Value:     none.
******************************************************************************/
void gponDevSetPloamIkIdx(unchar index){

	REG_SW_SET_KIDX gponKeyIdx;

	gponKeyIdx.Raw = IO_GREG(SW_SET_KIDX) ;	
	gponKeyIdx.Bits.sw_set_pik_idx = index ;
	gponKeyIdx.Bits.sw_set_pik_en = 1 ;
	IO_SREG(SW_SET_KIDX, gponKeyIdx.Raw) ;

}
/*****************************************************************************
Descriptor:    get the PLOAM_IK index
Input Args:    pointer of key index
Ret Value:     none.
******************************************************************************/
void gponDevGetPloamIkIdx(unchar *index){

	REG_CUR_KIDX gponCurKeyIdx;	
	
	gponCurKeyIdx.Raw = IO_GREG(CUR_KIDX) ;
	*index = gponCurKeyIdx.Bits.cur_pik_idx ;
}
/*****************************************************************************
Descriptor:    set the OMCI_IK index
Input Args:    key index
Ret Value:     none.
******************************************************************************/
void gponDevSetOmciIkIdx(unchar index){

	REG_SW_SET_KIDX gponKeyIdx;
	
	gponKeyIdx.Raw = IO_GREG(SW_SET_KIDX) ;	
	gponKeyIdx.Bits.sw_set_oik_idx = index ;
	gponKeyIdx.Bits.sw_set_oik_en = 1 ;	
	IO_SREG(SW_SET_KIDX, gponKeyIdx.Raw) ;
}
/*****************************************************************************
Descriptor:    get the OMCI_IK index
Input Args:    pointer of key index
Ret Value:     none.
******************************************************************************/
void gponDevGetOmciIkIdx(unchar *index){

	REG_CUR_KIDX gponCurKeyIdx;

	gponCurKeyIdx.Raw = IO_GREG(CUR_KIDX) ;
	*index = gponCurKeyIdx.Bits.cur_oik_idx ;
}
/******************************************************************************
Descriptor:    init the two key groups
Input Args:    none
Ret Value:     0:success;-1:fail
******************************************************************************/
void gponDevIntegrityKeySet(unsigned long arg){

	PON_MSG(MSG_SECUR, "PLOAM: init two key groups in O2_3\n") ;
	memcpy(gpGponPriv->gponSecurity.msk, gpGponPriv->gponSecurity.defaultMsk, GPON_MSK_LENS);
	gponDevSetMSK(gpGponPriv->gponSecurity.msk);

	/*key group1 based on default regitration id*/
	if(gponDevKeyGenStart(GPON_SK_GEN_START) == 0){
		PON_MSG(MSG_SECUR, "Hw generate sk based on default regId\n");
		gponDevGetSK((uint *)&gpGponPriv->gponSecurity.sk);
	}else{
		PON_MSG(MSG_ERR, "There is something wrong with sk calculate\n");
		return;
	}

	if(gponDevKeyGenStart(GPON_OMCI_IK_GEN_START) == 0){		
		PON_MSG(MSG_SECUR, "Hw generate omciIk 1\n");
		gponDevGetHwGenKey((int *)&gpGponPriv->gponSecurity.omciIk[GPON_OMCI_IK_IDX1]);
		gponDevSetOmciIk1(gpGponPriv->gponSecurity.omciIk[GPON_OMCI_IK_IDX1]);
	}else{
		PON_MSG(MSG_ERR, "There is something wrong with omci ik1 key calculate\n");
		return;
	}
	
	memcpy(gpGponPriv->gponSecurity.ploamIk[GPON_PLOAM_IK_IDX1], gpGponPriv->gponSecurity.defaultPloamIk, GPON_PLOAM_IK_LENS);
	gponDevSetPloamIk1(gpGponPriv->gponSecurity.ploamIk[GPON_PLOAM_IK_IDX1]); /*ploam ik1 is set to the default value of (0x55)*16  */		

	if(gponDevKeyGenStart(GPON_KEK_GEN_START) == 0){
		PON_MSG(MSG_SECUR, "Hw generate Kek 1\n");
		gponDevGetHwGenKey((int *)&gpGponPriv->gponSecurity.kek[GPON_OMCI_IK_IDX1]);
		gponDevSetKEK1(gpGponPriv->gponSecurity.kek[GPON_KEK_IK_IDX1]);
	}else{
		PON_MSG(MSG_ERR, "There is something wrong with kek1 calculate\n");
		return;
	}

	/*key group0  based on real registration id*/

	if(gponDevKeyGenStart(GPON_REGMSK_GEN_START) == 0){
		PON_MSG(MSG_SECUR, "Hw generate msk based on registerID\n");
		gponDevGetRegMSK((int *)&gpGponPriv->gponSecurity.msk);
		gponDevSetMSK(gpGponPriv->gponSecurity.msk);
	}else{
		PON_MSG(MSG_ERR, "There is something wrong with msk calculate\n");
		return;
	}	
	
	if(gponDevKeyGenStart(GPON_SK_GEN_START) == 0){
		PON_MSG(MSG_SECUR, "Hw generate sk base on real regId\n");
		gponDevGetSK((uint *)&gpGponPriv->gponSecurity.sk);
	}else{
		PON_MSG(MSG_ERR, "There is something wrong with sk calculate\n");
		return;
	}

	if(gponDevKeyGenStart(GPON_OMCI_IK_GEN_START) == 0){
		PON_MSG(MSG_SECUR, "Hw generate omciIk 0\n");
		gponDevGetHwGenKey((uint *)&gpGponPriv->gponSecurity.omciIk[GPON_OMCI_IK_IDX0]);
		gponDevSetOmciIk0(gpGponPriv->gponSecurity.omciIk[GPON_OMCI_IK_IDX0]);
	}else{
		PON_MSG(MSG_ERR, "There is something wrong with omci ik0 calculate\n");
		return;
	}

	if(gponDevKeyGenStart(GPON_PLOAM_IK_GEN_START) == 0){
		PON_MSG(MSG_SECUR, "Hw generate ploamIk 0\n");
		gponDevGetHwGenKey((int *)&gpGponPriv->gponSecurity.ploamIk[GPON_PLOAM_IK_IDX0]);
		gponDevSetPloamIk0(gpGponPriv->gponSecurity.ploamIk[GPON_PLOAM_IK_IDX0]);
	}else{
		PON_MSG(MSG_ERR, "There is something wrong with ploam ik0 key calculate\n");
		return;
	}

	if(gponDevKeyGenStart(GPON_KEK_GEN_START) == 0){
		PON_MSG(MSG_SECUR, "Hw generate kek 0\n");
		gponDevGetHwGenKey((int *)&gpGponPriv->gponSecurity.kek[GPON_KEK_IK_IDX0]);
		gponDevSetKEK0(gpGponPriv->gponSecurity.kek[GPON_KEK_IK_IDX0]);
	}else{
		PON_MSG(MSG_ERR, "There is something wrong with kek0 calculate\n");
		return;
	}
	return;
}
/******************************************************************************
Descriptor:    set the new key group based on new msk from OMCI
Input Args:    pointer of GPON_Security_t
Ret Value:     0:success;-1:fail
******************************************************************************/
int gponDevKeySetOmciBasedMSK(GPON_Security_t *gponSecurity){

	/*new key group  based on new msk*/

	gponDevSetMSK(gponSecurity->msk);
	
	if(gponDevKeyGenStart(GPON_SK_GEN_START) == GPON_KEY_GEN_SUCCESS){
		PON_MSG(MSG_SECUR, "Hw generate sk base on msk\n");
		gponDevGetSK((uint *)&gponSecurity->sk);
	}else{
		PON_MSG(MSG_ERR, "There is something wrong with sk calculate\n");
		return -1;
	}

	if(gponDevKeyGenStart(GPON_OMCI_IK_GEN_START) == GPON_KEY_GEN_SUCCESS){		
		if(gponSecurity->omciIkIdx == GPON_OMCI_IK_IDX0){
			PON_MSG(MSG_SECUR, "Hw generate omciIk 1\n");
			gponDevGetHwGenKey((uint *)&gponSecurity->omciIk[GPON_OMCI_IK_IDX1]);
			gponDevSetOmciIk1(gponSecurity->omciIk[GPON_OMCI_IK_IDX1]);
		}else{
			PON_MSG(MSG_SECUR, "Hw generate omciIk 0\n");
			gponDevGetHwGenKey((uint *)&gponSecurity->omciIk[GPON_OMCI_IK_IDX0]);
			gponDevSetOmciIk0(gponSecurity->omciIk[GPON_OMCI_IK_IDX0]);
		}
	}else{
		PON_MSG(MSG_ERR, "There is something wrong with set omci ik calculate\n");
		return -1;
	}

	if(gponDevKeyGenStart(GPON_PLOAM_IK_GEN_START) == GPON_KEY_GEN_SUCCESS){		
		if(gponSecurity->ploamIkIdx == GPON_PLOAM_IK_IDX0){
			PON_MSG(MSG_SECUR, "Hw generate ploamIk 1\n");
			gponDevGetHwGenKey((uint *)&gponSecurity->ploamIk[GPON_PLOAM_IK_IDX1]);
			gponDevSetPloamIk1(gponSecurity->ploamIk[GPON_PLOAM_IK_IDX1]);
		}else{
			PON_MSG(MSG_SECUR, "Hw generate ploamIk 0\n");
			gponDevGetHwGenKey((uint *)&gponSecurity->ploamIk[GPON_PLOAM_IK_IDX0]);
			gponDevSetPloamIk0(gponSecurity->ploamIk[GPON_PLOAM_IK_IDX0]);
		}
	}else{
		PON_MSG(MSG_ERR, "There is something wrong with set ploam ik key calculate\n");
		return -1;
	}

	if(gponDevKeyGenStart(GPON_KEK_GEN_START) == GPON_KEY_GEN_SUCCESS){
		if(gponSecurity->ploamIkIdx == GPON_KEK_IK_IDX0){
			PON_MSG(MSG_SECUR, "Hw generate kek 1\n");
			gponDevGetHwGenKey((uint *)&gponSecurity->kek[GPON_KEK_IK_IDX1]);
			gponDevSetKEK1(gponSecurity->kek[GPON_KEK_IK_IDX1]);
		}else{
			PON_MSG(MSG_SECUR, "Hw generate kek 0\n");
			gponDevGetHwGenKey((uint *)&gponSecurity->kek[GPON_KEK_IK_IDX0]);
			gponDevSetKEK0(gponSecurity->kek[GPON_KEK_IK_IDX0]);
		}
	}else{
		PON_MSG(MSG_ERR, "There is something wrong with set kek calculate\n");
		return -1;
	}
	return 0;

}

/*****************************************************************************
Descriptor:    set the cmac engine0 input data length and result data length
Input Args:    datalen: input msg data length; resultLen: result msg data length
Ret Value:     none.
******************************************************************************/
void gponDevSetCmac0Len(uint dataLen, uint resultLen){
	
	REG_SW0_ENCLEN gponCmac ;
	
	gponCmac.Raw = IO_GREG(SW0_ENCLEN);
	gponCmac.Bits.sw0_mdtlen = dataLen ;
	gponCmac.Bits.sw0_rdtlen = resultLen ;
	IO_SREG(SW0_ENCLEN, gponCmac.Raw);
	
}
/*****************************************************************************
Descriptor:    set  msg address to cmac engine0
Input Args:    msg address
Ret Value:     none.
******************************************************************************/
void gponDevSetCmac0MsgAddr(unchar * msgAddr){
	
	REG_SW0_MADDR gponCmacMsgAddr ;
  
	gponCmacMsgAddr.Bits.sw0_mdtaddr = K1_TO_PHYSICAL(msgAddr);
	IO_SREG(SW0_MADDR, gponCmacMsgAddr.Raw);
	
}
/*****************************************************************************
Descriptor:    set result msg address to cmac engine0
Input Args:    result msg address
Ret Value:     none.
******************************************************************************/
void gponDevSetCmac0ResultAddr(unchar * rsltAddr){
	
	REG_SW0_RADDR gponCmacResultAddr ;

	gponCmacResultAddr.Bits.sw0_rdtaddr = K1_TO_PHYSICAL(rsltAddr);
	IO_SREG(SW0_RADDR, gponCmacResultAddr.Raw);
	
}
/*****************************************************************************
Descriptor:    set key address to cmac engine0
Input Args:    key address
Ret Value:     none.
******************************************************************************/
void gponDevSetCmac0KeyAddr(unchar * keyAddr){
	
	REG_SW0_KADDR gponCmacKeyAddr ;

	gponCmacKeyAddr.Bits.sw0_kaddr = K1_TO_PHYSICAL(keyAddr);
	IO_SREG(SW0_KADDR, gponCmacKeyAddr.Raw);
	
}
/*****************************************************************************
Descriptor:    start cmac engine0 to calculate
Input Args:    keyIdx:OMCI_IK,PLOAM_IK,KEK index or normal encrypt
		      dir:
		      msgAddr: the address of the msg which needs to be encrypted
		      resultAddr: the address of the msg which is encrypted

		      In ARM architecture: key,msgAddr,resultAddr is physical address,
		      the software can't be operated directly(eg:memcpy, pointer
			  dereference...), otherwise will cause Call trace.
			  In MIPS architecture: msgAddr, resultAddr is virtual address

		      resultAddr_vir: the address of the msg which is encrypted,
							is virtual address, only used in ARM,in MIPS
							is NULL.
Ret Value:     0: success;-1: fail
******************************************************************************/

int gponDevSetCmac0Start(GPON_CMAC_KEY_INDEX_t keyIdx, GPON_CMAC_PAD_DIR_t dir, unchar *key,
		unchar *msgAddr, uint msgLen, unchar *resultAddr, unchar *resultAddr_vir, uint resultLen)
{
	REG_SW0_ENCSTART gponCmacStart ;
	REG_SW0_ENCINFO  gponCmacInfo ;       
	REG_INT_STATUS intStatus ;
    int RETRY_int = 3000 ;
	int RETRY = 3000 ;
	
	if(keyIdx > GPON_CMAC_NORMAL_ENCRYPT || msgLen == 0 || resultLen == 0){
		return -1;
	}

	/*set encryption key index and direction*/
	gponCmacInfo.Raw = IO_GREG(SW0_ENCINFO);
	if((keyIdx == GPON_CMAC_NORMAL_ENCRYPT) && (key !=NULL)){
		gponDevSetCmac0KeyAddr(key);
	}
    
	gponCmacInfo.Bits.sw0_enckidx = keyIdx;
	gponCmacInfo.Bits.sw0_encdic = dir;
	IO_SREG(SW0_ENCINFO, gponCmacInfo.Raw);
	
	/*set message and result data length and address */
	gponDevSetCmac0Len(msgLen, resultLen);
	gponDevSetCmac0MsgAddr(msgAddr);
	gponDevSetCmac0ResultAddr(resultAddr);

    /*Clear the interrupt status*/
    IO_SREG(INT_STATUS, GPON_SW0_MIC_DONE_INT) ;

	/*start to encrypt*/
	gponCmacStart.Raw = IO_GREG(SW0_ENCSTART);
	gponCmacStart.Bits.sw0_encstart = 1 ;
	IO_SREG(SW0_ENCSTART, gponCmacStart.Raw);

    /* Wait for done */
	while(RETRY_int--){
		intStatus.Raw = IO_GREG(INT_STATUS);
		if(intStatus.Bits.sw0_mic_done_int){
			IO_SREG(INT_STATUS, GPON_SW0_MIC_DONE_INT) ;
            /* Wait for dram done  */
            while(RETRY--){
				#ifdef TCSUPPORT_CPU_ARMV8_64
				if((resultAddr_vir != NULL) && ((*(resultAddr_vir + resultLen-1)) == 1))
				#else
                if((*(resultAddr + resultLen-1)) == 1)
				#endif
				{
                    return 0;
                }
                udelay(1);
            }
            return -1;
		}
		udelay(1);
	}
    return -2 ;
	
}

/*****************************************************************************
Descriptor:    set the cmac engine1 input data length and result data length
Input Args:    datalen: input msg data length; resultLen: result msg data length
Ret Value:     none.
******************************************************************************/
void gponDevSetCmac1Len(uint dataLen, uint resultLen){
	
	REG_SW1_ENCLEN gponCmac ;
	
	gponCmac.Raw = IO_GREG(SW1_ENCLEN);
	gponCmac.Bits.sw1_mdtlen = dataLen ;
	gponCmac.Bits.sw1_rdtlen = resultLen ;
	IO_SREG(SW1_ENCLEN, gponCmac.Raw);
	
}
/*****************************************************************************
Descriptor:    set  msg address to cmac engine1
Input Args:    msg address
Ret Value:     none.
******************************************************************************/
void gponDevSetCmac1MsgAddr(uint msgAddr){
	
	REG_SW1_MADDR gponCmacMsgAddr ;

	gponCmacMsgAddr.Bits.sw1_mdtaddr = K1_TO_PHYSICAL(msgAddr);
	IO_SREG(SW1_MADDR, gponCmacMsgAddr.Raw);
	
}
/*****************************************************************************
Descriptor:    set result msg address to cmac engine1
Input Args:    result msg address
Ret Value:     none.
******************************************************************************/
void gponDevSetCmac1ResultAddr(uint rsltAddr){
	
	REG_SW1_RADDR gponCmacResultAddr ;

	gponCmacResultAddr.Bits.sw1_rdtaddr = K1_TO_PHYSICAL(rsltAddr);
	IO_SREG(SW1_RADDR, gponCmacResultAddr.Raw);
	
}
/*****************************************************************************
Descriptor:    set key address to cmac engine1
Input Args:    key address
Ret Value:     none.
******************************************************************************/
void gponDevSetCmac1KeyAddr(uint keyAddr){
	
	REG_SW1_KADDR gponCmacKeyAddr ;

	gponCmacKeyAddr.Bits.sw1_kaddr = K1_TO_PHYSICAL(keyAddr);
	IO_SREG(SW1_KADDR, gponCmacKeyAddr.Raw);
	
}
/*****************************************************************************
Descriptor:    start cmac engine1 to calculate
Input Args:    keyIdx:OMCI_IK,PLOAM_IK,KEK index or normal encrypt
		      dir:
		      msgAddr: the address of the msg which needs to be encrypted
		      resultAddr: the address of the msg which is encrypted

		      In ARM architecture: key,msgAddr,resultAddr is physical address,
		      the software can't be operated directly(eg:memcpy, pointer
			  dereference...), otherwise will cause Call trace.
			  In MIPS architecture: msgAddr, resultAddr is virtual address

		      resultAddr_vir: the address of the msg which is encrypted,
							is virtual address,only used in ARM,in MIPS
							is NULL.
Ret Value:     0: success;-1: fail
******************************************************************************/

int gponDevSetCmac1Start(GPON_CMAC_KEY_INDEX_t keyIdx, GPON_CMAC_PAD_DIR_t dir, unchar *key,
		unchar *msgAddr, uint msgLen, unchar *resultAddr, unchar *resultAddr_vir, uint resultLen)
{
	REG_SW1_ENCSTART gponCmacStart ;
	REG_SW1_ENCINFO  gponCmacInfo ;
	REG_INT_STATUS   intStatus ;
    int RETRY_int = 3000 ;
	int RETRY = 3000 ;
	
	if(keyIdx > GPON_CMAC_NORMAL_ENCRYPT || msgLen == 0 || resultLen == 0){
		return -1;
	}

	/*set encryption key index and direction*/
	gponCmacInfo.Raw = IO_GREG(SW1_ENCINFO);
	if(keyIdx == GPON_CMAC_NORMAL_ENCRYPT){
		gponDevSetCmac1KeyAddr((regAddr_t)key);
	}
	gponCmacInfo.Bits.sw1_enckidx = keyIdx;
	gponCmacInfo.Bits.sw1_encdic = dir;
	IO_SREG(SW1_ENCINFO, gponCmacInfo.Raw);
	
	/*set message and result data length and address */
	gponDevSetCmac1Len(msgLen, resultLen);
	gponDevSetCmac1MsgAddr((regAddr_t)msgAddr);
	gponDevSetCmac1ResultAddr((regAddr_t)resultAddr);

	
	/*Clear the interrupt status*/
	IO_SREG(INT_STATUS, GPON_SW1_MIC_DONE_INT) ;
    
	/*start to encrypt*/
	gponCmacStart.Raw = IO_GREG(SW1_ENCSTART);
	gponCmacStart.Bits.sw1_encstart = 1 ;
	IO_SREG(SW1_ENCSTART, gponCmacStart.Raw);

      /* Wait for done */
	while(RETRY_int--){
		intStatus.Raw = IO_GREG(INT_STATUS);
		if(intStatus.Bits.sw1_mic_done_int){
			IO_SREG(INT_STATUS, GPON_SW1_MIC_DONE_INT) ;
            /* Wait for dram done  */
            while(RETRY--){
				#ifdef TCSUPPORT_CPU_ARMV8_64
				if((resultAddr_vir != NULL) && ((*(resultAddr_vir + resultLen-1)) == 1))
				#else
                if( (*(resultAddr + resultLen-1)) == 1)
				#endif
				{
                    return 0;
                }
                udelay(1);
            }
            return -1;
		}
		udelay(1);
	}
    return -2 ;    

}

/******************************************************************************
Descriptor:    set the data encryption key valid after key confirm
Input Args:    keyIdx: the first or second of unicast or broadcast aes key
Ret Value:     0: success;-1: fail
******************************************************************************/
int gponDevSetAesRxKeyValid(unchar index){

    REG_DS_AES_KEY_VLD gponAesKeyValid;

    gponAesKeyValid.Raw = IO_GREG(DS_AES_KEY_VLD);
    
    if(index == GPON_AES_UC_FIRST_KEY){
        gponAesKeyValid.Bits.ds_aes_uc_idx0_key_vld = XPON_ENABLE;
    }else if(index == GPON_AES_UC_SECOND_KEY){
        gponAesKeyValid.Bits.ds_aes_uc_idx1_key_vld = XPON_ENABLE;
    }else if(index ==GPON_AES_BC_FIRST_KEY){
        gponAesKeyValid.Bits.ds_aes_bc_idx0_key_vld = XPON_ENABLE;
    }else if(index ==GPON_AES_BC_SECOND_KEY){
        gponAesKeyValid.Bits.ds_aes_bc_idx1_key_vld = XPON_ENABLE;
    }else{
        return -1;
    }
	IO_SREG(DS_AES_KEY_VLD, gponAesKeyValid.Raw);
    
    return 0;
}
/******************************************************************************
Descriptor:    set the data encryption key invalid
Input Args:    keyIdx: the first or second of unicast or broadcast aes key
Ret Value:     0: success;-1: fail
******************************************************************************/
int gponDevSetAesRxKeyInvalid(unchar index){

    REG_DS_AES_KEY_VLD gponAesKeyValid;

    gponAesKeyValid.Raw = IO_GREG(DS_AES_KEY_VLD);
    
    if(index == GPON_AES_UC_FIRST_KEY){
        gponAesKeyValid.Bits.ds_aes_uc_idx0_key_vld = XPON_DISABLE;
    }else if(index == GPON_AES_UC_SECOND_KEY){
        gponAesKeyValid.Bits.ds_aes_uc_idx1_key_vld = XPON_DISABLE;
    }else if(index ==GPON_AES_BC_FIRST_KEY){
        gponAesKeyValid.Bits.ds_aes_bc_idx0_key_vld = XPON_DISABLE;
    }else if(index ==GPON_AES_BC_SECOND_KEY){
        gponAesKeyValid.Bits.ds_aes_bc_idx1_key_vld = XPON_DISABLE;
    }else{
        return -1;
    }
	IO_SREG(DS_AES_KEY_VLD, gponAesKeyValid.Raw);
    gponDevSetUpAesMode();
    return 0;
}

/******************************************************************************
Descriptor:    set all the unicast data key and multicast data key invalid
Input Args:    none
Ret Value:     none
******************************************************************************/
void gponDevResetAesRxKey(void){

    REG_DS_AES_KEY_VLD gponAesKeyValid;
    
    gponAesKeyValid.Raw = IO_GREG(DS_AES_KEY_VLD);
    gponAesKeyValid.Bits.ds_aes_uc_idx0_key_vld = XPON_DISABLE;
    gponAesKeyValid.Bits.ds_aes_uc_idx1_key_vld = XPON_DISABLE;
    gponAesKeyValid.Bits.ds_aes_bc_idx0_key_vld = XPON_DISABLE;
    gponAesKeyValid.Bits.ds_aes_bc_idx1_key_vld = XPON_DISABLE;
    IO_SREG(DS_AES_KEY_VLD, gponAesKeyValid.Raw);
}
/******************************************************************************
Descriptor:    set the data encryption key tx valid after key confirm
Input Args:    keyIdx: the first or second unicast aes key
Ret Value:     none
******************************************************************************/
void gponDevSetAesTxKeyValid(unchar index){

    REG_US_AES_KEY_CTRL gponAesTxKeyCtrl;
    REG_INT_STATUS   intStatus ;
    int RETRY = 3000 ;
    
    /*Clear the interrupt status*/
    IO_SREG(INT_STATUS, XGPON_US_KEY_SWITCH_DONE_INT) ;

    gponAesTxKeyCtrl.Raw = IO_GREG(US_AES_KEY_CTRL);
    
    if(index == GPON_AES_UC_FIRST_KEY){
        gponAesTxKeyCtrl.Bits.us_aes_key_idx = 0;
    }else if(index == GPON_AES_UC_SECOND_KEY){
        gponAesTxKeyCtrl.Bits.us_aes_key_idx = 1;
    }
	IO_SREG(US_AES_KEY_CTRL, gponAesTxKeyCtrl.Raw);
    /*set key index first, then set the key valid*/
    if( UPAES_MODE_NONE != gpGponPriv->gemUpAESMode)
    {
        gponAesTxKeyCtrl.Bits.us_aes_key_vld = XPON_ENABLE;
        IO_SREG(US_AES_KEY_CTRL, gponAesTxKeyCtrl.Raw);
    }else{
        gponAesTxKeyCtrl.Bits.us_aes_key_vld = XPON_DISABLE;
        IO_SREG(US_AES_KEY_CTRL, gponAesTxKeyCtrl.Raw);
    }
    
    /* Wait for done */
	while(RETRY--){
		intStatus.Raw = IO_GREG(INT_STATUS);
		if(intStatus.Bits.us_key_switch_done_int){
			IO_SREG(INT_STATUS, XGPON_US_KEY_SWITCH_DONE_INT) ;
        }
		udelay(1);
	}
}
/******************************************************************************
Descriptor:    set the data encryption key invalid
Input Args:    keyIdx: the first or second of unicast or broadcast aes key
Ret Value:     0: success;-1: fail
******************************************************************************/
void gponDevSetAesTxKeyInvalid(void){

    REG_US_AES_KEY_CTRL gponAesTxKeyCtrl;

    gponAesTxKeyCtrl.Raw = IO_GREG(US_AES_KEY_CTRL);
    gponAesTxKeyCtrl.Bits.us_aes_key_vld = XPON_DISABLE;
    IO_SREG(US_AES_KEY_CTRL, gponAesTxKeyCtrl.Raw);    
}

void gponDevSetPhyDsFecMde(GPON_10G_DEV_DS_FEC_MODE_T Mode){
	int phyFecMode = 0;
	
	if(Mode == GPON_10G_USE_DEFAULT_SET_AS_SEPC){
		phyFecMode = DS_FEC_SETTING_AS_SPEC;
	}else if(Mode == GPON_10G_USE_OCBODY_FIELD){
		phyFecMode = DS_FEC_SETTING_FORCE_OC;
	}else if(Mode == GPON_10G_DS_FEC_FORCE_OFF){
		phyFecMode = DS_FEC_SETTING_FORCE_OFF;
	}else if(Mode == GPON_10G_DS_FEC_FORCE_ON){
		phyFecMode = DS_FEC_SETTING_FORCE_ON;
	}	
	
	XPON_PHY_SET_RX_FEC(phyFecMode);
}

/******************************************************************************
Descriptor:    set the XGSPON DS fec force on
Input Args:    None
Ret Value:     None
******************************************************************************/
void gponDevSetDsFecMode(GPON_10G_DEV_DS_FEC_MODE_T Mode){
	REG_DBG_CAP_SETTING1 onuCapabiltySetting1;
	onuCapabiltySetting1.Raw = IO_GREG(DBG_CAP_SETTING1);
	onuCapabiltySetting1.Bits.ds_fec_mode = Mode;
	IO_SREG(DBG_CAP_SETTING1, onuCapabiltySetting1.Raw);   
}
/******************************************************************************
******************************************************************************/
void gponDevUnicastKeyExchange(unsigned long data){

    unchar dataKey[GPON_DATA_ENCRYPT_KEY_LENS]={0};
    unchar dataKeyIndex =0;
    unchar dataKeyContrl =0;
    unchar keyName[16] ={0x33,0x31,0x34,0x31,0x35,0x39,0x32,0x36,0x35,0x33,0x35,0x38,0x39,0x37,0x39,0x33};
    unchar *hashData = NULL;
    unchar *hashReslt = NULL;	   
    unchar kekIndex =0;
    unchar cmacKeyIndex =0;
   
    uint ret = 0;
    Key_Report_Config_t * keyConfig = (Key_Report_Config_t *)data ;    
     
#ifdef TCSUPPORT_CPU_ARMV8_64
	dma_addr_t dram_hashData_phy;
	dma_addr_t dram_hashReslt_phy;
	int count = 0;

	struct device *xpon_dev = NULL;

	if((xpon_dev = get_xpon_dev()) == NULL)
	{
		printk("\nget xpon dev fail\n");
		return;
	}
#endif

    dataKeyIndex = keyConfig->keyIndex;
    dataKeyContrl = keyConfig->keyControl;
        
    if(((dataKeyContrl != GPON_UC_KEY_GEN) && (dataKeyContrl != GPON_UC_KEY_CONFIRM)) ||
        ((dataKeyIndex != GPON_AES_UC_FIRST_KEY) && (dataKeyIndex != GPON_AES_UC_SECOND_KEY))){
            
        PON_MSG((MSG_ERR|MSG_SECUR), "PLOAM: Key report ploam deal with wrong key type, control:%d index:%d\n",dataKeyContrl,dataKeyIndex) ;
            return ;
        }

    if(dataKeyContrl== GPON_UC_KEY_GEN){          
        PON_MSG((MSG_OAM|MSG_SECUR), "\n");
        PON_MSG((MSG_OAM|MSG_SECUR), "PLOAM: receive generate new unicast data key index:%d \n",dataKeyIndex);
        if((gpGponPriv->gponSecurity.state == KEY_STATE_KN0)||(gpGponPriv->gponSecurity.state == KEY_STATE_KN1) ||
            (gpGponPriv->gponSecurity.state == KEY_STATE_KN3)||(gpGponPriv->gponSecurity.state == KEY_STATE_KN4))
        {
            GPON_START_TIMER(gpGponPriv->gponSecurity.TK4_timer,GPON_SECURITY_TK4_TIMER) ;  /*start TK4*/            
            PON_MSG((MSG_OAM|MSG_SECUR), "DATAKEY: Start timer TK4 \n");
            gpon_encryption_key_change_state(KEY_STATE_KN1);  /*change to KN1*/
            /* Generate a random number for unicast encryption key */
            get_random_bytes(dataKey, GPON_DATA_ENCRYPT_KEY_LENS);
            memcpy(gpGponPriv->gponSecurity.aesUcKey[dataKeyIndex -1],dataKey,GPON_DATA_ENCRYPT_KEY_LENS);
            if(dataKeyIndex == GPON_AES_UC_FIRST_KEY)
                gponDevSetAesUcKey0(gpGponPriv->gponSecurity.aesUcKey[dataKeyIndex -1]);
            else 
                gponDevSetAesUcKey1(gpGponPriv->gponSecurity.aesUcKey[dataKeyIndex -1]);
            /* use the aes ecb arithmetic from kernel to  encrypt the datakey by kek*/
            kekIndex = gpGponPriv->gponSecurity.kekIdx;
            ret = gpon_aes_ecb_encrypt(gpGponPriv->gponSecurity.aesEcbTfm,gpGponPriv->gponSecurity.kek[kekIndex],
                dataKey,GPON_DATA_ENCRYPT_KEY_LENS,keyConfig->dataEncryptedKey);
			if(ret != 0){
    			PON_MSG((MSG_ERR|MSG_SECUR), "PLOAM: gen new key by AES_ECB encrypt %d failed, %d\n",dataKeyIndex,ret) ;
				return ;
			}
            gpon_encryption_key_change_state(KEY_STATE_KN2);  /*change to KN2*/            
        }
        if(gpGponPriv->gponSecurity.state == KEY_STATE_KN2){
            /*send new key report ploam*/
            ploam_send_key_report_msg(keyConfig->seqNo, GPON_UC_KEY_GEN, dataKeyIndex,
                PLOAM_KEY_FRAGMENT_NUM, keyConfig->dataEncryptedKey, PLOAM_NEW_ENCRYPTED_KEY_LEN);
            /*set the unicast data aes rx key valid*/
            gponDevSetAesRxKeyValid(dataKeyIndex);
            GPON_START_TIMER(gpGponPriv->gponSecurity.TK5_timer,GPON_SECURITY_TK5_TIMER);  /*start TK5*/             
            PON_MSG((MSG_OAM|MSG_SECUR), "DATAKEY: Start timer TK5 \n");
        }
    }
    else if(dataKeyContrl == GPON_UC_KEY_CONFIRM){
        PON_MSG((MSG_OAM|MSG_SECUR), "PLOAM: receive comfirm new unicast data key index:%d \n",dataKeyIndex);
        if((gpGponPriv->gponSecurity.state == KEY_STATE_KN2)||(gpGponPriv->gponSecurity.state == KEY_STATE_KN4)){
            GPON_STOP_TIMER(gpGponPriv->gponSecurity.TK5_timer);   /*stop TK5*/            
            PON_MSG((MSG_OAM|MSG_SECUR), "DATAKEY: Stop timer TK5 \n");
            gpon_encryption_key_change_state(KEY_STATE_KN3);  /*change to KN3*/
            if(1 == gpGponPriv->gponSecurity.txKeyValid){
                /*set the unicast data aes tx new key valid,  tx old key invalid*/
                gponDevSetAesTxKeyValid(dataKeyIndex);
            }
            /*set the unicast data aes  rx old key invalid */
            if(dataKeyIndex == GPON_AES_UC_FIRST_KEY){
                gponDevSetAesRxKeyInvalid(GPON_AES_UC_SECOND_KEY);        
                gponDevClearAesUcKey1();
            }else{
                gponDevSetAesRxKeyInvalid(GPON_AES_UC_FIRST_KEY);
                gponDevClearAesUcKey0();
            }
            
#ifdef TCSUPPORT_CPU_ARMV8_64
			for (count = 0; (hashData==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
			{
				hashData = (unchar *)dma_alloc_coherent(xpon_dev, PLOAM_KEY_FRAGMENT_LEN, &dram_hashData_phy, GFP_ATOMIC);
			}

			if(hashData == NULL)
			{
                PON_MSG((MSG_ERR|MSG_SECUR),"PLOAM: ARM dma_alloc_coherent fail for hashData key confrim with cmac0 encrypt \n");
				return ;
			}

			for (count = 0; (hashReslt==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
			{
				hashReslt = (unchar *)dma_alloc_coherent(xpon_dev, PLOAM_EXISTING_HASH_RESULT_LEN, &dram_hashReslt_phy, GFP_ATOMIC);
			}

			if(hashReslt==NULL)
			{
                PON_MSG((MSG_ERR|MSG_SECUR),"PLOAM: ARM dma_alloc_coherent fail for hashReslt key confrim with cmac0 encrypt \n");
                dma_free_coherent(xpon_dev, PLOAM_KEY_FRAGMENT_LEN, hashData, dram_hashData_phy);
                return ;
		    }
#else
            hashData = CACHE_TO_NONCACHE((unchar *)kmalloc(PLOAM_KEY_FRAGMENT_LEN, GFP_KERNEL));
            if(hashData==NULL){
                PON_MSG((MSG_ERR|MSG_SECUR),"PLOAM: kmalloc fail for key confrim with cmac0 encrypt \n");
				return ;
			}
            hashReslt = CACHE_TO_NONCACHE((unchar *)kmalloc(PLOAM_EXISTING_HASH_RESULT_LEN, GFP_KERNEL));
            if(hashReslt==NULL){
                PON_MSG((MSG_ERR|MSG_SECUR),"PLOAM: kmalloc fail for key confrim with cmac0 encrypt \n");
                kfree(NONCACHE_TO_CACHE(hashData));
                return ;
		    }
        
            dma_cache_inv((long unsigned int) NONCACHE_TO_CACHE(hashData), PLOAM_KEY_FRAGMENT_LEN);            
            dma_cache_inv((long unsigned int) NONCACHE_TO_CACHE(hashReslt), PLOAM_EXISTING_HASH_RESULT_LEN);
#endif

            /*AES_CMAC hash the first existing key*/
            memcpy(hashData,gpGponPriv->gponSecurity.aesUcKey[dataKeyIndex -1], GPON_DATA_ENCRYPT_KEY_LENS);
            memcpy(hashData+16,keyName, GPON_DATA_ENCRYPT_KEY_LENS);
            memset(CACHE_TO_NONCACHE(hashReslt), 0x0, PLOAM_EXISTING_HASH_RESULT_LEN);
            if(gpGponPriv->gponSecurity.kekIdx == GPON_KEK_IK_IDX0){
                cmacKeyIndex = GPON_CMAC_KEK_IDX0;                
            }else{
                cmacKeyIndex = GPON_CMAC_KEK_IDX1;                 
            }

#ifdef TCSUPPORT_CPU_ARMV8_64
            ret = gponDevSetCmac0Start(cmacKeyIndex,GPON_CMAC_NORMAL,NULL,
	  				(unchar *)(dram_hashData_phy),PLOAM_KEY_FRAGMENT_LEN,(unchar *)(dram_hashReslt_phy),hashReslt,PLOAM_EXISTING_HASH_RESULT_LEN);			
#else
			ret = gponDevSetCmac0Start(cmacKeyIndex,GPON_CMAC_NORMAL,NULL,
    			    hashData,PLOAM_KEY_FRAGMENT_LEN,hashReslt,NULL,PLOAM_EXISTING_HASH_RESULT_LEN);
#endif
    		if(ret != 0){
    			PON_MSG((MSG_OAM|MSG_SECUR), "PLOAM: Key Confirm by AES_CMAC encrypt %d failed, %d\n",dataKeyIndex,ret) ;
    		}          
            ploam_send_key_report_msg(keyConfig->seqNo, GPON_UC_KEY_CONFIRM, dataKeyIndex, 
                PLOAM_KEY_FRAGMENT_NUM, CACHE_TO_NONCACHE(hashReslt), PLOAM_EXISTING_HASH_KEY_LEN);

            GPON_STOP_TIMER(gpGponPriv->gponSecurity.TK4_timer); /*stop TK4*/
            
            PON_MSG((MSG_OAM|MSG_SECUR), "DATAKEY: Stop timer TK4 \n");
            gpon_encryption_key_change_state(KEY_STATE_KN4);  /*change to KN4*/

#ifdef TCSUPPORT_CPU_ARMV8_64
			 dma_free_coherent(xpon_dev, PLOAM_KEY_FRAGMENT_LEN, hashData, dram_hashData_phy);
			 dma_free_coherent(xpon_dev, PLOAM_EXISTING_HASH_RESULT_LEN, hashReslt, dram_hashReslt_phy);
#else
            kfree(NONCACHE_TO_CACHE(hashData));
            kfree(NONCACHE_TO_CACHE(hashReslt));
#endif
        }
   
    }
}
/******************************************************************************
Descriptor:    get the tx phy ready status
Input Args:    none
Ret Value:     GPON_TCONT_CMD_SUCCESS: success;GPON_TCONT_CMD_FAIL: fail
******************************************************************************/
int gponDevGetTxSyncRdy(void)
{
	int RETRY = 3000 ;
	REG_DBG_RESYNC gponResync;
	
	/* Wait for done */
	while(RETRY--) {
		gponResync.Raw = IO_GREG(DBG_RESYNC) ;
		if(gponResync.Bits.tx_sync_rdy) {
			return GPON_TCONT_CMD_SUCCESS;
		} 
		udelay(1) ;
	}
	PON_MSG(MSG_OAM, "PLOAM: gponDevGetTxSyncRdy fail \n") ;
	return GPON_TCONT_CMD_FAIL;
}
/******************************************************************************
******************************************************************************/
void gponDevRangingAck(unsigned long data)
{
    unchar * seqNo =(unchar *)data ;

	if(gponDevGetTxSyncRdy() == GPON_TCONT_CMD_SUCCESS){
		ploam_send_acknowledge_msg(*(seqNo),XGPON_PLOAM_ACK_OK); /*send Ack in unicast */
		return;
    }
	return;
}

int gponDevRangingAck_test(unchar seqNo)
{

	if(gponDevGetTxSyncRdy() == GPON_TCONT_CMD_SUCCESS){
		ploam_send_acknowledge_msg(seqNo,XGPON_PLOAM_ACK_OK); /*send Ack in unicast */
		return 0;
    }
	return -1;
}


/*****************************************************************************
Descriptor:    reset pon mac
Input Args:    none
Ret Value:     none
******************************************************************************/
void gponDevMacReset(XPON_RESET_MODE_t mode){

	REG_SW_RST  gponRest;
    if((mode != XPON_RESET_HOLD_ON) && (mode != XPON_RESET_RELEASE)) {
        return;
    }

	gponRest.Raw = IO_GREG(SW_RST);
	gponRest.Bits.xgpon_mac_sw_rst_n = (mode == XPON_RESET_HOLD_ON) ? 0 : 1;	
	IO_SREG(SW_RST, gponRest.Raw);
}
	
/*****************************************************************************
Descriptor:    reset pon mac
Input Args:    none
Ret Value:     none
******************************************************************************/
void gponDevResetCtrl(XPON_RESET_MODE_t mode){
	
    ulong flags = 0;

	if((mode != XPON_RESET_HOLD_ON) && (mode != XPON_RESET_RELEASE)) {
		return;
	}
    spin_lock_irqsave(&gpGponPriv->mac_reset_lock, flags) ;
	
    if(mode == XPON_RESET_HOLD_ON){
        PON_MSG(MSG_INT, "MAC RESET: reset hold on\n") ;
        gponDevMbiStop(XPON_RESET_HOLD_ON, XPON_WITH_GDM2CDM2_STOP) ;
        gponDevMpiStop(XPON_RESET_HOLD_ON) ;        
        gponDevMacReset(XPON_RESET_HOLD_ON);  
        FE_API_SET_CHANNEL_RETIRE_ALL(FE_GDM_SEL_GDMA2, CONFIG_GPON_10G_MAX_TCONT);
    }else{
        PON_MSG(MSG_INT, "MAC RESET: reset release\n") ;
        gponDevMacReset(XPON_RESET_RELEASE);     
        gponDevMbiStop(XPON_RESET_RELEASE, XPON_WITH_GDM2CDM2_STOP) ;
        gponDevMpiStop(XPON_RESET_RELEASE) ;
    }
    spin_unlock_irqrestore(&gpGponPriv->mac_reset_lock, flags) ;	
}

/*****************************************************************************
Descriptor:    Set o2_3 and o4 PLOAM control status
Input Args:    sel: o2_3 and o4 PLOAM control status.
Ret Value:     none
******************************************************************************/
void gponDevSetO23O4PloamCtrl(XGPON_SW_HW_SELECT_T mode){
	
	REG_O23_O4_PLOAMU_CTRL  gponSwReply;
	
	gponSwReply.Raw = IO_GREG(O23_O4_PLOAMU_CTRL);
	gponSwReply.Bits.o23_o4_ploamu_ctrl = (mode == XGPON_HW) ? 0 : 1;
	IO_SREG(O23_O4_PLOAMU_CTRL, gponSwReply.Raw);

}
/*****************************************************************************
Descriptor:    Get o2_3 and o4 PLOAM control status
Input Args:    sel: o2_3 and o4 PLOAM control status.
Ret Value:	none
******************************************************************************/
void gponDevGetO23O4PloamCtrl(XGPON_SW_HW_SELECT_T *mode){

	REG_O23_O4_PLOAMU_CTRL  gponSwReply;
	
	gponSwReply.Raw = IO_GREG(O23_O4_PLOAMU_CTRL);
	*mode = (gponSwReply.Bits.o23_o4_ploamu_ctrl  == 0)? XGPON_HW : XGPON_SW;
}

/*****************************************************************************
Descriptor:    set eqd value
Input Args:    sel: eqd.
Ret Value:	none
******************************************************************************/
void gponDevSetEqd(uint eqd)
{
	REG_EQD gponEqd ;
	
	/* Setting the EqD to MAC register */
    gponEqd.Raw = IO_GREG(EQD) ;	
	gponEqd.Bits.eqd = eqd;
	IO_SREG(EQD, gponEqd.Raw) ;	
}

#if !defined(TCSUPPORT_CPU_AN7583)
/*****************************************************************************
Descriptor:    refresh eqd reg value and eqd_olt_init
Input Args:    sel: eqd.
Ret Value:	none
******************************************************************************/
void gponDevRefreshEqdValue(void)
{	
	REG_EQD gponEqd ;	

	gponEqd.Raw = IO_GREG(EQD);	
	if((gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGSPON) || (gpPonSysData->sysPonMode ==  XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G)) {		
		gponEqd.Bits.eqd = gpGponPriv->gponCfg.eqd_olt_absolute << 2;	
	} else {		
		gponEqd.Bits.eqd = gpGponPriv->gponCfg.eqd_olt_absolute;	
	}	
	gpGponPriv->gponCfg.eqd_olt_init = gpGponPriv->gponCfg.eqd_olt_absolute;	
	IO_SREG(EQD, gponEqd.Raw) ;
}
/*****************************************************************************
Descriptor:    set eqd value
Input Args:    sel: eqd.
Ret Value:	none
******************************************************************************/
int gponDevSetEqdValue(uint newEqd_eqd_olt_absolute,uint	eqd_olt_init ,unchar wanMode)
{
	uint newEqd = 0; // the new value for eqd reg
	uint EqdDiff = 0;
	REG_EQD gponEqd ;

	/* compute the eqd shift */
	if(eqd_olt_init > newEqd_eqd_olt_absolute)
		EqdDiff = eqd_olt_init - newEqd_eqd_olt_absolute;
	else
		EqdDiff = newEqd_eqd_olt_absolute - eqd_olt_init;
	
 /* determine whether eqd needs to be multiplied by 4 for comparison*/
	if(wanMode == XMCS_IF_WAN_DETECT_MODE_XGPON){
		EqdDiff = EqdDiff;
	} else if((wanMode == XMCS_IF_WAN_DETECT_MODE_XGSPON) ||(wanMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G) ){
		EqdDiff = (EqdDiff << 2);
	} else {
		PON_MSG(MSG_ERR, "error wan mode when set eqd,set error\n" );
		return -1;
	}
	/* Setting the EqD to MAC register */
	//printk ("eqd_write_counter = %d\n",gpGponPriv->gponCfg.eqd_write_counter);
	if(EqdDiff <= 255){
		gponEqd.Raw = IO_GREG(EQD);
		if(newEqd_eqd_olt_absolute > eqd_olt_init){
			newEqd = gponEqd.Raw + EqdDiff;
		} else if(newEqd_eqd_olt_absolute == eqd_olt_init){
			newEqd = gponEqd.Raw - 1;
		} else {
			newEqd = gponEqd.Raw - EqdDiff;
		}
		gponDevSetEqd(newEqd);
		return 0;
	} else {
	//resync Tx
		gponDevMpiTxStop(XPON_RESET_HOLD_ON);
		gponDevSwResyncCompleteProcess();
		gponDevMpiTxStop(XPON_RESET_RELEASE);
		PON_MSG((MSG_OAM|MSG_EQD), "refresh the eqd_olt_init in O5, eqd_olt_init:0x%.8X \n",eqd_olt_init);
		PON_MSG((MSG_ACT|MSG_EQD), "PLOAM: cur state %d, receive EqdDiff %d, eqd:0x%.8x, Resync Tx\n",GPON_CURR_STATE,EqdDiff,IO_GREG(EQD)) ;	
		return 0;
	}
}
#else
/*****************************************************************************
Descriptor:    set eqd value
Input Args:    sel: eqd.
Ret Value:	none
******************************************************************************/
int gponDevSetEqdValue(uint newEqd,uint * recordEqd,unchar wanMode,unchar adjustMode,unchar adjustDir)
{
	REG_EQD gponEqd ;
	
    gponEqd.Raw = IO_GREG(EQD) ;
	if(adjustMode == PLOAM_EQD_ABSOLUTE){
		if(wanMode == XMCS_IF_WAN_DETECT_MODE_XGPON){
			gponEqd.Bits.eqd = newEqd;
		}
		else if(wanMode == XMCS_IF_WAN_DETECT_MODE_XGSPON || wanMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G){
			gponEqd.Bits.eqd = (newEqd << 2);
		}else{
			PON_MSG(MSG_ERR, "error wan mode when set eqd\n" );
			return -1;
		}
	}else if(adjustMode == PLOAM_EQD_RELATIVE){
		if(wanMode == XMCS_IF_WAN_DETECT_MODE_XGSPON  || wanMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G){
			newEqd = (newEqd << 2);
		}
		if(adjustDir == PLOAM_EQD_POSITIVE){
			gponEqd.Bits.eqd += newEqd;
		}else if(adjustDir == PLOAM_EQD_NEGATIVE){
			gponEqd.Bits.eqd -= newEqd;
		}

	}else{
		PON_MSG(MSG_ERR, "error wan mode when set eqd\n" );
		return -1;
	}
	
	*recordEqd =gponEqd.Raw ;
	IO_SREG(EQD, gponEqd.Raw) ;	
	return 0;
}
#endif

/******************************************************************************
 Descriptor:	enable/disable sniffer mode. if enable, config sniffer to lan port.
 Input Args:	sniffer: sniffer mode config.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetSniffMode(GPON_10G_DEV_SNIFFER_MODE_T *sniffer)
{
	REG_SNF_CTRL gponSniffCtrl;
	REG_SNF_GPID gponSniffGpid;
	REG_DS_SNF_ETH_DASA_H16 gponDsSniffEthDasa;
    REG_US_SNF_ETH_DASA_H16 gponUsSniffEthDasa;
	REG_SNF_ETH_TAG gponSniffTag;
	REG_SNF_ETH_TYPE gponSniffType;

	if(sniffer->ds_enable != XPON_DISABLE && sniffer->ds_enable != XPON_ENABLE) {
		return -1;
		
	}
	if(sniffer->us_enable != XPON_DISABLE && sniffer->us_enable != XPON_ENABLE) {
		return -1;
	}

	/*fix 8b idle xgem problem*/
#if defined(TCSUPPORT_CPU_AN7583)
	gponSniffCtrl.Raw = IO_GREG(SNF_CTRL) ;
	if(sniffer->us_enable == XPON_ENABLE){
		gponSniffCtrl.Bits.snf_us_ovrw_8b_xgem = XPON_ENABLE;
	} else {
		gponSniffCtrl.Bits.snf_us_ovrw_8b_xgem = XPON_DISABLE;
	}
	IO_SREG(SNF_CTRL, gponSniffCtrl.Raw) ;
#endif

	gponSniffCtrl.Raw = IO_GREG(SNF_CTRL) ;
	gponSniffCtrl.Bits.snf_ds_en = sniffer->ds_enable;
	gponSniffCtrl.Bits.snf_us_en = sniffer->us_enable;
	
	gponSniffGpid.Raw = IO_GREG(SNF_GPID) ;
	gponSniffGpid.Bits.us_snf_gpid = sniffer->gemPortId;
	gponSniffGpid.Bits.ds_snf_gpid = sniffer->gemPortId;
	IO_SREG(SNF_GPID, gponSniffGpid.Raw) ;
	
	if(sniffer->ds_enable == XPON_ENABLE || (sniffer->us_enable == XPON_ENABLE)) {
		if(sniffer->lan_port != GPON_10G_SNIFFER_MODE_LAN0 && sniffer->lan_port != GPON_10G_SNIFFER_MODE_LAN1 &&
		   sniffer->lan_port != GPON_10G_SNIFFER_MODE_LAN2 && sniffer->lan_port != GPON_10G_SNIFFER_MODE_LAN3 &&
		   sniffer->lan_port !=0xf) {
		   return -1;
		}

		/* Set rx da/sa */
		gponDsSniffEthDasa.Bits.ds_snf_eth_da_h16  = sniffer->rx_da;
		gponDsSniffEthDasa.Bits.ds_snf_eth_sa_h16  = sniffer->rx_sa;
		IO_SREG(DS_SNF_ETH_DASA_H16, gponDsSniffEthDasa.Raw);

        /* Set tx da/sa */
		gponUsSniffEthDasa.Bits.us_snf_eth_da_h16  = sniffer->tx_da;
		gponUsSniffEthDasa.Bits.us_snf_eth_sa_h16  = sniffer->tx_sa;
		IO_SREG(US_SNF_ETH_DASA_H16, gponUsSniffEthDasa.Raw);
        
		/* Set eth type */
		gponSniffType.Bits.snf_eth_type =	sniffer->ethertype ;
		IO_SREG(SNF_ETH_TYPE, gponSniffType.Raw);
		
		/* Set TPID(special tag) */
		gponSniffTag.Bits.snf_eth_tpid = sniffer->tpid;
		IO_SREG(SNF_ETH_TAG, gponSniffTag.Raw);
		
	}

	/* set enable */
	IO_SREG(SNF_CTRL, gponSniffCtrl.Raw) ;

	return 0;
}

/******************************************************************************
 Descriptor:	get sniffer mode config.
 Input Args:	sniffer: used to store sniffer mode config.
 Ret Value:	none
******************************************************************************/
void gponDevGetSniffMode(GPON_10G_DEV_SNIFFER_MODE_T *sniffer)
{
	REG_SNF_CTRL gponSniffCtrl;
	REG_SNF_GPID gponSniffGpid;
	REG_DS_SNF_ETH_DASA_H16 gponDsSniffEthDasa;
    REG_US_SNF_ETH_DASA_H16 gponUsSniffEthDasa;
	REG_SNF_ETH_TAG gponSniffTag;
	REG_SNF_ETH_TYPE gponSniffType;	

	gponSniffCtrl.Raw = IO_GREG(SNF_CTRL) ;
	sniffer->ds_enable = gponSniffCtrl.Bits.snf_ds_en;
	sniffer->us_enable = gponSniffCtrl.Bits.snf_us_en;

    /*get gemport id used by sniffer */
    gponSniffGpid.Raw = IO_GREG(SNF_GPID) ;
	sniffer->gemPortId = gponSniffGpid.Bits.us_snf_gpid;

    /* Get rx */
    gponDsSniffEthDasa.Raw = IO_GREG(DS_SNF_ETH_DASA_H16) ;
    sniffer->rx_da =        gponDsSniffEthDasa.Bits.ds_snf_eth_da_h16;
    sniffer->rx_sa =        gponDsSniffEthDasa.Bits.ds_snf_eth_sa_h16;
    
	/* Get tx */
	gponUsSniffEthDasa.Raw = IO_GREG(US_SNF_ETH_DASA_H16) ;
	sniffer->tx_da =		 gponUsSniffEthDasa.Bits.us_snf_eth_da_h16;
	sniffer->tx_sa =		 gponUsSniffEthDasa.Bits.us_snf_eth_sa_h16;
    
	gponSniffType.Raw = IO_GREG(SNF_ETH_TYPE) ;
	sniffer->ethertype = gponSniffType.Bits.snf_eth_type;	
	
	/* Get TPID(special tag) */
	gponSniffTag.Raw =	IO_GREG(SNF_ETH_TAG) ;
	sniffer->tpid =		gponSniffTag.Bits.snf_eth_tpid;
	sniffer->lan_port =	sniffer->tpid;

}
/******************************************************************************
 Descriptor:	get normal counter.
 Input Args:	counter: used to store normal counter value.
 Ret Value:	none
******************************************************************************/
void gponDevGetNormalCounter(GPON_DEV_NORMAL_COUNTER_T *counter)
{
	REG_RX_XGTC_CNT    rx_mpi_sof_cnt;
	REG_TX_BST_CNT     tx_mpi_sof_cnt;
	REG_RX_PLOAMD_CNT  rx_ploamd_cnt;
    REG_TX_PLOAMU_CNT  tx_ploamu_cnt;
    REG_RX_OMCI_CNT    rx_omci_cnt;
    REG_TX_OMCI_CNT    tx_omci_cnt;
	REG_RX_XGEM_CNT    rx_xgem_cnt;
    REG_TX_XGEM_CNT    tx_xgem_cnt;
	REG_RX_MBI_CNT     rx_mbi_ack_cnt;
    REG_TX_MBI_CNT     tx_mbi_ack_cnt;
    REG_RX_NON_IDLE_BCNT   rx_non_idle_bcnt;
    REG_TX_NON_IDLE_BCNT   tx_non_idle_bcnt;
	REG_TX_NLF_XGEM_CNT    tx_nlf_xgem_cnt;
	REG_TX_ACK_PLOAMU_CNT  tx_ack_ploamu_cnt;
	REG_RX_BIP_PROTECT_WCNT rx_bip_protect_wcnt;
	REG_TX_IDLE_BCNT        tx_idle_bcnt;

	rx_mpi_sof_cnt.Raw =	  IO_GREG(RX_XGTC_CNT) ;
	counter->rx_mpi_sof_cnt = rx_mpi_sof_cnt.Bits.rx_mpi_sof_cnt;
	tx_mpi_sof_cnt.Raw =	  IO_GREG(TX_BST_CNT) ;
	counter->tx_mpi_sof_cnt = tx_mpi_sof_cnt.Bits.tx_mpi_sof_cnt;

	rx_ploamd_cnt.Raw =		 IO_GREG(RX_PLOAMD_CNT) ;
	counter->rx_ploamd_cnt = rx_ploamd_cnt.Bits.rx_ploamd_cnt;
	tx_ploamu_cnt.Raw =		 IO_GREG(TX_PLOAMU_CNT) ;
	counter->tx_ploamu_cnt = tx_ploamu_cnt.Bits.tx_ploamu_cnt;

	rx_omci_cnt.Raw =		  IO_GREG(RX_OMCI_CNT) ;
	counter->rx_omci_cnt_fe = rx_omci_cnt.Bits.rx_omci_cnt_fe;
	counter->rx_omci_cnt_mac = rx_omci_cnt.Bits.rx_omci_cnt_mac;	
	tx_omci_cnt.Raw =		  IO_GREG(TX_OMCI_CNT) ;
	counter->tx_omci_cnt_fe = tx_omci_cnt.Bits.tx_omci_cnt_fe;
	counter->tx_omci_cnt_mac = tx_omci_cnt.Bits.tx_omci_cnt_mac;
	
	rx_xgem_cnt.Raw =		IO_GREG(RX_XGEM_CNT) ;
	counter->rx_xgem_cnt = rx_xgem_cnt.Bits.rx_xgem_cnt;
	tx_xgem_cnt.Raw =		IO_GREG(TX_XGEM_CNT) ;
	counter->tx_xgem_cnt = tx_xgem_cnt.Bits.tx_xgem_cnt;

	rx_mbi_ack_cnt.Raw =	  IO_GREG(RX_MBI_CNT) ;
	counter->rx_mbi_ack_cnt = rx_mbi_ack_cnt.Bits.rx_mbi_ack_cnt;
	tx_mbi_ack_cnt.Raw =	  IO_GREG(TX_MBI_CNT) ;
	counter->tx_mbi_ack_cnt = tx_mbi_ack_cnt.Bits.tx_mbi_ack_cnt;

	rx_non_idle_bcnt.Raw =		IO_GREG(RX_NON_IDLE_BCNT) ;
	counter->rx_non_idle_bcnt = rx_non_idle_bcnt.Bits.rx_non_idle_bcnt;
	tx_non_idle_bcnt.Raw =		IO_GREG(TX_NON_IDLE_BCNT) ;
	counter->tx_non_idle_bcnt = tx_non_idle_bcnt.Bits.tx_non_idle_bcnt;

	tx_nlf_xgem_cnt.Raw =		IO_GREG(TX_NLF_XGEM_CNT) ;
	counter->tx_nlf_xgem_cnt = tx_nlf_xgem_cnt.Bits.tx_nlf_xgem_cnt;
	tx_ack_ploamu_cnt.Raw =		IO_GREG(TX_ACK_PLOAMU_CNT) ;
	counter->tx_ack_ploamu_cnt = tx_ack_ploamu_cnt.Bits.tx_ack_ploamu_cnt;	
	rx_bip_protect_wcnt.Raw =	IO_GREG(RX_BIP_PROTECT_WCNT) ;
	counter->rx_bip_protect_wcnt = rx_bip_protect_wcnt.Bits.rx_bip_protect_wcnt;
	tx_idle_bcnt.Raw =		IO_GREG(TX_IDLE_BCNT) ;
	counter->tx_idle_bcnt = tx_idle_bcnt.Bits.tx_idle_bcnt;
	

}

/******************************************************************************
 Descriptor:	get normal counter.
 Input Args:	counter: used to store normal counter value.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
void gponDevGetErrCounter(GPON_DEV_NORMAL_COUNTER_T *counter)
{

    REG_RX_HLEND_HEC_CNT       rx_hlend_hec_cnt;
    REG_RX_ALLOC_HEC_CNT       rx_alloc_hec_cnt;
    REG_RX_HDR_HEC_CNT         rx_hdr_hec_cnt;    
    REG_RX_PHY_HEC_ERR_CNT     rx_phy_hec_cnt;
    REG_RX_MIC_ERR_CNT         rx_mic_err_cnt;
    REG_RX_ETH_FCS_ERR_CNT     rx_eth_fcs_cnt;
    REG_RX_BIP_ERR_CNT         rx_bip_cnt;
    REG_RX_KEY_ERR_CNT         rx_key_err_cnt;
    REG_RX_LOST_WCNT           rx_los_wcnt;
    REG_INVLD_PROF_BST_GNT_CNT invld_prof_bst_gnt_cnt;
	REG_RX_MBI_XGEM_DROP_CNT   rx_mbi_xgem_drop_cnt;

	rx_hlend_hec_cnt.Raw =		IO_GREG(RX_HLEND_HEC_CNT) ;
	counter->rx_hlend_hec_1err_cnt = rx_hlend_hec_cnt.Bits.rx_hlend_hec_1err_cnt;
	counter->rx_hlend_hec_2err_cnt = rx_hlend_hec_cnt.Bits.rx_hlend_hec_2err_cnt;
	counter->rx_hlend_hec_3err_cnt = rx_hlend_hec_cnt.Bits.rx_hlend_hec_3err_cnt;

    rx_alloc_hec_cnt.Raw =		IO_GREG(RX_ALLOC_HEC_CNT) ;
	counter->rx_alloc_hec_1err_cnt = rx_alloc_hec_cnt.Bits.rx_alloc_hec_1err_cnt;
	counter->rx_alloc_hec_2err_cnt = rx_alloc_hec_cnt.Bits.rx_alloc_hec_3err_cnt;
	counter->rx_alloc_hec_3err_cnt = rx_alloc_hec_cnt.Bits.rx_alloc_hec_3err_cnt;

    rx_hdr_hec_cnt.Raw =		IO_GREG(RX_HDR_HEC_CNT) ;
	counter->rx_hdr_hec_1err_cnt = rx_hdr_hec_cnt.Bits.rx_hdr_hec_1err_cnt;
	counter->rx_hdr_hec_2err_cnt = rx_hdr_hec_cnt.Bits.rx_hdr_hec_2err_cnt;
	counter->rx_hdr_hec_3err_cnt = rx_hdr_hec_cnt.Bits.rx_hdr_hec_3err_cnt;

	rx_phy_hec_cnt.Raw =		IO_GREG(RX_PHY_HEC_ERR_CNT) ;
	counter->rx_pon_id_hec_err_cnt = rx_phy_hec_cnt.Bits.rx_pon_id_hec_err_cnt;
	counter->rx_sfc_hec_err_cnt = rx_phy_hec_cnt.Bits.rx_sfc_hec_err_cnt;

	rx_mic_err_cnt.Raw =		IO_GREG(RX_MIC_ERR_CNT) ;
	counter->rx_omci_mic_err_cnt = rx_mic_err_cnt.Bits.rx_omci_mic_err_cnt;
	counter->rx_ploam_mic_err_cnt = rx_mic_err_cnt.Bits.rx_ploam_mic_err_cnt;

	rx_eth_fcs_cnt.Raw =		IO_GREG(RX_ETH_FCS_ERR_CNT) ;
	counter->rx_eth_crc_err_cnt = rx_eth_fcs_cnt.Bits.rx_eth_crc_err_cnt;

	rx_bip_cnt.Raw =		IO_GREG(RX_BIP_ERR_CNT) ;
	counter->rx_bip_err_cnt = rx_bip_cnt.Bits.rx_bip_err_cnt;
	
	rx_key_err_cnt.Raw =	  IO_GREG(RX_KEY_ERR_CNT) ;
	counter->rx_key_err_cnt = rx_key_err_cnt.Bits.rx_key_err_cnt;

	rx_los_wcnt.Raw =		IO_GREG(RX_LOST_WCNT) ;
	counter->rx_lost_wcnt = rx_los_wcnt.Bits.rx_lost_wcnt;

	invld_prof_bst_gnt_cnt.Raw =	  IO_GREG(INVLD_PROF_BST_GNT_CNT) ;
	counter->invld_prof_bst_gnt_cnt = invld_prof_bst_gnt_cnt.Bits.invld_prof_bst_gnt_cnt;

	rx_mbi_xgem_drop_cnt.Raw = IO_GREG(RX_MBI_XGEM_DROP_CNT) ;
	counter->rx_mbi_xgem_drop_cnt = rx_mbi_xgem_drop_cnt.Bits.rx_mbi_xgem_drop_cnt;

}
/******************************************************************************
 Descriptor:	set normal counter clear.
 Input Args:	counter: used to clear counter value.
 Ret Value:	none
******************************************************************************/
void gponDevSetCounterClear(GPON_DEV_NORMAL_COUNTER_T *counter)
{
    REG_CNT_CLR   cnt_clear;
    
	cnt_clear.Raw =		IO_GREG(CNT_CLR);
	
	if(counter->clear == GPON_NORMAL_COUNTER_CLEAR){
		cnt_clear.Bits.nml_cnt_clr = XPON_ENABLE;
	}else if(counter->clear == GPON_ERR_COUNTER_CLEAR){
		cnt_clear.Bits.err_cnt_clr = XPON_ENABLE;
	}

	IO_SREG(CNT_CLR, cnt_clear.Raw);	
}
/******************************************************************************
 Descriptor:	get error status
 Input Args:	status: used to store error status value.
 Ret Value:	none
******************************************************************************/
void gponDevGetErrStatus(GPON_DEV_ERR_STATUS_T *status)
{
    REG_FIFO_ERR_STS   gponFifoErrSts;  
    REG_TX_ERR_STS     gponTxErrSts;
    REG_RX_ERR_STS     gponRxErrSts;
	REG_DBG_BWM_CKH_STS gponbwpChkStatus ;

	gponFifoErrSts.Raw =	IO_GREG(FIFO_ERR_STS) ;
	status->gponFifoErrSts.Raw = gponFifoErrSts.Raw;

	gponTxErrSts.Raw =	IO_GREG(TX_ERR_STS) ;
	status->gponTxErrSts.Raw = gponTxErrSts.Raw;

	gponRxErrSts.Raw =	IO_GREG(RX_ERR_STS) ;
	status->gponRxErrSts.Raw = gponRxErrSts.Raw;
	
	gponbwpChkStatus.Raw = IO_GREG(DBG_BWM_CKH_STS);
	status->gponbwpChkStatus.Raw = gponbwpChkStatus.Raw;
}
/******************************************************************************
 Descriptor:	set error status clear
 Input Args:	none
 Ret Value:	none
******************************************************************************/
void gponDevSetErrStatusClear(void)
{
#if defined(TCSUPPORT_CPU_AN7583)
	IO_SREG(FIFO_ERR_STS, IO_GREG(FIFO_ERR_ENABLE));
	IO_SREG(TX_ERR_STS, IO_GREG(TX_ERR_ENABLE));
	IO_SREG(RX_ERR_STS, IO_GREG(RX_ERR_ENABLE));
#else
	IO_SREG(FIFO_ERR_STS, 0xffffffff);
	IO_SREG(TX_ERR_STS, 0xffffffff);
	IO_SREG(RX_ERR_STS, 0xffffffff);
#endif		
	IO_SREG(DBG_BWM_CKH_STS, 0xffffffff);	
}

/*_____________________________________________________________________________
**      function name: gponDevClearCntAndErr
**      descriptions:
**           It's used to send the normal count and err counter and err status.
**      parameters:
**            None
**      global:
**             None
**      return:
**             None
**      call:
**   	         gponDevSetCounterClear
**            gponDevSetErrStatusClear
**      revision:
**            None
**____________________________________________________________________________
*/
void gponDevClearCntAndErr(void)
{
	GPON_DEV_NORMAL_COUNTER_T counter;
    
    counter.clear = GPON_ERR_COUNTER_CLEAR;
    gponDevSetCounterClear(&counter);
    counter.clear = GPON_NORMAL_COUNTER_CLEAR;
    gponDevSetCounterClear(&counter);
}

/******************************************************************************
 Descriptor:	set ploamd msg(exclude profile) filter and  profile ploam filter.
 Input Args:	filter: used to enable/disable the filter type.
 Ret Value:		0: Success.
               	    -1: Input Args error.
******************************************************************************/
int gponDevSetPloamFilterMode(GPON_10G_DEV_PLOAMD_FILTER_MODE_T *filter)
{
   
	REG_DBG_CAP_SETTING		gponFilter;
    
	if(filter->ploamd_filter != XPON_DISABLE && filter->ploamd_filter != XPON_ENABLE) {
		return -1;		
	}
	if(filter->profile_filter != XPON_DISABLE && filter->profile_filter != XPON_ENABLE) {
		return -1;
	}
	
	gponFilter.Raw =	IO_GREG(DBG_CAP_SETTING);
	gponFilter.Bits.rept_prof_flt = (gponFilter.Bits.rept_prof_flt == filter->profile_filter) ? gponFilter.Bits.rept_prof_flt : filter->profile_filter;
	gponFilter.Bits.rept_msg_flt = (gponFilter.Bits.rept_msg_flt == filter->ploamd_filter) ? gponFilter.Bits.rept_msg_flt : filter->ploamd_filter;
	IO_SREG(DBG_CAP_SETTING, gponFilter.Raw) ;
	
	return 0;
}
/******************************************************************************
 Descriptor:	get ploamd msg(exclude profile) filter and  profile ploam filter.
 Input Args:	filter: used to set the filter type.
 Ret Value:	none            	    
******************************************************************************/
void gponDevGetPloamFilterMode(GPON_10G_DEV_PLOAMD_FILTER_MODE_T *filter)
{
	REG_DBG_CAP_SETTING		gponFilter; 	
	
	gponFilter.Raw =	IO_GREG(DBG_CAP_SETTING);
	filter->profile_filter = gponFilter.Bits.rept_prof_flt;
	filter->ploamd_filter = gponFilter.Bits.rept_msg_flt;

}
/******************************************************************************
 Descriptor:	set dba backdoor mode and value.
 Input Args:	GPON_10G_DEV_DBA_BACKDOOR_T: used to set the modify type and modify value.
 Ret Value:	0: Success.
               	    -1: Input Args error.            	    
******************************************************************************/
int gponDevSetDbaBackdoorMode(GPON_10G_DEV_DBA_BACKDOOR_T *dba)
{
	REG_DBG_DBA_BACK_DOOR           gponDbaBackdoor;
    REG_DBG_DBA_MODIFY              gponDbaModify;  
    uint    modifyMode=0;
    uint    modifyValue=0;

    if(dba->mode > GPON_DBA_BACKDOOR_SHIFT_MODE)
        return -1;

    gponDbaBackdoor.Raw =   IO_GREG(DBG_DBA_BACK_DOOR);
    gponDbaModify.Raw =   IO_GREG(DBG_DBA_MODIFY);
    
    if((dba->mode == GPON_DBA_BACKDOOR_FIX_MODE)||(dba->mode == GPON_DBA_BACKDOOR_NOT_MODIFY)){
        gponDbaModify.Bits.dba_modify = dba->value;
    }else{
        modifyValue = (dba->value & 0x7fffffff);
        if(dba->mode == GPON_DBA_BACKDOOR_ADD_MODE){
            if(dba->addMode ==GPON_DBA_BACKDOOR_ADD){
                modifyMode = (GPON_DBA_BACKDOOR_ADD <<31);
            }
            else{
                modifyMode =(GPON_DBA_BACKDOOR_MINUS <<31);
            }
        }else if(dba->mode == GPON_DBA_BACKDOOR_SHIFT_MODE){
            if(dba->shiftMode ==GPON_DBA_BACKDOOR_LEFT_SHIFT){
                modifyMode =(GPON_DBA_BACKDOOR_LEFT_SHIFT <<31);
            }
            else{
                modifyMode =(GPON_DBA_BACKDOOR_RIGHT_SHIFT <<31);
            }
        }else{
            return -1;
        }
        gponDbaModify.Bits.dba_modify = (modifyValue | modifyMode);
    }           	
    gponDbaBackdoor.Bits.dba_modify_mode = dba->mode;
    IO_SREG(DBG_DBA_BACK_DOOR, gponDbaBackdoor.Raw) ;
    IO_SREG(DBG_DBA_MODIFY, gponDbaModify.Raw) ;

    return 0;    
}

#ifdef TCSUPPORT_CPU_AN7583
/******************************************************************************
 Descriptor:	set dba backdoor mode and value for 1/2 tcont.
 Input Args:	GPON_10G_DEV_DBA_BACKDOOR_T: used to set the modify type and modify value.
 Ret Value:	0: Success.
               	    -1: Input Args error.            	    
******************************************************************************/
int gponDevSetDbaBackdoorModeSeperate(GPON_10G_DEV_DBA_BACKDOOR_SEPERATE_T *dba)
{
	REG_DBG_DBA_BACK_DOOR           gponDbaBackdoor;
    REG_DBG_DBA_MODIFY              gponDbaModify;  
    uint    modifyMode=0;
    uint    modifyValue=0;

    if(dba->mode > GPON_DBA_BACKDOOR_SHIFT_MODE)
        return -1;

    gponDbaBackdoor.Raw =   IO_GREG(DBG_DBA_BACK_DOOR);
    gponDbaModify.Raw =   IO_GREG(DBG_DBA_MODIFY);
    
    if((dba->mode == GPON_DBA_BACKDOOR_FIX_MODE)||(dba->mode == GPON_DBA_BACKDOOR_NOT_MODIFY)){
        gponDbaModify.Bits.dba_modify = dba->value;
    }else{
        modifyValue = (dba->value & 0x7fffffff);
        if(dba->mode == GPON_DBA_BACKDOOR_ADD_MODE){
            if(dba->addMode ==GPON_DBA_BACKDOOR_ADD){
                modifyMode = (GPON_DBA_BACKDOOR_ADD <<31);
            }
            else{
                modifyMode =(GPON_DBA_BACKDOOR_MINUS <<31);
            }
        }else if(dba->mode == GPON_DBA_BACKDOOR_SHIFT_MODE){
            if(dba->shiftMode ==GPON_DBA_BACKDOOR_LEFT_SHIFT){
                modifyMode =(GPON_DBA_BACKDOOR_LEFT_SHIFT <<31);
            }
            else{
                modifyMode =(GPON_DBA_BACKDOOR_RIGHT_SHIFT <<31);
            }
        }else{
            return -1;
        }
        gponDbaModify.Bits.dba_modify = (modifyValue | modifyMode);
    }     
	if(dba->tcontSelect == GPON_DBA_BACKDOOR_FOR_TCONT_IDX0) {
		
    	gponDbaBackdoor.Bits.dba_modify_tcont_idx_0_mode = dba->mode;
		gponDbaBackdoor.Bits.dba_modify_tcont_idx_0_ovrw = dba->dba_backdoor_enable;
		gponDbaBackdoor.Bits.dba_modify_tcont_idx_0 = dba->tcontId;
		IO_SREG(DBG_DBA_MODIFY_IDX_0, gponDbaModify.Raw) ;
	} else if(dba->tcontSelect == GPON_DBA_BACKDOOR_FOR_TCONT_IDX1){
    	gponDbaBackdoor.Bits.dba_modify_tcont_idx_1_mode = dba->mode;
		gponDbaBackdoor.Bits.dba_modify_tcont_idx_1_ovrw = dba->dba_backdoor_enable;
		gponDbaBackdoor.Bits.dba_modify_tcont_idx_1 = dba->tcontId;
		IO_SREG(DBG_DBA_MODIFY_IDX_1, gponDbaModify.Raw) ;
	}
	IO_SREG(DBG_DBA_BACK_DOOR, gponDbaBackdoor.Raw) ;
    return 0;    
}
#endif
/******************************************************************************
 Descriptor:	get dba backdoor mode and value.
 Input Args:	dbaMode: fix,add,shift or none mode;modifyValue: modify value of different mode
 Ret Value:	none         	    
******************************************************************************/
void gponDevGetDbaBackdoorMode(uint *dbaMode,uint *modifyValue)
{

    REG_DBG_DBA_BACK_DOOR           gponDbaBackdoor;
    REG_DBG_DBA_MODIFY              gponDbaModify;
    
    gponDbaBackdoor.Raw =   IO_GREG(DBG_DBA_BACK_DOOR);
    gponDbaModify.Raw =   IO_GREG(DBG_DBA_MODIFY);

    *dbaMode = gponDbaBackdoor.Bits.dba_modify_mode;
    *modifyValue = gponDbaModify.Bits.dba_modify ;
}

/******************************************************************************
 Descriptor:	set downstream omci mic calculate by hw or sw.
 Input Args:    XGPON_SW_HW_SELECT_T
 Ret Value:	none         	    
******************************************************************************/
void gponDevSetDownstreamOmciMicCtrl(XGPON_SW_HW_SELECT_T ctrl)
{
    REG_DBG_CAP_SETTING		gponOmciMicCtrl;

    gponOmciMicCtrl.Raw =	IO_GREG(DBG_CAP_SETTING);
	gponOmciMicCtrl.Bits.hw_cal_ds_omci_mic = (ctrl == XGPON_HW) ? 1 : 0;
	IO_SREG(DBG_CAP_SETTING, gponOmciMicCtrl.Raw) ;
    omciMicErrSwCnt = 0;
}

/******************************************************************************
 Descriptor:	set upstream omci mic calculate by hw or sw.
 Input Args:    XGPON_SW_HW_SELECT_T
 Ret Value:	none         	    
******************************************************************************/
void gponDevSetUpstreamOmciMicCtrl(XGPON_SW_HW_SELECT_T ctrl)
{
    REG_DBG_CAP_SETTING		gponOmciMicCtrl;
    
    gponOmciMicCtrl.Raw =	IO_GREG(DBG_CAP_SETTING);
	gponOmciMicCtrl.Bits.hw_cal_us_omci_mic = (ctrl == XGPON_HW) ? 1 : 0;
	IO_SREG(DBG_CAP_SETTING, gponOmciMicCtrl.Raw) ;
}

/******************************************************************************
 Descriptor:	get upstream and downstream omci mic calculate mode ,hw or sw.
 Input Args:    upCtrl:upstream ctrl mode,downCtrl:downstream ctrl mode
 Ret Value:	none         	    
******************************************************************************/
void gponDevGetOmciMicCtrl(GPON_10G_DEV_OMCI_MIC_CTRL_T *omciMicCtrl)
{
    REG_DBG_CAP_SETTING		gponOmciMicCtrl;
    
    gponOmciMicCtrl.Raw =	IO_GREG(DBG_CAP_SETTING);
    omciMicCtrl->usOmciMicMode = (gponOmciMicCtrl.Bits.hw_cal_us_omci_mic == 0) ? XGPON_SW : XGPON_HW;    
	omciMicCtrl->dsOmciMicMode = (gponOmciMicCtrl.Bits.hw_cal_ds_omci_mic == 0) ? XGPON_SW : XGPON_HW;
}
/*****************************************************************************
******************************************************************************/
void gponDevSetMibCounterType(GPON_10G_COUNTER_TYPE_t type)
{
	REG_MIB_CTRL gponMibCtrl ;

	gponMibCtrl.Raw = IO_GREG(MIB_CTRL) ;
	gponMibCtrl.Bits.mib_frame_type = type ;	
	IO_SREG(MIB_CTRL, gponMibCtrl.Raw) ;	
}

/*****************************************************************************
******************************************************************************/
void gponDevGetMibCounterType(GPON_10G_COUNTER_TYPE_t *type)
{
	REG_MIB_CTRL gponMibCtrl ;
	gponMibCtrl.Raw = IO_GREG(MIB_CTRL) ;
	*type = gponMibCtrl.Bits.mib_frame_type ;	
}

/*****************************************************************************
******************************************************************************/
int gponDevGemMibTablesInit(void)
{
	int RETRY = RETRY_TIMES ;
	REG_MIB_TBL_CONFIG gponMibTblInit ;    
	REG_GPIDX_TBL_INIT gponGPIdxTblInit ;
	
	gponMibTblInit.Raw = 0 ;
	gponMibTblInit.Bits.mib_tbl_init_start = 1 ;
	IO_SREG(MIB_TBL_CONFIG, gponMibTblInit.Raw) ;

    gponGPIdxTblInit.Raw = 0 ;
    gponGPIdxTblInit.Bits.gpidx_tbl_init_start = 1 ;
    IO_SREG(GPIDX_TBL_INIT, gponGPIdxTblInit.Raw) ;

	
	while(RETRY--) {
		gponMibTblInit.Raw = IO_GREG(MIB_TBL_CONFIG) ;
        gponGPIdxTblInit.Raw = IO_GREG(GPIDX_TBL_INIT) ;
		if(gponMibTblInit.Bits.mib_tbl_init_done && gponGPIdxTblInit.Bits.gpidx_tbl_init_done) {
			return 0 ;
		} 
		udelay(1) ;
	}
	PON_MSG(MSG_ERR, "%s %d failed to init.\n",__FUNCTION__,__LINE__) ;
	return -ETIME ;
}
/*****************************************************************************
******************************************************************************/
int gponDevUpdateGemMibIdxTable(ushort addr, ushort data)
{
	int RETRY = RETRY_TIMES ;
	REG_GPIDX_TBL_CTRL gponGPIdxTblCtrl ;
	REG_GPIDX_TBL_STS gponGPIdxTblStatus ;
	
	gponGPIdxTblCtrl.Raw = 0 ;
	gponGPIdxTblCtrl.Bits.gpidx_tbl_cmd = 1 ; //Write Command
	gponGPIdxTblCtrl.Bits.gpidx_tbl_addr = addr ;
	gponGPIdxTblCtrl.Bits.gpidx_tbl_wdata = data ;
	IO_SREG(GPIDX_TBL_CTRL, gponGPIdxTblCtrl.Raw) ;
	
	while(RETRY--) {
		gponGPIdxTblStatus.Raw = IO_GREG(GPIDX_TBL_STS) ;
		if(gponGPIdxTblStatus.Bits.gpidx_cmd_done) {
			return 0 ;
		} 
		udelay(1) ;
	}
    PON_MSG(MSG_ERR, "%s %d failed to update.\n",__FUNCTION__,__LINE__) ;
	return -ETIME ;
}
/*****************************************************************************
******************************************************************************/
int gponDevGetGemMibIdxTable(ushort addr, uint *data)
{
	int RETRY = RETRY_TIMES ;
	REG_GPIDX_TBL_CTRL gponGPIdxTblCtrl ;
	REG_GPIDX_TBL_STS gponGPIdxTblStatus ;
	
	gponGPIdxTblCtrl.Raw = 0 ;
	gponGPIdxTblCtrl.Bits.gpidx_tbl_cmd = 0 ; //Read Command
	gponGPIdxTblCtrl.Bits.gpidx_tbl_addr = addr ;
	IO_SREG(GPIDX_TBL_CTRL, gponGPIdxTblCtrl.Raw) ;
	
	while(RETRY--) {
		gponGPIdxTblStatus.Raw = IO_GREG(GPIDX_TBL_STS) ;
		if(gponGPIdxTblStatus.Bits.gpidx_cmd_done) {
            *data = gponGPIdxTblStatus.Bits.gpidx_tbl_rdata ;
			return 0 ;
		} 
		udelay(1) ;
	}
	PON_MSG(MSG_ERR, "%s %d failed to get.\n",__FUNCTION__,__LINE__) ;
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
                    uint64_t *pData, unchar readClear)
{
	int RETRY = RETRY_TIMES ;
    REG_MIB_TBL_CONFIG gponMibTblInit ;
	REG_MIB_CTRL_STS gponGemMibCtrl ;
	
	if(gemPortId == 0xFFFF) {
		return -EFAULT ;	
	}

    gponMibTblInit.Raw = IO_GREG(MIB_TBL_CONFIG) ;
    gponMibTblInit.Bits.mib_tbl_rd_clr = readClear ;
	IO_SREG(MIB_TBL_CONFIG, gponMibTblInit.Raw) ;
    
	gponGemMibCtrl.Raw = 0 ;
	gponGemMibCtrl.Bits.mib_wr = 0 ; /* Read Command */	 
	gponGemMibCtrl.Bits.mib_type = type ;
	gponGemMibCtrl.Bits.mib_gpid = gemPortId ;
	IO_SREG(MIB_CTRL_STS, gponGemMibCtrl.Raw) ;

	while(RETRY--) {
		gponGemMibCtrl.Raw = IO_GREG(MIB_CTRL_STS) ;
		if(gponGemMibCtrl.Bits.mib_cmd_done) {
			*pData = (((uint64_t)IO_GREG(MIB_RDATA_H32))<<32)+(uint64_t)(IO_GREG(MIB_RDATA_L32)) ;
			return 0 ;
		} 
		udelay(1) ;
	}

	return -ETIME ;
}
/*****************************************************************************
******************************************************************************/
int gponDevClearSwCounter(void)
{
	int i;
	
	for(i = 0 ; i < CONFIG_GPON_10G_MAX_GEMPORT ; i++) {
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
	for(j = 1, i = 0 ; i < GPON_10G_MAX_GEM_ID ; i++) {
		gemIdx = (gpWanPriv->gpon.gemIdToIndex[i] & GPON_GEM_IDX_MASK);
		if(gemIdx<CONFIG_GPON_10G_MAX_GEMPORT && gpWanPriv->gpon.gemPort[gemIdx].info.valid) {
			idx++ ;
		}
		PON_MSG(MSG_DBG, "XGPON dev clear hw counter i = %d, j = %d, gemIdx = %x, idx = %d\n",i,j,gemIdx,idx);
        if(((i&0x1F)==0x1F) && (j < XGPON_GPID_TAB_MAX_ADDR)) {
			gponDevUpdateGemMibIdxTable(j, idx) ;
			j++ ;
		}
	}
	
	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gponDevClearGemPortCounter(ushort gemPortId)
{
    uint64_t pData = 0;
    
    if (0 !=  __gponDevGetGemPortCounter(gemPortId, GEMPORT_RX_FRAME_CNT,   &pData, 1) )
        return -1;

    if (0 !=  __gponDevGetGemPortCounter(gemPortId, GEMPORT_RX_PL_BYTE_CNT, &pData, 1) )
        return -1;

    if (0 !=  __gponDevGetGemPortCounter(gemPortId, GEMPORT_TX_FRAME_CNT,   &pData, 1) )
        return -1;

    if (0 !=  __gponDevGetGemPortCounter(gemPortId, GEMPORT_TX_PL_BYTE_CNT, &pData, 1) )
        return -1;

    return 0;
}
/*****************************************************************************
******************************************************************************/
int gponDevGetGemPortCounter(ushort gemPortId, GPON_GEMPORT_STATS_TYPE_t type, uint64_t *pData)
{
    return __gponDevGetGemPortCounter(gemPortId, type, pData, 0);
}
/*_____________________________________________________________________________
**      function name: gponDevSwReplyRegistrationPloam
**      descriptions:
**           It's used to send the registration ploam by sw.
**      parameters:
**            None
**      global:
**             None
**      return:
**             None
**      call:
**   	         gponDevGetTxSyncRdy
**            ploam_send_registration_msg
**      revision:
**            None
**____________________________________________________________________________
*/
void gponDevSwReplyRegistrationPloam(unsigned long arg){

    if(gponDevGetTxSyncRdy() == GPON_TCONT_CMD_SUCCESS)
    {
        ploam_send_registration_msg(0,gpGponPriv->gponCfg.reg_id);
        ploam_send_registration_msg(0,gpGponPriv->gponCfg.reg_id);
    }
}

int gponDevWaitTxAlignFifoUnused(void){
	REG_DBG_TX_ALIGN_FIFO_STS txAlignFifoUsed;
	int RETRY = 3000 ;

	while(RETRY--){
		txAlignFifoUsed.Raw = IO_GREG(DBG_TX_ALIGN_FIFO_STS);
		if(txAlignFifoUsed.Bits.tx_align_fifo_used == 0){
			return 0;
		}
	}
	
	return -1;
}

/******************************************************************************
 **      function name: gponDevSwResync
 **      descriptions:
 **           It's used to start the resync by sw
 **      parameters:
 **            None
 **      global:
 **             None
 **      return:
 **             None
 **      call:
 **             None
 **      revision:
 **            None
 **
 ******************************************************************************/
void gponDevSwResync(void)
{
	REG_DBG_RESYNC gponResync; 
    int RETRY = 3000 ;

#if !defined(TCSUPPORT_CPU_AN7583)	/*eqd adjust for sw_resync*/	
	gponDevRefreshEqdValue();
#endif	
    gponResync.Raw = IO_GREG(DBG_RESYNC) ;
	gponResync.Bits.sw_resync_en = GPON_SW_RESYNC_ENABLE;
	gponResync.Bits.sw_resync_start = GPON_SW_RESYNC;
	IO_SREG(DBG_RESYNC, gponResync.Raw) ;   
	while(RETRY--) {
		gponResync.Raw = IO_GREG(DBG_RESYNC) ;
		if(gponResync.Bits.tx_sync_rdy) {
			break  ;
		}
	}
}
int gponDevTxSyncRdy(void)
{
	int RETRY = 200 ;
	REG_DBG_RESYNC gponResync; 

	while(RETRY--) {
		gponResync.Raw = IO_GREG(DBG_RESYNC) ;
		if(gponResync.Bits.tx_sync_rdy) {
			return 0  ;
		}
		udelay(1);
	}

	return -1;
}
int gponDevSwResyncCompleteProcess(void){
	REG_US_PROF_VLD usProfileVld;
	uint32 regReadData = 0;
	
	regReadData = IO_GREG(US_PROF_VLD);

	/*Modify the value of us_prof_vld bit*/
	usProfileVld.Raw = regReadData;
	usProfileVld.Bits.us_prof3_vld = 0; 	//0:invalid
	usProfileVld.Bits.us_prof2_vld = 0;
	usProfileVld.Bits.us_prof1_vld = 0;
	usProfileVld.Bits.us_prof0_vld = 0;
	
	/*Disable upstream profile valid bit*/	
	IO_SREG(US_PROF_VLD, usProfileVld.Raw) ;
	
	/*Disable tx mbi*/
	gponDevTxMbiStop(XPON_RESET_HOLD_ON);
	
	/*Wait the current used entries of tx align FIFO are 0*/
	/*
	if(gponDevWaitTxAlignFifoUnused() != 0){
		PON_MSG(MSG_ERR,"ERROR: Waiting tx algin fifo unused timeout!\n");
		return -1;
	}
	*/
	/*Disable tx mpi*/
	gponDevMpiTxStop(XPON_RESET_HOLD_ON);
	
	/*Execute sw resync*/
	gponDevSwResync();
	
	/*Enable tx mpi*/
	gponDevMpiTxStop(XPON_RESET_RELEASE);
	
	if(gponDevTxSyncRdy() !=0){		
		printk("errror tx sync not ready in swresync complete process\n");	
	}
	
	/*Enable tx mbi*/
	gponDevTxMbiStop(XPON_RESET_RELEASE);
        /*Restore upstream profile valid bit*/
	IO_SREG(US_PROF_VLD, regReadData) ;

	return 0;
}

int gponDevSetTxLateStartErrResyncEn(XPON_Mode_t mode)
{
#if defined(TCSUPPORT_CPU_EN7581)
	REG_DBG_RESYNC gponResync;
	
	if(mode != XPON_DISABLE && mode != XPON_ENABLE) {
		return -1;
	}
	
	gponResync.Raw = IO_GREG(DBG_RESYNC) ;
	gponResync.Bits.tx_late_start_err_resync_en = mode ;
	IO_SREG(DBG_RESYNC, gponResync.Raw) ;
#endif
	return 0;
}
/*****************************************************************************
**		function name: gponDevSetIdleGemThreshold
**		descriptions:
**			 It's used to set the idle gem threshold
**		parameters:
**			  None
**		global:
**			   None
**		return:
**			   None
**		call:
**			   None
**		revision:
**			  None
**
******************************************************************************/
void gponDevSetIdleGemThreshold(ushort idle_gem_thld)
{
	REG_IDLE_GEM_CTRL	gponIdleGemThld;

	gponIdleGemThld.Raw = IO_GREG(IDLE_GEM_CTRL) ;
	gponIdleGemThld.Bits.idle_gem_thld = idle_gem_thld;
	//when fe clock is slow, idle_gem_max_size must be adjusted,otherwise it will cause TX FIFO err
	if(isFPGA && (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGSPON || gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G)) {
		gponIdleGemThld.Bits.idle_gem_max_size = 0x30;
	}
	IO_SREG(IDLE_GEM_CTRL, gponIdleGemThld.Raw) ;
	
}


#ifdef TCSUPPORT_CPU_EN7581
/******************************************************************************
 **      function name: gponDevGetRxIdleXgem
 **      descriptions:
 **           It's used to get rx idle xgem len and 1st word payload
 **      parameters:
 **            set get_rx_idle_xgem 1
 **      global:
 **             None
 **      return:
 **             None
 **      call:
 **             None
 **      revision:
 **            None
 **
******************************************************************************/
void gponDevGetRxIdleXgem(int getRxIdXg)
{
    REG_GET_RX_IDLE_XGEM	gponGetRxIdleXgem;
    REG_RX_IDLE_XGEM_1ST_PL	gponRxIdleXgem1stPl;

	gponGetRxIdleXgem.Raw =	IO_GREG(GET_RX_IDLE_XGEM) ;
	gponGetRxIdleXgem.Bits.get_rx_idle_xgem = getRxIdXg;
	IO_SREG(GET_RX_IDLE_XGEM, gponGetRxIdleXgem.Raw) ;

	udelay(125);
	gponGetRxIdleXgem.Raw =	IO_GREG(GET_RX_IDLE_XGEM) ;
	gponRxIdleXgem1stPl.Raw = IO_GREG(RX_IDLE_XGEM_1ST_PL) ;

	printk("rx_idle_xgem_len: %d    |   rx_idle_xgem_1st_pl:%.8x\n", gponGetRxIdleXgem.Bits.rx_idle_xgem_len,gponRxIdleXgem1stPl.Bits.rx_idle_xgem_1st_pl);
}

/*****************************************************************************
******************************************************************************/
int gponDevSetNewTod(XMCS_GponTodCfg_t new_tod) 
{
	/*float n1270 = 1.4677;	
	float n1577 = 1.4686;	
	float up_rate = 2.48832;	
	float down_rate = 9.95328;*/
	uint64_t  n1270_x10000 = 14677;
	uint64_t  n1577_x10000 = 14686;	
	uint64_t up_rate_x100000 = 248832;	 
	uint64_t down_rate_x100000 = 995328; 
	uint64_t dividend1 = 0;
	uint64_t divisor1 = 0;
	uint64_t dividend2 = 0;
	uint64_t divisor2 = 0;
	uint64_t dividend3 = 0;
	uint64_t divisor3 = 0;
	uint delta = 0;
	/*uint delta_float = 0;*/
	int xpon_tod_adjust = 0;
	REG_RSP_TIME gponRspTime = {0};
	REG_DBG_RX_PHY_DLY_OFS todRxPhyDly = {0};
	REG_TOD_SFC_L todSfcL = {0};
	REG_TOD_SFC_H todSfcH = {0};
	REG_DBG_DS_SPF_CNT_L dsSpfCntL = {0};
	REG_DBG_DS_SPF_CNT_H dsSpfCntH = {0};
	REG_TOD_UPD_CTRL todUpdCtrl = {0};
	xpon_tod_data_t tod_data = {0};
	
	PON_MSG(MSG_DBG,"Before gponDevSetNewTod superframe = 0x%x, sec_H16 = 0x%x, sec_L32 = 0x%x, nanosec = 0x%x\n",new_tod.superframe,new_tod.sec_H16, new_tod.sec, new_tod.nanosec);

	if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGSPON || gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G)
	{
		/*up_rate = 9.95328;  *///xgspon upstream rate 9.95328 Gbit/s;
		up_rate_x100000 = 995328;
	}
	else if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGPON || gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_2G \
		|| gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_2G_2G)
	{
		/*up_rate = 2.48832;	*///xgpon upstream rate 2.48832 Gbit/s;
		up_rate_x100000 = 248832;
	}
	
	gponRspTime.Raw = IO_GREG(RSP_TIME);
	todRxPhyDly.Raw = IO_GREG(DBG_RX_PHY_DLY_OFS);
	xpon_tod_adjust = gpon_tod_adjust;

	PON_MSG(MSG_DBG,"gponDevSetNewTod eqd = %d, gponRspTime.Raw = 0x%x, todRxPhyDly.Raw = 0x%x, xpon_tod_adjust = %d\n",gpGponPriv->gponCfg.eqd,gponRspTime.Raw,todRxPhyDly.Raw,xpon_tod_adjust);

	/*delta_float =(uint)((((gpGponPriv->gponCfg.eqd + gponRspTime.Bits.tresp * 64)/up_rate) * (n1577 / (n1270+n1577))) - (todRxPhyDly.Bits.rx_phy_dly_offset / down_rate) + xpon_tod_adjust);*/

	/*All numbers multiple the same times to avoid float number and maintain the result unchanged.*/
	dividend1 = ((uint64_t)gpGponPriv->gponCfg.eqd + (uint64_t)gponRspTime.Bits.tresp * 64) * 100000;
	divisor1 = up_rate_x100000;

	dividend2 = n1577_x10000;
	divisor2 = n1270_x10000 + n1577_x10000;

	dividend3 = (uint64_t)todRxPhyDly.Bits.rx_phy_dly_offset * 100000;
	divisor3 = down_rate_x100000;

	/*only operation of integer parts*/
	if(((dividend1 / divisor1 * dividend2 / divisor2) + xpon_tod_adjust) > (dividend3 / divisor3) )
	{
		delta = (uint)((dividend1 / divisor1 * dividend2 / divisor2) + xpon_tod_adjust - (dividend3 / divisor3));
	}
	else
	{
		delta = 0;
		PON_MSG(MSG_DBG,"phy_rx_dly is larger");
	}
	
	/*When the left result is less than right result, we should let delta minus one to affort the lack.*/
	if((((dividend1 / divisor1 * dividend2 % divisor2) * divisor1 ) + (dividend1 % divisor1 * dividend2)) * divisor3 < (dividend3 % divisor3) * divisor1 * divisor2)
	{
		if(delta >= 1)
		{
			delta--;
		}
	}

	if (delta > new_tod.nanosec)
	{
		if(1 <= new_tod.sec) 
		{	
			new_tod.sec = new_tod.sec -1;
		}
		else
		{	
			new_tod.sec_H16= new_tod.sec_H16 - 1;
			new_tod.sec = 0xFFFFFFFF;
		}
		
		new_tod.nanosec = new_tod.nanosec + 0x389ACA00 - delta;	//1s = 1000000000ns = 0x389ACA00ns
	}
	else
	{
		new_tod.nanosec = new_tod.nanosec - delta;
	}
	
	PON_MSG(MSG_DBG,"After gponDevSetNewTod delta = 0x%x, sec_H16 = 0x%x, sec_L32 = 0x%x, nanosec = 0x%x\n",delta,new_tod.sec_H16, new_tod.sec, new_tod.nanosec);

	todSfcL.Raw = IO_GREG(TOD_SFC_L);
	todSfcH.Raw = IO_GREG(TOD_SFC_H);
	dsSpfCntL.Raw = IO_GREG(DBG_DS_SPF_CNT_L);
	dsSpfCntH.Raw = IO_GREG(DBG_DS_SPF_CNT_H);
	
	todSfcL.Bits.tod_spf_cnt_l = new_tod.superframe;
	todSfcL.Bits.tod_spf_cnt_l |= (dsSpfCntL.Raw & 0xC0000000);  //ALU OLT OMCI use gpon 30 bit superframe, test code
	todSfcH.Raw = dsSpfCntH.Raw;
	IO_SREG(TOD_SFC_L, todSfcL.Raw);
	IO_SREG(TOD_SFC_H, todSfcH.Raw);

	PON_MSG(MSG_DBG,"gponDevSetNewTod todSfcL.Raw = 0x%x, todSfcH.Raw = 0x%x, dsSpfCntL.Raw = 0x%x, dsSpfCntH.Raw = 0x%x\n",todSfcL.Raw,todSfcH.Raw, dsSpfCntL.Raw, dsSpfCntH.Raw);
	
	tod_data.tod_source = 0;
	tod_data.superframe = new_tod.superframe;
	tod_data.sec_H16 = new_tod.sec_H16;
	tod_data.sec_L32= new_tod.sec;
	tod_data.nanosec = new_tod.nanosec;

	if(isFPGA)
	{	
		tod_data.tod_add_unit_int = 12;
		tod_data.tod_add_unit_nume = 209;
		tod_data.tod_add_unit_deno = 243;
	}
	else
	{
		tod_data.tod_add_unit_int = 6;
		tod_data.tod_add_unit_nume = 209;
		tod_data.tod_add_unit_deno = 486;
	}

	todUpdCtrl.Raw = IO_GREG(TOD_UPD_CTRL);
	todUpdCtrl.Bits.tod_upd_en = 1;
	IO_SREG(TOD_UPD_CTRL, todUpdCtrl.Raw);
	
	if(ECNT_API_XPON_TOD_GEN_SET_HOOK(tod_data))
	{
		PON_MSG(MSG_DBG,"[%s] [%d] return fail\n",__FUNCTION__, __LINE__);
		return -1;
	}

	return 0 ;
}

#endif


/*****************************************************************************
					phy register setting 
******************************************************************************/

int gponDevSetPhyProfile(PLOAM_RAW_Profile_T *pRecvProfMsg)
{
	PHY_Xgpon_Profile_Msg_T xgponPhyProfile;
	memset(&xgponPhyProfile,0,sizeof(xgponPhyProfile));

	xgponPhyProfile.profile_index = pRecvProfMsg->raw.prof_index;
	memcpy(xgponPhyProfile.preamble, pRecvProfMsg->raw.preamble, sizeof(xgponPhyProfile.preamble));
	memcpy(xgponPhyProfile.delimiter, pRecvProfMsg->raw.delimiter, sizeof(xgponPhyProfile.delimiter));
	xgponPhyProfile.preamble_repeat_count = pRecvProfMsg->raw.preamble_repeat_cnt;
	xgponPhyProfile.preamble_length = pRecvProfMsg->raw.preamble_lens;
	xgponPhyProfile.delimiter_length = pRecvProfMsg->raw.delimiter_lens;
	xgponPhyProfile.fec_indication = pRecvProfMsg->raw.fec;

	XPON_PHY_SET_API(PON_SET_PHY_XGPON_PROFILE, &xgponPhyProfile);
	return 0;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetPhyProfile(PHY_Xgpon_Profile_Msg_T *pXgponPhyProfile,unchar index)
{	
	pXgponPhyProfile->profile_index = index;
	XPON_PHY_GET_API(PON_GET_PHY_XGPON_PROFILE, pXgponPhyProfile);
	
	return 0;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetTCCounter(GPON_10G_TC_COUNTER_T *pXgponTcCounter)
{	
	REG_RX_HLEND_HEC_CNT	   rx_hlend_hec_cnt;
	REG_RX_ALLOC_HEC_CNT	   rx_alloc_hec_cnt;
	REG_RX_HDR_HEC_CNT		   rx_hdr_hec_cnt;	
	REG_RX_PHY_HEC_ERR_CNT	   rx_phy_hec_cnt;
	REG_INVLD_PROF_BST_GNT_CNT invld_prof_bst_gnt_cnt;
    REG_TX_XGEM_CNT    tx_xgem_cnt;
	REG_TX_NLF_XGEM_CNT    tx_nlf_xgem_cnt;
    REG_RX_LOST_WCNT           rx_los_wcnt;
    REG_RX_KEY_ERR_CNT         rx_key_err_cnt;
    REG_RX_NON_IDLE_BCNT   rx_non_idle_bcnt;
    REG_TX_NON_IDLE_BCNT   tx_non_idle_bcnt;

	rx_hlend_hec_cnt.Raw =	   IO_GREG(RX_HLEND_HEC_CNT) ;
	pXgponTcCounter->PSBdHECErrCount += rx_hlend_hec_cnt.Bits.rx_hlend_hec_1err_cnt;
	pXgponTcCounter->PSBdHECErrCount += rx_hlend_hec_cnt.Bits.rx_hlend_hec_2err_cnt;
	pXgponTcCounter->PSBdHECErrCount += rx_hlend_hec_cnt.Bits.rx_hlend_hec_3err_cnt;
	pXgponTcCounter->XGTCHECErrCount += rx_hlend_hec_cnt.Bits.rx_hlend_hec_1err_cnt;
	pXgponTcCounter->XGTCHECErrCount += rx_hlend_hec_cnt.Bits.rx_hlend_hec_2err_cnt;
	pXgponTcCounter->XGTCHECErrCount += rx_hlend_hec_cnt.Bits.rx_hlend_hec_3err_cnt;	
		
	rx_alloc_hec_cnt.Raw =	   IO_GREG(RX_ALLOC_HEC_CNT) ;
	pXgponTcCounter->PSBdHECErrCount += rx_alloc_hec_cnt.Bits.rx_alloc_hec_1err_cnt;
	pXgponTcCounter->PSBdHECErrCount += rx_alloc_hec_cnt.Bits.rx_alloc_hec_2err_cnt;
	pXgponTcCounter->PSBdHECErrCount += rx_alloc_hec_cnt.Bits.rx_alloc_hec_3err_cnt;
	
	rx_hdr_hec_cnt.Raw =	   IO_GREG(RX_HDR_HEC_CNT) ;
	pXgponTcCounter->PSBdHECErrCount += rx_hdr_hec_cnt.Bits.rx_hdr_hec_1err_cnt;
	pXgponTcCounter->PSBdHECErrCount += rx_hdr_hec_cnt.Bits.rx_hdr_hec_2err_cnt;
	pXgponTcCounter->PSBdHECErrCount += rx_hdr_hec_cnt.Bits.rx_hdr_hec_3err_cnt;
	pXgponTcCounter->XGEMHECErrCount += rx_hdr_hec_cnt.Bits.rx_hdr_hec_1err_cnt;
	pXgponTcCounter->XGEMHECErrCount += rx_hdr_hec_cnt.Bits.rx_hdr_hec_2err_cnt;
	pXgponTcCounter->XGEMHECErrCount += rx_hdr_hec_cnt.Bits.rx_hdr_hec_3err_cnt;
		
	rx_phy_hec_cnt.Raw =	   IO_GREG(RX_PHY_HEC_ERR_CNT) ;
	pXgponTcCounter->PSBdHECErrCount += rx_phy_hec_cnt.Bits.rx_pon_id_hec_err_cnt;
	pXgponTcCounter->PSBdHECErrCount += rx_phy_hec_cnt.Bits.rx_sfc_hec_err_cnt;

	invld_prof_bst_gnt_cnt.Raw =	IO_GREG(INVLD_PROF_BST_GNT_CNT) ;
	pXgponTcCounter->UnknownProfCount = invld_prof_bst_gnt_cnt.Bits.invld_prof_bst_gnt_cnt;

	tx_xgem_cnt.Raw =		IO_GREG(TX_XGEM_CNT) ;
	pXgponTcCounter->TransmitXGEMFrames = tx_xgem_cnt.Bits.tx_xgem_cnt;

	tx_nlf_xgem_cnt.Raw =		IO_GREG(TX_NLF_XGEM_CNT) ;
	pXgponTcCounter->FragmentXGEMFrames = tx_nlf_xgem_cnt.Bits.tx_nlf_xgem_cnt;

	rx_los_wcnt.Raw =		IO_GREG(RX_LOST_WCNT) ;
	pXgponTcCounter->XGEMHECLostWordCount = rx_los_wcnt.Bits.rx_lost_wcnt;

	rx_key_err_cnt.Raw =	  IO_GREG(RX_KEY_ERR_CNT) ;
	pXgponTcCounter->XGEMKeyErrors = rx_key_err_cnt.Bits.rx_key_err_cnt;

	tx_non_idle_bcnt.Raw =		IO_GREG(TX_NON_IDLE_BCNT) ;
	pXgponTcCounter->TransmitByteNoidleXGEMFrames = tx_non_idle_bcnt.Bits.tx_non_idle_bcnt;	

	rx_non_idle_bcnt.Raw =		IO_GREG(RX_NON_IDLE_BCNT) ;
	pXgponTcCounter->ReceiveByteNoidleXGEMFrames = rx_non_idle_bcnt.Bits.rx_non_idle_bcnt;

	pXgponTcCounter->LODSEventCount = gponTCLODSEvent;
	pXgponTcCounter->LODSEventRestoreCount = gponTCRestoreLODSEvent;
	pXgponTcCounter->ONUReactivLODSEvents = gponTCReactivLODSEvent;
    if (rdk_gtc_dbg.proc_flag)
    {
        pXgponTcCounter->XGTCHECErrCount = rdk_gtc_dbg.hecCounter.XGTCHECErrCount ;
        pXgponTcCounter->PSBdHECErrCount = rdk_gtc_dbg.hecCounter.PSBdHECErrCount ;
        pXgponTcCounter->XGEMHECLostWordCount = rdk_gtc_dbg.hecCounter.XGEMHECLostWordCount ;
        rdk_gtc_dbg.proc_flag = 0;
    }

	return 0;
}
/*****************************************************************************
**		function name: gponDevGetTcontInfo
**		descriptions:
**			 It's used to get tcont index according to allocId
**		parameters:
**			  allocId,tcontIdx
**		global:
**			   None
**		return:
**			   success: 0
**                     fail: EFAULT
**		call:
**			   gponDevGetTCont
**		revision:
**			  None
**
******************************************************************************/
int gponDevGetTcontInfo(ushort allocId, unchar * tcontIdx)
{
	int i;
	GPON_TCONT_t valid;
	ushort id;
	GPON_TCONT_CMD_RESULT_t result;

	for(i = 0 ; i < CONFIG_GPON_10G_MAX_TCONT ; i++) {
		/* read tcont status */
		result = gponDevGetTCont(&valid, i, &id);
		if(result == GPON_TCONT_CMD_SUCCESS) {
			if(id == allocId) {
				*tcontIdx = i;
				return 0;
			}
		} else {
			return -EFAULT ;
		}
	}

	return -EFAULT ;
}


/*****************************************************************************
******************************************************************************/
int gponDevGetDsMgntCounter(GPON_10G_DS_MGNT_COUNTER_T *pXgponDsMgntCounter)
{	
	REG_RX_MIC_ERR_CNT	rx_mic_err_cnt;
	REG_RX_PLOAMD_CNT	rx_ploamd_cnt;

	rx_mic_err_cnt.Raw = IO_GREG(RX_MIC_ERR_CNT) ;
	pXgponDsMgntCounter->PloamMicErrCnt = rx_mic_err_cnt.Bits.rx_ploam_mic_err_cnt;
	pXgponDsMgntCounter->OmciMicErrCnt = rx_mic_err_cnt.Bits.rx_omci_mic_err_cnt;
    if (rdk_mic_err_dbg.proc_micErr_flag)
    {
        pXgponDsMgntCounter->PloamMicErrCnt = rdk_mic_err_dbg.PloamMicErrCount;
        pXgponDsMgntCounter->OmciMicErrCnt = rdk_mic_err_dbg.OmciMicErrCount;
        rdk_mic_err_dbg.proc_micErr_flag = 0;
    }

	rx_ploamd_cnt.Raw = IO_GREG(RX_PLOAMD_CNT) ;
	pXgponDsMgntCounter->DsPloamMsgCnt = rx_ploamd_cnt.Bits.rx_ploamd_cnt;
	pXgponDsMgntCounter->ProfileMsgRx = gpGponPriv->dsPloamCounter[PLOAM_DOWN_MSG_PROFILE];
	pXgponDsMgntCounter->RangeTimeMsgRx = gpGponPriv->dsPloamCounter[PLOAM_DOWN_MSG_RANGING_TIME];
	pXgponDsMgntCounter->DeactOnuIdMsgRx = gpGponPriv->dsPloamCounter[PLOAM_DOWN_MSG_DEACTIVATE_ONUID];
	pXgponDsMgntCounter->DisableSnMsgRx = gpGponPriv->dsPloamCounter[PLOAM_DOWN_MSG_DISABLE_SERIAL_NUM];
	pXgponDsMgntCounter->ReqRegMsgRx = gpGponPriv->dsPloamCounter[PLOAM_DOWN_MSG_REQUEST_REGISTRATION];
	pXgponDsMgntCounter->AssignAllocIdMsgRx = gpGponPriv->dsPloamCounter[PLOAM_DOWN_MSG_ASSIGN_ALLOCID];
	pXgponDsMgntCounter->KeyCtrlMsgRx = gpGponPriv->dsPloamCounter[PLOAM_DOWN_MSG_KEY_CONTROL];
	pXgponDsMgntCounter->SleepAllowMsgRx = gpGponPriv->dsPloamCounter[PLOAM_DOWN_MSG_SLEEP_ALLOW];
	pXgponDsMgntCounter->BaseOmciMsgRx = 0; /*actural value in omci app*/
	pXgponDsMgntCounter->ExtOmciMsgRx = 0;  /*actural value in omci app*/
	pXgponDsMgntCounter->AssignOnuIdMsgRx = gpGponPriv->dsPloamCounter[PLOAM_DOWN_MSG_ASSIGN_ONUID];

	return 0;
}

/*****************************************************************************
******************************************************************************/
int gponDevGetUsMgntCounter(GPON_10G_US_MGNT_COUNTER_T *pXgponUsMgntCounter)
{	
	REG_TX_PLOAMU_CNT	tx_ploamu_cnt;

	tx_ploamu_cnt.Raw = IO_GREG(TX_PLOAMU_CNT) ;
	pXgponUsMgntCounter->UsPloamMsgCnt = tx_ploamu_cnt.Bits.tx_ploamu_cnt;
	pXgponUsMgntCounter->SnOnuMsgCnt = gpGponPriv->usPloamCounter[PLOAM_UP_MSG_SERIAL_NUMBER];
	pXgponUsMgntCounter->RegMsgCnt = gpGponPriv->usPloamCounter[PLOAM_UP_MSG_REGISTRATION];
	pXgponUsMgntCounter->KeyRptMsgCnt = gpGponPriv->usPloamCounter[PLOAM_UP_MSG_KEY_REPORT];
	pXgponUsMgntCounter->AckMsgCnt = gpGponPriv->usPloamCounter[PLOAM_UP_MSG_ACKNOWLEDGE];
	pXgponUsMgntCounter->SleepReqMsgCnt = gpGponPriv->usPloamCounter[PLOAM_UP_MSG_SLEEP_REQUEST];

	return 0;
}

/******************************************************************************
Descriptor:    set gemport upstream AES key mode
Input Args:    none
Ret Value:     none
******************************************************************************/
void gponDevSetUpAesMode(void){
    REG_DBG_CAP_SETTING gponCapSetting;

    if( UPAES_MODE_14BIT == gpGponPriv->gemUpAESMode ){
        /*set default mode to 14 bit mode */
        gponCapSetting.Raw = IO_GREG(DBG_CAP_SETTING);
        gponCapSetting.Bits.us_aes_seq_num_14b = XPON_ENABLE;
        IO_SREG(DBG_CAP_SETTING, gponCapSetting.Raw);
        PON_MSG(MSG_WARN,"set gemport UP AES mode to 128 bit\r\n");
    }else if (UPAES_MODE_128BIT == gpGponPriv->gemUpAESMode){
        /*set default mode to 128 bit ( HW )mode */
        gponCapSetting.Raw = IO_GREG(DBG_CAP_SETTING);
        gponCapSetting.Bits.us_aes_seq_num_14b = XPON_DISABLE;
        IO_SREG(DBG_CAP_SETTING, gponCapSetting.Raw);
        PON_MSG(MSG_WARN,"set gemport UP AES mode to 14 bit\r\n");
    }else if (UPAES_MODE_14BIT_NO_DATA == gpGponPriv->gemUpAESMode){
        /* disable data upstream AES omcc use 14 bit mode*/
        gponCapSetting.Raw = IO_GREG(DBG_CAP_SETTING);
        gponCapSetting.Bits.us_aes_seq_num_14b = XPON_ENABLE;
        IO_SREG(DBG_CAP_SETTING, gponCapSetting.Raw);
        PON_MSG(MSG_WARN,"set Data gemport UP AES disable omcc use 14 bit mode\r\n");
    }else if (UPAES_MODE_128BIT_NO_DATA == gpGponPriv->gemUpAESMode){
        /* disable data upstream AES omcc use 128 bit mode*/
        gponCapSetting.Raw = IO_GREG(DBG_CAP_SETTING);
        gponCapSetting.Bits.us_aes_seq_num_14b = XPON_DISABLE;
        IO_SREG(DBG_CAP_SETTING, gponCapSetting.Raw);
        PON_MSG(MSG_WARN,"set Data gemport UP AES disable omcc use 128 bit mode\r\n");
    }else{
        PON_MSG(MSG_WARN,"set both omcc and Data gemport UP AES disable\r\n");
    }
}


#if defined(TCSUPPORT_CPU_AN7583)
/******************************************************************************
Descriptor:    set bwmap len limit
Input Args:    32 bit reg value
Ret Value:     none
******************************************************************************/
void gponDevSetBwmLenLimit(__u32 bwmap_len_limit){
	IO_SREG(BWMAP_LEN_LIMIT,bwmap_len_limit);
}

/******************************************************************************
Descriptor:    set bwmap chk ctrl
Input Args:    32 bit reg value
Ret Value:     none
******************************************************************************/
void gponDevSetBwmChkCtrl(__u32 bwmap_chk_ctrl){
	IO_SREG(DBG_BWM_CHK_CTRL,bwmap_chk_ctrl);
}
#endif

/*****************************************************************************
******************************************************************************/
void gpon_INT_deinit(void)
{
	REG_INT_ENABLE gponIntEnable ;
	/* Set the INT mask */
	gponIntEnable.Raw = 0 ;
    /* Clear the INT status and enable the INT */
	IO_SREG(INT_STATUS, 0xFFFFFFFF) ;
	IO_SREG(INT_ENABLE, gponIntEnable.Raw) ;
}

/*****************************************************************************
******************************************************************************/
void gpon_INT_init(void)
{

	REG_INT_ENABLE gponIntEnable ;
		
	/* Set the INT mask */
	gponIntEnable.Raw = 0 ;
#if defined(TCSUPPORT_CPU_EN7581)
	gponIntEnable.Bits.cal_gnt_zero_int_en           = 1;
#endif
    gponIntEnable.Bits.o5_sn_onu_req_recv_int_en     = 0;
	gponIntEnable.Bits.turning_sfc_match_int_en      = 1;
	gponIntEnable.Bits.o9_gnt_recv_int_en            = 1;
	gponIntEnable.Bits.sw1_mic_done_int_en           = 0;
	gponIntEnable.Bits.sw0_mic_done_int_en	         = 0;
	gponIntEnable.Bits.key_cal_done_int_en           = 0;
    gponIntEnable.Bits.lwi_int_en                    = 0;
    gponIntEnable.Bits.fwi_int_en                    = 0;
    gponIntEnable.Bits.rx_err_int_en                 = 1;
    gponIntEnable.Bits.tx_err_int_en                 = 1;
    gponIntEnable.Bits.fifo_err_int_en               = 1;
	gponIntEnable.Bits.o5_eqd_adj_done_int_en        = 0;
	gponIntEnable.Bits.bwm_chk_err_int_en            = 1;
	gponIntEnable.Bits.dying_gasp_send_int_en        = 1;
	gponIntEnable.Bits.tod_1pps_int_en               = 0;
	gponIntEnable.Bits.tod_update_done_int_en        = 0;
	gponIntEnable.Bits.olt_ds_fec_chg_int_en         = 0;
	gponIntEnable.Bits.us_prof_idx_chg_int_en        = 0;
	gponIntEnable.Bits.us_key_switch_done_int_en     = 0;
	gponIntEnable.Bits.us_no_msg_send_int_en         = 0;
	gponIntEnable.Bits.o4_registration_send_int_en   = 1;
	gponIntEnable.Bits.o4_ranging_req_recv_int_en    = 1;
	gponIntEnable.Bits.o23_sn_onu_send_int_en        = 1;
	gponIntEnable.Bits.o23_sn_onu_req_recv_int_en    = 1;
	gponIntEnable.Bits.ploamu_send_int_en            = 0;
	gponIntEnable.Bits.ploamd_recv_int_en            = 1;	
   
	/* Clear the INT status and enable the INT */
	IO_SREG(INT_STATUS, 0xFFFFFFFF) ;
	IO_SREG(INT_ENABLE, gponIntEnable.Raw) ;
	/* enable fifo/tx/rx err int */
#if defined(TCSUPPORT_CPU_AN7583)
	IO_SREG(FIFO_ERR_ENABLE, FIFO_ERR_ENABLE_SETTING);
	IO_SREG(TX_ERR_ENABLE, TX_ERR_ENABLE_SETTING);
	IO_SREG(RX_ERR_ENABLE, RX_ERR_ENABLE_SETTING);
#endif	
}
/*****************************************************************************
******************************************************************************/
void gpon_dev_init(void)
{
	XPON_PHY_SET_RX_ENABLE();
	gponDevSetDsFecMode(gpGponPriv->gponCfg.dsFecMode);
	gponDevSetSerialNumber(gpGponPriv->gponCfg.sn);
	gponDevSetRegId(gpGponPriv->gponCfg.reg_id);
    gponDevSetDownstreamOmciMicCtrl(gpGponPriv->gponCfg.dsOmciMicCtrl);
    gponDevSetUpstreamOmciMicCtrl(gpGponPriv->gponCfg.usOmciMicCtrl);
    gponDevSetDyingGaspMode(&gpGponPriv->gponCfg.dyingGasp);
	gponDevSetErrMicPloamDrop(gpGponPriv->gponCfg.ploamMicErrDrop);    
    gponDevResetAesRxKey();
	gponDevSetTxLateStartErrResyncEn(gpGponPriv->gponCfg.txLateResyncEn);
    gponDevSetAesTxKeyInvalid();
	gponDevSetIdleGemThreshold(gpGponPriv->gponCfg.idleGemThreshold);
	gponDevSetMibCounterType(GPON_10G_COUNTER_TYPE_ETHERNET);
#if defined(TCSUPPORT_CPU_AN7583)	
	gponDevSetBwmLenLimit(gpGponPriv->gponCfg.bwmLenLimit);	
	gponDevSetBwmChkCtrl(gpGponPriv->gponCfg.bwmChkCtrl);
#endif
}
