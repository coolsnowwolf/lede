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
 * Purpose : RTK switch high-level API for RTL8367/RTL8367C
 * Feature : Here is a list of all functions and variables in SVLAN module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <svlan.h>
#include <vlan.h>
#include <string.h>

#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_svlan.h>

rtk_uint8               svlan_mbrCfgUsage[RTL8367C_SVIDXNO];
rtk_uint16              svlan_mbrCfgVid[RTL8367C_SVIDXNO];
rtk_svlan_lookupType_t  svlan_lookupType;
/* Function Name:
 *      rtk_svlan_init
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
 *      User can set matched ether type as service provider supported protocol.
 */
rtk_api_ret_t rtk_svlan_init(void)
{
    rtk_uint32 i;
    rtk_api_ret_t retVal;
    rtl8367c_svlan_memconf_t svlanMemConf;
    rtl8367c_svlan_s2c_t svlanSP2CConf;
    rtl8367c_svlan_mc2s_t svlanMC2SConf;
    rtk_uint32 svidx;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /*default use C-priority*/
    if ((retVal = rtl8367c_setAsicSvlanPrioritySel(SPRISEL_CTAGPRI)) != RT_ERR_OK)
        return retVal;

    /*Drop SVLAN untag frame*/
    if ((retVal = rtl8367c_setAsicSvlanIngressUntag(UNTAG_DROP)) != RT_ERR_OK)
        return retVal;

    /*Drop SVLAN unmatch frame*/
    if ((retVal = rtl8367c_setAsicSvlanIngressUnmatch(UNMATCH_DROP)) != RT_ERR_OK)
        return retVal;

    /*Set TPID to 0x88a8*/
    if ((retVal = rtl8367c_setAsicSvlanTpid(0x88a8)) != RT_ERR_OK)
        return retVal;

    /*Clean Uplink Port Mask to none*/
    if ((retVal = rtl8367c_setAsicSvlanUplinkPortMask(0)) != RT_ERR_OK)
        return retVal;

    /*Clean SVLAN Member Configuration*/
    for (i=0; i<= RTL8367C_SVIDXMAX; i++)
    {
        memset(&svlanMemConf, 0, sizeof(rtl8367c_svlan_memconf_t));
        if ((retVal = rtl8367c_setAsicSvlanMemberConfiguration(i, &svlanMemConf)) != RT_ERR_OK)
            return retVal;
    }

    /*Clean C2S Configuration*/
    for (i=0; i<= RTL8367C_C2SIDXMAX; i++)
    {
        if ((retVal = rtl8367c_setAsicSvlanC2SConf(i, 0,0,0)) != RT_ERR_OK)
            return retVal;
    }

    /*Clean SP2C Configuration*/
    for (i=0; i <= RTL8367C_SP2CMAX ; i++)
    {
        memset(&svlanSP2CConf, 0, sizeof(rtl8367c_svlan_s2c_t));
        if ((retVal = rtl8367c_setAsicSvlanSP2CConf(i, &svlanSP2CConf)) != RT_ERR_OK)
            return retVal;
    }

    /*Clean MC2S Configuration*/
    for (i=0 ; i<= RTL8367C_MC2SIDXMAX; i++)
    {
        memset(&svlanMC2SConf, 0, sizeof(rtl8367c_svlan_mc2s_t));
        if ((retVal = rtl8367c_setAsicSvlanMC2SConf(i, &svlanMC2SConf)) != RT_ERR_OK)
            return retVal;
    }


    if ((retVal = rtk_svlan_lookupType_set(SVLAN_LOOKUP_S64MBRCGF)) != RT_ERR_OK)
        return retVal;


    for (svidx = 0; svidx <= RTL8367C_SVIDXMAX; svidx++)
    {
        svlan_mbrCfgUsage[svidx] = FALSE;
    }


    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_servicePort_add
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
rtk_api_ret_t rtk_svlan_servicePort_add(rtk_port_t port)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmsk;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8367c_getAsicSvlanUplinkPortMask(&pmsk)) != RT_ERR_OK)
        return retVal;

    pmsk = pmsk | (1<<rtk_switch_port_L2P_get(port));

    if ((retVal = rtl8367c_setAsicSvlanUplinkPortMask(pmsk)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_servicePort_get
 * Description:
 *      Get service ports in the specified device.
 * Input:
 *      None
 * Output:
 *      pSvlan_portmask - pointer buffer of svlan ports.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must
 *      contain accept SVID in S-tag field.
 */
rtk_api_ret_t rtk_svlan_servicePort_get(rtk_portmask_t *pSvlan_portmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 phyMbrPmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvlan_portmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicSvlanUplinkPortMask(&phyMbrPmask)) != RT_ERR_OK)
        return retVal;

    if(rtk_switch_portmask_P2L_get(phyMbrPmask, pSvlan_portmask) != RT_ERR_OK)
        return RT_ERR_FAILED;


    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_servicePort_del
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
rtk_api_ret_t rtk_svlan_servicePort_del(rtk_port_t port)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmsk;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8367c_getAsicSvlanUplinkPortMask(&pmsk)) != RT_ERR_OK)
        return retVal;

    pmsk = pmsk & ~(1<<rtk_switch_port_L2P_get(port));

    if ((retVal = rtl8367c_setAsicSvlanUplinkPortMask(pmsk)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_tpidEntry_set
 * Description:
 *      Configure accepted S-VLAN ether type.
 * Input:
 *      svlan_tag_id - Ether type of S-tag frame parsing in uplink ports.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design.
 *      User can set matched ether type as service provider supported protocol.
 */
rtk_api_ret_t rtk_svlan_tpidEntry_set(rtk_svlan_tpid_t svlan_tag_id)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (svlan_tag_id>RTK_MAX_NUM_OF_PROTO_TYPE)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicSvlanTpid(svlan_tag_id)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_tpidEntry_get
 * Description:
 *      Get accepted S-VLAN ether type setting.
 * Input:
 *      None
 * Output:
 *      pSvlan_tag_id -  Ether type of S-tag frame parsing in uplink ports.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must
 *      contain accept SVID in S-tag field.
 */
rtk_api_ret_t rtk_svlan_tpidEntry_get(rtk_svlan_tpid_t *pSvlan_tag_id)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvlan_tag_id)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicSvlanTpid(pSvlan_tag_id)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_priorityRef_set
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
rtk_api_ret_t rtk_svlan_priorityRef_set(rtk_svlan_pri_ref_t ref)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (ref >= REF_PRI_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicSvlanPrioritySel(ref)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_priorityRef_get
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
rtk_api_ret_t rtk_svlan_priorityRef_get(rtk_svlan_pri_ref_t *pRef)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pRef)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicSvlanPrioritySel(pRef)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_memberPortEntry_set
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
 *      The API can set system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accepted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be dropped by default setup.
 *      - rtk_svlan_memberCfg_t->svid is SVID of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->memberport is member port mask of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->fid is filtering database of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->priority is priority of SVLAN member configuration.
 */
rtk_api_ret_t rtk_svlan_memberPortEntry_set(rtk_vlan_t svid, rtk_svlan_memberCfg_t *pSvlan_cfg)
{
    rtk_api_ret_t retVal;
    rtk_int32 i;
    rtk_uint32 empty_idx;
    rtl8367c_svlan_memconf_t svlanMemConf;
    rtk_uint32 phyMbrPmask;
    rtk_vlan_cfg_t vlanCfg;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvlan_cfg)
        return RT_ERR_NULL_POINTER;

    if(svid > RTL8367C_VIDMAX)
        return RT_ERR_SVLAN_VID;

    RTK_CHK_PORTMASK_VALID(&(pSvlan_cfg->memberport));

    RTK_CHK_PORTMASK_VALID(&(pSvlan_cfg->untagport));

    if (pSvlan_cfg->fiden > ENABLED)
        return RT_ERR_ENABLE;

    if (pSvlan_cfg->fid > RTL8367C_FIDMAX)
        return RT_ERR_L2_FID;

    if (pSvlan_cfg->priority > RTL8367C_PRIMAX)
        return RT_ERR_VLAN_PRIORITY;

    if (pSvlan_cfg->efiden > ENABLED)
        return RT_ERR_ENABLE;

    if (pSvlan_cfg->efid > RTL8367C_EFIDMAX)
        return RT_ERR_L2_FID;

    if(SVLAN_LOOKUP_C4KVLAN == svlan_lookupType)
    {
        if ((retVal = rtk_vlan_get(svid, &vlanCfg)) != RT_ERR_OK)
            return retVal;

        vlanCfg.mbr = pSvlan_cfg->memberport;
        vlanCfg.untag = pSvlan_cfg->untagport;

        if ((retVal = rtk_vlan_set(svid, &vlanCfg)) != RT_ERR_OK)
            return retVal;

        empty_idx = 0xFF;

        for (i = 0; i<= RTL8367C_SVIDXMAX; i++)
        {
            if (svid == svlan_mbrCfgVid[i] && TRUE == svlan_mbrCfgUsage[i])
            {
                memset(&svlanMemConf, 0, sizeof(rtl8367c_svlan_memconf_t));
                svlanMemConf.vs_svid        = svid;
                svlanMemConf.vs_efiden      = pSvlan_cfg->efiden;
                svlanMemConf.vs_efid        = pSvlan_cfg->efid;
                svlanMemConf.vs_priority    = pSvlan_cfg->priority;

                /*for create check*/
                if(0 == svlanMemConf.vs_efiden && 0 == svlanMemConf.vs_efid)
                    svlanMemConf.vs_efid = 1;

                if ((retVal = rtl8367c_setAsicSvlanMemberConfiguration(i, &svlanMemConf)) != RT_ERR_OK)
                    return retVal;

                return RT_ERR_OK;
            }
            else if (FALSE == svlan_mbrCfgUsage[i] && 0xFF == empty_idx)
            {
                empty_idx = i;
            }
        }

        if (empty_idx != 0xFF)
        {
            svlan_mbrCfgUsage[empty_idx] = TRUE;
            svlan_mbrCfgVid[empty_idx] = svid;

            memset(&svlanMemConf, 0, sizeof(rtl8367c_svlan_memconf_t));
            svlanMemConf.vs_svid        = svid;
            svlanMemConf.vs_efiden      = pSvlan_cfg->efiden;
            svlanMemConf.vs_efid        = pSvlan_cfg->efid;
            svlanMemConf.vs_priority    = pSvlan_cfg->priority;

            /*for create check*/
            if(0 == svlanMemConf.vs_efiden && 0 == svlanMemConf.vs_efid)
                svlanMemConf.vs_efid = 1;

            if ((retVal = rtl8367c_setAsicSvlanMemberConfiguration(empty_idx, &svlanMemConf)) != RT_ERR_OK)
                return retVal;

        }

        return RT_ERR_OK;
    }


    empty_idx = 0xFF;

    for (i = 0; i<= RTL8367C_SVIDXMAX; i++)
    {
        /*
        if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(i, &svlanMemConf)) != RT_ERR_OK)
            return retVal;
        */
        if (svid == svlan_mbrCfgVid[i] && TRUE == svlan_mbrCfgUsage[i])
        {
            svlanMemConf.vs_svid = svid;

            if(rtk_switch_portmask_L2P_get(&(pSvlan_cfg->memberport), &phyMbrPmask) != RT_ERR_OK)
                return RT_ERR_FAILED;

            svlanMemConf.vs_member = phyMbrPmask;

            if(rtk_switch_portmask_L2P_get(&(pSvlan_cfg->untagport), &phyMbrPmask) != RT_ERR_OK)
                return RT_ERR_FAILED;

            svlanMemConf.vs_untag = phyMbrPmask;

            svlanMemConf.vs_force_fid   = pSvlan_cfg->fiden;
            svlanMemConf.vs_fid_msti    = pSvlan_cfg->fid;
            svlanMemConf.vs_priority    = pSvlan_cfg->priority;
            svlanMemConf.vs_efiden      = pSvlan_cfg->efiden;
            svlanMemConf.vs_efid        = pSvlan_cfg->efid;

            /*all items are reset means deleting*/
            if( 0 == svlanMemConf.vs_member &&
                0 == svlanMemConf.vs_untag &&
                0 == svlanMemConf.vs_force_fid &&
                0 == svlanMemConf.vs_fid_msti &&
                0 == svlanMemConf.vs_priority &&
                0 == svlanMemConf.vs_efiden &&
                0 == svlanMemConf.vs_efid)
            {
                svlan_mbrCfgUsage[i] = FALSE;
                svlan_mbrCfgVid[i] = 0;

                /* Clear SVID also */
                svlanMemConf.vs_svid = 0;
            }
            else
            {
                svlan_mbrCfgUsage[i] = TRUE;
                svlan_mbrCfgVid[i] = svlanMemConf.vs_svid;

                if(0 == svlanMemConf.vs_svid)
                {
                    /*for create check*/
                    if(0 == svlanMemConf.vs_efiden && 0 == svlanMemConf.vs_efid)
                    {
                        svlanMemConf.vs_efid = 1;
                    }
                }
            }

            if ((retVal = rtl8367c_setAsicSvlanMemberConfiguration(i, &svlanMemConf)) != RT_ERR_OK)
                return retVal;

            return RT_ERR_OK;
        }
        else if (FALSE == svlan_mbrCfgUsage[i] && 0xFF == empty_idx)
        {
            empty_idx = i;
        }
    }

    if (empty_idx != 0xFF)
    {
        memset(&svlanMemConf, 0, sizeof(rtl8367c_svlan_memconf_t));
        svlanMemConf.vs_svid = svid;

        if(rtk_switch_portmask_L2P_get(&(pSvlan_cfg->memberport), &phyMbrPmask) != RT_ERR_OK)
            return RT_ERR_FAILED;

        svlanMemConf.vs_member = phyMbrPmask;

        if(rtk_switch_portmask_L2P_get(&(pSvlan_cfg->untagport), &phyMbrPmask) != RT_ERR_OK)
            return RT_ERR_FAILED;

        svlanMemConf.vs_untag = phyMbrPmask;

        svlanMemConf.vs_force_fid   = pSvlan_cfg->fiden;
        svlanMemConf.vs_fid_msti    = pSvlan_cfg->fid;
        svlanMemConf.vs_priority    = pSvlan_cfg->priority;

        svlanMemConf.vs_efiden      = pSvlan_cfg->efiden;
        svlanMemConf.vs_efid        = pSvlan_cfg->efid;

        /*change efid for empty svid 0*/
        if(0 == svlanMemConf.vs_svid)
        {   /*for create check*/
            if(0 == svlanMemConf.vs_efiden && 0 == svlanMemConf.vs_efid)
            {
                svlanMemConf.vs_efid = 1;
            }
        }

        svlan_mbrCfgUsage[empty_idx] = TRUE;
        svlan_mbrCfgVid[empty_idx] = svlanMemConf.vs_svid;

        if ((retVal = rtl8367c_setAsicSvlanMemberConfiguration(empty_idx, &svlanMemConf)) != RT_ERR_OK)
        {
            return retVal;
        }

        return RT_ERR_OK;
    }

    return RT_ERR_SVLAN_TABLE_FULL;
}

