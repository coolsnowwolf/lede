/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name:
    cmm_rdm_mt.c//Jelly20140123
*/

#ifdef MT_DFS_SUPPORT
/* Remember add RDM compiler flag - Shihwei20141104 */
#include "rt_config.h"
#include "hdev/hdev.h"
#include "wlan_config/config_internal.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
DFS_RADAR_THRESHOLD_PARAM g_arRadarThresholdParam[4] = {
	/* CE */
	{
		/* Pulse threshold parameters DFS_PULSE_THRESHOLD_PARAM */
		{40, -10, -80, 800, 3360, 128, 5200},
		/* Valid RADAR Types BOOLEAN*/
		{1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
		{
			/* RADAR detection threshold values SW_RADAR_TYPE_T*/
/* ETSI-1 */		{0, 1, 0, 6,  PPB_SIZE, 28, 0, 17, 990, 5010, 1, 1, 0, 0, 0},
/* ETSI-2 */		{0, 1, 0, 9,  PPB_SIZE, 28, 0, 27, 615, 5010, 1, 1, 0, 0, 0},
/* ETSI-3 */		{0, 1, 0, 15, PPB_SIZE, 28, 0, 27, 240, 445,  1, 1, 0, 0, 0},
/* ETSI-4 */		{0, 1, 0, 12, PPB_SIZE, 28, 0, 42, 240, 510,  1, 1, 0, 0, 0},
/* ETSI-5, 2PRI */	{0, 1, 1, 0, 0, 0, 0, 14, 2490, 3343, 0, 0, 12, 32, 28},
/* ETSI-5, 3PRI */	{0, 1, 1, 0, 0, 0, 0, 14, 2490, 3343, 0, 0, 15, 32, 24},
/* ETSI-6, 2PRI */	{0, 1, 1, 0, 0, 0, 0, 14,  823, 2510, 0, 0, 18, 32, 28},
/* ETSI-6, 3PRI */	{0, 1, 1, 0, 0, 0, 0, 14,  823, 2510, 0, 0, 27, 32, 24},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
		}
	},
	/* FCC */
	{
		/* Pulse threshold parameters DFS_PULSE_THRESHOLD_PARAM */
		{40, -10, -80, 800, 3360, 128, 5200},
		/* Valid RADAR Types BOOLEAN*/
		{1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{
			/* RADAR detection threshold values SW_RADAR_TYPE_T*/
/* FCC-0,1 */		{0, 1, 0, 9,  PPB_SIZE, 28, 0, 13, 508, 3076, 1, 1, 0, 0, 0},
/* FCC -2 */		{0, 1, 0, 12, PPB_SIZE, 28, 0, 17, 140, 240,  1, 1, 0, 0, 0},
/* FCC -3 */		{0, 1, 0, 8,  PPB_SIZE, 28, 0, 22, 190, 510,  1, 1, 0, 0, 0},
/* FCC -4 */		{0, 1, 0, 6,  PPB_SIZE, 28, 0, 32, 190, 510,  1, 1, 0, 0, 0},
/* FCC -6 */		{0, 1, 0, 9,  255,		28, 0, 13, 323, 343,  1, 32, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
		}
	},
	/* JAPAN */
	{
		/* Pulse threshold parameters DFS_PULSE_THRESHOLD_PARAM */
		{40, -10, -80, 800, 3360, 128, 5200},
		/* Valid RADAR Types BOOLEAN*/
		{1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{
			/* RADAR detection threshold values SW_RADAR_TYPE_T*/
/* FCC-0,1, W53-Fixed Pulse 1, W56 Fixed Pulse 1,2 */
					{0, 1, 0, 8,  PPB_SIZE, 28, 0, 13, 508, 3076, 1, 1, 0, 0, 0},
/* FCC -2, W56 Variable Pulse 4 */
					{0, 1, 0, 12, PPB_SIZE, 28, 0, 17, 140, 240,  1, 1, 0, 0, 0},
/* FCC -3, W56 Variable Pulse 5 */
					{0, 1, 0, 8,  PPB_SIZE, 28, 0, 22, 190, 510,  1, 1, 0, 0, 0},
/* FCC -4, W56 Variable Pulse 6 */
					{0, 1, 0, 6,  PPB_SIZE, 28, 0, 32, 190, 510,  1, 1, 0, 0, 0},
/* FCC type-6, W56 hopping */
					{0, 1, 0, 9,  PPB_SIZE, 28, 0, 13, 323, 343,  1, 32, 0, 0, 0},
/* W53 Fixed Pulse 2 */
					{0, 1, 0, 8,  PPB_SIZE, 28, 0, 14, 3836, 3856, 1, 1, 0, 0, 0},
/* W56 Fixed Pulse 3 */
					{0, 1, 0, 8,  PPB_SIZE, 28, 0, 14, 3990, 4010, 1, 1, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
		}
	},
	/* ALL AREA */
	{
		/* Pulse threshold parameters DFS_PULSE_THRESHOLD_PARAM */
		{40, -10, -80, 800, 3360, 128, 5200},
		/* Valid RADAR Types BOOLEAN*/
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
		{
			/* RADAR detection threshold values SW_RADAR_TYPE_T*/
/* FCC-0,1	*/		{0, 1, 0, 8,  PPB_SIZE, 28, 0, 13, 508, 3076, 1, 1, 0, 0, 0},
/* FCC-2	*/		{0, 1, 0, 12, PPB_SIZE, 28, 0, 17, 140, 240,  1, 1, 0, 0, 0},
/* FCC-3	*/		{0, 1, 0, 8,  PPB_SIZE, 28, 0, 22, 190, 510,  1, 1, 0, 0, 0},
/* FCC-4	*/		{0, 1, 0, 6,  PPB_SIZE, 28, 0, 32, 190, 510,  1, 1, 0, 0, 0},
/* FCC-6	*/		{0, 1, 0, 9,  255,		28, 0, 13, 323, 343,  1, 32, 0, 0, 0},
/* ETSI-1	*/		{0, 1, 0, 6,  PPB_SIZE, 28, 0, 17, 990, 5010, 1, 1, 0, 0, 0},
/* ETSI-2	*/		{0, 1, 0, 9,  PPB_SIZE, 28, 0, 27, 615, 5010, 1, 1, 0, 0, 0},
/* ETSI-3	*/		{0, 1, 0, 15, PPB_SIZE, 28, 0, 27, 240, 445, 1, 1, 0, 0, 0},
/* ETSI-4	*/		{0, 1, 0, 12, PPB_SIZE, 28, 0, 42, 240, 510, 1, 1, 0, 0, 0},
/* ETSI-5, 2PRI*/	{0, 1, 1, 0, 0, 0, 0, 14, 2490, 3343, 0, 0, 12, PPB_SIZE, 28},
/* ETSI-5, 3PRI*/	{0, 1, 1, 0, 0, 0, 0, 14, 2490, 3343, 0, 0, 15, PPB_SIZE, 24},
/* ETSI-6, 2PRI*/	{0, 1, 1, 0, 0, 0, 0, 14, 823,	2510, 0, 0, 18, PPB_SIZE, 28},
/* ETSI-6, 3PRI*/	{0, 1, 1, 0, 0, 0, 0, 14, 823,	2510, 0, 0, 27, PPB_SIZE, 24},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* Reserved */		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
		}
	}
};
#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

#ifdef RDM_FALSE_ALARM_DEBUG_SUPPORT
EXT_EVENT_RDD_REPORT_T rRadarInfo[HW_RDD_NUM];
#endif /* RDM_FALSE_ALARM_DEBUG_SUPPORT */

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
typedef int (*_k_ARC_ZeroWait_DFS_collision_report_callback_fun_type) (UCHAR SyncNum, UCHAR monitored_Ch, UCHAR Bw);
typedef int (*_k_ARC_ZeroWait_DFS_NOP_Timeout_report_callback_fun_type) (UCHAR Bw80ChNum, PDFS_REPORT_AVALABLE_CH_LIST pBw80AvailableChList, UCHAR Bw40ChNum, PDFS_REPORT_AVALABLE_CH_LIST pBw40AvailableChList, UCHAR Bw20ChNum, PDFS_REPORT_AVALABLE_CH_LIST pBw20AvailableChList);

_k_ARC_ZeroWait_DFS_collision_report_callback_fun_type radar_detected_callback_func;
_k_ARC_ZeroWait_DFS_CAC_Time_Meet_report_callback_fun_type DfsCacTimeOutCallBack;
_k_ARC_ZeroWait_DFS_NOP_Timeout_report_callback_fun_type DfsNopTimeOutCallBack;

void k_ZeroWait_DFS_Collision_Report_Callback_Function_Registeration(_k_ARC_ZeroWait_DFS_collision_report_callback_fun_type callback_detect_collision_func)
{
    radar_detected_callback_func = callback_detect_collision_func;
}

void k_ZeroWait_DFS_CAC_Time_Meet_Report_Callback_Function_Registeration(_k_ARC_ZeroWait_DFS_CAC_Time_Meet_report_callback_fun_type callback_CAC_time_meet_func)
{
    DfsCacTimeOutCallBack = callback_CAC_time_meet_func;
}

void k_ZeroWait_DFS_NOP_Timeout_Report_Callback_Function_Registeration(_k_ARC_ZeroWait_DFS_NOP_Timeout_report_callback_fun_type callback_NOP_Timeout_func)
{
    DfsNopTimeOutCallBack = callback_NOP_Timeout_func;
}

EXPORT_SYMBOL(k_ZeroWait_DFS_Collision_Report_Callback_Function_Registeration);
EXPORT_SYMBOL(k_ZeroWait_DFS_CAC_Time_Meet_Report_Callback_Function_Registeration);
EXPORT_SYMBOL(k_ZeroWait_DFS_NOP_Timeout_Report_Callback_Function_Registeration);

static VOID ZeroWaitDfsEnable(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR bZeroWaitDfsCtrl;

	bZeroWaitDfsCtrl = msg->zerowait_dfs_ctrl_msg.Enable;

#ifdef BACKGROUND_SCAN_SUPPORT
	DfsDedicatedDynamicCtrl(pAd, bZeroWaitDfsCtrl);
#endif
}

static VOID ZeroWaitDfsInitAvalChListUpdate(
    PRTMP_ADAPTER pAd,
    union dfs_zero_wait_msg *msg
)
{
	UCHAR Bw80TotalChNum;
	UCHAR Bw40TotalChNum;
	UCHAR Bw20TotalChNum;
	DFS_REPORT_AVALABLE_CH_LIST Bw80AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw40AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw20AvalChList[DFS_AVAILABLE_LIST_CH_NUM];

	Bw80TotalChNum = msg->aval_channel_list_msg.Bw80TotalChNum;
	Bw40TotalChNum = msg->aval_channel_list_msg.Bw40TotalChNum;
	Bw20TotalChNum = msg->aval_channel_list_msg.Bw20TotalChNum;

	memcpy(Bw80AvalChList,
		msg->aval_channel_list_msg.Bw80AvalChList,
		Bw80TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	memcpy(Bw40AvalChList,
		msg->aval_channel_list_msg.Bw40AvalChList,
		Bw40TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	memcpy(Bw20AvalChList,
		msg->aval_channel_list_msg.Bw20AvalChList,
		Bw20TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw20ChNum: %d\n", Bw20TotalChNum));
#ifdef DFS_DBG_LOG_0
	for (i = 0; i < Bw20TotalChNum; i++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw20 ChList[%d] Channel:%d\n",
			i, Bw20AvalChList[i].Channel));
	}
#endif
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw40ChNum: %d\n", Bw40TotalChNum));

#ifdef DFS_DBG_LOG_0
	for (i = 0; i < Bw40TotalChNum; i++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw40 ChList[%d] Channel:%d\n",
			i, Bw40AvalChList[i].Channel));
	}
#endif

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw80ChNum: %d\n", Bw80TotalChNum));

#ifdef DFS_DBG_LOG_0
	for (i = 0; i < Bw80TotalChNum; i++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw80 ChList[%d] Channel:%d\n",
			i, Bw80AvalChList[i].Channel));
	}
#endif
	ZeroWait_DFS_Initialize_Candidate_List(pAd,
	Bw80TotalChNum, Bw80AvalChList,
	Bw40TotalChNum, Bw40AvalChList,
	Bw20TotalChNum, Bw20AvalChList);
}

static VOID ZeroWaitDfsMonitorChUpdate(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR SynNum;
	UCHAR Channel;
	UCHAR Bw;
	BOOLEAN doCAC;

	SynNum = msg->set_monitored_ch_msg.SyncNum;
	Channel = msg->set_monitored_ch_msg.Channel;
	Bw = msg->set_monitored_ch_msg.Bw;
	doCAC = msg->set_monitored_ch_msg.doCAC;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] SynNum: %d, Channel: %d, Bw: %d \x1b[m \n",
	__FUNCTION__, SynNum, Channel, Bw));

#ifdef BACKGROUND_SCAN_SUPPORT
	if (SynNum == RDD_BAND0)
		DfsDedicatedInBandSetChannel(pAd, Channel, Bw, doCAC);
	else
		DfsDedicatedOutBandSetChannel(pAd, Channel, Bw);
#endif

}

static VOID ZeroWaitDfsSetNopToChList(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR Channel = 0, Bw = 0;
	USHORT NOPTime = 0;

	Channel = msg->nop_force_set_msg.Channel;
	Bw = msg->nop_force_set_msg.Bw;
	NOPTime = msg->nop_force_set_msg.NOPTime;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Channel: %d, Bw: %d, NOP: %d \x1b[m \n",
	__FUNCTION__, Channel, Bw, NOPTime));

	ZeroWait_DFS_set_NOP_to_Channel_List(pAd, Channel, Bw, NOPTime);

}

static VOID ZeroWaitDfsPreAssignNextTarget(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR Channel;
	UCHAR Bw;
	USHORT CacValue;

	Channel = msg->assign_next_target.Channel;
	Bw = msg->assign_next_target.Bw;
	CacValue = msg->assign_next_target.CacValue;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Channel: %d, Bw: %d \x1b[m \n",
	__FUNCTION__, Channel, Bw));

	ZeroWait_DFS_Pre_Assign_Next_Target_Channel(pAd, Channel, Bw, CacValue);
}

static VOID ZeroWaitShowTargetInfo(
	PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg
)
{
	UCHAR mode;

	mode = msg->target_ch_show.mode;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] mode: %d \x1b[m \n",
	__FUNCTION__, mode));

	ZeroWait_DFS_Next_Target_Show(pAd, mode);
}

static VOID ZeroWaitDfsMsgHandle(
	PRTMP_ADAPTER pAd,
	UCHAR *msg
)
{
	switch (*msg) {
	case ZERO_WAIT_DFS_ENABLE:
		ZeroWaitDfsEnable(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case INIT_AVAL_CH_LIST_UPDATE:
		ZeroWaitDfsInitAvalChListUpdate(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case MONITOR_CH_ASSIGN:
		ZeroWaitDfsMonitorChUpdate(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case NOP_FORCE_SET:
		ZeroWaitDfsSetNopToChList(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case PRE_ASSIGN_NEXT_TARGET:
		ZeroWaitDfsPreAssignNextTarget(pAd, (union dfs_zero_wait_msg *)msg);
		break;

	case SHOW_TARGET_INFO:
		ZeroWaitShowTargetInfo(pAd, (union dfs_zero_wait_msg *)msg);
		break;
	default:
		break;
	}
}

INT ZeroWaitDfsCmdHandler(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT * wrq
)
{
	INT status = NDIS_STATUS_SUCCESS;
	union dfs_zero_wait_msg msg;

	if (!wrq)
		return NDIS_STATUS_FAILURE;

	if (copy_from_user(&msg, wrq->u.data.pointer, wrq->u.data.length)) {
		status = -EFAULT;
	} else {
		ZeroWaitDfsMsgHandle(pAd, (CHAR *)&msg);
	}

	return status;
}

static VOID ZeroWaitDfsQueryNopOfChList(
    PRTMP_ADAPTER pAd,
    union dfs_zero_wait_msg *msg
)
{
	UCHAR i = 0;
	DfsProvideNopOfChList(pAd, msg);
	for (i = 0; i < msg->nop_of_channel_list_msg.NOPTotalChNum; i++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("NopReportChList[%d].Channel = %d, Bw = %d, NOP = %d\n",
		i, msg->nop_of_channel_list_msg.NopReportChList[i].Channel, msg->nop_of_channel_list_msg.NopReportChList[i].Bw,
		msg->nop_of_channel_list_msg.NopReportChList[i].NonOccupancy));
	}

}

static VOID ZeroWaitDfsQueryAvalChList(
    PRTMP_ADAPTER pAd,
    union dfs_zero_wait_msg *msg
)
{
	UINT_8 i, j, idx;

	UCHAR Bw80TotalChNum;
	UCHAR Bw40TotalChNum;
	UCHAR Bw20TotalChNum;
	DFS_REPORT_AVALABLE_CH_LIST Bw80AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw40AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw20AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];

	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {
		for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
			pDfsParam->AvailableBwChIdx[i][j] = 0xff;
	}

	for (i = 0; i < DBDC_BAND_NUM; i++) {
		if (pAd->Dot11_H[i].RDMode == RD_SWITCHING_MODE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel Lsit query fail during channel switch\n"));
			return;
		}
	}

	DfsBwChQueryAllList(pAd, BW_80, pDfsParam, TRUE);

	for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++) {
		if (pDfsParam->AvailableBwChIdx[BW_20][j] != 0xff) {
			idx = pDfsParam->AvailableBwChIdx[BW_20][j];
			Bw20AvailableChList[j].Channel = pDfsParam->DfsChannelList[idx].Channel;
			Bw20AvailableChList[j].RadarHitCnt = pDfsParam->DfsChannelList[idx].NOPClrCnt;
		} else
			break;
	}
	Bw20TotalChNum = j;

	for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++) {
		if (pDfsParam->AvailableBwChIdx[BW_40][j] != 0xff) {
			idx = pDfsParam->AvailableBwChIdx[BW_40][j];
			Bw40AvailableChList[j].Channel = pDfsParam->DfsChannelList[idx].Channel;
			Bw40AvailableChList[j].RadarHitCnt = pDfsParam->DfsChannelList[idx].NOPClrCnt;
		} else
			break;
	}
	Bw40TotalChNum = j;

	for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++) {
		if (pDfsParam->AvailableBwChIdx[BW_80][j] != 0xff) {
			idx = pDfsParam->AvailableBwChIdx[BW_80][j];
			Bw80AvailableChList[j].Channel = pDfsParam->DfsChannelList[idx].Channel;
			Bw80AvailableChList[j].RadarHitCnt = pDfsParam->DfsChannelList[idx].NOPClrCnt;
		} else
			break;
	}
	Bw80TotalChNum = j;

	msg->aval_channel_list_msg.Bw80TotalChNum = Bw80TotalChNum;
	msg->aval_channel_list_msg.Bw40TotalChNum = Bw40TotalChNum;
	msg->aval_channel_list_msg.Bw20TotalChNum = Bw20TotalChNum;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw20ChNum: %d\n", msg->aval_channel_list_msg.Bw20TotalChNum));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw40ChNum: %d\n", msg->aval_channel_list_msg.Bw40TotalChNum));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw80ChNum: %d\n", msg->aval_channel_list_msg.Bw80TotalChNum));

	memcpy(msg->aval_channel_list_msg.Bw80AvalChList,
		Bw80AvailableChList,
		Bw80TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	memcpy(msg->aval_channel_list_msg.Bw40AvalChList,
		Bw40AvailableChList,
		Bw40TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);

	memcpy(msg->aval_channel_list_msg.Bw20AvalChList,
		Bw20AvailableChList,
		Bw20TotalChNum * sizeof(DFS_REPORT_AVALABLE_CH_LIST)
	);
	for (idx = 0; i < msg->aval_channel_list_msg.Bw80TotalChNum; idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
		idx, pDfsParam->DfsChannelList[idx].Channel,
		pDfsParam->DfsChannelList[idx].NOPClrCnt));
	}
	for (idx = 0; i < msg->aval_channel_list_msg.Bw40TotalChNum; idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
		idx, pDfsParam->DfsChannelList[idx].Channel,
		pDfsParam->DfsChannelList[idx].NOPClrCnt));
	}
	for (idx = 0; i < msg->aval_channel_list_msg.Bw20TotalChNum; idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
		idx, pDfsParam->DfsChannelList[idx].Channel,
		pDfsParam->DfsChannelList[idx].NOPClrCnt));
	}
}

static VOID ZeroWaitDfsQueryMsgHandle(
	PRTMP_ADAPTER pAd,
	UCHAR *msg
)
{
	switch (*msg) {
	case QUERY_AVAL_CH_LIST:
		ZeroWaitDfsQueryAvalChList(pAd, (union dfs_zero_wait_msg *)msg);
		break;
	case QUERY_NOP_OF_CH_LIST:
		ZeroWaitDfsQueryNopOfChList(pAd, (union dfs_zero_wait_msg *)msg);
		break;
	default:
		break;
	}
}

INT ZeroWaitDfsQueryCmdHandler(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT * wrq
)
{
	INT status = NDIS_STATUS_SUCCESS;
	union dfs_zero_wait_msg msg;
	os_zero_mem(&msg, sizeof(union dfs_zero_wait_msg));
	ZeroWaitDfsQueryMsgHandle(pAd, (CHAR *)&msg);
	wrq->u.data.length = sizeof(union dfs_zero_wait_msg);

	if (copy_to_user(wrq->u.data.pointer, &msg, wrq->u.data.length)) {
		status = -EFAULT;
	}

	return status;
}

static inline BOOLEAN AutoChannelSkipListCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	UCHAR i;

	for (i = 0; i < pAd->ApCfg.AutoChannelSkipListNum; i++) {
		if (Ch == pAd->ApCfg.AutoChannelSkipList[i])
			return TRUE;
	}

	return FALSE;
}

static inline UCHAR CentToPrim(
	UCHAR Channel)
{
	return Channel - 2;
}

static BOOLEAN DfsCheckChAvailableByBw(
	UCHAR Channel, UCHAR Bw, PDFS_PARAM pDfsParam)
{
	UCHAR i = 0, j = 0, k = 0;
	UCHAR *pBwChGroup = NULL;
	UCHAR BW40_CH_GROUP[][2] = {
	{36, 40}, {44, 48},
	{52, 56}, {60, 64},
	{100, 104}, {108, 112},
	{116, 120}, {124, 128},
	{132, 136}, {140, 144},
	{149, 153}, {157, 161}, {0, 0}
	};

	UCHAR BW80_CH_GROUP[][4] = {
	{36, 40, 44, 48},
	{52, 56, 60, 64},
	{100, 104, 108, 112},
	{116, 120, 124, 128},
	{132, 136, 140, 144},
	{149, 153, 157, 161},
	{0, 0, 0, 0}
	};

	UCHAR BW160_CH_GROUP[][8] = {
	{36, 40, 44, 48, 52, 56, 60, 64},
	{100, 104, 108, 112, 116, 120, 124, 128}
	};

	if (Bw == BW_20)
		return TRUE;
	else if (Bw == BW_40) {
		pBwChGroup = &BW40_CH_GROUP[0][0];
		while (*pBwChGroup != 0) {
			if (*pBwChGroup == Channel)
				break;
			i++;
			if (i > sizeof(BW40_CH_GROUP))
				return FALSE;
			pBwChGroup++;
		}
		for (j = 0; j < pDfsParam->ChannelListNum; j++) {
			if (pDfsParam->DfsChannelList[j].Channel == BW40_CH_GROUP[i/2][0])
				break;
		}

		if (j == pDfsParam->ChannelListNum)
			return FALSE;
		else if (pDfsParam->DfsChannelList[j+1].Channel == BW40_CH_GROUP[i/2][1])
			return TRUE;
	} else if (Bw == BW_80 || Bw == BW_8080) {
		pBwChGroup = &BW80_CH_GROUP[0][0];
		while (*pBwChGroup != 0) {
			if (*pBwChGroup == Channel)
				break;
			i++;
			if (i > sizeof(BW80_CH_GROUP))
				return FALSE;
			pBwChGroup++;
		}
		for (j = 0; j < pDfsParam->ChannelListNum; j++) {
			if (pDfsParam->DfsChannelList[j].Channel == BW80_CH_GROUP[i/4][0])
				break;
		}
		if (j == pDfsParam->ChannelListNum)
			return FALSE;
		else if ((pDfsParam->DfsChannelList[j+1].Channel == BW80_CH_GROUP[i/4][1])
			&& (pDfsParam->DfsChannelList[j+2].Channel == BW80_CH_GROUP[i/4][2])
			&& (pDfsParam->DfsChannelList[j+3].Channel == BW80_CH_GROUP[i/4][3])
		)
			return TRUE;

	} else if (Bw == BW_160) {
		pBwChGroup = &BW160_CH_GROUP[0][0];
		while (*pBwChGroup != 0) {
			if (*pBwChGroup == Channel)
				break;
			i++;
			if (i > sizeof(BW160_CH_GROUP))
				return FALSE;
			pBwChGroup++;
		}
		for (j = 0; j < pDfsParam->ChannelListNum; j++) {
			if (pDfsParam->DfsChannelList[j].Channel == BW160_CH_GROUP[i/8][0])
				break;
		}
		if (j == pDfsParam->ChannelListNum)
			return FALSE;
		else {
			for (k = 1; k < 7 ; k++) {
				if (pDfsParam->DfsChannelList[j+k].Channel != BW160_CH_GROUP[i/8][k])
					return FALSE;
			}
			return TRUE;
		}
	}

	return FALSE;
}

