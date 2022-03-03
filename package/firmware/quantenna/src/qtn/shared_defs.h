/*SH1
*******************************************************************************
**                                                                           **
**         Copyright (c) 2010 Quantenna Communications Inc                   **
**                            All Rights Reserved                            **
**                                                                           **
**  Author      : Quantenna Communications Inc                               **
**  File        : shared_params.h                                            **
**  Description :                                                            **
**                                                                           **
*******************************************************************************
**                                                                           **
**  Redistribution and use in source and binary forms, with or without       **
**  modification, are permitted provided that the following conditions       **
**  are met:                                                                 **
**  1. Redistributions of source code must retain the above copyright        **
**     notice, this list of conditions and the following disclaimer.         **
**  2. Redistributions in binary form must reproduce the above copyright     **
**     notice, this list of conditions and the following disclaimer in the   **
**     documentation and/or other materials provided with the distribution.  **
**  3. The name of the author may not be used to endorse or promote products **
**     derived from this software without specific prior written permission. **
**                                                                           **
**  Alternatively, this software may be distributed under the terms of the   **
**  GNU General Public License ("GPL") version 2, or (at your option) any    **
**  later version as published by the Free Software Foundation.              **
**                                                                           **
**  In the case this software is distributed under the GPL license,          **
**  you should have received a copy of the GNU General Public License        **
**  along with this software; if not, write to the Free Software             **
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA  **
**                                                                           **
**  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR       **
**  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES**
**  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  **
**  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,         **
**  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT **
**  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,**
**  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    **
**  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT      **
**  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF **
**  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.        **
**                                                                           **
*******************************************************************************
EH1*/

#ifndef _SHARED_DEFS_H_
#define _SHARED_DEFS_H_

#include "shared_defs_common.h"

#ifdef TOPAZ_PLATFORM
#define QTN_SWITCH_CHANNEL_TIME_AVG	3750	/* microseconds */
#else
#define QTN_SWITCH_CHANNEL_TIME_AVG	3500	/* microseconds */
#endif

#define IEEE80211_MAX_NAV	32767

/* SCS (ACI/CCI Detection and Mitigation) APIs */
enum qtn_vap_scs_cmds {
	IEEE80211_SCS_SET_ENABLE = 1,
	IEEE80211_SCS_SET_DEBUG_ENABLE,
	IEEE80211_SCS_SET_SAMPLE_ENABLE,
	IEEE80211_SCS_SET_SAMPLE_DWELL_TIME,
	IEEE80211_SCS_SET_SAMPLE_INTERVAL,
	IEEE80211_SCS_SET_THRSHLD_SMPL_PKTNUM,
	IEEE80211_SCS_SET_THRSHLD_PRI_CCA,
	IEEE80211_SCS_SET_THRSHLD_SEC_CCA,
	IEEE80211_SCS_SET_THRSHLD_SMPL_AIRTIME,
	IEEE80211_SCS_SET_WF_CCA,
	IEEE80211_SCS_SET_WF_RSSI,
	IEEE80211_SCS_SET_WF_CRC_ERR,
	IEEE80211_SCS_SET_WF_LPRE,
	IEEE80211_SCS_SET_WF_SPRE,
	IEEE80211_SCS_SET_WF_RETRIES,
	IEEE80211_SCS_SET_WF_DFS,
	IEEE80211_SCS_SET_WF_MAX_TX_PWR,
	IEEE80211_SCS_SET_REPORT_ONLY,
	IEEE80211_SCS_SET_CCA_INTF_RATIO,
	IEEE80211_SCS_SET_CCA_IDLE_THRSHLD,
	IEEE80211_SCS_SET_CCA_INTF_LO_THR,
	IEEE80211_SCS_SET_CCA_INTF_HI_THR,
	IEEE80211_SCS_SET_CCA_SMPL_DUR,
	IEEE80211_SCS_GET_REPORT,
	IEEE80211_SCS_GET_INTERNAL_STATS,
	IEEE80211_SCS_SET_CCA_INTF_SMTH_FCTR,
	IEEE80211_SCS_RESET_RANKING_TABLE,
	IEEE80211_SCS_SET_CHAN_MTRC_MRGN,
	IEEE80211_SCS_SET_RSSI_SMTH_FCTR,
	IEEE80211_SCS_SET_ATTEN_ADJUST,
	IEEE80211_SCS_SET_THRSHLD_ATTEN_INC,
	IEEE80211_SCS_SET_THRSHLD_DFS_REENTRY,
	IEEE80211_SCS_SET_THRSHLD_DFS_REENTRY_MINRATE,
	IEEE80211_SCS_SET_PMBL_ERR_SMTH_FCTR,
	IEEE80211_SCS_SET_PMBL_ERR_RANGE,
	IEEE80211_SCS_SET_PMBL_ERR_MAPPED_INTF_RANGE,
	IEEE80211_SCS_SET_THRSHLD_LOAD,
	IEEE80211_SCS_SET_PMBL_ERR_WF,
	IEEE80211_SCS_SET_THRSHLD_AGING_NOR,
	IEEE80211_SCS_SET_THRSHLD_AGING_DFSREENT,
	IEEE80211_SCS_SET_THRSHLD_DFS_REENTRY_INTF,
	IEEE80211_SCS_SET_PMP_RPT_CCA_SMTH_FCTR,
	IEEE80211_SCS_SET_PMP_RX_TIME_SMTH_FCTR,
	IEEE80211_SCS_SET_PMP_TX_TIME_SMTH_FCTR,
	IEEE80211_SCS_SET_PMP_STATS_STABLE_PERCENT,
	IEEE80211_SCS_SET_PMP_STATS_STABLE_RANGE,
	IEEE80211_SCS_SET_PMP_STATS_CLEAR_INTERVAL,
	IEEE80211_SCS_SET_PMP_TXTIME_COMPENSATION,
	IEEE80211_SCS_SET_PMP_RXTIME_COMPENSATION,
	IEEE80211_SCS_SET_PMP_TDLSTIME_COMPENSATION,
	IEEE80211_SCS_SET_SWITCH_CHANNEL_MANUALLY,
	IEEE80211_SCS_SET_AS_RX_TIME_SMTH_FCTR,
	IEEE80211_SCS_SET_AS_TX_TIME_SMTH_FCTR,
	IEEE80211_SCS_SET_STATS_START,
	IEEE80211_SCS_SET_CCA_IDLE_SMTH_FCTR,
	IEEE80211_SCS_SET_PMBL_ERR_THRSHLD,
	IEEE80211_SCS_SET_CCA_INTF_DFS_MARGIN,
	IEEE80211_SCS_SET_MAX
};