/* Function Name:
 *      rtk_svlan_memberPortEntry_get
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
 *      The API can get system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accepted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be dropped.
 */
rtk_api_ret_t rtk_svlan_memberPortEntry_get(rtk_vlan_t svid, rtk_svlan_memberCfg_t *pSvlan_cfg)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i;
    rtl8367c_svlan_memconf_t svlanMemConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvlan_cfg)
        return RT_ERR_NULL_POINTER;

    if (svid > RTL8367C_VIDMAX)
        return RT_ERR_SVLAN_VID;


    for (i = 0; i<= RTL8367C_SVIDXMAX; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(i, &svlanMemConf)) != RT_ERR_OK)
            return retVal;

        if (svid == svlanMemConf.vs_svid)
        {
            pSvlan_cfg->svid        = svlanMemConf.vs_svid;

            if(rtk_switch_portmask_P2L_get(svlanMemConf.vs_member,&(pSvlan_cfg->memberport)) != RT_ERR_OK)
                return RT_ERR_FAILED;

            if(rtk_switch_portmask_P2L_get(svlanMemConf.vs_untag,&(pSvlan_cfg->untagport)) != RT_ERR_OK)
                return RT_ERR_FAILED;

            pSvlan_cfg->fiden       = svlanMemConf.vs_force_fid;
            pSvlan_cfg->fid         = svlanMemConf.vs_fid_msti;
            pSvlan_cfg->priority    = svlanMemConf.vs_priority;
            pSvlan_cfg->efiden      = svlanMemConf.vs_efiden;
            pSvlan_cfg->efid        = svlanMemConf.vs_efid;

            return RT_ERR_OK;
        }
    }

    return RT_ERR_SVLAN_ENTRY_NOT_FOUND;

}

