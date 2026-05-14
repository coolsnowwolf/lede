/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : RTL8367/RTL8367C switch high-level API
 *
 * Feature : The file includes MIB module high-layer API defination
 *
 */

#ifndef __RTK_API_MIB_H__
#define __RTK_API_MIB_H__

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


typedef enum rtk_stat_port_type_e{

    /* RX */
    ifInOctets_H = 0,
    ifInOctets_L,
    ifOutOctets_H,
    ifOutOctets_L,
    ifInUcastPkts_H,
    ifInUcastPkts_L,
    ifInMulticastPkts_H,
    ifInMulticastPkts_L,
    ifInBroadcastPkts_H,
    ifInBroadcastPkts_L,
    ifOutUcastPkts_H,
    ifOutUcastPkts_L,
    ifOutMulticastPkts_H,
    ifOutMulticastPkts_L,
    ifOutBroadcastPkts_H,
    ifOutBroadcastPkts_L,
    
    ifOutDiscards,  
    dot1dTpPortInDiscards,
    dot3StatsSingleCollisionFrames,
    dot3StatMultipleCollisionFrames,
    dot3sDeferredTransmissions,
    dot3StatsLateCollisions,
    dot3StatsExcessiveCollisions,
    dot3StatsSymbolErrors,
    dot3ControlInUnknownOpcodes,
    dot3InPauseFrames,
    dot3OutPauseFrames,
    etherStatsDropEvents,
    tx_etherStatsBroadcastPkts,       
    tx_etherStatsMulticastPkts,       
    tx_etherStatsCRCAlignErrors,      
    rx_etherStatsCRCAlignErrors,      
    tx_etherStatsUndersizePkts,       
    rx_etherStatsUndersizePkts,       
    tx_etherStatsOversizePkts,        
    rx_etherStatsOversizePkts,        
    tx_etherStatsFragments,           
    rx_etherStatsFragments,           
    tx_etherStatsJabbers,             
    rx_etherStatsJabbers,             
    tx_etherStatsCollisions,          
    tx_etherStatsPkts64Octets,        
    rx_etherStatsPkts64Octets,        
    tx_etherStatsPkts65to127Octets,   
    rx_etherStatsPkts65to127Octets,   
    tx_etherStatsPkts128to255Octets,  
    rx_etherStatsPkts128to255Octets,  
    tx_etherStatsPkts256to511Octets,  
    rx_etherStatsPkts256to511Octets,  
    tx_etherStatsPkts512to1023Octets, 
    rx_etherStatsPkts512to1023Octets, 
    tx_etherStatsPkts1024to1518Octets,
    rx_etherStatsPkts1024to1518Octets,

    rx_etherStatsUndersizedropPkts = 54,        
    tx_etherStatsPkts1519toMaxOctets,      
    rx_etherStatsPkts1519toMaxOctets,      
    tx_etherStatsPktsOverMaxOctets,        
    rx_etherStatsPktsOverMaxOctets,        
    tx_etherStatsPktsFlexibleOctetsSET1,   
    rx_etherStatsPktsFlexibleOctetsSET1,   
    tx_etherStatsPktsFlexibleOctetsCRCSET1,
    rx_etherStatsPktsFlexibleOctetsCRCSET1,
    tx_etherStatsPktsFlexibleOctetsSET0,   
    rx_etherStatsPktsFlexibleOctetsSET0,   
    tx_etherStatsPktsFlexibleOctetsCRSET0C,
    rx_etherStatsPktsFlexibleOctetsCRSET0C,
    lengthFieldError,                      
    falseCarrieimes,                       
    underSizeOctets,                       
    framingErrors,                         

    rxMacDiscards = 72,      
    rxMacIPGShortDropRT,

    dot1dTpLearnedEntryDiscards = 75,
    egrQueue7DropPktRT,         
    egrQueue6DropPktRT,         
    egrQueue5DropPktRT,         
    egrQueue4DropPktRT,         
    egrQueue3DropPktRT,         
    egrQueue2DropPktRT,         
    egrQueue1DropPktRT,         
    egrQueue0DropPktRT,         
    egrQueue7OutPktRT,          
    egrQueue6OutPktRT,          
    egrQueue5OutPktRT,          
    egrQueue4OutPktRT,          
    egrQueue3OutPktRT,          
    egrQueue2OutPktRT,          
    egrQueue1OutPktRT,          
    egrQueue0OutPktRT,          
    TxGoodCnt_H,   
    TxGoodCnt_L, 
    RxGoodCnt_H,  
    RxGoodCnt_L,
    RxErrorCnt,                 
    TxErrorCnt,  
    TxGoodCnt_phy_H,   
    TxGoodCnt_phy_L, 
    RxGoodCnt_phy_H,  
    RxGoodCnt_phy_L,
    RxErrorCnt_phy,                 
    TxErrorCnt_phy, 

}rtk_stat_port_type_t;


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

#endif /* __RTK_API_MIB_H__ */

