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
 * Feature : Here is a list of all functions and variables in SVLAN module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
//#include <dal/rtl8373/rtl8373_reg_definition.h>
#include <dal/rtl8373/dal_rtl8373_vlan.h>
#include <dal/rtl8373/dal_rtl8373_svlan.h>
#include <dal/rtl8373/rtl8373_asicdrv.h>
#include <string.h>


/* Function Name:
 *      dal_rtl8373_svlanInit
 * Description:
 *      Initialize SVLAN Configuration
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design.
 *      User can set mathced ether type as service provider supported protocol.
 */
rtk_api_ret_t dal_rtl8373_svlanInit(void)
{
    rtk_uint32 idx = 0;
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();
    /*default use C-priority*/
    if ((retVal = dal_rtl8373_svlanPriRef_set(REF_CTAG_PRI)) != RT_ERR_OK)
        return retVal;

    /*Drop SVLAN untag frame*/
    if ((retVal = dal_rtl8373_svlanUntagAction_set(UNTAG_DROP, 0)) != RT_ERR_OK)
        return retVal;
    /*Set TPID to 0x88a8*/
    if ((retVal = dal_rtl8373_svlanTpid_set(0x88a8)) != RT_ERR_OK)
        return retVal;
    /*Clean Uplink Port Mask to none*/
    if ((retVal = rtl8373_setAsicReg(RTL8373_VS_UPLINK_PORT_ADDR,0)) != RT_ERR_OK)
        return retVal;
    /*Clean C2S Configuration*/
    for (idx = 0; idx <= RTL8373_C2SIDXMAX;  idx++)
    {
        if ((retVal = rtl8373_setAsicReg(RTL8373_VLAN_C2S_ENTRY_ADDR(idx)+4, 0)) != RT_ERR_OK)
                return retVal;

        if ((retVal = rtl8373_setAsicReg(RTL8373_VLAN_C2S_ENTRY_ADDR(idx), 0)) != RT_ERR_OK)
                return retVal;
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_svlanServicePort_add
 * Description:
 *      Add one service port in the specified device
 * Input:
 *      port - Port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must
 *      contain accept SVID in S-tag field.
 */
rtk_api_ret_t dal_rtl8373_svlanServicePort_add(rtk_port_t port)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 pmsk = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8373_getAsicReg(RTL8373_VS_UPLINK_PORT_ADDR, &pmsk)) != RT_ERR_OK)
        return retVal;

    pmsk = pmsk | (1<<rtk_switch_port_L2P_get(port));

    if ((retVal = rtl8373_setAsicReg(RTL8373_VS_UPLINK_PORT_ADDR, pmsk)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_svlanServicePort_get
 * Description:
 *      Get service ports in the specified device.
 * Input:
 *      None
 * Output:
 *      pSvlanPmsk - pointer buffer of svlan ports.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must
 *      contain accept SVID in S-tag field.
 */
rtk_api_ret_t dal_rtl8373_svlanServicePort_get(rtk_portmask_t *pSvlanPmsk)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 phyMbrPmask = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvlanPmsk)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicReg(RTL8373_VS_UPLINK_PORT_ADDR, &phyMbrPmask)) != RT_ERR_OK)
        return retVal;


    if(rtk_switch_portmask_P2L_get(phyMbrPmask, pSvlanPmsk) != RT_ERR_OK)
        return RT_ERR_FAILED;


    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_svlanservicePort_del
 * Description:
 *      Delete one service port in the specified device
 * Input:
 *      port - Port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API is removing SVLAN service port in the specified device.
 */