/* Function Name:
 *      rtk_svlan_memberPortEntry_adv_set
 * Description:
 *      Configure system SVLAN member by index
 * Input:
 *      idx         - Index (0 ~ 63)
 *      psvlan_cfg  - SVLAN member configuration
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
 *      The API can set system 64 accepted s-tag frame format by index.
 *      - rtk_svlan_memberCfg_t->svid is SVID of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->memberport is member port mask of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->fid is filtering database of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->priority is priority of SVLAN member configuration.
 */
rtk_api_ret_t rtk_svlan_memberPortEntry_adv_set(rtk_uint32 idx, rtk_svlan_memberCfg_t *pSvlan_cfg)
{
    rtk_api_ret_t retVal;
    rtl8367c_svlan_memconf_t svlanMemConf;
    rtk_uint32 phyMbrPmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvlan_cfg)
        return RT_ERR_NULL_POINTER;

    if (idx > RTL8367C_SVIDXMAX)
        return RT_ERR_SVLAN_ENTRY_INDEX;

    if (pSvlan_cfg->svid>RTL8367C_VIDMAX)
        return RT_ERR_SVLAN_VID;

    RTK_CHK_PORTMASK_VALID(&(pSvlan_cfg->memberport));

    RTK_CHK_PORTMASK_VALID(&(pSvlan_cfg->untagport));

    if (pSvlan_cfg->fiden > ENABLED)
        return RT_ERR_ENABLE;

    if (pSvlan_cfg->fid > RTL8367C_FIDMAX)
        return RT_ERR_L2_FID;

    if (pSvlan_cfg->priority > RTL8367C_PRIMAX)
        return RT_ERR_VLAN_PRIORITY;

    if (pSvlan_cfg->efiden > ENABLED)
        return RT_ERR_ENABLE;

    if (pSvlan_cfg->efid > RTL8367C_EFIDMAX)
        return RT_ERR_L2_FID;

    memset(&svlanMemConf, 0, sizeof(rtl8367c_svlan_memconf_t));
    svlanMemConf.vs_svid        = pSvlan_cfg->svid;
    if(rtk_switch_portmask_L2P_get(&(pSvlan_cfg->memberport), &phyMbrPmask) != RT_ERR_OK)
        return RT_ERR_FAILED;

    svlanMemConf.vs_member = phyMbrPmask;

    if(rtk_switch_portmask_L2P_get(&(pSvlan_cfg->untagport), &phyMbrPmask) != RT_ERR_OK)
        return RT_ERR_FAILED;

    svlanMemConf.vs_untag = phyMbrPmask;


    svlanMemConf.vs_force_fid   = pSvlan_cfg->fiden;
    svlanMemConf.vs_fid_msti    = pSvlan_cfg->fid;
    svlanMemConf.vs_priority    = pSvlan_cfg->priority;
    svlanMemConf.vs_efiden      = pSvlan_cfg->efiden;
    svlanMemConf.vs_efid        = pSvlan_cfg->efid;

    if(0 == svlanMemConf.vs_svid &&
        0 == svlanMemConf.vs_member &&
        0 == svlanMemConf.vs_untag &&
        0 == svlanMemConf.vs_force_fid &&
        0 == svlanMemConf.vs_fid_msti &&
        0 == svlanMemConf.vs_priority &&
        0 == svlanMemConf.vs_efiden &&
        0 == svlanMemConf.vs_efid)
    {
        svlan_mbrCfgUsage[idx] = FALSE;
        svlan_mbrCfgVid[idx] = 0;
    }
    else
    {
        svlan_mbrCfgUsage[idx] = TRUE;
        svlan_mbrCfgVid[idx] = svlanMemConf.vs_svid;
    }

    if ((retVal = rtl8367c_setAsicSvlanMemberConfiguration(idx, &svlanMemConf)) != RT_ERR_OK)
        return retVal;


    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_memberPortEntry_adv_get
 * Description:
 *      Get SVLAN member Configure by index.
 * Input:
 *      idx         - Index (0 ~ 63)
 * Output:
 *      pSvlan_cfg  - SVLAN member configuration
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can get system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accepted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be dropped.
 */
