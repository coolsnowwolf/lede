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
* Purpose  : RTL8373 switch low-level API for RTL8373
* Feature  : Here is a list of all functions and variables in NIC module
* Note:
*******************************************************************************/

#include <rtk_types.h>
#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8373/dal_rtl8373_nic.h>
#include <dal/rtl8373/rtl8373_asicdrv.h>

/* Function Name:
 *      dal_rtl8373_nic_rst_set
 * Description:
 *      NIC Reset, Write 1 will reset NIC, all the NIC registers and parameters will set to default. After the reset is finished, this bit will turn to 0.
 * Input:
 *      enabled                          -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rst_set(void)
{
    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_RST_GLB_CTRL_0_ADDR, RTL8373_RST_GLB_CTRL_0_SW_NIC_RST_OFFSET, ENABLED));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_txStopAddr_set
 * Description:
 *      Set nic buffer total size (txstop boundary)
 * Input:
 *      addr                          - address
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_txStopAddr_set(rtk_uint32 addr)
{

    if (addr > RTK_NIC_TXSTOP_MAX )
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBits(RTL8373_NIC_BUFFSIZE_CTRL_ADDR, RTL8373_NIC_BUFFSIZE_CTRL_TXSTOP_ADDR_MASK, addr));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_txStopAddr_get
 * Description:
 *      Get nic buffer total size (txstop boundary)
 * Input:
 *      none
 * Output:
 *      addr                          - address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 * Note:
 *
 */
ret_t dal_rtl8373_nic_txStopAddr_get(rtk_uint32 *pAddr)
{

    if (NULL ==  pAddr)
            return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBits(RTL8373_NIC_BUFFSIZE_CTRL_ADDR, RTL8373_NIC_BUFFSIZE_CTRL_TXSTOP_ADDR_MASK, pAddr));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxStopAddr_set
 * Description:
 *      Set nic rx buffer size
 * Input:
 *      addr                          - address
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxStopAddr_set(rtk_uint32 addr)
{

    if (addr > RTK_NIC_TXSTOP_MAX )
        return RT_ERR_INPUT;

    RTK_ERR_CHK(rtl8373_setAsicRegBits(RTL8373_NIC_RXBUFF_CTRL_ADDR, RTL8373_NIC_RXBUFF_CTRL_RXSTOP_ADDR_MASK,  addr));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxStopAddr_get
 * Description:
 *      Set nic rx buffer size
 * Input:
 *      none
 * Output:
 *      addr                          - address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxStopAddr_get(rtk_uint32 *pAddr)
{

    if (NULL ==  pAddr)
            return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBits(RTL8373_NIC_RXBUFF_CTRL_ADDR, RTL8373_NIC_RXBUFF_CTRL_RXSTOP_ADDR_MASK, pAddr));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_swRxCurPktAddr_get
 * Description:
 *      Get nic rx buffer received pkt length
 * Input:
 *      none
 * Output:
 *      addr                          - address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 * Note:
 *
 */
ret_t dal_rtl8373_nic_swRxCurPktAddr_get(rtk_uint32 *pAddr)
{

    if (NULL ==  pAddr)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicReg(RTL8373_NIC_RX_CURR_PKT_ADDR,  pAddr));
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxReceivedPktLen_get
 * Description:
 *      Get nic rx buffer received pkt length
 * Input:
 *      none
 * Output:
 *      pLength                          -
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxReceivedPktLen_get(rtk_uint32 *pLength)
{

    if (NULL ==  pLength)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicReg(RTL8373_NIC_RX_BUFF_DATA_ADDR,  pLength));
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_txAvailableSpace_get
 * Description:
 *      Get nic tx available free space.
 * Input:
 *      none
 * Output:
 *      addr                          - address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 * Note:
 *
 */
ret_t dal_rtl8373_nic_txAvailSpace_get(rtk_uint32 *pLength)
{

    if (NULL ==  pLength)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicReg(RTL8373_NIC_TX_BUFF_AVAIL_ADDR, pLength));
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_moduleEn_set
 * Description:
 *      enable/disable nic mocule.
 * Input:
 *      enabled  - enable or  disable 
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_moduleEn_set(rtk_enable_t enabled)
{
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;
    
    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_DW8051_CFG_ADDR, RTL8373_DW8051_CFG_NIC_EN_OFFSET, (enabled ? 1: 0) ));
    return RT_ERR_OK;
    
}