#define IEEE80211_SCS_STATE_INIT			0
#define IEEE80211_SCS_STATE_RESET			1
#define IEEE80211_SCS_STATE_CHANNEL_SWITCHING		2
#define IEEE80211_SCS_STATE_MEASUREMENT_CHANGE_CLEAN	3    /* param change */
#define IEEE80211_SCS_STATE_PERIOD_CLEAN		4

#define IEEE80211_SCS_COMPARE_INIT_TIMER	5
#define IEEE80211_SCS_COMPARE_TIMER_INTVAL	2
#define IEEE80211_CCA_SAMPLE_DUR		IEEE80211_SCS_COMPARE_TIMER_INTVAL /* seconds */
#define IEEE80211_SCS_CHAN_CURRENT		0
#define IEEE80211_SCS_CHAN_ALL			0xFF
#define IEEE80211_SCS_THRSHLD_MAX		100	/* metric */
#define IEEE80211_SCS_THRSHLD_MIN		1	/* metric */
#define IEEE80211_SCS_SMPL_DWELL_TIME_MAX	24	/* milliseconds, limited by max NAV reservation */
#define IEEE80211_SCS_SMPL_DWELL_TIME_MIN	5	/* milliseconds */
#define IEEE80211_SCS_SMPL_DWELL_TIME_DEFAULT	10	/* milliseconds */
#define IEEE80211_SCS_SMPL_INTV_MAX		3600	/* seconds */
#define IEEE80211_SCS_SMPL_INTV_MIN		1	/* seconds */
#define IEEE80211_SCS_SMPL_INTV_DEFAULT		5	/* seconds */
#define IEEE80211_SCS_THRSHLD_SMPL_PKTNUM_DEFAULT	16	/* packet number */
#define IEEE80211_SCS_THRSHLD_SMPL_PKTNUM_MAX	1000	/* packet number */
#define IEEE80211_SCS_THRSHLD_SMPL_PKTNUM_MIN	1	/* packet number */
#ifdef TOPAZ_PLATFORM
#define IEEE80211_SCS_THRSHLD_SMPL_AIRTIME_DEFAULT	200	/* ms */
#else
#define IEEE80211_SCS_THRSHLD_SMPL_AIRTIME_DEFAULT	300	/* ms */
#endif
#define IEEE80211_SCS_THRSHLD_SMPL_AIRTIME_MAX	1000	/* ms */
#define IEEE80211_SCS_THRSHLD_SMPL_AIRTIME_MIN	1	/* ms */
#define IEEE80211_SCS_THRSHLD_PMBL_ERR_MAX	10000	/* count */
#define IEEE80211_SCS_THRSHLD_PMBL_ERR_MIN	1	/* count */

/*
 * Packet rate threshold is determined by how many packets we can hold in buffer without drop
 * during off-channel period. It is limited by:
 * - sw queue length of each node/tid
 * - global resource shared by all node/tid, such as tqew descriptors and msdu headers.
 * Current value doesn't apply to the scenario when tqew descriptors are already used up by large
 * number of stations.
 */