rtk_api_ret_t dal_rtl8373_svlanServicePort_del(rtk_port_t port)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 pmsk = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8373_getAsicReg(RTL8373_VS_UPLINK_PORT_ADDR, &pmsk)) != RT_ERR_OK)
        return retVal;

    pmsk = pmsk & ~(1<<rtk_switch_port_L2P_get(port));

    if ((retVal = rtl8373_setAsicReg(RTL8373_VS_UPLINK_PORT_ADDR, pmsk)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_svlanTpid_set
 * Description:
 *      Configure accepted S-VLAN ether type.
 * Input:
 *      svlanTpid - Ether type of S-tag frame parsing in uplink ports.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design.
 *      User can set mathced ether type as service provider supported protocol.
 */
rtk_api_ret_t dal_rtl8373_svlanTpid_set(rtk_uint32 svlanTpid)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (svlanTpid>RTK_MAX_NUM_OF_PROTO_TYPE)
        return RT_ERR_INPUT;

    if ((retVal = rtl8373_setAsicReg(RTL8373_VS_GLB_CTRL_ADDR, svlanTpid)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_svlanTpid_get
 * Description:
 *      Get accepted S-VLAN ether type setting.
 * Input:
 *      None
 * Output:
 *      pSvlanTpid -  Ether type of S-tag frame parsing in uplink ports.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must
 *      contain accept SVID in S-tag field.
 */
rtk_api_ret_t dal_rtl8373_svlanTpid_get(rtk_uint32 *pSvlanTpid)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvlanTpid)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicReg(RTL8373_VS_GLB_CTRL_ADDR, &regVal)) != RT_ERR_OK)
        return retVal;

    *pSvlanTpid = (regVal & RTK_MAX_NUM_OF_PROTO_TYPE);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_svlanPriRef_set
 * Description:
 *      Set S-VLAN upstream priority reference setting.
 * Input:
 *      ref - reference selection parameter.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      The API can set the upstream SVLAN tag priority reference source. The related priority
 *      sources are as following:
 *      - REF_INTERNAL_PRI,
 *      - REF_CTAG_PRI,
 *      - REF_SVLAN_PRI,
 *      - REF_PB_PRI.
 */
rtk_api_ret_t dal_rtl8373_svlanPriRef_set(rtk_svlan_pri_ref_t ref)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (ref >= REF_PRI_END)
        return RT_ERR_INPUT;
    
    if ((retVal = rtl8373_setAsicRegBits(RTL8373_VS_CTRL_ADDR, RTL8373_VS_CTRL_SPRISEL_MASK, ref)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_svlanPriRef_get
 * Description:
 *      Get S-VLAN upstream priority reference setting.
 * Input:
 *      None
 * Output:
 *      pRef - reference selection parameter.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API can get the upstream SVLAN tag priority reference source. The related priority
 *      sources are as following:
 *      - REF_INTERNAL_PRI,
 *      - REF_CTAG_PRI,
 *      - REF_SVLAN_PRI,
 *      - REF_PB_PRI
 */
rtk_api_ret_t dal_rtl8373_svlanPriRef_get(rtk_svlan_pri_ref_t *pRef)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pRef)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_VS_CTRL_ADDR, RTL8373_VS_CTRL_SPRISEL_MASK, pRef)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_svlanmemberPortEntry_set
 * Description:
 *      Configure system SVLAN member content
 * Input:
 *      svid - SVLAN id
 *      psvlan_cfg - SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameter.
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_SVLAN_TABLE_FULL - SVLAN configuration is full.
 * Note:
 *      The API can set system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped by default setup.
 *      - rtk_svlan_memberCfg_t->svid is SVID of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->memberport is member port mask of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->fid is filtering database of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->priority is priority of SVLAN member configuration.
 */
rtk_api_ret_t dal_rtl8373_svlanMbrPortEntry_set(rtk_vlan_t svid, rtk_svlan_memberCfg_t *pSvlan_cfg)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 phyMbrPmask, phyUntagPmask;
    dal_rtl8373_user_vlan4kentry vlan4kEntry;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvlan_cfg)
        return RT_ERR_NULL_POINTER;

    if(svid > RTL8373_VIDMAX)
        return RT_ERR_SVLAN_VID;

    RTK_CHK_PORTMASK_VALID(&pSvlan_cfg->memberport);

    RTK_CHK_PORTMASK_VALID(&pSvlan_cfg->untagport);

    if (pSvlan_cfg->fid > RTL8373_FIDMAX)
        return RT_ERR_L2_FID;

    if (pSvlan_cfg->chk_ivl_svl> ENABLED)
        return RT_ERR_INPUT;    

    if (pSvlan_cfg->ivl_svl> ENABLED)
        return RT_ERR_INPUT;    

    if (pSvlan_cfg->fiden !=0)
        return RT_ERR_CHIP_NOT_SUPPORTED;

    if (pSvlan_cfg->priority != 0)
        return RT_ERR_CHIP_NOT_SUPPORTED;

    if (pSvlan_cfg->efiden != 0)
        return RT_ERR_CHIP_NOT_SUPPORTED;

    if (pSvlan_cfg->efid != 0)
        return RT_ERR_CHIP_NOT_SUPPORTED;

    /* Get physical port mask */
    if(rtk_switch_portmask_L2P_get(&(pSvlan_cfg->memberport), &phyMbrPmask) != RT_ERR_OK)
        return RT_ERR_FAILED;
    if(rtk_switch_portmask_L2P_get(&(pSvlan_cfg->untagport), &phyUntagPmask) != RT_ERR_OK)
        return RT_ERR_FAILED;    

    memset(&vlan4kEntry, 0, sizeof(dal_rtl8373_user_vlan4kentry));
    vlan4kEntry.vid = svid;
    if ((retVal = _dal_rtl8373_getAsicVlan4kEntry(&vlan4kEntry)) != RT_ERR_OK)
        return retVal;
    
    vlan4kEntry.vid = svid;
    vlan4kEntry.mbr = phyMbrPmask;
    vlan4kEntry.untag = phyUntagPmask;
    vlan4kEntry.svlan_chk_ivl_svl = pSvlan_cfg->chk_ivl_svl;
    vlan4kEntry.ivl_svl = pSvlan_cfg->ivl_svl;    
    vlan4kEntry.fid_msti = pSvlan_cfg->fid;

    if ((retVal = _dal_rtl8373_setAsicVlan4kEntry(&vlan4kEntry)) != RT_ERR_OK)
        return retVal;  

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_svlanmemberPortEntry_get
 * Description:
 *      Get SVLAN member Configure.
 * Input:
 *      svid - SVLAN id
 * Output:
 *      pSvlan_cfg - SVLAN member configuration
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can get system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped.
 */
