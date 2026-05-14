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
 * $Revision$
 * $Date$
 *
 * Purpose : RTK switch high-level API for RTL8373
 * Feature : Here is a list of all functions and variables in VLAN module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8373/dal_rtl8373_vlan.h>
#include <dal/rtl8373/rtl8373_asicdrv.h>
#include <string.h>


#if defined(CONFIG_RTL8373_ASICDRV_TEST)
dal_rtl8373_user_vlan4kentry Rtl8371cVirtualVlanTable[RTL8373_VIDMAX + 1];
#endif

static void _dal_rtl8373_Vlan4kStUser2Smi(dal_rtl8373_user_vlan4kentry *pUserVlan4kEntry, rtk_uint32 *pSmiVlan4kEntry)
{
    rtk_uint32 regValue = 0;
    regValue |= (pUserVlan4kEntry->mbr & 0x3FF);
    regValue |= (pUserVlan4kEntry->untag & 0x3FF) << 10;
    regValue |= (pUserVlan4kEntry->fid_msti & 0xF) << 20;
    regValue |= (pUserVlan4kEntry->svlan_chk_ivl_svl & 0x1) << 24;
    regValue |= (pUserVlan4kEntry->ivl_svl & 0x1) << 25;
    
    *pSmiVlan4kEntry = regValue;
}

static void _dal_rtl8373_Vlan4kStSmi2User(rtk_uint32 smiVlan4kEntry, dal_rtl8373_user_vlan4kentry *pUserVlan4kEntry)
{
    pUserVlan4kEntry->mbr               = (rtk_uint16) (smiVlan4kEntry& 0x3FF);
    pUserVlan4kEntry->untag             = (rtk_uint16) ((smiVlan4kEntry >> 10) & 0x3FF);
    pUserVlan4kEntry->fid_msti          = (rtk_uint16) ((smiVlan4kEntry >> 20) & 0xF);
    pUserVlan4kEntry->svlan_chk_ivl_svl = (rtk_uint16) ((smiVlan4kEntry >> 24) & 0x1) ;
    pUserVlan4kEntry->ivl_svl           = (rtk_uint16) ((smiVlan4kEntry >> 25) & 0x1) ;
}