rtk_api_ret_t rtk_svlan_memberPortEntry_adv_get(rtk_uint32 idx, rtk_svlan_memberCfg_t *pSvlan_cfg)
{
    rtk_api_ret_t retVal;
    rtl8367c_svlan_memconf_t svlanMemConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvlan_cfg)
        return RT_ERR_NULL_POINTER;

    if (idx > RTL8367C_SVIDXMAX)
        return RT_ERR_SVLAN_ENTRY_INDEX;

    if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(idx, &svlanMemConf)) != RT_ERR_OK)
        return retVal;

    pSvlan_cfg->svid        = svlanMemConf.vs_svid;
    if(rtk_switch_portmask_P2L_get(svlanMemConf.vs_member,&(pSvlan_cfg->memberport)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    if(rtk_switch_portmask_P2L_get(svlanMemConf.vs_untag,&(pSvlan_cfg->untagport)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    pSvlan_cfg->fiden       = svlanMemConf.vs_force_fid;
    pSvlan_cfg->fid         = svlanMemConf.vs_fid_msti;
    pSvlan_cfg->priority    = svlanMemConf.vs_priority;
    pSvlan_cfg->efiden      = svlanMemConf.vs_efiden;
    pSvlan_cfg->efid        = svlanMemConf.vs_efid;

    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_svlan_defaultSvlan_set
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
rtk_api_ret_t rtk_svlan_defaultSvlan_set(rtk_port_t port, rtk_vlan_t svid)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i;
    rtl8367c_svlan_memconf_t svlanMemConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    /* svid must be 0~4095 */
    if (svid > RTL8367C_VIDMAX)
        return RT_ERR_SVLAN_VID;

    for (i = 0; i < RTL8367C_SVIDXNO; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(i, &svlanMemConf)) != RT_ERR_OK)
            return retVal;

        if (svid == svlanMemConf.vs_svid)
        {
            if ((retVal = rtl8367c_setAsicSvlanDefaultVlan(rtk_switch_port_L2P_get(port), i)) != RT_ERR_OK)
                return retVal;

            return RT_ERR_OK;
        }
    }

    return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
}

/* Function Name:
 *      rtk_svlan_defaultSvlan_get
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
rtk_api_ret_t rtk_svlan_defaultSvlan_get(rtk_port_t port, rtk_vlan_t *pSvid)
{
    rtk_api_ret_t retVal;
    rtk_uint32 idx;
    rtl8367c_svlan_memconf_t svlanMemConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvid)
        return RT_ERR_NULL_POINTER;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8367c_getAsicSvlanDefaultVlan(rtk_switch_port_L2P_get(port), &idx)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(idx, &svlanMemConf)) != RT_ERR_OK)
        return retVal;

    *pSvid = svlanMemConf.vs_svid;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_c2s_add
 * Description:
 *      Configure SVLAN C2S table
 * Input:
 *      vid - VLAN ID
 *      src_port - Ingress Port
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
 *      SVID to matched packet. There are 128 SVLAN C2S configurations.
 */
rtk_api_ret_t rtk_svlan_c2s_add(rtk_vlan_t vid, rtk_port_t src_port, rtk_vlan_t svid)
{
    rtk_api_ret_t retVal, i;
    rtk_uint32 empty_idx;
    rtk_uint32 evid, pmsk, svidx, c2s_svidx;
    rtl8367c_svlan_memconf_t svlanMemConf;
    rtk_port_t phyPort;
    rtk_uint16 doneFlag;


    /* Check initialization state */
    RTK_CHK_INIT_STATE();


    if (vid > RTL8367C_VIDMAX)
        return RT_ERR_VLAN_VID;

    if (svid > RTL8367C_VIDMAX)
        return RT_ERR_SVLAN_VID;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(src_port);

    phyPort = rtk_switch_port_L2P_get(src_port);

    empty_idx = 0xFFFF;
    svidx = 0xFFFF;
    doneFlag = FALSE;

    for (i = 0; i<= RTL8367C_SVIDXMAX; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(i, &svlanMemConf)) != RT_ERR_OK)
            return retVal;

        if (svid == svlanMemConf.vs_svid)
        {
            svidx = i;
            break;
        }
    }

    if (0xFFFF == svidx)
        return RT_ERR_SVLAN_VID;

    for (i=RTL8367C_C2SIDXMAX; i>=0; i--)
    {
        if ((retVal = rtl8367c_getAsicSvlanC2SConf(i, &evid, &pmsk, &c2s_svidx)) != RT_ERR_OK)
                return retVal;

        if (evid == vid)
        {
            /* Check Src_port */
            if(pmsk & (1 << phyPort))
            {
                /* Check SVIDX */
                if(c2s_svidx == svidx)
                {
                    /* All the same, do nothing */
                }
                else
                {
                    /* New svidx, remove src_port and find a new slot to add a new entry */
                    pmsk = pmsk & ~(1 << phyPort);
                    if(pmsk == 0)
                        c2s_svidx = 0;

                    if ((retVal = rtl8367c_setAsicSvlanC2SConf(i, vid, pmsk, c2s_svidx)) != RT_ERR_OK)
                        return retVal;
                }
            }
            else
            {
                if(c2s_svidx == svidx && doneFlag == FALSE)
                {
                    pmsk = pmsk | (1 << phyPort);
                    if ((retVal = rtl8367c_setAsicSvlanC2SConf(i, vid, pmsk, svidx)) != RT_ERR_OK)
                        return retVal;

                    doneFlag = TRUE;
                }
            }
        }
        else if (evid==0&&pmsk==0)
        {
            empty_idx = i;
        }
    }

    if (0xFFFF != empty_idx && doneFlag ==FALSE)
    {
       if ((retVal = rtl8367c_setAsicSvlanC2SConf(empty_idx, vid, (1<<phyPort), svidx)) != RT_ERR_OK)
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
 *      rtk_svlan_c2s_del
 * Description:
 *      Delete one C2S entry
 * Input:
 *      vid - VLAN ID
 *      src_port - Ingress Port
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
rtk_api_ret_t rtk_svlan_c2s_del(rtk_vlan_t vid, rtk_port_t src_port)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i;
    rtk_uint32 evid, pmsk, svidx;
    rtk_port_t phyPort;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (vid > RTL8367C_EVIDMAX)
        return RT_ERR_VLAN_VID;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(src_port);
    phyPort = rtk_switch_port_L2P_get(src_port);

    for (i = 0; i <= RTL8367C_C2SIDXMAX; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanC2SConf(i, &evid, &pmsk, &svidx)) != RT_ERR_OK)
            return retVal;

        if (evid == vid)
        {
            if(pmsk & (1 << phyPort))
            {
                pmsk = pmsk & ~(1 << phyPort);
                if(pmsk == 0)
                {
                    vid = 0;
                    svidx = 0;
                }

                if ((retVal = rtl8367c_setAsicSvlanC2SConf(i, vid, pmsk, svidx)) != RT_ERR_OK)
                    return retVal;

                return RT_ERR_OK;
            }
        }
    }

    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_svlan_c2s_get
 * Description:
 *      Get configure SVLAN C2S table
 * Input:
 *      vid - VLAN ID
 *      src_port - Ingress Port
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
rtk_api_ret_t rtk_svlan_c2s_get(rtk_vlan_t vid, rtk_port_t src_port, rtk_vlan_t *pSvid)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i;
    rtk_uint32 evid, pmsk, svidx;
    rtl8367c_svlan_memconf_t svlanMemConf;
    rtk_port_t phyPort;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvid)
        return RT_ERR_NULL_POINTER;

    if (vid > RTL8367C_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(src_port);
    phyPort = rtk_switch_port_L2P_get(src_port);

    for (i = 0; i <= RTL8367C_C2SIDXMAX; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanC2SConf(i, &evid, &pmsk, &svidx)) != RT_ERR_OK)
            return retVal;

        if (evid == vid)
        {
            if(pmsk & (1 << phyPort))
            {
                if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(svidx, &svlanMemConf)) != RT_ERR_OK)
                    return retVal;

                *pSvid = svlanMemConf.vs_svid;
                return RT_ERR_OK;
            }
        }
    }

    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_svlan_untag_action_set
 * Description:
 *      Configure Action of downstream UnStag packet
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
rtk_api_ret_t rtk_svlan_untag_action_set(rtk_svlan_untag_action_t action, rtk_vlan_t svid)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      i;
    rtl8367c_svlan_memconf_t svlanMemConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (action >= UNTAG_END)
        return RT_ERR_OUT_OF_RANGE;

    if(action == UNTAG_ASSIGN)
    {
        if (svid > RTL8367C_VIDMAX)
            return RT_ERR_SVLAN_VID;
    }

    if ((retVal = rtl8367c_setAsicSvlanIngressUntag((rtk_uint32)action)) != RT_ERR_OK)
        return retVal;

    if(action == UNTAG_ASSIGN)
    {
        for (i = 0; i < RTL8367C_SVIDXNO; i++)
        {
            if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(i, &svlanMemConf)) != RT_ERR_OK)
                return retVal;

            if (svid == svlanMemConf.vs_svid)
            {
                if ((retVal = rtl8367c_setAsicSvlanUntagVlan(i)) != RT_ERR_OK)
                    return retVal;

                return RT_ERR_OK;
            }
        }

        return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_untag_action_get
 * Description:
 *      Get Action of downstream UnStag packet
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
 *      only referenced when the action is UNTAG_ASSIGN
 */