rtk_api_ret_t dal_rtl8373_svlanMbrPortEntry_get(rtk_vlan_t svid, rtk_svlan_memberCfg_t *pSvlan_cfg)
{
    rtk_api_ret_t retVal = 0;
    dal_rtl8373_user_vlan4kentry vlan4kEntry;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvlan_cfg)
        return RT_ERR_NULL_POINTER;

    if (svid > RTL8373_VIDMAX)
        return RT_ERR_SVLAN_VID;

    memset(&vlan4kEntry, 0, sizeof(dal_rtl8373_user_vlan4kentry));
    vlan4kEntry.vid = svid;
    if ((retVal = _dal_rtl8373_getAsicVlan4kEntry(&vlan4kEntry)) != RT_ERR_OK)
        return retVal;

    memset(pSvlan_cfg, 0, sizeof(rtk_svlan_memberCfg_t));
    pSvlan_cfg->svid        = vlan4kEntry.vid;
    if(rtk_switch_portmask_P2L_get(vlan4kEntry.mbr,&(pSvlan_cfg->memberport)) != RT_ERR_OK)
        return RT_ERR_FAILED;
    if(rtk_switch_portmask_P2L_get(vlan4kEntry.untag,&(pSvlan_cfg->untagport)) != RT_ERR_OK)
        return RT_ERR_FAILED;
    pSvlan_cfg->chk_ivl_svl = vlan4kEntry.svlan_chk_ivl_svl;
    pSvlan_cfg->ivl_svl     = vlan4kEntry.ivl_svl;    
    pSvlan_cfg->fid         = vlan4kEntry.fid_msti;
       
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_svlanDftSvlan_set
 * Description:
 *      Configure default egress SVLAN.
 * Input:
 *      port - Source port
 *      svid - SVLAN id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_INPUT                    - Invalid input parameter.
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 * Note:
 *      The API can set port n S-tag format index while receiving frame from port n
 *      is transmit through uplink port with s-tag field
 */
