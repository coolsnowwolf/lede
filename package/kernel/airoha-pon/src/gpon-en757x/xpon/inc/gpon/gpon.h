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
#define GPON_START_TIMER(timer)			{ mod_timer(&timer, (jiffies + ((timer.data*HZ)/1000))) ; }
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
	unchar					reiSeq ; 	/* rei ploam message sequence */
	uint					to1Timer ;	/* o1 timer time for onu activation */
	uint					to2Timer ;	/* o2 timer time for onu activation */
	uint					berInterval ; /* BER interval from OLT, the uint is ms */
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
#ifdef TCSUPPORT_CPU_EN7521
	uint					sleep_count;
	uint 					phy_psync_to_sof_delay;
	GPON_DEV_SNIFFER_MODE_T sniffer_mode;
	/* wan cpu protection */
	QDMA_RxRateLimitCfg_T	wan_rxRateLimitCfg;			/* bfb55120 */
	QDMA_RxRateLimitSet_T	wan_rxRateLimit;			/* bfb55124 */
	/* lan cpu protection */
	QDMA_RxRateLimitCfg_T	lan_rxRateLimitCfg;			/* bfb45120 */
	QDMA_RxRateLimitSet_T	lan_rxRateLimit;			/* bfb45124 */
#else /* TCSUPPORT_CPU_EN7521 */
	uint					reg4208;
	unchar					rstDbgDly;
	unchar					macPhyReset; 
#endif

    unchar                  sr_blk_size;
    GPON_TCONT_T            tconts;
    GPON_DYING_GASP_INFO_T	dying_info;
	REG_G_PLOu_GUARD_BIT    gponGuardBit ;
	REG_G_PLOu_DELM_BIT     gponDelmBit ;
	REG_G_PLOu_PRMBL_TYPE1_2 gponPrmblType ;
    REG_G_PLOu_PRMBL_TYPE3  gponT3Prmbl ;
    REG_G_PRE_ASSIGNED_DLY  gponPreAssignDly ;

	struct {
		unchar				isRequestKey		: 1 ;
		unchar				isTodUpdate 		: 1 ;
		unchar				dvtGponLosFlag		: 1 ;
		unchar 				eqdOffsetFlag		: 1 ;		/* eqd offset flag 1--neg*/
		unchar				preambleFlag		: 1 ;
		unchar				sniffer				: 1 ;		
		unchar				hotplug				: 1 ;
#ifdef TCSUPPORT_CPU_EN7521
		unchar				isPloamFilter		: 1 ;
		unchar				isBwmStopTimeErrInt	: 1 ;
#endif
	} flags ;
} GPON_Config_T ;


/** GPON_GlbPriv_T means "GPON Global Private data".
 */
typedef struct {
	unsigned int            bip_cnt_val;
	ENUM_GponState_t 		state ;
	struct timer_list		to1_timer ;
	struct timer_list		to2_timer ;
	struct timer_list		ber_timer ;
	
	ploam_recv_handler_t	ploamRecvHandler[PLOAM_DOWN_MAX_TYPE] ;
	PLOAM_RAW_General_T		prePloamMsg ;
	
	GPON_Config_T			gponCfg ;
	
	spinlock_t				act_lock ;			/* spin lock for GPON activation */
#ifdef TCSUPPORT_CPU_EN7521
	spinlock_t				swReset_lock ;
#endif
    unchar                  typeBOnGoing     : 1 ;
	unchar	                disableSnFlag    : 1 ;
    unchar                  emergencystate   : 1 ;
	struct Gpon_Recovery_S* pGponRecovery;
} GPON_GlbPriv_T ;

typedef enum {
    GPON_DEV_PLAIN_RESET          ,
    GPON_DEV_RESET_WITH_FE_RESET ,
} GPON_DEV_RESET_TYPE_t;

/**************************************************************************
**************************************************************************/
int gpon_disable_with_option(GPON_DEV_RESET_TYPE_t reset_type);
void gpon_set_ploamu_int(unchar flag);
void gpon_init_dying_info(void);
static inline int gpon_disable(void)
{
    return gpon_disable_with_option(GPON_DEV_PLAIN_RESET);
}

int gpon_enable(void) ;
void prepare_gpon(void);
void gpon_isr(void) ;
void gpon_detect_los_lof(void) ;
void gpon_detect_phy_ready(void) ;
void gpon_ber_interval_expires(unsigned long arg) ;
void gpon_recv_ploam_message(void);
void schedule_gpon_dev_reset(GPON_DEV_RESET_TYPE_t type);
void gpon_stop_timer(void);

#endif /* _GPON_H_ */

