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
    
#include <rtk_switch.h>
#include <rtk_error.h>
#include <nic.h>

#include <string.h>
    
#include <dal/dal_mgmt.h>


/* Function Name:
 *      rtk_nic_rst_set
 * Description:
 *      nic reset
 * Input:
 *      enabled  -  enable or disable
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_ENABLE
 * Note:
 *      nic reset
 */
rtk_api_ret_t rtk_nic_rst_set(void)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->nic_rst_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rst_set();
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_txstop_set
 * Description:
 *      Set rtk nic buffer tx stop address
 * Input:
 *      addr       -  txstop address
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_ENABLE
 * Note:
 *
 */
rtk_api_ret_t rtk_nic_txstop_set( rtk_uint32 addr)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->nic_txStopAddr_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_txStopAddr_set(addr);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_txstop_get
 * Description:
 *      Get rtk nic buffer tx stop address
 * Input:
 *      none
 * Output:
 *      pAddr  -  the pointer of address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *
 */
rtk_api_ret_t rtk_nic_txstop_get(rtk_uint32 *pAddr)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->nic_txStopAddr_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_txStopAddr_get(pAddr);
    RTK_API_UNLOCK();

    return retVal;
}


/* Function Name:
 *      rtk_nic_rxstop_set
 * Description:
 *      Set rtk nic buffer rx stop address
 * Input:
 *      addr       -  rxstop address
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_ENABLE
 * Note:
 *
 */