rtk_api_ret_t dal_rtl8373_svlanDfltSvlan_set(rtk_port_t port, rtk_vlan_t svid)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    /* svid must be 0~4095 */
    if (svid > RTL8373_VIDMAX)
        return RT_ERR_SVLAN_VID;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_VS_PORT_DFLT_SVID_ADDR(port), RTL8373_VS_PORT_DFLT_SVID_PORT_DFLT_SVID_MASK(port) , svid)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_svlanDfltSvlan_get
 * Description:
 *      Get the configure default egress SVLAN.
 * Input:
 *      port - Source port
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get port n S-tag format index while receiving frame from port n
 *      is transmit through uplink port with s-tag field
 */
rtk_api_ret_t dal_rtl8373_svlanDfltSvlan_get(rtk_port_t port, rtk_vlan_t *pSvid)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvid)
        return RT_ERR_NULL_POINTER;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_VS_PORT_DFLT_SVID_ADDR(port), RTL8373_VS_PORT_DFLT_SVID_PORT_DFLT_SVID_MASK(port), pSvid)) != RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_svlanC2S_add
 * Description:
 *      Configure SVLAN C2S table
 * Input:
 *      vid - VLAN ID
 *      srcPort - Ingress Port
 *      svid - SVLAN VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port ID.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can set system C2S configuration. ASIC will check upstream's VID and assign related
 *      SVID to mathed packet. There are 128 SVLAN C2S configurations.
 */
rtk_api_ret_t dal_rtl8373_svlanC2S_add(rtk_vlan_t vid, rtk_port_t srcPort, rtk_vlan_t svid)
{
    rtk_api_ret_t retVal = 0, i = 0;
    rtk_uint32 empty_idx = 0;
    rtk_port_t phyPort;
    rtk_uint16 doneFlag = 0;
    rtk_uint32 idx_svid = 0, idx_pmsk = 0, idx_cvid = 0;


    /* Check initialization state */
    RTK_CHK_INIT_STATE();


    if (vid > RTL8373_VIDMAX)
        return RT_ERR_VLAN_VID;

    if (svid > RTL8373_VIDMAX)
        return RT_ERR_SVLAN_VID;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(srcPort);

    phyPort = rtk_switch_port_L2P_get(srcPort);
    empty_idx = 0xFFFF;
    doneFlag = FALSE;

    for (i = RTL8373_C2SIDXMAX; i>=0; i--)
    {
        if ((retVal = rtl8373_getAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) , RTL8373_VLAN_C2S_ENTRY_SVID_ASSIGN_MASK, &idx_svid)) != RT_ERR_OK)
                return retVal;

        if ((retVal = rtl8373_getAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) + 4, RTK_MAX_PORT_MASK, &idx_pmsk)) != RT_ERR_OK)
                return retVal;

        if ((retVal = rtl8373_getAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) + 4, (0xFFF << RTK_MAX_NUM_OF_PORT), &idx_cvid)) != RT_ERR_OK)
                return retVal;


        if (idx_cvid == vid)
        {
            /* Check Src_port */
            if(idx_pmsk & (1 << phyPort))
            {
                /* Check SVIDX */
                if(idx_svid == svid)
                {
                    /* All the same, do nothing */
                }
                else
                {
                    /* New svidx, remove src_port and find a new slot to add a new enrty */
                    idx_pmsk = idx_pmsk & ~(1 << phyPort);
                    if(idx_pmsk == 0)
                        idx_svid = 0;

                    if ((retVal = rtl8373_setAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) , RTL8373_VLAN_C2S_ENTRY_SVID_ASSIGN_MASK, idx_svid)) != RT_ERR_OK)
                            return retVal;
                    
                    if ((retVal = rtl8373_setAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) + 4, RTK_MAX_PORT_MASK, idx_pmsk)) != RT_ERR_OK)
                            return retVal;
                }
            }
            else
            {
                if(idx_svid == svid && doneFlag == FALSE)
                {
                    idx_pmsk = idx_pmsk | (1 << phyPort);
                    if ((retVal = rtl8373_setAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) + 4, RTK_MAX_PORT_MASK, idx_pmsk)) != RT_ERR_OK)
                            return retVal;
                    
                    doneFlag = TRUE;
                }
            }
        }
        else if (idx_svid==0&&idx_pmsk==0)
        {
            empty_idx = i;
        }
    }

    if (0xFFFF != empty_idx && doneFlag ==FALSE)
    {
       if ((retVal = rtl8373_setAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(empty_idx) , RTL8373_VLAN_C2S_ENTRY_SVID_ASSIGN_MASK, svid)) != RT_ERR_OK)
               return retVal;
       
       if ((retVal = rtl8373_setAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(empty_idx) + 4, RTK_MAX_PORT_MASK, (1<<phyPort))) != RT_ERR_OK)
               return retVal;
       
       if ((retVal = rtl8373_setAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(empty_idx) + 4, (0xFFF << RTK_MAX_NUM_OF_PORT), vid)) != RT_ERR_OK)
               return retVal;       

       return RT_ERR_OK;
    }
    else if(doneFlag == TRUE)
    {
        return RT_ERR_OK;
    }

    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      dal_rtl8373_svlanC2S_del
 * Description:
 *      Delete one C2S entry
 * Input:
 *      vid - VLAN ID
 *      srcPort - Ingress Port
 *      svid - SVLAN VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_VLAN_VID         - Invalid VID parameter.
 *      RT_ERR_PORT_ID          - Invalid port ID.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can delete system C2S configuration. There are 128 SVLAN C2S configurations.
 */