rtk_api_ret_t rtk_svlan_untag_action_get(rtk_svlan_untag_action_t *pAction, rtk_vlan_t *pSvid)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      svidx;
    rtl8367c_svlan_memconf_t svlanMemConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pAction || NULL == pSvid)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicSvlanIngressUntag(pAction)) != RT_ERR_OK)
        return retVal;

    if(*pAction == UNTAG_ASSIGN)
    {
        if ((retVal = rtl8367c_getAsicSvlanUntagVlan(&svidx)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(svidx, &svlanMemConf)) != RT_ERR_OK)
            return retVal;

        *pSvid = svlanMemConf.vs_svid;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_unmatch_action_set
 * Description:
 *      Configure Action of downstream Unmatch packet
 * Input:
 *      action  - Action for Unmatch
 *      svid    - The SVID assigned to Unmatch packet
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
 *      The API can configure action of downstream Un-match packet. A SVID assigned
 *      to the un-match is also supported by this API. The parameter of svid is
 *      only referenced when the action is set to UNMATCH_ASSIGN
 */
rtk_api_ret_t rtk_svlan_unmatch_action_set(rtk_svlan_unmatch_action_t action, rtk_vlan_t svid)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      i;
    rtl8367c_svlan_memconf_t svlanMemConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (action >= UNMATCH_END)
        return RT_ERR_OUT_OF_RANGE;

    if (action == UNMATCH_ASSIGN)
    {
        if (svid > RTL8367C_VIDMAX)
            return RT_ERR_SVLAN_VID;
    }

    if ((retVal = rtl8367c_setAsicSvlanIngressUnmatch((rtk_uint32)action)) != RT_ERR_OK)
        return retVal;

    if(action == UNMATCH_ASSIGN)
    {
        for (i = 0; i < RTL8367C_SVIDXNO; i++)
        {
            if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(i, &svlanMemConf)) != RT_ERR_OK)
                return retVal;

            if (svid == svlanMemConf.vs_svid)
            {
                if ((retVal = rtl8367c_setAsicSvlanUnmatchVlan(i)) != RT_ERR_OK)
                    return retVal;

                return RT_ERR_OK;
            }
        }

        return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_unmatch_action_get
 * Description:
 *      Get Action of downstream Unmatch packet
 * Input:
 *      None
 * Output:
 *      pAction  - Action for Unmatch
 *      pSvid    - The SVID assigned to Unmatch packet
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can Get action of downstream Un-match packet. A SVID assigned
 *      to the un-match is also retrieved by this API. The parameter pSvid is
 *      only referenced when the action is UNMATCH_ASSIGN
 */
rtk_api_ret_t rtk_svlan_unmatch_action_get(rtk_svlan_unmatch_action_t *pAction, rtk_vlan_t *pSvid)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      svidx;
    rtl8367c_svlan_memconf_t svlanMemConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pAction || NULL == pSvid)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicSvlanIngressUnmatch(pAction)) != RT_ERR_OK)
        return retVal;

    if(*pAction == UNMATCH_ASSIGN)
    {
        if ((retVal = rtl8367c_getAsicSvlanUnmatchVlan(&svidx)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(svidx, &svlanMemConf)) != RT_ERR_OK)
            return retVal;

        *pSvid = svlanMemConf.vs_svid;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_unassign_action_set
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
rtk_api_ret_t rtk_svlan_unassign_action_set(rtk_svlan_unassign_action_t action)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (action >= UNASSIGN_END)
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicSvlanEgressUnassign((rtk_uint32)action);

    return retVal;
}

/* Function Name:
 *      rtk_svlan_unassign_action_get
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
rtk_api_ret_t rtk_svlan_unassign_action_get(rtk_svlan_unassign_action_t *pAction)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pAction)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8367c_getAsicSvlanEgressUnassign(pAction);

    return retVal;
}

/* Function Name:
 *      rtk_svlan_dmac_vidsel_set
 * Description:
 *      Set DMAC CVID selection
 * Input:
 *      port    - Port
 *      enable  - state of DMAC CVID Selection
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      This API can set DMAC CVID Selection state
 */
rtk_api_ret_t rtk_svlan_dmac_vidsel_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsicSvlanDmacCvidSel(rtk_switch_port_L2P_get(port), enable)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_dmac_vidsel_get
 * Description:
 *      Get DMAC CVID selection
 * Input:
 *      port    - Port
 * Output:
 *      pEnable - state of DMAC CVID Selection
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      This API can get DMAC CVID Selection state
 */
rtk_api_ret_t rtk_svlan_dmac_vidsel_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(port);

    if ((retVal = rtl8367c_getAsicSvlanDmacCvidSel(rtk_switch_port_L2P_get(port), pEnable)) != RT_ERR_OK)
            return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_ipmc2s_add
 * Description:
 *      add ip multicast address to SVLAN
 * Input:
 *      svid    - SVLAN VID
 *      ipmc    - ip multicast address
 *      ipmcMsk - ip multicast mask
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
 *      The API can set IP multicast to SVID configuration. If upstream packet is IPv4 multicast
 *      packet and DIP is matched MC2S configuration, ASIC will assign egress SVID to the packet.
 *      There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