rtk_api_ret_t rtk_nic_rxstop_set( rtk_uint32 addr)
{
    rtk_api_ret_t retVal;
    
    if (NULL == RT_MAPPER->nic_rxStopAddr_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxStopAddr_set(addr);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_rxstop_get
 * Description:
 *      Get rtk nic buffer rx stop address
 * Input:
 *      none
 * Output:
 *      pAddr  -  the pointer of address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *
 */
rtk_api_ret_t rtk_nic_rxstop_get(rtk_uint32 *pAddr)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->nic_rxStopAddr_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxStopAddr_get(pAddr);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_swRxCurPktAddr_get
 * Description:
 *      Get Switch Rx current packet address
 * Input:
 *      none
 * Output:
 *      pAddr  -  the pointer of address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *
 */
rtk_api_ret_t rtk_nic_swRxCurPktAddr_get(rtk_uint32 *pAddr)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->nic_swRxCurPktAddr_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_swRxCurPktAddr_get(pAddr);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_rxDataLength_get
 * Description:
 *      Get nic rx buffer received packet length
 * Input:
 *      none
 * Output:
 *      pLength  - rx buffer received packet length
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *
 */
rtk_api_ret_t rtk_nic_rxReceivedPktLen_get(rtk_uint32 *pLength)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->nic_rxReceivedPktLen_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxReceivedPktLen_get(pLength);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_txAvailableSpace_get
 * Description:
 *      Get nic tx buffer available free space
 * Input:
 *      none
 * Output:
 *      pAddr  -  the pointer of address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *
 */
rtk_api_ret_t rtk_nic_txAvailSpace_get(rtk_uint32 *pLength)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->nic_txAvailSpace_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_txAvailSpace_get(pLength);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_moduleEn_set
 * Description:
 *      Enable/Disable NIC module .
 * Input:
 *      none
 * Output:
 *      pStatus - nic status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_FOUND
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *
 * Note:
 *      None
 */
rtk_api_ret_t rtk_nic_moduleEn_set(rtk_enable_t enable)
{
    rtk_api_ret_t   retVal ;

    if (NULL == RT_MAPPER->nic_moduleEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_moduleEn_set(enable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_moduleEn_get
 * Description:
 *      Get NIC module status .
 * Input:
 *      none
 * Output:
 *      pStatus - nic module  status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_FOUND
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *
 * Note:
 *      None
 */
rtk_api_ret_t rtk_nic_moduleEn_get(rtk_enable_t *pStatus)
{
    rtk_api_ret_t   retVal ;
    
    if (NULL == RT_MAPPER->nic_moduleEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_moduleEn_get(pStatus);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_rxEn_set
 * Description:
 *      Set NIC rx status .
 * Input:
 *      none
 * Output:
 *      pStatus - rx status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_FOUND
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *
 * Note:
 *      None
 */
rtk_api_ret_t rtk_nic_rxEn_set(rtk_enable_t enable)
{
    rtk_api_ret_t   retVal ;

    if (NULL == RT_MAPPER->nic_rxEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxEn_set(enable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_rx_en_get
 * Description:
 *      Get NIC rx status .
 * Input:
 *      none
 * Output:
 *      pStatus - rx status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_FOUND
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *
 * Note:
 *      None
 */
rtk_api_ret_t rtk_nic_rxEn_get(rtk_enable_t *pStatus)
{
    rtk_api_ret_t   retVal ;
    
    if (NULL == RT_MAPPER->nic_rxEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxEn_get(pStatus);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_txEn_set
 * Description:
 *      Set NIC tx status .
 * Input:
 *      none
 * Output:
 *      pStatus - tx status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_FOUND
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *
 * Note:
 *      None
 */
rtk_api_ret_t rtk_nic_txEn_set(rtk_enable_t enable)
{
    rtk_api_ret_t   retVal ;


    if (NULL == RT_MAPPER->nic_txEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_txEn_set(enable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_tx_en_get
 * Description:
 *      Get NIC tx status .
 * Input:
 *      none
 * Output:
 *      pStatus - tx status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_FOUND
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *
 * Note:
 *      None
 */
rtk_api_ret_t rtk_nic_txEn_get(rtk_enable_t *pStatus)
{
    rtk_uint32      retVal ;

    if (NULL == RT_MAPPER->nic_txEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_txEn_get(pStatus);
    RTK_API_UNLOCK();

    return retVal;
}


/* Function Name:
 *      rtk_nic_rxRemoveCrc_set
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
rtk_api_ret_t  rtk_nic_rxRemoveCrc_set(rtk_enable_t enabled)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_rxRemoveCrc_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxRemoveCrc_set(enabled);
    RTK_API_UNLOCK();

    return retVal;
}

rtk_api_ret_t  rtk_nic_rxRemoveCrc_get(rtk_enable_t *pEnabled)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_rxRemoveCrc_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxRemoveCrc_get(pEnabled);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_rxPaddingEn_set
 * Description:
 *      Setting Nic rx padding control 
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
rtk_api_ret_t  rtk_nic_rxPaddingEn_set(rtk_enable_t enabled)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_rxPaddingEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxPaddingEn_set(enabled);
    RTK_API_UNLOCK();

    return retVal;
}

rtk_api_ret_t  rtk_nic_rxPaddingEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_rxPaddingEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxPaddingEn_get(pEnabled);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_rxFreeSpaceThd_set
 * Description:
 *      Setting Nic rx buffer free space threshold.
 * Input:
 *      val  - free space threshold(uint: 8Byte)                      -
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
rtk_api_ret_t  rtk_nic_rxFreeSpaceThd_set(rtk_uint32 val)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_rxFreeSpaceThd_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxFreeSpaceThd_set(val);
    RTK_API_UNLOCK();

    return retVal;
}

rtk_api_ret_t  rtk_nic_rxFreeSpaceThd_get(rtk_uint32 *pVal)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_rxFreeSpaceThd_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxFreeSpaceThd_get(pVal);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
*      rtk_nic_rxCrcErrEn_set
* Description:
*      enable rx crc error pkt or not.
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
rtk_api_ret_t rtk_nic_rxCrcErrEn_set(rtk_enable_t enabled)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_rxCrcErrEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxCrcErrEn_set(enabled);
    RTK_API_UNLOCK();

    return retVal;
}

rtk_api_ret_t rtk_nic_rxCrcErrEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_rxCrcErrEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxCrcErrEn_get(pEnabled);
    RTK_API_UNLOCK();
    
    return retVal;
}

/* Function Name:
 *      rtk_nic_rxL3CrcErrEn_set
 * Description:
 *      enable rx l3 crc error pkt or not.
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
rtk_api_ret_t rtk_nic_rxL3CrcErrEn_set(rtk_enable_t enabled)
{
    rtk_uint32   retVal ;
    if (NULL == RT_MAPPER->nic_rxL3CrcErrEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxL3CrcErrEn_set(enabled);
    RTK_API_UNLOCK();

    return retVal;
}

rtk_api_ret_t rtk_nic_rxL3CrcErrEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32   retVal ;
    if (NULL == RT_MAPPER->nic_rxL3CrcErrEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxL3CrcErrEn_get(pEnabled);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_rxL4CrcErrEn_set
 * Description:
 *      enable rx l4 crc error pkt or not.
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
rtk_api_ret_t rtk_nic_rxL4CrcErrEn_set(rtk_enable_t enabled)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_rxL4CrcErrEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxL4CrcErrEn_set(enabled);
    RTK_API_UNLOCK();

    return retVal;
}

rtk_api_ret_t rtk_nic_rxL4CrcErrEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32   retVal ;
    if (NULL == RT_MAPPER->nic_rxL4CrcErrEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxL4CrcErrEn_get(pEnabled);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_rxArpPassEn_set
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
rtk_api_ret_t rtk_nic_rxArpEn_set(rtk_enable_t enabled)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_rxArpEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxArpEn_set(enabled);
    RTK_API_UNLOCK();

    return retVal;
}

rtk_api_ret_t rtk_nic_rxArpEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_rxArpEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxArpEn_get(pEnabled);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_rxAllPktEn_set
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
rtk_api_ret_t rtk_nic_rxAllPktEn_set(rtk_enable_t enabled)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_rxAllPktEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxAllPktEn_set(enabled);
    RTK_API_UNLOCK();

    return retVal;
}

rtk_api_ret_t rtk_nic_rxAllPktEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_rxAllPktEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxAllPktEn_get(pEnabled);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_nic_rxPhyPktSel_set
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
rtk_api_ret_t rtk_nic_rxPhyPktSel_set(rtk_nic_rxpps_t behavior)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_rxPhyPktSel_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxPhyPktSel_set(behavior);
    RTK_API_UNLOCK();

    return retVal;
}

rtk_api_ret_t rtk_nic_rxPhyPktSel_get(rtk_nic_rxpps_t *pBehavior)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_rxPhyPktSel_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxPhyPktSel_get(pBehavior);
    RTK_API_UNLOCK();
    
    return retVal;
}

/* Function Name:
 *      rtk_nic_rxMultiPktEn_set
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
rtk_api_ret_t rtk_nic_rxMultiPktEn_set(rtk_enable_t enabled)
{
    rtk_uint32   retVal ;
    if (NULL == RT_MAPPER->nic_rxMultiPktEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxMultiPktEn_set(enabled);
    RTK_API_UNLOCK();
    
    return retVal;
}

rtk_api_ret_t rtk_nic_rxMultiPktEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_rxMultiPktEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxMultiPktEn_get(pEnabled);
    RTK_API_UNLOCK();
    
    return retVal;
}

/* Function Name:
 *      rtk_nic_rxBcPktEn_set
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
rtk_api_ret_t rtk_nic_rxBcPktEn_set(rtk_enable_t enabled)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_rxBcPktEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxBcPktEn_set(enabled);
    RTK_API_UNLOCK();
    
    return retVal;
}

rtk_api_ret_t rtk_nic_rxBcPktEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_rxBcPktEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxBcPktEn_get(pEnabled);
    RTK_API_UNLOCK();
    
    return retVal;
}

/* Function Name:
 *      rtk_nic_mcHashFltrEn_set
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
rtk_api_ret_t rtk_nic_mcHashFltrEn_set(rtk_enable_t enabled)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_mcHashFltrEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_mcHashFltrEn_set(enabled);
    RTK_API_UNLOCK();
    
    return retVal;
}

rtk_api_ret_t rtk_nic_mcHashFltrEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32   retVal ;
    if (NULL == RT_MAPPER->nic_mcHashFltrEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_mcHashFltrEn_get(pEnabled);
    RTK_API_UNLOCK();
    
    return retVal;     
}
/* Function Name:
 *      rtk_nic_PhyPktHashFltrEn_set
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
rtk_api_ret_t rtk_nic_phyPktHashFltrEn_set(rtk_enable_t enabled)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_phyPktHashFltrEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_phyPktHashFltrEn_set(enabled);
    RTK_API_UNLOCK();
    
    return retVal;
}

rtk_api_ret_t rtk_nic_phyPktHashFltrEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_phyPktHashFltrEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_phyPktHashFltrEn_get(pEnabled);
    RTK_API_UNLOCK();
    
    return retVal;
}


/* Function Name:
 *      rtk_nic_mcHashTblVal_set
 * Description:
 *      Set nic rx multicast pkt hash table value.
 * Input:
 *      type  - high 32 bits value / low 32 bits value   
 *      val - value
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
rtk_api_ret_t rtk_nic_mcHashTblVal_set(rtk_nic_hashValType_t type, rtk_uint32 val)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_mcHashTblVal_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_mcHashTblVal_set(type, val);
    RTK_API_UNLOCK();
    
    return retVal;
}

rtk_api_ret_t rtk_nic_mcHashTblVal_get(rtk_nic_hashValType_t type, rtk_uint32 *pVal)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_mcHashTblVal_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_mcHashTblVal_get(type, pVal);
    RTK_API_UNLOCK();
    
    return retVal;
}


/* Function Name:
 *      rtk_nic_phyPktHashTblVal_set
 * Description:
 *       Set nic rx unicast pkt hash table value.
 * Input:
 *      type  - high 32 bits value / low 32 bits value
 *      val - value
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
rtk_api_ret_t rtk_nic_phyPktHashTblVal_set(rtk_nic_hashValType_t type, rtk_uint32 val)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_phyPktHashTblVal_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_phyPktHashTblVal_set(type, val);
    RTK_API_UNLOCK();
    
    return retVal;
}

rtk_api_ret_t rtk_nic_phyPktHashTblVal_get(rtk_nic_hashValType_t type, rtk_uint32 *pVal)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_phyPktHashTblVal_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_phyPktHashTblVal_get(type, pVal);
    RTK_API_UNLOCK();
    
    return retVal;
}

/* Function Name:
 *      rtk_nic_rxMTU_set
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
rtk_api_ret_t rtk_nic_rxMTU_set(rtk_nic_RxMTU_t length)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_rxMTU_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxMTU_set(length);
    RTK_API_UNLOCK();
    
    return retVal;
}

rtk_api_ret_t rtk_nic_rxMTU_get(rtk_nic_RxMTU_t *pLength)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_rxMTU_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_rxMTU_get(pLength);
    RTK_API_UNLOCK();
    
    return retVal;
}

/* Function Name:
 *      rtk_nic_LoopbackEn_set
 * Description:
 *      enable nic loopback ablity.
 * Input:
 *      enabled  - enable or disable
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
rtk_api_ret_t rtk_nic_loopbackEn_set(rtk_enable_t enabled)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_loopbackEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_loopbackEn_set(enabled);
    RTK_API_UNLOCK();
    
    return retVal;
}

rtk_api_ret_t rtk_nic_loopbackEn_get(rtk_enable_t *pEnabled)
{
    rtk_uint32   retVal;

    if (NULL == RT_MAPPER->nic_loopbackEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_loopbackEn_get(pEnabled);
    RTK_API_UNLOCK();
    
    return retVal;
}

/* Function Name:
 *      rtk_nic_InterruptEn_set
 * Description:
 *      enable nic Rx Interrupt and TX Error Interrupt.
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
rtk_api_ret_t rtk_nic_interruptEn_set(rtk_enable_t rxie, rtk_enable_t txee)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_interruptEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_interruptEn_set(rxie, txee);
    RTK_API_UNLOCK();
    
    return retVal;
}

rtk_api_ret_t rtk_nic_interruptEn_get(rtk_enable_t * pRxie, rtk_enable_t *pTxee)
{
    rtk_uint32   retVal ;
    
    if (NULL == RT_MAPPER->nic_interruptEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_interruptEn_get(pRxie, pTxee);
    RTK_API_UNLOCK();
    
    return retVal;
}

/* Function Name:
 *      rtk_nic_InterruptStatus_set
 * Description:
 *      get nic Rx Interrupt's status and TX Error Interrupt's status.
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
rtk_api_ret_t rtk_nic_interruptStatus_get(rtk_uint32 * pRxis, rtk_uint32 *pTxes)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_interruptStatus_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_interruptStatus_get(pRxis, pTxes);
    RTK_API_UNLOCK();
    
    return retVal;
}

rtk_api_ret_t rtk_nic_interruptStatus_clear(rtk_uint32 rxis, rtk_uint32 txes)
{
    rtk_uint32   retVal ;

    if (NULL == RT_MAPPER->nic_interruptStatus_clear)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->nic_interruptStatus_clear(rxis, txes);
    RTK_API_UNLOCK();
    
    return retVal;
}




