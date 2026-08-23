#ifndef _GPON_H_
#define _GPON_H_


#include <common/xpondrv.h>
#include <gpon/gpon_const.h>
#include <gpon/gpon_ploam.h>
#include <gpon/gpon_act.h>
#include <gpon/gpon_dev.h>
#include <gpon/gpon_qos.h>
#include <xmcs/xmcs_fdet.h>
#include "gpon/gpon_recovery.h"

#include <linux/hardirq.h>
#ifdef TCSUPPORT_CPU_EN7521
#include <ecnt_hook/ecnt_hook_qdma.h>
#endif
#include <ecnt_hook/ecnt_hook_pon_mac.h>
extern unsigned int gTypeOfAction;


typedef void (*gponTimerCallback)(TIMER_FUN_PAAM);
int gpon_create_timer(struct timer_list *timer, gponTimerCallback callback, unsigned long param);
#define GPON_CREATE_TIMER(timer,func,para) gpon_create_timer(timer,func,para)

		/* del_timer_sync cannot be used from interrupt context. */
#define GPON_STOP_TIMER(timer)	\
			{ \
				if(in_interrupt()) { \
					del_timer(&timer) ; \
				} else {\
					del_timer_sync(&timer) ; \
				} \
			}
//#define GPON_START_TIMER(timer)			{ timer.expires = jiffies + ((timer.data*HZ)/1000) ; add_timer(&timer) ; }
#define GPON_START_TIMER(timer,para)  { timer.expires = para; mod_timer(&timer, (jiffies + ((timer.expires*HZ)/1000))) ; }
#define SCU_DYING_GASP_STATUS			(1<<16)

typedef struct {
    REG_G_TCONT_ID_0_1     gponTCont0 ;
    REG_G_TCONT_ID_2_3     gponTCont2 ;
    REG_G_TCONT_ID_4_5     gponTCont4 ;
    REG_G_TCONT_ID_6_7     gponTCont6 ;
    REG_G_TCONT_ID_8_9     gponTCont8 ;
    REG_G_TCONT_ID_10_11   gponTCont10 ;
    REG_G_TCONT_ID_12_13   gponTCont12 ;
    REG_G_TCONT_ID_14_15   gponTCont14 ;
} GPON_TCONT_T;

#define GPON_DG_DEFAULT_DELAY_INTERVAL		250
#define GPON_DG_ONEBYONE_TOTAL_CNT			3

typedef enum {
    SET_LIMIT    = 0x0,
    RECOVER_LIMIT   = 0x1,
} ENUM_LIMIT_TYPE;

typedef enum {
    GPON_DG_NORMAL		 ,
    GPON_DG_DELAY_US    ,
    GPON_DG_SEND_ONEBYONE,
} GPON_DYING_GASP_OPT_e;

typedef struct{
    unchar 		opt_type;
    unchar		total_send_cnt;
    unchar		current_send_cnt;
    unchar		dying_flag;
    uint		delay_time;
} GPON_DYING_GASP_INFO_T, *GPON_DYING_GASP_INFO_P;

extern GPON_DYING_GASP_INFO_P gp_dying_info;