#define IEEE80211_SCS_THRSHLD_SMPL_TX_PKTRATE	(1024 - 128)	/* margin = 128 + hw ring size */
#define IEEE80211_SCS_THRSHLD_SMPL_RX_PKTRATE	IEEE80211_SCS_THRSHLD_SMPL_TX_PKTRATE /* assume qtn peer */
#define IEEE80211_SCS_THRSHLD_ATTEN_INC_DFT	5	/* db */
#define IEEE80211_SCS_THRSHLD_ATTEN_INC_MIN     0       /* db */
#define IEEE80211_SCS_THRSHLD_ATTEN_INC_MAX     20      /* db */
#define IEEE80211_SCS_THRSHLD_DFS_REENTRY_DFT	60	/* seconds */
#define IEEE80211_SCS_THRSHLD_DFS_REENTRY_MIN   0       /* seconds */
#define IEEE80211_SCS_THRSHLD_DFS_REENTRY_MAX   0xffff  /* seconds */
#define IEEE80211_SCS_THRSHLD_DFS_REENTRY_INTF_MIN   0
#define IEEE80211_SCS_THRSHLD_DFS_REENTRY_INTF_MAX   100
#define IEEE80211_SCS_THRSHLD_DFS_REENTRY_INTF_DFT   40
#define IEEE80211_SCS_THRSHLD_DFS_REENTRY_MINRATE_UNIT	100	/* kbps */
#define IEEE80211_SCS_THRSHLD_DFS_REENTRY_MINRATE_DFT	5	/* unit: 100kbps */
#define IEEE80211_SCS_THRSHLD_DFS_REENTRY_MINRATE_MIN   0       /* unit: 100kbps */
#define IEEE80211_SCS_THRSHLD_DFS_REENTRY_MINRATE_MAX   0xffff  /* unit: 100kbps */
#define IEEE80211_SCS_THRSHLD_AGING_MIN         0
#define IEEE80211_SCS_THRSHLD_AGING_MAX         0xFFFF
#define IEEE80211_SCS_THRSHLD_AGING_NOR_DFT     (60 * 6)
#define IEEE80211_SCS_THRSHLD_AGING_DFSREENT_DFT  5
#define IEEE80211_SCS_CCA_DUR_MAX		10	/* seconds */
#define IEEE80211_SCS_CCA_DUR_MIN		2	/* seconds */
#define IEEE80211_SCS_CCA_INTF_SCALE		1000	/* milliseconds */
#define IEEE80211_SCS_SENDING_QOSNULL_TIME_AVG	1000	/* microseconds */
#define IEEE80211_SCS_SMPL_TIME_MARGIN		2000	/* microseconds */
#define IEEE80211_SCS_SMPL_TIME_OFFSET_SEND_QOSNULL	5000	/* microseconds */
#define IEEE80211_SCS_SMPL_TIME_SENDING_ALL_BEACONS	25000	/* microseconds, the time duration for transmitting all beacons */
#define IEEE80211_CCA_INTF_SMTH_FCTR_NOXP_DFT	75
#define IEEE80211_CCA_INTF_SMTH_FCTR_XPED_DFT	90
#define IEEE80211_CCA_INTF_SMTH_FCTR_MIN	0
#define IEEE80211_CCA_INTF_SMTH_FCTR_MAX	100
#define IEEE80211_SCS_CHAN_MTRC_MRGN_MAX	100
#define IEEE80211_SCS_CHAN_MTRC_MRGN_DFT	5
#define IEEE80211_SCS_RSSI_SMTH_FCTR_UP_DFT	75
#define IEEE80211_SCS_RSSI_SMTH_FCTR_DOWN_DFT	25
#define IEEE80211_SCS_RSSI_SMTH_FCTR_MAX	100
#define IEEE80211_SCS_ATTEN_ADJUST_MIN		-20
#define IEEE80211_SCS_ATTEN_ADJUST_MAX		20
#define IEEE80211_SCS_ATTEN_ADJUST_DFT		5
#define IEEE80211_SCS_BRCM_RXGLITCH_THRSHLD_SCALE_DFT    40
#define IEEE80211_SCS_PMBL_ERR_SMTH_FCTR_MIN    0
#define IEEE80211_SCS_PMBL_ERR_SMTH_FCTR_MAX    100
#define IEEE80211_SCS_PMBL_ERR_SMTH_FCTR_DFT    66
#define IEEE80211_SCS_CCA_IDLE_SMTH_FCTR_MIN    0
#define IEEE80211_SCS_CCA_IDLE_SMTH_FCTR_MAX    100
#define IEEE80211_SCS_CCA_IDLE_SMTH_FCTR_DFT    50
#define IEEE80211_SCS_PMP_RPT_CCA_SMTH_FCTR_MAX    100
#define IEEE80211_SCS_PMP_RPT_CCA_SMTH_FCTR_DFT    66
#define IEEE80211_SCS_PMP_RX_TIME_SMTH_FCTR_MAX    100
#define IEEE80211_SCS_PMP_RX_TIME_SMTH_FCTR_DFT    66
#define IEEE80211_SCS_PMP_TX_TIME_SMTH_FCTR_MAX    100
#define IEEE80211_SCS_PMP_TX_TIME_SMTH_FCTR_DFT    66
#define IEEE80211_SCS_PMP_STATS_STABLE_PERCENT_MAX  100
#define IEEE80211_SCS_PMP_STATS_STABLE_PERCENT_DFT  30
#define IEEE80211_SCS_PMP_STATS_STABLE_RANGE_MAX    1000
#define IEEE80211_SCS_PMP_STATS_STABLE_RANGE_DFT    50
#define IEEE80211_SCS_PMP_STATS_CLEAR_INTERVAL_MAX  3600 /* seconds */
#define IEEE80211_SCS_PMP_STATS_CLEAR_INTERVAL_DFT  60 /* seconds */
#define IEEE80211_SCS_AS_RX_TIME_SMTH_FCTR_MAX    100
#define IEEE80211_SCS_AS_RX_TIME_SMTH_FCTR_DFT    50
#define IEEE80211_SCS_AS_TX_TIME_SMTH_FCTR_MAX    100
#define IEEE80211_SCS_AS_TX_TIME_SMTH_FCTR_DFT    50

