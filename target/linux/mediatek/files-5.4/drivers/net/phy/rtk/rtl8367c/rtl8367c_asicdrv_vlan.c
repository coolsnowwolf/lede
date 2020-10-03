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
 * Feature : VLAN related functions
 *
 */
#include <rtl8367c_asicdrv_vlan.h>

#include <string.h>

#if defined(CONFIG_RTL8367C_ASICDRV_TEST)
rtl8367c_user_vlan4kentry Rtl8370sVirtualVlanTable[RTL8367C_VIDMAX + 1];
#endif

static void _rtl8367c_VlanMCStUser2Smi(rtl8367c_vlanconfiguser *pVlanCg, rtk_uint16 *pSmiVlanCfg)
{
    pSmiVlanCfg[0] |= pVlanCg->mbr & 0x07FF;

    pSmiVlanCfg[1] |= pVlanCg->fid_msti & 0x000F;

    pSmiVlanCfg[2] |= pVlanCg->vbpen & 0x0001;
    pSmiVlanCfg[2] |= (pVlanCg->vbpri & 0x0007) << 1;
    pSmiVlanCfg[2] |= (pVlanCg->envlanpol & 0x0001) << 4;
    pSmiVlanCfg[2] |= (pVlanCg->meteridx & 0x003F) << 5;

    pSmiVlanCfg[3] |= pVlanCg->evid & 0x1FFF;
}

static void _rtl8367c_VlanMCStSmi2User(rtk_uint16 *pSmiVlanCfg, rtl8367c_vlanconfiguser *pVlanCg)
{
    pVlanCg->mbr            = pSmiVlanCfg[0] & 0x07FF;
    pVlanCg->fid_msti       = pSmiVlanCfg[1] & 0x000F;
    pVlanCg->meteridx       = (pSmiVlanCfg[2] >> 5) & 0x003F;
    pVlanCg->envlanpol      = (pSmiVlanCfg[2] >> 4) & 0x0001;
    pVlanCg->vbpri          = (pSmiVlanCfg[2] >> 1) & 0x0007;
    pVlanCg->vbpen          = pSmiVlanCfg[2] & 0x0001;
    pVlanCg->evid           = pSmiVlanCfg[3] & 0x1FFF;
}

static void _rtl8367c_Vlan4kStUser2Smi(rtl8367c_user_vlan4kentry *pUserVlan4kEntry, rtk_uint16 *pSmiVlan4kEntry)
{
    pSmiVlan4kEntry[0] |= (pUserVlan4kEntry->mbr & 0x00FF);
    pSmiVlan4kEntry[0] |= (pUserVlan4kEntry->untag & 0x00FF) << 8;

    pSmiVlan4kEntry[1] |= (pUserVlan4kEntry->fid_msti & 0x000F);
    pSmiVlan4kEntry[1] |= (pUserVlan4kEntry->vbpen & 0x0001) << 4;
    pSmiVlan4kEntry[1] |= (pUserVlan4kEntry->vbpri & 0x0007) << 5;
    pSmiVlan4kEntry[1] |= (pUserVlan4kEntry->envlanpol & 0x0001) << 8;
    pSmiVlan4kEntry[1] |= (pUserVlan4kEntry->meteridx & 0x001F) << 9;
    pSmiVlan4kEntry[1] |= (pUserVlan4kEntry->ivl_svl & 0x0001) << 14;

    pSmiVlan4kEntry[2] |= ((pUserVlan4kEntry->mbr & 0x0700) >> 8);
    pSmiVlan4kEntry[2] |= ((pUserVlan4kEntry->untag & 0x0700) >> 8) << 3;
    pSmiVlan4kEntry[2] |= ((pUserVlan4kEntry->meteridx & 0x0020) >> 5) << 6;
}


static void _rtl8367c_Vlan4kStSmi2User(rtk_uint16 *pSmiVlan4kEntry, rtl8367c_user_vlan4kentry *pUserVlan4kEntry)
{
    pUserVlan4kEntry->mbr = (pSmiVlan4kEntry[0] & 0x00FF) | ((pSmiVlan4kEntry[2] & 0x0007) << 8);
    pUserVlan4kEntry->untag = ((pSmiVlan4kEntry[0] & 0xFF00) >> 8) | (((pSmiVlan4kEntry[2] & 0x0038) >> 3) << 8);
    pUserVlan4kEntry->fid_msti = pSmiVlan4kEntry[1] & 0x000F;
    pUserVlan4kEntry->vbpen = (pSmiVlan4kEntry[1] & 0x0010) >> 4;
    pUserVlan4kEntry->vbpri = (pSmiVlan4kEntry[1] & 0x00E0) >> 5;
    pUserVlan4kEntry->envlanpol = (pSmiVlan4kEntry[1] & 0x0100) >> 8;
    pUserVlan4kEntry->meteridx = ((pSmiVlan4kEntry[1] & 0x3E00) >> 9) | (((pSmiVlan4kEntry[2] & 0x0040) >> 6) << 5);
    pUserVlan4kEntry->ivl_svl = (pSmiVlan4kEntry[1] & 0x4000) >> 14;
}