typedef struct {
	unchar 					onu_id ;	/* ONU ID, the default is GPON_UNASSIGN_ONU_ID */
	unchar					sn[GPON_SN_LENS] ;	/* onu serial number, the default is MTKG00000001 */
	unchar                  PasswdLength;
	unchar					hexFlag;
    unchar                  emergencystate;
	unchar 					passwd[GPON_PASSWD_LENS] ;  
	unchar 					keyIdx ;
	unchar					key[GPON_ENCRYPT_KEY_LENS] ;
	ushort					omcc ;		/* omci channel gem port id */
	unchar					omccGemEnCry;/* omci channel gem encryption flag*/
	unchar					reiSeq ; 	/* rei ploam message sequence */
	uint					to1Timer ;	/* o1 timer time for onu activation */
	uint					to2Timer ;	/* o2 timer time for onu activation */
	uint 					hardware_timer; /* hardware timer for dev update hardware cnts */
	uint					berInterval ; /* BER interval from OLT, the uint is ms */
	uint					silenceInterval ; /* Silence interval the uint is ms */
	uint					eqd ; 		/* keep the latest eqd value */
	uint					byteDelay ; /* keep the latest MAC byte delay value */
	unchar					bitDelay ;  /* keep the latest PHY bit delay value */
	unchar 					eqdO4Offset ;   /* O4 eqd Offset */
	unchar 					eqdO5Offset ;   /* O5 eqd Offset */
	unchar					t3PreambleOffset ;
	unchar					dvtPcpCheck ;
	uint					dvtPcpCounter ;
	uint					dvtPcpCheckErr ;
	ushort					onuResponseTime;
	unchar					internalDelayFineTune ;
    unchar                  dis_ranging_in_o5;
	ushort					idle_gem_thld;
	GPON_COUNTER_TYPE_t		counter_type;
    unchar					tx_power;
	unchar					preamble_t3_pat;
    unchar                  phy_guard_bit_num;
#ifdef TCSUPPORT_CPU_EN7521
	uint					sleep_count;
	uint 					phy_psync_to_sof_delay;
	GPON_DEV_SNIFFER_MODE_T sniffer_mode;
	/* wan cpu protection */
	QDMA_RxRateLimitCfg_T	wan_rxRateLimitCfg;			/* bfb55120 */
	QDMA_RxRateLimitGet_T	wan_rxRateLimit;			/* bfb55124 */
	/* lan cpu protection */
	QDMA_RxRateLimitCfg_T	lan_rxRateLimitCfg;			/* bfb45120 */
	QDMA_RxRateLimitGet_T	lan_rxRateLimit;			/* bfb45124 */
#else /* TCSUPPORT_CPU_EN7521 */
	uint					reg4208;
	unchar					rstDbgDly;
	unchar					macPhyReset; 
#endif

    unchar                  sr_blk_size;
    unchar                  consid_tx_rx_ph;
    unchar                  sw_reset_flag;
    GPON_TCONT_T            tconts;
    GPON_DYING_GASP_INFO_T	dying_info;
	REG_G_PLOu_GUARD_BIT    gponGuardBit ;
	REG_G_PLOu_DELM_BIT     gponDelmBit ;
	REG_G_PLOu_PRMBL_TYPE1_2 gponPrmblType ;
    REG_G_PLOu_PRMBL_TYPE3  gponT3Prmbl ;
    REG_G_PRE_ASSIGNED_DLY  gponPreAssignDly ;
	ushort					rdiSendNum;
	unchar					rdiErrCtrlEn;
	uint					rdiErrCtrlThreshold;

	struct {
		unchar				isRequestKey		: 1 ;
		unchar				isTodUpdate 		: 1 ;
		unchar				dvtGponLosFlag		: 1 ;
		unchar 				eqdOffsetFlag		: 1 ;		/* eqd offset flag 1--neg*/
		unchar				preambleFlag		: 1 ;
		unchar				sniffer				: 1 ;		
		unchar				hotplug				: 1 ;
        unchar              maxRdmDlyFlag       : 1 ;
#ifdef TCSUPPORT_CPU_EN7521
		unchar				isPloamFilter		: 1 ;
		unchar				isBwmStopTimeErrInt	: 1 ;
#endif
	} flags ;

	struct
	{
		char				cage[8];
		char				moduleVendor[GPON_MODULE_VENDOR_LEN];
		char				moduleName[GPON_MODULE_NAME_LEN];
		char				moduleVersion[GPON_MODULE_VERSION_LEN];
		char				moduleFwVersion[GPON_MODULE_FW_VERSION_LEN];
		char				connector[24];
		char				status[GPON_MODULE_STATUS_LEN];
	}phyMedModule;
} GPON_Config_T ;

typedef enum {
	TRAFFIC_DOWN = 0,
	TRAFFIC_UP
} Traffic_Status_t ;
/** GPON_GlbPriv_T means "GPON Global Private data".
 */
