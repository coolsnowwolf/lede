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
 * $Date: 2017-03-08 15:13:58 +0800 (週三, 08 三月 2017) $
 *
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : Port mirror related functions
 *
 */
#include <rtl8367c_asicdrv_mirror.h>
/* Function Name:
 *      rtl8367c_setAsicPortMirror
 * Description:
 *      Set port mirror function
 * Input:
 *      source  - Source port
 *      monitor - Monitor (destination) port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortMirror(rtk_uint32 source, rtk_uint32 monitor)
{
    ret_t retVal;

    if((source > RTL8367C_PORTIDMAX) || (monitor > RTL8367C_PORTIDMAX))
        return RT_ERR_PORT_ID;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_MIRROR_CTRL_REG, RTL8367C_MIRROR_SOURCE_PORT_MASK, source);
    if(retVal != RT_ERR_OK)
        return retVal;


    return rtl8367c_setAsicRegBits(RTL8367C_MIRROR_CTRL_REG, RTL8367C_MIRROR_MONITOR_PORT_MASK, monitor);
}
/* Function Name:
 *      rtl8367c_getAsicPortMirror
 * Description:
 *      Get port mirror function
 * Input:
 *      pSource     - Source port
 *      pMonitor - Monitor (destination) port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortMirror(rtk_uint32 *pSource, rtk_uint32 *pMonitor)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_MIRROR_CTRL_REG, RTL8367C_MIRROR_SOURCE_PORT_MASK, pSource);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367c_getAsicRegBits(RTL8367C_MIRROR_CTRL_REG, RTL8367C_MIRROR_MONITOR_PORT_MASK, pMonitor);
}
/* Function Name:
 *      rtl8367c_setAsicPortMirrorRxFunction
 * Description:
 *      Set the mirror function on RX of the mirrored
 * Input:
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortMirrorRxFunction(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_MIRROR_CTRL_REG, RTL8367C_MIRROR_RX_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicPortMirrorRxFunction
 * Description:
 *      Get the mirror function on RX of the mirrored
 * Input:
 *      pEnabled    - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortMirrorRxFunction(rtk_uint32* pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_MIRROR_CTRL_REG, RTL8367C_MIRROR_RX_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367c_setAsicPortMirrorTxFunction
 * Description:
 *      Set the mirror function on TX of the mirrored
 * Input:
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortMirrorTxFunction(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_MIRROR_CTRL_REG, RTL8367C_MIRROR_TX_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicPortMirrorTxFunction
 * Description:
 *      Get the mirror function on TX of the mirrored
 * Input:
 *      pEnabled    - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortMirrorTxFunction(rtk_uint32* pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_MIRROR_CTRL_REG, RTL8367C_MIRROR_TX_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367c_setAsicPortMirrorIsolation
 * Description:
 *      Set the traffic isolation on monitor port
 * Input:
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortMirrorIsolation(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_MIRROR_CTRL_REG, RTL8367C_MIRROR_ISO_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicPortMirrorIsolation
 * Description:
 *      Get the traffic isolation on monitor port
 * Input:
 *      pEnabled    - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortMirrorIsolation(rtk_uint32* pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_MIRROR_CTRL_REG, RTL8367C_MIRROR_ISO_OFFSET, pEnabled);
}

/* Function Name:
 *      rtl8367c_setAsicPortMirrorMask
 * Description:
 *      Set mirror source port mask
 * Input:
 *      SourcePortmask  - Source Portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_MASK- Port Mask Error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortMirrorMask(rtk_uint32 SourcePortmask)
{
    if( SourcePortmask > RTL8367C_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_MIRROR_SRC_PMSK, RTL8367C_MIRROR_SRC_PMSK_MASK, SourcePortmask);
}

/* Function Name:
 *      rtl8367c_getAsicPortMirrorMask
 * Description:
 *      Get mirror source port mask
 * Input:
 *      None
 * Output:
 *      pSourcePortmask     - Source Portmask
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_MASK- Port Mask Error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortMirrorMask(rtk_uint32 *pSourcePortmask)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_MIRROR_SRC_PMSK, RTL8367C_MIRROR_SRC_PMSK_MASK, pSourcePortmask);
}

/* Function Name:
 *      rtl8367c_setAsicPortMirrorVlanRxLeaky
 * Description:
 *      Set the mirror function of VLAN RX leaky
 * Input:
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortMirrorVlanRxLeaky(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_MIRROR_CTRL2, RTL8367C_MIRROR_RX_VLAN_LEAKY_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicPortMirrorVlanRxLeaky
 * Description:
 *      Get the mirror function of VLAN RX leaky
 * Input:
 *      pEnabled    - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortMirrorVlanRxLeaky(rtk_uint32* pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_MIRROR_CTRL2, RTL8367C_MIRROR_RX_VLAN_LEAKY_OFFSET, pEnabled);
}

/* Function Name:
 *      rtl8367c_setAsicPortMirrorVlanTxLeaky
 * Description:
 *      Set the mirror function of VLAN TX leaky
 * Input:
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortMirrorVlanTxLeaky(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_MIRROR_CTRL2, RTL8367C_MIRROR_TX_VLAN_LEAKY_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicPortMirrorVlanTxLeaky
 * Description:
 *      Get the mirror function of VLAN TX leaky
 * Input:
 *      pEnabled    - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortMirrorVlanTxLeaky(rtk_uint32* pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_MIRROR_CTRL2, RTL8367C_MIRROR_TX_VLAN_LEAKY_OFFSET, pEnabled);
}

/* Function Name:
 *      rtl8367c_setAsicPortMirrorIsolationRxLeaky
 * Description:
 *      Set the mirror function of  Isolation RX leaky
 * Input:
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortMirrorIsolationRxLeaky(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_MIRROR_CTRL2, RTL8367C_MIRROR_RX_ISOLATION_LEAKY_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicPortMirrorIsolationRxLeaky
 * Description:
 *      Get the mirror function of VLAN RX leaky
 * Input:
 *      pEnabled    - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortMirrorIsolationRxLeaky(rtk_uint32* pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_MIRROR_CTRL2, RTL8367C_MIRROR_RX_ISOLATION_LEAKY_OFFSET, pEnabled);
}

/* Function Name:
 *      rtl8367c_setAsicPortMirrorIsolationTxLeaky
 * Description:
 *      Set the mirror function of Isolation TX leaky
 * Input:
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortMirrorIsolationTxLeaky(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_MIRROR_CTRL2, RTL8367C_MIRROR_TX_ISOLATION_LEAKY_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicPortMirrorIsolationTxLeaky
 * Description:
 *      Get the mirror function of VLAN TX leaky
 * Input:
 *      pEnabled    - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortMirrorIsolationTxLeaky(rtk_uint32* pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_MIRROR_CTRL2, RTL8367C_MIRROR_TX_ISOLATION_LEAKY_OFFSET, pEnabled);
}

/* Function Name:
 *      rtl8367c_setAsicPortMirrorRealKeep
 * Description:
 *      Set the mirror function of keep format
 * Input:
 *      mode    - 1: keep original format, 0: follow VLAN config
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortMirrorRealKeep(rtk_uint32 mode)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_MIRROR_CTRL2, RTL8367C_MIRROR_REALKEEP_EN_OFFSET, mode);
}
/* Function Name:
 *      rtl8367c_getAsicPortMirrorRealKeep
 * Description:
 *      Get the mirror function of keep format
 * Input:
 *      pMode   - 1: keep original format, 0: follow VLAN config
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortMirrorRealKeep(rtk_uint32* pMode)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_MIRROR_CTRL2, RTL8367C_MIRROR_REALKEEP_EN_OFFSET, pMode);
}

/* Function Name:
 *      rtl8367c_setAsicPortMirrorOverride
 * Description:
 *      Set the mirror function of override
 * Input:
 *      rxMirror    - 1: output rx Mirror format, 0: output forward format
 *      txMirror    - 1: output tx Mirror format, 0: output forward format
 *      aclMirror   - 1: output ACL Mirror format, 0: output forward format
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortMirrorOverride(rtk_uint32 rxMirror, rtk_uint32 txMirror, rtk_uint32 aclMirror)
{
    ret_t retVal;

    if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_MIRROR_CTRL3, RTL8367C_MIRROR_RX_OVERRIDE_EN_OFFSET, rxMirror)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_MIRROR_CTRL3, RTL8367C_MIRROR_TX_OVERRIDE_EN_OFFSET, txMirror)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_MIRROR_CTRL3, RTL8367C_MIRROR_ACL_OVERRIDE_EN_OFFSET, aclMirror)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicPortMirrorOverride
 * Description:
 *      Get the mirror function of override
 * Input:
 *      None
 * Output:
 *      pRxMirror   - 1: output rx Mirror format, 0: output forward format
 *      pTxMirror   - 1: output tx Mirror format, 0: output forward format
 *      pAclMirror  - 1: output ACL Mirror format, 0: output forward format
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortMirrorOverride(rtk_uint32 *pRxMirror, rtk_uint32 *pTxMirror, rtk_uint32 *pAclMirror)
{
    ret_t retVal;

    if((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_MIRROR_CTRL3, RTL8367C_MIRROR_RX_OVERRIDE_EN_OFFSET, pRxMirror)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_MIRROR_CTRL3, RTL8367C_MIRROR_TX_OVERRIDE_EN_OFFSET, pTxMirror)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_MIRROR_CTRL3, RTL8367C_MIRROR_ACL_OVERRIDE_EN_OFFSET, pAclMirror)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