/* Function Name:
 *      dal_rtl8373_nic_moduleEn_get
 * Description:
 *      Get nic mocule status.
 * Input:
 *      none 
 * Output:
 *      pEnabled - enable/disable
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_moduleEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;

    if (NULL ==  pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_DW8051_CFG_ADDR, RTL8373_DW8051_CFG_NIC_EN_OFFSET, &regData));
    
    *pEnabled = (rtk_enable_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxEn_set
 * Description:
 *      enable nic rx or not.
 * Input:
 *      enabled  - enable or  disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxEn_set(rtk_enable_t enabled)
{
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RX_EN_OFFSET, (enabled ? 1: 0) ));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxEn_get
 * Description:
 *      get nic rx status.
 * Input:
 *      none                       
 * Output:
 *      pEnabled  - enable or  disable  
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;

    if (NULL ==  pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RX_EN_OFFSET, &regData));
    
    *pEnabled = (rtk_enable_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxRemoveCrc_set
 * Description:
 *      enable rx remove crc or not.
 * Input:
 *      enabled  - enable or  disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxRemoveCrc_set(rtk_enable_t enabled)
{        
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RMCRC_EN_OFFSET, (enabled ? 1 : 0)));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxRemoveCrc_get
 * Description:
 *      get nic rx remove crc enable or not.
 * Input:
 *      none                         
 * Output:
 *      pEnabled - enable or  disable 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxRemoveCrc_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;

    if (NULL ==  pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RMCRC_EN_OFFSET, &regData));
    *pEnabled = (rtk_enable_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxPaddingEn_set
 * Description:
 *      enable nic rx padding.
 * Input:
 *      enabled  - enable or disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxPaddingEn_set(rtk_enable_t enabled)
{      
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RXPAD_OFFSET, (enabled ? 1 : 0)));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxPaddingEn_get
 * Description:
 *      get nic rx padding status.
 * Input:
 *      none                          -
 * Output:
 *      *pEnabled - enable or disable
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxPaddingEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;

    if (NULL ==  pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RXPAD_OFFSET, &regData));
    *pEnabled = (rtk_enable_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxFreeSpaceThd_set
 * Description:
 *      set nic rx buffer free space threshlod
 * Input:
 *      val  -  free space threshlod value (uint:8Byte)                       -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxFreeSpaceThd_set(rtk_uint32 val)
{
    if(val > RTK_RXFST_WIDTH)
        return RT_ERR_RANGE;

    RTK_ERR_CHK(rtl8373_setAsicRegBits(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RXFST_MASK, val));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxFreeSpaceThd_get
 * Description:
 *      Get nic rx buffer free space threshlod
 * Input:
 *       none                   -
 * Output:
 *      *pval  -  free space threshlod value (uint:8Byte)   
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxFreeSpaceThd_get(rtk_nic_RxMTU_t *pVal)
{
    rtk_uint32 regData = 0;
    
    if (NULL == pVal)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBits(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RXFST_MASK, &regData));

    *pVal = (rtk_nic_RxMTU_t)regData;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_nic_rxCrcErrEn_set
 * Description:
 *     To set rx crc error pkt recieve or drop.
 * Input:
 *      enabled  - enable or  disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxCrcErrEn_set(rtk_enable_t enabled)
{
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RCRCEPE_OFFSET, (enabled ? 1 : 0)));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxCrcErrEn_get
 * Description:
 *      Get rx crc error pkt recieved or be drop.
 * Input:
 *      none                        -
 * Output:
 *      *pEnabled - enable recieve or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxCrcErrEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;

    if (NULL ==  pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR,  RTL8373_NIC_RX_CTRL_RCRCEPE_OFFSET, &regData));
    *pEnabled = (rtk_enable_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxL3CrcErrEn_set
 * Description:
 *      enable rx l3 crc error pkt recieved or not.
 * Input:
 *      enabled  - enable or  disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxL3CrcErrEn_set(rtk_enable_t enabled)
{
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RL3CEPE_OFFSET, (enabled ? 1 : 0)));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxL3CrcErrEn_get
 * Description:
 *      Get rx l3 crc error pkt recieved or be drop.
 * Input:
 *      none                        -
 * Output:
 *      *pEnabled - enable recieve or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxL3CrcErrEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;

    if (NULL ==  pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RL3CEPE_OFFSET, &regData));
    *pEnabled = (rtk_enable_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxL4CrcErrEn_set
 * Description:
 *      enable rx l4 crc error pkt recieved or not.
 * Input:
 *      enabled  - enable or  disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxL4CrcErrEn_set(rtk_enable_t enabled)
{
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RL4CEPE_OFFSET, (enabled ? 1 : 0)));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxL4CrcErrEn_get
 * Description:
 *      Get rx l4 crc error pkt recieved or be drop.
 * Input:
 *      none                        -
 * Output:
 *      *pEnabled - enable recieve or not
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxL4CrcErrEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;

    if (NULL ==  pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RL4CEPE_OFFSET, &regData));
    *pEnabled = (rtk_enable_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxArpEn_set
 * Description:
 *      enable ARP pkt pass or not.
 * Input:
 *      enabled  - enable or  disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxArpEn_set(rtk_enable_t enabled)
{
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_ARPPE_OFFSET, (enabled ? 1 : 0)));

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_nic_rxArpEn_get
 * Description:
 *      Get ARP pkt pass or not config.
 * Input:
 *      none                    -
 * Output:
 *      *pEnabled  - enable or  disable    
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxArpEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;

    if (NULL ==  pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_ARPPE_OFFSET, &regData));
    *pEnabled = (rtk_enable_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxAllPktEn_set
 * Description:
 *      enable nic rx all pkt or not.
 * Input:
 *      enabled  - enable or  disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxAllPktEn_set(rtk_enable_t enabled)
{
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RXAPE_OFFSET, (enabled ? 1 : 0)));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxAllPktEn_get
 * Description:
 *      get nic rx all pkt or not config.
 * Input:
 *      none                       -
 * Output:
 *      *pEnabled  - enable or  disable 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxAllPktEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;

    if (NULL ==  pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RXAPE_OFFSET, &regData));
    *pEnabled = (rtk_enable_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxPhyPktSel_set
 * Description:
 *      enable nic Receive Physical Address Packet Select or not.
 * Input:
 *      enabled  - enable or  disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxPhyPktSel_set(rtk_nic_rxpps_t behavior)
{
    if(behavior >= NIC_RX_PPS_END)
        return RT_ERR_RANGE;

    RTK_ERR_CHK(rtl8373_setAsicRegBits(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RXPPS_MASK, behavior));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxPhyPktSel_get
 * Description:
 *      Get nic Receive Physical Address Packet Select or not config.
 * Input:
 *      none                       -
 * Output:
 *      *pBehavior  - enable or  disable 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxPhyPktSel_get(rtk_nic_rxpps_t *pBehavior)
{
    rtk_uint32 regData = 0;

    if (NULL ==  pBehavior)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBits(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RXPPS_MASK, &regData));
    *pBehavior = (rtk_nic_rxpps_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxMultiPktEn_set
 * Description:
 *      enable nic Receive multicast packet or not.
 * Input:
 *      enabled  - enable or  disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxMultiPktEn_set(rtk_enable_t enabled)
{    
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RXMPE_OFFSET, (enabled ? 1 : 0)));

    return RT_ERR_OK;
}

ret_t dal_rtl8373_nic_rxMultiPktEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;

    if (NULL ==  pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RXMPE_OFFSET, &regData));
    *pEnabled = (rtk_enable_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxBcPktEn_set
 * Description:
 *      enable nic Receive broadcast packet or not.
 * Input:
 *      enabled  - enable or  disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxBcPktEn_set(rtk_enable_t enabled)
{
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RXBPE_OFFSET, (enabled ? 1 : 0)));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxBcPktEn_get
 * Description:
 *      Get nic Receive broadcast packet config.
 * Input:
 *      none                        -
 * Output:
 *      *pEnabled  - enable or  disable   
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxBcPktEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;

    if (NULL ==  pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RXBPE_OFFSET, &regData));
    *pEnabled = (rtk_enable_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_MultiHashFltrEn_set
 * Description:
 *      enable nic Received Multicast Packets Hash Filtering or not.
 * Input:
 *      enabled  - enable or  disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_mcHashFltrEn_set(rtk_enable_t enabled)
{
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_HFMPE_OFFSET, (enabled ? 1 : 0)));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_MultiHashFltrEn_get
 * Description:
 *      get nic Received Multicast Packets Hash Filtering config.
 * Input:
 *      none                      -
 * Output:
 *      *pEnabled  - enable or  disable
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_mcHashFltrEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;

    if (NULL ==  pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_HFMPE_OFFSET, &regData));
    *pEnabled = (rtk_enable_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_PhyPktHashFltrEn_set
 * Description:
 *      enable nic Received unicast Packets Hash Filtering or not.
 * Input:
 *      enabled  - enable or  disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_phyPktHashFltrEn_set(rtk_enable_t enabled)
{
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_HFPPE_OFFSET, (enabled ? 1 : 0)));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_phyPktHashFltrEn_get
 * Description:
 *      Get nic Received unicast Packets Hash Filtering config.
 * Input:
 *      none                        -
 * Output:
 *      *pEnabled  - enable or  disable
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_phyPktHashFltrEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;

    if (NULL ==  pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_HFPPE_OFFSET, &regData));
    *pEnabled = (rtk_enable_t)regData;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_phyPktHashTblVal_set
 * Description:
 *      Set unicast Packets Hash table.
 * Input:
 *      type  - high 32 bits value / low 32 bits value
 *      val - value            -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_phyPktHashTblVal_set(rtk_nic_hashValType_t type, rtk_uint32 val)
{
   rtk_uint32 regAddr = 0; 
   
    if(type > NIC_HASH_VAL_HIGH_WORD)
        return RT_ERR_INPUT;

    regAddr = RTL8373_NIC_UC_HASH_TBL_ADDR(0) + (type << 2) ;
    RTK_ERR_CHK(rtl8373_setAsicReg(regAddr, val));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_phyPktHashTblVal_get
 * Description:
 *      Get unicast Packets Hash table.
 * Input:
 *      type  - high 32 bits value / low 32 bits value
 * Output:
 *      *pVal - value      
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_phyPktHashTblVal_get(rtk_nic_hashValType_t type, rtk_uint32 *pVal)
{
    rtk_uint32 regAddr = 0; 
    
    if(type > NIC_HASH_VAL_HIGH_WORD)
        return RT_ERR_INPUT;
    if (NULL == pVal)
        return RT_ERR_NULL_POINTER;
    
    regAddr = RTL8373_NIC_UC_HASH_TBL_ADDR(0) + (type << 2) ;
    RTK_ERR_CHK(rtl8373_getAsicReg(regAddr, pVal));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_multiPktHashTblVal_set
 * Description:
 *      Set multicast Packets Hash table.
 * Input:
 *      type  - high 32 bits value / low 32 bits value
 *      val - value                -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_mcHashTblVal_set(rtk_nic_hashValType_t type, rtk_uint32 val)
{
   rtk_uint32 regAddr = 0; 
   
    if(type > NIC_HASH_VAL_HIGH_WORD)
        return RT_ERR_INPUT;

    regAddr = RTL8373_NIC_MC_HASH_TBL_ADDR(0) + (type << 2) ;
    RTK_ERR_CHK(rtl8373_setAsicReg(regAddr, val));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_mcHashTblVal_get
 * Description:
 *      Get multicast Packets Hash table.
 * Input:
 *      type  - high 32 bits value / low 32 bits value
 * Output:
 *      *pVal - value      
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_mcHashTblVal_get(rtk_nic_hashValType_t type, rtk_uint32 *pVal)
{
    rtk_uint32 regAddr = 0; 
    
    if(type > NIC_HASH_VAL_HIGH_WORD)
        return RT_ERR_INPUT;
    if (NULL == pVal)
        return RT_ERR_NULL_POINTER;
    
    regAddr = RTL8373_NIC_MC_HASH_TBL_ADDR(0) + (type << 2) ;
    RTK_ERR_CHK(rtl8373_getAsicReg(regAddr, pVal));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxMTU_set
 * Description:
 *      set nic RXMTU.
 * Input:
 *      length  - max length nic could recieved                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxMTU_set(rtk_nic_RxMTU_t lenIdx)
{
    if(lenIdx > NIC_RX_MTU_END)
        return RT_ERR_RANGE;

    RTK_ERR_CHK(rtl8373_setAsicRegBits(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RXMTU_MASK, lenIdx));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_rxMTU_get
 * Description:
 *      Get nic RXMTU.
 * Input:
 *      none                       -
 * Output:
 *      *pLenIdx  - max length nic could recieved 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_rxMTU_get(rtk_nic_RxMTU_t *pLenIdx)
{
    rtk_uint32 regData = 0;
    
    if (NULL == pLenIdx)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBits(RTL8373_NIC_RX_CTRL_ADDR, RTL8373_NIC_RX_CTRL_RXMTU_MASK, &regData));

    *pLenIdx = (rtk_nic_RxMTU_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_txEn_set
 * Description:
 *      enable nic tx ablity.
 * Input:
 *      enabled  - enable or  disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_txEn_set(rtk_enable_t enabled)
{
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_NIC_TX_CTRL_ADDR, RTL8373_NIC_TX_CTRL_TX_EN_OFFSET, (enabled ? 1 : 0 )));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_txEn_get
 * Description:
 *      Get nic tx ablity.
 * Input:
 *      none                  -
 * Output:
 *      *pEnabled  - enable or  disable      
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_txEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;
    
    if (NULL == pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_NIC_TX_CTRL_ADDR, RTL8373_NIC_TX_CTRL_TX_EN_OFFSET, &regData));
    *pEnabled = (rtk_enable_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_LoopbackEn_set
 * Description:
 *      enable nic loopback ablity.
 * Input:
 *      enabled  - enable or  disable                        -
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_loopbackEn_set(rtk_enable_t enabled)
{
    if(enabled > RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_NIC_TX_CTRL_ADDR, RTL8373_NIC_TX_CTRL_LOOPBACK_EN_OFFSET, (enabled ? 1 : 0 )));

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_LoopbackEn_get
 * Description:
 *      Get nic loopback ablity.
 * Input:
 *      none                    -
 * Output:
 *      *pEnabled  - enable or  disable    
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_loopbackEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32 regData = 0;

    if (NULL == pEnabled)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_NIC_TX_CTRL_ADDR, RTL8373_NIC_TX_CTRL_LOOPBACK_EN_OFFSET, &regData));
    *pEnabled = (rtk_enable_t)regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_interruptEn_set
 * Description:
 *      enable nix rx pkt interrupt and nic tx error interrupt.
 * Input:
 *      rxie  - enable or  disable nic rx pkt interrupt                       -
 *      txee  - enable or  disable nic tx error interrupt               
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_interruptEn_set(rtk_enable_t rxie, rtk_enable_t txee)
{
    rtk_uint32 mask = 0;
    if((rxie > RTK_ENABLE_END) || (txee > RTK_ENABLE_END))
        return RT_ERR_ENABLE;

    mask = (rxie ? 1 : 0 ) << RTL8373_NIC_INT_MSK_RXIE_OFFSET;
    mask |= (txee ? 1 : 0 );
    RTK_ERR_CHK(rtl8373_setAsicReg(RTL8373_NIC_INT_MSK_ADDR, mask));
    
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_interruptEn_get
 * Description:
 *      Get nix rx pkt interrupt and nic tx error interrupt config .
 * Input:
 *      none        
 * Output:
 *      *pRxie  -  nic rx pkt interrupt config:enable or disable                       -
 *      *pTxee  - nic tx error interrupt config:enable or disable 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_interruptEn_get(rtk_enable_t *pRxie, rtk_enable_t *pTxee)
{    
    rtk_uint32 regData = 0;
    
    if ((NULL == pRxie) || (NULL == pTxee))
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicReg(RTL8373_NIC_INT_MSK_ADDR, &regData));
    *pRxie = (rtk_enable_t)((regData >> RTL8373_NIC_INT_MSK_RXIE_OFFSET) & 1);
    *pTxee = (rtk_enable_t)(regData & 1);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_interruptStatus_get
 * Description:
 *      get nix rx pkt interrupt and nic tx error interrupt status .
 * Input:
 *      none        
 * Output:
 *      *pRxis  -  nic rx pkt interrupt occured or not                       -
 *      *pTxes  - nic tx error interrupt occured or not  
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_interruptStatus_get(rtk_uint32 *pRxis, rtk_uint32 *pTxes)
{
    rtk_uint32 regData = 0;
    if (NULL == pRxis)
        return RT_ERR_NULL_POINTER;
    if (NULL == pTxes)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicReg(RTL8373_NIC_INT_STS_ADDR,  &regData));
    
    *pRxis = (regData >> RTL8373_NIC_INT_STS_RXIS_OFFSET) & 1;
    *pTxes = (regData & 1);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_nic_interruptStatus_clear
 * Description:
 *      clear nix rx pkt interrupt status and nic tx error interrupt status .
 * Input:
 *      rxis  - clear nic rx pkt interrupt status                      -
 *      txes  - clear or  disable nic tx error interrupt  status       
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
ret_t dal_rtl8373_nic_interruptStatus_clear(rtk_uint32 rxis, rtk_uint32 txes)
{
    rtk_uint32 regData = 0;
    if (rxis > 1)
        return RT_ERR_INPUT;
    if (txes > 1)
        return RT_ERR_INPUT;
    
    regData = (rxis << RTL8373_NIC_INT_STS_RXIS_OFFSET) |txes;
    RTK_ERR_CHK(rtl8373_setAsicReg(RTL8373_NIC_INT_STS_ADDR, regData));

    return RT_ERR_OK;
}

