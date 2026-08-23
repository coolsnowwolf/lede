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
#ifndef _GPON_DEV_H
#define _GPON_DEV_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include "gpon/gpon_reg.h"
#include "gpon/gpon_ploam_raw.h"
#include "gpon/gpon.h"
#include <ecnt_hook/ecnt_hook_fe.h>
#include <ecnt_hook/ecnt_hook_pon_phy.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define GPON_REGMSK_GEN_START 	 1
#define GPON_SK_GEN_START 		 2
#define GPON_OMCI_IK_GEN_START   4
#define GPON_PLOAM_IK_GEN_START  8
#define GPON_KEK_GEN_START 	     16

#define XGPON_RSP_TIME 	 		  0x551
#define XGSPON_RSP_TIME 	 	  0x1600
#define XGPON_PREAMBLE_PEPEATE_MASK 	  0x1F
#define XGSPON_PREAMBLE_PEPEATE_MASK 	  0xFF
#define XGPON_GPID_TAB_MAX_ADDR	  2048
#define RETRY_TIMES	  10


#ifdef TCSUPPORT_CPU_ARMV8_64
#define K1_TO_PHYSICAL(x) 				(((__u64)(x)&0x3fffffff)+0x80000000)  //dram_phyAddr_to_dmaAddr(x)
#define CACHE_TO_NONCACHE(addr)			(addr)
#define DMA_ALLOC_MAX_NUM				10
#else
#define K1_TO_PHYSICAL(x) 				((__u32)(x) & 0x1fffffff)  /* kseg1 to physical */
#define CACHE_TO_NONCACHE(addr)			(unchar *)((__u32)(addr) | 0xa0000000)
#define NONCACHE_TO_CACHE(addr)			(unchar *)((__u32)(addr) & 0xdfffffff)
#endif


#define PHYSICAL_TO_K1(x) 				((__u32)(x) | 0xa0000000)  /* physical to kseg1 */

typedef enum {
	GPON_DG_SW = 0,
	GPON_DG_HW,
} GPON_DG_SW_HW_SELECT_t ;

typedef enum {
	GPON_MIC_FAIL_NODROP = 0,
	GPON_MIC_FAIL_DROP,
} GPON_PLOAM_MIC_DROP_SELECT_t ;

typedef enum {
	GPON_ONU_ID_INVALID = 0,
	GPON_ONU_ID_VALID,
} GPON_ONUID_t ;

typedef enum {
	GPON_TCONT_INVALID = 0,
	GPON_TCONT_VALID,
} GPON_TCONT_t ;

typedef enum {
	GPON_TCONT_CMD_SUCCESS = 0,
	GPON_TCONT_CMD_FAIL,
} GPON_TCONT_CMD_RESULT_t ;

typedef enum {
	GPON_TCONT_READ = 0,
	GPON_TCONT_WRITE,
} GPON_TCONT_CMD_t ;

typedef enum {
	GPON_KEY_GEN_SUCCESS = 0,
	GPON_KEY_GEN_FAIL,
} GPON_KEY_GEM_RESULT_t ;

typedef enum {
	GPON_PLOAM_IK_IDX0 = 0,
	GPON_PLOAM_IK_IDX1,
} GPON_PLOAM_IK_INDEX_t ;

typedef enum {
	GPON_OMCI_IK_IDX0 = 0,
	GPON_OMCI_IK_IDX1,
} GPON_OMCI_IK_INDEX_t ;

typedef enum {
	GPON_KEK_IK_IDX0 = 0,
	GPON_KEK_IK_IDX1,
} GPON_KEK_IK_INDEX_t ;

typedef enum {
	GPON_AES_UC_FIRST_KEY = 1,
	GPON_AES_UC_SECOND_KEY,
	GPON_AES_BC_FIRST_KEY,
	GPON_AES_BC_SECOND_KEY,
} GPON_AES_KEY_INDEX_t ;

typedef enum {
	GPON_SMA_INVALID = 0,
	GPON_SMA_VALID,
} GPON_SMA_t ;

typedef enum {
	GPON_REG_ID_NOT_REPORT = 0,
	GPON_REG_ID_REPORTED,
} GPON_REGISTER_ID_STATE_t ;

typedef enum {
	GPON_UC_KEY_GEN = 0,
	GPON_UC_KEY_CONFIRM,
} GPON_UC_KEY_CTRL_t ;