static BOOLEAN ByPassChannelByBw(
	UCHAR Channel, UCHAR Bw, PDFS_PARAM pDfsParam)
{
	UINT_8 i;
	BOOLEAN BwSupport = FALSE;

	for (i = 0; i < pDfsParam->ChannelListNum; i++) {
		if (Channel == pDfsParam->DfsChannelList[i].Channel) {
			if (Bw == BW_8080) {
				BwSupport = (pDfsParam->DfsChannelList[i].SupportBwBitMap) & BIT(BW_80);
			} else {
				BwSupport = (pDfsParam->DfsChannelList[i].SupportBwBitMap) & BIT(Bw);
			}
		}
	}

	if (BwSupport)
		return FALSE;
	else
		return TRUE;

}

UCHAR DfsPrimToCent(
	UCHAR Channel, UCHAR Bw)
{
	UINT_8 i = 0;

	UCHAR CH_EXT_ABOVE[] = {
	36, 44, 52, 60,
	100, 108, 116, 124,
	132, 140, 149, 157, 0
	};

	UCHAR CH_EXT_BELOW[] = {
	40, 48, 56, 64,
	104, 112, 120, 128,
	136, 144, 153, 161, 0
	};

	if (Bw == BW_20)
		return Channel;
	else if (Bw == BW_40) {
		while (CH_EXT_ABOVE[i] != 0) {
			if (Channel == CH_EXT_ABOVE[i]) {
				return Channel + 2;
			} else if (Channel == CH_EXT_BELOW[i]) {
				return Channel - 2;
			}
			i++;
		}
	} else if (Bw == BW_80 || Bw == BW_8080)
		return vht_cent_ch_freq(Channel, VHT_BW_80);
	else if (Bw == BW_160)
		return vht_cent_ch_freq(Channel, VHT_BW_160);

	return Channel;
}

UCHAR DfsGetBgndParameter(
	IN PRTMP_ADAPTER pAd, UCHAR QueryParam)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	switch (QueryParam) {
	case INBAND_CH:
		return pDfsParam->Band0Ch;
		break;
	case INBAND_BW:
		return pDfsParam->Bw;
		break;
	case OUTBAND_CH:
		return pDfsParam->OutBandCh;
		break;
	case OUTBAND_BW:
		return pDfsParam->OutBandBw;
		break;
	case ORI_INBAND_CH:
		return pDfsParam->OrigInBandCh;
		break;
	case ORI_INBAND_BW:
		return pDfsParam->OrigInBandBw;
		break;
	default:
		return pDfsParam->Band0Ch;
		break;
	}
}

VOID DfsGetSysParameters(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR vht_cent2,
	UCHAR phy_bw)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR prim_ch;
	UCHAR bandIdx;
	CHANNEL_CTRL *pChCtrl;
#ifdef DOT11_VHT_AC
	UCHAR c2;
#endif /*DOT11_VHT_AC*/

	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	prim_ch = wdev->channel;
	bandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, bandIdx);

#ifdef DOT11_VHT_AC
	c2 = vht_cent2;
	if (phy_bw == BW_8080) {
		pDfsParam->PrimCh = prim_ch;

		if (prim_ch < CentToPrim(c2))
			pDfsParam->PrimBand = RDD_BAND0;
		else
			pDfsParam->PrimBand = RDD_BAND1;
		pDfsParam->Band0Ch = (pDfsParam->PrimBand == RDD_BAND0) ? prim_ch : CentToPrim(c2);
		pDfsParam->Band1Ch = (pDfsParam->PrimBand == RDD_BAND0) ? CentToPrim(c2) : prim_ch;
	} else
#endif
	{
		pDfsParam->PrimCh = prim_ch;
		pDfsParam->PrimBand = RDD_BAND0;
		pDfsParam->Band0Ch = prim_ch;
		pDfsParam->Band1Ch = 0;
	}

	pDfsParam->Bw = phy_bw;
	pDfsParam->Dot11_H[bandIdx].RDMode = pDot11h->RDMode;
	pDfsParam->bIEEE80211H = pAd->CommonCfg.bIEEE80211H;
	pDfsParam->bDBDCMode = pAd->CommonCfg.dbdc_mode;
	pDfsParam->bDfsEnable = pAd->CommonCfg.DfsParameter.bDfsEnable;
}


VOID DfsParamInit(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR i;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->PrimBand = RDD_BAND0;
	pDfsParam->DfsChBand[0] = FALSE; /* Smaller channel */
	pDfsParam->DfsChBand[1] = FALSE; /* Larger channel number */
	pDfsParam->RadarDetected[0] = FALSE; /* Smaller channel number */
	pDfsParam->RadarDetected[1] = FALSE; /* larger channel number */
	pDfsParam->RadarDetectState = FALSE;
	if ((pDfsParam->NeedSetNewChList == DFS_SET_NEWCH_INIT)
	|| (pDfsParam->ChannelListNum == 0))
		pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_ENABLED;
	else
		pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_DISABLED;
	pDfsParam->bDfsCheck = FALSE;
	pDfsParam->bNoSwitchCh = FALSE;
	pDfsParam->bShowPulseInfo = FALSE;
	pDfsParam->bNoAvailableCh = FALSE;
	pDfsParam->bZeroWaitCacSecondHandle = FALSE;
	pDfsParam->bDBDCMode = pAd->CommonCfg.dbdc_mode;
	pDfsParam->DbdcAdditionCh = 0;
	pDfsParam->bDedicatedZeroWaitSupport = FALSE;
#ifdef ONDEMAND_DFS
	pDfsParam->bOnDemandZeroWaitSupport = FALSE;
	pDfsParam->bOnDemandChannelListValid = FALSE;
#endif

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	pDfsParam->bV10ChannelListValid = FALSE;
	pDfsParam->bV10BootACSValid = FALSE;
	pDfsParam->gV10OffChnlWaitTime = 0;
	pDfsParam->bV10W56APDownEnbl = FALSE;
	pDfsParam->bV10APBcnUpdateEnbl =  FALSE;
	pDfsParam->bV10W56GrpValid = FALSE;
	pDfsParam->bV10APInterfaceDownEnbl = FALSE;
	pDfsParam->bV10W56SwitchVHT80 = FALSE;
#endif

	pDfsParam->OutBandCh = 0;
	pDfsParam->OutBandBw = 0;
	pDfsParam->bZeroWaitSupport = 0;
	pDfsParam->bOutBandAvailable = FALSE;
	pDfsParam->DedicatedOutBandCacCount = 0;
	pDfsParam->bSetInBandCacReStart = FALSE;
	pDfsParam->bDedicatedZeroWaitDefault = FALSE;
	pDfsParam->bInitOutBandBranch = FALSE;
	pDfsParam->RadarHitReport = FALSE;
	pDfsParam->OutBandAvailableCh = 0;
	pDfsParam->targetCh = 0;
	pDfsParam->targetBw = 0;
	pDfsParam->targetCacValue = 0;

#ifdef RDM_FALSE_ALARM_DEBUG_SUPPORT
	pDfsParam->fgRadarEmulate = FALSE;
#endif /* RDM_FALSE_ALARM_DEBUG_SUPPORT */

#ifdef CONFIG_RCSA_SUPPORT
	pDfsParam->fSendRCSA = FALSE;
	pDfsParam->ChSwMode = 1;
#endif
	for (i = 0; i < DBDC_BAND_NUM; i++)
		pAd->Dot11_H[i].DfsZeroWaitChMovingTime = 3;
	DfsStateMachineInit(pAd, &pAd->CommonCfg.DfsParameter.DfsStatMachine, pAd->CommonCfg.DfsParameter.DfsStateFunc);
}

#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
VOID DfsThresholdParamInit(
		IN PRTMP_ADAPTER	pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	PDFS_RADAR_THRESHOLD_PARAM prRadarThresholdParam = &pDfsParam->rRadarThresholdParam;

	/* Initialise threshold parameters*/
	pDfsParam->fgHwRDDLogEnable = FALSE;
	pDfsParam->fgSwRDDLogEnable = FALSE;
	pDfsParam->fgSwRDDLogCond = TRUE;
	pDfsParam->fgRadarEmulate = FALSE;
	pDfsParam->u2FCC_LPN_MIN = 8;

	if (pDfsParam->fgRDRegionConfigured == TRUE) {
			switch (pAd->CommonCfg.RDDurRegion) {
			case CE:
				NdisCopyMemory(prRadarThresholdParam, &g_arRadarThresholdParam[CE], sizeof(DFS_RADAR_THRESHOLD_PARAM));
				break;
			case FCC:
				NdisCopyMemory(prRadarThresholdParam, &g_arRadarThresholdParam[FCC], sizeof(DFS_RADAR_THRESHOLD_PARAM));
				break;
			case JAP:
			case JAP_W53:
			case JAP_W56:
				NdisCopyMemory(prRadarThresholdParam, &g_arRadarThresholdParam[JAP], sizeof(DFS_RADAR_THRESHOLD_PARAM));
				break;
			default: /* All Area */
				NdisCopyMemory(prRadarThresholdParam, &g_arRadarThresholdParam[3], sizeof(DFS_RADAR_THRESHOLD_PARAM));
		}
	} else /* All Area */
		NdisCopyMemory(prRadarThresholdParam, &g_arRadarThresholdParam[3], sizeof(DFS_RADAR_THRESHOLD_PARAM));
}
#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */

VOID DfsStateMachineInit(
	IN RTMP_ADAPTER * pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, DFS_MAX_STATE, DFS_MAX_MSG, (STATE_MACHINE_FUNC)Drop, DFS_BEFORE_SWITCH, DFS_MACHINE_BASE);
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_CAC_END, (STATE_MACHINE_FUNC)DfsCacEndUpdate);
#ifdef ONDEMAND_DFS
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_ONDEMAND_CAC_FINISH, (STATE_MACHINE_FUNC)DfsOutBandCacPass);
#endif
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_V10_W56_APDOWN_ENBL, (STATE_MACHINE_FUNC)DfsV10W56APDownEnbl);
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_V10_W56_APDOWN_FINISH, (STATE_MACHINE_FUNC)DfsV10W56APDownPass);
	StateMachineSetAction(Sm, DFS_BEFORE_SWITCH, DFS_V10_ACS_CSA_UPDATE, (STATE_MACHINE_FUNC)DfsV10APBcnUpdate);
#endif
}

INT Set_RadarDetectStart_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	ULONG value, ret1, ret2;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	struct freq_oper oper;
	UCHAR phy_bw;
	value = os_str_tol(arg, 0, 10);
	if (hc_radio_query_by_rf(pAd, RFIC_5GHZ, &oper) != HC_STATUS_OK)
		return FALSE;
	phy_bw = oper.bw;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_RadarDetectStart_Proc:\n"));

	if (value == 0) {
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0, 0);
		ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, RXSEL_0, REG_DEFAULT);
		ret1 = mtRddControl(pAd, RDD_DET_MODE, HW_RDD0, 0, RDD_DETMODE_ON);
		pDfsParam->bNoSwitchCh = TRUE;
	} else if (value == 1) {
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
		ret1 = mtRddControl(pAd, RDD_START, HW_RDD1, RXSEL_0, REG_DEFAULT);
		ret1 = mtRddControl(pAd, RDD_DET_MODE, HW_RDD1, 0, RDD_DETMODE_ON);
		pDfsParam->bNoSwitchCh = TRUE;
	} else if (value == 2) {
#ifdef DOT11_VHT_AC
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0, 0);
		ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, RXSEL_0, REG_DEFAULT);
		ret1 = mtRddControl(pAd, RDD_DET_MODE, HW_RDD0, 0, RDD_DETMODE_ON);

		if (phy_bw == BW_8080 || phy_bw == BW_160) {
			ret2 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
			ret2 = mtRddControl(pAd, RDD_START, HW_RDD1, RXSEL_0, REG_DEFAULT);
			ret2 = mtRddControl(pAd, RDD_DET_MODE, HW_RDD1, 0, RDD_DETMODE_ON);
		} else
#endif
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_RadarDetectStart_Proc: Bandwidth not 80+80 or 160\n"));

		pDfsParam->bNoSwitchCh = TRUE;
	} else
		;

	return TRUE;
}


INT Set_RadarDetectStop_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	ULONG value, ret1, ret2;
	struct freq_oper oper;
	UCHAR phy_bw;
	if (hc_radio_query_by_rf(pAd, RFIC_5GHZ, &oper) != HC_STATUS_OK)
		return FALSE;

	phy_bw = oper.bw;
	value = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_RadarDetectStop_Proc:\n"));

	if (value == 0)
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0, 0);
	else if (value == 1)
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
	else if (value == 2) {
		ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0, 0);
#ifdef DOT11_VHT_AC

		if (phy_bw == BW_8080 || phy_bw == BW_160)
			ret2 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
		else
#endif
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("In Set_RadarDetectStop_Proc: Bandwidth not 80+80 or 160\n"));
	} else
		;

	return TRUE;
}

INT Set_ByPassCac_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	UCHAR value; /* CAC time */
	UCHAR i;
	value = os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("set CAC value to %d\n", value));
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		if (pAd->Dot11_H[i].RDMode == RD_SILENCE_MODE)
			pAd->Dot11_H[i].RDCount = pAd->Dot11_H[i].ChMovingTime;
	}
	return TRUE;
}

INT Set_RDDReport_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	UCHAR value;
	value = os_str_tol(arg, 0, 10);

#ifdef RDM_FALSE_ALARM_DEBUG_SUPPORT
	pAd->CommonCfg.DfsParameter.fgRadarEmulate = TRUE;
#endif /* RDM_FALSE_ALARM_DEBUG_SUPPORT */

#if defined(MT7615) || defined(MT7622)
	WrapDfsRddReportHandle(pAd, value);
#endif
	return TRUE;
}

INT Set_DfsChannelShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	UCHAR value;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	value = os_str_tol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("Current 5G channel, Band0Ch: %d, Band1Ch: %d\n",
				 pDfsParam->Band0Ch, pDfsParam->Band1Ch));
	return TRUE;
}

INT Set_DfsBwShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	ULONG value;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	value = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("Current DFS Bw is %d\n", pDfsParam->Bw));
	return TRUE;
}

INT Set_DfsRDModeShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	ULONG value;
	UCHAR i;
	UCHAR BssIdx;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdevEach = NULL;
	struct DOT11_H *pDot11hEach = NULL;

	value = os_str_tol(arg, 0, 10);

	for (i = 0; i < DBDC_BAND_NUM; i++) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("pAd->RDMode[%d]=%d\n",
				 i,
				 pAd->Dot11_H[i].RDMode
				 ));
	}
	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BssIdx: %d\n", BssIdx));
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdevEach = &pMbss->wdev;
		if (pMbss == NULL || wdevEach == NULL)
			continue;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wdevIdx: %d. BandIdx: %d, channel: %d\n", wdevEach->wdev_idx, HcGetBandByWdev(wdevEach), wdevEach->channel));
		pDot11hEach = wdevEach->pDot11_H;
		if (pDot11hEach == NULL)
			continue;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RDMode: %d\n\n", pDot11hEach->RDMode));
	}
	return TRUE;
}

INT Set_DfsRDDRegionShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	ULONG value;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	value = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("RDD Region is %d\n",
			 pDfsParam->RDDurRegion));
	return TRUE;
}

INT Show_DfsNonOccupancy_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("[%s]:\n", __func__));

	for (i = 0; i < pDfsParam->ChannelListNum; i++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("DfsChannelList[%d].Channel = %d, NonOccupancy = %d, NOPClrCnt = %d, NOPSetByBw = %d, NOPSaveForClear is %d, SupportBwBitMap is %d\n",
		i,
		pDfsParam->DfsChannelList[i].Channel,
		pDfsParam->DfsChannelList[i].NonOccupancy,
		pDfsParam->DfsChannelList[i].NOPClrCnt,
		pDfsParam->DfsChannelList[i].NOPSetByBw,
		pDfsParam->DfsChannelList[i].NOPSaveForClear,
		pDfsParam->DfsChannelList[i].SupportBwBitMap));
	}
	return TRUE;
}

#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
INT Set_DfsDefaultRDDThresholdParam(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR ucRadarTypeIdx = 0;
	CMD_RDM_RADAR_THRESHOLD_UPDATE_T RadarThreshold = {0};
	CMD_RDM_PULSE_THRESHOLD_UPDATE_T PulseThresholdUpdate = {0};
	PDFS_RADAR_THRESHOLD_PARAM prRadarThresholdParam = NULL;
	PSW_RADAR_TYPE_T prRadarType = NULL;

	prRadarThresholdParam = &pAd->CommonCfg.DfsParameter.rRadarThresholdParam;
	MtCmdSetFcc5MinLPN(pAd, pAd->CommonCfg.DfsParameter.u2FCC_LPN_MIN);

	PulseThresholdUpdate.u4PP_PulseWidthMAX = prRadarThresholdParam->rPulseThresholdParam.u4PulseWidthMax;
	PulseThresholdUpdate.i4PulsePowerMAX = prRadarThresholdParam->rPulseThresholdParam.i4PulsePwrMax;
	PulseThresholdUpdate.i4PulsePowerMIN = prRadarThresholdParam->rPulseThresholdParam.i4PulsePwrMin;
	PulseThresholdUpdate.u4PRI_MIN_STGR = prRadarThresholdParam->rPulseThresholdParam.u4PRI_MIN_STGR;
	PulseThresholdUpdate.u4PRI_MAX_STGR = prRadarThresholdParam->rPulseThresholdParam.u4PRI_MAX_STGR;
	PulseThresholdUpdate.u4PRI_MIN_CR = prRadarThresholdParam->rPulseThresholdParam.u4PRI_MIN_CR;
	PulseThresholdUpdate.u4PRI_MAX_CR = prRadarThresholdParam->rPulseThresholdParam.u4PRI_MAX_CR;
	MtCmdSetPulseThresholdParam(pAd, &PulseThresholdUpdate);

	for (ucRadarTypeIdx = 0; ucRadarTypeIdx < RT_NUM; ucRadarTypeIdx++) {
		prRadarType = &prRadarThresholdParam->arRadarType[ucRadarTypeIdx];
		RadarThreshold.u2RadarType = ucRadarTypeIdx;
		RadarThreshold.ucRT_ENB = prRadarType->ucRT_ENB;
		RadarThreshold.ucRT_STGR = prRadarType->ucRT_STGR;
		RadarThreshold.ucRT_CRPN_MIN = prRadarType->ucRT_CRPN_MIN;
		RadarThreshold.ucRT_CRPN_MAX = prRadarType->ucRT_CRPN_MAX;
		RadarThreshold.ucRT_CRPR_MIN = prRadarType->ucRT_CRPR_MIN;
		RadarThreshold.ucRT_PW_MIN = prRadarType->ucRT_PW_MIN;
		RadarThreshold.ucRT_PW_MAX = prRadarType->ucRT_PW_MAX;
		RadarThreshold.u4RT_PRI_MIN = prRadarType->u4RT_PRI_MIN;
		RadarThreshold.u4RT_PRI_MAX = prRadarType->u4RT_PRI_MAX;
		RadarThreshold.ucRT_CRBN_MIN = prRadarType->ucRT_CRBN_MIN;
		RadarThreshold.ucRT_CRBN_MAX = prRadarType->ucRT_CRBN_MAX;
		RadarThreshold.ucRT_STGPN_MIN = prRadarType->ucRT_STGPN_MIN;
		RadarThreshold.ucRT_STGPN_MAX = prRadarType->ucRT_STGPN_MAX;
		RadarThreshold.ucRT_STGPR_MIN = prRadarType->ucRT_STGPR_MIN;
		MtCmdSetRadarThresholdParam(pAd, &RadarThreshold);
	}

	return TRUE;
}
#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */

INT Set_DfsNOP_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	ULONG value;
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	value = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("Set NOP of all channel as %ld.\n", value));

	for (i = 0; i < pDfsParam->ChannelListNum; i++) {
		pDfsParam->DfsChannelList[i].NonOccupancy = value;
		pDfsParam->DfsChannelList[i].NOPClrCnt = 0;
		pDfsParam->DfsChannelList[i].NOPSetByBw = 0;
	}

	return TRUE;
}

/* DFS Zero Wait */
INT Set_DfsZeroWaitCacTime_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg)
{
	UCHAR Value;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	Value = (UCHAR) os_str_tol(arg, 0, 10);
	pDfsParam->DfsZeroWaitCacTime = Value;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("[%s]CacTime=%d/%d\n",
			 __func__,
			 Value,
			 pDfsParam->DfsZeroWaitCacTime));
	return TRUE;
}

INT Set_DedicatedBwCh_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	i;
	CHAR *value = 0;
	UCHAR SynNum = 0, Channel = 0, Bw = 0, doCAC = 1;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]\n", __FUNCTION__));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0: /* Set Syn Num*/
			SynNum = simple_strtol(value, 0, 10);
			break;
		case 1: /* Set InBand ControlChannel */
			Channel = simple_strtol(value, 0, 10);
			break;
		case 2: /* Set InBand Bw*/
			Bw = simple_strtol(value, 0, 10);
			break;
		case 3: /* Set doCAC*/
			doCAC = simple_strtol(value, 0, 10);
			break;
		default:
			break;
		}
	}

#ifdef BACKGROUND_SCAN_SUPPORT
		if (SynNum == RDD_BAND0)
			DfsDedicatedInBandSetChannel(pAd, Channel, Bw, doCAC);
		else
			DfsDedicatedOutBandSetChannel(pAd, Channel, Bw);
#endif

	return TRUE;
}

INT Set_DfsZeroWaitDynamicCtrl_Proc(
		RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Value;

	Value = (UCHAR) simple_strtol(arg, 0, 10);

#ifdef BACKGROUND_SCAN_SUPPORT
	DfsDedicatedDynamicCtrl(pAd, Value);
#endif

	return TRUE;
}

INT Set_DfsZeroWaitNOP_Proc(
		RTMP_ADAPTER * pAd, RTMP_STRING *arg)
{
	INT	i;
	CHAR *value = 0;
	UCHAR Channel = 0, Bw = 0;
	USHORT NOPTime = 0;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]\n", __FUNCTION__));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			Channel = simple_strtol(value, 0, 10);
			break;
		case 1:
			Bw = simple_strtol(value, 0, 10);
			break;
		case 2:
			NOPTime = simple_strtol(value, 0, 10);
			break;
		default:
			break;
		}
	}

	ZeroWait_DFS_set_NOP_to_Channel_List(pAd, Channel, Bw, NOPTime);

	return TRUE;
}

INT Set_DfsTargetCh_Proc(
		RTMP_ADAPTER * pAd, RTMP_STRING *arg)
{
		INT	i;
	CHAR *value = 0;
	UCHAR Channel = 0, Bw = 0;
	USHORT CacValue = 0;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]\n", __FUNCTION__));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			Channel = simple_strtol(value, 0, 10);
			break;
		case 1:
			Bw = simple_strtol(value, 0, 10);
			break;
		case 2:
			CacValue = simple_strtol(value, 0, 10);
			break;
		default:
			break;
		}
	}

	ZeroWait_DFS_Pre_Assign_Next_Target_Channel(pAd, Channel, Bw, CacValue);

	return TRUE;
}