ret_t _dal_rtl8373_setAsicVlan4kEntry(dal_rtl8373_user_vlan4kentry *pVlan4kEntry )
{
    rtk_uint32  vlanEntryVal = 0;
    rtk_uint32  retVal = 0;
    rtk_uint32  regData = 0;
  
    if(pVlan4kEntry->vid > RTL8373_VIDMAX)
        return RT_ERR_VLAN_VID;

    if(pVlan4kEntry->mbr > RTL8373_PORTMASK)
        return RT_ERR_PORT_MASK;

    if(pVlan4kEntry->untag > RTL8373_PORTMASK)
        return RT_ERR_PORT_MASK;

    if(pVlan4kEntry->fid_msti > RTL8373_FIDMAX)
        return RT_ERR_L2_FID;

    if(pVlan4kEntry->ivl_svl> RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if(pVlan4kEntry->svlan_chk_ivl_svl> RTK_ENABLE_END)
        return RT_ERR_INPUT;

    /* Prepare Data */
    _dal_rtl8373_Vlan4kStUser2Smi(pVlan4kEntry, &vlanEntryVal);

    retVal = rtl8373_setAsicReg(RTL8373_ITA_WRITE_DATA0_ADDR (0), vlanEntryVal);
    if(retVal != RT_ERR_OK)
        return retVal;

    /*write control word*/
    regData = (pVlan4kEntry->vid) << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET;
    regData |= (TB_TARGET_CVLAN << RTL8373_ITA_CTRL0_TLB_TYPE_OFFSET);
    regData |= (TB_OP_WRITE << RTL8373_ITA_CTRL0_TLB_ACT_OFFSET);
    regData |= (TB_EXECUTE << RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET);
    #if 0
    /* Write Address (VLAN_ID) */
    regData = pVlan4kEntry->vid;
    regAddr = RTL8373_ITA_CTRL0_ADDR;
    retVal = rtl8373_setAsicRegBits(regAddr,RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET, regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    /* Write Command */
    /* Write ACS_CMD register */
    retVal = rtl8373_setAsicRegBit(regAddr, RTL8373_ITA_CTRL0_TLB_ACT_OFFSET, TB_OP_WRITE);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(regAddr, RTL8373_ITA_CTRL0_TLB_TYPE_MASK, TB_TARGET_CVLAN);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(regAddr, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET , TB_EXECUTE);
    if(retVal != RT_ERR_OK)
        return retVal;
#endif
    retVal = rtl8373_setAsicReg(RTL8373_ITA_CTRL0_ADDR, regData);
    if(retVal != RT_ERR_OK)
        return retVal;


    /*wait access finished */
    do{
        retVal = rtl8373_getAsicReg(RTL8373_ITA_CTRL0_ADDR, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;
    }while(regData & 0x1);

#if defined(CONFIG_RTL8373_ASICDRV_TEST)
    memcpy(&Rtl8367dVirtualVlanTable[pVlan4kEntry->vid], pVlan4kEntry, sizeof(dal_rtl8373_user_vlan4kentry));
#endif

    return RT_ERR_OK;
}


ret_t _dal_rtl8373_getAsicVlan4kEntry(dal_rtl8373_user_vlan4kentry *pVlan4kEntry )
{
    rtk_uint32  retVal = 0;
    rtk_uint32  regData = 0;
    rtk_uint32  regAddr = 0;
    rtk_uint32  busyCounter = 0;

    if(pVlan4kEntry->vid > RTL8373_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* Polling status */
    busyCounter = RTL8373_VLAN_BUSY_CHECK_NO;
    regAddr = RTL8373_ITA_CTRL0_ADDR;
    do{
        retVal = rtl8373_getAsicRegBit(regAddr, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET , &regData);
        if(retVal != RT_ERR_OK)
            return retVal;
        busyCounter --;
        if(busyCounter == 0)
            return RT_ERR_BUSYWAIT_TIMEOUT;

    }while(regData);
    
     /* Write Address (VLAN_ID) */
     regAddr = RTL8373_ITA_CTRL0_ADDR;
     regData = pVlan4kEntry->vid;
     retVal = rtl8373_setAsicRegBits(regAddr, RTL8373_ITA_CTRL0_TBL_ADDR_MASK, regData);
     if(retVal != RT_ERR_OK)
         return retVal;
    
     /* Read Command */
     /* Write ACS_CMD register */
     retVal = rtl8373_setAsicRegBit(regAddr, RTL8373_ITA_CTRL0_TLB_ACT_OFFSET, TB_OP_READ);
     if(retVal != RT_ERR_OK)
         return retVal;
    
     retVal = rtl8373_setAsicRegBits(regAddr, RTL8373_ITA_CTRL0_TLB_TYPE_MASK, TB_TARGET_CVLAN);
     if(retVal != RT_ERR_OK)
         return retVal;
    
     retVal = rtl8373_setAsicRegBit(regAddr, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET , TB_EXECUTE);
     if(retVal != RT_ERR_OK)
         return retVal;
    
    /*wait access finished */
     busyCounter = RTL8373_VLAN_BUSY_CHECK_NO;
     do{
         retVal = rtl8373_getAsicRegBit(regAddr, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET , &regData);
         if(retVal != RT_ERR_OK)
             return retVal;
         busyCounter --;
         if(busyCounter == 0)
             return RT_ERR_BUSYWAIT_TIMEOUT;
     }while(regData);
    
      /* Read VLAN data from register */
     retVal = rtl8373_getAsicReg(RTL8373_ITA_READ_DATA0_ADDR(0) , &regData);
     if(retVal != RT_ERR_OK)
         return retVal;

     _dal_rtl8373_Vlan4kStSmi2User(regData, pVlan4kEntry);


#if defined(CONFIG_RTL8373_ASICDRV_TEST)
    memcpy(pVlan4kEntry, &Rtl8367dVirtualVlanTable[pVlan4kEntry->vid], sizeof(dal_rtl8373_user_vlan4kentry));
#endif

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_init
 * Description:
 *      Initialize VLAN.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      VLAN is disabled by default. User has to call this API to enable VLAN before
 *      using it. And It will set a default VLAN(vid 1) including all ports and set
 *      all ports PVID to the default VLAN.
 */
rtk_api_ret_t dal_rtl8373_vlan_init(void)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 idx = 0;
    dal_rtl8373_user_vlan4kentry vlan4K;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Set a default VLAN with vid 1 to 4K table for all ports */
    memset(&vlan4K, 0, sizeof(dal_rtl8373_user_vlan4kentry));
    vlan4K.vid = 1;
    vlan4K.mbr = RTK_PHY_PORTMASK_ALL;
    vlan4K.untag = RTK_PHY_PORTMASK_ALL;
    vlan4K.fid_msti = 0;
    if ((retVal = _dal_rtl8373_setAsicVlan4kEntry(&vlan4K)) != RT_ERR_OK)
        return retVal;

    /* Set all ports PVID to default VLAN and tag-mode to original */
    RTK_SCAN_ALL_LOG_PORT(idx)
    {
        if ((retVal = dal_rtl8373_vlan_portPvid_set(idx, 1)) != RT_ERR_OK)
            return retVal;
        if ((retVal = dal_rtl8373_vlan_tagMode_set(idx, VLAN_EGRESS_TAG_MODE_ORIGINAL)) != RT_ERR_OK)
            return retVal;
    }

    /* Enable Ingress filter */
    for(idx = 0; idx < RTK_MAX_NUM_OF_PORT; idx++)
    {
        if ((retVal = dal_rtl8373_vlan_portIgrFilterEnable_set(idx, ENABLED)) != RT_ERR_OK)
            return retVal;
    }

    /* enable VLAN egress filter */
    if ((retVal = dal_rtl8373_vlan_egrFilterEnable_set(ENABLED)) != RT_ERR_OK)
        return retVal;
    
    /*invalid all L2 disable learning table*/       
    if((retVal = rtl8373_setAsicReg(RTL8373_VLAN_L2_LRN_DIS_ADDR(0),0)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtl8373_setAsicReg(RTL8373_VLAN_L2_LRN_DIS_ADDR(1),0)) != RT_ERR_OK)
        return retVal; 


    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_set
 * Description:
 *      Set a VLAN entry.
 * Input:
 *      vid - VLAN ID to configure.
 *      pVlanCfg - VLAN Configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameters.
 *      RT_ERR_L2_FID               - Invalid FID.
 *      RT_ERR_VLAN_PORT_MBR_EXIST  - Invalid member port mask.
 *      RT_ERR_VLAN_VID             - Invalid VID parameter.
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_vlan_set(rtk_vlan_t vid, rtk_vlan_entry_t *pVlanCfg)
{
    rtk_api_ret_t retVal = 0;
    dal_rtl8373_user_vlan4kentry vlan4K;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* vid must be 0~4095 */
    if (vid > RTL8373_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* Null pointer check */
    if(NULL == pVlanCfg)
        return RT_ERR_NULL_POINTER;
    
    memset(&vlan4K, 0, sizeof(dal_rtl8373_user_vlan4kentry));

    /* Check port mask valid */
    RTK_CHK_PORTMASK_VALID(&pVlanCfg->mbr);

    /* Check untag port mask valid */
    RTK_CHK_PORTMASK_VALID(&pVlanCfg->untag);

    /* fid must be 0~15 */
    if(pVlanCfg->fid_msti > RTL8373_FIDMAX)
        return RT_ERR_L2_FID;

    /* svlan_chk_svl_ivl*/
    if(pVlanCfg->svlan_chk_ivl_svl > 1)
        return RT_ERR_INPUT;

    /*svl_ivl*/
    if(pVlanCfg->ivl_svl > 1)
        return RT_ERR_INPUT;



    /* update 4K table */
    memset(&vlan4K, 0, sizeof(dal_rtl8373_user_vlan4kentry));
    vlan4K.vid = vid;

    vlan4K.mbr    = (pVlanCfg->mbr.bits[0]) & RTK_MAX_PORT_MASK;
    vlan4K.untag  = (pVlanCfg->untag.bits[0]) & RTK_MAX_PORT_MASK;
    
    vlan4K.svlan_chk_ivl_svl = pVlanCfg->svlan_chk_ivl_svl;
    vlan4K.ivl_svl      = pVlanCfg->ivl_svl;
    vlan4K.fid_msti     = pVlanCfg->fid_msti;

    if ((retVal = _dal_rtl8373_setAsicVlan4kEntry(&vlan4K)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_get
 * Description:
 *      Get a VLAN entry.
 * Input:
 *      vid - VLAN ID to configure.
 * Output:
 *      pVlanCfg - VLAN Configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_vlan_get(rtk_vlan_t vid, rtk_vlan_entry_t *pVlanCfg)
{
    rtk_api_ret_t retVal = 0;
    dal_rtl8373_user_vlan4kentry vlan4K;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* vid must be 0~8191 */
    if (vid > RTL8373_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* Null pointer check */
    if(NULL == pVlanCfg)
        return RT_ERR_NULL_POINTER;
    
    memset(&vlan4K, 0, sizeof(dal_rtl8373_user_vlan4kentry));
    vlan4K.vid = vid;

    if ((retVal = _dal_rtl8373_getAsicVlan4kEntry(&vlan4K)) != RT_ERR_OK)
        return retVal;

    pVlanCfg->mbr.bits[0] = vlan4K.mbr;
    pVlanCfg->untag.bits[0] = vlan4K.untag;
    pVlanCfg->svlan_chk_ivl_svl = vlan4K.svlan_chk_ivl_svl;
    pVlanCfg->ivl_svl    = vlan4K.ivl_svl;
    pVlanCfg->fid_msti  = vlan4K.fid_msti;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_egrFilterEnable_set
 * Description:
 *      Set VLAN egress filter.
 * Input:
 *      egrFilter - Egress filtering
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid input parameters.
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_vlan_egrFilterEnable_set(rtk_enable_t egrFilter)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(egrFilter >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    /* enable VLAN */
    if ((retVal = rtl8373_setAsicRegBit(RTL8373_VLAN_CTRL_ADDR, RTL8373_VLAN_CTRL_CVLAN_FILTER_OFFSET, egrFilter)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_egrFilterEnable_get
 * Description:
 *      Get VLAN egress filter.
 * Input:
 *      pEgrFilter - Egress filtering
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - NULL Pointer.
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_vlan_egrFilterEnable_get(rtk_enable_t *pEgrFilter)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 state = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEgrFilter)
        return RT_ERR_NULL_POINTER;

    /* enable VLAN */
    if ((retVal = rtl8373_getAsicRegBit(RTL8373_VLAN_CTRL_ADDR, RTL8373_VLAN_CTRL_CVLAN_FILTER_OFFSET, &state)) != RT_ERR_OK)
        return retVal;

    *pEgrFilter = (rtk_enable_t)state;
    return RT_ERR_OK;
}

/* Function Name:
 *     dal_rtl8373_vlan_portPvid_set
 * Description:
 *      Set port to specified VLAN ID(PVID).
 * Input:
 *      port - Port id.
 *      pvid - Specified VLAN ID.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_VLAN_PRIORITY        - Invalid priority.
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - VLAN entry not found.
 *      RT_ERR_VLAN_VID             - Invalid VID parameter.
 * Note:
 *       The API is used for Port-based VLAN. The untagged frame received from the
 *       port will be classified to the specified VLAN and assigned to the specified priority.
 */
rtk_api_ret_t dal_rtl8373_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    /* vid must be 0~8191 */
    if (pvid > RTL8373_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* priority  */

    retVal = rtl8373_setAsicRegBits(RTL8373_VLAN_PORT_PB_VLAN_ADDR(port), RTL8373_VLAN_PORT_PB_VLAN_PVID_MASK(port), pvid);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_portPvid_get
 * Description:
 *      Get VLAN ID(PVID) on specified port.
 * Input:
 *      port - Port id.
 * Output:
 *      pPvid - Specified VLAN ID.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *     The API can get the PVID and 802.1p priority for the PVID of Port-based VLAN.
 */
rtk_api_ret_t dal_rtl8373_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pPvid)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8373_getAsicRegBits(RTL8373_VLAN_PORT_PB_VLAN_ADDR(port), RTL8373_VLAN_PORT_PB_VLAN_PVID_MASK(port), pPvid);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_portIgrFilterEnable_set
 * Description:
 *      Set VLAN ingress for each port.
 * Input:
 *      port - Port id.
 *      igrFilter - VLAN ingress function enable status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      The status of vlan ingress filter is as following:
 *      - DISABLED
 *      - ENABLED
 *      While VLAN function is enabled, ASIC will decide VLAN ID for each received frame and get belonged member
 *      ports from VLAN table. If received port is not belonged to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
 */
rtk_api_ret_t dal_rtl8373_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igrFilter)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (igrFilter >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_VLAN_PORT_IGR_FLTR_ADDR(port), RTL8373_VLAN_PORT_IGR_FLTR_IGR_FLTR_ACT_OFFSET(port), (rtk_uint32)igrFilter)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_portIgrFilterEnable_get
 * Description:
 *      Get VLAN Ingress Filter
 * Input:
 *      port        - Port id.
 * Output:
 *      pIgrFilter - VLAN ingress function enable status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *     The API can Get the VLAN ingress filter status.
 *     The status of vlan ingress filter is as following:
 *     - DISABLED
 *     - ENABLED
 */
rtk_api_ret_t dal_rtl8373_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgrFilter)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regData = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pIgrFilter)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_VLAN_PORT_IGR_FLTR_ADDR(port), RTL8373_VLAN_PORT_IGR_FLTR_IGR_FLTR_ACT_OFFSET(port), &regData)) != RT_ERR_OK)
        return retVal;

    *pIgrFilter = (rtk_enable_t)regData;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN accept_frame_type
 * Input:
 *      port                - Port id.
 *      acceptFrameType   - accept frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_PORT_ID                  - Invalid port number.
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE   - Invalid frame type.
 * Note:
 *      The API is used for checking 802.1Q tagged frames.
 *      The accept frame type as following:
 *      - ACCEPT_FRAME_TYPE_ALL
 *      - ACCEPT_FRAME_TYPE_TAG_ONLY
 *      - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
rtk_api_ret_t dal_rtl8373_vlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t acceptFrameType)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (acceptFrameType >= ACCEPT_FRAME_TYPE_END)
        return RT_ERR_VLAN_ACCEPT_FRAME_TYPE;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_VLAN_PORT_AFT_ADDR(port), RTL8373_VLAN_PORT_AFT_CTAG_ACCEPT_TYPE_MASK(port), acceptFrameType)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN accept_frame_type
 * Input:
 *      port - Port id.
 * Output:
 *      pAcceptFrameType - accept frame type
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *     The API can Get the VLAN ingress filter.
 *     The accept frame type as following:
 *     - ACCEPT_FRAME_TYPE_ALL
 *     - ACCEPT_FRAME_TYPE_TAG_ONLY
 *     - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
rtk_api_ret_t dal_rtl8373_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAcceptFrameType)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regData = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pAcceptFrameType)
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8373_getAsicRegBits( RTL8373_VLAN_PORT_AFT_ADDR(port), RTL8373_VLAN_PORT_AFT_CTAG_ACCEPT_TYPE_MASK(port) , &regData)) != RT_ERR_OK)
        return retVal;

    *pAcceptFrameType = (rtk_vlan_acceptFrameType_t)regData;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_tagMode_set
 * Description:
 *      Set CVLAN egress tag mode
 * Input:
 *      port        - Port id.
 *      tagMode    - The egress tag mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      The API can set Egress tag mode. There are 4 mode for egress tag:
 *      - VLAN_EGRESS_TAG_MODE_ORIGINAL,
 *      - VLAN_EGRESS_TAG_MODE_KEEP_FORMAT,
 *      - VLAN_EGRESS_TAG_MODE_PRI.
 *      - VLAN_EGRESS_TAG_MODE_REAL_KEEP,
 */
