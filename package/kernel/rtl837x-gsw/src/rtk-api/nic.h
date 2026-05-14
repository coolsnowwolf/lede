/*******************************************************************************
* Copyright (C), 2013,  Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*
* File Name:
* Author   : Cynthia_wang
* Version  :
* Date     : 2020-9-24
* Purpose  : RTL8373 switch high-level API for RTL8373
* Feature  : Here is a list of all functions and variables in NIC module
* Note:
*******************************************************************************/


#ifndef __RTK_API_NIC_H__
#define __RTK_API_NIC_H__

#define RTK_NIC_RXSTOP_MAX                          (0x7FF)
#define RTK_NIC_TXSTOP_MAX                          (0x7FF)

#define RTK_ETHER_TYPE_MAX                          (0xFFFF)
#define RTK_RXFST_WIDTH                                (0xFF)

typedef enum rtk_nic_RxMTU_e
{
    NIC_RX_MTU_1534 = 0,
    NIC_RX_MTU_2048 ,
    NIC_RX_MTU_4096 ,
    NIC_RX_MTU_END
} rtk_nic_RxMTU_t;

typedef enum rtk_nic_rxpps_e
{
    NIC_RX_PPS_DROPALL = 0,
    NIC_RX_PPS_RXMATCHED ,
    NIC_RX_PPS_RXUNMATCHED ,
    NIC_RX_PPS_RXALL,
    NIC_RX_PPS_END
} rtk_nic_rxpps_t;

typedef enum rtk_nic_hashValType_e
{
    NIC_HASH_VAL_LOW_WORD = 0,
    NIC_HASH_VAL_HIGH_WORD,
    NIC_HASH_TYPE_END
} rtk_nic_hashValType_t;


extern rtk_api_ret_t rtk_nic_rst_set(void);
extern rtk_api_ret_t rtk_nic_txstop_set(rtk_uint32 addr);
extern rtk_api_ret_t rtk_nic_txstop_get(rtk_uint32 *pAddr);
extern rtk_api_ret_t rtk_nic_rxstop_set( rtk_uint32 addr);
extern rtk_api_ret_t rtk_nic_rxstop_get(rtk_uint32 *pAddr);
extern rtk_api_ret_t rtk_nic_swRxCurPktAddr_get(rtk_uint32 *pAddr);
extern rtk_api_ret_t rtk_nic_rxReceivedPktLen_get(rtk_uint32 *pLength);
extern rtk_api_ret_t rtk_nic_txAvailSpace_get(rtk_uint32 *pLength);
extern rtk_api_ret_t rtk_nic_moduleEn_set(rtk_enable_t enable);
extern rtk_api_ret_t rtk_nic_moduleEn_get(rtk_enable_t *pStatus);
extern rtk_api_ret_t rtk_nic_rxEn_set(rtk_enable_t enable);
extern rtk_api_ret_t rtk_nic_rxEn_get(rtk_enable_t *pStatus);
extern rtk_api_ret_t rtk_nic_txEn_set(rtk_enable_t enable);
extern rtk_api_ret_t rtk_nic_txEn_get(rtk_enable_t *pStatus);
extern rtk_api_ret_t rtk_nic_rxRemoveCrc_set(rtk_enable_t enabled);
extern rtk_api_ret_t rtk_nic_rxRemoveCrc_get(rtk_enable_t *pEnabled);
extern rtk_api_ret_t rtk_nic_rxPaddingEn_set(rtk_enable_t enabled);
extern rtk_api_ret_t rtk_nic_rxPaddingEn_get(rtk_enable_t *pEnabled);
extern rtk_api_ret_t rtk_nic_rxFreeSpaceThd_set(rtk_uint32 val);
extern rtk_api_ret_t rtk_nic_rxFreeSpaceThd_get(rtk_uint32 *pVal);
extern rtk_api_ret_t rtk_nic_rxCrcErrEn_set(rtk_enable_t enabled);
extern rtk_api_ret_t rtk_nic_rxCrcErrEn_get(rtk_enable_t *pEnabled);
extern rtk_api_ret_t rtk_nic_rxL3CrcErrEn_set(rtk_enable_t enabled);
extern rtk_api_ret_t rtk_nic_rxL3CrcErrEn_get(rtk_enable_t *pEnabled);
extern rtk_api_ret_t rtk_nic_rxL4CrcErrEn_set(rtk_enable_t enabled);
extern rtk_api_ret_t rtk_nic_rxL4CrcErrEn_get(rtk_enable_t *pEnabled);
extern rtk_api_ret_t rtk_nic_rxArpEn_set(rtk_enable_t enabled);
extern rtk_api_ret_t rtk_nic_rxArpEn_get(rtk_enable_t *pEnabled);
extern rtk_api_ret_t rtk_nic_rxAllPktEn_set(rtk_enable_t enabled);
extern rtk_api_ret_t rtk_nic_rxAllPktEn_get(rtk_enable_t *pEnabled);
extern rtk_api_ret_t rtk_nic_rxPhyPktSel_set(rtk_nic_rxpps_t behavior);
extern rtk_api_ret_t rtk_nic_rxPhyPktSel_get(rtk_nic_rxpps_t *pBehavior);
extern rtk_api_ret_t rtk_nic_rxMultiPktEn_set(rtk_enable_t enabled);
extern rtk_api_ret_t rtk_nic_rxMultiPktEn_get(rtk_enable_t *pEnabled);
extern rtk_api_ret_t rtk_nic_rxBcPktEn_set(rtk_enable_t enabled);
extern rtk_api_ret_t rtk_nic_rxBcPktEn_get(rtk_enable_t *pEnabled);
extern rtk_api_ret_t rtk_nic_mcHashFltrEn_set(rtk_enable_t enabled);
extern rtk_api_ret_t rtk_nic_mcHashFltrEn_get(rtk_enable_t *pEnabled);
extern rtk_api_ret_t rtk_nic_phyPktHashFltrEn_set(rtk_enable_t enabled);
extern rtk_api_ret_t rtk_nic_phyPktHashFltrEn_get(rtk_enable_t *pEnabled);
extern rtk_api_ret_t rtk_nic_mcHashTblVal_set(rtk_nic_hashValType_t type, rtk_uint32 val);
extern rtk_api_ret_t rtk_nic_mcHashTblVal_get(rtk_nic_hashValType_t type, rtk_uint32 *pVal);
extern rtk_api_ret_t rtk_nic_phyPktHashTblVal_set(rtk_nic_hashValType_t type, rtk_uint32 val);
extern rtk_api_ret_t rtk_nic_phyPktHashTblVal_get(rtk_nic_hashValType_t type, rtk_uint32 *pVal);
extern rtk_api_ret_t rtk_nic_rxMTU_set(rtk_nic_RxMTU_t length);
extern rtk_api_ret_t rtk_nic_rxMTU_get(rtk_nic_RxMTU_t *pLength);
extern rtk_api_ret_t rtk_nic_loopbackEn_set(rtk_enable_t enabled);
extern rtk_api_ret_t rtk_nic_loopbackEn_get(rtk_enable_t *pEnabled);
extern rtk_api_ret_t rtk_nic_interruptEn_set(rtk_enable_t rxie, rtk_enable_t txee);
extern rtk_api_ret_t rtk_nic_interruptEn_get(rtk_enable_t *pRxie, rtk_enable_t *pTxee);
extern rtk_api_ret_t rtk_nic_interruptStatus_get(rtk_uint32 * pRxis, rtk_uint32 *pTxes);
extern rtk_api_ret_t rtk_nic_interruptStatus_clear(rtk_uint32 rxis, rtk_uint32 txes);


#endif