/* Function Name:
 *      rtl8367c_setAsicVlanMemberConfig
 * Description:
 *      Set 32 VLAN member configurations
 * Input:
 *      index       - VLAN member configuration index (0~31)
 *      pVlanCg - VLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameter
 *      RT_ERR_L2_FID               - Invalid FID
 *      RT_ERR_PORT_MASK            - Invalid portmask
 *      RT_ERR_FILTER_METER_ID      - Invalid meter
 *      RT_ERR_QOS_INT_PRIORITY     - Invalid priority
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - Invalid VLAN member configuration index
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicVlanMemberConfig(rtk_uint32 index, rtl8367c_vlanconfiguser *pVlanCg)
{
    ret_t  retVal;
    rtk_uint32 regAddr;
    rtk_uint32 regData;
    rtk_uint16 *tableAddr;
    rtk_uint32 page_idx;
    rtk_uint16 smi_vlancfg[RTL8367C_VLAN_MBRCFG_LEN];

    /* Error Checking  */
    if(index > RTL8367C_CVIDXMAX)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;

    if(pVlanCg->evid > RTL8367C_EVIDMAX)
        return RT_ERR_INPUT;


    if(pVlanCg->mbr > RTL8367C_PORTMASK)
        return RT_ERR_PORT_MASK;

    if(pVlanCg->fid_msti > RTL8367C_FIDMAX)
        return RT_ERR_L2_FID;

    if(pVlanCg->meteridx > RTL8367C_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(pVlanCg->vbpri > RTL8367C_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    memset(smi_vlancfg, 0x00, sizeof(rtk_uint16) * RTL8367C_VLAN_MBRCFG_LEN);
    _rtl8367c_VlanMCStUser2Smi(pVlanCg, smi_vlancfg);
    tableAddr = smi_vlancfg;

    for(page_idx = 0; page_idx < 4; page_idx++)  /* 4 pages per VLAN Member Config */
    {
        regAddr = RTL8367C_VLAN_MEMBER_CONFIGURATION_BASE + (index * 4) + page_idx;
        regData = *tableAddr;

        retVal = rtl8367c_setAsicReg(regAddr, regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        tableAddr++;
    }

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicVlanMemberConfig
 * Description:
 *      Get 32 VLAN member configurations
 * Input:
 *      index       - VLAN member configuration index (0~31)
 *      pVlanCg - VLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameter
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - Invalid VLAN member configuration index
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicVlanMemberConfig(rtk_uint32 index, rtl8367c_vlanconfiguser *pVlanCg)
{
    ret_t  retVal;
    rtk_uint32 page_idx;
    rtk_uint32 regAddr;
    rtk_uint32 regData;
    rtk_uint16 *tableAddr;
    rtk_uint16 smi_vlancfg[RTL8367C_VLAN_MBRCFG_LEN];

    if(index > RTL8367C_CVIDXMAX)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;

    memset(smi_vlancfg, 0x00, sizeof(rtk_uint16) * RTL8367C_VLAN_MBRCFG_LEN);
    tableAddr  = smi_vlancfg;

    for(page_idx = 0; page_idx < 4; page_idx++)  /* 4 pages per VLAN Member Config */
    {
        regAddr = RTL8367C_VLAN_MEMBER_CONFIGURATION_BASE + (index * 4) + page_idx;

        retVal = rtl8367c_getAsicReg(regAddr, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *tableAddr = (rtk_uint16)regData;
        tableAddr++;
    }

    _rtl8367c_VlanMCStSmi2User(smi_vlancfg, pVlanCg);
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicVlan4kEntry
 * Description:
 *      Set VID mapped entry to 4K VLAN table
 * Input:
 *      pVlan4kEntry - 4K VLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameter
 *      RT_ERR_L2_FID               - Invalid FID
 *      RT_ERR_VLAN_VID             - Invalid VID parameter (0~4095)
 *      RT_ERR_PORT_MASK            - Invalid portmask
 *      RT_ERR_FILTER_METER_ID      - Invalid meter
 *      RT_ERR_QOS_INT_PRIORITY     - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicVlan4kEntry(rtl8367c_user_vlan4kentry *pVlan4kEntry )
{
    rtk_uint16              vlan_4k_entry[RTL8367C_VLAN_4KTABLE_LEN];
    rtk_uint32                  page_idx;
    rtk_uint16                  *tableAddr;
    ret_t                   retVal;
    rtk_uint32                  regData;

    if(pVlan4kEntry->vid > RTL8367C_VIDMAX)
        return RT_ERR_VLAN_VID;

    if(pVlan4kEntry->mbr > RTL8367C_PORTMASK)
        return RT_ERR_PORT_MASK;

    if(pVlan4kEntry->untag > RTL8367C_PORTMASK)
        return RT_ERR_PORT_MASK;

    if(pVlan4kEntry->fid_msti > RTL8367C_FIDMAX)
        return RT_ERR_L2_FID;

    if(pVlan4kEntry->meteridx > RTL8367C_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(pVlan4kEntry->vbpri > RTL8367C_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    memset(vlan_4k_entry, 0x00, sizeof(rtk_uint16) * RTL8367C_VLAN_4KTABLE_LEN);
    _rtl8367c_Vlan4kStUser2Smi(pVlan4kEntry, vlan_4k_entry);

    /* Prepare Data */
    tableAddr = vlan_4k_entry;
    for(page_idx = 0; page_idx < RTL8367C_VLAN_4KTABLE_LEN; page_idx++)
    {
        regData = *tableAddr;
        retVal = rtl8367c_setAsicReg(RTL8367C_TABLE_ACCESS_WRDATA_BASE + page_idx, regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        tableAddr++;
    }

    /* Write Address (VLAN_ID) */
    regData = pVlan4kEntry->vid;
    retVal = rtl8367c_setAsicReg(RTL8367C_TABLE_ACCESS_ADDR_REG, regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    /* Write Command */
    retVal = rtl8367c_setAsicRegBits(RTL8367C_TABLE_ACCESS_CTRL_REG, RTL8367C_TABLE_TYPE_MASK | RTL8367C_COMMAND_TYPE_MASK,RTL8367C_TABLE_ACCESS_REG_DATA(TB_OP_WRITE,TB_TARGET_CVLAN));
    if(retVal != RT_ERR_OK)
        return retVal;

#if defined(CONFIG_RTL8367C_ASICDRV_TEST)
    memcpy(&Rtl8370sVirtualVlanTable[pVlan4kEntry->vid], pVlan4kEntry, sizeof(rtl8367c_user_vlan4kentry));
#endif

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicVlan4kEntry
 * Description:
 *      Get VID mapped entry to 4K VLAN table
 * Input:
 *      pVlan4kEntry - 4K VLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_VLAN_VID         - Invalid VID parameter (0~4095)
 *      RT_ERR_BUSYWAIT_TIMEOUT - LUT is busy at retrieving
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicVlan4kEntry(rtl8367c_user_vlan4kentry *pVlan4kEntry )
{
    rtk_uint16                  vlan_4k_entry[RTL8367C_VLAN_4KTABLE_LEN];
    rtk_uint32                  page_idx;
    rtk_uint16                  *tableAddr;
    ret_t                       retVal;
    rtk_uint32                  regData;
    rtk_uint32                  busyCounter;

    if(pVlan4kEntry->vid > RTL8367C_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* Polling status */
    busyCounter = RTL8367C_VLAN_BUSY_CHECK_NO;
    while(busyCounter)
    {
        retVal = rtl8367c_getAsicRegBit(RTL8367C_TABLE_ACCESS_STATUS_REG, RTL8367C_TABLE_LUT_ADDR_BUSY_FLAG_OFFSET,&regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        if(regData == 0)
            break;

        busyCounter --;
        if(busyCounter == 0)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }

    /* Write Address (VLAN_ID) */
    regData = pVlan4kEntry->vid;
    retVal = rtl8367c_setAsicReg(RTL8367C_TABLE_ACCESS_ADDR_REG, regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    /* Read Command */
    retVal = rtl8367c_setAsicRegBits(RTL8367C_TABLE_ACCESS_CTRL_REG, RTL8367C_TABLE_TYPE_MASK | RTL8367C_COMMAND_TYPE_MASK, RTL8367C_TABLE_ACCESS_REG_DATA(TB_OP_READ,TB_TARGET_CVLAN));
    if(retVal != RT_ERR_OK)
        return retVal;

    /* Polling status */
    busyCounter = RTL8367C_VLAN_BUSY_CHECK_NO;
    while(busyCounter)
    {
        retVal = rtl8367c_getAsicRegBit(RTL8367C_TABLE_ACCESS_STATUS_REG, RTL8367C_TABLE_LUT_ADDR_BUSY_FLAG_OFFSET,&regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        if(regData == 0)
            break;

        busyCounter --;
        if(busyCounter == 0)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }

    /* Read VLAN data from register */
    tableAddr = vlan_4k_entry;
    for(page_idx = 0; page_idx < RTL8367C_VLAN_4KTABLE_LEN; page_idx++)
    {
        retVal = rtl8367c_getAsicReg(RTL8367C_TABLE_ACCESS_RDDATA_BASE + page_idx, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *tableAddr = regData;
        tableAddr++;
    }

    _rtl8367c_Vlan4kStSmi2User(vlan_4k_entry, pVlan4kEntry);

#if defined(CONFIG_RTL8367C_ASICDRV_TEST)
    memcpy(pVlan4kEntry, &Rtl8370sVirtualVlanTable[pVlan4kEntry->vid], sizeof(rtl8367c_user_vlan4kentry));
#endif

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicVlanAccpetFrameType
 * Description:
 *      Set per-port acceptable frame type
 * Input:
 *      port        - Physical port number (0~10)
 *      frameType   - The acceptable frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - Success
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_PORT_ID                  - Invalid port number
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE   - Invalid frame type
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicVlanAccpetFrameType(rtk_uint32 port, rtl8367c_accframetype frameType)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(frameType >= FRAME_TYPE_MAX_BOUND)
        return RT_ERR_VLAN_ACCEPT_FRAME_TYPE;

    return rtl8367c_setAsicRegBits(RTL8367C_VLAN_ACCEPT_FRAME_TYPE_REG(port), RTL8367C_VLAN_ACCEPT_FRAME_TYPE_MASK(port), frameType);
}
/* Function Name:
 *      rtl8367c_getAsicVlanAccpetFrameType
 * Description:
 *      Get per-port acceptable frame type
 * Input:
 *      port        - Physical port number (0~10)
 *      pFrameType  - The acceptable frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - Success
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_PORT_ID                  - Invalid port number
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE   - Invalid frame type
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicVlanAccpetFrameType(rtk_uint32 port, rtl8367c_accframetype *pFrameType)
{
    rtk_uint32 regData;
    ret_t  retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8367c_getAsicRegBits(RTL8367C_VLAN_ACCEPT_FRAME_TYPE_REG(port), RTL8367C_VLAN_ACCEPT_FRAME_TYPE_MASK(port), &regData)) != RT_ERR_OK)
        return retVal;

    *pFrameType = (rtl8367c_accframetype)regData;
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicVlanIngressFilter
 * Description:
 *      Set VLAN Ingress Filter
 * Input:
 *      port        - Physical port number (0~10)
 *      enabled     - Enable or disable Ingress filter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicVlanIngressFilter(rtk_uint32 port, rtk_uint32 enabled)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367c_setAsicRegBit(RTL8367C_VLAN_INGRESS_REG, port, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicVlanIngressFilter
 * Description:
 *      Get VLAN Ingress Filter
 * Input:
 *      port        - Physical port number (0~10)
 *      pEnable     - Enable or disable Ingress filter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicVlanIngressFilter(rtk_uint32 port, rtk_uint32 *pEnable)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367c_getAsicRegBit(RTL8367C_VLAN_INGRESS_REG, port, pEnable);
}
/* Function Name:
 *      rtl8367c_setAsicVlanEgressTagMode
 * Description:
 *      Set CVLAN egress tag mode
 * Input:
 *      port        - Physical port number (0~10)
 *      tagMode     - The egress tag mode. Including Original mode, Keep tag mode and Priority tag mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_INPUT    - Invalid input parameter
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicVlanEgressTagMode(rtk_uint32 port, rtl8367c_egtagmode tagMode)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(tagMode >= EG_TAG_MODE_END)
        return RT_ERR_INPUT;

    return rtl8367c_setAsicRegBits(RTL8367C_PORT_MISC_CFG_REG(port), RTL8367C_VLAN_EGRESS_MDOE_MASK, tagMode);
}
/* Function Name:
 *      rtl8367c_getAsicVlanEgressTagMode
 * Description:
 *      Get CVLAN egress tag mode
 * Input:
 *      port        - Physical port number (0~10)
 *      pTagMode    - The egress tag mode. Including Original mode, Keep tag mode and Priority tag mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicVlanEgressTagMode(rtk_uint32 port, rtl8367c_egtagmode *pTagMode)
{
    rtk_uint32 regData;
    ret_t  retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8367c_getAsicRegBits(RTL8367C_PORT_MISC_CFG_REG(port), RTL8367C_VLAN_EGRESS_MDOE_MASK, &regData)) != RT_ERR_OK)
        return retVal;

    *pTagMode = (rtl8367c_egtagmode)regData;
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicVlanPortBasedVID
 * Description:
 *      Set port based VID which is indexed to 32 VLAN member configurations
 * Input:
 *      port    - Physical port number (0~10)
 *      index   - Index to VLAN member configuration
 *      pri     - 1Q Port based VLAN priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number
 *      RT_ERR_QOS_INT_PRIORITY     - Invalid priority
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - Invalid VLAN member configuration index
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicVlanPortBasedVID(rtk_uint32 port, rtk_uint32 index, rtk_uint32 pri)
{
    rtk_uint32 regAddr, bit_mask;
    ret_t  retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(index > RTL8367C_CVIDXMAX)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;

    if(pri > RTL8367C_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    regAddr = RTL8367C_VLAN_PVID_CTRL_REG(port);
    bit_mask = RTL8367C_PORT_VIDX_MASK(port);
    retVal = rtl8367c_setAsicRegBits(regAddr, bit_mask, index);
    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8367C_VLAN_PORTBASED_PRIORITY_REG(port);
    bit_mask = RTL8367C_VLAN_PORTBASED_PRIORITY_MASK(port);
    retVal = rtl8367c_setAsicRegBits(regAddr, bit_mask, pri);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicVlanPortBasedVID
 * Description:
 *      Get port based VID which is indexed to 32 VLAN member configurations
 * Input:
 *      port    - Physical port number (0~10)
 *      pIndex  - Index to VLAN member configuration
 *      pPri    - 1Q Port based VLAN priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicVlanPortBasedVID(rtk_uint32 port, rtk_uint32 *pIndex, rtk_uint32 *pPri)
{
    rtk_uint32 regAddr,bit_mask;
    ret_t  retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    regAddr = RTL8367C_VLAN_PVID_CTRL_REG(port);
    bit_mask = RTL8367C_PORT_VIDX_MASK(port);
    retVal = rtl8367c_getAsicRegBits(regAddr, bit_mask, pIndex);
    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8367C_VLAN_PORTBASED_PRIORITY_REG(port);
    bit_mask = RTL8367C_VLAN_PORTBASED_PRIORITY_MASK(port);
    retVal = rtl8367c_getAsicRegBits(regAddr, bit_mask, pPri);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicVlanProtocolBasedGroupData
 * Description:
 *      Set protocol and port based group database
 * Input:
 *      index       - Index to VLAN member configuration
 *      pPbCfg  - Protocol and port based group database entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameter
 *      RT_ERR_VLAN_PROTO_AND_PORT  - Invalid protocol base group database index
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicVlanProtocolBasedGroupData(rtk_uint32 index, rtl8367c_protocolgdatacfg *pPbCfg)
{
    rtk_uint32  frameType;
    rtk_uint32  etherType;
    ret_t   retVal;

    /* Error Checking */
    if(index > RTL8367C_PROTOVLAN_GIDX_MAX)
        return RT_ERR_VLAN_PROTO_AND_PORT;

    if(pPbCfg->frameType >= PPVLAN_FRAME_TYPE_END )
        return RT_ERR_INPUT;

    frameType = pPbCfg->frameType;
    etherType = pPbCfg->etherType;

    /* Frame type */
    retVal = rtl8367c_setAsicRegBits(RTL8367C_VLAN_PPB_FRAMETYPE_REG(index), RTL8367C_VLAN_PPB_FRAMETYPE_MASK, frameType);
    if(retVal != RT_ERR_OK)
        return retVal;

    /* Ether type */
    retVal = rtl8367c_setAsicReg(RTL8367C_VLAN_PPB_ETHERTYPR_REG(index), etherType);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicVlanProtocolBasedGroupData
 * Description:
 *      Get protocol and port based group database
 * Input:
 *      index       - Index to VLAN member configuration
 *      pPbCfg  - Protocol and port based group database entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameter
 *      RT_ERR_VLAN_PROTO_AND_PORT  - Invalid protocol base group database index
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicVlanProtocolBasedGroupData(rtk_uint32 index, rtl8367c_protocolgdatacfg *pPbCfg)
{
    rtk_uint32  frameType;
    rtk_uint32  etherType;
    ret_t   retVal;

    /* Error Checking */
    if(index > RTL8367C_PROTOVLAN_GIDX_MAX)
        return RT_ERR_VLAN_PROTO_AND_PORT;

    /* Read Frame type */
    retVal = rtl8367c_getAsicRegBits(RTL8367C_VLAN_PPB_FRAMETYPE_REG(index), RTL8367C_VLAN_PPB_FRAMETYPE_MASK, &frameType);
    if(retVal != RT_ERR_OK)
        return retVal;

    /* Read Ether type */
    retVal = rtl8367c_getAsicReg(RTL8367C_VLAN_PPB_ETHERTYPR_REG(index), &etherType);
    if(retVal != RT_ERR_OK)
        return retVal;


    pPbCfg->frameType = frameType;
    pPbCfg->etherType = etherType;
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicVlanPortAndProtocolBased
 * Description:
 *      Set protocol and port based VLAN configuration
 * Input:
 *      port        - Physical port number (0~10)
 *      index       - Index of protocol and port based database index
 *      pPpbCfg     - Protocol and port based VLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameter
 *      RT_ERR_PORT_ID              - Invalid port number
 *      RT_ERR_QOS_INT_PRIORITY     - Invalid priority
 *      RT_ERR_VLAN_PROTO_AND_PORT  - Invalid protocol base group database index
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - Invalid VLAN member configuration index
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicVlanPortAndProtocolBased(rtk_uint32 port, rtk_uint32 index, rtl8367c_protocolvlancfg *pPpbCfg)
{
    rtk_uint32  reg_addr, bit_mask, bit_value;
    ret_t   retVal;

    /* Error Checking */
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(index > RTL8367C_PROTOVLAN_GIDX_MAX)
        return RT_ERR_VLAN_PROTO_AND_PORT;

    if( (pPpbCfg->valid != FALSE) && (pPpbCfg->valid != TRUE) )
        return RT_ERR_INPUT;

    if(pPpbCfg->vlan_idx > RTL8367C_CVIDXMAX)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;

    if(pPpbCfg->priority > RTL8367C_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    /* Valid bit */
    reg_addr  = RTL8367C_VLAN_PPB_VALID_REG(index);
    bit_mask  = 0x0001 << port;
    bit_value = ((TRUE == pPpbCfg->valid) ? 0x1 : 0x0);
    retVal    = rtl8367c_setAsicRegBits(reg_addr, bit_mask, bit_value);
    if(retVal != RT_ERR_OK)
        return retVal;

    /* Calculate the actual register address for CVLAN index*/
    if(port < 8)
    {
        reg_addr = RTL8367C_VLAN_PPB_CTRL_REG(index, port);
        bit_mask = RTL8367C_VLAN_PPB_CTRL_MASK(port);
    }
    else if(port == 8)
    {
        reg_addr = RTL8367C_REG_VLAN_PPB0_CTRL4;
        bit_mask = RTL8367C_VLAN_PPB0_CTRL4_PORT8_INDEX_MASK;
    }
    else if(port == 9)
    {
        reg_addr = RTL8367C_REG_VLAN_PPB0_CTRL4;
        bit_mask = RTL8367C_VLAN_PPB0_CTRL4_PORT9_INDEX_MASK;
    }
    else if(port == 10)
    {
        reg_addr = RTL8367C_REG_VLAN_PPB0_CTRL4;
        bit_mask = RTL8367C_VLAN_PPB0_CTRL4_PORT10_INDEX_MASK;
    }

    bit_value = pPpbCfg->vlan_idx;
    retVal  = rtl8367c_setAsicRegBits(reg_addr, bit_mask, bit_value);
    if(retVal != RT_ERR_OK)
        return retVal;

    /* write priority */
    reg_addr  = RTL8367C_VLAN_PPB_PRIORITY_ITEM_REG(port, index);
    bit_mask  = RTL8367C_VLAN_PPB_PRIORITY_ITEM_MASK(port);
    bit_value = pPpbCfg->priority;
    retVal    = rtl8367c_setAsicRegBits(reg_addr, bit_mask, bit_value);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicVlanPortAndProtocolBased
 * Description:
 *      Get protocol and port based VLAN configuration
 * Input:
 *      port        - Physical port number (0~7)
 *      index       - Index of protocol and port based database index
 *      pPpbCfg     - Protocol and port based VLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameter
 *      RT_ERR_PORT_ID              - Invalid port number
 *      RT_ERR_VLAN_PROTO_AND_PORT  - Invalid protocol base group database index
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicVlanPortAndProtocolBased(rtk_uint32 port, rtk_uint32 index, rtl8367c_protocolvlancfg *pPpbCfg)
{
    rtk_uint32  reg_addr, bit_mask, bit_value;
    ret_t   retVal;

    /* Error Checking */
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(index > RTL8367C_PROTOVLAN_GIDX_MAX)
        return RT_ERR_VLAN_PROTO_AND_PORT;

    if(pPpbCfg == NULL)
        return RT_ERR_INPUT;

    /* Valid bit */
    reg_addr  = RTL8367C_VLAN_PPB_VALID_REG(index);
    bit_mask  = 0x0001 << port;
    retVal    = rtl8367c_getAsicRegBits(reg_addr, bit_mask, &bit_value);
    if(retVal != RT_ERR_OK)
        return retVal;

    pPpbCfg->valid = bit_value;

    /* CVLAN index */
    if(port < 8)
    {
        reg_addr = RTL8367C_VLAN_PPB_CTRL_REG(index, port);
        bit_mask = RTL8367C_VLAN_PPB_CTRL_MASK(port);
    }
    else if(port == 8)
    {
        reg_addr = RTL8367C_REG_VLAN_PPB0_CTRL4;
        bit_mask = RTL8367C_VLAN_PPB0_CTRL4_PORT8_INDEX_MASK;
    }
    else if(port == 9)
    {
        reg_addr = RTL8367C_REG_VLAN_PPB0_CTRL4;
        bit_mask = RTL8367C_VLAN_PPB0_CTRL4_PORT9_INDEX_MASK;
    }
    else if(port == 10)
    {
        reg_addr = RTL8367C_REG_VLAN_PPB0_CTRL4;
        bit_mask = RTL8367C_VLAN_PPB0_CTRL4_PORT10_INDEX_MASK;
    }

    retVal = rtl8367c_getAsicRegBits(reg_addr, bit_mask, &bit_value);
    if(retVal != RT_ERR_OK)
        return retVal;

    pPpbCfg->vlan_idx = bit_value;


    /* priority */
    reg_addr = RTL8367C_VLAN_PPB_PRIORITY_ITEM_REG(port,index);
    bit_mask = RTL8367C_VLAN_PPB_PRIORITY_ITEM_MASK(port);
    retVal = rtl8367c_getAsicRegBits(reg_addr, bit_mask, &bit_value);
    if(retVal != RT_ERR_OK)
        return retVal;

    pPpbCfg->priority = bit_value;
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicVlanFilter
 * Description:
 *      Set enable CVLAN filtering function
 * Input:
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicVlanFilter(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_VLAN_CTRL, RTL8367C_VLAN_CTRL_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicVlanFilter
 * Description:
 *      Get enable CVLAN filtering function
 * Input:
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicVlanFilter(rtk_uint32* pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_VLAN_CTRL, RTL8367C_VLAN_CTRL_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367c_setAsicVlanUntagDscpPriorityEn
 * Description:
 *      Set enable Dscp to untag 1Q priority
 * Input:
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicVlanUntagDscpPriorityEn(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_UNTAG_DSCP_PRI_CFG, RTL8367C_UNTAG_DSCP_PRI_CFG_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicVlanUntagDscpPriorityEn
 * Description:
 *      Get enable Dscp to untag 1Q priority
 * Input:
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicVlanUntagDscpPriorityEn(rtk_uint32* enabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_UNTAG_DSCP_PRI_CFG, RTL8367C_UNTAG_DSCP_PRI_CFG_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_setAsicPortBasedFid
 * Description:
 *      Set port based FID
 * Input:
 *      port    - Physical port number (0~10)
 *      fid     - Port based fid
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_L2_FID   - Invalid FID
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortBasedFid(rtk_uint32 port, rtk_uint32 fid)
{
    rtk_uint32  reg_addr;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(fid > RTL8367C_FIDMAX)
        return RT_ERR_L2_FID;

    if(port < 8)
        return rtl8367c_setAsicReg(RTL8367C_PORT_PBFID_REG(port),fid);
    else {
        reg_addr = RTL8367C_REG_PORT8_PBFID + port-8;
        return rtl8367c_setAsicReg(reg_addr, fid);
    }

}
/* Function Name:
 *      rtl8367c_getAsicPortBasedFid
 * Description:
 *      Get port based FID
 * Input:
 *      port    - Physical port number (0~7)
 *      pFid    - Port based fid
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortBasedFid(rtk_uint32 port, rtk_uint32* pFid)
{
    rtk_uint32  reg_addr;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 8)
        return rtl8367c_getAsicReg(RTL8367C_PORT_PBFID_REG(port), pFid);
    else{
        reg_addr = RTL8367C_REG_PORT8_PBFID + port-8;
        return rtl8367c_getAsicReg(reg_addr, pFid);
    }
}
/* Function Name:
 *      rtl8367c_setAsicPortBasedFidEn
 * Description:
 *      Set port based FID selection enable
 * Input:
 *      port    - Physical port number (0~10)
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortBasedFidEn(rtk_uint32 port, rtk_uint32 enabled)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367c_setAsicRegBit(RTL8367C_REG_PORT_PBFIDEN,port, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicPortBasedFidEn
 * Description:
 *      Get port based FID selection enable
 * Input:
 *      port    - Physical port number (0~10)
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortBasedFidEn(rtk_uint32 port, rtk_uint32* pEnabled)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367c_getAsicRegBit(RTL8367C_REG_PORT_PBFIDEN,port, pEnabled);
}
/* Function Name:
 *      rtl8367c_setAsicSpanningTreeStatus
 * Description:
 *      Set spanning tree state per each port
 * Input:
 *      port    - Physical port number (0~10)
 *      msti    - Multiple spanning tree instance
 *      state   - Spanning tree state for msti
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_MSTI         - Invalid msti parameter
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_MSTP_STATE   - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicSpanningTreeStatus(rtk_uint32 port, rtk_uint32 msti, rtk_uint32 state)
{
    rtk_uint32  reg_addr,bits_msk;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(msti > RTL8367C_MSTIMAX)
        return RT_ERR_MSTI;

    if(state > STPST_FORWARDING)
        return RT_ERR_MSTP_STATE;

    if(port < 8)
        return rtl8367c_setAsicRegBits(RTL8367C_VLAN_MSTI_REG(msti,port), RTL8367C_VLAN_MSTI_MASK(port),state);
    else{
        reg_addr = RTL8367C_VLAN_MSTI_REG(msti,port);
        switch(port){
            case 8: bits_msk = RTL8367C_VLAN_MSTI0_CTRL1_PORT8_STATE_MASK;break;
            case 9: bits_msk = RTL8367C_VLAN_MSTI0_CTRL1_PORT9_STATE_MASK;break;
            case 10: bits_msk = RTL8367C_VLAN_MSTI0_CTRL1_PORT10_STATE_MASK;break;
        }
        return rtl8367c_setAsicRegBits(reg_addr, bits_msk,state);
    }
}
/* Function Name:
 *      rtl8367c_getAsicSpanningTreeStatus
 * Description:
 *      Set spanning tree state per each port
 * Input:
 *      port    - Physical port number (0~10)
 *      msti    - Multiple spanning tree instance
 *      pState  - Spanning tree state for msti
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_MSTI         - Invalid msti parameter
 *      RT_ERR_PORT_ID      - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicSpanningTreeStatus(rtk_uint32 port, rtk_uint32 msti, rtk_uint32* pState)
{
    rtk_uint32  reg_addr,bits_msk;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(msti > RTL8367C_MSTIMAX)
        return RT_ERR_MSTI;

    if(port < 8)
        return rtl8367c_getAsicRegBits(RTL8367C_VLAN_MSTI_REG(msti,port), RTL8367C_VLAN_MSTI_MASK(port), pState);
    else{
        reg_addr = RTL8367C_VLAN_MSTI_REG(msti,port);
        switch(port){
            case 8: bits_msk = RTL8367C_VLAN_MSTI0_CTRL1_PORT8_STATE_MASK;break;
            case 9: bits_msk = RTL8367C_VLAN_MSTI0_CTRL1_PORT9_STATE_MASK;break;
            case 10: bits_msk = RTL8367C_VLAN_MSTI0_CTRL1_PORT10_STATE_MASK;break;
        }
        return rtl8367c_getAsicRegBits(reg_addr, bits_msk, pState);
    }

}

/* Function Name:
 *      rtl8367c_setAsicVlanTransparent
 * Description:
 *      Set VLAN transparent
 * Input:
 *      port        - Physical port number (0~10)
 *      portmask    - portmask(0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid portmask
 *      RT_ERR_PORT_ID      - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicVlanTransparent(rtk_uint32 port, rtk_uint32 portmask)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(portmask > RTL8367C_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_VLAN_EGRESS_TRANS_CTRL0 + port, RTL8367C_VLAN_EGRESS_TRANS_CTRL0_MASK, portmask);
}

/* Function Name:
 *      rtl8367c_getAsicVlanTransparent
 * Description:
 *      Get VLAN transparent
 * Input:
 *      port        - Physical port number (0~10)
 * Output:
 *      pPortmask   - Ingress port mask
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid portmask
 *      RT_ERR_PORT_ID      - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicVlanTransparent(rtk_uint32 port, rtk_uint32 *pPortmask)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367c_getAsicRegBits(RTL8367C_REG_VLAN_EGRESS_TRANS_CTRL0 + port, RTL8367C_VLAN_EGRESS_TRANS_CTRL0_MASK, pPortmask);
}

/* Function Name:
 *      rtl8367c_setAsicVlanEgressKeep
 * Description:
 *      Set per egress port VLAN keep mode
 * Input:
 *      port        - Physical port number (0~10)
 *      portmask    - portmask(0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid portmask
 *      RT_ERR_PORT_ID      - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicVlanEgressKeep(rtk_uint32 port, rtk_uint32 portmask)
{
    rtk_uint32 regAddr, bit_mask;
    ret_t  retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(portmask > RTL8367C_PORTMASK)
        return RT_ERR_PORT_MASK;

    if(port < 8){
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL0 + (port>>1),RTL8367C_PORT0_VLAN_KEEP_MASK_MASK<<((port&1)*8),portmask & 0xff);
        if(retVal != RT_ERR_OK)
            return retVal;
        regAddr = RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL0_EXT + (port>>1);
        bit_mask = RTL8367C_PORT0_VLAN_KEEP_MASK_EXT_MASK;
        bit_mask <<= (port&1)*3;
        retVal = rtl8367c_setAsicRegBits(regAddr, bit_mask, (portmask>>8)&0x7);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else{
        switch(port){
            case 8:
                regAddr = RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL4;
                bit_mask = RTL8367C_PORT8_VLAN_KEEP_MASK_MASK;
                retVal = rtl8367c_setAsicRegBits(regAddr, bit_mask, portmask & 0xff);
                if(retVal != RT_ERR_OK)
                    return retVal;
                regAddr = RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL4_EXT;
                bit_mask = RTL8367C_PORT8_VLAN_KEEP_MASK_EXT_MASK;
                retVal = rtl8367c_setAsicRegBits(regAddr, bit_mask, (portmask>>8)&0x7);
                if(retVal != RT_ERR_OK)
                    return retVal;
                break;

            case 9:
                regAddr = RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL4;
                bit_mask = RTL8367C_PORT9_VLAN_KEEP_MASK_MASK;
                retVal = rtl8367c_setAsicRegBits(regAddr, bit_mask, portmask & 0xff);
                if(retVal != RT_ERR_OK)
                    return retVal;
                regAddr = RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL4_EXT;
                bit_mask = RTL8367C_PORT9_VLAN_KEEP_MASK_EXT_MASK;
                retVal = rtl8367c_setAsicRegBits(regAddr, bit_mask, (portmask>>8)&0x7);
                if(retVal != RT_ERR_OK)
                    return retVal;
                break;

            case 10:
                regAddr = RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL5;
                bit_mask = RTL8367C_VLAN_EGRESS_KEEP_CTRL5_MASK;
                retVal = rtl8367c_setAsicRegBits(regAddr, bit_mask, portmask & 0xff);
                if(retVal != RT_ERR_OK)
                    return retVal;
                regAddr = RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL5_EXT;
                bit_mask = RTL8367C_VLAN_EGRESS_KEEP_CTRL5_EXT_MASK;
                retVal = rtl8367c_setAsicRegBits(regAddr, bit_mask, (portmask>>8)&0x7);
                if(retVal != RT_ERR_OK)
                    return retVal;
                break;
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicVlanEgressKeep
 * Description:
 *      Get per egress port VLAN keep mode
 * Input:
 *      port        - Physical port number (0~7)
 *      pPortmask   - portmask(0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicVlanEgressKeep(rtk_uint32 port, rtk_uint32* pPortmask)
{
    rtk_uint32 regAddr, bit_mask, regval_l, regval_h;
    ret_t  retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 8){
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL0 + (port>>1),RTL8367C_PORT0_VLAN_KEEP_MASK_MASK<<((port&1)*8),&regval_l);
        if(retVal != RT_ERR_OK)
            return retVal;
        regAddr = RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL0_EXT + (port>>1);
        bit_mask = RTL8367C_PORT0_VLAN_KEEP_MASK_EXT_MASK;
        bit_mask <<= (port&1)*3;
        retVal = rtl8367c_getAsicRegBits(regAddr, bit_mask, &regval_h);
        if(retVal != RT_ERR_OK)
            return retVal;
        *pPortmask = (regval_h << 8) | regval_l;
    }
    else{
        switch(port){
            case 8:
                regAddr = RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL4;
                bit_mask = RTL8367C_PORT8_VLAN_KEEP_MASK_MASK;
                retVal = rtl8367c_getAsicRegBits(regAddr, bit_mask, &regval_l);
                if(retVal != RT_ERR_OK)
                    return retVal;
                regAddr = RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL4_EXT;
                bit_mask = RTL8367C_PORT8_VLAN_KEEP_MASK_EXT_MASK;
                retVal = rtl8367c_getAsicRegBits(regAddr, bit_mask, &regval_h);
                if(retVal != RT_ERR_OK)
                    return retVal;

                *pPortmask = (regval_h << 8) | regval_l;
                break;

            case 9:
                regAddr = RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL4;
                bit_mask = RTL8367C_PORT9_VLAN_KEEP_MASK_MASK;
                retVal = rtl8367c_getAsicRegBits(regAddr, bit_mask, &regval_l);
                if(retVal != RT_ERR_OK)
                    return retVal;
                regAddr = RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL4_EXT;
                bit_mask = RTL8367C_PORT9_VLAN_KEEP_MASK_EXT_MASK;
                retVal = rtl8367c_getAsicRegBits(regAddr, bit_mask, &regval_h);
                if(retVal != RT_ERR_OK)
                    return retVal;

                *pPortmask = (regval_h << 8) | regval_l;
                break;

            case 10:
                regAddr = RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL5;
                bit_mask = RTL8367C_VLAN_EGRESS_KEEP_CTRL5_MASK;
                retVal = rtl8367c_getAsicRegBits(regAddr, bit_mask, &regval_l);
                if(retVal != RT_ERR_OK)
                    return retVal;
                regAddr = RTL8367C_REG_VLAN_EGRESS_KEEP_CTRL5_EXT;
                bit_mask = RTL8367C_VLAN_EGRESS_KEEP_CTRL5_EXT_MASK;
                retVal = rtl8367c_getAsicRegBits(regAddr, bit_mask, &regval_h);
                if(retVal != RT_ERR_OK)
                    return retVal;

                *pPortmask = (regval_h << 8) | regval_l;
                break;
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setReservedVidAction
 * Description:
 *      Set reserved VID action
 * Input:
 *      vid0Action      - VID 0 action
 *      vid4095Action   - VID 4095 action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_INPUT    - Error input
 * Note:
 *      None
 */
ret_t rtl8367c_setReservedVidAction(rtk_uint32 vid0Action, rtk_uint32 vid4095Action)
{
    ret_t   retVal;

    if(vid0Action >= RES_VID_ACT_END)
        return RT_ERR_INPUT;

    if(vid4095Action >= RES_VID_ACT_END)
        return RT_ERR_INPUT;

    if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_VLAN_EXT_CTRL, RTL8367C_VLAN_VID0_TYPE_OFFSET, vid0Action)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_VLAN_EXT_CTRL, RTL8367C_VLAN_VID4095_TYPE_OFFSET, vid4095Action)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getReservedVidAction
 * Description:
 *      Get reserved VID action
 * Input:
 *      pVid0Action     - VID 0 action
 *      pVid4095Action  - VID 4095 action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *      None
 */
ret_t rtl8367c_getReservedVidAction(rtk_uint32 *pVid0Action, rtk_uint32 *pVid4095Action)
{
    ret_t   retVal;

    if(pVid0Action == NULL)
        return RT_ERR_NULL_POINTER;

    if(pVid4095Action == NULL)
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_VLAN_EXT_CTRL, RTL8367C_VLAN_VID0_TYPE_OFFSET, pVid0Action)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_VLAN_EXT_CTRL, RTL8367C_VLAN_VID4095_TYPE_OFFSET, pVid4095Action)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}

/* Function Name:
 *      rtl8367c_setRealKeepRemarkEn
 * Description:
 *      Set Real Keep Remark
 * Input:
 *      enabled         - 0: 1P remarking is forbidden at real keep packet, 1: 1P remarking is enabled at real keep packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_INPUT    - Error input
 * Note:
 *      None
 */
ret_t rtl8367c_setRealKeepRemarkEn(rtk_uint32 enabled)
{
    ret_t   retVal;

    if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_VLAN_EXT_CTRL, RTL8367C_VLAN_1P_REMARK_BYPASS_REALKEEP_OFFSET, enabled)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getRealKeepRemarkEn
 * Description:
 *      Get Real Keep Remark
 * Input:
 *      None
 * Output:
 *      pEnabled        - 0: 1P remarking is forbidden at real keep packet, 1: 1P remarking is enabled at real keep packet
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_INPUT    - Error input
 * Note:
 *      None
 */
ret_t rtl8367c_getRealKeepRemarkEn(rtk_uint32 *pEnabled)
{
    ret_t   retVal;

    if((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_VLAN_EXT_CTRL, RTL8367C_VLAN_1P_REMARK_BYPASS_REALKEEP_OFFSET, pEnabled)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_resetVlan
 * Description:
 *      Reset VLAN table
 * Input:
 *      None.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_resetVlan(void)
{
    ret_t   retVal;

    if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_VLAN_EXT_CTRL2, RTL8367C_VLAN_EXT_CTRL2_OFFSET, 1)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