VOID DfsDedicatedExclude(IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if (pDfsParam->bDedicatedZeroWaitSupport == TRUE) {
		if (pDfsParam->Bw == BW_160 || pDfsParam->Bw == BW_8080) {
			pDfsParam->bDedicatedZeroWaitSupport = FALSE;
		}
	}
}

VOID DfsSetCalibration(
	IN PRTMP_ADAPTER pAd, UINT_32 DisableDfsCal)
{
	if (!DisableDfsCal)
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Enable DFS calibration in firmware.\n"));
	else {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Disable DFS calibration in firmware.\n"));
		mtRddControl(pAd, DISABLE_DFS_CAL, HW_RDD0, 0, 0);
	}
}

VOID DfsSetZeroWaitCacSecond(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->bZeroWaitCacSecondHandle = TRUE;
}

BOOLEAN DfsBypassRadarStateCheck(struct wifi_dev *wdev)
{
	struct hdev_obj *obj = NULL;
	struct radio_dev *rdev;
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return FALSE;

	obj = wdev->pHObj;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return FALSE;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return FALSE;
	}

	rdev = obj->rdev;

	if (pDot11h->RDMode == RD_NORMAL_MODE)
		return TRUE;
	return FALSE;
}
BOOLEAN DfsRadarChannelCheck(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR vht_cent2,
	UCHAR phy_bw)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	BOOLEAN ret = FALSE;

	if (!IS_CH_ABAND(wdev->channel)) {
		return FALSE;
	}

	/*DfsGetSysParameters(pAd, wdev, vht_cent2, phy_bw);*/

	if (!pDfsParam->bDfsEnable)
		return FALSE;
#ifdef DOT11_VHT_AC
	if (phy_bw == BW_8080) {
		ret = (RadarChannelCheck(pAd, wdev->channel)
		|| RadarChannelCheck(pAd, CentToPrim(vht_cent2)));
	} else if ((phy_bw == BW_160) && (wdev->channel >= GROUP1_LOWER && wdev->channel <= GROUP1_UPPER)) {
		ret = TRUE;
	} else
#endif
	{
		ret = RadarChannelCheck(pAd, wdev->channel);
	}

#ifdef ONDEMAND_DFS
	if ((IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd)) ||
		((IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd) == FALSE) && (ret == TRUE)))
#else
	if (ret == TRUE)
#endif
		DfsGetSysParameters(pAd, wdev, vht_cent2, phy_bw);
	else if ((pAd->CommonCfg.dbdc_mode == TRUE) && IS_CH_ABAND(wdev->channel)) {
		pDfsParam->DbdcAdditionCh = wdev->channel;
	}
	return ret;
}

#ifdef PRE_CAL_TRX_SET1_SUPPORT
VOID DfsCacEndLoadDCOCData(
	IN PRTMP_ADAPTER pAd)
{
	USHORT doCal1 = 0;
	UINT16 OrigCentralFreq = 0;
	UINT16 CentralFreq = 0;
	BOOLEAN toCR = TRUE;
	BOOLEAN bSecBW80 = 0;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UINT_8 i = 0;
	ULONG Offset = 0;
	RXDCOC_RESULT_T RxDcocResult;
	struct wifi_dev *wdev = NULL;

	if (pDfsParam->bDBDCMode == 0)
		return;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		if (WMODE_CAP_5G(wdev->PhyMode)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s] Valid wdev.\n", __func__));
			break;
		} else
			continue;
	}

	if (wdev == NULL)
		return;

	/*Work Around to reload RXDCOC data when CAC ends*/
#ifdef RTMP_FLASH_SUPPORT
	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		rtmp_ee_flash_read(pAd, 0x52, &doCal1);
#endif
	if (pAd->E2pAccessMode == E2P_BIN_MODE)
		rtmp_ee_bin_read16(pAd, 0x52, &doCal1);

	if ((doCal1 & (1 << 1)) != 0) {
		OrigCentralFreq = DfsGetCentCh(pAd, pDfsParam->Band0Ch, pDfsParam->Bw, wdev) * 5 + 5000;

		if (pDfsParam->Bw == BW_20) {
			/* find nearest BW40 central to apply */
			for (i = 0; i < K_A40_SIZE; i++) {
				UINT delta = (OrigCentralFreq >= K_A40Freq[i])?(OrigCentralFreq-K_A40Freq[i]):(K_A40Freq[i]-OrigCentralFreq);

				if (delta <= 10) {
					CentralFreq = K_A40Freq[i];
					break;
				}
			}

			if (i == K_A40_SIZE) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : UNEXPECTED. FREQ[%d] CANT FIND LEGAL CHANNEL TO APPLY, PLEASE CHECK!! \n"
					, __FUNCTION__, CentralFreq));
				/* send command to tell FW do online K */
				MtCmdGetRXDCOCCalResult(pAd, toCR, CentralFreq, pDfsParam->Bw, ABAND, bSecBW80, TRUE, &RxDcocResult);
				return;
			}
		} else if (pDfsParam->Bw == BW_40) {
			/* prevent illegal channel */
			for (i = 0; i < K_A40_SIZE; i++) {
				if (OrigCentralFreq == K_A40Freq[i]) {
					CentralFreq = OrigCentralFreq;
					break;
				}
			}
			if (i == K_A40_SIZE) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : UNEXPECTED. FREQ[%d] @BW[%d] CANT FIND LEGAL CHANNEL TO APPLY, PLEASE CHECK!! \n"
					, __FUNCTION__, CentralFreq, BW_40));
				/* send command to tell FW do online K */
				MtCmdGetRXDCOCCalResult(pAd, toCR, CentralFreq, pDfsParam->Bw, ABAND, bSecBW80, TRUE, &RxDcocResult);
				return;
			}
		} else if (pDfsParam->Bw == BW_80) {
			/* prevent illegal channel */
			for (i = 0; i < K_A80_SIZE; i++) {
				if (CentralFreq == K_A80Freq[i]) {
					CentralFreq = OrigCentralFreq;
					break;
				}
			}
			if (i == K_A80_SIZE) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : UNEXPECTED. FREQ[%d] @BW[%d] CANT FIND LEGAL CHANNEL TO APPLY, PLEASE CHECK!! \n"
					, __FUNCTION__, CentralFreq, BW_80));
				/* send command to tell FW do online K */
				MtCmdGetRXDCOCCalResult(pAd, toCR, CentralFreq, pDfsParam->Bw, ABAND, bSecBW80, TRUE, &RxDcocResult);
				return;
			}
		}

		for (i = 0; i < K_ALL_SIZE; i++) {
			if (K_AllFreq[i] == CentralFreq)
				break;
		}

		if (i == K_ALL_SIZE) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s : UNEXPECTED. FREQ[%d] CANT FIND LEGAL CHANNEL TO APPLY, PLEASE CHECK!! \n"
				, __FUNCTION__, CentralFreq));
			/* send command to tell FW do online K */
			MtCmdGetRXDCOCCalResult(pAd, toCR, CentralFreq, pDfsParam->Bw, ABAND, bSecBW80, TRUE, &RxDcocResult);
			return;
		}

		Offset = i * RXDCOC_SIZE;

		memcpy(&RxDcocResult.ucDCOCTBL_I_WF0_SX0_LNA[0], pAd->CalDCOCImage + Offset, RXDCOC_SIZE);

		ShowDCOCData(pAd, RxDcocResult);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s() : reload Central CH [%d] BW [%d] from cetral freq [%d]  offset [%lx] \n",
				__FUNCTION__, OrigCentralFreq, pDfsParam->Bw, CentralFreq, DCOC_FLASH_OFFSET + Offset));

		MtCmdGetRXDCOCCalResult(pAd, toCR, CentralFreq, pDfsParam->Bw, ABAND, bSecBW80, FALSE, &RxDcocResult);
	}
}
#endif

VOID DfsCacEndUpdate(
	RTMP_ADAPTER * pAd,
	MLME_QUEUE_ELEM *Elem)
{
	UCHAR bandIdx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UINT_8 BssIdx;
	struct wifi_dev *wdev = NULL;
	UCHAR wdev_band_index;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] CAC end. Enable MAC TX.\n", __func__));
	bandIdx = (UCHAR)(Elem->Priv);
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	DfsCacEndLoadDCOCData(pAd);
#endif
	mtRddControl(pAd, CAC_END, HW_RDD0, 0, bandIdx);
	if (DfsCacTimeOutCallBack) {
		DfsCacTimeOutCallBack(RDD_BAND0, pDfsParam->Bw, pDfsParam->Band0Ch);
	}
	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
		wdev_band_index = HcGetBandByWdev(wdev);
		if ((wdev->bAllowBeaconing) && (wdev_band_index == bandIdx) && (!wdev->bcn_buf.bBcnSntReq)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Enabling Beaconing.\n", __func__));
			UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_ENABLE_TX);
		}
	}
}

#ifdef ONDEMAND_DFS
VOID DfsOutBandCacPass(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] CAC end. Out of Timer Context.\n", __func__));

	if (pDfsParam->bOutBandAvailable) {
		/* Assign 4x4 Mode */
		SET_ONDEMAND_DFS_MODE(pAd, ONDEMAND_4x4MODE);
		/* Assign CAC Passed DFS Outband Channel to Inband Channel */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] New Inband Channel %d\n",
					__func__, pDfsParam->OutBandCh));
		DfsDedicatedInBandSetChannel(pAd, pDfsParam->OutBandCh, pDfsParam->OutBandBw, FALSE);
	}
}
#endif

NTSTATUS DfsChannelSwitchTimeoutAction(
	PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UINT_32 SetChInfo;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	UINT8 bandIdx;
	UINT8 BssIdx;
	UINT8 NextCh;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	NdisMoveMemory(&SetChInfo, CMDQelmt->buffer, sizeof(UINT_32));

	bandIdx = (SetChInfo >> 16) & 0xff;
	BssIdx = (SetChInfo >> 8) & 0xff;
	NextCh = SetChInfo & 0xff;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] bandIdx: %d, BssIdx: %d, NextCh: %d\n",
		__func__, bandIdx, BssIdx, NextCh));

	pMbss = &pAd->ApCfg.MBSSID[BssIdx];
	wdev = &pMbss->wdev;
	pDfsParam->RadarHitIdxRecord = bandIdx;

#ifdef BACKGROUND_SCAN_SUPPORT
	DedicatedZeroWaitStop(pAd, FALSE);
#endif

	rtmp_set_channel(pAd, wdev, NextCh);

	if (pAd->CommonCfg.dbdc_mode) {
		MtCmdSetDfsTxStart(pAd, bandIdx);
	} else
		MtCmdSetDfsTxStart(pAd, DBDC_BAND0);
	DfsReportCollision(pAd);
	return 0;
}

NTSTATUS DfsAPRestart(
	PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UINT_32 SetChInfo;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	UINT8 bandIdx;
	UINT8 BssIdx;
	UINT8 NextCh;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	NdisMoveMemory(&SetChInfo, CMDQelmt->buffer, sizeof(UINT_32));

	bandIdx = (SetChInfo >> 16) & 0xff;
	BssIdx = (SetChInfo >> 8) & 0xff;
	NextCh = SetChInfo & 0xff;

	pMbss = &pAd->ApCfg.MBSSID[BssIdx];
	wdev = &pMbss->wdev;
	pDfsParam->RadarHitIdxRecord = bandIdx;

	APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);

	if (pAd->CommonCfg.dbdc_mode) {
		MtCmdSetDfsTxStart(pAd, bandIdx);
	} else
		MtCmdSetDfsTxStart(pAd, DBDC_BAND0);

	return 0;
}

VOID DfsCacNormalStart(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR CompareMode)
{
	struct DOT11_H *pDot11h = NULL;
	UCHAR bandIdx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;


	if (wdev == NULL)
		return;

	bandIdx = HcGetBandByWdev(wdev);

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	if ((pDfsParam->RDDurRegion == CE)
		&& DfsCacRestrictBand(pAd, pDfsParam->Bw, pDfsParam->Band0Ch, pDfsParam->Band1Ch)) {
		/* Weather band channel */
		if (pDfsParam->targetCh != 0)
			pDot11h->ChMovingTime = pDfsParam->targetCacValue;
		else
			pDot11h->ChMovingTime = CAC_WETHER_BAND;
	} else {
		if (pDfsParam->targetCh != 0)
			pDot11h->ChMovingTime = pDfsParam->targetCacValue;
		else
			pDot11h->ChMovingTime = CAC_NON_WETHER_BAND;
	}

	if ((pDot11h->RDMode == RD_SILENCE_MODE) && (CompareMode == RD_SILENCE_MODE)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] CAC %d seconds start . Disable MAC TX\n",
				__func__, pDot11h->ChMovingTime));
		mtRddControl(pAd, CAC_START, HW_RDD0, 0, bandIdx);
	} else if ((pDot11h->RDMode == RD_NORMAL_MODE) && (CompareMode == RD_NORMAL_MODE)) {
#if defined(MT7615) || defined(MT7622)
		if (!IS_CH_ABAND(wdev->channel)) {
			UCHAR BssIdx;
			BSS_STRUCT *pMbss = NULL;
			struct wifi_dev *wdevEach = NULL;
			struct DOT11_H *pDot11hEach = NULL;
			for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
				pMbss = &pAd->ApCfg.MBSSID[BssIdx];
				wdevEach = &pMbss->wdev;
				if (pMbss == NULL || wdevEach == NULL)
					continue;
				if (wdevEach->pHObj == NULL)
					continue;
				pDot11hEach = wdevEach->pDot11_H;
				if (pDot11hEach == NULL)
					continue;
				if (pDot11hEach->RDMode == RD_SILENCE_MODE) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] 2G channel with 5G silence channel exist\n", __func__));
					return;
				}
			}
		}
#endif
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Normal start. Enable MAC TX\n", __func__));
		mtRddControl(pAd, NORMAL_START, HW_RDD0, 0, bandIdx);
	} else
		;
}

BOOLEAN DfsCacRestrictBand(
	IN PRTMP_ADAPTER pAd, IN UCHAR Bw, IN UCHAR Ch, IN UCHAR SecCh)
{
	BOOLEAN ret = FALSE;
#ifdef DOT11_VHT_AC
	if (Bw == BW_8080) {
		return RESTRICTION_BAND_1(pAd, Ch, Bw) || RESTRICTION_BAND_1(pAd, SecCh, Bw);
	} else if ((Bw == BW_160) && (Ch >= GROUP3_LOWER && Ch <= RESTRICTION_BAND_HIGH)) {
		return TRUE;
	} else
#endif
	{
		if (strncmp(pAd->CommonCfg.CountryCode, "KR", 2) == 0)
			ret = RESTRICTION_BAND_KOREA(pAd, Ch, Bw);
		else
			ret = RESTRICTION_BAND_1(pAd, Ch, Bw);
		return ret;
	}
}

VOID DfsBuildChannelList(
    IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev)
{
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	/*non DFS channel, no need to update chanel list*/
	if ((pAd->CommonCfg.dbdc_mode == TRUE) && !RadarChannelCheck(pAd, wdev->channel))
		return;
	if (pDfsParam->NeedSetNewChList == DFS_SET_NEWCH_ENABLED) {
		pDfsParam->ChannelListNum = pChCtrl->ChListNum;
		pDfsParam->RDDurRegion = pAd->CommonCfg.RDDurRegion;
		for (i = 0; i < pDfsParam->ChannelListNum; i++) {
			pDfsParam->DfsChannelList[i].Channel = pChCtrl->ChList[i].Channel;
			pDfsParam->DfsChannelList[i].NonOccupancy = pChCtrl->ChList[i].RemainingTimeForUse;
		}
		for (i = 0; i < pDfsParam->ChannelListNum; i++) {

			if (DfsCheckChAvailableByBw(pDfsParam->DfsChannelList[i].Channel, BW_20, pDfsParam))
				pDfsParam->DfsChannelList[i].SupportBwBitMap |= 0x01;
			if (DfsCheckChAvailableByBw(pDfsParam->DfsChannelList[i].Channel, BW_40, pDfsParam))
				pDfsParam->DfsChannelList[i].SupportBwBitMap |= 0x02;
			if (DfsCheckChAvailableByBw(pDfsParam->DfsChannelList[i].Channel, BW_80, pDfsParam)
			|| DfsCheckChAvailableByBw(pDfsParam->DfsChannelList[i].Channel, BW_8080, pDfsParam))
				pDfsParam->DfsChannelList[i].SupportBwBitMap |= 0x04;
			if (DfsCheckChAvailableByBw(pDfsParam->DfsChannelList[i].Channel, BW_160, pDfsParam))
				pDfsParam->DfsChannelList[i].SupportBwBitMap |= 0x08;
		}
	}

	DfsBuildChannelGroupByBw(pAd);
	pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_DISABLED;
}

VOID DfsBuildChannelGroupByBw(
    IN PRTMP_ADAPTER pAd)
{
	UINT_8 i/*, j*/;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	INT_8 BW40GroupIdx = -1;
	INT_8 BW80GroupIdx = -1;
	INT_8 BW160GroupIdx = -1;
	INT_8 BW40GroupMemberCnt = 0;
	INT_8 BW80GroupMemberCnt = 0;
	INT_8 BW160GroupMemberCnt = 0;
	UINT_8 PreviousBW40CentCh = 0xff;
	UINT_8 PreviousBW80CentCh = 0xff;
	UINT_8 PreviousBW160CentCh = 0xff;

	for (i = 0; i < pDfsParam->ChannelListNum; i++) {
		if (!IS_CH_ABAND(pDfsParam->DfsChannelList[i].Channel))
			continue;
		if (!ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, BW_40, pDfsParam)) {
			if (DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_40) != PreviousBW40CentCh) {
				BW40GroupMemberCnt = 0;
				if ((++BW40GroupIdx < DFS_BW40_GROUP_NUM) && (BW40GroupMemberCnt < DFS_BW40_PRIMCH_NUM))
					pDfsParam->Bw40GroupIdx[BW40GroupIdx][BW40GroupMemberCnt] = i;
			} else {
				if ((BW40GroupIdx >= 0) && (BW40GroupIdx < DFS_BW40_GROUP_NUM)
				 && (++BW40GroupMemberCnt < DFS_BW40_PRIMCH_NUM))
					pDfsParam->Bw40GroupIdx[BW40GroupIdx][BW40GroupMemberCnt] = i;
			}

			PreviousBW40CentCh = DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_40);
		}

		if (!ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, BW_80, pDfsParam)) {
			if (DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_80) != PreviousBW80CentCh) {
				BW80GroupMemberCnt = 0;
				if ((++BW80GroupIdx < DFS_BW80_GROUP_NUM) && (BW80GroupMemberCnt < DFS_BW80_PRIMCH_NUM))
					pDfsParam->Bw80GroupIdx[BW80GroupIdx][BW80GroupMemberCnt] = i;
			} else {
				if ((BW80GroupIdx >= 0) && (BW80GroupIdx < DFS_BW80_GROUP_NUM)
				 && (++BW80GroupMemberCnt < DFS_BW80_PRIMCH_NUM))
				pDfsParam->Bw80GroupIdx[BW80GroupIdx][BW80GroupMemberCnt] = i;
			}

			PreviousBW80CentCh = DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_80);
		}
		if (!ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, BW_160, pDfsParam)) {
			if (DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_160) != PreviousBW160CentCh) {
				BW160GroupMemberCnt = 0;
				if ((++BW160GroupIdx < DFS_BW160_GROUP_NUM) && (BW160GroupMemberCnt < DFS_BW160_PRIMCH_NUM))
					pDfsParam->Bw160GroupIdx[BW160GroupIdx][BW160GroupMemberCnt] = i;
			} else {
				if ((BW160GroupIdx >= 0) && (BW160GroupIdx < DFS_BW160_GROUP_NUM)
				 && (++BW160GroupMemberCnt < DFS_BW160_PRIMCH_NUM))
					pDfsParam->Bw160GroupIdx[BW160GroupIdx][BW160GroupMemberCnt] = i;
			}

			PreviousBW160CentCh = DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_160);
		}

	}
}

BOOLEAN DfsCheckBwGroupAllAvailable(
    UCHAR CheckChIdx, UCHAR Bw, IN PDFS_PARAM pDfsParam)
{
	UCHAR *pBwxxGroupIdx = NULL;
	UCHAR i, j;
	UCHAR GroupNum = 4, BwxxPrimNum = 4;

	if (Bw == BW_20)
		return TRUE;
	else if (Bw == BW_40) {
		pBwxxGroupIdx = &pDfsParam->Bw40GroupIdx[0][0];
		GroupNum = DFS_BW40_GROUP_NUM;
		BwxxPrimNum = DFS_BW40_PRIMCH_NUM;
	} else if (Bw == BW_80) {
		pBwxxGroupIdx = &pDfsParam->Bw80GroupIdx[0][0];
		GroupNum = DFS_BW80_GROUP_NUM;
		BwxxPrimNum = DFS_BW80_PRIMCH_NUM;
	} else if (Bw == BW_160) {
		pBwxxGroupIdx = &pDfsParam->Bw160GroupIdx[0][0];
		GroupNum = DFS_BW160_GROUP_NUM;
		BwxxPrimNum = DFS_BW160_PRIMCH_NUM;
	} else
		return FALSE;


	for (i = 0; i < GroupNum*BwxxPrimNum; i++) {

		if (*pBwxxGroupIdx == CheckChIdx) {
			break;
		}
		pBwxxGroupIdx++;
	}
	if (i >= GroupNum*BwxxPrimNum)
		return FALSE;

	j = i%BwxxPrimNum;
	i = i/BwxxPrimNum;

	pBwxxGroupIdx = pBwxxGroupIdx - j;

	for (j = 0; j < BwxxPrimNum; j++) {
		if (pDfsParam->DfsChannelList[*pBwxxGroupIdx].NonOccupancy != 0)
			return FALSE;
		if ((pDfsParam->DfsChannelList[*pBwxxGroupIdx].NonOccupancy == 0)
		 && (pDfsParam->DfsChannelList[*pBwxxGroupIdx].NOPClrCnt != 0)
		 && (pDfsParam->DfsChannelList[*pBwxxGroupIdx].NOPSetByBw <= Bw)
		 )
			return FALSE;

		pBwxxGroupIdx++;
	}

	return TRUE;
}

BOOLEAN DfsSwitchCheck(
	IN PRTMP_ADAPTER	pAd,
	UCHAR	Channel,
	UCHAR bandIdx
)
{
	if ((pAd->Dot11_H[bandIdx].RDMode == RD_SILENCE_MODE) && (Channel > 14)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[DfsSwitchCheck]: DFS ByPass TX calibration.\n"));
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSwitchCheck]: NON DFS calibration.\n"));
		return FALSE;
	}
}

BOOLEAN DfsStopWifiCheck(
	IN PRTMP_ADAPTER	pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	return (pDfsParam->bNoAvailableCh == TRUE);
}

VOID DfsNonOccupancyUpdate(
	IN PRTMP_ADAPTER pAd)
{
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			if (pChCtrl->ChList[i].Channel > 14)
				pChCtrl->ChList[i].RemainingTimeForUse = pDfsParam->DfsChannelList[i].NonOccupancy;
		}
	}
}