rtk_api_ret_t dal_rtl8373_svlanC2S_del(rtk_vlan_t vid, rtk_port_t srcPort)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 i = 0;
    rtk_port_t phyPort;
    rtk_uint32 idx_svid, idx_pmsk, idx_cvid;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (vid > RTL8373_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(srcPort);
    phyPort = rtk_switch_port_L2P_get(srcPort);

    for (i = 0; i <= RTL8373_C2SIDXMAX; i++)
    {      
        if ((retVal = rtl8373_getAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) , RTL8373_VLAN_C2S_ENTRY_SVID_ASSIGN_MASK, &idx_svid)) != RT_ERR_OK)
            return retVal;
        
        if ((retVal = rtl8373_getAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) + 4, RTK_MAX_PORT_MASK, &idx_pmsk)) != RT_ERR_OK)
            return retVal;
        
        if ((retVal = rtl8373_getAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) + 4, (0xFFF << RTK_MAX_NUM_OF_PORT), &idx_cvid)) != RT_ERR_OK)
            return retVal;

        if (idx_cvid == vid)
        {
            if(idx_pmsk & (1 << phyPort))
            {
                idx_pmsk = idx_pmsk & ~(1 << phyPort);
                if(idx_pmsk == 0)
                {
                    idx_cvid = 0;
                    idx_svid = 0;
                }
                
                if ((retVal = rtl8373_setAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) , RTL8373_VLAN_C2S_ENTRY_SVID_ASSIGN_MASK, idx_svid)) != RT_ERR_OK)
                        return retVal;
                
                if ((retVal = rtl8373_setAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) + 4, RTK_MAX_PORT_MASK, idx_pmsk)) != RT_ERR_OK)
                        return retVal;
                
                if ((retVal = rtl8373_setAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) + 4, (0xFFF << RTK_MAX_NUM_OF_PORT), idx_cvid)) != RT_ERR_OK)
                        return retVal;       

                return RT_ERR_OK;
            }
        }
    }

    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      dal_rtl8373_svlanC2S_get
 * Description:
 *      Get configure SVLAN C2S table
 * Input:
 *      vid - VLAN ID
 *      srcPort - Ingress Port
 * Output:
 *      pSvid - SVLAN ID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port ID.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *     The API can get system C2S configuration. There are 128 SVLAN C2S configurations.
 */