typedef enum {
	GPON_CMAC_PLOAM_IDX0 = 0,
	GPON_CMAC_PLOAM_IDX1,
	GPON_CMAC_OMCI_IDX0,
	GPON_CMAC_OMCI_IDX1,
	GPON_CMAC_KEK_IDX0,
	GPON_CMAC_KEK_IDX1,
	GPON_CMAC_NORMAL_ENCRYPT =7,	
} GPON_CMAC_KEY_INDEX_t ;

typedef enum {
	GPON_CMAC_NORMAL = 0,	
	GPON_CMAC_DOWNSTREAM,
	GPON_CMAC_UPSTREAM,
} GPON_CMAC_PAD_DIR_t ;

typedef enum {
	GPON_NORMAL_COUNTER_CLEAR = 0,
	GPON_ERR_COUNTER_CLEAR,
} GPON_COUNTER_CLEAR_t ;

typedef enum {
	GPON_CMAC_IDX0 = 0,
	GPON_CMAC_IDX1,
} GPON_CMAC_INDEX_t ;

typedef enum{
    GEMPORT_RX_FRAME_CNT = 0,
    GEMPORT_RX_PL_BYTE_CNT,
    GEMPORT_TX_FRAME_CNT,
    GEMPORT_TX_PL_BYTE_CNT,
}GPON_GEMPORT_STATS_TYPE_t;

typedef enum {
    XPON_RESET_HOLD_ON = 0,
    XPON_RESET_RELEASE,
} XPON_RESET_MODE_t;

typedef enum {
    XPON_WITHOUT_GDM2CDM2_STOP = 0,
    XPON_WITH_GDM2CDM2_STOP,
} XPON_GDM2CDM2_STOP_FLAG_t;

typedef enum{
    GPON_SW_RESYNC_DISABLE = 0,
    GPON_SW_RESYNC_ENABLE,
}GPON_SW_RESYNC_STATUS_t;

typedef enum{
    GPON_SW_NOT_RESYNC = 0,
    GPON_SW_RESYNC,
}GPON_SW_RESYNC_CTRL_t;

typedef struct {	
    uint      rx_mpi_sof_cnt;
    uint      tx_mpi_sof_cnt;
    uint      rx_ploamd_cnt;
    uint      tx_ploamu_cnt;
    uint      rx_omci_cnt_fe;
	uint      tx_omci_cnt_fe;
	uint      rx_omci_cnt_mac;
	uint      tx_omci_cnt_mac;	
    uint      rx_xgem_cnt;
	uint      tx_xgem_cnt;
	uint      rx_mbi_ack_cnt;
	uint      tx_mbi_ack_cnt;
	uint      rx_non_idle_bcnt;
	uint      tx_non_idle_bcnt;	

    uint      rx_hlend_hec_1err_cnt;
    uint      rx_hlend_hec_2err_cnt;
    uint      rx_hlend_hec_3err_cnt;

    uint      rx_alloc_hec_1err_cnt;
    uint      rx_alloc_hec_2err_cnt;
    uint      rx_alloc_hec_3err_cnt;
    
    uint      rx_hdr_hec_1err_cnt;
    uint      rx_hdr_hec_2err_cnt;
    uint      rx_hdr_hec_3err_cnt;
    
	uint      rx_pon_id_hec_err_cnt;
	uint      rx_sfc_hec_err_cnt;
	
	uint      rx_omci_mic_err_cnt;
	uint      rx_ploam_mic_err_cnt;
	
	uint      rx_eth_crc_err_cnt;
	uint      rx_bip_err_cnt;
	uint      rx_key_err_cnt;
	uint	  rx_lost_wcnt;
	uint	  invld_prof_bst_gnt_cnt;
	uint	  rx_mbi_xgem_drop_cnt;
	
	uint	  tx_nlf_xgem_cnt;
	uint	  tx_ack_ploamu_cnt;
	uint      rx_bip_protect_wcnt;
	uint      tx_idle_bcnt;
	GPON_COUNTER_CLEAR_t clear;
} GPON_DEV_NORMAL_COUNTER_T;


typedef struct
{
	ushort gemId;
	unchar valid;
	unchar type;
	unchar encrypt;
}GEM_INFO_T;