#if defined(OFFCHANNEL_SCAN_FEATURE) && defined (ONDEMAND_DFS)
VOID DfsNonOccupancyCmpltnEvent(
	IN PRTMP_ADAPTER pAd,
	IN UINT_8 chIndex,
	IN BOOLEAN isNOPSaveForClear)
{
	OFFCHANNEL_SCAN_MSG Rsp;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if ((((isNOPSaveForClear) ? (pDfsParam->DfsChannelList[chIndex].NOPSaveForClear)
		: (pDfsParam->DfsChannelList[chIndex].NonOccupancy)) == CHAN_NON_OCCUPANCY_FREE)
		&& (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd))) {
		Rsp.Action = DFS_CHANNEL_NOP_CMPLT;
		memcpy(Rsp.ifrn_name, pAd->ScanCtrl.if_name, IFNAMSIZ);
		printk("[%s] channel %d NOP Cmplt action NopClr = %d\n",
			__func__, pDfsParam->DfsChannelList[chIndex].Channel, isNOPSaveForClear);
		Rsp.data.operating_ch_info.channel = pDfsParam->DfsChannelList[chIndex].Channel;
		Rsp.data.operating_ch_info.cfg_ht_bw = wlan_config_get_ht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
		Rsp.data.operating_ch_info.cfg_vht_bw = wlan_config_get_vht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
		Rsp.data.operating_ch_info.RDDurRegion = 0;
		Rsp.data.operating_ch_info.region = GetCountryRegionFromCountryCode(pAd->CommonCfg.CountryCode);
		Rsp.data.operating_ch_info.is4x4Mode = IS_ONDEMAND_DFS_MODE_4x4(pAd);
		RtmpOSWrielessEventSend(
			pAd->net_dev,
			RT_WLAN_EVENT_CUSTOM,
			OID_OFFCHANNEL_INFO,
			NULL,
			(UCHAR *) &Rsp,
			sizeof(OFFCHANNEL_SCAN_MSG));
	}
}
#endif
VOID DfsNonOccupancyCountDown(/*RemainingTimeForUse --*/
	IN PRTMP_ADAPTER pAd)
{
	UINT_8 i;
	UCHAR BssIdx;
	UINT_32 SetChInfo = 0;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	BOOLEAN Band0Available = FALSE, Band1Available = FALSE;

	for (i = 0; i < pDfsParam->ChannelListNum; i++) {
		if (pDfsParam->DfsChannelList[i].NonOccupancy > 0) {
			pDfsParam->DfsChannelList[i].NonOccupancy--;
#if defined(OFFCHANNEL_SCAN_FEATURE) && defined (ONDEMAND_DFS)
			if (pDfsParam->DfsChannelList[i].NonOccupancy == CHAN_NON_OCCUPANCY_FREE) {
				/* NOP Cmpltn event to App */
				DfsNonOccupancyCmpltnEvent (pAd, i, FALSE);
			}
#endif
#ifdef WAPP_SUPPORT
			if (pDfsParam->DfsChannelList[i].NonOccupancy == 0) {
				int j = 0;

				for (j = 0; j < WDEV_NUM_MAX; j++) {
					if (pAd->wdev_list[j]) {
						if (HcGetBandByWdev(pAd->wdev_list[j])
							!= pDfsParam->RadarHitIdxRecord)
								continue;
						wapp_send_radar_detect_notif(pAd, pAd->wdev_list[j],
							pDfsParam->DfsChannelList[i].Channel, TRUE);
						break;
					}
				}
			}
#endif
		}
		if (pDfsParam->DfsChannelList[i].NOPSaveForClear > 0) {
			pDfsParam->DfsChannelList[i].NOPSaveForClear--;
#if defined(OFFCHANNEL_SCAN_FEATURE) && defined(ONDEMAND_DFS)
			if (pDfsParam->DfsChannelList[i].NOPSaveForClear == CHAN_NON_OCCUPANCY_FREE) {
				/* NOP Cmpltn event to App */
				DfsNonOccupancyCmpltnEvent(pAd, i, TRUE);
			}
#endif
		}
		else if ((pDfsParam->DfsChannelList[i].NOPSaveForClear == 0) && (pDfsParam->DfsChannelList[i].NOPClrCnt != 0))
			pDfsParam->DfsChannelList[i].NOPClrCnt = 0;
	}
	if (pDfsParam->bNoAvailableCh == TRUE) {
		for (i = 0; i < pDfsParam->ChannelListNum; i++) {
			if ((pDfsParam->Bw != BW_8080) && (pDfsParam->DfsChannelList[i].Channel == pDfsParam->PrimCh)) {
				if (pDfsParam->DfsChannelList[i].NonOccupancy == 0) {
					Band0Available = TRUE;
				}
			}
			if ((pDfsParam->Bw == BW_8080) && (pDfsParam->DfsChannelList[i].Channel == pDfsParam->Band0Ch)) {
				if (pDfsParam->DfsChannelList[i].NonOccupancy == 0) {
					Band0Available = TRUE;
				}
			}
			if ((pDfsParam->Bw == BW_8080) && (pDfsParam->DfsChannelList[i].Channel == pDfsParam->Band1Ch)) {
				if (pDfsParam->DfsChannelList[i].NonOccupancy == 0) {
					Band1Available = TRUE;
				}

			}
		}
		if (((pDfsParam->Bw != BW_8080) && (Band0Available == TRUE))
		|| ((pDfsParam->Bw == BW_8080) && (Band0Available == TRUE) && (Band1Available == TRUE))) {
			pDfsParam->bNoAvailableCh = FALSE;
			for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
				pMbss = &pAd->ApCfg.MBSSID[BssIdx];
				wdev = &pMbss->wdev;
				if (HcGetBandByWdev(wdev) != pDfsParam->RadarHitIdxRecord)
					continue;
				SetChInfo |= wdev->channel;
				SetChInfo |= (BssIdx << 8);
				SetChInfo |= (pDfsParam->RadarHitIdxRecord << 16);
				RTEnqueueInternalCmd(pAd, CMDTHRED_DFS_AP_RESTART, &SetChInfo, sizeof(UINT_32));
			}
		}
	}
}

VOID WrapDfsSetNonOccupancy(/*Set Channel non-occupancy time */
	IN PRTMP_ADAPTER pAd,
	UCHAR bandIdx
)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	DfsSetNonOccupancy(pAd, pDfsParam, bandIdx);
}

VOID DfsSetNonOccupancy(/*Set Channel non-occupancy time */
	IN PRTMP_ADAPTER pAd,
	IN PDFS_PARAM pDfsParam,
	UCHAR bandIdx
)
{
	UINT_8 i;
	/*Only Bw20, Bw40, Bw80 refer to this value*/
	UINT_8 TargetCh, TargetBw, TargetChDfsBand;

	if (pDfsParam->Dot11_H[bandIdx].RDMode == RD_SWITCHING_MODE)
		return;

#ifdef ONDEMAND_DFS
	if (((pDfsParam->bDedicatedZeroWaitSupport == TRUE) || (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd))) && (pDfsParam->RadarDetected[1] == TRUE)) {
#else
	if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && (pDfsParam->RadarDetected[1] == TRUE)) {
#endif
		TargetCh = pDfsParam->OutBandCh;
		TargetBw = pDfsParam->OutBandBw;
		TargetChDfsBand = pDfsParam->DfsChBand[1];
	} else {
		TargetCh = pDfsParam->Band0Ch;
		TargetBw = pDfsParam->Bw;
		TargetChDfsBand = pDfsParam->DfsChBand[0];
	}
	if ((pDfsParam->Bw == BW_160) &&
		(pDfsParam->DfsChBand[0] || pDfsParam->DfsChBand[1])) {
		for (i = 0; i < pDfsParam->ChannelListNum; i++) {
			if (vht_cent_ch_freq(pDfsParam->DfsChannelList[i].Channel, VHT_BW_160) == vht_cent_ch_freq(pDfsParam->Band0Ch, VHT_BW_160)) {
				pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				pDfsParam->DfsChannelList[i].NOPSetByBw = pDfsParam->Bw;
			}
		}
	}
	if ((TargetBw == BW_80) && TargetChDfsBand) {
		for (i = 0; i < pDfsParam->ChannelListNum; i++) {
			if (vht_cent_ch_freq(pDfsParam->DfsChannelList[i].Channel, VHT_BW_80) == vht_cent_ch_freq(TargetCh, VHT_BW_80)) {
				pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				pDfsParam->DfsChannelList[i].NOPSetByBw = TargetBw;
			}
		}
	} else if (TargetBw == BW_40 && TargetChDfsBand) {
		for (i = 0; i < pDfsParam->ChannelListNum; i++) {
			if ((TargetCh == pDfsParam->DfsChannelList[i].Channel)) {
				pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				pDfsParam->DfsChannelList[i].NOPSetByBw = TargetBw;
			} else if (((TargetCh) >> 2 & 1) && ((pDfsParam->DfsChannelList[i].Channel - TargetCh) == 4)) {
				pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				pDfsParam->DfsChannelList[i].NOPSetByBw = TargetBw;
			} else if (!((TargetCh) >> 2 & 1) && ((TargetCh - pDfsParam->DfsChannelList[i].Channel) == 4)) {
				pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				pDfsParam->DfsChannelList[i].NOPSetByBw = TargetBw;
			}
			else
				;
		}
	} else if (TargetBw == BW_20 && TargetChDfsBand) {
		for (i = 0; i < pDfsParam->ChannelListNum; i++) {
			if ((TargetCh == pDfsParam->DfsChannelList[i].Channel)) {
				pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				pDfsParam->DfsChannelList[i].NOPSetByBw = TargetBw;
			}
		}
	} else if (pDfsParam->Bw == BW_8080 && pDfsParam->DfsChBand[0] && pDfsParam->RadarDetected[0]) {
		for (i = 0; i < pDfsParam->ChannelListNum; i++) {
			if (vht_cent_ch_freq(pDfsParam->DfsChannelList[i].Channel, VHT_BW_8080) == vht_cent_ch_freq(pDfsParam->Band0Ch, VHT_BW_8080)) {
				pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				pDfsParam->DfsChannelList[i].NOPSetByBw = pDfsParam->Bw;
			}
		}
	} else if (pDfsParam->Bw == BW_8080 && pDfsParam->DfsChBand[1] && pDfsParam->RadarDetected[1]) {
		for (i = 0; i < pDfsParam->ChannelListNum; i++) {
			if (vht_cent_ch_freq(pDfsParam->DfsChannelList[i].Channel, VHT_BW_8080) == vht_cent_ch_freq(pDfsParam->Band1Ch, VHT_BW_8080)) {
				pDfsParam->DfsChannelList[i].NonOccupancy = CHAN_NON_OCCUPANCY;
				pDfsParam->DfsChannelList[i].NOPSetByBw = pDfsParam->Bw;
			}
		}
	} else
		;
}

VOID WrapDfsRddReportHandle(/*handle the event of EXT_EVENT_ID_RDD_REPORT*/
	IN PRTMP_ADAPTER pAd, UCHAR ucRddIdx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR NextCh = 0;
	UCHAR NextBw = 0;
	UCHAR KeepBw = 0;
	UCHAR bandIdx;
	UCHAR BssIdx;
	UINT_32 SetChInfo = 0;
	BSS_STRUCT *pMbss = NULL;
	BOOLEAN RadarBandId[DBDC_BAND_NUM];
	UINT_8 i = 0;
	struct wifi_dev *wdev = NULL;
#ifdef OFFCHANNEL_SCAN_FEATURE
	OFFCHANNEL_SCAN_MSG Rsp;
#endif

#if defined(ONDEMAND_DFS) || defined(DFS_VENDOR10_CUSTOM_FEATURE)
	USHORT BwChannel;
#endif

#ifdef ONDEMAND_DFS
	BOOL isCacPassed = FALSE;
#endif
#ifdef TR181_SUPPORT
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
#endif
#ifdef WAPP_SUPPORT
	UCHAR first_wdev = TRUE;
#endif
#if defined(MT7615) || defined(MT7622)
	if (pDfsParam->Bw == BW_8080 || pDfsParam->Bw == BW_160)
		bandIdx = HW_RDD0;
	else
#endif
		bandIdx = ucRddIdx;

	pDfsParam->Dot11_H[bandIdx].RDMode = pAd->Dot11_H[bandIdx].RDMode;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WrapDfsRddReportHandle]:  Radar detected !!!!!!!!!!!!!!!!!\n"));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WrapDfsRddReportHandle]:  ucRddIdx: %d\n", ucRddIdx));
#ifdef OFFCHANNEL_SCAN_FEATURE
	Rsp.Action = DFS_RADAR_HIT;
	memcpy(Rsp.ifrn_name, pAd->ScanCtrl.if_name, IFNAMSIZ);
	printk("%s channel switch action = %d\n", __func__, Rsp.Action);
	pAd->radar_hit = TRUE;
	Rsp.data.operating_ch_info.channel = (ucRddIdx) ? (pDfsParam->OutBandCh) : (HcGetChannelByRf(pAd, RFIC_5GHZ));
	Rsp.data.operating_ch_info.cfg_ht_bw = wlan_config_get_ht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
	Rsp.data.operating_ch_info.cfg_vht_bw = wlan_config_get_vht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
	Rsp.data.operating_ch_info.RDDurRegion = pAd->CommonCfg.RDDurRegion;
	Rsp.data.operating_ch_info.region = GetCountryRegionFromCountryCode(pAd->CommonCfg.CountryCode);
#ifdef ONDEMAND_DFS
	Rsp.data.operating_ch_info.is4x4Mode = IS_ONDEMAND_DFS_MODE_4x4(pAd);
#else
	Rsp.data.operating_ch_info.is4x4Mode = 1;
#endif
	RtmpOSWrielessEventSend(
			pAd->net_dev,
			RT_WLAN_EVENT_CUSTOM,
			OID_OFFCHANNEL_INFO,
			NULL,
			(UCHAR *) &Rsp,
			sizeof(OFFCHANNEL_SCAN_MSG));
#endif
	if (pDfsParam->bNoSwitchCh)
		return;

#if defined(MT7615) || defined(MT7622)
	if ((pDfsParam->Bw == BW_8080) && (pDfsParam->PrimBand == RDD_BAND1)) /*Prim in idx 4~7. WorkAround for "Always set BB Prim 0~3 in IBF issue"*/
		ucRddIdx = 1 - ucRddIdx;
	if ((pDfsParam->Bw == BW_160)
		&& (pDfsParam->PrimCh >= GROUP4_LOWER && pDfsParam->PrimCh <= GROUP4_UPPER))
		ucRddIdx = 1 - ucRddIdx;
#endif
	if (!DfsRddReportHandle(pAd, pDfsParam, ucRddIdx, bandIdx))
		return;
	/*ByPass these setting when Dedicated/OnDemand DFS Zero wait, SynB/Band1 detect radar*/
#ifdef ONDEMAND_DFS
	if (((pDfsParam->bDedicatedZeroWaitSupport == TRUE) || (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd))) && (pDfsParam->RadarDetected[1] == TRUE)) {
#else
	if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && (pDfsParam->RadarDetected[1] == TRUE)) {
#endif
		;
	} else {
		if (pDfsParam->Dot11_H[bandIdx].RDMode == RD_SILENCE_MODE)
		pAd->Dot11_H[bandIdx].RDCount = 0;
	}
	WrapDfsSetNonOccupancy(pAd, bandIdx);
#ifdef BACKGROUND_SCAN_SUPPORT
	/*For Dedicated DFS zero wait, SynB/Band1 detect radar, choose another channel as Band1*/
#ifdef ONDEMAND_DFS
	if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) || (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd))) {
#else
	if (pDfsParam->bDedicatedZeroWaitSupport == TRUE) {
#endif
		if (pDfsParam->RadarDetected[1] == TRUE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RDD1 detect. Please switch to another outBand channel\n"));
			ZeroWait_DFS_collision_report(pAd, HW_RDD1, pDfsParam->OutBandCh, pDfsParam->OutBandBw);
			if (pDfsParam->bDedicatedZeroWaitDefault) {
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_OUTBAND_RADAR_FOUND, 0, NULL, 0);
			RTMP_MLME_HANDLER(pAd);
			}
			/*ZeroWait_DFS_collision_report(pAd, HW_RDD1, pDfsParam->OutBandCh, pDfsParam->OutBandBw);*/
			return;
		}
		if ((pDfsParam->RadarDetected[0] == TRUE) && GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
#ifdef ONDEMAND_DFS
			if (IS_ONDEMAND_DFS_MODE_4x4(pAd) == ONDEMAND_2x2MODE)
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RDD0 detect. OutBand channel come back to InBand\n"));
			else
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RDD0 detect. Move to 2x2 Mode for CAC \n"));
#else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RDD0 detect. OutBand channel come back to InBand\n"));
#endif
		}
	}
#endif
	if (pDfsParam->RadarDetected[0] == TRUE) {
		pDfsParam->OrigInBandCh = pDfsParam->Band0Ch;
		pDfsParam->OrigInBandBw = pDfsParam->Bw;
		pDfsParam->RadarHitReport = TRUE;
#ifdef CONFIG_RCSA_SUPPORT
		pDfsParam->fSendRCSA = TRUE;
#endif
	}
	/*Keep BW info because the BW may be changed after selecting a new channel*/
	KeepBw = pDfsParam->Bw;
#ifdef ONDEMAND_DFS
	if (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd) && (pDfsParam->RadarDetected[0] == TRUE)) {
		if (pDfsParam->Band0Ch == pDfsParam->OutBandCh) {
			/* Device in 4x4 Mode */
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Device in 4x4 Mode \n", __func__));
			isCacPassed = TRUE;
		}
		BwChannel = DfsOnDemandSelectBestChannel (pAd, FALSE);
		pDfsParam->PrimBand = RDD_BAND0;
		pDfsParam->Band0Ch = pDfsParam->PrimCh = BwChannel & 0xFF;
		pDfsParam->Bw = BwChannel >> 8;
		if (isCacPassed && RadarChannelCheck(pAd, pDfsParam->Band0Ch)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] DFS Channel %d - Move to 2x2 \n",
				__func__, pDfsParam->Band0Ch));
			pDfsParam->OutBandCh = pDfsParam->Band0Ch;
			pDfsParam->OutBandBw = pDfsParam->Bw;
			/* Switch to 2x2 Mode */
			SET_ONDEMAND_DFS_MODE(pAd, ONDEMAND_2x2MODE);
			pAd->ApCfg.bAutoChannelAtBootup = TRUE;
			/* Non-DFS InBand Channel */
			pDfsParam->Band0Ch = pDfsParam->PrimCh = DfsOnDemandSelectBestChannel (pAd, TRUE);
		} else {
			/* Reset Outband Ch/Bw as New Channel Non-DFS */
			pDfsParam->OutBandCh = 0;
			pDfsParam->OutBandBw = 0;
			/* Switch to 4x4 Mode */
			SET_ONDEMAND_DFS_MODE(pAd, ONDEMAND_4x4MODE);
		}
	} else
#endif

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		if (IS_SUPPORT_V10_DFS(pAd) && (pDfsParam->RadarDetected[0] == TRUE)) {
			BwChannel = DfsV10SelectBestChannel(pAd, HcGetChannelByRf(pAd, RFIC_5GHZ));

			/* AP BCN Update for ACS Case */
			if (IS_V10_AP_BCN_UPDATE_ENBL(pAd)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("[%s] BCN Update \n", __func__));

				MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_V10_ACS_CSA_UPDATE, 0, NULL, 0);
				return;
			}

			/* W56 Channel Exhausted : Ap Down for 30 Minutes */
			if (!BwChannel && IS_V10_W56_AP_DOWN_ENBLE(pAd)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("[%s] AP Down %ld\n", __func__, pDfsParam->gV10W56TrgrApDownTime));
				SET_V10_W56_AP_DOWN(pAd, FALSE);

				pDfsParam->DfsChBand[0] = FALSE;
				pDfsParam->DfsChBand[1] = FALSE;
				pDfsParam->RadarDetected[0] = FALSE;
				pDfsParam->RadarDetected[1] = FALSE;
				return;
			}

			pDfsParam->PrimBand = RDD_BAND0;
			pDfsParam->Band0Ch = pDfsParam->PrimCh = BwChannel & 0xFF;
			pDfsParam->Bw = BwChannel >> 8;
		} else {
			WrapDfsSelectChannel(pAd, bandIdx);
		}
#else
		WrapDfsSelectChannel(pAd, bandIdx);
#endif

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]PrimCh: %d, Band0Ch:%d, Band1Ch:%d\n",
			 __func__, pDfsParam->PrimCh, pDfsParam->Band0Ch, pDfsParam->Band1Ch));
	NextCh = pDfsParam->PrimCh;
	for (i = 0; i < DBDC_BAND_NUM; i++)
		RadarBandId[i] = FALSE;
	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		SetChInfo = 0;
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;
		if (wdev->pHObj == NULL)
			continue;
		if (HcGetBandByWdev(wdev) != bandIdx)
			continue;

		if (RadarBandId[bandIdx] == TRUE)
			continue;
		else
			RadarBandId[bandIdx] = TRUE;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]Update wdev of BssIdx %d\n",
				 __func__,
				 BssIdx));
#ifdef WAPP_SUPPORT
		if (first_wdev) {
			wapp_send_radar_detect_notif(pAd, wdev, wdev->channel, 0);
			first_wdev = FALSE;
		}
#endif
		/*Adjust Bw*/
#ifdef BACKGROUND_SCAN_SUPPORT
#ifdef ONDEMAND_DFS
		if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd)) && GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
#else
		if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
#endif
			DfsAdjustBwSetting(wdev, pDfsParam->Bw, pDfsParam->OutBandBw);
			NextBw = pDfsParam->OutBandBw;
		} else {
#else
		{
#endif /* BACKGROUND_SCAN_SUPPORT */

			DfsAdjustBwSetting(wdev, KeepBw, pDfsParam->Bw);
			NextBw = pDfsParam->Bw;
		}

		if (pDfsParam->Dot11_H[bandIdx].RDMode == RD_NORMAL_MODE) {
			pDfsParam->DfsChBand[0] = FALSE;
			pDfsParam->DfsChBand[1] = FALSE;
			pDfsParam->RadarDetected[0] = FALSE;
			pDfsParam->RadarDetected[1] = FALSE;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]\x1b[1;33m Normal Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
					 __func__,
					 NextCh,
					 NextBw));
			rtmp_set_channel(pAd, wdev, NextCh);
		} else if (pDfsParam->Dot11_H[bandIdx].RDMode == RD_SILENCE_MODE) {
			pDfsParam->DfsChBand[0] = FALSE;
			pDfsParam->DfsChBand[1] = FALSE;
			pDfsParam->RadarDetected[0] = FALSE;
			pDfsParam->RadarDetected[1] = FALSE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s]Silence Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
					 __func__,
					 NextCh,
					 NextBw));
			SetChInfo |= NextCh;
			SetChInfo |= (BssIdx << 8);
			SetChInfo |= (bandIdx << 16);
			RTEnqueueInternalCmd(pAd, CMDTHRED_DFS_CAC_TIMEOUT, &SetChInfo, sizeof(UINT_32));
			RTMP_MLME_HANDLER(pAd);
		}
	}
#ifdef TR181_SUPPORT
	/*increase radio channel change count due to radar detection*/
	/*todo: find rdev using api, instead of direct access*/
	ctrl->rdev[bandIdx].pRadioCtrl->DFSTriggeredChannelChangeCount++;
	ctrl->rdev[bandIdx].pRadioCtrl->TotalChannelChangeCount++;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]channel changed for Band[%d]\n", __func__, bandIdx));
#endif

}

BOOLEAN DfsRddReportHandle(/*handle the event of EXT_EVENT_ID_RDD_REPORT*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam, UCHAR ucRddIdx, UCHAR bandIdx)
{
	BOOLEAN RadarDetected = FALSE;
	UCHAR BssIdx;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	BOOLEAN RadarChannel = FALSE;

	if (ucRddIdx == 0 && (pDfsParam->RadarDetected[0] == FALSE) && (pDfsParam->DfsChBand[0])
		&& (pDfsParam->Dot11_H[bandIdx].RDMode != RD_SWITCHING_MODE)) {
		pDfsParam->RadarDetected[0] = TRUE;
		RadarDetected = TRUE;
	}

#ifdef DOT11_VHT_AC
	if ((DBDC_BAND_NUM > 1) || (pDfsParam->Bw == BW_160)) {
		if (ucRddIdx == 1 && (pDfsParam->RadarDetected[1] == FALSE) && (pDfsParam->DfsChBand[1])
			&& (pDfsParam->Dot11_H[bandIdx].RDMode != RD_SWITCHING_MODE)) {
			pDfsParam->RadarDetected[1] = TRUE;
			RadarDetected = TRUE;
		}
	}
#endif

	if (pDfsParam->bDBDCMode && DBDC_BAND_NUM > 1) {
		if (ucRddIdx == 1 && (pDfsParam->RadarDetected[1] == FALSE) && (pDfsParam->DfsChBand[0])
			&& (pDfsParam->Dot11_H[bandIdx].RDMode != RD_SWITCHING_MODE)) {
			pDfsParam->RadarDetected[1] = TRUE;
			RadarDetected = TRUE;
		}
	}

	/* DFS zero wait case */
	if (pDfsParam->bZeroWaitSupport) {
		if ((ucRddIdx == 1) &&
			(pDfsParam->RadarDetected[1] == FALSE) &&
			(pDfsParam->DfsChBand[0]) &&
			(pDfsParam->ZeroWaitDfsState != DFS_RADAR_DETECT)) {
			pDfsParam->RadarDetected[1] = TRUE;
			RadarDetected = TRUE;
		}
	}
	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;
		if (wdev->pHObj == NULL)
			continue;
		if (HcGetBandByWdev(wdev) != bandIdx)
			continue;

		if (RadarChannelCheck(pAd, wdev->channel)) {
			RadarChannel = TRUE;
			break;
		}
	}
	if (RadarChannel == FALSE) {
		RadarDetected = FALSE;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("[WrapDfsRddReportHandle]:  No wdev work on Radar Channel!\n"));
	}

	return RadarDetected;
}