#define IEEE80211_SCS_SMTH_RBS_TIME			80

#define IEEE80211_SCS_PMBL_ERR_RANGE_MIN        1000
#define IEEE80211_SCS_PMBL_ERR_RANGE_MAX        0xFFFF
#define IEEE80211_SCS_PMBL_ERR_RANGE_DFT        5000
#define IEEE80211_SCS_PMBL_ERR_MAPPED_INTF_RANGE_MIN  0
#define IEEE80211_SCS_PMBL_ERR_MAPPED_INTF_RANGE_MAX  100
#define IEEE80211_SCS_PMBL_ERR_MAPPED_INTF_RANGE_DFT  40
#define IEEE80211_SCS_PMBL_ERR_WF_MIN           0
#define IEEE80211_SCS_PMBL_ERR_WF_MAX           100
#define IEEE80211_SCS_PMBL_SHORT_WF_DFT         0
#define IEEE80211_SCS_PMBL_LONG_WF_DFT          100
#define IEEE80211_SCS_THRSHLD_LOADED_MIN        0
#define IEEE80211_SCS_THRSHLD_LOADED_MAX        1000
#define IEEE80211_SCS_THRSHLD_LOADED_DFT        20

#define IEEE80211_SCS_CHAN_POWER_CUTPOINT       15
#define IEEE80211_SCS_NORMALIZE(_v, _duration)       (((_v) < (0xFFFFFFFF / IEEE80211_SCS_CCA_INTF_SCALE)) ?  \
							((_v) * IEEE80211_SCS_CCA_INTF_SCALE / (_duration)) : \
							((_v) / (_duration) * IEEE80211_SCS_CCA_INTF_SCALE))

#define IEEE80211_SCS_SMOOTH(_old, _new, _fctr)	(((_old) * (_fctr) + (_new) * (100 - (_fctr))) / 100)

#define IEEE80211_SCS_OFFCHAN_WHOLE_DUR(_dwell_us)	((_dwell_us) +					\
							(2 * QTN_SWITCH_CHANNEL_TIME_AVG) +		\
							IEEE80211_SCS_SENDING_QOSNULL_TIME_AVG +	\
							IEEE80211_SCS_SMPL_TIME_MARGIN)

#define IEEE80211_SCS_VALUE_S			0
#define IEEE80211_SCS_VALUE_M			0xffff
#define IEEE80211_SCS_WF_VALUE_M		0xff
#define IEEE80211_SCS_COMMAND_S			16
#define IEEE80211_SCS_COMMAND_M			0xffff

#define IEEE80211_SCS_STA_CCA_REQ_CC		0x1
#define IEEE80211_SCS_SELF_CCA_CC               0x2
#define IEEE80211_SCS_ATTEN_INC_CC		0x4
#define IEEE80211_SCS_BRCM_STA_TRIGGER_CC	0x8
#define IEEE80211_SCS_CCA_INTF_CC               (IEEE80211_SCS_STA_CCA_REQ_CC | IEEE80211_SCS_SELF_CCA_CC)
#define IEEE80211_SCS_INTF_CC                   (IEEE80211_SCS_CCA_INTF_CC | IEEE80211_SCS_BRCM_STA_TRIGGER_CC)

#define	IEEE80211_REMAIN_CHAN_MIN_RSV_PERD	2

enum ieee80211_scs_update_mode {
	IEEE80211_SCS_OFFCHAN,		/* off-channel, use smoothing and omit current channel */
	IEEE80211_SCS_COCHAN,		/* co-channel mode */
	IEEE80211_SCS_INIT_SCAN,	/* like off-channel but include current channel */
};

#define SCSLOG_CRIT                             0
#define SCSLOG_NOTICE                           1
#define SCSLOG_INFO                             2
#define SCSLOG_VERBOSE                          3
#define SCSLOG_LEVEL_MAX                        3
#if !defined(MUC_BUILD) && !defined(DSP_BUILD) && !defined(AUC_BUILD)
#define SCSDBG(_level, _fmt, ...)            do {               \
		if (ic->ic_scs.scs_debug_enable >= (_level)) {  \
			DBGFN("SCS: " _fmt, ##__VA_ARGS__);     \
		}                                               \
	} while (0)
#endif