rtk_api_ret_t dal_rtl8373_svlanC2S_get(rtk_vlan_t vid, rtk_port_t srcPort, rtk_vlan_t *pSvid)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 i = 0;
    rtk_uint32 idx_svid, idx_pmsk, idx_cvid;
    rtk_port_t phyPort;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvid)
        return RT_ERR_NULL_POINTER;

    if (vid > RTL8373_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(srcPort);
    phyPort = rtk_switch_port_L2P_get(srcPort);

    for (i = 0; i <= RTL8373_C2SIDXMAX; i++)
    {
        if ((retVal = rtl8373_getAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) , RTL8373_VLAN_C2S_ENTRY_SVID_ASSIGN_MASK, &idx_svid)) != RT_ERR_OK)
            return retVal;
        
        if ((retVal = rtl8373_getAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) + 4, RTK_MAX_PORT_MASK, &idx_pmsk)) != RT_ERR_OK)
            return retVal;
        
        if ((retVal = rtl8373_getAsicRegBits(RTL8373_VLAN_C2S_ENTRY_ADDR(i) + 4, (0xFFF << RTK_MAX_NUM_OF_PORT), &idx_cvid)) != RT_ERR_OK)
            return retVal;

        if (idx_cvid == vid)
        {
            if(idx_pmsk & (1 << phyPort))
            {
                *pSvid = idx_svid;
                return RT_ERR_OK;
            }
        }
    }

    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      dal_rtl8373_svlanUntagAction_set
 * Description:
 *      Configure Action of downstream Un-Stag packet
 * Input:
 *      action  - Action for UnStag
 *      svid    - The SVID assigned to UnStag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can configure action of downstream Un-Stag packet. A SVID assigned
 *      to the un-stag is also supported by this API. The parameter of svid is
 *      only referenced when the action is set to UNTAG_ASSIGN
 */
