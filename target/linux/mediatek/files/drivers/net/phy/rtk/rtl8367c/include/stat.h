/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * Purpose : RTL8367/RTL8367C switch high-level API
 *
 * Feature : The file includes MIB module high-layer API defination
 *
 */

#ifndef __RTK_API_STAT_H__
#define __RTK_API_STAT_H__

/*
 * Data Type Declaration
 */
typedef rtk_u_long_t rtk_stat_counter_t;

/* global statistic counter structure */
typedef struct rtk_stat_global_cntr_s
{
    rtk_uint64 dot1dTpLearnedEntryDiscards;
}rtk_stat_global_cntr_t;

typedef enum rtk_stat_global_type_e
{
    DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX = 58,
    MIB_GLOBAL_CNTR_END
}rtk_stat_global_type_t;

/* port statistic counter structure */
typedef struct rtk_stat_port_cntr_s
{
    rtk_uint64 ifInOctets;
    rtk_uint32 dot3StatsFCSErrors;
    rtk_uint32 dot3StatsSymbolErrors;
    rtk_uint32 dot3InPauseFrames;
    rtk_uint32 dot3ControlInUnknownOpcodes;
    rtk_uint32 etherStatsFragments;
    rtk_uint32 etherStatsJabbers;
    rtk_uint32 ifInUcastPkts;
    rtk_uint32 etherStatsDropEvents;
    rtk_uint64 etherStatsOctets;
    rtk_uint32 etherStatsUndersizePkts;
    rtk_uint32 etherStatsOversizePkts;
    rtk_uint32 etherStatsPkts64Octets;
    rtk_uint32 etherStatsPkts65to127Octets;
    rtk_uint32 etherStatsPkts128to255Octets;
    rtk_uint32 etherStatsPkts256to511Octets;
    rtk_uint32 etherStatsPkts512to1023Octets;
    rtk_uint32 etherStatsPkts1024toMaxOctets;
    rtk_uint32 etherStatsMcastPkts;
    rtk_uint32 etherStatsBcastPkts;
    rtk_uint64 ifOutOctets;
    rtk_uint32 dot3StatsSingleCollisionFrames;
    rtk_uint32 dot3StatsMultipleCollisionFrames;
    rtk_uint32 dot3StatsDeferredTransmissions;
    rtk_uint32 dot3StatsLateCollisions;
    rtk_uint32 etherStatsCollisions;
    rtk_uint32 dot3StatsExcessiveCollisions;
    rtk_uint32 dot3OutPauseFrames;
    rtk_uint32 dot1dBasePortDelayExceededDiscards;
    rtk_uint32 dot1dTpPortInDiscards;
    rtk_uint32 ifOutUcastPkts;
    rtk_uint32 ifOutMulticastPkts;
    rtk_uint32 ifOutBrocastPkts;
    rtk_uint32 outOampduPkts;
    rtk_uint32 inOampduPkts;
    rtk_uint32 pktgenPkts;
    rtk_uint32 inMldChecksumError;
    rtk_uint32 inIgmpChecksumError;
    rtk_uint32 inMldSpecificQuery;
    rtk_uint32 inMldGeneralQuery;
    rtk_uint32 inIgmpSpecificQuery;
    rtk_uint32 inIgmpGeneralQuery;
    rtk_uint32 inMldLeaves;
    rtk_uint32 inIgmpLeaves;
    rtk_uint32 inIgmpJoinsSuccess;
    rtk_uint32 inIgmpJoinsFail;
    rtk_uint32 inMldJoinsSuccess;
    rtk_uint32 inMldJoinsFail;
    rtk_uint32 inReportSuppressionDrop;
    rtk_uint32 inLeaveSuppressionDrop;
    rtk_uint32 outIgmpReports;
    rtk_uint32 outIgmpLeaves;
    rtk_uint32 outIgmpGeneralQuery;
    rtk_uint32 outIgmpSpecificQuery;
    rtk_uint32 outMldReports;
    rtk_uint32 outMldLeaves;
    rtk_uint32 outMldGeneralQuery;
    rtk_uint32 outMldSpecificQuery;
    rtk_uint32 inKnownMulticastPkts;
    rtk_uint32 ifInMulticastPkts;
    rtk_uint32 ifInBroadcastPkts;
    rtk_uint32 ifOutDiscards;
}rtk_stat_port_cntr_t;