/* OCAC (Off-channel CAC) APIs */
enum qtn_ocac_cmds {
	IEEE80211_OCAC_SET_ENABLE = 1,
	IEEE80211_OCAC_SET_DISABLE,
	IEEE80211_OCAC_SET_DEBUG_LEVEL,
	IEEE80211_OCAC_SET_DWELL_TIME,
	IEEE80211_OCAC_SET_DURATION,
	IEEE80211_OCAC_SET_THRESHOLD_FAT,
	IEEE80211_OCAC_SET_DUMP_COUNTS,
	IEEE80211_OCAC_SET_CAC_TIME,
	IEEE80211_OCAC_SET_THRESHOLD_TRAFFIC,
	IEEE80211_OCAC_SET_TIMER_INTERVAL,
	IEEE80211_OCAC_SET_DUMP_TSFLOG,
	IEEE80211_OCAC_SET_DUMP_CFG,
	IEEE80211_OCAC_SET_TRAFFIC_CONTROL,
	IEEE80211_OCAC_SET_THRESHOLD_CCA_INTF,
	IEEE80211_OCAC_SET_REPORT_ONLY,
	IEEE80211_OCAC_SET_DUMP_CCA_COUNTS,
	IEEE80211_OCAC_SET_OFFSET_TXHALT,
	IEEE80211_OCAC_SET_OFFSET_OFFCHAN,
	IEEE80211_OCAC_SET_THRESHOLD_FAT_DEC,
	IEEE80211_OCAC_SET_TIMER_EXPIRE_INIT,
	IEEE80211_OCAC_SET_SECURE_DWELL_TIME,
	IEEE80211_OCAC_SET_BEACON_INTERVAL,
	IEEE80211_OCAC_SET_WEATHER_DURATION,
	IEEE80211_OCAC_SET_WEATHER_CAC_TIME,
	IEEE80211_OCAC_SET_MAX
};

enum qtn_ocac_get_cmds {
	IEEE80211_OCAC_GET_STATUS = 1,
	IEEE80211_OCAC_GET_AVAILABILITY,
};

#define IEEE80211_OCAC_CLEAN_STATS_STOP		0
#define IEEE80211_OCAC_CLEAN_STATS_START	1
#define IEEE80211_OCAC_CLEAN_STATS_RESET	2


#define IEEE80211_OCAC_DWELL_TIME_MIN		5	/* milliseconds */
#define IEEE80211_OCAC_DWELL_TIME_MAX		200	/* milliseconds */
#define IEEE80211_OCAC_DWELL_TIME_DEFAULT	50	/* milliseconds */

#define IEEE80211_OCAC_SECURE_DWELL_TIME_MIN		5	/* milliseconds */
#define IEEE80211_OCAC_SECURE_DWELL_TIME_MAX		23	/* milliseconds */
#define IEEE80211_OCAC_SECURE_DWELL_TIME_DEFAULT	23	/* milliseconds */

#define IEEE80211_OCAC_DURATION_MIN		1	/* seconds */
#define IEEE80211_OCAC_DURATION_MAX		64800	/* seconds */
#define IEEE80211_OCAC_DURATION_DEFAULT		360	/* seconds */

#define IEEE80211_OCAC_CAC_TIME_MIN		1	/* seconds */
#define IEEE80211_OCAC_CAC_TIME_MAX		64800	/* seconds */
#define IEEE80211_OCAC_CAC_TIME_DEFAULT		145	/* seconds */

#define IEEE80211_OCAC_WEA_DURATION_MIN		60	/* seconds */
#define IEEE80211_OCAC_WEA_DURATION_MAX		86400	/* seconds */

#define IEEE80211_OCAC_WEA_CAC_TIME_MIN		1	/* seconds */
#define IEEE80211_OCAC_WEA_CAC_TIME_MAX		86400	/* seconds */

#define IEEE80211_OCAC_THRESHOLD_FAT_MIN	1	/* percent */
#define IEEE80211_OCAC_THRESHOLD_FAT_MAX	100	/* percent */
#define IEEE80211_OCAC_THRESHOLD_FAT_DEFAULT	65	/* percent */

#define IEEE80211_OCAC_THRESHOLD_TRAFFIC_MIN		1	/* percent */
#define IEEE80211_OCAC_THRESHOLD_TRAFFIC_MAX		100	/* percent */
#define IEEE80211_OCAC_THRESHOLD_TRAFFIC_DEFAULT	35	/* percent */

#define IEEE80211_OCAC_OFFSET_TXHALT_MIN		2	/* milliseconds */
#define IEEE80211_OCAC_OFFSET_TXHALT_MAX		80	/* milliseconds */
#define IEEE80211_OCAC_OFFSET_TXHALT_DEFAULT		10	/* milliseconds */

#define IEEE80211_OCAC_OFFSET_OFFCHAN_MIN		2	/* milliseconds */
#define IEEE80211_OCAC_OFFSET_OFFCHAN_MAX		80	/* milliseconds */
#define IEEE80211_OCAC_OFFSET_OFFCHAN_DEFAULT		5	/* milliseconds */

#define IEEE80211_OCAC_TRAFFIC_CTRL_DEFAULT		1	/* on */

#define IEEE80211_OCAC_THRESHOLD_CCA_INTF_MIN		1	/* percent */
#define IEEE80211_OCAC_THRESHOLD_CCA_INTF_MAX		100	/* percent */
#define IEEE80211_OCAC_THRESHOLD_CCA_INTF_DEFAULT	20	/* percent */

#define IEEE80211_OCAC_THRESHOLD_FAT_DEC_MIN		1	/* percent */
#define IEEE80211_OCAC_THRESHOLD_FAT_DEC_MAX		100	/* percent */
#define IEEE80211_OCAC_THRESHOLD_FAT_DEC_DEFAULT	10	/* percent */

