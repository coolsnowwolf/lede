#ifndef _GPON_DEV_H_
#define _GPON_DEV_H_

#include "gpon/gpon_reg.h"
#include "gpon/gpon_ploam_raw.h"

#ifdef __KERNEL__
#include <ecnt_hook/ecnt_hook_pon_phy.h>
#endif

#include <xpon_global/private/xmcs_gpon.h>

typedef struct {
	ushort		gemPortId ;
	unchar		isValid ;		//0:invalid, 1:valid
	unchar		isEncrypted ;	//0:not encrypted, 1:encrypted
} GPONDEV_GemPortInfo_T ;

typedef struct {
	REG_G_AES_ACTIVE_KEY0 aesActiveKey0;	// 4064
	REG_G_AES_ACTIVE_KEY1 aesActiveKey1;	// 4068
	REG_G_AES_ACTIVE_KEY2 aesActiveKey2;	// 406C
	REG_G_AES_ACTIVE_KEY3 aesActiveKey3;	// 4070
} GPON_DEV_AES_ACTIVE_KEY_T;

typedef struct {
	REG_G_AES_SHADOW_KEY0 aesShadowKey0;	// 4074
	REG_G_AES_SHADOW_KEY1 aesShadowKey1;	// 4078
	REG_G_AES_SHADOW_KEY2 aesShadowKey2;	// 407C
	REG_G_AES_SHADOW_KEY3 aesShadowKey3;	// 4080
} GPON_DEV_AES_SHADOW_KEY_T;

typedef enum {
	GPON_TCONT_INVALID = 0,
	GPON_TCONT_VALID,
} GPON_TCONT_t ;

typedef enum {
	GPON_TCONT_CMD_SUCCESS = 0,
	GPON_TCONT_CMD_FAIL,
} GPON_TCONT_CMD_RESULT_t ;


typedef enum{
    GEMPORT_RX_FRAME_CNT = 0,
    GEMPORT_RX_PL_BYTE_CNT,
    GEMPORT_TX_FRAME_CNT,
    GEMPORT_TX_PL_BYTE_CNT,
}GPON_GEMPORT_STATS_TYPE_t;

#ifdef TCSUPPORT_CPU_EN7521
typedef enum {
	GPON_TCONT_READ = 0,
	GPON_TCONT_WRITE,
} GPON_TCONT_CMD_t ;

typedef enum {
	GPON_AES_KEY_SWITCH_BY_SW_DONE_PROCESSING = 0,
	GPON_AES_KEY_SWITCH_BY_SW_DONE,
} GPON_AES_KEY_SWITCH_BY_SW_DONE_t ;

#define GPON_AES_KEY_SWITCH_BY_SW_CMD_SET	(1)
#define GPON_DEFAULT_DBA_BLOCK_SIZE			(48)

/* FILTER PLOAM */
#define GPON_FILTER_US_OVERHEAD_IN_O5		(1 << 0)
#define GPON_FILTER_EXT_BST_LEN_IN_O5		(1 << 8)

/* SW resync */
#define GPON_SW_RESYNC_DISABLE				(0)
#define GPON_SW_RESYNC_ENABLE				(1)
#define GPON_SW_NOT_RESYNC					(0)
#define GPON_SW_RESYNC						(1)

/* dba slight modify */
#define GPON_DBA_SLIGHT_MODIFY_DISABLE		(0)
#define GPON_DBA_SLIGHT_MODIFY_ENABLE		(1)

/* GPON MAC SW reset */
#define GPON_MAC_SW_RESET_INTERVAL_US		(1)

/* GPON MAC SW resync*/
#define GPON_MAC_SW_RESYNC_INTERVAL_US		(1)

/* GPON MAC Tx sync opt */
#define GPON_MAC_TX_SYNC_OPT_O4				(1)
#define GPON_MAC_TX_SYNC_OPT_BOTH_O4_O5		(0)

/* GPON MAC SW reset */
#define GPON_MAC_SW_RESET_ENABLE			(0)
#define GPON_MAC_SW_RESET_DISABLE			(1)
#endif

#ifndef TCSUPPORT_CPU_EN7521
#define GPON_MAC_RESET_BIT    (4)
#define PHY_DIGITAL_RESET_BIT (2)
#define PHY_ANALOG_RESET_BIT  (1)

#define MAC_4208_PHY_RX_FIX   (8)
#define MAC_4208_PHY_RX_DLY   (4)
#define MAC_4208_INTERNAL_DLY (2)
#define MAC_4208_PHY_TX_DLY   (1)
#endif /* TCSUPPORT_CPU_EN7521 */

#define GPON_RDI_TASK_DISABLE   ( 0 )
#define GPON_RDI_TASK_ENABLE    ( 1 )
#define GPON_RDI_TASK_INIT      ( 2 )
#define GPON_RDI_TASK_KILL      ( 3 )

#ifdef __KERNEL__
void select_xpon_wan(Xpon_Phy_Mode_t mode);
#endif