typedef struct {
	unsigned int            bip_cnt_val;
	ENUM_GponState_t 		state ;
	struct timer_list		to1_timer ;
	struct timer_list		to2_timer ;
	struct timer_list		hardware_timer ;
	struct timer_list		ber_timer ;
	struct timer_list		silence_timer ;
	uint 					gpon_silence;
	Traffic_Status_t	gpon_traffic_status;
	struct tasklet_struct   gpon_dev_reset;
	struct tasklet_struct   gpon_channel_retire;
#if !defined(TCSUPPORT_CPU_EN7523) && !defined(TCSUPPORT_CPU_EN7581)
	struct tasklet_struct   gpon_check_rx_loss;
#endif
	atomic_t                retire_flag;
	
	ploam_recv_handler_t	ploamRecvHandler[PLOAM_DOWN_MAX_TYPE] ;
	PLOAM_RAW_General_T		prePloamMsg ;
	
	GPON_Config_T			gponCfg ;
	
	spinlock_t				act_lock ;			/* spin lock for GPON activation */
#ifdef TCSUPPORT_CPU_EN7521
	spinlock_t				swReset_lock ;
#endif
    spinlock_t				mac_reset_lock ;
    unchar                  typeBOnGoing     : 1 ;
	unchar	                disableSnFlag    : 1 ;
    unchar                  emergencystate   : 1 ;
	unchar                  lofEnableFlag    : 1 ;
	struct Gpon_Recovery_S* pGponRecovery;
	unsigned int            gTypeOfHotplug;
	GPON_Alarm_T			gponAlarm;
	int           			gponAlmKeepTime;
	unsigned long 			jiffiesClk[GPON_ALARM_MAX_NUM];
	unsigned int			activationCnt;
	GPON_PLOAM_CNT_T		ploamMsgcounter;
} GPON_GlbPriv_T ;

typedef enum {
    GPON_DEV_PLAIN_RESET          ,
    GPON_DEV_RESET_WITH_FE_RESET ,
    GPON_DEV_RESET_WITH_O7_RESET ,
} GPON_DEV_RESET_TYPE_t;

typedef enum {
	RDI_INDEX	,
	PEE_INDEX	,
	LOS_INDEX	,
	LOF_INDEX	,
	DIS_INDEX	,
	DACT_INDEX	,
	MIS_INDEX	,
	MEM_INDEX	,
	SUF_INDEX	,
	SF_INDEX	,
	SD_INDEX	,
	LCDG_INDEX	,
	TF_INDEX	,
	ROGUE_INDEX	,
	LODS_INDEX	,
	MAX_INDEX	,
}ALARM_INDEX_T;

typedef struct {
    unchar SD_thld;
    unchar SF_thld;
    uint   SD_cnt;
    uint   SF_cnt;
}GPON_SD_SF_Info_t;

extern GPON_SD_SF_Info_t gGpon_SD_SF_Info;
/**************************************************************************
**************************************************************************/
int gpon_disable_with_option(GPON_DEV_RESET_TYPE_t reset_type);
void gpon_set_ploamu_int(unchar flag);
void gpon_init_dying_info(void);
extern int DISABLE_CALL_TRACE_DEBUG_FLAG;
static inline int gpon_disable(void)
{
	if(DISABLE_CALL_TRACE_DEBUG_FLAG)
		dump_stack();
	
    return gpon_disable_with_option(GPON_DEV_PLAIN_RESET);
}

int gpon_enable(void) ;
void prepare_gpon(preXponMode_t mode);
void gpon_isr(void) ;
void gpon_detect_los_lof(PHY_Event_Type_t) ;
void gpon_detect_phy_ready(void) ;
void gpon_ber_interval_expires(TIMER_FUN_PAAM arg) ;
void gpon_silence_interval_expires(TIMER_FUN_PAAM arg) ;
void gpon_start_silence(void);
void gpon_stop_silence(void);
void gpon_update_silence(void);
void gpon_recv_ploam_message(void);
void schedule_gpon_dev_reset(GPON_DEV_RESET_TYPE_t type);
void gpon_stop_timer(void);
void gpon_set_cpu_load_version(ENUM_LIMIT_TYPE type);
void gpon_detect_dying_gasp(void);
void gpon_check_alarm_jiffiesClk(void);
void gpon_set_alarmBit(ALARM_INDEX_T index);
void gpon_clear_alarmBit(ALARM_INDEX_T index);
void gpon_reset_alarm_jiffiesClk(void);
void gpon_clear_all_alarm(void);

extern int irq_set_affinity_ex(unsigned int irq, const struct cpumask *cpumask);
#if defined(TCSUPPORT_CPU_EN7526) || defined(TCSUPPORT_CPU_EN7527)
extern void gpon_software_rdi_send(void);
extern unsigned char RDI_TIMER_EN;
extern struct timer_list rdi_timer;
extern unsigned char RDI_SEND_ENABLE;
extern unsigned int RDI_SEND_TIMES;
extern unsigned int RDI_SEND_DELAY;
#endif

#endif /* _GPON_H_ */