rtk_api_ret_t dal_rtl8373_vlan_tagMode_set(rtk_port_t port, rtk_vlan_egressTagMode_t tagMode)
{
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (tagMode >= VLAN_EGRESS_TAG_MODE_END)
        return RT_ERR_PORT_ID;

    return rtl8373_setAsicRegBits(RTL8373_VLAN_PORT_EGR_TAG_ADDR(port), RTL8373_VLAN_PORT_EGR_TAG_MODE_MASK(port), tagMode);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_tagMode_get
 * Description:
 *      Get CVLAN egress tag mode
 * Input:
 *      port - Port id.
 * Output:
 *      pTagMode - The egress tag mode.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get Egress tag mode. There are 4 mode for egress tag:
 *      - VLAN_EGRESS_TAG_MODE_ORIGINAL,
 *      - VLAN_EGRESS_TAG_MODE_KEEP_FORMAT,
 *      - VLAN_EGRESS_TAG_MODE_PRI.
 *      - VLAN_EGRESS_TAG_MODE_REAL_KEEP,
 */
rtk_api_ret_t dal_rtl8373_vlan_tagMode_get(rtk_port_t port, rtk_vlan_egressTagMode_t *pTagMode)
{
    rtk_uint32  mode = 0;
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pTagMode)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8373_getAsicRegBits(RTL8373_VLAN_PORT_EGR_TAG_ADDR(port), RTL8373_VLAN_PORT_EGR_TAG_MODE_MASK(port), &mode);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pTagMode = (rtk_vlan_egressTagMode_t)mode;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_portTransparent_set
 * Description:
 *      Set VLAN transparent mode
 * Input:
 *      egrPort        - Egress Port id.
 *      pIgrPmsk      - Ingress Port Mask.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
rtk_api_ret_t dal_rtl8373_vlan_portTransparent_set(rtk_port_t egrPort, rtk_portmask_t *pIgrPmsk)
{
     rtk_api_ret_t retVal = 0;

     /* Check initialization state */
     RTK_CHK_INIT_STATE();

     /* Check Port Valid */
     if(egrPort > RTL8373_PORTIDMAX)
         return RT_ERR_PORT_ID;  

     if(NULL == pIgrPmsk)
        return RT_ERR_NULL_POINTER;
     
     if(pIgrPmsk->bits[0] > RTL8373_PORTMASK)
         return RT_ERR_PORT_MASK;

     if ((retVal = rtl8373_setAsicRegBits(RTL8373_VLAN_PORT_EGR_TRANS_ADDR(egrPort), RTL8373_VLAN_PORT_EGR_TRANS_PMSK_MASK(egrPort), pIgrPmsk->bits[0])) != RT_ERR_OK)
         return retVal;

     return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_portTransparent_get
 * Description:
 *      Get VLAN transparent mode
 * Input:
 *      egrPort        - Egress Port id.
 * Output:
 *      pIgrPmsk      - Ingress Port Mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
rtk_api_ret_t dal_rtl8373_vlan_portTransparent_get(rtk_port_t egrPort, rtk_portmask_t *pIgrPmsk)
{
     rtk_api_ret_t retVal = 0;
     rtk_uint32    pmask = 0;

     /* Check initialization state */
     RTK_CHK_INIT_STATE();

     /* Check Port Valid */
    if(egrPort > RTL8373_PORTIDMAX)
         return RT_ERR_PORT_ID;

     if(NULL == pIgrPmsk)
        return RT_ERR_NULL_POINTER;

     if ((retVal = rtl8373_getAsicRegBits(RTL8373_VLAN_PORT_EGR_TRANS_ADDR(egrPort), RTL8373_VLAN_PORT_EGR_TRANS_PMSK_MASK(egrPort),&pmask)) != RT_ERR_OK)
         return retVal;

     pIgrPmsk->bits[0] = pmask;

     return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_keep_set
 * Description:
 *      Set VLAN egress keep mode
 * Input:
 *      egrPort        - Egress Port id.
 *      pIgrPmsk      - Ingress Port Mask.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
rtk_api_ret_t dal_rtl8373_vlan_keep_set(rtk_port_t egrPort, rtk_portmask_t *pIgrPmsk)
{
     rtk_api_ret_t retVal = 0;

     /* Check initialization state */
     RTK_CHK_INIT_STATE();

     /* Check Port Valid */
    if(egrPort > RTL8373_PORTIDMAX)
         return RT_ERR_PORT_ID;

     if(NULL == pIgrPmsk)
        return RT_ERR_NULL_POINTER;

    if(pIgrPmsk->bits[0] > RTL8373_PORTMASK)
         return RT_ERR_PORT_MASK;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_VLAN_PORT_EGR_KEEP_ADDR(egrPort), RTL8373_VLAN_PORT_EGR_KEEP_PMSK_MASK(egrPort), pIgrPmsk->bits[0])) != RT_ERR_OK)
         return retVal;

     return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_keep_get
 * Description:
 *      Get VLAN egress keep mode
 * Input:
 *      egrPort        - Egress Port id.
 * Output:
 *      pIgrPmsk      - Ingress Port Mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
rtk_api_ret_t dal_rtl8373_vlan_keep_get(rtk_port_t egrPort, rtk_portmask_t *pIgrPmsk)
{
     rtk_api_ret_t retVal = 0;
     rtk_uint32    pmask = 0;

     /* Check initialization state */
     RTK_CHK_INIT_STATE();

     /* Check Port Valid */
    if(egrPort > RTL8373_PORTIDMAX)
         return RT_ERR_PORT_ID;

     if(NULL == pIgrPmsk)
        return RT_ERR_NULL_POINTER;

     if ((retVal = rtl8373_getAsicRegBits(RTL8373_VLAN_PORT_EGR_KEEP_ADDR(egrPort), RTL8373_VLAN_PORT_EGR_KEEP_PMSK_MASK(egrPort),&pmask)) != RT_ERR_OK)
         return retVal;

     pIgrPmsk->bits[0] = pmask;

     return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_stg_set
 * Description:
 *      Set spanning tree group instance of the vlan to the specified device
 * Input:
 *      vid - Specified VLAN ID.
 *      stg - spanning tree group instance.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_MSTI         - Invalid msti parameter
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *      The API can set spanning tree group instance of the vlan to the specified device.
 */
rtk_api_ret_t dal_rtl8373_vlan_stg_set(rtk_vlan_t vid, rtk_stp_msti_id_t stg)
{
    rtk_api_ret_t retVal = 0;
    dal_rtl8373_user_vlan4kentry vlan4K;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* vid must be 0~4095 */
    if (vid > RTL8373_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* stg must be 0~3 */
    if (stg > RTL8373_MSTIMAX)
        return RT_ERR_MSTI;

    memset(&vlan4K, 0, sizeof(dal_rtl8373_user_vlan4kentry));

    /* update 4K table */
    vlan4K.vid = vid;
    if ((retVal = _dal_rtl8373_getAsicVlan4kEntry(&vlan4K)) != RT_ERR_OK)
        return retVal;

    vlan4K.fid_msti= stg;
    if ((retVal = _dal_rtl8373_setAsicVlan4kEntry(&vlan4K)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_stg_get
 * Description:
 *      Get spanning tree group instance of the vlan to the specified device
 * Input:
 *      vid - Specified VLAN ID.
 * Output:
 *      pStg - spanning tree group instance.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *      The API can get spanning tree group instance of the vlan to the specified device.
 */
rtk_api_ret_t dal_rtl8373_vlan_stg_get(rtk_vlan_t vid, rtk_stp_msti_id_t *pStg)
{
    rtk_api_ret_t retVal = 0;
    dal_rtl8373_user_vlan4kentry vlan4K;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* vid must be 0~4095 */
    if (vid > RTL8373_VIDMAX)
        return RT_ERR_VLAN_VID;

    if(NULL == pStg)
        return RT_ERR_NULL_POINTER;

    memset(&vlan4K, 0, sizeof(dal_rtl8373_user_vlan4kentry));

    /* update 4K table */
    vlan4K.vid = vid;
    if ((retVal = _dal_rtl8373_getAsicVlan4kEntry(&vlan4K)) != RT_ERR_OK)
        return retVal;

    *pStg = vlan4K.fid_msti;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_portFid_set
 * Description:
 *      Set port-based filtering database
 * Input:
 *      port - Port id.
 *      enable - ebable port-based FID
 *      fid - Specified filtering database.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_L2_FID - Invalid fid.
 *      RT_ERR_INPUT - Invalid input parameter.
 *      RT_ERR_PORT_ID - Invalid port ID.
 * Note:
 *      The API can set port-based filtering database. If the function is enabled, all input
 *      packets will be assigned to the port-based fid regardless vlan tag.
 */
rtk_api_ret_t dal_rtl8373_vlan_portFid_set(rtk_port_t port, rtk_enable_t enable, rtk_fid_t fid)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    if(port > RTL8373_PORTIDMAX)
         return RT_ERR_PORT_ID;

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    /* fid must be 0~3 */
    if (fid > RTL8373_FIDMAX)
        return RT_ERR_L2_FID;
    
    if ((retVal = rtl8373_setAsicRegBit(RTL8373_PORT_BASED_FID_EN_ADDR, port, enable))!=RT_ERR_OK)
        return retVal;
    
    if ((retVal = rtl8373_setAsicRegBits(RTL8373_PORT_BASED_FID_ADDR(port),  RTL8373_PORT_BASED_FID_FID_MASK(port) , fid))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_portFid_get
 * Description:
 *      Get port-based filtering database
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - ebable port-based FID
 *      pFid - Specified filtering database.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port ID.
 * Note:
 *      The API can get port-based filtering database status. If the function is enabled, all input
 *      packets will be assigned to the port-based fid regardless vlan tag.
 */
rtk_api_ret_t dal_rtl8373_vlan_portFid_get(rtk_port_t port, rtk_enable_t *pEnable, rtk_fid_t *pFid)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 enable = 0 ;
    rtk_uint32 fid = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    if(port > RTL8373_PORTIDMAX)
         return RT_ERR_PORT_ID;

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if(NULL == pFid)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_PORT_BASED_FID_EN_ADDR, port, &enable))!=RT_ERR_OK)
          return retVal;
    
    if ((retVal = rtl8373_getAsicRegBits(RTL8373_PORT_BASED_FID_ADDR(port),  RTL8373_PORT_BASED_FID_FID_MASK(port) , &fid))!=RT_ERR_OK)
          return retVal;

    *pEnable = (rtk_enable_t) (enable);
    *pFid = (rtk_fid_t)fid;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_reservedVidAction_set
 * Description:
 *      Set Action of VLAN ID = 0 & 4095 tagged packet
 * Input:
 *      actVid0     - Action for VID 0.
 *      actVid4095  - Action for VID 4095.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_vlan_reservedVidAction_set(rtk_vlan_resVidAction_t actVid0, rtk_vlan_resVidAction_t actVid4095)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(actVid0 >= RSV_VID_ACTION_END)
        return RT_ERR_INPUT;

    if(actVid4095 >= RSV_VID_ACTION_END)
        return RT_ERR_INPUT;

    if((retVal = rtl8373_setAsicRegBit(RTL8373_VLAN_CTRL_ADDR, RTL8373_VLAN_CTRL_VID0_TYPE_OFFSET, actVid0)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8373_setAsicRegBit(RTL8373_VLAN_CTRL_ADDR, RTL8373_VLAN_CTRL_VID4095_TYPE_OFFSET, actVid4095)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_reservedVidAction_get
 * Description:
 *      Get Action of VLAN ID = 0 & 4095 tagged packet
 * Input:
 *      pAction_vid0     - Action for VID 0.
 *      pAction_vid4095  - Action for VID 4095.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - NULL Pointer
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_vlan_reservedVidAction_get(rtk_vlan_resVidAction_t *pActVid0, rtk_vlan_resVidAction_t *pActVid4095)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(pActVid0 == NULL)
        return RT_ERR_NULL_POINTER;

    if(pActVid4095 == NULL)
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8373_getAsicRegBit(RTL8373_VLAN_CTRL_ADDR, RTL8373_VLAN_CTRL_VID0_TYPE_OFFSET, (rtk_uint32 *)pActVid0)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8373_getAsicRegBit(RTL8373_VLAN_CTRL_ADDR, RTL8373_VLAN_CTRL_VID4095_TYPE_OFFSET, (rtk_uint32 *)pActVid4095)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_realKeepRemarkEnable_set
 * Description:
 *      Set Real keep 1p remarking feature
 * Input:
 *      enabled     - State of 1p remarking at real keep packet
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_vlan_realKeepRemarkEnable_set(rtk_enable_t enabled)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(enabled >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if((retVal = rtl8373_setAsicRegBit(RTL8373_VLAN_TAG_PRI_CFG_ADDR, RTL8373_VLAN_TAG_PRI_CFG_RMK1P_BYPASS_REALKEEP_OFFSET, enabled)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_vlan_realKeepRemarkEnable_get
 * Description:
 *      Get Real keep 1p remarking feature
 * Input:
 *      None.
 * Output:
 *      pEnabled     - State of 1p remarking at real keep packet
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_vlan_realKeepRemarkEnable_get(rtk_enable_t *pEnabled)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnabled)
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8373_getAsicRegBit(RTL8373_VLAN_TAG_PRI_CFG_ADDR, RTL8373_VLAN_TAG_PRI_CFG_RMK1P_BYPASS_REALKEEP_OFFSET, (rtk_uint32 *)pEnabled)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*******************************************************************************
* Function Name: dal_rtl8373_vlan_disL2Learn_entry_set
*
* Description:
*   config a L2 disable learning entry which based on vlan id
*Input:
*       index: entry index
*       pDisL2LearnCfg: L2 disable learning database
*Output:
*       None
*Return:
*       RT_ERR_OK           - OK
*       RT_ERR_FAILED       - Failed
*       RT_ERR_ENTRY_INDEX  - error entry
*       RT_ERR_ENABLE       - error action
*       RT_ERR_NULL_POINTER - NULL Pointer
*       RT_ERR_VLAN_VID     - Invalid VID parameter.

*Note:  None
*******************************************************************************/
rtk_api_ret_t dal_rtl8373_vlan_disL2Learn_entry_set(rtk_vlan_disL2_learn_t *pDisL2LearnCfg)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint8 available = 0xFF, same = 0xFF, i=0;
    rtk_uint32 vid = 0, index = 0, regData = 0;
    rtk_vlan_disL2_learn_t  tempData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();
    if(NULL ==pDisL2LearnCfg)
        return RT_ERR_NULL_POINTER;

    if (pDisL2LearnCfg->vid > RTL8373_VIDMAX)
        return RT_ERR_INPUT;

    if ((pDisL2LearnCfg->act > 1) ||(pDisL2LearnCfg->valid > 1) )
        return RT_ERR_INPUT;

    vid = pDisL2LearnCfg->vid;
    for(i=0; i<2; i++)
    {        
        if((retVal = rtl8373_getAsicReg(RTL8373_VLAN_L2_LRN_DIS_ADDR(i),&regData)) != RT_ERR_OK)
            return retVal;
        
        tempData.valid = (regData & RTL8373_VLAN_L2_LRN_DIS_VALID_MASK) >> RTL8373_VLAN_L2_LRN_DIS_VALID_OFFSET;
        tempData.vid = (regData & RTL8373_VLAN_L2_LRN_DIS_VID_MASK) >> RTL8373_VLAN_L2_LRN_DIS_VID_OFFSET;
        tempData.act = (regData & RTL8373_VLAN_L2_LRN_DIS_ACT_MASK) >> RTL8373_VLAN_L2_LRN_DIS_ACT_OFFSET;

        if(tempData.valid == 0)
            available = i;
        if(tempData.vid == vid)
            same = i;
    }
    if( (available == 0xFF) && (same == 0xFF) )
        return RT_ERR_TBL_FULL;
    else if( (available == 0xFF) && (same != 0xFF) )
        index = same;
    else if( (available != 0xFF) && (same == 0xFF) )
        index = available;
    else
        index = same;

    regData =(pDisL2LearnCfg->act << RTL8373_VLAN_L2_LRN_DIS_ACT_OFFSET) |(pDisL2LearnCfg->vid << RTL8373_VLAN_L2_LRN_DIS_VID_OFFSET);
    regData |=  pDisL2LearnCfg->valid ;        

    /* set entry*/
    if((retVal = rtl8373_setAsicReg(RTL8373_VLAN_L2_LRN_DIS_ADDR(index),regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*******************************************************************************
* Function Name: dal_rtl8373_vlan_disL2Learn_entry_get
*
* Description:
*   get vlan based disable L2 entry data
*Input:
*       index: entry index
*       
*Output:
*       pDisL2LearnCfg: L2 disable learning database
*Return:
*       RT_ERR_OK           - OK
*       RT_ERR_FAILED       - Failed
*       RT_ERR_ENTRY_INDEX  - error entry
*       RT_ERR_ENABLE       - error action
*       RT_ERR_NULL_POINTER - NULL Pointer
*       RT_ERR_VLAN_VID     - Invalid VID parameter.

*Note:  None
*******************************************************************************/
rtk_api_ret_t dal_rtl8373_vlan_disL2Learn_entry_get(rtk_uint32 index, rtk_vlan_disL2_learn_t *pDisL2LearnCfg)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regData = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();
    if( index > RTL8373_DISABLE_L2_LEARN_MAX)
        return RT_ERR_INPUT;
    if(NULL ==pDisL2LearnCfg)
        return RT_ERR_NULL_POINTER;

    
    if((retVal = rtl8373_getAsicReg(RTL8373_VLAN_L2_LRN_DIS_ADDR(index),&regData)) != RT_ERR_OK)
        return retVal;
       
    pDisL2LearnCfg->valid = (regData & RTL8373_VLAN_L2_LRN_DIS_VALID_MASK) >> RTL8373_VLAN_L2_LRN_DIS_VALID_OFFSET;
    pDisL2LearnCfg->vid = (regData & RTL8373_VLAN_L2_LRN_DIS_VID_MASK) >> RTL8373_VLAN_L2_LRN_DIS_VID_OFFSET;
    pDisL2LearnCfg->act = (regData & RTL8373_VLAN_L2_LRN_DIS_ACT_MASK) >> RTL8373_VLAN_L2_LRN_DIS_ACT_OFFSET;

    return RT_ERR_OK;
}
/* Function Name:
 *      dal_rtl8373_vlan_reset
 * Description:
 *      Reset VLAN
 * Input:
 *      None.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_vlan_reset(void)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if((retVal = rtl8373_setAsicRegBit(RTL8373_VLAN_CTRL_ADDR, RTL8373_VLAN_CTRL_TABLE_RST_OFFSET, ENABLED)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