VOID WrapDfsSelectChannel(/*Select new channel*/
	IN PRTMP_ADAPTER pAd, UCHAR bandIdx)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

#ifdef DOT11_VHT_AC
	UCHAR cen_ch_2 = 0;
#endif /*DOT11_VHT_AC*/

	DfsSelectChannel(pAd, pDfsParam);

#ifdef DOT11_VHT_AC

	if (pDfsParam->Bw == BW_8080) {
		if (pDfsParam->PrimBand == RDD_BAND0) {
			cen_ch_2
				= vht_cent_ch_freq(pDfsParam->Band1Ch, VHT_BW_8080);/* Central channel 2; */
		} else {
			cen_ch_2
				= vht_cent_ch_freq(pDfsParam->Band0Ch, VHT_BW_8080);/* Central channel 2;; */
		}

		wlan_config_set_cen_ch_2_all(&pAd->wpf, cen_ch_2);
	}

#endif
}

VOID DfsSelectChannel(/*Select new channel*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam)
{
	UCHAR tempCh = 0;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]:RadarDetected[1]=%d, pDfsParam->DfsChBand[0]=%d\n",
			 __func__,
			 pDfsParam->RadarDetected[1],
			 pDfsParam->DfsChBand[0]));

	if (pDfsParam->Bw == BW_8080) {
		if (pDfsParam->Band0Ch < pDfsParam->Band1Ch) {
			if (pDfsParam->RadarDetected[0] && pDfsParam->DfsChBand[0]) {
				pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE, pDfsParam->Band1Ch);
			}
			if (pDfsParam->RadarDetected[1] && pDfsParam->DfsChBand[1]) {
				pDfsParam->Band1Ch = WrapDfsRandomSelectChannel(pAd, FALSE, pDfsParam->Band0Ch);

			}
		}

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSelectChannel]: 80+80MHz band, selected is %d, %d\n", pDfsParam->Band0Ch, pDfsParam->Band1Ch));

		if (pDfsParam->PrimBand == RDD_BAND0)
			pDfsParam->PrimCh = pDfsParam->Band0Ch;
		else
			pDfsParam->PrimCh = pDfsParam->Band1Ch;

		if (pDfsParam->Band1Ch < pDfsParam->Band0Ch) {
			tempCh = pDfsParam->Band1Ch;
			pDfsParam->Band1Ch = pDfsParam->Band0Ch;
			pDfsParam->Band0Ch = tempCh;
		}

		if (pDfsParam->PrimCh == pDfsParam->Band0Ch)
			pDfsParam->PrimBand = RDD_BAND0;
		else
			pDfsParam->PrimBand = RDD_BAND1;
	} else {
		if ((pDfsParam->Bw == BW_160)
			&& ((pDfsParam->RadarDetected[0] && pDfsParam->DfsChBand[0])
				|| (pDfsParam->RadarDetected[1] && pDfsParam->DfsChBand[1]))) {
			pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE, 0);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSelectChannel]: Single band, selected is %d\n", pDfsParam->Band0Ch));
		} else if (pDfsParam->RadarDetected[0] && pDfsParam->DfsChBand[0]) {
#ifdef BACKGROUND_SCAN_SUPPORT
			if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
				tempCh = WrapDfsRandomSelectChannel(pAd, FALSE, 0);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSelectChannel]: Single band, tempCh selected is %d\n", tempCh));
				pDfsParam->Band0Ch = pDfsParam->OutBandCh;
				if (RadarChannelCheck(pAd, tempCh)) {
					/* If DFS channel is selected randomly by SynA, SynA will use the DFS channel of SynB*/
				pDfsParam->Band0Ch = pDfsParam->OutBandCh;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RDD0 detect. OutBand channel come back to InBand\n"));
				} else {
					pDfsParam->Band0Ch = tempCh;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RDD0 detect. InBand channel is switched to another non-DFS channel randomly\n"));
				}
			}
			else
#endif
				pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE, pDfsParam->DbdcAdditionCh);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSelectChannel]: Single band, selected is %d\n", pDfsParam->Band0Ch));
		} else
			;

		if (pDfsParam->bDBDCMode) {
			if (pDfsParam->RadarDetected[1] && pDfsParam->DfsChBand[0]) {
				pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE, pDfsParam->DbdcAdditionCh);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[DfsSelectChannel]: DBDC band, selected is %d\n", pDfsParam->Band0Ch));
			}
		}


		/* DFS zero wait case */
		if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd)) {
			if (pDfsParam->RadarDetected[1] && pDfsParam->DfsChBand[0]) {
				pDfsParam->Band0Ch = WrapDfsRandomSelectChannel(pAd, FALSE, 0);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]:DFS zero wait,New Ch=%d\n",
						 __func__,
						 pDfsParam->Band0Ch));
			}
		}

		pDfsParam->PrimCh = pDfsParam->Band0Ch;
		pDfsParam->PrimBand = RDD_BAND0;
	}
}

UCHAR WrapDfsRandomSelectChannel(/*Select new channel using random selection*/
	IN PRTMP_ADAPTER pAd, BOOLEAN bSkipDfsCh, UCHAR avoidedCh)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR Channel;
	if (pDfsParam->targetCh != 0) {
		if ((pDfsParam->targetCh != pDfsParam->Band0Ch)
		|| (pDfsParam->targetBw != pDfsParam->Bw)) {
			pDfsParam->Bw = pDfsParam->targetBw;
			Channel = pDfsParam->targetCh;
			return Channel;
		} else {
			pDfsParam->targetCh = 0;
			pDfsParam->targetBw = 0;
			pDfsParam->targetCacValue = 0;
		}
	}

	return DfsRandomSelectChannel(pAd, pDfsParam, bSkipDfsCh, avoidedCh);
}

UCHAR DfsRandomSelectChannel(/*Select new channel using random selection*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam, BOOLEAN bSkipDfsCh, UCHAR avoidedCh)
{
	UINT_8 i, cnt, ch;
	UINT_8 TempChList[MAX_NUM_OF_CHANNELS] = {0};
	cnt = 0;

	if (pDfsParam->bIEEE80211H) {

		for (i = 0; i < pDfsParam->ChannelListNum; i++) {

			if (pDfsParam->DfsChannelList[i].NonOccupancy)
				continue;

			if (AutoChannelSkipListCheck(pAd, pDfsParam->DfsChannelList[i].Channel) == TRUE)
				continue;

			if (!IS_CH_ABAND(pDfsParam->DfsChannelList[i].Channel))
				continue;

			/* Skip DFS channel for DFS zero wait using case */
			if (bSkipDfsCh) {
				if (RadarChannelCheck(pAd, pDfsParam->DfsChannelList[i].Channel))
					continue;
			}

			if (ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, pDfsParam->Bw, pDfsParam))
				continue;

			if ((avoidedCh != 0) && (pDfsParam->Bw == BW_8080)
				&& DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_80) == DfsPrimToCent(avoidedCh, BW_80))
				continue;

			if ((avoidedCh != 0) && (pDfsParam->bDBDCMode == TRUE)
				&& DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, pDfsParam->Bw)
				== DfsPrimToCent(avoidedCh, pDfsParam->Bw))
				continue;

			if (!DfsDedicatedCheckChBwValid(pAd, pDfsParam->DfsChannelList[i].Channel, pDfsParam->Bw))
				continue;

			/* Store available channel to temp list */
			TempChList[cnt++] = pDfsParam->DfsChannelList[i].Channel;
		}

		if (cnt)
			ch = TempChList[RandomByte(pAd) % cnt];
		else {
			USHORT MinTime = 0xFFFF;
			UINT_16 BwChannel = 0;
			ch = 0;
			pDfsParam->bNoAvailableCh = FALSE;
			if (pDfsParam->Bw != BW_8080) {
				BwChannel = DfsBwChQueryByDefault(pAd, BW_160, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, FALSE);
				ch = BwChannel & 0xff;
				pDfsParam->Bw = BwChannel>>8;
				if (ch == 0) {/*No available channel to use*/
					pDfsParam->bNoAvailableCh = TRUE;

					for (i = 0; i < pDfsParam->ChannelListNum; i++) {
						if (pDfsParam->DfsChannelList[i].NonOccupancy < MinTime) {
							if (!IS_CH_ABAND(pDfsParam->DfsChannelList[i].Channel))
								continue;
							if (ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, pDfsParam->Bw, pDfsParam))
								continue;
							if ((avoidedCh != 0)
								&& DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, BW_80) == DfsPrimToCent(avoidedCh, BW_80))
								continue;
							MinTime = pDfsParam->DfsChannelList[i].NonOccupancy;
							ch = pDfsParam->DfsChannelList[i].Channel;
						}
					}
				}
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]:When no available Ch, new pDfsParam->Bw: %d\n",
						__func__, pDfsParam->Bw));
			} else {
				pDfsParam->bNoAvailableCh = TRUE;

				i = RandomByte(pAd) % (pDfsParam->ChannelListNum);
				while (ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, BW_8080, pDfsParam)) {
					i = RandomByte(pAd) % (pDfsParam->ChannelListNum);
				}
				ch = pDfsParam->DfsChannelList[i].Channel;
			}
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]:Currently no immediately available Channel. Choose Ch %d\n",
					 __func__, ch));
		}
	} else {
		ch = pDfsParam->DfsChannelList[RandomByte(pAd) % pDfsParam->ChannelListNum].Channel;

		if (ch == 0)
			ch = pDfsParam->DfsChannelList[0].Channel;

		/* Don't care IEEE80211 disable when bSkipDfsCh is FALSE */
	}

	return ch;
}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
UINT_8 DFS_V10_W52_LIST[V10_W52_SIZE] = {36, 40, 44, 48};
UINT_8 DFS_V10_W53_LIST[V10_W53_SIZE] = {52, 56, 60, 64};
UINT_8 DFS_V10_W56_VHT80_LIST[V10_W56_VHT80_A_SIZE + V10_W56_VHT80_B_SIZE] = {100, 104, 108, 112,\
	116, 120, 124, 128};
UINT_8 DFS_V10_W56_LIST[V10_W56_VHT80_A_SIZE + V10_W56_VHT80_B_SIZE + V10_W56_VHT20_SIZE] = {100, 104, 108, 112,\
	116, 120, 124, 128, 132, 136, 140};
UINT_8 DFS_V10_W56_VHT80_LISTA[V10_W56_VHT80_A_SIZE] = {100, 104, 108, 112};
UINT_8 DFS_V10_W56_VHT80_LISTB[V10_W56_VHT80_B_SIZE] = {116, 120, 124, 128};
UINT_8 DFS_V10_W56_VHT20_LIST[V10_W56_VHT20_SIZE] = {132, 136, 140};

UINT_8 DfsV10FindNonNopChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 chGrp,
	IN UCHAR		 grpWidth)
{
	UCHAR ChIdx = 0;
	UINT_8 channel = 0;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if ((chGrp == W53 || chGrp == W56) && grpWidth && wdev) {
		/*Skip Non occupancy channel*/
		for (ChIdx = 0; ChIdx < grpWidth; ChIdx++) {
			channel = (chGrp == W53) ? (DFS_V10_W53_LIST[ChIdx]) : (DFS_V10_W56_LIST[ChIdx]);
			if (CheckNonOccupancyChannel(pAd, wdev, channel))
				return channel;
		}
	}

	return 0;
}

UINT_8 DfsV10W56FindMaxNopDuration(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR ChIdx = 0;
	USHORT channelNopTime = 0;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	for (ChIdx = 0; ChIdx < pDfsParam->ChannelListNum; ChIdx++) {
		if (pDfsParam->DfsChannelList[ChIdx].Channel >= 100 && pDfsParam->DfsChannelList[ChIdx].Channel <= 140) {
			if (channelNopTime < pDfsParam->DfsChannelList[ChIdx].NonOccupancy)
				channelNopTime = pDfsParam->DfsChannelList[ChIdx].NonOccupancy;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s] MAX NOP %d \n", __func__, channelNopTime));
	return channelNopTime;
}


BOOLEAN DfsV10CheckGrpChnlLeft(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 chGrp,
	IN UCHAR		 grpWidth)
{
	UCHAR ChIdx = 0, ChCnt = 0;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT32 ifIndex = pObj->ioctl_if;
	struct wifi_dev *wdev;

	if (chGrp == W53 || chGrp == W56 || chGrp == W56_UAB || chGrp == W56_UC) {
		if (pObj->ioctl_if == INT_MBSSID)
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		else
			wdev = &pAd->ApCfg.MBSSID[0].wdev;

		/*Skip Non occupancy channel*/
		for (ChIdx = 0; ChIdx < grpWidth; ChIdx++) {
			if (CheckNonOccupancyChannel(pAd, wdev,
				((chGrp == W53) ? (DFS_V10_W53_LIST[ChIdx]) :
				((wlan_config_get_vht_bw(wdev) == VHT_BW_80) ?
					(DFS_V10_W56_VHT80_LIST[ChIdx]) : ((chGrp == W56) ?
					(DFS_V10_W56_LIST[ChIdx]) : (DFS_V10_W56_VHT20_LIST[ChIdx])))))) {
				ChCnt++;
			}
		}
	}

	if (ChCnt)
		return TRUE;
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s]ChGrp %d VHT20 ChCnt %d Empty \n", __func__, chGrp, ChCnt));
		return FALSE;
	}
}

BOOLEAN DfsV10CheckChnlGrpW52(
	IN UCHAR Channel)
{
	UCHAR i = 0;

	while (i < V10_W52_SIZE && Channel != DFS_V10_W52_LIST[i]) {
		i++;
	}

	if (i < V10_W52_SIZE)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN DfsV10CheckChnlGrpW53(
	IN UCHAR Channel)
{
	UCHAR i = 0;

	while (i < V10_W53_SIZE && Channel != DFS_V10_W53_LIST[i]) {
		i++;
	}

	if (i < V10_W53_SIZE)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN DfsV10CheckChnlGrpW56UA(
	IN UCHAR Channel)
{
	UCHAR i = 0;

	while (i < V10_W56_VHT80_A_SIZE && Channel != DFS_V10_W56_VHT80_LISTA[i]) {
		i++;
	}

	if (i < V10_W56_VHT80_A_SIZE)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN DfsV10CheckChnlGrpW56UB(
	IN UCHAR Channel)
{
	UCHAR i = 0;

	while (i < V10_W56_VHT80_B_SIZE && Channel != DFS_V10_W56_VHT80_LISTB[i]) {
		i++;
	}

	if (i < V10_W56_VHT80_B_SIZE)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN DfsV10CheckChnlGrpW56UC(
	IN UCHAR Channel)
{
	UCHAR i = 0;

	while (i < V10_W56_VHT20_SIZE && Channel != DFS_V10_W56_VHT20_LIST[i]) {
		i++;
	}

	if (i < V10_W56_VHT20_SIZE)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN DfsV10CheckW56Grp(
	IN UCHAR channel)
{
	BOOLEAN isW56 = FALSE;

	if (DfsV10CheckChnlGrpW56UA(channel))
		isW56 = TRUE;
	else if (DfsV10CheckChnlGrpW56UB(channel))
		isW56 = TRUE;
	else if (DfsV10CheckChnlGrpW56UC(channel))
		isW56 = TRUE;
	else
		isW56 = FALSE;

	return isW56;
}

UCHAR DfsV10CheckChnlGrp(
	IN UCHAR Channel)
{
	if (DfsV10CheckChnlGrpW52(Channel))
		return W52;
	else if (DfsV10CheckChnlGrpW53(Channel))
		return W53;
	else if (DfsV10CheckChnlGrpW56UA(Channel))
		return W56_UA;
	else if (DfsV10CheckChnlGrpW56UB(Channel))
		return W56_UB;
	else if (DfsV10CheckChnlGrpW56UC(Channel))
		return W56_UC;
	else
		return NA_GRP;
}

BOOLEAN DfsV10W56APDownStart(
	IN PRTMP_ADAPTER pAd,
	IN PAUTO_CH_CTRL pAutoChCtrl,
	IN ULONG	     V10W56TrgrApDownTime)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if (pAutoChCtrl == NULL)
		return FALSE;

	/* Disable AP 30 Minutes */
	pDfsParam->gV10W56TrgrApDownTime = V10W56TrgrApDownTime;
	SET_V10_W56_AP_DOWN(pAd, TRUE);

	/* ReEnable Boot ACS */
	SET_V10_BOOTACS_INVALID(pAd, FALSE);

	SET_V10_W56_GRP_VALID(pAd, TRUE);

	MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_V10_W56_APDOWN_ENBL, 0, NULL, 0);

	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;

	return TRUE;
}

USHORT DfsV10SelectBestChannel(/*Select the Channel from Rank List by ACS*/
	IN PRTMP_ADAPTER pAd,
	IN UCHAR oldChannel)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	USHORT BwChannel = 0;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT32 ifIndex = pObj->ioctl_if;
	struct wifi_dev *wdev;
	UCHAR BandIdx = BAND0;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;

	if (pObj->ioctl_if == INT_MBSSID)
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	else
		wdev = &pAd->ApCfg.MBSSID[0].wdev;

	BandIdx = HcGetBandByWdev(wdev);

	pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

	/* New Channel Identification */
	if (pAd->ApCfg.bAutoChannelAtBootup) {
		/* Pick AutoCh2 Update from List */
		BwChannel = SelectBestV10Chnl_From_List(pAd);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] ACS Disable\n", __func__));
		/* Push CSA BCN Update out of interrupt context */
		SET_V10_AP_BCN_UPDATE_ENBL(pAd, TRUE);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s Select channel = %d from V10 list\n", __func__, BwChannel));

	BwChannel |= (pDfsParam->Bw << 8);
	return BwChannel;
}

/* Weighing Factor for W56>W52>W53 Priority */
VOID DfsV10AddWeighingFactor(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *pwdev)
{
	UCHAR channelIdx = 0, chnlGrp = 0;
	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

	/* ACS Disable: Weighing Factor Not Required */
	if (!pAd->ApCfg.bAutoChannelAtBootup)
		return;

	for (channelIdx = 0; channelIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; channelIdx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel %3d : Busy Time = %6u\n",
				 pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channelIdx].Channel, pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx]));

	chnlGrp = DfsV10CheckChnlGrp(pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channelIdx].Channel);

	if (chnlGrp == W52) {
		if (pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx])
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] *= V10_WEIGH_FACTOR_W52;
		else
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] += (V10_WEIGH_FACTOR_W52 * 10);
	} else if (chnlGrp == W53) {
		if (pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx])
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] *= V10_WEIGH_FACTOR_W53;
		else
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] += (V10_WEIGH_FACTOR_W53 * 10);
	} else if (chnlGrp >= W56_UA && chnlGrp <= W56_UC) {
		if (pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx])
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] *= V10_WEIGH_FACTOR_W56;
		else
			pAutoChCtrl->pChannelInfo->chanbusytime[channelIdx] += (V10_WEIGH_FACTOR_W56 * 10);
	} else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("[%s] Error Group Ch%d", __func__, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[channelIdx].Channel));
	}
}


VOID DfsV10W56APDownTimeCountDown(/*RemainingTimeForUse --*/
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if (IS_SUPPORT_V10_DFS(pAd) && IS_V10_W56_AP_DOWN_ENBLE(pAd)
		&& pDfsParam->gV10W56TrgrApDownTime > 0) {
		pDfsParam->gV10W56TrgrApDownTime--;
		if (!pDfsParam->gV10W56TrgrApDownTime) {
			/* Bring Up AP */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] AP Down Pass \n", __func__));
			MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_V10_W56_APDOWN_FINISH, 0, NULL, 0);
		}
	}
}

VOID DfsV10W56APDownEnbl(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	struct DOT11_H *pDot11hTest = NULL;
	struct wifi_dev *wdev;
	UCHAR BandIdx;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];

	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT32 ifIndex = pObj->ioctl_if;

	if (pObj->ioctl_if == INT_MBSSID)
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	else
		wdev = &pAd->ApCfg.MBSSID[0].wdev;

	BandIdx = HcGetBandByWdev(wdev);
	pDot11hTest = &pAd->Dot11_H[BandIdx];

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("[%s] W56 Down Time Start %d\n", __func__, IS_V10_W56_AP_DOWN_ENBLE(pAd)));

	if (IS_SUPPORT_V10_DFS(pAd) && (!IS_V10_W56_AP_DOWN_ENBLE(pAd) || IS_V10_APINTF_DOWN(pAd))) {
		pDot11hTest->RDCount = 0;
		MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CAC_END, 0, NULL, HcGetBandByWdev(wdev));
		pDot11hTest->RDMode = RD_NORMAL_MODE;

		SET_V10_W56_AP_DOWN(pAd, TRUE);
		APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
}


VOID DfsV10W56APDownPass(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] W56 Down Time Pass\n", __func__));

	if (IS_SUPPORT_V10_DFS(pAd) && IS_V10_W56_AP_DOWN_ENBLE(pAd)) {
		SET_V10_W56_AP_DOWN(pAd, FALSE);
		SET_V10_APINTF_DOWN(pAd, FALSE);
		APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
}

VOID DfsV10APBcnUpdate(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	USHORT BwChannel = 0;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT32 ifIndex = pObj->ioctl_if;
	struct wifi_dev *wdev;
	UCHAR BandIdx = BAND0;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;
	UCHAR NextCh = 0, CurCh = 0;
	UCHAR NextBw = 0;
	UCHAR KeepBw = 0;
	UCHAR BssIdx;
	UINT_32 SetChInfo = 0;
	BSS_STRUCT *pMbss = NULL;

	if (pObj->ioctl_if == INT_MBSSID)
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	else
		wdev = &pAd->ApCfg.MBSSID[0].wdev;

	BandIdx = HcGetBandByWdev(wdev);

	pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

	/* Backup Original channel as we are doing off Channel scan */
	CurCh = wdev->channel;

	if (IS_V10_AP_BCN_UPDATE_ENBL(pAd))
		SET_V10_AP_BCN_UPDATE_ENBL(pAd, FALSE);

	ApAutoChannelSkipListBuild(pAd, wdev);
	if (DfsV10CheckW56Grp(wdev->channel)) {
		if (wlan_config_get_vht_bw(wdev) == VHT_BW_2040) {
			if ((DfsV10CheckGrpChnlLeft(pAd, W56, V10_W56_SIZE) == FALSE)
				|| (IS_V10_W56_VHT80_SWITCHED(pAd) && DfsV10CheckGrpChnlLeft(pAd, W56_UC, V10_W56_VHT20_SIZE) == FALSE)) {
				if (IS_V10_W56_VHT80_SWITCHED(pAd)) {
					/* VHT 20 -> VHT 80 */
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s] BW Switched to VHT80\n", __func__));
					wlan_config_set_ht_bw(wdev, HT_BW_40);
					wlan_config_set_vht_bw(wdev, VHT_BW_80);

#ifdef MCAST_RATE_SPECIFIC
					pAd->CommonCfg.MCastPhyMode.field.BW = HT_BW_40;
					pAd->CommonCfg.MCastPhyMode_5G.field.BW = HT_BW_40;
#ifdef MCAST_BCAST_RATE_SET_SUPPORT
					pAd->CommonCfg.BCastPhyMode.field.BW = pAd->CommonCfg.MCastPhyMode.field.BW;
					pAd->CommonCfg.BCastPhyMode_5G.field.BW = pAd->CommonCfg.MCastPhyMode_5G.field.BW;
#endif /* MCAST_BCAST_RATE_SET_SUPPORT */
#endif /* MCAST_RATE_SPECIFIC */

					SET_V10_W56_VHT80_SWITCH(pAd, FALSE);
				} else
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("[%s] BW Not Switched to VHT80 %d\n", __func__, IS_V10_W56_VHT80_SWITCHED(pAd)));

				/* No Channel Left in W53/ W56_UC VHT20 Case */
				if (DfsV10W56APDownStart(pAd, pAutoChCtrl, V10_W56_APDOWN_TIME))
					goto W56APDOWN;
				else
					ASSERT(BwChannel);
			}
		} else if ((wlan_config_get_vht_bw(wdev) == VHT_BW_80)
			&& (DfsV10CheckGrpChnlLeft(pAd, W56_UAB, V10_W56_VHT80_SIZE) == FALSE)) {
			/* VHT80 -> VHT20 */
			wlan_config_set_ht_bw(wdev, HT_BW_20);
			wlan_config_set_vht_bw(wdev, VHT_BW_2040);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s] BW Switched to VHT20\n", __func__));