/* port statistic counter index */
typedef enum rtk_stat_port_type_e
{
    STAT_IfInOctets = 0,
    STAT_Dot3StatsFCSErrors,
    STAT_Dot3StatsSymbolErrors,
    STAT_Dot3InPauseFrames,
    STAT_Dot3ControlInUnknownOpcodes,
    STAT_EtherStatsFragments,
    STAT_EtherStatsJabbers,
    STAT_IfInUcastPkts,
    STAT_EtherStatsDropEvents,
    STAT_EtherStatsOctets,
    STAT_EtherStatsUnderSizePkts,
    STAT_EtherOversizeStats,
    STAT_EtherStatsPkts64Octets,
    STAT_EtherStatsPkts65to127Octets,
    STAT_EtherStatsPkts128to255Octets,
    STAT_EtherStatsPkts256to511Octets,
    STAT_EtherStatsPkts512to1023Octets,
    STAT_EtherStatsPkts1024to1518Octets,
    STAT_EtherStatsMulticastPkts,
    STAT_EtherStatsBroadcastPkts,
    STAT_IfOutOctets,
    STAT_Dot3StatsSingleCollisionFrames,
    STAT_Dot3StatsMultipleCollisionFrames,
    STAT_Dot3StatsDeferredTransmissions,
    STAT_Dot3StatsLateCollisions,
    STAT_EtherStatsCollisions,
    STAT_Dot3StatsExcessiveCollisions,
    STAT_Dot3OutPauseFrames,
    STAT_Dot1dBasePortDelayExceededDiscards,
    STAT_Dot1dTpPortInDiscards,
    STAT_IfOutUcastPkts,
    STAT_IfOutMulticastPkts,
    STAT_IfOutBroadcastPkts,
    STAT_OutOampduPkts,
    STAT_InOampduPkts,
    STAT_PktgenPkts,
    STAT_InMldChecksumError,
    STAT_InIgmpChecksumError,
    STAT_InMldSpecificQuery,
    STAT_InMldGeneralQuery,
    STAT_InIgmpSpecificQuery,
    STAT_InIgmpGeneralQuery,
    STAT_InMldLeaves,
    STAT_InIgmpInterfaceLeaves,
    STAT_InIgmpJoinsSuccess,
    STAT_InIgmpJoinsFail,
    STAT_InMldJoinsSuccess,
    STAT_InMldJoinsFail,
    STAT_InReportSuppressionDrop,
    STAT_InLeaveSuppressionDrop,
    STAT_OutIgmpReports,
    STAT_OutIgmpLeaves,
    STAT_OutIgmpGeneralQuery,
    STAT_OutIgmpSpecificQuery,
    STAT_OutMldReports,
    STAT_OutMldLeaves,
    STAT_OutMldGeneralQuery,
    STAT_OutMldSpecificQuery,
    STAT_InKnownMulticastPkts,
    STAT_IfInMulticastPkts,
    STAT_IfInBroadcastPkts,
    STAT_IfOutDiscards,
    STAT_PORT_CNTR_END
}rtk_stat_port_type_t;

typedef enum rtk_logging_counter_mode_e
{
    LOGGING_MODE_32BIT = 0,
    LOGGING_MODE_64BIT,
    LOGGING_MODE_END
}rtk_logging_counter_mode_t;

typedef enum rtk_logging_counter_type_e
{
    LOGGING_TYPE_PACKET = 0,
    LOGGING_TYPE_BYTE,
    LOGGING_TYPE_END
}rtk_logging_counter_type_t;

typedef enum rtk_stat_lengthMode_e
{
    LENGTH_MODE_EXC_TAG = 0,
    LENGTH_MODE_INC_TAG,
    LENGTH_MODE_END
}rtk_stat_lengthMode_t;



/* Function Name:
 *      rtk_stat_global_reset
 * Description:
 *      Reset global MIB counter.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Reset MIB counter of ports. API will use global reset while port mask is all-ports.
 */
extern rtk_api_ret_t rtk_stat_global_reset(void);

/* Function Name:
 *      rtk_stat_port_reset
 * Description:
 *      Reset per port MIB counter by port.
 * Input:
 *      port - port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *
 */
extern rtk_api_ret_t rtk_stat_port_reset(rtk_port_t port);

/* Function Name:
 *      rtk_stat_queueManage_reset
 * Description:
 *      Reset queue manage MIB counter.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *
 */
extern rtk_api_ret_t rtk_stat_queueManage_reset(void);

/* Function Name:
 *      rtk_stat_global_get
 * Description:
 *      Get global MIB counter
 * Input:
 *      cntr_idx - global counter index.
 * Output:
 *      pCntr - global counter value.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      Get global MIB counter by index definition.
 */