rtk_api_ret_t rtk_svlan_ipmc2s_add(ipaddr_t ipmc, ipaddr_t ipmcMsk,rtk_vlan_t svid)
{
    rtk_api_ret_t retVal, i;
    rtk_uint32 empty_idx;
    rtk_uint32 svidx;
    rtl8367c_svlan_memconf_t svlanMemConf;
    rtl8367c_svlan_mc2s_t svlanMC2SConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (svid > RTL8367C_VIDMAX)
        return RT_ERR_SVLAN_VID;

    if ((ipmc&0xF0000000)!=0xE0000000)
        return RT_ERR_INPUT;

    svidx = 0xFFFF;

    for (i = 0; i < RTL8367C_SVIDXNO; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(i, &svlanMemConf)) != RT_ERR_OK)
            return retVal;

        if (svid == svlanMemConf.vs_svid)
        {
            svidx = i;
            break;
        }
    }

    if (0xFFFF == svidx)
            return RT_ERR_SVLAN_ENTRY_NOT_FOUND;


    empty_idx = 0xFFFF;

    for (i = RTL8367C_MC2SIDXMAX; i >= 0; i--)
    {
        if ((retVal = rtl8367c_getAsicSvlanMC2SConf(i, &svlanMC2SConf)) != RT_ERR_OK)
            return retVal;

        if (TRUE == svlanMC2SConf.valid)
        {
            if (svlanMC2SConf.format == SVLAN_MC2S_MODE_IP &&
                svlanMC2SConf.sdata==ipmc&&
                svlanMC2SConf.smask==ipmcMsk)
            {
                svlanMC2SConf.svidx = svidx;
                if ((retVal = rtl8367c_setAsicSvlanMC2SConf(i, &svlanMC2SConf)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else
        {
            empty_idx = i;
        }
    }

    if (empty_idx!=0xFFFF)
    {
        svlanMC2SConf.valid = TRUE;
        svlanMC2SConf.svidx = svidx;
        svlanMC2SConf.format = SVLAN_MC2S_MODE_IP;
        svlanMC2SConf.sdata = ipmc;
        svlanMC2SConf.smask = ipmcMsk;
        if ((retVal = rtl8367c_setAsicSvlanMC2SConf(empty_idx, &svlanMC2SConf)) != RT_ERR_OK)
            return retVal;
        return RT_ERR_OK;
    }

    return RT_ERR_OUT_OF_RANGE;

}

/* Function Name:
 *      rtk_svlan_ipmc2s_del
 * Description:
 *      delete ip multicast address to SVLAN
 * Input:
 *      ipmc    - ip multicast address
 *      ipmcMsk - ip multicast mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can delete IP multicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
rtk_api_ret_t rtk_svlan_ipmc2s_del(ipaddr_t ipmc, ipaddr_t ipmcMsk)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i;
    rtl8367c_svlan_mc2s_t svlanMC2SConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((ipmc&0xF0000000)!=0xE0000000)
        return RT_ERR_INPUT;

    for (i = 0; i <= RTL8367C_MC2SIDXMAX; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanMC2SConf(i, &svlanMC2SConf)) != RT_ERR_OK)
            return retVal;

        if (TRUE == svlanMC2SConf.valid)
        {
            if (svlanMC2SConf.format == SVLAN_MC2S_MODE_IP &&
                svlanMC2SConf.sdata==ipmc&&
                svlanMC2SConf.smask==ipmcMsk)
            {
                memset(&svlanMC2SConf, 0, sizeof(rtl8367c_svlan_mc2s_t));
                if ((retVal = rtl8367c_setAsicSvlanMC2SConf(i, &svlanMC2SConf)) != RT_ERR_OK)
                    return retVal;
                return RT_ERR_OK;
            }
        }
    }

    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_svlan_ipmc2s_get
 * Description:
 *      Get ip multicast address to SVLAN
 * Input:
 *      ipmc    - ip multicast address
 *      ipmcMsk - ip multicast mask
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The API can get IP multicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
rtk_api_ret_t rtk_svlan_ipmc2s_get(ipaddr_t ipmc, ipaddr_t ipmcMsk, rtk_vlan_t *pSvid)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i;
    rtl8367c_svlan_memconf_t svlanMemConf;
    rtl8367c_svlan_mc2s_t svlanMC2SConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvid)
        return RT_ERR_NULL_POINTER;

    if ((ipmc&0xF0000000)!=0xE0000000)
        return RT_ERR_INPUT;

    for (i = 0; i <= RTL8367C_MC2SIDXMAX; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanMC2SConf(i, &svlanMC2SConf)) != RT_ERR_OK)
            return retVal;

        if (TRUE == svlanMC2SConf.valid &&
            svlanMC2SConf.format == SVLAN_MC2S_MODE_IP &&
            svlanMC2SConf.sdata == ipmc &&
            svlanMC2SConf.smask == ipmcMsk)
        {
            if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(svlanMC2SConf.svidx, &svlanMemConf)) != RT_ERR_OK)
                return retVal;
            *pSvid = svlanMemConf.vs_svid;
            return RT_ERR_OK;
        }
    }

    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_svlan_l2mc2s_add
 * Description:
 *      Add L2 multicast address to SVLAN
 * Input:
 *      mac     - L2 multicast address
 *      macMsk  - L2 multicast address mask
 *      svid    - SVLAN VID
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
 *      The API can set L2 Multicast to SVID configuration. If upstream packet is L2 multicast
 *      packet and DMAC is matched, ASIC will assign egress SVID to the packet. There are 32
 *      SVLAN multicast configurations for IP and L2 multicast.
 */
rtk_api_ret_t rtk_svlan_l2mc2s_add(rtk_mac_t mac, rtk_mac_t macMsk, rtk_vlan_t svid)
{
    rtk_api_ret_t retVal, i;
    rtk_uint32 empty_idx;
    rtk_uint32 svidx, l2add, l2Mask;
    rtl8367c_svlan_memconf_t svlanMemConf;
    rtl8367c_svlan_mc2s_t svlanMC2SConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (svid > RTL8367C_VIDMAX)
        return RT_ERR_SVLAN_VID;

    if (mac.octet[0]!= 1&&mac.octet[1]!=0)
        return RT_ERR_INPUT;

    l2add = (mac.octet[2] << 24) | (mac.octet[3] << 16) | (mac.octet[4] << 8) | mac.octet[5];
    l2Mask = (macMsk.octet[2] << 24) | (macMsk.octet[3] << 16) | (macMsk.octet[4] << 8) | macMsk.octet[5];

    svidx = 0xFFFF;

    for (i = 0; i < RTL8367C_SVIDXNO; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(i, &svlanMemConf)) != RT_ERR_OK)
            return retVal;

        if (svid == svlanMemConf.vs_svid)
        {
            svidx = i;
            break;
        }
    }

    if (0xFFFF == svidx)
        return RT_ERR_SVLAN_ENTRY_NOT_FOUND;

    empty_idx = 0xFFFF;

    for (i = RTL8367C_MC2SIDXMAX; i >=0; i--)
    {
        if ((retVal = rtl8367c_getAsicSvlanMC2SConf(i, &svlanMC2SConf)) != RT_ERR_OK)
            return retVal;

        if (TRUE == svlanMC2SConf.valid)
        {
            if (svlanMC2SConf.format == SVLAN_MC2S_MODE_MAC &&
                svlanMC2SConf.sdata==l2add&&
                svlanMC2SConf.smask==l2Mask)
            {
                svlanMC2SConf.svidx = svidx;
                if ((retVal = rtl8367c_setAsicSvlanMC2SConf(i, &svlanMC2SConf)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else
        {
            empty_idx = i;
        }
    }

    if (empty_idx!=0xFFFF)
    {
        svlanMC2SConf.valid = TRUE;
        svlanMC2SConf.svidx = svidx;
        svlanMC2SConf.format = SVLAN_MC2S_MODE_MAC;
        svlanMC2SConf.sdata = l2add;
        svlanMC2SConf.smask = l2Mask;

        if ((retVal = rtl8367c_setAsicSvlanMC2SConf(empty_idx, &svlanMC2SConf)) != RT_ERR_OK)
            return retVal;
        return RT_ERR_OK;
    }

    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_svlan_l2mc2s_del
 * Description:
 *      delete L2 multicast address to SVLAN
 * Input:
 *      mac     - L2 multicast address
 *      macMsk  - L2 multicast address mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can delete Multicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
rtk_api_ret_t rtk_svlan_l2mc2s_del(rtk_mac_t mac, rtk_mac_t macMsk)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i;
    rtk_uint32 l2add, l2Mask;
    rtl8367c_svlan_mc2s_t svlanMC2SConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (mac.octet[0]!= 1&&mac.octet[1]!=0)
        return RT_ERR_INPUT;

    l2add = (mac.octet[2] << 24) | (mac.octet[3] << 16) | (mac.octet[4] << 8) | mac.octet[5];
    l2Mask = (macMsk.octet[2] << 24) | (macMsk.octet[3] << 16) | (macMsk.octet[4] << 8) | macMsk.octet[5];

    for (i = 0; i <= RTL8367C_MC2SIDXMAX; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanMC2SConf(i, &svlanMC2SConf)) != RT_ERR_OK)
            return retVal;

        if (TRUE == svlanMC2SConf.valid)
        {
            if (svlanMC2SConf.format == SVLAN_MC2S_MODE_MAC &&
                svlanMC2SConf.sdata==l2add&&
                svlanMC2SConf.smask==l2Mask)
            {
                memset(&svlanMC2SConf, 0, sizeof(rtl8367c_svlan_mc2s_t));
                if ((retVal = rtl8367c_setAsicSvlanMC2SConf(i, &svlanMC2SConf)) != RT_ERR_OK)
                    return retVal;
                return RT_ERR_OK;
            }
        }
    }

    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_svlan_l2mc2s_get
 * Description:
 *      Get L2 multicast address to SVLAN
 * Input:
 *      mac     - L2 multicast address
 *      macMsk  - L2 multicast address mask
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can get L2 multicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
rtk_api_ret_t rtk_svlan_l2mc2s_get(rtk_mac_t mac, rtk_mac_t macMsk, rtk_vlan_t *pSvid)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i;
    rtk_uint32 l2add,l2Mask;
    rtl8367c_svlan_memconf_t svlanMemConf;
    rtl8367c_svlan_mc2s_t svlanMC2SConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pSvid)
        return RT_ERR_NULL_POINTER;

    if (mac.octet[0]!= 1&&mac.octet[1]!=0)
        return RT_ERR_INPUT;

    l2add = (mac.octet[2] << 24) | (mac.octet[3] << 16) | (mac.octet[4] << 8) | mac.octet[5];
    l2Mask = (macMsk.octet[2] << 24) | (macMsk.octet[3] << 16) | (macMsk.octet[4] << 8) | macMsk.octet[5];

    for (i = 0; i <= RTL8367C_MC2SIDXMAX; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanMC2SConf(i, &svlanMC2SConf)) != RT_ERR_OK)
            return retVal;

        if (TRUE == svlanMC2SConf.valid)
        {
            if (svlanMC2SConf.format == SVLAN_MC2S_MODE_MAC &&
                svlanMC2SConf.sdata==l2add&&
                svlanMC2SConf.smask==l2Mask)
            {
                if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(svlanMC2SConf.svidx, &svlanMemConf)) != RT_ERR_OK)
                    return retVal;
                *pSvid = svlanMemConf.vs_svid;

                return RT_ERR_OK;
            }
        }
    }

    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_svlan_sp2c_add
 * Description:
 *      Add system SP2C configuration
 * Input:
 *      cvid        - VLAN ID
 *      dst_port    - Destination port of SVLAN to CVLAN configuration
 *      svid        - SVLAN VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can add SVID & Destination Port to CVLAN configuration. The downstream frames with assigned
 *      SVID will be add C-tag with assigned CVID if the output port is the assigned destination port.
 *      There are 128 SP2C configurations.
 */