#ifdef MCAST_RATE_SPECIFIC
			pAd->CommonCfg.MCastPhyMode.field.BW = HT_BW_20;
			pAd->CommonCfg.MCastPhyMode_5G.field.BW = HT_BW_20;
#ifdef MCAST_BCAST_RATE_SET_SUPPORT
			pAd->CommonCfg.BCastPhyMode.field.BW = pAd->CommonCfg.MCastPhyMode.field.BW;
			pAd->CommonCfg.BCastPhyMode_5G.field.BW = pAd->CommonCfg.MCastPhyMode_5G.field.BW;
#endif /* MCAST_BCAST_RATE_SET_SUPPORT */
#endif /* MCAST_RATE_SPECIFIC */

			SET_V10_W56_VHT80_SWITCH(pAd, TRUE);
			ApAutoChannelSkipListBuild(pAd, wdev);
		}
	}

	/* Perform Off Channel Scan to find channel */
	SET_V10_OFF_CHNL_TIME(pAd, V10_BGND_SCAN_TIME);
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;
	BwChannel = MTAPAutoSelectChannel(pAd, wdev, ChannelAlgBusyTime, TRUE);
	SET_V10_OFF_CHNL_TIME(pAd, V10_NORMAL_SCAN_TIME);
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_SELECTED;

	/* Return to Original RADAR Hit Channel */
	/* Update channel of wdev as new channel */
	AutoChSelUpdateChannel(pAd, CurCh, TRUE, wdev);

	/* Update primay channel */
	wlan_operate_set_prim_ch(wdev, wdev->channel);

W56APDOWN:
	/* W56 Channel Exhausted : Ap Down for 30 Minutes */
	if (!BwChannel && IS_V10_W56_AP_DOWN_ENBLE(pAd)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s] AP Down %ld\n", __func__, pDfsParam->gV10W56TrgrApDownTime));
		SET_V10_W56_AP_DOWN(pAd, FALSE);

		pDfsParam->DfsChBand[0] = FALSE;
		pDfsParam->DfsChBand[1] = FALSE;
		pDfsParam->RadarDetected[0] = FALSE;
		pDfsParam->RadarDetected[1] = FALSE;
		return;
	}

	pDfsParam->PrimBand = RDD_BAND0;
	pDfsParam->Band0Ch = pDfsParam->PrimCh = BwChannel & 0xFF;
	pDfsParam->Bw = BwChannel >> 8;

	NextCh = pDfsParam->PrimCh;
	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;
		if (wdev->pHObj == NULL)
			continue;
		if (HcGetBandByWdev(wdev) != BandIdx)
			continue;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]Update wdev of BssIdx %d\n",
				 __func__,
				 BssIdx));
		/*Adjust Bw*/
#ifdef BACKGROUND_SCAN_SUPPORT
#ifdef ONDEMAND_DFS
		if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd)) && GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
#else
		if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
#endif
			DfsAdjustBwSetting(wdev, pDfsParam->Bw, pDfsParam->OutBandBw);
			NextBw = pDfsParam->OutBandBw;
		} else {
#else
		{
#endif /* BACKGROUND_SCAN_SUPPORT */

			DfsAdjustBwSetting(wdev, KeepBw, pDfsParam->Bw);
			NextBw = pDfsParam->Bw;
		}

		if (pDfsParam->Dot11_H[BandIdx].RDMode == RD_NORMAL_MODE) {
			pDfsParam->DfsChBand[0] = FALSE;
			pDfsParam->DfsChBand[1] = FALSE;
			pDfsParam->RadarDetected[0] = FALSE;
			pDfsParam->RadarDetected[1] = FALSE;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]\x1b[1;33m Normal Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
					 __func__,
					 NextCh,
					 NextBw));
			rtmp_set_channel(pAd, wdev, NextCh);
		} else if (pDfsParam->Dot11_H[BandIdx].RDMode == RD_SILENCE_MODE) {
			pDfsParam->DfsChBand[0] = FALSE;
			pDfsParam->DfsChBand[1] = FALSE;
			pDfsParam->RadarDetected[0] = FALSE;
			pDfsParam->RadarDetected[1] = FALSE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s]Silence Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
					 __func__,
					 NextCh,
					 NextBw));
			SetChInfo |= NextCh;
			SetChInfo |= (BssIdx << 8);
			SetChInfo |= (BandIdx << 16);
			RTEnqueueInternalCmd(pAd, CMDTHRED_DFS_CAC_TIMEOUT, &SetChInfo, sizeof(UINT_32));
			RTMP_MLME_HANDLER(pAd);
		}
	}
}
#endif


#ifdef ONDEMAND_DFS
BOOLEAN CheckNonOccupancyOnDemandChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel)
{
	INT i;
	BOOLEAN InNOP = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

#ifdef MT_DFS_SUPPORT
	DfsNonOccupancyUpdate(pAd);
#endif

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (pChCtrl->ChList[i].Channel == channel) {
			if (pChCtrl->ChList[i].RemainingTimeForUse > 0) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("ERROR: previous detection of a radar on this channel(Channel=%d).\n",
						  pChCtrl->ChList[i].Channel));
				InNOP = TRUE;
				break;
			}
		}
	}

	if ((InNOP == FALSE)
#ifdef MT_DFS_SUPPORT
		|| DfsStopWifiCheck(pAd)
#endif
	)
		return TRUE;
	else
		return FALSE;
}

USHORT DfsOnDemandSelectBestChannel(/*Select the Channel from Rank List by Bgnd Scan*/
	IN PRTMP_ADAPTER pAd, BOOLEAN bSkipDfsCh)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	USHORT BwChannel = 0;
	UCHAR i = 0;

	if (IS_ONDEMAND_ACS_LIST_VALID(pAd)) {
		/* Fetch Channel from ACS Channel List */
		for (i = 0; i < pDfsParam->MaxGroupCount; i++) {
			if (pDfsParam->OnDemandChannelList[i].isConsumed) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]:Channel %d Consumed\n",
					 __func__, pDfsParam->OnDemandChannelList[i].Channel));
				continue;
			}

			if (!CheckNonOccupancyOnDemandChannel(pAd, pDfsParam->OnDemandChannelList[i].Channel)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]:Channel %d NOP\n",
					 __func__, pDfsParam->OnDemandChannelList[i].Channel));
				continue;
			}

			if ((bSkipDfsCh) ? (RadarChannelCheck(pAd, pDfsParam->OnDemandChannelList[i].Channel)) : (FALSE)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]:Channel %d Skip\n",
					 __func__, pDfsParam->OnDemandChannelList[i].Channel));
				continue;
			}

			if ((pDfsParam->OnDemandChannelList[i].Channel == pDfsParam->Band0Ch)
				|| (pDfsParam->OnDemandChannelList[i].Channel == pDfsParam->OutBandCh)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]:Channel %d In Use\n",
					 __func__, pDfsParam->OnDemandChannelList[i].Channel));
				continue;
			}

			BwChannel = pDfsParam->OnDemandChannelList[i].Channel;
			BwChannel |= (pDfsParam->Bw << 8) ;
			pDfsParam->OnDemandChannelList[i].isConsumed = TRUE;
			break;
		}

		if (BwChannel == 0) {
			pDfsParam->bOnDemandChannelListValid = FALSE;
			os_free_mem(pDfsParam->OnDemandChannelList);
		}
	}
#if defined(OFFCHANNEL_SCAN_FEATURE) && defined (ONDEMAND_DFS)
	else {
		BwChannel = SelectBestChannel_From_List(pAd, TRUE, bSkipDfsCh);
		MTWF_LOG(DBG_CAT_ALL, DBG_CAT_ALL, DBG_LVL_OFF, ("%s Select BW channel = %d from background list\n", __func__, BwChannel));
		BwChannel |= (pDfsParam->Bw << 8);
		/* Fetch Channel from BCS Ranking List */
	}
#endif
	/* Trigger Bgnd Scan to ReBuild RankList */
	return BwChannel;
}
#endif
USHORT DfsBwChQueryByDefault(/*Query current available BW & Channel list or select default*/
	IN PRTMP_ADAPTER pAd, UCHAR Bw, PDFS_PARAM pDfsParam, UCHAR level, BOOLEAN bDefaultSelect, BOOLEAN SkipNonDfsCh)
{
	USHORT BwChannel = 0;
	UINT_8 ch = 0;
	UINT_8 i, j, SelectIdx;
	UINT_8 AvailableChCnt = 0;
	BOOLEAN nonWetherBandChExist = FALSE;
	BOOLEAN isSelectWetherBandCh = FALSE;
	if (pDfsParam->bIEEE80211H == FALSE) {
		ch = pDfsParam->DfsChannelList[RandomByte(pAd)%pDfsParam->ChannelListNum].Channel;
		BwChannel |= ch;
		BwChannel |= (Bw << 8);
		return BwChannel;
	}

	for (i = 0; i < pDfsParam->ChannelListNum; i++) {
		if (AutoChannelSkipListCheck(pAd, pDfsParam->DfsChannelList[i].Channel) == TRUE)
			continue;
		if ((SkipNonDfsCh == TRUE) && (!RadarChannelCheck(pAd, pDfsParam->DfsChannelList[i].Channel)))
			continue;
		if (!IS_CH_ABAND(pDfsParam->DfsChannelList[i].Channel))
			continue;
		if (ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, Bw, pDfsParam))
			continue;
		if ((pDfsParam->DfsChannelList[i].NonOccupancy == 0)
		 && (pDfsParam->DfsChannelList[i].NOPClrCnt != 0)
		 && (pDfsParam->DfsChannelList[i].NOPSetByBw == Bw)
		)
			continue;
		if (DfsCheckBwGroupAllAvailable(i, Bw, pDfsParam) == FALSE)
			continue;
		if (DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, pDfsParam->Bw) == DfsPrimToCent(pDfsParam->Band0Ch, pDfsParam->Bw))
			continue;
		if (DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, Bw) == DfsPrimToCent(pDfsParam->Band0Ch, Bw))
			continue;
		if ((level == DFS_BW_CH_QUERY_LEVEL1)
		&& ((pDfsParam->DfsChannelList[i].NonOccupancy == 0) && (pDfsParam->DfsChannelList[i].NOPClrCnt == 0)))
			pDfsParam->AvailableBwChIdx[Bw][AvailableChCnt++] = i;
		if ((level == DFS_BW_CH_QUERY_LEVEL2)
		&& (pDfsParam->DfsChannelList[i].NonOccupancy == 0))
			pDfsParam->AvailableBwChIdx[Bw][AvailableChCnt++] = i;
	}

	if (AvailableChCnt > 0) {
		for (j = 0; j < AvailableChCnt; j++) {
			SelectIdx = pDfsParam->AvailableBwChIdx[Bw][j];
			if ((pDfsParam->RDDurRegion != CE)
				|| !DfsCacRestrictBand(pAd, Bw, pDfsParam->DfsChannelList[SelectIdx].Channel, 0)) {
				nonWetherBandChExist = TRUE;
				break;
			}
		}
		/*randomly select a ch for this BW*/
		SelectIdx = pDfsParam->AvailableBwChIdx[Bw][RandomByte(pAd)%AvailableChCnt];

		if ((pDfsParam->RDDurRegion == CE)
			&& DfsCacRestrictBand(pAd, Bw, pDfsParam->DfsChannelList[SelectIdx].Channel, 0))
			isSelectWetherBandCh = TRUE;
		while (isSelectWetherBandCh == TRUE && nonWetherBandChExist == TRUE) {
			SelectIdx = pDfsParam->AvailableBwChIdx[Bw][RandomByte(pAd)%AvailableChCnt];

			if ((pDfsParam->RDDurRegion == CE)
				&& DfsCacRestrictBand(pAd, Bw, pDfsParam->DfsChannelList[SelectIdx].Channel, 0))
				isSelectWetherBandCh = TRUE;
			else
				isSelectWetherBandCh = FALSE;
		}
		BwChannel |= pDfsParam->DfsChannelList[SelectIdx].Channel;
		BwChannel |= (Bw << 8);
		return BwChannel;
	} else if (level == DFS_BW_CH_QUERY_LEVEL1)
		BwChannel = DfsBwChQueryByDefault(pAd, Bw, pDfsParam, DFS_BW_CH_QUERY_LEVEL2, bDefaultSelect, SkipNonDfsCh);
	else if (level == DFS_BW_CH_QUERY_LEVEL2) {
		if (Bw > BW_20) {
			/*Clear NOP of the current BW*/
			for (i = 0; i < pDfsParam->ChannelListNum; i++) {
				if ((pDfsParam->DfsChannelList[i].NonOccupancy != 0) && (pDfsParam->DfsChannelList[i].NOPSetByBw == Bw)) {
					pDfsParam->DfsChannelList[i].NOPSaveForClear = pDfsParam->DfsChannelList[i].NonOccupancy;
					pDfsParam->DfsChannelList[i].NonOccupancy = 0;
					pDfsParam->DfsChannelList[i].NOPClrCnt++;
				}
			}
			/*reduce BW*/
			BwChannel = DfsBwChQueryByDefault(pAd, Bw - 1, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, bDefaultSelect, SkipNonDfsCh);
		} else
			;/*Will return BwChannel = 0*/
	} else
		;
	return BwChannel;

}

VOID DfsBwChQueryAllList(/*Query current All available BW & Channel list*/
	IN PRTMP_ADAPTER pAd, UCHAR Bw, PDFS_PARAM pDfsParam, BOOLEAN SkipWorkingCh)
{
	UINT_8 i;
	UINT_8 AvailableChCnt = 0;

	if (pDfsParam->bIEEE80211H == FALSE)
		return ;

	for (i = 0; i < pDfsParam->ChannelListNum; i++) {
		if (AutoChannelSkipListCheck(pAd, pDfsParam->DfsChannelList[i].Channel) == TRUE)
			continue;

		if (!IS_CH_ABAND(pDfsParam->DfsChannelList[i].Channel))
			continue;

		if (ByPassChannelByBw(pDfsParam->DfsChannelList[i].Channel, Bw, pDfsParam))
			continue;

		if ((pDfsParam->DfsChannelList[i].NonOccupancy == 0)
		 && (pDfsParam->DfsChannelList[i].NOPClrCnt != 0)
		 && (pDfsParam->DfsChannelList[i].NOPSetByBw <= Bw)
		)
			continue;

		if (DfsCheckBwGroupAllAvailable(i, Bw, pDfsParam) == FALSE)
			continue;

		if (SkipWorkingCh == TRUE) {
			if (DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, pDfsParam->Bw) == DfsPrimToCent(pDfsParam->Band0Ch, pDfsParam->Bw))
				continue;

			if (DfsPrimToCent(pDfsParam->DfsChannelList[i].Channel, Bw) == DfsPrimToCent(pDfsParam->Band0Ch, Bw))
				continue;
		}

		if (pDfsParam->DfsChannelList[i].NonOccupancy == 0)
			pDfsParam->AvailableBwChIdx[Bw][AvailableChCnt++] = i;
	}

	if (Bw > BW_20) {
		/*Clear NOP of the current BW*/
		for (i = 0; i < pDfsParam->ChannelListNum; i++) {
			if ((pDfsParam->DfsChannelList[i].NonOccupancy != 0) && (pDfsParam->DfsChannelList[i].NOPSetByBw == Bw)) {
				pDfsParam->DfsChannelList[i].NOPSaveForClear = pDfsParam->DfsChannelList[i].NonOccupancy;
				pDfsParam->DfsChannelList[i].NonOccupancy = 0;
				pDfsParam->DfsChannelList[i].NOPClrCnt++;
			}
		}
		DfsBwChQueryAllList(pAd, Bw - 1, pDfsParam, SkipWorkingCh);
	}

}

BOOLEAN DfsDedicatedCheckChBwValid(
	IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UINT_8 i, j, idx;

#ifdef ONDEMAND_DFS
	if ((pDfsParam->bDedicatedZeroWaitSupport == FALSE) && (!IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd)))
		return TRUE;
#else
	if (pDfsParam->bDedicatedZeroWaitSupport == FALSE)
		return TRUE;
#endif

	for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {
		for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
			pDfsParam->AvailableBwChIdx[i][j] = 0xff;
	}

	DfsBwChQueryAllList(pAd, BW_80, pDfsParam, FALSE);

	for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {
		for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++) {
			if (pDfsParam->AvailableBwChIdx[i][j] != 0xff) {
				idx = pDfsParam->AvailableBwChIdx[i][j];
				/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF
				("i: %d, j: %d, ChannelList[%d], Ch %d, Bw:%d, CHannel: %d\n",
				i, j, idx, pDfsParam->DfsChannelList[idx].Channel,
				Bw, Channel));*/
				if ((pDfsParam->DfsChannelList[idx].Channel == Channel)
				 && (Bw == i)) {
					/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("Return TRUE\n"));*/
					return TRUE;
				}
			}
		}
	}
	return FALSE;

}

VOID DfsAdjustBwSetting(
	struct wifi_dev *wdev, UCHAR CurrentBw, UCHAR NewBw)
{
	UCHAR HtBw;
	UCHAR VhtBw;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (NewBw == CurrentBw)
		return;

	switch (NewBw) {
	case BW_20:
		HtBw = BW_20;
		VhtBw = VHT_BW_2040;
		break;
	case BW_40:
		HtBw = BW_40;
		VhtBw = VHT_BW_2040;
		break;
	case BW_80:
		HtBw = BW_40;
		VhtBw = VHT_BW_80;
		break;
	case BW_160:
		HtBw = BW_40;
		VhtBw = VHT_BW_160;
		break;
	default:
		return;
		break;
	}

	cfg->ht_conf.ht_bw = HtBw;
	cfg->vht_conf.vht_bw = VhtBw;
}

VOID WrapDfsRadarDetectStart(/*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev
)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	struct freq_oper oper;
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	if (!IS_CH_ABAND(wdev->channel)) {
		return;
	}

	if (hc_radio_query_by_wdev(wdev, &oper)) {
		return;
	}

	if (wdev) {
		pDot11h = wdev->pDot11_H;
	}

	if (pDot11h == NULL)
		return;

	if (pDfsParam->bShowPulseInfo)
		return;

	pDfsParam->DfsChBand[0] = RadarChannelCheck(pAd, pDfsParam->Band0Ch);

#ifdef DOT11_VHT_AC

	if (pDfsParam->Bw == BW_8080)
		pDfsParam->DfsChBand[1] = RadarChannelCheck(pAd, pDfsParam->Band1Ch);

	if (pDfsParam->Bw == BW_160)
		pDfsParam->DfsChBand[1] = pDfsParam->DfsChBand[0];

	if ((pDfsParam->Bw == BW_160) && (pDfsParam->PrimCh >= GROUP1_LOWER && pDfsParam->PrimCh <= GROUP1_UPPER))
		pDfsParam->DfsChBand[1] = TRUE;

#endif
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WrapDfsRadarDetectStart]: DfsChBand[0]: %d, DfsChBand[1]: %d\n",
			 pDfsParam->DfsChBand[0],
			 pDfsParam->DfsChBand[1]));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s] Bandwidth: %d, RDMode: %d\n",
			 __func__,
			 pDfsParam->Bw,
			 pDot11h->RDMode));
	DfsRadarDetectStart(pAd, pDfsParam, wdev);
}

VOID DfsRadarDetectStart(/*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd,
	PDFS_PARAM pDfsParam,
	struct wifi_dev *wdev
)
{
	INT ret1 = TRUE;
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;

	if (ScanRunning(pAd) || (pDot11h->RDMode == RD_SWITCHING_MODE))
		return;

#ifdef ONDEMAND_DFS
	if ((pDot11h->RDMode == RD_SILENCE_MODE)
		|| (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd)
		&& RadarChannelCheck(pAd, wdev->channel) && GET_BGND_STATE(pAd, BGND_ONDMND_CNLSWITCH_ON))) {
#else
	if (pDot11h->RDMode == RD_SILENCE_MODE) {
#endif
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]:ZeroWaitState:%d\n",
				 __func__,
				 GET_MT_ZEROWAIT_DFS_STATE(pAd)));
		if (pDfsParam->RadarDetectState == FALSE) {
			if (pDfsParam->bDBDCMode) {
				/* DBDC mode DFS and Zero wait DFS case */
				ret1 = mtRddControl(pAd, RDD_START, HcGetBandByWdev(wdev), RXSEL_0, REG_DEFAULT); /* RddSel=0: Use band1/RX2 to detect radar */
			}
#ifdef DOT11_VHT_AC
			else if (pDfsParam->Bw == BW_160) {
#if defined(MT7615) || defined(MT7622)
				/*Prim in idx 4~7. WorkAround for "Always set BB Prim 0~3 in IBF issue*/
				if ((pDfsParam->PrimCh >= GROUP2_LOWER && pDfsParam->PrimCh <= GROUP2_UPPER)) {
					ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, RXSEL_0, REG_DEFAULT);
				} else {
#endif

					if ((pDfsParam->Band0Ch >= GROUP1_LOWER && pDfsParam->Band0Ch <= GROUP2_UPPER))
						;
					else
						ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, 0, REG_DEFAULT);

					ret1 = mtRddControl(pAd, RDD_START, HW_RDD1, RXSEL_0, REG_DEFAULT);
				}
			} else if (pDfsParam->Bw == BW_8080) {
#if defined(MT7615) || defined(MT7622)
				if (pDfsParam->PrimBand == RDD_BAND1) {/*Prim in idx 4~7. WorkAround for "Always set BB Prim 0~3 in IBF issue"*/
					if (pDfsParam->DfsChBand[1])
						ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, RXSEL_0, REG_DEFAULT);
					if (pDfsParam->DfsChBand[0])
						ret1 = mtRddControl(pAd, RDD_START, HW_RDD1, RXSEL_0, REG_DEFAULT);
				} else
#endif
				{/*Prim in idx 0~3*/
					if (pDfsParam->DfsChBand[0])
						ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, RXSEL_0, REG_DEFAULT);
					if (pDfsParam->DfsChBand[1])
						ret1 = mtRddControl(pAd, RDD_START, HW_RDD1, RXSEL_0, REG_DEFAULT);
				}
			}

#endif
			else
				ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, RXSEL_0, REG_DEFAULT);
		}

		pDfsParam->RadarDetectState = TRUE;
	} else if (DfsIsOutBandAvailable(pAd) && pDfsParam->bDedicatedZeroWaitSupport) {
		ret1 = mtRddControl(pAd, RDD_START, HW_RDD0, RXSEL_0, REG_DEFAULT);
	}
}