extern rtk_api_ret_t rtk_stat_global_get(rtk_stat_global_type_t cntr_idx, rtk_stat_counter_t *pCntr);

/* Function Name:
 *      rtk_stat_global_getAll
 * Description:
 *      Get all global MIB counter
 * Input:
 *      None
 * Output:
 *      pGlobal_cntrs - global counter structure.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      Get all global MIB counter by index definition.
 */
extern rtk_api_ret_t rtk_stat_global_getAll(rtk_stat_global_cntr_t *pGlobal_cntrs);

/* Function Name:
 *      rtk_stat_port_get
 * Description:
 *      Get per port MIB counter by index
 * Input:
 *      port        - port id.
 *      cntr_idx    - port counter index.
 * Output:
 *      pCntr - MIB retrived counter.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Get per port MIB counter by index definition.
 */
extern rtk_api_ret_t rtk_stat_port_get(rtk_port_t port, rtk_stat_port_type_t cntr_idx, rtk_stat_counter_t *pCntr);

/* Function Name:
 *      rtk_stat_port_getAll
 * Description:
 *      Get all counters of one specified port in the specified device.
 * Input:
 *      port - port id.
 * Output:
 *      pPort_cntrs - buffer pointer of counter value.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      Get all MIB counters of one port.
 */
extern rtk_api_ret_t rtk_stat_port_getAll(rtk_port_t port, rtk_stat_port_cntr_t *pPort_cntrs);

/* Function Name:
 *      rtk_stat_logging_counterCfg_set
 * Description:
 *      Set the type and mode of Logging Counter
 * Input:
 *      idx     - The index of Logging Counter. Should be even number only.(0,2,4,6,8.....30)
 *      mode    - 32 bits or 64 bits mode
 *      type    - Packet counter or byte counter
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_OUT_OF_RANGE - Out of range.
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      Set the type and mode of Logging Counter.
 */
extern rtk_api_ret_t rtk_stat_logging_counterCfg_set(rtk_uint32 idx, rtk_logging_counter_mode_t mode, rtk_logging_counter_type_t type);

/* Function Name:
 *      rtk_stat_logging_counterCfg_get
 * Description:
 *      Get the type and mode of Logging Counter
 * Input:
 *      idx     - The index of Logging Counter. Should be even number only.(0,2,4,6,8.....30)
 * Output:
 *      pMode   - 32 bits or 64 bits mode
 *      pType   - Packet counter or byte counter
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_OUT_OF_RANGE - Out of range.
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - NULL Pointer
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      Get the type and mode of Logging Counter.
 */
extern rtk_api_ret_t rtk_stat_logging_counterCfg_get(rtk_uint32 idx, rtk_logging_counter_mode_t *pMode, rtk_logging_counter_type_t *pType);

/* Function Name:
 *      rtk_stat_logging_counter_reset
 * Description:
 *      Reset Logging Counter
 * Input:
 *      idx     - The index of Logging Counter. (0~31)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_OUT_OF_RANGE - Out of range.
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Reset Logging Counter.
 */
extern rtk_api_ret_t rtk_stat_logging_counter_reset(rtk_uint32 idx);

/* Function Name:
 *      rtk_stat_logging_counter_get
 * Description:
 *      Get Logging Counter
 * Input:
 *      idx     - The index of Logging Counter. (0~31)
 * Output:
 *      pCnt    - Logging counter value
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_OUT_OF_RANGE - Out of range.
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Get Logging Counter.
 */
extern rtk_api_ret_t rtk_stat_logging_counter_get(rtk_uint32 idx, rtk_uint32 *pCnt);

/* Function Name:
 *      rtk_stat_lengthMode_set
 * Description:
 *      Set Legnth mode.
 * Input:
 *      txMode     - The length counting mode
 *      rxMode     - The length counting mode
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_INPUT        - Out of range.
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *
 */
extern rtk_api_ret_t rtk_stat_lengthMode_set(rtk_stat_lengthMode_t txMode, rtk_stat_lengthMode_t rxMode);

/* Function Name:
 *      rtk_stat_lengthMode_get
 * Description:
 *      Get Legnth mode.
 * Input:
 *      None.
 * Output:
 *      pTxMode       - The length counting mode
 *      pRxMode       - The length counting mode
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_INPUT        - Out of range.
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 */
extern rtk_api_ret_t rtk_stat_lengthMode_get(rtk_stat_lengthMode_t *pTxMode, rtk_stat_lengthMode_t *pRxMode);

#endif /* __RTK_API_STAT_H__ */