#define IEEE80211_OCAC_TIMER_INTERVAL_MIN		1	/* seconds */
#define IEEE80211_OCAC_TIMER_INTERVAL_MAX		100	/* seconds */
#define IEEE80211_OCAC_TIMER_INTERVAL_DEFAULT		2	/* seconds */

#define IEEE80211_OCAC_BEACON_INTERVAL_MIN		100	/* TUs */
#define IEEE80211_OCAC_BEACON_INTERVAL_MAX		1000	/* TUs */
#define IEEE80211_OCAC_BEACON_INTERVAL_DEFAULT		100	/* TUs */

#define IEEE80211_OCAC_TIMER_EXPIRE_INIT_MIN		1	/* seconds */
#define IEEE80211_OCAC_TIMER_EXPIRE_INIT_MAX		65000	/* seconds */
#define IEEE80211_OCAC_TIMER_EXPIRE_INIT_DEFAULT	2	/* seconds */


#define IEEE80211_OCAC_VALUE_S			0
#define IEEE80211_OCAC_VALUE_M			0xffff
#define IEEE80211_OCAC_COMMAND_S		16
#define IEEE80211_OCAC_COMMAND_M		0xffff
#define IEEE80211_OCAC_COMPRESS_VALUE_F		0x8000
#define IEEE80211_OCAC_COMPRESS_VALUE_M		0x7fff

#define IEEE80211_OCAC_TIME_MARGIN		2000	/* microseconds */

#define OCACLOG_CRIT				0
#define OCACLOG_WARNING				1
#define OCACLOG_NOTICE				2
#define OCACLOG_INFO				3
#define OCACLOG_VERBOSE				4
#define OCACLOG_LEVEL_MAX			4
#if !defined(MUC_BUILD) && !defined(DSP_BUILD) && !defined(AUC_BUILD)
#define OCACDBG(_level, _fmt, ...)            do {               \
		if (ic->ic_ocac.ocac_cfg.ocac_debug_level >= (_level)) {  \
			DBGFN("DFS_s_radio: " _fmt, ##__VA_ARGS__);     \
		}                                               \
        } while (0)
#endif

#define QTN_M2A_EVENT_TYPE_DTIM		1
#define	QTN_M2A_PS_EVENT_PM_ENABLE	2		/* enable power management */
#define	QTN_M2A_PS_EVENT_PM_DISABLE	3		/* disable power management */
#define	QTN_M2A_PS_EVENT_PS_POLL	4		/* ps poll */
#define	QTN_M2A_EVENT_TYPE_UAPSD_SP	5		/* U-APSD SP */
#define QTN_M2A_EVENT_PTID_FLAG_SET     6               /* Set per-TID flag(muc) */

/* Common definitions for flags used to indicate ieee80211_node's states */
#define	IEEE80211_NODE_AUTH		0x0001	/* authorized for data */
#define	IEEE80211_NODE_QOS		0x0002	/* QoS enabled */
#define	IEEE80211_NODE_ERP		0x0004	/* ERP enabled */
#define	IEEE80211_NODE_HT		0x0008	/* HT enabled */
/* NB: this must have the same value as IEEE80211_FC1_PWR_MGT */
#define	IEEE80211_NODE_PWR_MGT		0x0010	/* power save mode enabled */
#define	IEEE80211_NODE_PS_DELIVERING	0x0040	/* STA out of PS, getting delivery */
#define	IEEE80211_NODE_PS_POLL		0x0080	/* power save ps poll mode */
#define	IEEE80211_NODE_AREF		0x0020	/* authentication ref held */
#define IEEE80211_NODE_2_TX_CHAINS      0x0400  /* this node needs to use 2 TX chain only, for IOT purpose */
#define IEEE80211_NODE_UAPSD		0x1000
#define IEEE80211_NODE_WDS_PEER		0x2000	/* this node is the wds peer in a wds vap */
#define IEEE80211_NODE_VHT		0x4000	/* VHT enabled */
#define IEEE80211_NODE_TPC		0x8000	/* indicate tpc capability */

/* Common definitions for ext_flags */
#define IEEE80211_NODE_TDLS_PTI_REQ	0x0001	/* Should sending PTI request to peer */
#define IEEE80211_NODE_TDLS_PTI_PENDING	0x0002	/* PTI request xmit to peer but not responsed */
#define IEEE80211_NODE_UAPSD_SP_IN_PROGRESS	0x0004	/* U-APSD SP in progress */
#define IEEE80211_NODE_TDLS_PTI_RESP	0x0008	/* PTI response frame received */

#define	IEEE80211_NODE_TDLS_MASK	0x000B	/* Mask for TDLS bits */

#define QTN_VAP_PRIORITY_RESERVED	2	/* reserve the low values for internal use */
#define QTN_VAP_PRIORITY_NUM		4
#define QTN_VAP_PRIORITY_MGMT		(QTN_VAP_PRIORITY_RESERVED + QTN_VAP_PRIORITY_NUM)
#define QTN_TACMAP_HW_PRI_NUM		8	/* hw limitation for 128 node mode */
#define QTN_TACMAP_PRI_PER_VAP		8	/* for maximum 8 TIDs */
#define QTN_TACMAP_SW_PRI_BASE		64	/* values below this are used for "bad apple" nodes */