rtk_api_ret_t rtk_svlan_sp2c_add(rtk_vlan_t svid, rtk_port_t dst_port, rtk_vlan_t cvid)
{
    rtk_api_ret_t retVal, i;
    rtk_uint32 empty_idx, svidx;
    rtl8367c_svlan_memconf_t svlanMemConf;
    rtl8367c_svlan_s2c_t svlanSP2CConf;
    rtk_port_t port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (svid > RTL8367C_VIDMAX)
        return RT_ERR_SVLAN_VID;

    if (cvid > RTL8367C_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(dst_port);
    port = rtk_switch_port_L2P_get(dst_port);

    svidx = 0xFFFF;

    for (i = 0; i < RTL8367C_SVIDXNO; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(i, &svlanMemConf)) != RT_ERR_OK)
            return retVal;

        if (svid == svlanMemConf.vs_svid)
        {
            svidx = i;
            break;
        }
    }

    if (0xFFFF == svidx)
        return RT_ERR_SVLAN_ENTRY_NOT_FOUND;

    empty_idx = 0xFFFF;

    for (i=RTL8367C_SP2CMAX; i >=0 ; i--)
    {
        if ((retVal = rtl8367c_getAsicSvlanSP2CConf(i, &svlanSP2CConf)) != RT_ERR_OK)
            return retVal;

        if ( (svlanSP2CConf.svidx == svidx) && (svlanSP2CConf.dstport == port) && (svlanSP2CConf.valid == 1))
        {
            empty_idx = i;
            break;
        }
        else if (svlanSP2CConf.valid == 0)
        {
            empty_idx = i;
        }
    }

    if (empty_idx!=0xFFFF)
    {
        svlanSP2CConf.valid     = 1;
        svlanSP2CConf.vid       = cvid;
        svlanSP2CConf.svidx     = svidx;
        svlanSP2CConf.dstport   = port;

        if ((retVal = rtl8367c_setAsicSvlanSP2CConf(empty_idx, &svlanSP2CConf)) != RT_ERR_OK)
            return retVal;
        return RT_ERR_OK;
    }

    return RT_ERR_OUT_OF_RANGE;

}

/* Function Name:
 *      rtk_svlan_sp2c_get
 * Description:
 *      Get configure system SP2C content
 * Input:
 *      svid        - SVLAN VID
 *      dst_port    - Destination port of SVLAN to CVLAN configuration
 * Output:
 *      pCvid - VLAN ID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 * Note:
 *     The API can get SVID & Destination Port to CVLAN configuration. There are 128 SP2C configurations.
 */