int gpon_dev_init(void) ;
int gponDevGetPloamMsg(PLOAM_RAW_General_T *pPloamMsg) ;
int gponDevSendPloamMsg(PLOAM_RAW_General_T *pPloamMsg, uint times) ;
int gponDevSet1PPSInt(XPON_Mode_t mode) ;
int gponDevSetNewTod(uint superframe, uint sec, uint nanosec) ; 
int gponDevGetCurrentTod(uint *pSec, uint *pNanosec) ;
int gponDevGetNewTod(uint *pNewSec, uint *pNewNanoSec);
int gponDevGetTodSwitchTime(uint *spf);
int gponDevEnable1PPSInt(XPON_Mode_t mode) ;
int gponDevSetSerialNumber(unchar *sn) ;
void gponDevResetGemInfo(void) ;
int gponDevGetSuperframe(uint *counter);
int gponDevGetGemInfo(ushort gemPortId, unchar *pValid, unchar *pEncrypted) ;
int gponDevSetGemInfo(ushort gemPortId, unchar isValid, unchar isEncrypted) ;
int gponDevDumpGemInfo(void);
int gponDevGemMibTablesInit(void) ;
int gponDevUpdateGemMibIdxTable(unchar addr, ushort data) ;
int gponDevGetGemPortCounter(ushort gemPortId, GPON_GEMPORT_STATS_TYPE_t type, uint *pHData, uint *pLData) ;
void gponDevResetAllocId(void) ;
int gponDevIsChannelValid(unchar channel) ;
void gpon_dev_reset_GPON_MAC(void);
#ifdef TCSUPPORT_CPU_EN7521
int gponDevSetSniffMode(GPON_DEV_SNIFFER_MODE_T *sniffer);
GPON_AES_KEY_SWITCH_BY_SW_DONE_t gponDevSetAesKeySwitchBySw(void);
int gponDevSwReset(void);
int gponDevSwResync(void);
int gponDevSetDbaBackdoor(XPON_Mode_t enable);
int gponDevSetDbaBackdoorTotal(uint total_buf_size);
int gponDevSetDbaBackdoorGreen(uint green_size);
int gponDevSetDbaBackdoorYellow(uint yellow_size);
int gponDevGetDbaBackdoor(GPON_DEV_DBA_BACKDOOR_T *dba_backdoor);
int gponDevSetDbaSlightModify(XPON_Mode_t enable);
int gponDevSetDbaSlightModifyTotal(ushort total_size);
int gponDevSetDbaSlightModifyGreen(ushort green_size);
int gponDevSetDbaSlightModifyYellow(ushort yellow_size);
int gponDevGetDbaSlightModify(GPON_DEV_SLIGHT_MODIFY_T *dba_slight_modify);
int gponDevSetDbaShiftModify(XPON_Mode_t enable);
int gponDevSetDbaShiftModifyTotal(struct XMCS_DBAShiftMod_S *gponShiftTotal);
int gponDevSetDbaShiftModifyGreen(struct XMCS_DBAShiftMod_S *gponShiftGreen);
int gponDevSetDbaShiftModifyYellow(struct XMCS_DBAShiftMod_S *gponShiftYellow);
int gponDevSetBwmStopTimeInt(XPON_Mode_t enable) ;
int gponDevSetTx4bytesAlign(XPON_Mode_t enable);
int gponDevGetTx4bytesAlign(XPON_Mode_t *enable);
int gponDevGetTxSync(unchar *tx_sync);
int gponDevGetO3O4PloamCtrl(GPON_SW_HW_SELECT_T *sel);
int gponDevSetO3O4PloamCtrl(GPON_SW_HW_SELECT_T sel);
int gponDevSetFilterUpstreamOverheadPLOAM(XPON_Mode_t enable);
int gponDevGetFilterUpstreamOverheadPLOAM(XPON_Mode_t *enable);
int gponDevSetFilterExtBurstLengthPLOAM(XPON_Mode_t enable);
int gponDevGetFilterExtBurstLengthPLOAM(XPON_Mode_t *enable);
int gponDevSet1ppsHighWidth(uint width);
int gponDevGet1ppsHighWidth(uint *width);
int gponDevSetSendPloamuWaitMode(GPON_DEV_SEND_PLOAMU_WAIT_MODE_T mode);
int gponDevGetSendPloamuWaitMode(GPON_DEV_SEND_PLOAMU_WAIT_MODE_T *mode);
#endif
int gponDevSetTodClkPeriod(ushort period);
int gponDevSetIntMask(uint mask);
int gponDevGetIntMask(uint *mask);
GPON_TCONT_CMD_RESULT_t gponDevSetTCont(GPON_TCONT_t isValid, int tcont_index, ushort allocId);
GPON_TCONT_CMD_RESULT_t gponDevGetTCont(GPON_TCONT_t *isValid, int tcont_index, ushort *allocId);
int gponDevDisableTCont(ushort allocId) ;
int gponDevEnableTCont(ushort allocId) ;
int gponDevDumpTcontInfo(void);
int gponDevDumpCsr(void);
void gpon_dev_init_reset(void) ;
int gponDevDeactiveOnu(void) ;
int gponDevSetEncryptKey(unchar *aesKey) ;
int gponDevGetEncryptKey(GPON_DEV_ENCRYPT_KEY_INFO_T * aesKey);
int gponDevSetKeySwithTime(uint counter) ;
int gponDevSetDBABlockSize(ushort blockSize) ;
int gponDevGetDBABlockSize(ushort *blockSize);
int gponDevHardwareDyingGasp(GPON_SW_HW_SELECT_T mode) ;
int gponDevGetDyingGaspMode(GPON_SW_HW_SELECT_T *mode);
int gponDevSetDyingGaspNum(uint num);
int gponDevGetDyingGaspNum(uint *num);
int gponDevSetIdleGemThreshold(ushort idle_gem_thld);
int gponDevgetIdleGemThreshold(ushort *idle_gem_thld);
int gponDevSetCounterType(GPON_COUNTER_TYPE_t type);
int gponDevGetCounterType(GPON_COUNTER_TYPE_t *type);
int gponDevSetResponseTime(ushort time);
int gponDevGetResponseTime(ushort *time);
int gponDevSetInternalDelayFineTune(unchar delay);
int gponDevGetInternalDelayFineTune(unchar *delay);
int gponDevClearSwCounter(void);
int gponDevClearHwCounter(void);
int gponDevMbiStop(XPON_Mode_t mode) ;
int gponDevCheckTContReg(ushort channelId, ushort allocId);

#endif /* _GPON_DEV_H_ */