rtk_api_ret_t dal_rtl8373_svlanUntagAction_set(rtk_svlan_untag_action_t action, rtk_vlan_t svid)
{
    rtk_api_ret_t   retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (action >= UNTAG_END)
        return RT_ERR_OUT_OF_RANGE;

    if(action == UNTAG_ASSIGN)
    {
        if (svid > RTL8373_VIDMAX)
            return RT_ERR_SVLAN_VID;
    }

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_VS_CTRL_ADDR, RTL8373_VS_CTRL_UNTAG_MASK, action)) != RT_ERR_OK)
        return retVal;

    if(action == UNTAG_ASSIGN)
    {
        if ((retVal = rtl8373_setAsicRegBits(RTL8373_VS_UNTAG_SVID_ADDR, RTL8373_VS_UNTAG_SVID_UNTAG_SVID_MASK, svid)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_svlanUntagAction_get
 * Description:
 *      Get Action of downstream Un-Stag packet
 * Input:
 *      None
 * Output:
 *      pAction  - Action for UnStag
 *      pSvid    - The SVID assigned to UnStag packet
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can Get action of downstream Un-Stag packet. A SVID assigned
 *      to the un-stag is also retrieved by this API. The parameter pSvid is
 *      only refernced when the action is UNTAG_ASSIGN
 */
rtk_api_ret_t dal_rtl8373_svlanUntagAction_get(rtk_svlan_untag_action_t *pAction, rtk_vlan_t *pSvid)
{
    rtk_api_ret_t   retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pAction || NULL == pSvid)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_VS_CTRL_ADDR, RTL8373_VS_CTRL_UNTAG_MASK, pAction)) != RT_ERR_OK)
        return retVal;

    if(*pAction == UNTAG_ASSIGN)
    {
        if ((retVal = rtl8373_getAsicRegBits(RTL8373_VS_UNTAG_SVID_ADDR, RTL8373_VS_UNTAG_SVID_UNTAG_SVID_MASK,  pSvid)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_svlanUnassignAction_set
 * Description:
 *      Configure Action of upstream without svid assign action
 * Input:
 *      action  - Action for Un-assign
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can configure action of upstream Un-assign svid packet. If action is not
 *      trap to CPU, the port-based SVID sure be assign as system need
 */
rtk_api_ret_t dal_rtl8373_svlanUnassignAction_set(rtk_svlan_unassign_action_t action)
{
    rtk_api_ret_t   retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (action >= UNASSIGN_END)
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8373_setAsicRegBit(RTL8373_VS_CTRL_ADDR, RTL8373_VS_CTRL_UIFSEG_OFFSET, action);

    return retVal;
}

/* Function Name:
 *      dal_rtl8373_svlanUnassignAction_get
 * Description:
 *      Get action of upstream without svid assignment
 * Input:
 *      None
 * Output:
 *      pAction  - Action for Un-assign
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_svlanUnassignAction_get(rtk_svlan_unassign_action_t *pAction)
{
    rtk_api_ret_t   retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pAction)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8373_getAsicRegBit(RTL8373_VS_CTRL_ADDR, RTL8373_VS_CTRL_UIFSEG_OFFSET, pAction);

    return retVal;
}

/* Function Name:
 *      dal_rtl8373_svlanTrapPri_set
 * Description:
 *      Set svlan trap priority
 * Input:
 *      priority - priority for trap packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_QOS_INT_PRIORITY
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_svlanTrapPri_set(rtk_pri_t priority)
{
    rtk_api_ret_t   retVal = 0;

    RTK_CHK_INIT_STATE();

    if(priority > RTL8373_PRIMAX)
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8373_setAsicRegBits(RTL8373_SVLAN_TRAP_CTRL_ADDR, RTL8373_SVLAN_TRAP_CTRL_PRI_MASK, priority);

    return retVal;
} 

/* Function Name:
 *      dal_rtl8373_svlanTrapPri_get
 * Description:
 *      Get svlan trap priority
 * Input:
 *      None
 * Output:
 *      pPriority - priority for trap packets
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_svlanTrapPri_get(rtk_pri_t *pPriority)
{
    rtk_api_ret_t   retVal = 0;

    RTK_CHK_INIT_STATE();

    if(NULL == pPriority)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8373_getAsicRegBits(RTL8373_SVLAN_TRAP_CTRL_ADDR, RTL8373_SVLAN_TRAP_CTRL_PRI_MASK, pPriority);
    
    return retVal;
}   /* end of rtk_svlan_trapPri_get */

/* Function Name:
 *      dal_rtl8373_svlanTrapCpumsk_set
 * Description:
 *      Set svlan trap cpu mask
 * Input:
 *      mask - cpu mask; bit0: internal cpu; bit1: external cpu
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_QOS_INT_PRIORITY
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_svlanTrapCpumsk_set(rtk_pri_t mask)
{
    rtk_api_ret_t   retVal = 0;

    RTK_CHK_INIT_STATE();

    if(mask > RTL8373_PRIMAX)
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8373_setAsicRegBits(RTL8373_SVLAN_TRAP_CTRL_ADDR, RTL8373_SVLAN_TRAP_CTRL_CPU_PMSK_MASK, mask);

    return retVal;
} 

/* Function Name:
 *      dal_rtl8373_svlanTrapCpumsk_get
 * Description:
 *      Get svlan trap cpu mask
 * Input:
 *      None
 * Output:
 *      pMask - traped cpu mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_svlanTrapCpumsk_get(rtk_pri_t *pMask)
{
    rtk_api_ret_t   retVal = 0;

    RTK_CHK_INIT_STATE();

    if(NULL == pMask)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8373_getAsicRegBits(RTL8373_SVLAN_TRAP_CTRL_ADDR, RTL8373_SVLAN_TRAP_CTRL_CPU_PMSK_MASK, pMask);
    
    return retVal;
}  