VOID WrapDfsRadarDetectStop(/*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	DfsRadarDetectStop(pAd, pDfsParam);
}

VOID DfsRadarDetectStop(/*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam)
{
	INT ret1 = TRUE, ret2 = TRUE;
	pDfsParam->RadarDetectState = FALSE;

	if (!pDfsParam->bDfsEnable)
		return;

	ret1 = mtRddControl(pAd, RDD_STOP, HW_RDD0, 0, 0);
	ret2 = mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
}

#ifdef ONDEMAND_DFS
VOID DfsOnDemandInBandRDDStart(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	pDfsParam->RadarDetected[0] = FALSE;
	pDfsParam->DfsChBand[0] = RadarChannelCheck(pAd, pDfsParam->Band0Ch);
	if (pDfsParam->DfsChBand[0]) {
		mtRddControl(pAd, RDD_START, HW_RDD0, RXSEL_0, REG_DEFAULT);

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s]: In-Band Radar Start Channel: %d \x1b[m \n",
			__func__, pDfsParam->Band0Ch));
	}
}

UCHAR DfsOnDemandDynamicChannelUpdate(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR Channel)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
	UCHAR Status = DFS_CHNL_UPDATE_FAIL;
	struct wlan_config *cfg = NULL;
	UCHAR phy_bw = 0;
	USHORT BwChannel = 0;

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] NULL Pointer \n", __func__));
		return Status;
	}
	if ((IS_ONDEMAND_DFS_MODE_4x4(pAd)) == FALSE) {
		/* 2x2 State: Return Error to App as Channel Switch NA*/
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] 2x2 Mode Channel = %d\n", __func__, Channel));
		return Status;
	} else {
		/* 4x4 State: Switch to New Channel */
		if (RadarChannelCheck(pAd, Channel)) {
			if (!CheckNonOccupancyOnDemandChannel(pAd, Channel)) {
				/* Channel under NOP Restriction */
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("[%s] NOP Mode Channel = %d\n", __func__, Channel));
				return Status;
			}
			cfg = (struct wlan_config *)wdev->wpf_cfg;
			if (cfg == NULL)
				return Status;
			if (cfg->ht_conf.ht_bw == HT_BW_20)
				phy_bw = BW_20;
			else if (cfg->ht_conf.ht_bw == HT_BW_40) {
				if (cfg->vht_conf.vht_bw == VHT_BW_2040)
					phy_bw = BW_40;
				else if (cfg->vht_conf.vht_bw == VHT_BW_80)
					phy_bw = BW_80;
				else if (cfg->vht_conf.vht_bw == VHT_BW_160)
					phy_bw = BW_160;
				else if (cfg->vht_conf.vht_bw == VHT_BW_8080)
					phy_bw = BW_8080;
			}
			/* Make User Channel as Outband*/
			pDfsParam->OutBandCh = Channel;
			pDfsParam->OutBandBw = phy_bw;
			/* Check if current Channel is DFS */
			if (RadarChannelCheck(pAd, wdev->channel)) {
				/* Pick Non-DFS CHannel from Rank List and move to 2x2 */
				BwChannel = DfsOnDemandSelectBestChannel(pAd, TRUE);
				pDfsParam->Band0Ch = BwChannel & 0xFF;
				pDfsParam->Bw      = BwChannel >> 8;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("[%s]1 OB Chnl %d IB Channel = %d\n", __func__, pDfsParam->OutBandCh, pDfsParam->Band0Ch));
				return DFS_CHNL_UPDATE_PASS;

			} else {
				pDfsParam->Band0Ch = wdev->channel;
				pDfsParam->Bw      = phy_bw;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("[%s]2 OB Chnl %d IB Channel = %d\n", __func__, pDfsParam->OutBandCh, pDfsParam->Band0Ch));
				Status = DFS_CHNL_UPDATE_WAIT;
			}

			/* Restore to Default Bgnd State */
			BgndScanCtrl->BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;

			/* Move to 2x2 */
			/* Initiate OnDemand DFS Zero-Wait State Machine */
			DfsDedicatedOutBandSetChannel(pAd, pDfsParam->OutBandCh, pDfsParam->OutBandBw);
		} else {
			/* Non-DFS Channel selected by User */
			/* Erase Outband Channel State Variables */
			pDfsParam->OutBandCh = 0;
			pDfsParam->OutBandBw = 0;

			pDfsParam->Band0Ch = Channel;
			BgndScanCtrl->BgndScanStatMachine.CurrState = BGND_ONDMND_CNLSWITCH_ON;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] 4x4 Mode Non DFS Channel = %d\n", __func__, Channel));
			Status = DFS_CHNL_UPDATE_PASS;
		}
	}

	return Status;
}
#endif
VOID DfsDedicatedOutBandRDDStart(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	pDfsParam->RadarDetected[1] = FALSE;
	pDfsParam->DfsChBand[1] = RadarChannelCheck(pAd, pDfsParam->OutBandCh);
	if (pDfsParam->DfsChBand[1]) {
		mtRddControl(pAd, RDD_START, HW_RDD1, RXSEL_0, REG_DEFAULT);
		DfsOutBandCacReset(pAd);

		if ((pDfsParam->RDDurRegion == CE)
		 && DfsCacRestrictBand(pAd, pDfsParam->OutBandBw, pDfsParam->OutBandCh, 0))
			pDfsParam->DedicatedOutBandCacTime = 605;
		else
			pDfsParam->DedicatedOutBandCacTime = 65;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s]: Dedicated CAC time: %d \x1b[m \n",
			__func__, pDfsParam->DedicatedOutBandCacTime));
	}
}

VOID DfsDedicatedOutBandRDDRunning(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	USHORT BwChannel;

	mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);

	if ((pDfsParam->bDedicatedZeroWaitDefault == TRUE)
#ifdef ONDEMAND_DFS
		|| IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd)) {
#else
		) {
#endif

#ifdef ONDEMAND_DFS
		if (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd)) {
			/* Pick best channel from Rank List*/
			BwChannel = DfsOnDemandSelectBestChannel (pAd, FALSE);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\x1b[1;33m [%s]: New Channel: %d \x1b[m \n",
				__func__, (BwChannel & 0xff)));

			if (!RadarChannelCheck(pAd, (BwChannel & 0xff))) {

				/* Non-Dfs Channel Selected; Erase Outband Channel
				State Variables, as we are moving in 4*4 Mode */

				pDfsParam->Band0Ch = BwChannel & 0xff;
				pDfsParam->OutBandBw = BwChannel >> 8;
			}
		} else {

#endif
			BwChannel = DfsBwChQueryByDefault(pAd, BW_80, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, TRUE);
#ifdef ONDEMAND_DFS
		}
#endif
		pDfsParam->OutBandCh = BwChannel & 0xff;
		pDfsParam->OutBandBw = BwChannel>>8;
	}
}

VOID DfsDedicatedOutBandRDDStop(
	IN PRTMP_ADAPTER pAd)
{
	mtRddControl(pAd, RDD_STOP, HW_RDD1, 0, 0);
}

BOOLEAN DfsIsRadarHitReport(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	return pDfsParam->RadarHitReport == TRUE;
}

VOID DfsRadarHitReportReset(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->RadarHitReport = FALSE;
}

VOID DfsReportCollision(
	IN PRTMP_ADAPTER pAd)
{
#ifdef BACKGROUND_SCAN_SUPPORT
#ifdef ONDEMAND_DFS
	if ((IS_SUPPORT_DEDICATED_ZEROWAIT_DFS(pAd) || (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd)))
		&& DfsIsRadarHitReport(pAd)) {
#else
	if (IS_SUPPORT_DEDICATED_ZEROWAIT_DFS(pAd) && DfsIsRadarHitReport(pAd)) {
#endif
		ZeroWait_DFS_collision_report(pAd, HW_RDD0,
		GET_BGND_PARAM(pAd, ORI_INBAND_CH), GET_BGND_PARAM(pAd, ORI_INBAND_BW));
		DfsRadarHitReportReset(pAd);
	}
#endif
}

BOOLEAN DfsIsTargetChAvailable(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	if ((pDfsParam->targetCh != 0) && (pDfsParam->targetCacValue == 0))
		return TRUE;

	return FALSE;
}

BOOLEAN DfsIsOutBandAvailable(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	BOOLEAN bAvailable;

	bAvailable = (pDfsParam->bOutBandAvailable == TRUE)
		&& (pDfsParam->bSetInBandCacReStart == FALSE);

	if (bAvailable == TRUE)
		pDfsParam->OutBandAvailableCh = pDfsParam->Band0Ch;

	if (pDfsParam->Band0Ch == pDfsParam->OutBandAvailableCh)
		bAvailable = TRUE;
	else
		pDfsParam->OutBandAvailableCh = 0;

	return bAvailable;
}

VOID DfsOutBandCacReset(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->DedicatedOutBandCacCount = 0;
	pDfsParam->bOutBandAvailable = FALSE;
}

VOID DfsSetCacRemainingTime(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;
	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;

	if (pDfsParam->bDedicatedZeroWaitSupport == TRUE) {
		if ((pDot11h->RDMode == RD_SILENCE_MODE) && (pDfsParam->bSetInBandCacReStart == FALSE)) {
			pDot11h->RDCount = pDfsParam->DedicatedOutBandCacCount;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Remaining CAC time is %d \x1b[m \n",
			__func__, pDot11h->ChMovingTime - pDot11h->RDCount));
		}
	}

	pDfsParam->bSetInBandCacReStart = FALSE;
	DfsOutBandCacReset(pAd);

}

VOID DfsOutBandCacCountUpdate(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

#ifdef BACKGROUND_SCAN_SUPPORT
	if (!GET_BGND_STATE(pAd, BGND_RDD_DETEC))
		return;
#endif

#ifdef ONDEMAND_DFS
	if (((pDfsParam->bDedicatedZeroWaitSupport == TRUE) || (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd))) && (pDfsParam->bOutBandAvailable == FALSE) && (pDfsParam->DedicatedOutBandCacTime > 0)) {
#else
	if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE) && (pDfsParam->bOutBandAvailable == FALSE)) {
#endif
		if (pDfsParam->DedicatedOutBandCacCount++ > pDfsParam->DedicatedOutBandCacTime) {
			pDfsParam->bOutBandAvailable = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] OutBand(SynB) CAC complete and is available now.\n", __func__));
			if (DfsCacTimeOutCallBack) {
				DfsCacTimeOutCallBack(RDD_BAND1, pDfsParam->OutBandBw, pDfsParam->OutBandCh);
			}

#ifdef ONDEMAND_DFS
			if (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd)) {
				MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_ONDEMAND_CAC_FINISH, 0, NULL, 0);
			}
#endif
		}
	}
}

VOID DfsDedicatedExamineSetNewCh(
	IN struct _RTMP_ADAPTER *pAd, UCHAR Channel)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR InputCentCh = DfsPrimToCent(Channel, pDfsParam->Bw);

	if ((pDfsParam->bDedicatedZeroWaitSupport == FALSE)
#ifdef ONDEMAND_DFS
		&& (!IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd))
#endif
		)
		return;

	if (InputCentCh == DfsPrimToCent(pDfsParam->OutBandCh, pDfsParam->Bw))
		pDfsParam->bSetInBandCacReStart = FALSE;
	else
		pDfsParam->bSetInBandCacReStart = TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief     Configure (Enable/Disable) HW RDD and RDD wrapper module
*
* \param[in] ucRddCtrl
*            ucRddIdex
*
*
* \return    None
*/
/*----------------------------------------------------------------------------*/

INT mtRddControl(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR ucRddCtrl,
	IN UCHAR ucRddIdex,
	IN UCHAR ucRddRxSel,
	IN UCHAR ucSetVal)
{
	INT ret = TRUE;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[mtRddControl]RddCtrl=%d, RddIdx=%d, RddRxSel=%d\n", ucRddCtrl, ucRddIdex, ucRddRxSel));
	ret = MtCmdRddCtrl(pAd, ucRddCtrl, ucRddIdex, ucRddRxSel, ucSetVal);
	return ret;
}

UCHAR DfsGetCentCh(IN PRTMP_ADAPTER pAd, IN UCHAR Channel, IN UCHAR bw, struct wifi_dev *wdev)
{
	UCHAR CentCh = 0;

	if (bw == BW_20)
		CentCh = Channel;

#ifdef DOT11_N_SUPPORT
	else if ((bw == BW_40) && N_ChannelGroupCheck(pAd, Channel, wdev)) {
#ifdef A_BAND_SUPPORT

		if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
			(Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
			CentCh = Channel + 2;
		else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
			(Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161))
			CentCh = Channel - 2;
#endif /* A_BAND_SUPPORT */
	}

#ifdef DOT11_VHT_AC
	else if (bw == BW_80) {
		if (vht80_channel_group(pAd, Channel))
			CentCh = vht_cent_ch_freq(Channel, VHT_BW_80);
	} else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s]Error!Unexpected Bw=%d!!\n",
				 __func__,
				 bw));
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]Control/Central Ch=%d/%d;Bw=%d\n",
			 __func__,
			 Channel,
			 CentCh,
			 bw));
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	return CentCh;
}

#ifdef BACKGROUND_SCAN_SUPPORT
/* MBSS Zero Wait */
BOOLEAN MbssZeroWaitStopValidate(PRTMP_ADAPTER pAd, UCHAR MbssCh, INT MbssIdx)
{
	BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
	BOOLEAN ZeroWaitStop = FALSE;

	if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) &&
		CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC) &&
		(pAd->ApCfg.BssidNum > 1)
	   ) {
		if (GET_MT_MT_INIT_ZEROWAIT_MBSS(pAd)) {
			ZeroWaitStop = TRUE;
			/* Only mbss0 to do ZeroWaitStop for non-mbss init scenarios */
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Mbss ZeroWait Stop, Mbss/CurCh=%d/%d,mbssIdx=%d,BgnStat=%ld\n",
					 __func__,
					 MbssCh,
					 HcGetChannelByRf(pAd, RFIC_5GHZ),
					 MbssIdx,
					 pAd->BgndScanCtrl.BgndScanStatMachine.CurrState));
			UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_MBSS_CAC);
			/* Terminate Zero wait flow */
			DfsZeroWaitStopAction(pAd, NULL);

			/* Update DfsZeroWait channel */
			if (MbssCh != 0 && MbssIdx != 0)
				BgndScanCtrl->DfsZeroWaitChannel = MbssCh;
		} else {
			if (pAd->ApCfg.BssidNum > 1)
				UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE);
		}
	}

	return ZeroWaitStop;
}

VOID ZeroWaitUpdateForMbss(PRTMP_ADAPTER pAd, BOOLEAN bZeroWaitStop, UCHAR MbssCh, INT MbssIdx)
{
#ifdef BACKGROUND_SCAN_SUPPORT
	BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
#endif
	UINT8 Channel;

	if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd)
		&& (!CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_INSERV_MONI))
		&& (pAd->ApCfg.BssidNum > 1)
		&& (((MbssIdx == pAd->ApCfg.BssidNum - 1) || bZeroWaitStop) ||
			(GET_MT_MT_INIT_ZEROWAIT_MBSS(pAd) && CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE)))
	   ) {
		/* Non-Dfs CH selection for new  Zero Wait trigger if ineed */
		if (RadarChannelCheck(pAd, MbssCh)) {
			BgndScanCtrl->DfsZeroWaitChannel = MbssCh;
			Channel = WrapDfsRandomSelectChannel(pAd, TRUE, 0); /* Skip DFS CH */
			wdev_sync_ch_by_rfic(pAd, RFIC_5GHZ, Channel);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][Dfs]New Mbss/CurCh=%d/%d\n",
					 __func__,
					 Channel,
					 HcGetChannelByRf(pAd, RFIC_5GHZ)));
			UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC);
		} else {
			if (RadarChannelCheck(pAd, BgndScanCtrl->DfsZeroWaitChannel)) {
				if (MbssIdx != pAd->ApCfg.BssidNum - 1) {
					/* Temporary Zero wait state for MBSS nonDFS CH */
					UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE);
				} else {
					/* For latest MBSS DFS CH */
					UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC);
				}
			} else {
				BgndScanCtrl->DfsZeroWaitChannel = 0;
				UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE);
			}

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s][NonDfs]New Mbss/CurCh=%d/%d,ZeroWaitCh=%d,stat=%d\n",
					 __func__,
					 MbssCh,
					 HcGetChannelByRf(pAd, RFIC_5GHZ),
					 BgndScanCtrl->DfsZeroWaitChannel,
					 GET_MT_ZEROWAIT_DFS_STATE(pAd)));
		}
	}
}

VOID DfsDedicatedScanStart(IN PRTMP_ADAPTER pAd)
{
	USHORT BwChannel;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	if ((pDfsParam->bDedicatedZeroWaitSupport == TRUE)
		&& (pDfsParam->bDedicatedZeroWaitDefault == TRUE)) {

			BwChannel = DfsBwChQueryByDefault(pAd, BW_80, pDfsParam, DFS_BW_CH_QUERY_LEVEL1, TRUE, TRUE);
			pDfsParam->OutBandCh = BwChannel & 0xff;
			pDfsParam->OutBandBw = BwChannel>>8;

		if (pDfsParam->OutBandCh == 0) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] No available Outband BW\n",
					 __func__));
			return;
		}

		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_DEDICATE_RDD_REQ, 0, NULL, 0);
		RTMP_MLME_HANDLER(pAd);
	}

#ifdef ONDEMAND_DFS
	if (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd) && RadarChannelCheck(pAd, pAd->CommonCfg.DfsParameter.Band0Ch)
		&& !GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
		/* Update current background state to Radar Detection */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Bgnd State Updated to BGND_RDD_DETEC\n", __func__));
		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_RDD_DETEC;

		/* Trigger CAC only when Required */
		pDfsParam->bOutBandAvailable = TRUE;
	}
#endif
}

VOID DfsInitDedicatedScanStart(IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	if (pDfsParam->bInitOutBandBranch == TRUE) {
		pDfsParam->bInitOutBandBranch = FALSE;
		DfsDedicatedScanStart(pAd);
	}
}

VOID DfsSetInitDediatedScanStart(IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->bInitOutBandBranch = TRUE;
}

VOID DfsDedicatedInBandSetChannel(
	IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw, BOOLEAN doCAC)
{
	UCHAR NextCh;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR bandIdx;
	UCHAR BssIdx;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	struct DOT11_H *pDot11h = NULL;
	UINT_32 SetChInfo = 0;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] SynNum: %d, Channel: %d, Bw: %d \x1b[m \n",
		__func__, 0, Channel, Bw));

#ifdef ONDEMAND_DFS
	if ((pDfsParam->bDedicatedZeroWaitSupport == FALSE) && (!IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd)))
		return;
#else
	if (pDfsParam->bDedicatedZeroWaitSupport == FALSE)
		return;
#endif

	if (!DfsDedicatedCheckChBwValid(pAd, Channel, Bw)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Not a Valid InBand Channel. Fail. \x1b[m \n", __FUNCTION__));
		return;
	}

	if (Channel == 0 || ((Channel == pDfsParam->OutBandCh)
		&&  (Bw == pDfsParam->OutBandBw))) {
		Channel = pDfsParam->OutBandCh;
		Bw = pDfsParam->OutBandBw;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m InBand set to OutBand Channel %d, Bw :%d \x1b[m \n", Channel, Bw));
	} else {
		pDfsParam->bSetInBandCacReStart = TRUE;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m InBand set to non-OutBand Channel %d, Bw %d \x1b[m \n", Channel, Bw));
	}

	bandIdx = HcGetBandByChannel(pAd, Channel);
	pDot11h = &pAd->Dot11_H[bandIdx];

	if ((Channel == pDfsParam->Band0Ch)  &&  (Bw == pDfsParam->Bw)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m This is current Ch %d, Bw %d \x1b[m \n", Channel, Bw));
		if ((doCAC == FALSE) && (pDot11h->RDMode == RD_SILENCE_MODE)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m Enable beacon now \x1b[m \n"));
			pDot11h->RDCount = pDot11h->ChMovingTime;
		}
		return;
	}

	if (doCAC == FALSE) {
		pDfsParam->bSetInBandCacReStart = FALSE;
		pDfsParam->bOutBandAvailable = TRUE;
	}


	pDfsParam->OrigInBandCh = pDfsParam->Band0Ch;
	pDfsParam->OrigInBandBw = pDfsParam->Bw;
	pDfsParam->Band0Ch = Channel;
	pDfsParam->PrimCh = pDfsParam->Band0Ch;
	pDfsParam->PrimBand = RDD_BAND0;
	NextCh = pDfsParam->PrimCh;

	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		pMbss = &pAd->ApCfg.MBSSID[BssIdx];
		wdev = &pMbss->wdev;

		if (wdev->pHObj == NULL)
			continue;
		if (HcGetBandByWdev(wdev) != bandIdx)
			continue;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]Update wdev of BssIdx %d\n",
				 __func__,
				 BssIdx));

		/*Adjust Bw*/
		DfsAdjustBwSetting(wdev, pDfsParam->Bw, Bw);

		if (pDot11h->RDMode == RD_NORMAL_MODE) {
			pDfsParam->DfsChBand[0] = FALSE;
			pDfsParam->DfsChBand[1] = FALSE;
			pDfsParam->RadarDetected[0] = FALSE;
			pDfsParam->RadarDetected[1] = FALSE;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]\x1b[1;33m Normal Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
						 __func__,
						 NextCh,
						 Bw));

			rtmp_set_channel(pAd, wdev, NextCh);
		} else if (pDot11h->RDMode == RD_SILENCE_MODE) {
			pDfsParam->DfsChBand[0] = FALSE;
			pDfsParam->DfsChBand[1] = FALSE;
			pDfsParam->RadarDetected[0] = FALSE;
			pDfsParam->RadarDetected[1] = FALSE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s]Silence Mode. Update Uniform Ch=%d, BW=%d \x1b[m\n",
						 __func__,
						 NextCh,
						 Bw));
			SetChInfo |= NextCh;
			SetChInfo |= (BssIdx << 8);
			SetChInfo |= (bandIdx << 16);

			RTEnqueueInternalCmd(pAd, CMDTHRED_DFS_CAC_TIMEOUT, &SetChInfo, sizeof(UINT_32));
			RTMP_MLME_HANDLER(pAd);
		}
	}
}

VOID DfsDedicatedOutBandSetChannel(IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] SynNum: %d, Channel: %d, Bw: %d \x1b[m \n",
		__FUNCTION__, 1, Channel, Bw));

#ifdef ONDEMAND_DFS
	if ((pDfsParam->bDedicatedZeroWaitSupport == FALSE) && (!IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd)))
		return;
#else
	if (pDfsParam->bDedicatedZeroWaitSupport == FALSE)
		return;
#endif

	if (!DfsDedicatedCheckChBwValid(pAd, Channel, Bw)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Not a Valid OutBand Channel. Fail. \x1b[m \n", __FUNCTION__));
		return;
	}
	if (!RadarChannelCheck(pAd, Channel)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Not a DFS Channel. No need to split out for Radar Detection. \x1b[m \n", __FUNCTION__));
		return;
	}

	if (Channel != 0) {
		pDfsParam->OutBandCh = Channel;
		pDfsParam->OutBandBw = Bw;
	} else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m Pick OutBand Ch by internal Alogorithm \x1b[m \n"));
	}

	if (GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m Dediated Running: OutBand set Channel to %d \x1b[m \n", Channel));
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_OUTBAND_SWITCH, 0, NULL, 0);
		RTMP_MLME_HANDLER(pAd);
	} else if (GET_BGND_STATE(pAd, BGND_SCAN_IDLE)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m Dedicated Start: OutBand set Channel to %d \x1b[m \n", Channel));
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_DEDICATE_RDD_REQ, 0, NULL, 0);
		RTMP_MLME_HANDLER(pAd);
	} else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m Wrong state. OutBand Set Channel Fail \x1b[m \n"));
	}
}

VOID DfsDedicatedDynamicCtrl(IN PRTMP_ADAPTER pAd, UINT_32 DfsDedicatedOnOff)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] DfsDedicatedOnOff: %d \x1b[m \n",
		__FUNCTION__, DfsDedicatedOnOff));

	if (DfsDedicatedOnOff == DYNAMIC_ZEROWAIT_OFF) {
		if (GET_BGND_STATE(pAd, BGND_RDD_DETEC)) {
			pDfsParam->OrigInBandCh = pDfsParam->PrimCh;
			pDfsParam->OrigInBandBw = pDfsParam->Bw;
			DedicatedZeroWaitStop(pAd, FALSE);
			DfsOutBandCacReset(pAd);
			pDfsParam->RadarDetected[1] = FALSE;
		} else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Already in 4x4 mode \x1b[m \n", __FUNCTION__));
	} else	{
		if (GET_BGND_STATE(pAd, BGND_RDD_DETEC))
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Already in 2x2 mode \x1b[m \n", __FUNCTION__));
		else if (pDfsParam->OutBandCh == 0)
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] No SynB Info Recorded. Fail. \x1b[m \n", __FUNCTION__));
		else {

			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_DEDICATE_RDD_REQ, 0, NULL, 0);
			RTMP_MLME_HANDLER(pAd);
		}
	}
}
#endif /* BACKGROUND_SCAN_SUPPORT */