/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
void gpon_INT_deinit(void);
int  gponDevGetPloamMsg(PLOAM_RAW_General_T *pPloamMsg) ;
int  gponDevTxMbiStop(XPON_RESET_MODE_t mode);
int  gponDevMbiStop(XPON_RESET_MODE_t mode, XPON_GDM2CDM2_STOP_FLAG_t flag) ;
int  gponDevMpiStop(XPON_RESET_MODE_t mode);
int  gponDevMpiTxStop(XPON_RESET_MODE_t mode);
int gponDevMpiRxStop(XPON_RESET_MODE_t mode);
int  gponDevSetSerialNumber(unchar *sn) ;
void gponDevSetOnuId(uint onuId, unchar valid) ;
void gponDevSetPonTag(unchar *ponTag) ;
void gponDevSetRegId(unchar *regId);
void gponDevSetProfileInfo(unchar profIndex, uint profLens, unchar version);
int  gponDevKeyGenStart(uint keyType);
void gponDevGetHwGenKey(uint *key);
int  gponDevDisableTCont(ushort allocId);
int  gponDevEnableTCont(ushort allocId);
int  gponDevDumpTcontInfo(void);
int gponDevAssignNewAllocId(unsigned long data);
#if defined(TCSUPPORT_CPU_AN7583)
void set_FE_CHN_RETIRE_DONE(TIMER_FUN_PAAM arg);
#endif
void gponDevResetGemInfo(void) ;
int  gponDevGetGemInfo(ushort gemPortId, unchar *pValid, unchar *pType,unchar *pEncrypted);
int  gponDevDumpGemInfo(void);
int  gponDevSetGemInfo(ushort gemPortId, unchar isValid, unchar isType,unchar isEncrypted);
void gponDevSetAesUcKey0(unchar *key);
void gponDevClearAesUcKey0(void);
void gponDevSetAesUcKey1(unchar *key);
void gponDevClearAesUcKey1(void);
void gponDevSetAesBcKey0(unchar *key);
void gponDevCleanAesBcKey0(void);
void gponDevSetAesBcKey1(unchar *key);
void gponDevCleanAesBcKey1(void);
void gponDevSetPloamIkIdx(unchar index);
void gponDevGetPloamIkIdx(unchar *index);
void gponDevSetOmciIkIdx(unchar index);
void gponDevGetOmciIkIdx(unchar *index);
void  gponDevIntegrityKeySet(unsigned long arg);

int  gponDevKeySetOmciBasedMSK(GPON_Security_t *gponSecurity);
int  gponDevSetCmac0Start(GPON_CMAC_KEY_INDEX_t keyIdx, GPON_CMAC_PAD_DIR_t dir, unchar *key,
		unchar *msgAddr, uint msgLen, unchar *resultAddr, unchar *resultAddr_vir, uint resultLen);
int  gponDevSetCmac1Start(GPON_CMAC_KEY_INDEX_t keyIdx, GPON_CMAC_PAD_DIR_t dir, unchar *key,
		unchar *msgAddr, uint msgLen, unchar *resultAddr, unchar *resultAddr_vir, uint resultLen);

int gponDevSetAesRxKeyValid(unchar index);
int gponDevSetAesRxKeyInvalid(unchar index);
void gponDevResetAesRxKey(void);
void gponDevSetAesTxKeyValid(unchar index);
void gponDevSetAesTxKeyInvalid(void);
void gponDevUnicastKeyExchange(unsigned long data);

int  gponDevGetTxSyncRdy(void);
void gponDevRangingAck(unsigned long data);
int gponDevRangingAck_test(unchar seqNo);
void gponDevSetO23O4PloamCtrl(XGPON_SW_HW_SELECT_T mode);
void gponDevGetO23O4PloamCtrl(XGPON_SW_HW_SELECT_T *mode);
void gponDevSetEqd(uint eqd);
#if !defined(TCSUPPORT_CPU_AN7583) 
int gponDevSetEqdValue(uint newEqd_eqd_olt_absolute,uint eqd_olt_init ,unchar wanMode);
void gponDevRefreshEqdValue(void);
#else
int gponDevSetEqdValue(uint newEqd,uint * recordEqd,unchar wanMode,unchar adjustMode,unchar adjustDir);
#endif
int feDevGdm2Cdm2Stop(XPON_RESET_MODE_t mode);