/* Quantenna specific flags (ni_qtn_flags), do not modify in Auc */
#define QTN_IS_BCM_NODE			0x0000001
#define QTN_IS_IPAD_NODE		0x0000002
#define QTN_IS_IPHONE5_NODE		0x0000004
#define QTN_IS_IPAD3_NODE		0x0000008
#define QTN_IS_INTEL_5100_NODE		0x0000010
#define QTN_IS_INTEL_5300_NODE		0x0000020
#define QTN_IS_SAMSUNG_GALAXY_NODE	0x0000040
#define QTN_IS_NOT_4ADDR_CAPABLE_NODE	0x0000080
#define QTN_AC_BE_INHERITANCE_UPTO_VO	0x0000100
#define QTN_AC_BE_INHERITANCE_UPTO_VI	0x0000200
#define QTN_IS_INTEL_NODE		0x0000400
#define QTN_IS_IPAD_AIR_NODE		0x0000800
#define QTN_IS_IPAD4_NODE		0x0001000
#define QTN_IS_REALTEK_NODE		0x0004000
#define	QTN_NODE_TX_RESTRICTED		0x0008000 /* restricted tx enabled */
#define	QTN_NODE_TX_RESTRICT_RTS	0x0010000 /* use RTS to confirm node is lost */
#define QTN_IS_NO_RXAMSDU_NO_BF_NODE	0x0020000
#define QTN_NODE_RXAMSDU_SUPPORT	0x0040000 /* node support TX amsdu */
#define QTN_NODE_11N_TXAMSDU_OFF	0x0080000
#define	QTN_NODE_TXOP_RESTRICTED	0x0100000
/*
 * Bits that can be updated again by Lhost after association creation. Explicit definition helps
 * avoid overwriting bits maintained by MuC itself.
 */
#define QTN_FLAGS_UPDATABLE_BITS	(QTN_IS_INTEL_NODE)

/* QTN bandwidth definition */
#define QTN_BW_20M	0
#define QTN_BW_40M	1
#define QTN_BW_80M	2
#define QTN_BW_MAX	QTN_BW_80M

#define QTN_MAILBOX_INVALID	0xffffffff	/* Invalid value to indicate mailbox is disabled */

enum ni_tdls_status {
	IEEE80211_TDLS_NODE_STATUS_NONE = 0,
	IEEE80211_TDLS_NODE_STATUS_INACTIVE = 1,
	IEEE80211_TDLS_NODE_STATUS_STARTING = 2,
	IEEE80211_TDLS_NODE_STATUS_ACTIVE = 3,
	IEEE80211_TDLS_NODE_STATUS_IDLE = 4
};

/* WoWLAN APIs */
enum qtn_vap_wowlan_cmds {
	IEEE80211_WOWLAN_HOST_POWER_SAVE = 1,
	IEEE80211_WOWLAN_MATCH_TYPE,
	IEEE80211_WOWLAN_L2_ETHER_TYPE,
	IEEE80211_WOWLAN_L3_UDP_PORT,
	IEEE80211_WOWLAN_MAGIC_PATTERN,
	IEEE80211_WOWLAN_MAGIC_PATTERN_GET,
	IEEE80211_WOWLAN_SET_MAX
};
/*
 * Definitions relating to individual fields from phy_stats,
 * shared between the Q driver and the APIs.
 */

/*
 * Error Sum needs to be reported together with the corresponding Number of
 * Symbols; getting them in separate operations would introduce a race condition
 * where the Error Sum and the Number of Symbols came from different
 * PHY stat blocks.
 */

#define QTN_PHY_AVG_ERROR_SUM_NSYM_NAME			"avg_error_sum_nsym"

#define QTN_PHY_EVM_MANTISSA_SHIFT		5
#define QTN_PHY_EVM_EXPONENT_MASK		0x1f

enum qtn_phy_stat_field {
	QTN_PHY_NOSUCH_FIELD = -1,
	QTN_PHY_AVG_ERROR_SUM_NSYM_FIELD,
};

#define QTN_M2A_TX_SCALE_BITS	4
#define QTN_M2A_TX_SCALE_MASK	((1 << QTN_M2A_TX_SCALE_BITS) - 1)

/* only for little endian */
#if defined(AUC_BUILD)
#define U64_LOW32(_v)		((uint32_t)(_v))
#define U64_HIGH32(_v)		((uint32_t)((_v) >> 32))
#else
#define U64_LOW32(_v)		(((uint32_t*)&(_v))[0])
#define U64_HIGH32(_v)		(((uint32_t*)&(_v))[1])
#endif

#define U64_COMPARE_GE(_a, _b)	((U64_HIGH32(_a) > U64_HIGH32(_b)) ||	\
				((U64_HIGH32(_a) == U64_HIGH32(_b)) && (U64_LOW32(_a) >= U64_LOW32(_b))))