INT Set_ModifyChannelList_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Value;
	UCHAR Bw80Num = 4;
	UCHAR Bw40Num = 10;
	UCHAR Bw20Num = 11;

	DFS_REPORT_AVALABLE_CH_LIST Bw80AvailableChList[4]
	= {{116, 0}, {120, 0}, {124, 0}, {128, 0} };
	DFS_REPORT_AVALABLE_CH_LIST Bw40AvailableChList[10]
	= {{100, 0}, {104, 0}, {108, 0}, {112, 0}, {116, 0}, {120, 0}, {124, 0}, {128, 0}, {132, 0}, {136, 0} };
	DFS_REPORT_AVALABLE_CH_LIST Bw20AvailableChList[11]
	= {{100, 0}, {104, 0}, {108, 0}, {112, 0}, {116, 0}, {120, 0}, {124, 0}, {128, 0}, {132, 0}, {136, 0}, {140, 0} };

	Value = (UCHAR) simple_strtol(arg, 0, 10);

	ZeroWait_DFS_Initialize_Candidate_List(pAd,
	Bw80Num, &Bw80AvailableChList[0],
	Bw40Num, &Bw40AvailableChList[0],
	Bw20Num, &Bw20AvailableChList[0]);

	return TRUE;
}
INT Show_available_BwCh_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	DfsProvideAvailableChList(pAd);
	return TRUE;
}

INT Show_NOP_Of_ChList(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	union dfs_zero_wait_msg msg;
	UCHAR i = 0;
	os_zero_mem(&msg, sizeof(union dfs_zero_wait_msg));

	DfsProvideNopOfChList(pAd, &msg);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	("[%s]\n", __func__));

	for (i = 0; i < msg.nop_of_channel_list_msg.NOPTotalChNum; i++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("NopReportChList[%d].Channel = %d, Bw = %d, NOP = %d\n",
		i, msg.nop_of_channel_list_msg.NopReportChList[i].Channel, msg.nop_of_channel_list_msg.NopReportChList[i].Bw,
		msg.nop_of_channel_list_msg.NopReportChList[i].NonOccupancy));
	}

	return TRUE;
}

INT Show_Target_Ch_Info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ZeroWait_DFS_Next_Target_Show(pAd, 1);
	return TRUE;
}

VOID ZeroWait_DFS_Initialize_Candidate_List(
	IN PRTMP_ADAPTER pAd,
	UCHAR Bw80Num, PDFS_REPORT_AVALABLE_CH_LIST pBw80AvailableChList,
	UCHAR Bw40Num, PDFS_REPORT_AVALABLE_CH_LIST pBw40AvailableChList,
	UCHAR Bw20Num, PDFS_REPORT_AVALABLE_CH_LIST pBw20AvailableChList)
{
	UINT_8 i = 0, j = 0, k = 0;
	UINT_8 ChIdx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR SupportBwBitMap[MAX_NUM_OF_CHS] = {0};
	UCHAR OrigSupportBwBitMap[MAX_NUM_OF_CHS] = {0};

	for (i = 0; i < DBDC_BAND_NUM; i++) {
		if (pAd->Dot11_H[i].RDMode == RD_SWITCHING_MODE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel Lsit init fail during channel switch\n"));
			return;
		}
	}

	i = 0;

	for (ChIdx = 0; ChIdx < pDfsParam->ChannelListNum; ChIdx++) {
		if (pBw80AvailableChList->Channel == pDfsParam->DfsChannelList[ChIdx].Channel) {
			SupportBwBitMap[ChIdx] |= 0x04;
			if (i++ < Bw80Num)
				pBw80AvailableChList++;
		}
		if (pBw40AvailableChList->Channel == pDfsParam->DfsChannelList[ChIdx].Channel) {
			SupportBwBitMap[ChIdx] |= 0x02;
			if (j++ < Bw40Num)
				pBw40AvailableChList++;
		}
		if (pBw20AvailableChList->Channel == pDfsParam->DfsChannelList[ChIdx].Channel) {
			SupportBwBitMap[ChIdx] |= 0x01;
			if (k++ < Bw20Num)
				pBw20AvailableChList++;
		}
	}
	for (ChIdx = 0; ChIdx < pDfsParam->ChannelListNum; ChIdx++) {
		OrigSupportBwBitMap[ChIdx] = pDfsParam->DfsChannelList[ChIdx].SupportBwBitMap;

		if (OrigSupportBwBitMap[ChIdx] >= 0x07) {
			if (SupportBwBitMap[ChIdx] == 0x07)
				;
			else if (SupportBwBitMap[ChIdx] == 0x03) {
				pDfsParam->DfsChannelList[ChIdx].NOPSetByBw = BW_80;
				pDfsParam->DfsChannelList[ChIdx].NOPClrCnt = 1;
				pDfsParam->DfsChannelList[ChIdx].NOPSaveForClear = 1800;
			} else if (SupportBwBitMap[ChIdx] == 0x01) {
				pDfsParam->DfsChannelList[ChIdx].NOPSetByBw = BW_40;
				pDfsParam->DfsChannelList[ChIdx].NOPClrCnt = 1;
				pDfsParam->DfsChannelList[ChIdx].NOPSaveForClear = 1800;
			} else if (SupportBwBitMap[ChIdx] == 0x0) {
				pDfsParam->DfsChannelList[ChIdx].NOPSetByBw = BW_20;
				pDfsParam->DfsChannelList[ChIdx].NOPClrCnt = 1;
				pDfsParam->DfsChannelList[ChIdx].NOPSaveForClear = 1800;
			} else
				;
		} else if (OrigSupportBwBitMap[ChIdx] == 0x03) {
			if (SupportBwBitMap[ChIdx] == 0x03)
				;
			else if (SupportBwBitMap[ChIdx] == 0x01) {
				pDfsParam->DfsChannelList[ChIdx].NOPSetByBw = BW_40;
				pDfsParam->DfsChannelList[ChIdx].NOPClrCnt = 1;
				pDfsParam->DfsChannelList[ChIdx].NOPSaveForClear = 1800;
			} else if (SupportBwBitMap[ChIdx] == 0x0) {
				pDfsParam->DfsChannelList[ChIdx].NOPSetByBw = BW_20;
				pDfsParam->DfsChannelList[ChIdx].NOPClrCnt = 1;
				pDfsParam->DfsChannelList[ChIdx].NOPSaveForClear = 1800;
			} else
				;
		} else if (OrigSupportBwBitMap[ChIdx] == 0x01) {
			if (SupportBwBitMap[ChIdx] == 0x01)
				;
			else if (SupportBwBitMap[ChIdx] == 0x0) {
				pDfsParam->DfsChannelList[ChIdx].NOPSetByBw = BW_20;
				pDfsParam->DfsChannelList[ChIdx].NOPClrCnt = 1;
				pDfsParam->DfsChannelList[ChIdx].NOPSaveForClear = 1800;
			} else
				;
		} else
			;
	}

	for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {
		for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
			pDfsParam->AvailableBwChIdx[i][j] = 0xff;
	}

	DfsBwChQueryAllList(pAd, BW_80, pDfsParam, TRUE);

	for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {
#ifdef DFS_DBG_LOG_0
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw: %d\n", i));
#endif
		for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++) {
			if (pDfsParam->AvailableBwChIdx[i][j] != 0xff) {
				ChIdx = pDfsParam->AvailableBwChIdx[i][j];
#ifdef DFS_DBG_LOG_0
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
				ChIdx, pDfsParam->DfsChannelList[ChIdx].Channel,
				pDfsParam->DfsChannelList[ChIdx].NOPClrCnt));
#endif
			}
		}
	}
}

VOID DfsProvideAvailableChList(
	IN PRTMP_ADAPTER pAd)
{
	UINT_8 i, j, idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {
		for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++)
			pDfsParam->AvailableBwChIdx[i][j] = 0xff;
	}

	for (i = 0; i < DBDC_BAND_NUM; i++) {
		if (pAd->Dot11_H[i].RDMode == RD_SWITCHING_MODE)
			return;
	}

	DfsBwChQueryAllList(pAd, BW_80, pDfsParam, TRUE);

	for (i = 0; i < DFS_AVAILABLE_LIST_BW_NUM; i++) {

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bw: %d\n", i));

		for (j = 0; j < DFS_AVAILABLE_LIST_CH_NUM; j++) {
			if (pDfsParam->AvailableBwChIdx[i][j] != 0xff) {
				idx = pDfsParam->AvailableBwChIdx[i][j];
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("ChannelList[%d], Ch %d, RadarHitCnt: %d\n",
				idx, pDfsParam->DfsChannelList[idx].Channel,
				pDfsParam->DfsChannelList[idx].NOPClrCnt));
			}
		}
	}
}

VOID DfsProvideNopOfChList(
	IN PRTMP_ADAPTER pAd,
	union dfs_zero_wait_msg *msg)
{
	UINT_8 i;
	UINT_8 j = 0;

	NOP_REPORT_CH_LIST NopReportChList[DFS_AVAILABLE_LIST_CH_NUM];
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	for (i = 0; i < pDfsParam->ChannelListNum; i++) {
		if (pDfsParam->DfsChannelList[i].NonOccupancy != 0) {
			NopReportChList[j].Channel = pDfsParam->DfsChannelList[i].Channel;
			NopReportChList[j].Bw = pDfsParam->DfsChannelList[i].NOPSetByBw;
			NopReportChList[j].NonOccupancy = pDfsParam->DfsChannelList[i].NonOccupancy;
			j++;
		} else if (pDfsParam->DfsChannelList[i].NOPSaveForClear != 0) {
			NopReportChList[j].Channel = pDfsParam->DfsChannelList[i].Channel;
			NopReportChList[j].Bw = pDfsParam->DfsChannelList[i].NOPSetByBw;
			NopReportChList[j].NonOccupancy = pDfsParam->DfsChannelList[i].NOPSaveForClear;
			j++;
		}
	}

	for (i = 0; i < j; i++) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("Local NopReportChList[%d].Channel = %d, Bw = %d, NOP = %d\n",
			i, NopReportChList[i].Channel, NopReportChList[i].Bw,
			NopReportChList[i].NonOccupancy));
	}

	msg->nop_of_channel_list_msg.NOPTotalChNum = j;
	memcpy(msg->nop_of_channel_list_msg.NopReportChList,
	NopReportChList,
	j * sizeof(NOP_REPORT_CH_LIST)
	);
}

VOID ZeroWait_DFS_set_NOP_to_Channel_List(
	IN PRTMP_ADAPTER pAd, IN UCHAR Channel, UCHAR Bw, USHORT NOPTime)
{
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Channel: %d, Bw: %d, NOP: %d\n",
	__FUNCTION__, Channel, Bw, NOPTime));

	if (Bw > BW_80) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Not a valid BW for ZeroWait\n",
		__func__));
		return;
	}
	if (!RadarChannelCheck(pAd, Channel)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Ch %d is not a DFS channel. InValid\n",
		__func__, Channel));
		return;
	}
	if (ByPassChannelByBw(Channel, Bw, pDfsParam)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Ch%d doesn't support BW %d\n",
		__FUNCTION__, Channel, Bw));
		return;
	}
	for (i = 0; i < pDfsParam->ChannelListNum; i++) {
		if (Channel == pDfsParam->DfsChannelList[i].Channel) {
			pDfsParam->DfsChannelList[i].NOPSetByBw = Bw;
			pDfsParam->DfsChannelList[i].NOPClrCnt++;
			if (Bw == BW_80) {
				pDfsParam->DfsChannelList[i].NOPSaveForClear = NOPTime;
			} else if (Bw == BW_40) {
				pDfsParam->DfsChannelList[i].NOPSaveForClear = NOPTime;
			} else if (Bw == BW_20) {
				pDfsParam->DfsChannelList[i].NonOccupancy = NOPTime;
			}
		}
	}
}

VOID ZeroWait_DFS_Pre_Assign_Next_Target_Channel(
	IN PRTMP_ADAPTER pAd, IN UCHAR Channel, IN UCHAR Bw, IN USHORT CacValue)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	pDfsParam->targetCh = Channel;
	pDfsParam->targetBw = Bw;
	pDfsParam->targetCacValue = CacValue;
}

VOID ZeroWait_DFS_Next_Target_Show(
	IN PRTMP_ADAPTER pAd, IN UCHAR mode)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if (mode != 0)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m[%s] Target Channel: %d, Target Bw: %d, Target CAC value:%d \x1b[m \n",
		__FUNCTION__, pDfsParam->targetCh, pDfsParam->targetBw, pDfsParam->targetCacValue));

}

VOID ZeroWait_DFS_collision_report(
	IN PRTMP_ADAPTER pAd, IN UCHAR SynNum, IN UCHAR Channel, UCHAR Bw)
{
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m[%s] SynNum: %d, Channel: %d, Bw:%d \x1b[m \n", __FUNCTION__, SynNum, Channel, Bw));

	if (radar_detected_callback_func) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m[%s] Call back func \x1b[m \n", __FUNCTION__));
		radar_detected_callback_func(SynNum,
					Channel,
					Bw
					);
	}

}

VOID DfsZeroHandOffRecovery(IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	if (pDot11h) {
		if (pDot11h->RDMode == RD_SILENCE_MODE) {
			mtRddControl(pAd, RDD_RESUME_BF, HW_RDD0, 0, 0);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Resume BF.\n", __func__));
		}
	}
}

VOID DfsSetNewChInit(IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_INIT;
}

#ifdef RDM_FALSE_ALARM_DEBUG_SUPPORT
INT Show_DFS_Debug_Proc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *arg)
{
	INT16 value, Index, Index2;
	value = (INT16)simple_strtol(arg, 0, 10);

	if (value == 1) {
		memset(&rRadarInfo[0], 0, sizeof(rRadarInfo[0]));
		memset(&rRadarInfo[1], 0, sizeof(rRadarInfo[1]));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Info clear\n"));
	} else if (value == 0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Debug info Start\n"));

		for (Index2 = HW_RDD0; Index2 <= HW_RDD1; Index2++) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BAND%d INFO\n", Index2));

			for (Index = 0; Index < rRadarInfo[Index2].ucLongPulseNum; Index++) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d\t", Index));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%u\t",
					rRadarInfo[Index2].arLongPulse[Index].u4LongStartTime));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%u\t",
					rRadarInfo[Index2].arLongPulse[Index].u2LongPulseWidth));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d\n",
					rRadarInfo[Index2].arLongPulse[Index].i2LongPulsePower));
			}

			for (Index = 0; Index < rRadarInfo[Index2].ucPeriodicPulseNum; Index++) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d\t", Index));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%u\t",
					rRadarInfo[Index2].arPeriodicPulse[Index].u4PeriodicStartTime));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%u\t",
					rRadarInfo[Index2].arPeriodicPulse[Index].u2PeriodicPulseWidth));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d\n",
					rRadarInfo[Index2].arPeriodicPulse[Index].i2PeriodicPulsePower));
			}
		}
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Debug info End\n"));
	}
	return TRUE;
}

VOID UpdateRadarInfo(
	P_EXT_EVENT_RDD_REPORT_T prRadarReport)
{
	if (prRadarReport == NULL)
		return;

	if (prRadarReport->ucRddIdx == HW_RDD0)
		rRadarInfo[HW_RDD0] = *prRadarReport;
	else if (prRadarReport->ucRddIdx == HW_RDD1)
		rRadarInfo[HW_RDD1] = *prRadarReport;
}

VOID DumpRadarSwPulsesInfo(
	PRTMP_ADAPTER pAd,
	P_EXT_EVENT_RDD_REPORT_T prRadarReport)
{
	UINT8 ucIndex = 0, ucRadarTypeIdx = 0;
	UINT32 u4PRI = 0;
	BOOLEAN fgPeriodicRadarDetected = FALSE;
	PSW_RADAR_TYPE_T prRadarType = NULL;
	BOOLEAN fgSwRDDLogCond = pAd->CommonCfg.DfsParameter.fgSwRDDLogCond;
	PDFS_RADAR_THRESHOLD_PARAM prRadarThresholdParam =  &pAd->CommonCfg.DfsParameter.rRadarThresholdParam;

	if (prRadarReport == NULL)
		return;
	if ((prRadarReport->ucConstantPRFDetected == 1) || (prRadarReport->ucStaggeredPRFDetected == 1))
		fgPeriodicRadarDetected = TRUE;

	if ((prRadarReport->ucLongDetected == 1) || (fgPeriodicRadarDetected == TRUE) || (fgSwRDDLogCond == FALSE)) {

		if (prRadarReport->ucLongDetected == 1) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("===> RDD-%d: Long pulse radar is detected\n", prRadarReport->ucRddIdx));
		} else {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("===> RDD-%d: No Long pulse radar is detected\n", prRadarReport->ucRddIdx));
		}

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
			("LPN = %d (FCC5_LPN = %d)\n", prRadarReport->ucOutLPN, pAd->CommonCfg.DfsParameter.u2FCC_LPN_MIN));

		if (prRadarReport->ucLongPulseNum) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("\n----------------------Long pulse buffer----------------------\n"));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("Index\t| ST(us)\t | PW(us)\t | Power(dBm)\t | PRI(us)\n"));

			for (ucIndex = 0; ucIndex < prRadarReport->ucLongPulseNum; ucIndex++) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
					("%u\t%u\t\t", ucIndex, (UINT32)(prRadarReport->arLongPulse[ucIndex].u4LongStartTime * 4/10)));
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
					("%u\t\t%d\t\t", (UINT16)(prRadarReport->arLongPulse[ucIndex].u2LongPulseWidth * 4/10),
					(INT16)((prRadarReport->arLongPulse[ucIndex].i2LongPulsePower - 1024)/4)));
				if (ucIndex == 0)
					u4PRI = 0;
				else
					u4PRI = (UINT32)((prRadarReport->arLongPulse[ucIndex].u4LongStartTime -
						prRadarReport->arLongPulse[ucIndex - 1].u4LongStartTime) * 4/10);
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("%d\n", u4PRI));
			}
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("-----------------------------------------------------------\n"));
		}

		if (fgPeriodicRadarDetected == TRUE) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("===> RDD-%d: Periodic radar (RT-%d, RT_STGR = %d) is detected\n",
					prRadarReport->ucRddIdx, prRadarReport->ucRadarTypeIdx, prRadarReport->ucStaggeredPRFDetected));
			ucRadarTypeIdx = prRadarReport->ucRadarTypeIdx;
			prRadarType = &prRadarThresholdParam->arRadarType[ucRadarTypeIdx];
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
						("SPN = %d\n", prRadarReport->ucOutSPN));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRPN = %d \t(RT_CRPN_MIN = %d, RT_CRPN_MAX = %d)\n",
					prRadarReport->ucOutCRPN, prRadarType->ucRT_CRPN_MIN, prRadarType->ucRT_CRPN_MAX));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRPR = %d/%d \t(RT_CRPR_MIN = %d/%d)\n",
				prRadarReport->ucOutCRPN, prRadarReport->ucPeriodicPulseNum, prRadarType->ucRT_CRPR_MIN, PPB_SIZE));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRPW = %d \t(RT_PW_MIN = %d, RT_PW_MAX = %d)\n",
					prRadarReport->ucOutCRPW, prRadarType->ucRT_PW_MIN, prRadarType->ucRT_PW_MAX));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_CONST = %d \t(RT_PRI_MIN = %d, RT_PRI_MAX = %d)\n",
					prRadarReport->u4OutPRI_CONST, prRadarType->u4RT_PRI_MIN, prRadarType->u4RT_PRI_MAX));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRBN = %d \t(RT_CRBN_MIN = %d, RT_CRBN_MAX = %d)\n",
					prRadarReport->ucOutCRBN, prRadarType->ucRT_CRBN_MIN, prRadarType->ucRT_CRBN_MAX));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_STG1 = %d \t(RT_PRI_MIN = %d, RT_PRI_MAX*3 = %d)\n",
					prRadarReport->u4OutPRI_STG1, prRadarType->u4RT_PRI_MIN, prRadarType->u4RT_PRI_MAX * 3));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_STG2 = %d \t(RT_PRI_MIN = %d, RT_PRI_MAX*3 = %d)\n",
					prRadarReport->u4OutPRI_STG2, prRadarType->u4RT_PRI_MIN, prRadarType->u4RT_PRI_MAX * 3));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_STG3 = %d\n", prRadarReport->u4OutPRI_STG3));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("STGPW = %d \t(RT_PW_MIN = %d, RT_PW_MAX = %d)\n",
					prRadarReport->ucOutSTGPW, prRadarType->ucRT_PW_MIN, prRadarType->ucRT_PW_MAX));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("STGPN = %d \t(RT_STGPN_MIN = %d, RT_STGPN_MAX = %d)\n",
					prRadarReport->ucOutSTGPN, prRadarType->ucRT_STGPN_MIN, prRadarType->ucRT_STGPN_MAX));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("STGPR = %d/%d \t(RT_STGPR_MIN = %d/%d)\n",
				prRadarReport->ucOutSTGPN, prRadarReport->ucPeriodicPulseNum, prRadarType->ucRT_STGPR_MIN, PPB_SIZE));
		} else {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("===> RDD-%d: No periodic radar is detected\n", prRadarReport->ucRddIdx));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
						("SPN = %d\n", prRadarReport->ucOutSPN));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRPN = %d\n", prRadarReport->ucOutCRPN));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRPR = %d/%d\n",
				prRadarReport->ucOutCRPN, prRadarReport->ucPeriodicPulseNum));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRPW = %d\n",
					prRadarReport->ucOutCRPW));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_CONST = %d\n",
					prRadarReport->u4OutPRI_CONST));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("CRBN = %d \n",
					prRadarReport->ucOutCRBN));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_STG1 = %d \n",
					prRadarReport->u4OutPRI_STG1));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_STG2 = %d \n",
					prRadarReport->u4OutPRI_STG2));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("PRI_STG3 = %d\n", prRadarReport->u4OutPRI_STG3));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("STGPW = %d \n",
					prRadarReport->ucOutSTGPW));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("STGPN = %d \n",
					prRadarReport->ucOutSTGPN));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("STGPR = %d/%d\n",
				prRadarReport->ucOutSTGPN, prRadarReport->ucPeriodicPulseNum));
		}

		if (prRadarReport->ucPeriodicPulseNum) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("\n----------------------Short pulse buffer----------------------\n"));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("Index\t| ST(us)\t | PW(us)\t | Power(dBm)\t | PRI(us)\n"));

			for (ucIndex = 0; ucIndex < prRadarReport->ucPeriodicPulseNum; ucIndex++) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("%u\t%u\t\t", ucIndex, (UINT32)(prRadarReport->arPeriodicPulse[ucIndex].u4PeriodicStartTime * 4/10)));
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("%u\t\t%d\t\t", (UINT16)(prRadarReport->arPeriodicPulse[ucIndex].u2PeriodicPulseWidth * 4/10),
				(INT16)(prRadarReport->arPeriodicPulse[ucIndex].i2PeriodicPulsePower - 1024)/4));
				if (ucIndex == 0)
					u4PRI = 0;
				else
					u4PRI = (UINT32)((prRadarReport->arPeriodicPulse[ucIndex].u4PeriodicStartTime -
									prRadarReport->arPeriodicPulse[ucIndex - 1].u4PeriodicStartTime) * 4/10);
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("%d\n", u4PRI));
			}
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("--------------------------------------------------------------\n"));
		}
	}

	return;
}

VOID DumpRadarHwPulsesInfo(
	PRTMP_ADAPTER pAd,
	P_EXT_EVENT_RDD_REPORT_T prRadarReport)
{
	UINT8 ucIndex = 0;

	if (prRadarReport == NULL)
		return;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
		("\n--------------------------------------------------------------\n"));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("===> RDD-%d: Interrupt\n", prRadarReport->ucRddIdx));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
		("\n------------------------HW pulse buffer-----------------------\n"));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
		("Index\t | ST(us)\t | PW(us)\t | Power(dBm)\t | \tSC\t | \tReset\n"));

	for (ucIndex = 0; ucIndex < prRadarReport->ucHwPulseNum; ucIndex++) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
			("%d\t\t%u\t\t", ucIndex, (UINT32)(prRadarReport->arContent[ucIndex].u4HwStartTime * 4/10)));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
			("%u\t\t%d\t\t", (UINT16)(prRadarReport->arContent[ucIndex].u2HwPulseWidth * 4/10),
			(INT16)(prRadarReport->arContent[ucIndex].i2HwPulsePower - 1024)/4));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
			("%s\t\t%s\n", prRadarReport->arContent[ucIndex].ucScPass ? "true":"false",
			prRadarReport->arContent[ucIndex].ucSwReset ? "true":"false"));
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
		("--------------------------------------------------------------\n"));

	return;
}

#endif /* RDM_FALSE_ALARM_DEBUG_SUPPORT */

#endif /*MT_DFS_SUPPORT*/