int  gponDevSetSniffMode(GPON_10G_DEV_SNIFFER_MODE_T *sniffer);
void gponDevGetSniffMode(GPON_10G_DEV_SNIFFER_MODE_T *sniffer);
void gponDevGetNormalCounter(GPON_DEV_NORMAL_COUNTER_T *counter);
void gponDevGetErrCounter(GPON_DEV_NORMAL_COUNTER_T *counter);
void gponDevSetCounterClear(GPON_DEV_NORMAL_COUNTER_T *counter);
void gponDevGetErrStatus(GPON_DEV_ERR_STATUS_T *status);
void gponDevSetErrStatusClear(void);
int  gponDevSetPloamFilterMode(GPON_10G_DEV_PLOAMD_FILTER_MODE_T *filter);
void gponDevGetPloamFilterMode(GPON_10G_DEV_PLOAMD_FILTER_MODE_T *filter);
void gponGetNGPonMode(uint *mode);
void gponDevSetRspTime(ushort time);
void gponDevGetRspTime(ushort *time);
int gponDevSetErrMicPloamDrop(XPON_Mode_t mode);
void gponDevGetErrMicPloamDrop(XPON_Mode_t *mode);
int gponDevSetDyingGaspMode(GPON_10G_DYING_GASP_MODE_T *arg);
int gponDevGetDyingGaspMode(GPON_10G_DYING_GASP_MODE_T *arg);
int gponDevSetDbaBackdoorMode(GPON_10G_DEV_DBA_BACKDOOR_T *dba);
void gponDevGetDbaBackdoorMode(uint *dbaMode,uint *modifyValue);
#ifdef TCSUPPORT_CPU_AN7583
int gponDevSetDbaBackdoorModeSeperate(GPON_10G_DEV_DBA_BACKDOOR_SEPERATE_T *dba);
void gponDevSetBwmLenLimit(__u32 bwmap_len_limit);
void gponDevSetBwmChkCtrl(__u32 bwmap_chk_ctrl);
#endif
void gponDevSetDownstreamOmciMicCtrl(XGPON_SW_HW_SELECT_T ctrl);
void gponDevSetUpstreamOmciMicCtrl(XGPON_SW_HW_SELECT_T ctrl);
void gponDevSetMibCounterType(GPON_10G_COUNTER_TYPE_t type);
void gponDevGetMibCounterType(GPON_10G_COUNTER_TYPE_t *type);
void gponDevGetOmciMicCtrl(GPON_10G_DEV_OMCI_MIC_CTRL_T *omciMicCtrl);
int gponDevGemMibTablesInit(void);
int gponDevUpdateGemMibIdxTable(ushort addr, ushort data);
int gponDevClearGemPortCounter(ushort gemPortId);
int gponDevClearHwCounter(void);
int gponDevGetGemPortCounter(ushort gemPortId, GPON_GEMPORT_STATS_TYPE_t type, uint64_t *pData);
void gponDevSwReplyRegistrationPloam(unsigned long arg);
void gponDevSwResync(void);
int gponDevTxSyncRdy(void);
void gponDevClearCntAndErr(void);
int gponDevWaitTxAlignFifoUnused(void);
int gponDevSwResyncCompleteProcess(void);
void gponDevSetPhyDsFecMde(GPON_10G_DEV_DS_FEC_MODE_T Mode);

#ifdef TCSUPPORT_CPU_EN7581
void gponDevGetRxIdleXgem(int getRxIdXg);
int gponDevSetNewTod(XMCS_GponTodCfg_t new_tod) ; 
#endif
int gponDevCheckGemInfoInvalid(ushort gemPortId);

int gponDevcheckGemInfo(GEM_INFO_T *pSwGemInfo, GEM_INFO_T *pRdGemInfo);
int gponDevGetSwGemInfo(ushort gemId, GEM_INFO_T *pSwGemInfo);


int gponDevSetPhyProfile(PLOAM_RAW_Profile_T *pRecvProfMsg);
int gponDevGetPhyProfile(PHY_Xgpon_Profile_Msg_T *pXgponPhyProfile,unchar index);
void gponDevResetCtrl(XPON_RESET_MODE_t mode);
void gpon_INT_init(void);
int gponDevGetGemMibIdxTable(ushort addr, uint *data);
void gponDevSetPloamIk0(unchar *key);
void gponDevSetPloamIk1(unchar *key);
void gponDevSetKEK0(unchar *key);
void gponDevSetKEK1(unchar *key);
void gponDevSetOmciIk0(unchar *key);
void gponDevSetOmciIk1(unchar *key);
int gponDevGetTCCounter(GPON_10G_TC_COUNTER_T *pXgponTcCounter);
int gponDevGetTcontInfo(ushort allocId, unchar * tcontIdx);
int gponDevGetDsMgntCounter(GPON_10G_DS_MGNT_COUNTER_T *pXgponDsMgntCounter);
int gponDevGetUsMgntCounter(GPON_10G_US_MGNT_COUNTER_T *pXgponUsMgntCounter);
void gponDevSetUpAesMode(void);
GPON_TCONT_CMD_RESULT_t gponDevSetTCont(GPON_TCONT_t isValid, int tcont_index, ushort allocId);
void gponDevMacReset(XPON_RESET_MODE_t mode);
void gponDevSetDsFecMode(GPON_10G_DEV_DS_FEC_MODE_T Mode);


void gpon_dev_init(void);

#endif /*_GPON_DEV_H*/
