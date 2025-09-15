/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 76306 $
 * $Date: 2017-03-08 15:13:58 +0800 (¶g¤T, 08 ¤T¤ë 2017) $
 *
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : MIB related functions
 *
 */

#ifndef _RTL8367C_ASICDRV_MIB_H_
#define _RTL8367C_ASICDRV_MIB_H_

#include <rtl8367c_asicdrv.h>

#define RTL8367C_MIB_PORT_OFFSET                (0x7C)
#define RTL8367C_MIB_LEARNENTRYDISCARD_OFFSET   (0x420)

#define RTL8367C_MAX_LOG_CNT_NUM                (32)
#define RTL8367C_MIB_MAX_LOG_CNT_IDX            (RTL8367C_MAX_LOG_CNT_NUM - 1)
#define RTL8367C_MIB_LOG_CNT_OFFSET             (0x3E0)
#define RTL8367C_MIB_MAX_LOG_MODE_IDX           (16-1)

typedef enum RTL8367C_MIBCOUNTER_E{

    /* RX */
    ifInOctets = 0,

    dot3StatsFCSErrors,
    dot3StatsSymbolErrors,
    dot3InPauseFrames,
    dot3ControlInUnknownOpcodes,

    etherStatsFragments,
    etherStatsJabbers,
    ifInUcastPkts,
    etherStatsDropEvents,

    ifInMulticastPkts,
    ifInBroadcastPkts,
    inMldChecksumError,
    inIgmpChecksumError,
    inMldSpecificQuery,
    inMldGeneralQuery,
    inIgmpSpecificQuery,
    inIgmpGeneralQuery,
    inMldLeaves,
    inIgmpLeaves,

    /* TX/RX */
    etherStatsOctets,

    etherStatsUnderSizePkts,
    etherOversizeStats,
    etherStatsPkts64Octets,
    etherStatsPkts65to127Octets,
    etherStatsPkts128to255Octets,
    etherStatsPkts256to511Octets,
    etherStatsPkts512to1023Octets,
    etherStatsPkts1024to1518Octets,

    /* TX */
    ifOutOctets,

    dot3StatsSingleCollisionFrames,
    dot3StatMultipleCollisionFrames,
    dot3sDeferredTransmissions,
    dot3StatsLateCollisions,
    etherStatsCollisions,
    dot3StatsExcessiveCollisions,
    dot3OutPauseFrames,
    ifOutDiscards,

    /* ALE */
    dot1dTpPortInDiscards,
    ifOutUcastPkts,
    ifOutMulticastPkts,
    ifOutBroadcastPkts,
    outOampduPkts,
    inOampduPkts,

    inIgmpJoinsSuccess,
    inIgmpJoinsFail,
    inMldJoinsSuccess,
    inMldJoinsFail,
    inReportSuppressionDrop,
    inLeaveSuppressionDrop,
    outIgmpReports,
    outIgmpLeaves,
    outIgmpGeneralQuery,
    outIgmpSpecificQuery,
    outMldReports,
    outMldLeaves,
    outMldGeneralQuery,
    outMldSpecificQuery,
    inKnownMulticastPkts,

    /*Device only */
    dot1dTpLearnedEntryDiscards,
    RTL8367C_MIBS_NUMBER,

}RTL8367C_MIBCOUNTER;


extern ret_t rtl8367c_setAsicMIBsCounterReset(rtk_uint32 greset, rtk_uint32 qmreset, rtk_uint32 pmask);
extern ret_t rtl8367c_getAsicMIBsCounter(rtk_uint32 port,RTL8367C_MIBCOUNTER mibIdx, rtk_uint64* pCounter);
extern ret_t rtl8367c_getAsicMIBsLogCounter(rtk_uint32 index, rtk_uint32 *pCounter);
extern ret_t rtl8367c_getAsicMIBsControl(rtk_uint32* pMask);

extern ret_t rtl8367c_setAsicMIBsResetValue(rtk_uint32 value);
extern ret_t rtl8367c_getAsicMIBsResetValue(rtk_uint32* value);

extern ret_t rtl8367c_setAsicMIBsUsageMode(rtk_uint32 mode);
extern ret_t rtl8367c_getAsicMIBsUsageMode(rtk_uint32* pMode);
extern ret_t rtl8367c_setAsicMIBsTimer(rtk_uint32 timer);
extern ret_t rtl8367c_getAsicMIBsTimer(rtk_uint32* pTimer);
extern ret_t rtl8367c_setAsicMIBsLoggingMode(rtk_uint32 index, rtk_uint32 mode);
extern ret_t rtl8367c_getAsicMIBsLoggingMode(rtk_uint32 index, rtk_uint32* pMode);
extern ret_t rtl8367c_setAsicMIBsLoggingType(rtk_uint32 index, rtk_uint32 type);
extern ret_t rtl8367c_getAsicMIBsLoggingType(rtk_uint32 index, rtk_uint32* pType);
extern ret_t rtl8367c_setAsicMIBsResetLoggingCounter(rtk_uint32 index);
extern ret_t rtl8367c_setAsicMIBsLength(rtk_uint32 txLengthMode, rtk_uint32 rxLengthMode);
extern ret_t rtl8367c_getAsicMIBsLength(rtk_uint32 *pTxLengthMode, rtk_uint32 *pRxLengthMode);

#endif /*#ifndef _RTL8367C_ASICDRV_MIB_H_*/

