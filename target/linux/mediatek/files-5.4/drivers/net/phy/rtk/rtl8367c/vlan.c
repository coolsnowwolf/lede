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
 * Feature : Here is a list of all functions and variables in VLAN module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <vlan.h>
#include <rate.h>
#include <string.h>

#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_vlan.h>
#include <rtl8367c_asicdrv_dot1x.h>

typedef enum vlan_mbrCfgType_e
{
    MBRCFG_UNUSED = 0,
    MBRCFG_USED_BY_VLAN,
    MBRCFG_END
}vlan_mbrCfgType_t;

static rtk_vlan_t           vlan_mbrCfgVid[RTL8367C_CVIDXNO];
static vlan_mbrCfgType_t    vlan_mbrCfgUsage[RTL8367C_CVIDXNO];

/* Function Name:
 *      rtk_vlan_init
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
rtk_api_ret_t rtk_vlan_init(void)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i;
    rtl8367c_user_vlan4kentry vlan4K;
    rtl8367c_vlanconfiguser vlanMC;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Clean Database */
    memset(vlan_mbrCfgVid, 0x00, sizeof(rtk_vlan_t) * RTL8367C_CVIDXNO);
    memset(vlan_mbrCfgUsage, 0x00, sizeof(vlan_mbrCfgType_t) * RTL8367C_CVIDXNO);

    /* clean 32 VLAN member configuration */
    for (i = 0; i <= RTL8367C_CVIDXMAX; i++)
    {
        vlanMC.evid = 0;
        vlanMC.mbr = 0;
        vlanMC.fid_msti = 0;
        vlanMC.envlanpol = 0;
        vlanMC.meteridx = 0;
        vlanMC.vbpen = 0;
        vlanMC.vbpri = 0;
        if ((retVal = rtl8367c_setAsicVlanMemberConfig(i, &vlanMC)) != RT_ERR_OK)
            return retVal;
    }

    /* Set a default VLAN with vid 1 to 4K table for all ports */
    memset(&vlan4K, 0, sizeof(rtl8367c_user_vlan4kentry));
    vlan4K.vid = 1;
    vlan4K.mbr = RTK_PHY_PORTMASK_ALL;
    vlan4K.untag = RTK_PHY_PORTMASK_ALL;
    vlan4K.fid_msti = 0;
    if ((retVal = rtl8367c_setAsicVlan4kEntry(&vlan4K)) != RT_ERR_OK)
        return retVal;

    /* Also set the default VLAN to 32 member configuration index 0 */
    memset(&vlanMC, 0, sizeof(rtl8367c_vlanconfiguser));
    vlanMC.evid = 1;
    vlanMC.mbr = RTK_PHY_PORTMASK_ALL;
    vlanMC.fid_msti = 0;
    if ((retVal = rtl8367c_setAsicVlanMemberConfig(0, &vlanMC)) != RT_ERR_OK)
            return retVal;

    /* Set all ports PVID to default VLAN and tag-mode to original */
    RTK_SCAN_ALL_PHY_PORTMASK(i)
    {
        if ((retVal = rtl8367c_setAsicVlanPortBasedVID(i, 0, 0)) != RT_ERR_OK)
            return retVal;
        if ((retVal = rtl8367c_setAsicVlanEgressTagMode(i, EG_TAG_MODE_ORI)) != RT_ERR_OK)
            return retVal;
    }

    /* Updata Databse */
    vlan_mbrCfgUsage[0] = MBRCFG_USED_BY_VLAN;
    vlan_mbrCfgVid[0] = 1;

    /* Enable Ingress filter */
    RTK_SCAN_ALL_PHY_PORTMASK(i)
    {
        if ((retVal = rtl8367c_setAsicVlanIngressFilter(i, ENABLED)) != RT_ERR_OK)
            return retVal;
    }

    /* enable VLAN */
    if ((retVal = rtl8367c_setAsicVlanFilter(ENABLED)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_set
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
rtk_api_ret_t rtk_vlan_set(rtk_vlan_t vid, rtk_vlan_cfg_t *pVlanCfg)
{
    rtk_api_ret_t retVal;
    rtk_uint32 phyMbrPmask;
    rtk_uint32 phyUntagPmask;
    rtl8367c_user_vlan4kentry vlan4K;
    rtl8367c_vlanconfiguser vlanMC;
    rtk_uint32 idx;
    rtk_uint32 empty_index = 0xffff;
    rtk_uint32 update_evid = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* vid must be 0~8191 */
    if (vid > RTL8367C_EVIDMAX)
        return RT_ERR_VLAN_VID;

    /* Null pointer check */
    if(NULL == pVlanCfg)
        return RT_ERR_NULL_POINTER;

    /* Check port mask valid */
    RTK_CHK_PORTMASK_VALID(&(pVlanCfg->mbr));

    if (vid <= RTL8367C_VIDMAX)
    {
        /* Check untag port mask valid */
        RTK_CHK_PORTMASK_VALID(&(pVlanCfg->untag));
    }

    /* IVL_EN */
    if(pVlanCfg->ivl_en >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    /* fid must be 0~15 */
    if(pVlanCfg->fid_msti > RTL8367C_FIDMAX)
        return RT_ERR_L2_FID;

    /* Policing */
    if(pVlanCfg->envlanpol >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    /* Meter ID */
    if(pVlanCfg->meteridx > RTK_MAX_METER_ID)
        return RT_ERR_INPUT;

    /* VLAN based priority */
    if(pVlanCfg->vbpen >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    /* Priority */
    if(pVlanCfg->vbpri > RTL8367C_PRIMAX)
        return RT_ERR_INPUT;

    /* Get physical port mask */
    if(rtk_switch_portmask_L2P_get(&(pVlanCfg->mbr), &phyMbrPmask) != RT_ERR_OK)
        return RT_ERR_FAILED;

    if(rtk_switch_portmask_L2P_get(&(pVlanCfg->untag), &phyUntagPmask) != RT_ERR_OK)
        return RT_ERR_FAILED;

    if (vid <= RTL8367C_VIDMAX)
    {
        /* update 4K table */
        memset(&vlan4K, 0, sizeof(rtl8367c_user_vlan4kentry));
        vlan4K.vid = vid;

        vlan4K.mbr    = (phyMbrPmask & 0xFFFF);
        vlan4K.untag  = (phyUntagPmask & 0xFFFF);

        vlan4K.ivl_svl      = pVlanCfg->ivl_en;
        vlan4K.fid_msti     = pVlanCfg->fid_msti;
        vlan4K.envlanpol    = pVlanCfg->envlanpol;
        vlan4K.meteridx     = pVlanCfg->meteridx;
        vlan4K.vbpen        = pVlanCfg->vbpen;
        vlan4K.vbpri        = pVlanCfg->vbpri;

        if ((retVal = rtl8367c_setAsicVlan4kEntry(&vlan4K)) != RT_ERR_OK)
            return retVal;

        /* Update Member configuration if exist */
        for (idx = 0; idx <= RTL8367C_CVIDXMAX; idx++)
        {
            if(vlan_mbrCfgUsage[idx] == MBRCFG_USED_BY_VLAN)
            {
                if(vlan_mbrCfgVid[idx] == vid)
                {
                    /* Found! Update */
                    if(phyMbrPmask == 0x00)
                    {
                        /* Member port = 0x00, delete this VLAN from Member Configuration */
                        memset(&vlanMC, 0x00, sizeof(rtl8367c_vlanconfiguser));
                        if ((retVal = rtl8367c_setAsicVlanMemberConfig(idx, &vlanMC)) != RT_ERR_OK)
                            return retVal;

                        /* Clear Database */
                        vlan_mbrCfgUsage[idx] = MBRCFG_UNUSED;
                        vlan_mbrCfgVid[idx]   = 0;
                    }
                    else
                    {
                        /* Normal VLAN config, update to member configuration */
                        vlanMC.evid = vid;
                        vlanMC.mbr = vlan4K.mbr;
                        vlanMC.fid_msti = vlan4K.fid_msti;
                        vlanMC.meteridx = vlan4K.meteridx;
                        vlanMC.envlanpol= vlan4K.envlanpol;
                        vlanMC.vbpen = vlan4K.vbpen;
                        vlanMC.vbpri = vlan4K.vbpri;
                        if ((retVal = rtl8367c_setAsicVlanMemberConfig(idx, &vlanMC)) != RT_ERR_OK)
                            return retVal;
                    }

                    break;
                }
            }
        }
    }
    else
    {
        /* vid > 4095 */
        for (idx = 0; idx <= RTL8367C_CVIDXMAX; idx++)
        {
            if(vlan_mbrCfgUsage[idx] == MBRCFG_USED_BY_VLAN)
            {
                if(vlan_mbrCfgVid[idx] == vid)
                {
                    /* Found! Update */
                    if(phyMbrPmask == 0x00)
                    {
                        /* Member port = 0x00, delete this VLAN from Member Configuration */
                        memset(&vlanMC, 0x00, sizeof(rtl8367c_vlanconfiguser));
                        if ((retVal = rtl8367c_setAsicVlanMemberConfig(idx, &vlanMC)) != RT_ERR_OK)
                            return retVal;

                        /* Clear Database */
                        vlan_mbrCfgUsage[idx] = MBRCFG_UNUSED;
                        vlan_mbrCfgVid[idx]   = 0;
                    }
                    else
                    {
                        /* Normal VLAN config, update to member configuration */
                        vlanMC.evid = vid;
                        vlanMC.mbr = phyMbrPmask;
                        vlanMC.fid_msti = pVlanCfg->fid_msti;
                        vlanMC.meteridx = pVlanCfg->meteridx;
                        vlanMC.envlanpol= pVlanCfg->envlanpol;
                        vlanMC.vbpen = pVlanCfg->vbpen;
                        vlanMC.vbpri = pVlanCfg->vbpri;
                        if ((retVal = rtl8367c_setAsicVlanMemberConfig(idx, &vlanMC)) != RT_ERR_OK)
                            return retVal;

                        break;
                    }

                    update_evid = 1;
                }
            }

            if(vlan_mbrCfgUsage[idx] == MBRCFG_UNUSED)
            {
                if(0xffff == empty_index)
                    empty_index = idx;
            }
        }

        /* doesn't find out same EVID entry and there is empty index in member configuration */
        if( (phyMbrPmask != 0x00) && (update_evid == 0) && (empty_index != 0xFFFF) )
        {
            vlanMC.evid = vid;
            vlanMC.mbr = phyMbrPmask;
            vlanMC.fid_msti = pVlanCfg->fid_msti;
            vlanMC.meteridx = pVlanCfg->meteridx;
            vlanMC.envlanpol= pVlanCfg->envlanpol;
            vlanMC.vbpen = pVlanCfg->vbpen;
            vlanMC.vbpri = pVlanCfg->vbpri;
            if ((retVal = rtl8367c_setAsicVlanMemberConfig(empty_index, &vlanMC)) != RT_ERR_OK)
                return retVal;

            vlan_mbrCfgUsage[empty_index] = MBRCFG_USED_BY_VLAN;
            vlan_mbrCfgVid[empty_index] = vid;

        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_get
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
rtk_api_ret_t rtk_vlan_get(rtk_vlan_t vid, rtk_vlan_cfg_t *pVlanCfg)
{
    rtk_api_ret_t retVal;
    rtk_uint32 phyMbrPmask;
    rtk_uint32 phyUntagPmask;
    rtl8367c_user_vlan4kentry vlan4K;
    rtl8367c_vlanconfiguser vlanMC;
    rtk_uint32 idx;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* vid must be 0~8191 */
    if (vid > RTL8367C_EVIDMAX)
        return RT_ERR_VLAN_VID;

    /* Null pointer check */
    if(NULL == pVlanCfg)
        return RT_ERR_NULL_POINTER;

    if (vid <= RTL8367C_VIDMAX)
    {
        vlan4K.vid = vid;

        if ((retVal = rtl8367c_getAsicVlan4kEntry(&vlan4K)) != RT_ERR_OK)
            return retVal;

        phyMbrPmask   = vlan4K.mbr;
        phyUntagPmask = vlan4K.untag;
        if(rtk_switch_portmask_P2L_get(phyMbrPmask, &(pVlanCfg->mbr)) != RT_ERR_OK)
            return RT_ERR_FAILED;

        if(rtk_switch_portmask_P2L_get(phyUntagPmask, &(pVlanCfg->untag)) != RT_ERR_OK)
            return RT_ERR_FAILED;

        pVlanCfg->ivl_en    = vlan4K.ivl_svl;
        pVlanCfg->fid_msti  = vlan4K.fid_msti;
        pVlanCfg->envlanpol = vlan4K.envlanpol;
        pVlanCfg->meteridx  = vlan4K.meteridx;
        pVlanCfg->vbpen     = vlan4K.vbpen;
        pVlanCfg->vbpri     = vlan4K.vbpri;
    }
    else
    {
        for (idx = 0; idx <= RTL8367C_CVIDXMAX; idx++)
        {
            if(vlan_mbrCfgUsage[idx] == MBRCFG_USED_BY_VLAN)
            {
                if(vlan_mbrCfgVid[idx] == vid)
                {
                    if ((retVal = rtl8367c_getAsicVlanMemberConfig(idx, &vlanMC)) != RT_ERR_OK)
                        return retVal;

                    phyMbrPmask   = vlanMC.mbr;
                    if(rtk_switch_portmask_P2L_get(phyMbrPmask, &(pVlanCfg->mbr)) != RT_ERR_OK)
                        return RT_ERR_FAILED;

                    pVlanCfg->untag.bits[0] = 0;
                    pVlanCfg->ivl_en    = 0;
                    pVlanCfg->fid_msti  = vlanMC.fid_msti;
                    pVlanCfg->envlanpol = vlanMC.envlanpol;
                    pVlanCfg->meteridx  = vlanMC.meteridx;
                    pVlanCfg->vbpen     = vlanMC.vbpen;
                    pVlanCfg->vbpri     = vlanMC.vbpri;
                }
            }
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_egrFilterEnable_set
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
rtk_api_ret_t rtk_vlan_egrFilterEnable_set(rtk_enable_t egrFilter)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(egrFilter >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    /* enable VLAN */
    if ((retVal = rtl8367c_setAsicVlanFilter((rtk_uint32)egrFilter)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_egrFilterEnable_get
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
rtk_api_ret_t rtk_vlan_egrFilterEnable_get(rtk_enable_t *pEgrFilter)
{
    rtk_api_ret_t retVal;
    rtk_uint32 state;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEgrFilter)
        return RT_ERR_NULL_POINTER;

    /* enable VLAN */
    if ((retVal = rtl8367c_getAsicVlanFilter(&state)) != RT_ERR_OK)
        return retVal;

    *pEgrFilter = (rtk_enable_t)state;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_mbrCfg_set
 * Description:
 *      Set a VLAN Member Configuration entry by index.
 * Input:
 *      idx     - Index of VLAN Member Configuration.
 *      pMbrcfg - VLAN member Configuration.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *     Set a VLAN Member Configuration entry by index.
 */
rtk_api_ret_t rtk_vlan_mbrCfg_set(rtk_uint32 idx, rtk_vlan_mbrcfg_t *pMbrcfg)
{
    rtk_api_ret_t           retVal;
    rtk_uint32              phyMbrPmask;
    rtl8367c_vlanconfiguser mbrCfg;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Error check */
    if(pMbrcfg == NULL)
        return RT_ERR_NULL_POINTER;

    if(idx > RTL8367C_CVIDXMAX)
        return RT_ERR_INPUT;

    if(pMbrcfg->evid > RTL8367C_EVIDMAX)
        return RT_ERR_INPUT;

    if(pMbrcfg->fid_msti > RTL8367C_FIDMAX)
        return RT_ERR_L2_FID;

    if(pMbrcfg->envlanpol >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(pMbrcfg->meteridx > RTK_MAX_METER_ID)
        return RT_ERR_FILTER_METER_ID;

    if(pMbrcfg->vbpen >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(pMbrcfg->vbpri > RTL8367C_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    /* Check port mask valid */
    RTK_CHK_PORTMASK_VALID(&(pMbrcfg->mbr));

    mbrCfg.evid         = pMbrcfg->evid;
    mbrCfg.fid_msti     = pMbrcfg->fid_msti;
    mbrCfg.envlanpol    = pMbrcfg->envlanpol;
    mbrCfg.meteridx     = pMbrcfg->meteridx;
    mbrCfg.vbpen        = pMbrcfg->vbpen;
    mbrCfg.vbpri        = pMbrcfg->vbpri;

    if(rtk_switch_portmask_L2P_get(&(pMbrcfg->mbr), &phyMbrPmask) != RT_ERR_OK)
        return RT_ERR_FAILED;

    mbrCfg.mbr = phyMbrPmask;

    if ((retVal = rtl8367c_setAsicVlanMemberConfig(idx, &mbrCfg)) != RT_ERR_OK)
        return retVal;

    /* Update Database */
    if( (mbrCfg.evid == 0) && (mbrCfg.mbr == 0) )
    {
        vlan_mbrCfgUsage[idx] = MBRCFG_UNUSED;
        vlan_mbrCfgVid[idx] = 0;
    }
    else
    {
        vlan_mbrCfgUsage[idx] = MBRCFG_USED_BY_VLAN;
        vlan_mbrCfgVid[idx] = mbrCfg.evid;
    }

    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_vlan_mbrCfg_get
 * Description:
 *      Get a VLAN Member Configuration entry by index.
 * Input:
 *      idx - Index of VLAN Member Configuration.
 * Output:
 *      pMbrcfg - VLAN member Configuration.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *     Get a VLAN Member Configuration entry by index.
 */
rtk_api_ret_t rtk_vlan_mbrCfg_get(rtk_uint32 idx, rtk_vlan_mbrcfg_t *pMbrcfg)
{
    rtk_api_ret_t           retVal;
    rtk_uint32              phyMbrPmask;
    rtl8367c_vlanconfiguser mbrCfg;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Error check */
    if(pMbrcfg == NULL)
        return RT_ERR_NULL_POINTER;

    if(idx > RTL8367C_CVIDXMAX)
        return RT_ERR_INPUT;

    memset(&mbrCfg, 0x00, sizeof(rtl8367c_vlanconfiguser));
    if ((retVal = rtl8367c_getAsicVlanMemberConfig(idx, &mbrCfg)) != RT_ERR_OK)
        return retVal;

    pMbrcfg->evid       = mbrCfg.evid;
    pMbrcfg->fid_msti   = mbrCfg.fid_msti;
    pMbrcfg->envlanpol  = mbrCfg.envlanpol;
    pMbrcfg->meteridx   = mbrCfg.meteridx;
    pMbrcfg->vbpen      = mbrCfg.vbpen;
    pMbrcfg->vbpri      = mbrCfg.vbpri;

    phyMbrPmask = mbrCfg.mbr;
    if(rtk_switch_portmask_P2L_get(phyMbrPmask, &(pMbrcfg->mbr)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}

/* Function Name:
 *     rtk_vlan_portPvid_set
 * Description:
 *      Set port to specified VLAN ID(PVID).
 * Input:
 *      port - Port id.
 *      pvid - Specified VLAN ID.
 *      priority - 802.1p priority for the PVID.
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
rtk_api_ret_t rtk_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority)
{
    rtk_api_ret_t retVal;
    rtk_uint32 index;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    /* vid must be 0~8191 */
    if (pvid > RTL8367C_EVIDMAX)
        return RT_ERR_VLAN_VID;

    /* priority must be 0~7 */
    if (priority > RTL8367C_PRIMAX)
        return RT_ERR_VLAN_PRIORITY;

    if((retVal = rtk_vlan_checkAndCreateMbr(pvid, &index)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicVlanPortBasedVID(rtk_switch_port_L2P_get(port), index, priority)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portPvid_get
 * Description:
 *      Get VLAN ID(PVID) on specified port.
 * Input:
 *      port - Port id.
 * Output:
 *      pPvid - Specified VLAN ID.
 *      pPriority - 802.1p priority for the PVID.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *     The API can get the PVID and 802.1p priority for the PVID of Port-based VLAN.
 */
rtk_api_ret_t rtk_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t *pPriority)
{
    rtk_api_ret_t retVal;
    rtk_uint32 index, pri;
    rtl8367c_vlanconfiguser mbrCfg;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pPvid)
        return RT_ERR_NULL_POINTER;

    if(NULL == pPriority)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicVlanPortBasedVID(rtk_switch_port_L2P_get(port), &index, &pri)) != RT_ERR_OK)
        return retVal;

    memset(&mbrCfg, 0x00, sizeof(rtl8367c_vlanconfiguser));
    if ((retVal = rtl8367c_getAsicVlanMemberConfig(index, &mbrCfg)) != RT_ERR_OK)
        return retVal;

    *pPvid = mbrCfg.evid;
    *pPriority = pri;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_set
 * Description:
 *      Set VLAN ingress for each port.
 * Input:
 *      port - Port id.
 *      igr_filter - VLAN ingress function enable status.
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
rtk_api_ret_t rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igr_filter)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (igr_filter >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsicVlanIngressFilter(rtk_switch_port_L2P_get(port), igr_filter)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_get
 * Description:
 *      Get VLAN Ingress Filter
 * Input:
 *      port        - Port id.
 * Output:
 *      pIgr_filter - VLAN ingress function enable status.
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
rtk_api_ret_t rtk_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgr_filter)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pIgr_filter)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicVlanIngressFilter(rtk_switch_port_L2P_get(port), pIgr_filter)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN accept_frame_type
 * Input:
 *      port                - Port id.
 *      accept_frame_type   - accept frame type
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
rtk_api_ret_t rtk_vlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (accept_frame_type >= ACCEPT_FRAME_TYPE_END)
        return RT_ERR_VLAN_ACCEPT_FRAME_TYPE;

    if ((retVal = rtl8367c_setAsicVlanAccpetFrameType(rtk_switch_port_L2P_get(port), (rtl8367c_accframetype)accept_frame_type)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN accept_frame_type
 * Input:
 *      port - Port id.
 * Output:
 *      pAccept_frame_type - accept frame type
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
rtk_api_ret_t rtk_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type)
{
    rtk_api_ret_t retVal;
    rtl8367c_accframetype   acc_frm_type;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pAccept_frame_type)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicVlanAccpetFrameType(rtk_switch_port_L2P_get(port), &acc_frm_type)) != RT_ERR_OK)
        return retVal;

    *pAccept_frame_type = (rtk_vlan_acceptFrameType_t)acc_frm_type;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_protoAndPortBasedVlan_add
 * Description:
 *      Add the protocol-and-port-based vlan to the specified port of device.
 * Input:
 *      port  - Port id.
 *      pInfo - Protocol and port based VLAN configuration information.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_VLAN_VID         - Invalid VID parameter.
 *      RT_ERR_VLAN_PRIORITY    - Invalid priority.
 *      RT_ERR_TBL_FULL         - Table is full.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *      The frame type is shown in the following:
 *      - FRAME_TYPE_ETHERNET
 *      - FRAME_TYPE_RFC1042
 *      - FRAME_TYPE_LLCOTHER
 */
rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_add(rtk_port_t port, rtk_vlan_protoAndPortInfo_t *pInfo)
{
    rtk_api_ret_t retVal, i;
    rtk_uint32 exist, empty, used, index;
    rtl8367c_protocolgdatacfg ppb_data_cfg;
    rtl8367c_protocolvlancfg ppb_vlan_cfg;
    rtl8367c_provlan_frametype tmp;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pInfo)
        return RT_ERR_NULL_POINTER;

    if (pInfo->proto_type > RTK_MAX_NUM_OF_PROTO_TYPE)
        return RT_ERR_OUT_OF_RANGE;

    if (pInfo->frame_type >= FRAME_TYPE_END)
        return RT_ERR_OUT_OF_RANGE;

    if (pInfo->cvid > RTL8367C_VIDMAX)
        return RT_ERR_VLAN_VID;

    if (pInfo->cpri > RTL8367C_PRIMAX)
        return RT_ERR_VLAN_PRIORITY;

    exist = 0xFF;
    empty = 0xFF;
    for (i = RTL8367C_PROTOVLAN_GIDX_MAX; i >= 0; i--)
    {
        if ((retVal = rtl8367c_getAsicVlanProtocolBasedGroupData(i, &ppb_data_cfg)) != RT_ERR_OK)
            return retVal;
        tmp = pInfo->frame_type;
        if (ppb_data_cfg.etherType == pInfo->proto_type && ppb_data_cfg.frameType == tmp)
        {
            /*Already exist*/
            exist = i;
            break;
        }
        else if (ppb_data_cfg.etherType == 0 && ppb_data_cfg.frameType == 0)
        {
            /*find empty index*/
            empty = i;
        }
    }

    used = 0xFF;
    /*No empty and exist index*/
    if (0xFF == exist && 0xFF == empty)
        return RT_ERR_TBL_FULL;
    else if (exist<RTL8367C_PROTOVLAN_GROUPNO)
    {
       /*exist index*/
       used = exist;
    }
    else if (empty<RTL8367C_PROTOVLAN_GROUPNO)
    {
        /*No exist index, but have empty index*/
        ppb_data_cfg.frameType = pInfo->frame_type;
        ppb_data_cfg.etherType = pInfo->proto_type;
        if ((retVal = rtl8367c_setAsicVlanProtocolBasedGroupData(empty, &ppb_data_cfg)) != RT_ERR_OK)
            return retVal;
        used = empty;
    }
    else
        return RT_ERR_FAILED;

    if((retVal = rtk_vlan_checkAndCreateMbr(pInfo->cvid, &index)) != RT_ERR_OK)
        return retVal;

    ppb_vlan_cfg.vlan_idx = index;
    ppb_vlan_cfg.valid = TRUE;
    ppb_vlan_cfg.priority = pInfo->cpri;
    if ((retVal = rtl8367c_setAsicVlanPortAndProtocolBased(rtk_switch_port_L2P_get(port), used, &ppb_vlan_cfg)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_protoAndPortBasedVlan_get
 * Description:
 *      Get the protocol-and-port-based vlan to the specified port of device.
 * Input:
 *      port - Port id.
 *      proto_type - protocol-and-port-based vlan protocol type.
 *      frame_type - protocol-and-port-based vlan frame type.
 * Output:
 *      pInfo - Protocol and port based VLAN configuration information.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 *      RT_ERR_TBL_FULL         - Table is full.
 * Note:
 *     The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *     The frame type is shown in the following:
 *      - FRAME_TYPE_ETHERNET
 *      - FRAME_TYPE_RFC1042
 *      - FRAME_TYPE_LLCOTHER
 */
rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_get(rtk_port_t port, rtk_vlan_proto_type_t proto_type, rtk_vlan_protoVlan_frameType_t frame_type, rtk_vlan_protoAndPortInfo_t *pInfo)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i;
    rtk_uint32 ppb_idx;
    rtl8367c_protocolgdatacfg ppb_data_cfg;
    rtl8367c_protocolvlancfg ppb_vlan_cfg;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (proto_type > RTK_MAX_NUM_OF_PROTO_TYPE)
        return RT_ERR_OUT_OF_RANGE;

    if (frame_type >= FRAME_TYPE_END)
        return RT_ERR_OUT_OF_RANGE;

   ppb_idx = 0;

    for (i = 0; i<= RTL8367C_PROTOVLAN_GIDX_MAX; i++)
    {
        if ((retVal = rtl8367c_getAsicVlanProtocolBasedGroupData(i, &ppb_data_cfg)) != RT_ERR_OK)
            return retVal;

        if ( (ppb_data_cfg.frameType == (rtl8367c_provlan_frametype)frame_type) && (ppb_data_cfg.etherType == proto_type) )
        {
            ppb_idx = i;
            break;
        }
        else if (RTL8367C_PROTOVLAN_GIDX_MAX == i)
            return RT_ERR_TBL_FULL;
    }

    if ((retVal = rtl8367c_getAsicVlanPortAndProtocolBased(rtk_switch_port_L2P_get(port), ppb_idx, &ppb_vlan_cfg)) != RT_ERR_OK)
        return retVal;

    if (FALSE == ppb_vlan_cfg.valid)
        return RT_ERR_FAILED;

    pInfo->frame_type = frame_type;
    pInfo->proto_type = proto_type;
    pInfo->cvid = vlan_mbrCfgVid[ppb_vlan_cfg.vlan_idx];
    pInfo->cpri = ppb_vlan_cfg.priority;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_protoAndPortBasedVlan_del
 * Description:
 *      Delete the protocol-and-port-based vlan from the specified port of device.
 * Input:
 *      port        - Port id.
 *      proto_type  - protocol-and-port-based vlan protocol type.
 *      frame_type  - protocol-and-port-based vlan frame type.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 *      RT_ERR_TBL_FULL         - Table is full.
 * Note:
 *     The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *     The frame type is shown in the following:
 *      - FRAME_TYPE_ETHERNET
 *      - FRAME_TYPE_RFC1042
 *      - FRAME_TYPE_LLCOTHER
 */
rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_del(rtk_port_t port, rtk_vlan_proto_type_t proto_type, rtk_vlan_protoVlan_frameType_t frame_type)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i, bUsed;
    rtk_uint32 ppb_idx;
    rtl8367c_protocolgdatacfg ppb_data_cfg;
    rtl8367c_protocolvlancfg ppb_vlan_cfg;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (proto_type > RTK_MAX_NUM_OF_PROTO_TYPE)
        return RT_ERR_OUT_OF_RANGE;

    if (frame_type >= FRAME_TYPE_END)
        return RT_ERR_OUT_OF_RANGE;

   ppb_idx = 0;

    for (i = 0; i<= RTL8367C_PROTOVLAN_GIDX_MAX; i++)
    {
        if ((retVal = rtl8367c_getAsicVlanProtocolBasedGroupData(i, &ppb_data_cfg)) != RT_ERR_OK)
            return retVal;

        if ( (ppb_data_cfg.frameType == (rtl8367c_provlan_frametype)frame_type) && (ppb_data_cfg.etherType == proto_type) )
        {
            ppb_idx = i;
            ppb_vlan_cfg.valid = FALSE;
            ppb_vlan_cfg.vlan_idx = 0;
            ppb_vlan_cfg.priority = 0;
            if ((retVal = rtl8367c_setAsicVlanPortAndProtocolBased(rtk_switch_port_L2P_get(port), ppb_idx, &ppb_vlan_cfg)) != RT_ERR_OK)
                return retVal;
        }
    }

    bUsed = FALSE;
    RTK_SCAN_ALL_PHY_PORTMASK(i)
    {
        if ((retVal = rtl8367c_getAsicVlanPortAndProtocolBased(i, ppb_idx, &ppb_vlan_cfg)) != RT_ERR_OK)
            return retVal;

        if (TRUE == ppb_vlan_cfg.valid)
        {
            bUsed = TRUE;
                break;
        }
    }

    if (FALSE == bUsed) /*No Port use this PPB Index, Delete it*/
    {
        ppb_data_cfg.etherType=0;
        ppb_data_cfg.frameType=0;
        if ((retVal = rtl8367c_setAsicVlanProtocolBasedGroupData(ppb_idx, &ppb_data_cfg)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_protoAndPortBasedVlan_delAll
 * Description:
 *     Delete all protocol-and-port-based vlans from the specified port of device.
 * Input:
 *      port - Port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *     The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *     Delete all flow table protocol-and-port-based vlan entries.
 */
rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_delAll(rtk_port_t port)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i, j, bUsed[4];
    rtl8367c_protocolgdatacfg ppb_data_cfg;
    rtl8367c_protocolvlancfg ppb_vlan_cfg;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    for (i = 0; i<= RTL8367C_PROTOVLAN_GIDX_MAX; i++)
    {
        ppb_vlan_cfg.valid = FALSE;
        ppb_vlan_cfg.vlan_idx = 0;
        ppb_vlan_cfg.priority = 0;
        if ((retVal = rtl8367c_setAsicVlanPortAndProtocolBased(rtk_switch_port_L2P_get(port), i, &ppb_vlan_cfg)) != RT_ERR_OK)
            return retVal;
    }

    bUsed[0] = FALSE;
    bUsed[1] = FALSE;
    bUsed[2] = FALSE;
    bUsed[3] = FALSE;
    RTK_SCAN_ALL_PHY_PORTMASK(i)
    {
        for (j = 0; j <= RTL8367C_PROTOVLAN_GIDX_MAX; j++)
        {
            if ((retVal = rtl8367c_getAsicVlanPortAndProtocolBased(i,j, &ppb_vlan_cfg)) != RT_ERR_OK)
                return retVal;

            if (TRUE == ppb_vlan_cfg.valid)
            {
                bUsed[j] = TRUE;
            }
        }
    }

    for (i = 0; i<= RTL8367C_PROTOVLAN_GIDX_MAX; i++)
    {
        if (FALSE == bUsed[i]) /*No Port use this PPB Index, Delete it*/
        {
            ppb_data_cfg.etherType=0;
            ppb_data_cfg.frameType=0;
            if ((retVal = rtl8367c_setAsicVlanProtocolBasedGroupData(i, &ppb_data_cfg)) != RT_ERR_OK)
                return retVal;
        }
    }



    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_tagMode_set
 * Description:
 *      Set CVLAN egress tag mode
 * Input:
 *      port        - Port id.
 *      tag_mode    - The egress tag mode.
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
 *      - VLAN_TAG_MODE_ORIGINAL,
 *      - VLAN_TAG_MODE_KEEP_FORMAT,
 *      - VLAN_TAG_MODE_PRI.
 *      - VLAN_TAG_MODE_REAL_KEEP_FORMAT,
 */
rtk_api_ret_t rtk_vlan_tagMode_set(rtk_port_t port, rtk_vlan_tagMode_t tag_mode)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (tag_mode >= VLAN_TAG_MODE_END)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8367c_setAsicVlanEgressTagMode(rtk_switch_port_L2P_get(port), tag_mode)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_tagMode_get
 * Description:
 *      Get CVLAN egress tag mode
 * Input:
 *      port - Port id.
 * Output:
 *      pTag_mode - The egress tag mode.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get Egress tag mode. There are 4 mode for egress tag:
 *      - VLAN_TAG_MODE_ORIGINAL,
 *      - VLAN_TAG_MODE_KEEP_FORMAT,
 *      - VLAN_TAG_MODE_PRI.
 *      - VLAN_TAG_MODE_REAL_KEEP_FORMAT,
 */
rtk_api_ret_t rtk_vlan_tagMode_get(rtk_port_t port, rtk_vlan_tagMode_t *pTag_mode)
{
    rtk_api_ret_t retVal;
    rtl8367c_egtagmode  mode;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pTag_mode)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicVlanEgressTagMode(rtk_switch_port_L2P_get(port), &mode)) != RT_ERR_OK)
        return retVal;

    *pTag_mode = (rtk_vlan_tagMode_t)mode;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_transparent_set
 * Description:
 *      Set VLAN transparent mode
 * Input:
 *      egr_port        - Egress Port id.
 *      pIgr_pmask      - Ingress Port Mask.
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
rtk_api_ret_t rtk_vlan_transparent_set(rtk_port_t egr_port, rtk_portmask_t *pIgr_pmask)
{
     rtk_api_ret_t retVal;
     rtk_uint32    pmask;

     /* Check initialization state */
     RTK_CHK_INIT_STATE();

     /* Check Port Valid */
     RTK_CHK_PORT_VALID(egr_port);

     if(NULL == pIgr_pmask)
        return RT_ERR_NULL_POINTER;

     RTK_CHK_PORTMASK_VALID(pIgr_pmask);

     if(rtk_switch_portmask_L2P_get(pIgr_pmask, &pmask) != RT_ERR_OK)
        return RT_ERR_FAILED;

     if ((retVal = rtl8367c_setAsicVlanTransparent(rtk_switch_port_L2P_get(egr_port), pmask)) != RT_ERR_OK)
         return retVal;

     return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_transparent_get
 * Description:
 *      Get VLAN transparent mode
 * Input:
 *      egr_port        - Egress Port id.
 * Output:
 *      pIgr_pmask      - Ingress Port Mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
rtk_api_ret_t rtk_vlan_transparent_get(rtk_port_t egr_port, rtk_portmask_t *pIgr_pmask)
{
     rtk_api_ret_t retVal;
     rtk_uint32    pmask;

     /* Check initialization state */
     RTK_CHK_INIT_STATE();

     /* Check Port Valid */
     RTK_CHK_PORT_VALID(egr_port);

     if(NULL == pIgr_pmask)
        return RT_ERR_NULL_POINTER;

     if ((retVal = rtl8367c_getAsicVlanTransparent(rtk_switch_port_L2P_get(egr_port), &pmask)) != RT_ERR_OK)
         return retVal;

     if(rtk_switch_portmask_P2L_get(pmask, pIgr_pmask) != RT_ERR_OK)
        return RT_ERR_FAILED;

     return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_keep_set
 * Description:
 *      Set VLAN egress keep mode
 * Input:
 *      egr_port        - Egress Port id.
 *      pIgr_pmask      - Ingress Port Mask.
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
rtk_api_ret_t rtk_vlan_keep_set(rtk_port_t egr_port, rtk_portmask_t *pIgr_pmask)
{
     rtk_api_ret_t retVal;
     rtk_uint32    pmask;

     /* Check initialization state */
     RTK_CHK_INIT_STATE();

     /* Check Port Valid */
     RTK_CHK_PORT_VALID(egr_port);

     if(NULL == pIgr_pmask)
        return RT_ERR_NULL_POINTER;

     RTK_CHK_PORTMASK_VALID(pIgr_pmask);

     if(rtk_switch_portmask_L2P_get(pIgr_pmask, &pmask) != RT_ERR_OK)
        return RT_ERR_FAILED;

     if ((retVal = rtl8367c_setAsicVlanEgressKeep(rtk_switch_port_L2P_get(egr_port), pmask)) != RT_ERR_OK)
         return retVal;

     return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_keep_get
 * Description:
 *      Get VLAN egress keep mode
 * Input:
 *      egr_port        - Egress Port id.
 * Output:
 *      pIgr_pmask      - Ingress Port Mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
rtk_api_ret_t rtk_vlan_keep_get(rtk_port_t egr_port, rtk_portmask_t *pIgr_pmask)
{
     rtk_api_ret_t retVal;
     rtk_uint32    pmask;

     /* Check initialization state */
     RTK_CHK_INIT_STATE();

     /* Check Port Valid */
     RTK_CHK_PORT_VALID(egr_port);

     if(NULL == pIgr_pmask)
        return RT_ERR_NULL_POINTER;

     if ((retVal = rtl8367c_getAsicVlanEgressKeep(rtk_switch_port_L2P_get(egr_port), &pmask)) != RT_ERR_OK)
         return retVal;

     if(rtk_switch_portmask_P2L_get(pmask, pIgr_pmask) != RT_ERR_OK)
        return RT_ERR_FAILED;

     return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_stg_set
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
rtk_api_ret_t rtk_vlan_stg_set(rtk_vlan_t vid, rtk_stp_msti_id_t stg)
{
    rtk_api_ret_t retVal;
    rtl8367c_user_vlan4kentry vlan4K;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* vid must be 0~4095 */
    if (vid > RTL8367C_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* priority must be 0~15 */
    if (stg > RTL8367C_MSTIMAX)
        return RT_ERR_MSTI;

    /* update 4K table */
    vlan4K.vid = vid;
    if ((retVal = rtl8367c_getAsicVlan4kEntry(&vlan4K)) != RT_ERR_OK)
        return retVal;

    vlan4K.fid_msti= stg;
    if ((retVal = rtl8367c_setAsicVlan4kEntry(&vlan4K)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_stg_get
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
rtk_api_ret_t rtk_vlan_stg_get(rtk_vlan_t vid, rtk_stp_msti_id_t *pStg)
{
    rtk_api_ret_t retVal;
    rtl8367c_user_vlan4kentry vlan4K;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* vid must be 0~4095 */
    if (vid > RTL8367C_VIDMAX)
        return RT_ERR_VLAN_VID;

    if(NULL == pStg)
        return RT_ERR_NULL_POINTER;

    /* update 4K table */
    vlan4K.vid = vid;
    if ((retVal = rtl8367c_getAsicVlan4kEntry(&vlan4K)) != RT_ERR_OK)
        return retVal;

    *pStg = vlan4K.fid_msti;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portFid_set
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
rtk_api_ret_t rtk_vlan_portFid_set(rtk_port_t port, rtk_enable_t enable, rtk_fid_t fid)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    /* fid must be 0~4095 */
    if (fid > RTK_FID_MAX)
        return RT_ERR_L2_FID;

    if ((retVal = rtl8367c_setAsicPortBasedFidEn(rtk_switch_port_L2P_get(port), enable))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicPortBasedFid(rtk_switch_port_L2P_get(port), fid))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portFid_get
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
rtk_api_ret_t rtk_vlan_portFid_get(rtk_port_t port, rtk_enable_t *pEnable, rtk_fid_t *pFid)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if(NULL == pFid)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortBasedFidEn(rtk_switch_port_L2P_get(port), pEnable))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicPortBasedFid(rtk_switch_port_L2P_get(port), pFid))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_UntagDscpPriorityEnable_set
 * Description:
 *      Set Untag DSCP priority assign
 * Input:
 *      enable - state of Untag DSCP priority assign
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_ENABLE          - Invalid input parameters.
 * Note:
 *
 */
rtk_api_ret_t rtk_vlan_UntagDscpPriorityEnable_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsicVlanUntagDscpPriorityEn((rtk_uint32)enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_UntagDscpPriorityEnable_get
 * Description:
 *      Get Untag DSCP priority assign
 * Input:
 *      None
 * Output:
 *      pEnable - state of Untag DSCP priority assign
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_NULL_POINTER    - Null pointer
 * Note:
 *
 */
rtk_api_ret_t rtk_vlan_UntagDscpPriorityEnable_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32  value;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicVlanUntagDscpPriorityEn(&value)) != RT_ERR_OK)
        return retVal;

    *pEnable = (rtk_enable_t)value;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_stp_mstpState_set
 * Description:
 *      Configure spanning tree state per each port.
 * Input:
 *      port - Port id
 *      msti - Multiple spanning tree instance.
 *      stp_state - Spanning tree state for msti
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_MSTI         - Invalid msti parameter.
 *      RT_ERR_MSTP_STATE   - Invalid STP state.
 * Note:
 *      System supports per-port multiple spanning tree state for each msti.
 *      There are four states supported by ASIC.
 *      - STP_STATE_DISABLED
 *      - STP_STATE_BLOCKING
 *      - STP_STATE_LEARNING
 *      - STP_STATE_FORWARDING
 */
rtk_api_ret_t rtk_stp_mstpState_set(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t stp_state)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (msti > RTK_MAX_NUM_OF_MSTI)
        return RT_ERR_MSTI;

    if (stp_state >= STP_STATE_END)
        return RT_ERR_MSTP_STATE;

    if ((retVal = rtl8367c_setAsicSpanningTreeStatus(rtk_switch_port_L2P_get(port), msti, stp_state)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_stp_mstpState_get
 * Description:
 *      Get spanning tree state per each port.
 * Input:
 *      port - Port id.
 *      msti - Multiple spanning tree instance.
 * Output:
 *      pStp_state - Spanning tree state for msti
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_MSTI         - Invalid msti parameter.
 * Note:
 *      System supports per-port multiple spanning tree state for each msti.
 *      There are four states supported by ASIC.
 *      - STP_STATE_DISABLED
 *      - STP_STATE_BLOCKING
 *      - STP_STATE_LEARNING
 *      - STP_STATE_FORWARDING
 */
rtk_api_ret_t rtk_stp_mstpState_get(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t *pStp_state)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (msti > RTK_MAX_NUM_OF_MSTI)
        return RT_ERR_MSTI;

    if(NULL == pStp_state)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicSpanningTreeStatus(rtk_switch_port_L2P_get(port), msti, pStp_state)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_checkAndCreateMbr
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
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - VLAN not found
 *      RT_ERR_TBL_FULL     - Member Configuration table full
 * Note:
 *
 */
rtk_api_ret_t rtk_vlan_checkAndCreateMbr(rtk_vlan_t vid, rtk_uint32 *pIndex)
{
    rtk_api_ret_t retVal;
    rtl8367c_user_vlan4kentry vlan4K;
    rtl8367c_vlanconfiguser vlanMC;
    rtk_uint32 idx;
    rtk_uint32 empty_idx = 0xFFFF;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* vid must be 0~8191 */
    if (vid > RTL8367C_EVIDMAX)
        return RT_ERR_VLAN_VID;

    /* Null pointer check */
    if(NULL == pIndex)
        return RT_ERR_NULL_POINTER;

    /* Get 4K VLAN */
    if (vid <= RTL8367C_VIDMAX)
    {
        memset(&vlan4K, 0x00, sizeof(rtl8367c_user_vlan4kentry));
        vlan4K.vid = vid;
        if ((retVal = rtl8367c_getAsicVlan4kEntry(&vlan4K)) != RT_ERR_OK)
            return retVal;
    }

    /* Search exist entry */
    for (idx = 0; idx <= RTL8367C_CVIDXMAX; idx++)
    {
        if(vlan_mbrCfgUsage[idx] == MBRCFG_USED_BY_VLAN)
        {
            if(vlan_mbrCfgVid[idx] == vid)
            {
                /* Found! return index */
                *pIndex = idx;
                return RT_ERR_OK;
            }
        }
    }

    /* Not found, Read H/W Member Configuration table to update database */
    for (idx = 0; idx <= RTL8367C_CVIDXMAX; idx++)
    {
        if ((retVal = rtl8367c_getAsicVlanMemberConfig(idx, &vlanMC)) != RT_ERR_OK)
            return retVal;

        if( (vlanMC.evid == 0) && (vlanMC.mbr == 0x00))
        {
            vlan_mbrCfgUsage[idx]   = MBRCFG_UNUSED;
            vlan_mbrCfgVid[idx]     = 0;
        }
        else
        {
            vlan_mbrCfgUsage[idx]   = MBRCFG_USED_BY_VLAN;
            vlan_mbrCfgVid[idx]     = vlanMC.evid;
        }
    }

    /* Search exist entry again */
    for (idx = 0; idx <= RTL8367C_CVIDXMAX; idx++)
    {
        if(vlan_mbrCfgUsage[idx] == MBRCFG_USED_BY_VLAN)
        {
            if(vlan_mbrCfgVid[idx] == vid)
            {
                /* Found! return index */
                *pIndex = idx;
                return RT_ERR_OK;
            }
        }
    }

    /* try to look up an empty index */
    for (idx = 0; idx <= RTL8367C_CVIDXMAX; idx++)
    {
        if(vlan_mbrCfgUsage[idx] == MBRCFG_UNUSED)
        {
            empty_idx = idx;
            break;
        }
    }

    if(empty_idx == 0xFFFF)
    {
        /* No empty index */
        return RT_ERR_TBL_FULL;
    }

    if (vid > RTL8367C_VIDMAX)
    {
        /* > 4K, there is no 4K entry, create on member configuration directly */
        memset(&vlanMC, 0x00, sizeof(rtl8367c_vlanconfiguser));
        vlanMC.evid = vid;
        if ((retVal = rtl8367c_setAsicVlanMemberConfig(empty_idx, &vlanMC)) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        /* Copy from 4K table */
        vlanMC.evid = vid;
        vlanMC.mbr = vlan4K.mbr;
        vlanMC.fid_msti = vlan4K.fid_msti;
        vlanMC.meteridx= vlan4K.meteridx;
        vlanMC.envlanpol= vlan4K.envlanpol;
        vlanMC.vbpen = vlan4K.vbpen;
        vlanMC.vbpri = vlan4K.vbpri;
        if ((retVal = rtl8367c_setAsicVlanMemberConfig(empty_idx, &vlanMC)) != RT_ERR_OK)
            return retVal;
    }

    /* Update Database */
    vlan_mbrCfgUsage[empty_idx] = MBRCFG_USED_BY_VLAN;
    vlan_mbrCfgVid[empty_idx] = vid;

    *pIndex = empty_idx;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_reservedVidAction_set
 * Description:
 *      Set Action of VLAN ID = 0 & 4095 tagged packet
 * Input:
 *      action_vid0     - Action for VID 0.
 *      action_vid4095  - Action for VID 4095.
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
rtk_api_ret_t rtk_vlan_reservedVidAction_set(rtk_vlan_resVidAction_t action_vid0, rtk_vlan_resVidAction_t action_vid4095)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(action_vid0 >= RESVID_ACTION_END)
        return RT_ERR_INPUT;

    if(action_vid4095 >= RESVID_ACTION_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setReservedVidAction((rtk_uint32)action_vid0, (rtk_uint32)action_vid4095)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_reservedVidAction_get
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
rtk_api_ret_t rtk_vlan_reservedVidAction_get(rtk_vlan_resVidAction_t *pAction_vid0, rtk_vlan_resVidAction_t *pAction_vid4095)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(pAction_vid0 == NULL)
        return RT_ERR_NULL_POINTER;

    if(pAction_vid4095 == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getReservedVidAction((rtk_uint32 *)pAction_vid0, (rtk_uint32 *)pAction_vid4095)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_realKeepRemarkEnable_set
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
rtk_api_ret_t rtk_vlan_realKeepRemarkEnable_set(rtk_enable_t enabled)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(enabled >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setRealKeepRemarkEn((rtk_uint32)enabled)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_realKeepRemarkEnable_get
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
rtk_api_ret_t rtk_vlan_realKeepRemarkEnable_get(rtk_enable_t *pEnabled)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnabled)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getRealKeepRemarkEn((rtk_uint32 *)pEnabled)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_reset
 * Description:
 *      Reset VLAN
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
rtk_api_ret_t rtk_vlan_reset(void)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((retVal = rtl8367c_resetVlan()) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