#define U64_COMPARE_GT(_a, _b)	((U64_HIGH32(_a) > U64_HIGH32(_b)) ||	\
				((U64_HIGH32(_a) == U64_HIGH32(_b)) && (U64_LOW32(_a) > U64_LOW32(_b))))

#define U64_COMPARE_LE(_a, _b)	((U64_HIGH32(_a) < U64_HIGH32(_b)) ||	\
				((U64_HIGH32(_a) == U64_HIGH32(_b)) && (U64_LOW32(_a) <= U64_LOW32(_b))))

#define U64_COMPARE_LT(_a, _b)	((U64_HIGH32(_a) < U64_HIGH32(_b)) ||	\
				((U64_HIGH32(_a) == U64_HIGH32(_b)) && (U64_LOW32(_a) < U64_LOW32(_b))))

#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"
#define MACSTRL "%02x:%02x:%02x:%02x:%02x:%02x"	/* for MuC and Auc which don't support "X" */
#endif

/*
 * VSP/QTM
 * Macro TOPAZ_QTM is used to help identify changes between original VSP and QTM.
 * In Lhost kernel driver, it must be used within CONFIG_QVSP(in kernel .config).
 * CONFIG_QVSP	TOPAZ_QTM	ruby		topaz
 * Y		1		invalid		*QTM works
 * Y		0		*VSP works	VSP alive but doesn't work for HDP
 * N		1		invalid		*no VSP/QTM
 * N		0		*no VSP		no VSP/QTM, and no QTM changes in MuC and AuC
 * So generally, sololy changing CONFIG_QVSP works for both ruby and topaz as indicated by *.
 * But to throughly clean QTM code in AuC and MuC, disable TOPAZ_QTM in topaz below.
 */
#ifdef TOPAZ_PLATFORM
	#define TOPAZ_QTM		1
#else
	#define TOPAZ_QTM		0
#endif

#define COMPILE_TIME_ASSERT(constant_expr)	\
do {						\
	switch(0) {				\
		case 0:				\
		case constant_expr:		\
		;				\
	}					\
} while(0)

/**@addtogroup DFSAPIs
 *@{*/
/**
 * Reason for channel change
 */
enum ieee80211_csw_reason {
	/**
	 * Reason is unknown
	 */
	IEEE80211_CSW_REASON_UNKNOWN,
	/**
	 * Smart channel selection
	 */
	IEEE80211_CSW_REASON_SCS,
	/**
	 * Radar detection
	 */
	IEEE80211_CSW_REASON_DFS,
	/**
	 * Channel set by user
	 */
	IEEE80211_CSW_REASON_MANUAL,
	/**
	 * Configuration change
	 */
	IEEE80211_CSW_REASON_CONFIG,
	/**
	 * Scan initiated by user
	 */
	IEEE80211_CSW_REASON_SCAN,
	/**
	 * Off-channel CAC
	 */
	IEEE80211_CSW_REASON_OCAC,
	/**
	 * Channel switch announcement
	 */
	IEEE80211_CSW_REASON_CSA,
	/**
	 * TDLS Channel switch announcement
	 */
	IEEE80211_CSW_REASON_TDLS_CS,
	/**
	 * Number of values
	 */
	IEEE80211_CSW_REASON_MAX
};
/**@}*/

/*
 * Reasons for channel switches that are not recorded and therefore
 * should not be listed in QCSAPI documentation
 */
enum ieee80211_csw_reason_private {
	IEEE80211_CSW_REASON_SAMPLING = IEEE80211_CSW_REASON_MAX,
	IEEE80211_CSW_REASON_OCAC_RUN,
	IEEE80211_CSW_REASON_BGSCAN,
};

/* Keep this in sync with swfeat_desc */
enum swfeat {
	SWFEAT_ID_MODE_AP,
	SWFEAT_ID_MODE_STA,
	SWFEAT_ID_MODE_REPEATER,
	SWFEAT_ID_PCIE_RC,
	SWFEAT_ID_VHT,
	SWFEAT_ID_2X2,
	SWFEAT_ID_2X4,
	SWFEAT_ID_4X4,
	SWFEAT_ID_HS20,
	SWFEAT_ID_WPA2_ENT,
	SWFEAT_ID_MESH,
	SWFEAT_ID_TDLS,
	SWFEAT_ID_OCAC,
	SWFEAT_ID_QHOP,
	SWFEAT_ID_QSV,
	SWFEAT_ID_QSV_NEIGH,
	SWFEAT_ID_MU_MIMO,
	SWFEAT_ID_DUAL_CHAN_VIRT,
	SWFEAT_ID_DUAL_CHAN,
	SWFEAT_ID_DUAL_BAND_VIRT,
	SWFEAT_ID_DUAL_BAND,
	SWFEAT_ID_QTM_PRIO,
	SWFEAT_ID_QTM,
	SWFEAT_ID_SPEC_ANALYZER,
	SWFEAT_ID_MAX
};

#define SWFEAT_MAP_SIZE (SWFEAT_ID_MAX / 8 + 1)

/* Used to scale temperature measurements */
#define QDRV_TEMPSENS_COEFF    100000
#define QDRV_TEMPSENS_COEFF10  (10 * QDRV_TEMPSENS_COEFF)

#endif /* _SHARED_DEFS_H_ */