rtk_api_ret_t rtk_svlan_sp2c_get(rtk_vlan_t svid, rtk_port_t dst_port, rtk_vlan_t *pCvid)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i, svidx;
    rtl8367c_svlan_memconf_t svlanMemConf;
    rtl8367c_svlan_s2c_t svlanSP2CConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pCvid)
        return RT_ERR_NULL_POINTER;

    if (svid > RTL8367C_VIDMAX)
        return RT_ERR_SVLAN_VID;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(dst_port);
    dst_port = rtk_switch_port_L2P_get(dst_port);

    svidx = 0xFFFF;

    for (i = 0; i < RTL8367C_SVIDXNO; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(i, &svlanMemConf)) != RT_ERR_OK)
            return retVal;

        if (svid == svlanMemConf.vs_svid)
        {
            svidx = i;
            break;
        }
    }

    if (0xFFFF == svidx)
        return RT_ERR_SVLAN_ENTRY_NOT_FOUND;

    for (i = 0; i <= RTL8367C_SP2CMAX; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanSP2CConf(i, &svlanSP2CConf)) != RT_ERR_OK)
            return retVal;

        if ( (svlanSP2CConf.svidx == svidx) && (svlanSP2CConf.dstport == dst_port) && (svlanSP2CConf.valid == 1) )
        {
            *pCvid = svlanSP2CConf.vid;
            return RT_ERR_OK;
        }
    }

    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_svlan_sp2c_del
 * Description:
 *      Delete system SP2C configuration
 * Input:
 *      svid        - SVLAN VID
 *      dst_port    - Destination port of SVLAN to CVLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The API can delete SVID & Destination Port to CVLAN configuration. There are 128 SP2C configurations.
 */
rtk_api_ret_t rtk_svlan_sp2c_del(rtk_vlan_t svid, rtk_port_t dst_port)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i, svidx;
    rtl8367c_svlan_memconf_t svlanMemConf;
    rtl8367c_svlan_s2c_t svlanSP2CConf;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (svid > RTL8367C_VIDMAX)
        return RT_ERR_SVLAN_VID;

    /* Check port Valid */
    RTK_CHK_PORT_VALID(dst_port);
    dst_port = rtk_switch_port_L2P_get(dst_port);

    svidx = 0xFFFF;

    for (i = 0; i < RTL8367C_SVIDXNO; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanMemberConfiguration(i, &svlanMemConf)) != RT_ERR_OK)
            return retVal;

        if (svid == svlanMemConf.vs_svid)
        {
            svidx = i;
            break;
        }
    }

    if (0xFFFF == svidx)
        return RT_ERR_SVLAN_ENTRY_NOT_FOUND;

    for (i = 0; i <= RTL8367C_SP2CMAX; i++)
    {
        if ((retVal = rtl8367c_getAsicSvlanSP2CConf(i, &svlanSP2CConf)) != RT_ERR_OK)
            return retVal;

        if ( (svlanSP2CConf.svidx == svidx) && (svlanSP2CConf.dstport == dst_port) && (svlanSP2CConf.valid == 1) )
        {
            svlanSP2CConf.valid     = 0;
            svlanSP2CConf.vid       = 0;
            svlanSP2CConf.svidx     = 0;
            svlanSP2CConf.dstport   = 0;

            if ((retVal = rtl8367c_setAsicSvlanSP2CConf(i, &svlanSP2CConf)) != RT_ERR_OK)
                return retVal;
            return RT_ERR_OK;
        }

    }

    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_svlan_lookupType_set
 * Description:
 *      Set lookup type of SVLAN
 * Input:
 *      type        - lookup type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      none
 */
rtk_api_ret_t rtk_svlan_lookupType_set(rtk_svlan_lookupType_t type)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= SVLAN_LOOKUP_END)
        return RT_ERR_CHIP_NOT_SUPPORTED;


    svlan_lookupType = type;

    retVal = rtl8367c_setAsicSvlanLookupType((rtk_uint32)type);

    return retVal;
}

/* Function Name:
 *      rtk_svlan_lookupType_get
 * Description:
 *      Get lookup type of SVLAN
 * Input:
 *      pType       - lookup type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      none
 */
rtk_api_ret_t rtk_svlan_lookupType_get(rtk_svlan_lookupType_t *pType)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pType)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8367c_getAsicSvlanLookupType(pType);

    svlan_lookupType = *pType;

    return retVal;
}

/* Function Name:
 *      rtk_svlan_trapPri_set
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
rtk_api_ret_t rtk_svlan_trapPri_set(rtk_pri_t priority)
{
    rtk_api_ret_t   retVal;

    RTK_CHK_INIT_STATE();

    if(priority > RTL8367C_PRIMAX)
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicSvlanTrapPriority(priority);

    return retVal;
}   /* end of rtk_svlan_trapPri_set */

/* Function Name:
 *      rtk_svlan_trapPri_get
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
rtk_api_ret_t rtk_svlan_trapPri_get(rtk_pri_t *pPriority)
{
    rtk_api_ret_t   retVal;

    RTK_CHK_INIT_STATE();

    if(NULL == pPriority)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8367c_getAsicSvlanTrapPriority(pPriority);

    return retVal;
}   /* end of rtk_svlan_trapPri_get */


/* Function Name:
 *      rtk_svlan_checkAndCreateMbr
 * Description:
 *      Check and create Member configuration and return index
 * Input:
 *      vid  - VLAN id.
 * Output:
 *      pIndex  - Member configuration index
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_VLAN_VID     - Invalid VLAN ID.
 *      RT_ERR_TBL_FULL     - Member Configuration table full
 * Note:
 *
 */
rtk_api_ret_t rtk_svlan_checkAndCreateMbr(rtk_vlan_t vid, rtk_uint32 *pIndex)
{
    rtk_api_ret_t retVal;
    rtk_uint32 svidx;
    rtk_uint32 empty_idx = 0xFFFF;
    rtl8367c_svlan_memconf_t svlan_cfg;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* vid must be 0~4095 */
    if (vid > RTL8367C_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* Null pointer check */
    if(NULL == pIndex)
        return RT_ERR_NULL_POINTER;

    /* Search exist entry */
    for (svidx = 0; svidx <= RTL8367C_SVIDXMAX; svidx++)
    {
        if(svlan_mbrCfgUsage[svidx] == TRUE)
        {
            if(svlan_mbrCfgVid[svidx] == vid)
            {
                /* Found! return index */
                *pIndex = svidx;
                return RT_ERR_OK;
            }
        }
        else if(empty_idx == 0xFFFF)
        {
            empty_idx = svidx;
        }

    }

    if(empty_idx == 0xFFFF)
    {
        /* No empty index */
        return RT_ERR_TBL_FULL;
    }

    svlan_mbrCfgUsage[empty_idx] = TRUE;
    svlan_mbrCfgVid[empty_idx] = vid;

    memset(&svlan_cfg, 0, sizeof(rtl8367c_svlan_memconf_t));

    svlan_cfg.vs_svid = vid;
    /*for create check*/
    if(vid == 0)
    {
        svlan_cfg.vs_efid = 1;
    }

    if((retVal = rtl8367c_setAsicSvlanMemberConfiguration(empty_idx, &svlan_cfg)) != RT_ERR_OK)
        return retVal;

    *pIndex = empty_idx;
    return RT_ERR_OK;
}

