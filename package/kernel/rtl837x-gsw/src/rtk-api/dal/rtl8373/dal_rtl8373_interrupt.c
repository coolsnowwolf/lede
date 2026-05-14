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
 * Purpose : RTK switch high-level API for RTL8367/RTL8373
 * Feature : Here is a list of all functions and variables in interrupt module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal_rtl8373_interrupt.h>
#include <rtl8373_asicdrv.h>
#include <string.h>



/* Function Name:
 *      dal_rtl8373_intMode_set
 * Description:
 *      Set interrupt mode
 * Input:
 *      mode : 0 high;   1: low;  2 positive pulse;  3 negative pulse
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_intMode_set(rtk_int_polarity_t mode)
{
    rtk_api_ret_t retVal;


    if ((retVal = rtl8373_setAsicRegBits(RTL8373_ISR_SW_INT_MODE_ADDR, RTL8373_ISR_SW_INT_MODE_SW_INT_MODE_MASK, mode)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_intMode_get
 * Description:
 *      Set interrupt mode
 * Input:
 *      pMode : 0 high;   1: low;  2 positive pulse;  3 negative pulse
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_intMode_get(rtk_int_polarity_t* pMode)
{
    rtk_api_ret_t retVal;


    if ((retVal = rtl8373_getAsicRegBits(RTL8373_ISR_SW_INT_MODE_ADDR, RTL8373_ISR_SW_INT_MODE_SW_INT_MODE_MASK, pMode)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_portLinkChgIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_portLinkChgIMR_set(rtk_uint32 type, rtk_uint32 port, rtk_uint32 enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_PORT_LINK_STS_CHG_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << port);
        else
            regData &= ~(1 << port);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_INT_PORT_LINK_STS_CHG_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_PORT_LINK_STS_CHG_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << port);
        else
            regData &= ~(1 << port);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_INT_PORT_LINK_STS_CHG_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portLinkChgIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portLinkChgIMR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_PORT_LINK_STS_CHG_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> port) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_PORT_LINK_STS_CHG_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> port) & 1;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_gphyIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      gphy: gphy id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_gphyIMR_set(rtk_uint32 type, rtk_uint32 phy, rtk_uint32 enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_GPHY_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << phy);
        else
            regData &= ~(1 << phy);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_INT_GPHY_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_GPHY_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << phy);
        else
            regData &= ~(1 << phy);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_EXT_GPHY_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_gphyIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_gphyIMR_get(rtk_uint32 type, rtk_uint32 phy, rtk_uint32* pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_GPHY_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> phy) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_GPHY_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> phy) & 1;
    }
    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_portLrnOverIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_portLrnOverIMR_set(rtk_uint32 type, rtk_uint32 port, rtk_uint32 enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_LEARNOVER_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << port);
        else
            regData &= ~(1 << port);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_INT_LEARNOVER_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_LEARNOVER_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << port);
        else
            regData &= ~(1 << port);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_EXT_LEARNOVER_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portLrnOverIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portLrnOverIMR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_LEARNOVER_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> port) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_LEARNOVER_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> port) & 1;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portRLFDIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_portRLFDIMR_set(rtk_uint32 type, rtk_uint32 port, rtk_uint32 enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_RLFD_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << port);
        else
            regData &= ~(1 << port);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_INT_RLFD_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_RLFD_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << port);
        else
            regData &= ~(1 << port);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_EXT_RLFD_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portRLFDIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portRLFDIMR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_RLFD_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> port) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_RLFD_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> port) & 1;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portWolIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_portWolIMR_set(rtk_uint32 type, rtk_uint32 port, rtk_uint32 enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_WOL_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << port);
        else
            regData &= ~(1 << port);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_INT_WOL_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_WOL_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << port);
        else
            regData &= ~(1 << port);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_EXT_WOL_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portWolIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portWolIMR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_WOL_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> port) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_WOL_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> port) & 1;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portSdsLnkFltIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_portSdsLnkFltIMR_set(rtk_uint32 type, rtk_uint32 sds, rtk_uint32 enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_SERDES_LINK_FAULT_P_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << sds);
        else
            regData &= ~(1 << sds);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_INT_SERDES_LINK_FAULT_P_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_SERDES_LINK_FAULT_P_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << sds);
        else
            regData &= ~(1 << sds);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_EXT_SERDES_LINK_FAULT_P_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portSdsLnkFltIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portSdsLnkFltIMR_get(rtk_uint32 type, rtk_uint32 sds, rtk_uint32* pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_SERDES_LINK_FAULT_P_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> sds) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_SERDES_LINK_FAULT_P_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> sds) & 1;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portSdsUpdPhyIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_portSdsUpdPhyIMR_set(rtk_uint32 type, rtk_uint32 sds, rtk_uint32 enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_SDS_UPD_PHYSTS0_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << sds);
        else
            regData &= ~(1 << sds);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_INT_SDS_UPD_PHYSTS0_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_SDS_UPD_PHYSTS0_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << sds);
        else
            regData &= ~(1 << sds);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_EXT_SDS_UPD_PHYSTS0_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portSdsUpdPhyIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portSdsUpdPhyIMR_get(rtk_uint32 type, rtk_uint32 sds, rtk_uint32* pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_SDS_UPD_PHYSTS0_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> sds) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_SDS_UPD_PHYSTS0_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> sds) & 1;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_gpioIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_gpioIMR_set(rtk_uint32 type, rtk_uint32 gpio, rtk_uint32 enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_GPIO_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << gpio);
        else
            regData &= ~(1 << gpio);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_INT_GPIO_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_GPIO_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        if(enable == 1)
            regData |= (1 << gpio);
        else
            regData &= ~(1 << gpio);

        if ((retVal = rtl8373_setAsicReg(RTL8373_IMR_EXT_GPIO_ADDR, regData)) != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_gpioIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_gpioIMR_get(rtk_uint32 type, rtk_uint32 gpio, rtk_uint32* pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_INT_GPIO_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> gpio) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_IMR_EXT_GPIO_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pEnable = (regData >> gpio) & 1;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_miscIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_miscIMR_set(rtk_uint32 type, interrupt_misc_t interrupt, rtk_uint32 enable)
{
    rtk_api_ret_t retVal;

    if(type == 0)
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, interrupt, enable)) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, interrupt, enable)) != RT_ERR_OK)
            return retVal;
    }    

#if 0
    /*internal interrupt*/
    if(type == 0)
    {
        if (interrupt == TM_HIGH)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_TM_HIGH_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_TM_HIGH_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == TM_LOW)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_TM_LOW_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_TM_LOW_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SMI_CHECK_REG_0)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 2), 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 2), 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SMI_CHECK_REG_1)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 3), 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 3), 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SMI_CHECK_REG_2)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 4), 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 4), 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SMI_CHECK_REG_3)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 5), 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 5), 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SMI_CHECK_REG_4)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 6), 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 6), 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SDS_RX_SYM_ERR_0)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 7), 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 7), 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SDS_RX_SYM_ERR_1)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 8), 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 8), 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SAMOVE)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_SAMOVE_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_SAMOVE_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == AUTO_REC)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_AUTO_REC_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_AUTO_REC_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == ACL)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_ACL_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_ACL_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == LOOP_DETEC)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_LOOP_DETECTION_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_LOOP_DETECTION_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == METER_EXCEED)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_METER_EXCEED_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_METER_EXCEED_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == ROUT_PBUF)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_ROUT_PBUF_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_ROUT_PBUF_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == PTP1588)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_PTP1588_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_PTP1588_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
    }
    else  /*external interrupt*/
    {
        if (interrupt == TM_HIGH)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_TM_HIGH_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_TM_HIGH_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == TM_LOW)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_TM_LOW_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_TM_LOW_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SMI_CHECK_REG_0)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 2), 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 2), 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SMI_CHECK_REG_1)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 3), 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 3), 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SMI_CHECK_REG_2)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 4), 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 4), 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SMI_CHECK_REG_3)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 5), 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 5), 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SMI_CHECK_REG_4)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 6), 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 6), 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SDS_RX_SYM_ERR_0)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 7), 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 7), 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SDS_RX_SYM_ERR_1)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 8), 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 8), 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == SAMOVE)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_SAMOVE_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_SAMOVE_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == AUTO_REC)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_AUTO_REC_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_AUTO_REC_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == ACL)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_ACL_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_ACL_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == INCPU)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_8051_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_8051_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == LOOP_DETEC)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_LOOP_DETECTION_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_LOOP_DETECTION_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == METER_EXCEED)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_METER_EXCEED_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_METER_EXCEED_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == ROUT_PBUF)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_ROUT_PBUF_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_ROUT_PBUF_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if (interrupt == PTP1588)
        {
            if(enable == 1)
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_PTP1588_OFFSET, 1)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8373_setAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_PTP1588_OFFSET, 0)) != RT_ERR_OK)
                    return retVal;
            }
        }
    }

#endif    
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_miscIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_miscIMR_get(rtk_uint32 type, interrupt_misc_t interrupt, rtk_uint32* pEnable)
{
    rtk_api_ret_t retVal;


    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, interrupt, pEnable)) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, interrupt, pEnable)) != RT_ERR_OK)
            return retVal;
    } 

#if 0
    /*internal interrupt*/
    if(type == 0)
    {
        if (interrupt == TM_HIGH)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_TM_HIGH_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == TM_LOW)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_TM_LOW_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_0)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 2), pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_1)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 3), pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_2)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 4), pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_3)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 5), pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_4)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 6), pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SDS_RX_SYM_ERR_0)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 7), pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SDS_RX_SYM_ERR_1)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, (1 << 8), pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SAMOVE)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_SAMOVE_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == AUTO_REC)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_AUTO_REC_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
        
        }
        else if (interrupt == ACL)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_ACL_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;

        }
        else if (interrupt == LOOP_DETEC)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_LOOP_DETECTION_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == METER_EXCEED)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_METER_EXCEED_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == ROUT_PBUF)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_ROUT_PBUF_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;

        }
        else if (interrupt == PTP1588)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_INT_MISC_ADDR, RTL8373_IMR_INT_MISC_IMR_INT_PTP1588_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;

        }
    }
    else  /*external interrupt*/
    {
        if (interrupt == TM_HIGH)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_TM_HIGH_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == TM_LOW)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_TM_LOW_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_0)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 2), pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_1)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 3), pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_2)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 4), pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_3)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 5), pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_4)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 6), pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SDS_RX_SYM_ERR_0)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 7), pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SDS_RX_SYM_ERR_1)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, (1 << 8), pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SAMOVE)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_SAMOVE_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == AUTO_REC)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_AUTO_REC_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
        
        }
        else if (interrupt == ACL)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_ACL_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;

        }
        else if (interrupt == INCPU)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_ACL_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;

        }
        else if (interrupt == LOOP_DETEC)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_LOOP_DETECTION_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == METER_EXCEED)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_METER_EXCEED_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == ROUT_PBUF)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_ROUT_PBUF_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;

        }
        else if (interrupt == PTP1588)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_IMR_EXT_MISC_ADDR, RTL8373_IMR_EXT_MISC_IMR_EXT_PTP1588_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;

        }
    }

#endif    
    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_portLinkChgISR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     pStatus: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portLinkChgISR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pStatus)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_INT_PORT_LINK_STS_CHG_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> port) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_EXT_PORT_LINK_STS_CHG_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> port) & 1;
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_portLinkChgISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portLinkChgISR_clear(rtk_uint32 type, rtk_uint32 port)
{
    rtk_api_ret_t retVal;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_INT_PORT_LINK_STS_CHG_ADDR, port,1)) != RT_ERR_OK)
            return retVal;

    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_EXT_PORT_LINK_STS_CHG_ADDR, port,1)) != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_gphyIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_gphyISR_get(rtk_uint32 type, rtk_uint32 phy, rtk_uint32* pStatus)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_INT_GPHY_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> phy) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_EXT_GPHY_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> phy) & 1;
    }
    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_gphyISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_gphyISR_clear(rtk_uint32 type, rtk_uint32 phy)
{
    rtk_api_ret_t retVal;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_INT_GPHY_ADDR, phy, 1)) != RT_ERR_OK)
            return retVal;

    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_EXT_GPHY_ADDR, phy, 1)) != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_portLrnOverIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portLrnOverISR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pStatus)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_INT_LEARNOVER_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> port) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_EXT_LEARNOVER_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> port) & 1;
    }
    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_portLrnOverISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portLrnOverISR_clear(rtk_uint32 type, rtk_uint32 port)
{
    rtk_api_ret_t retVal;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_INT_LEARNOVER_ADDR, port, 1)) != RT_ERR_OK)
            return retVal;

    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_EXT_LEARNOVER_ADDR, port, 1)) != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_portRLFDIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portRLFDISR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pStatus)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_INT_TM_RLFD_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> port) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_EXT_TM_RLFD_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> port) & 1;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portRLFDISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portRLFDISR_clear(rtk_uint32 type, rtk_uint32 port)
{
    rtk_api_ret_t retVal;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_INT_TM_RLFD_ADDR, port, 1)) != RT_ERR_OK)
            return retVal;

    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_EXT_TM_RLFD_ADDR, port, 1)) != RT_ERR_OK)
            return retVal;

    }
    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_portWolIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portWolISR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pStatus)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_INT_WOL_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> port) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_EXT_WOL_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> port) & 1;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portWolISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portWolISR_clear(rtk_uint32 type, rtk_uint32 port)
{
    rtk_api_ret_t retVal;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_INT_WOL_ADDR, port, 1)) != RT_ERR_OK)
            return retVal;

    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_EXT_WOL_ADDR, port, 1)) != RT_ERR_OK)
            return retVal;

    }
    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_portSdsLnkFltIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portSdsLnkFltISR_get(rtk_uint32 type, rtk_uint32 sds, rtk_uint32* pStatus)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_INT_SERDES_LINK_FAULT_P_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> sds) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_EXT_SERDES_LINK_FAULT_P_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> sds) & 1;
    }
    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_portSdsLnkFltISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portSdsLnkFltISR_clear(rtk_uint32 type, rtk_uint32 sds)
{
    rtk_api_ret_t retVal;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_INT_SERDES_LINK_FAULT_P_ADDR, sds, 1)) != RT_ERR_OK)
            return retVal;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_EXT_SERDES_LINK_FAULT_P_ADDR, sds, 1)) != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}





/* Function Name:
 *      dal_rtl8373_portSdsUpdPhyIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portSdsUpdPhyISR_get(rtk_uint32 type, rtk_uint32 sds, rtk_uint32* pStatus)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_INT_SDS_UPD_PHYSTS0_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> sds) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_EXT_SDS_UPD_PHYSTS0_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> sds) & 1;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portSdsUpdPhyISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portSdsUpdPhyISR_clear(rtk_uint32 type, rtk_uint32 sds)
{
    rtk_api_ret_t retVal;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_EXT_SDS_UPD_PHYSTS0_ADDR, sds, 1)) != RT_ERR_OK)
            return retVal;

    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_EXT_SDS_UPD_PHYSTS0_ADDR, sds, 1)) != RT_ERR_OK)
            return retVal;

    }
    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_gpioIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_gpioISR_get(rtk_uint32 type, rtk_uint32 gpio, rtk_uint32* pStatus)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_INT_GPIO_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> gpio) & 1;
    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicReg(RTL8373_ISR_EXT_GPIO_ADDR, &regData)) != RT_ERR_OK)
            return retVal;

        *pStatus = (regData >> gpio) & 1;
    }
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_gpioISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_gpioISR_clear(rtk_uint32 type, rtk_uint32 gpio)
{
    rtk_api_ret_t retVal;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_INT_GPIO_ADDR, gpio, 1)) != RT_ERR_OK)
            return retVal;

    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_EXT_GPIO_ADDR,  gpio, 1)) != RT_ERR_OK)
            return retVal;

    }
    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_miscIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_miscISR_get(rtk_uint32 type, interrupt_misc_t interrupt, rtk_uint32* pStatus)
{
    rtk_api_ret_t retVal;

    if (type == 0)
    {
        if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, interrupt, pStatus)) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, interrupt, pStatus)) != RT_ERR_OK)
            return retVal;
    }


#if 0
    /*internal interrupt*/
    if(type == 0)
    {
        if (interrupt == TM_HIGH)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, RTL8373_ISR_INT_MISC_ISR_EXT_TM_HIGH_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == TM_LOW)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, RTL8373_ISR_INT_MISC_ISR_INT_TM_LOW_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_0)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, (1 << 2), pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_1)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, (1 << 3), pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_2)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, (1 << 4), pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_3)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, (1 << 5), pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_4)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, (1 << 6), pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SDS_RX_SYM_ERR_0)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, (1 << 7), pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SDS_RX_SYM_ERR_1)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, (1 << 8), pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SAMOVE)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, RTL8373_ISR_INT_MISC_ISR_INT_SAMOVE_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == AUTO_REC)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, RTL8373_ISR_INT_MISC_ISR_INT_AUTO_REC_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;
        
        }
        else if (interrupt == ACL)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, RTL8373_ISR_INT_MISC_ISR_INT_ACL_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;

        }
        else if (interrupt == LOOP_DETEC)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, RTL8373_ISR_INT_MISC_ISR_INT_LOOP_DETECTION_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == METER_EXCEED)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, RTL8373_ISR_INT_MISC_ISR_INT_METER_EXCEED_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == ROUT_PBUF)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, RTL8373_ISR_INT_MISC_ISR_INT_ROUT_PBUF_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;

        }
        else if (interrupt == PTP1588)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, RTL8373_ISR_INT_MISC_ISR_INT_PTP1588_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;

        }
    }
    else  /*external interrupt*/
    {
        if (interrupt == TM_HIGH)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, RTL8373_ISR_EXT_MISC_ISR_EXT_TM_HIGH_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == TM_LOW)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, RTL8373_ISR_EXT_MISC_ISR_EXT_TM_LOW_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_0)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, (1 << 2), pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_1)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, (1 << 3), pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_2)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, (1 << 4), pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_3)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, (1 << 5), pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SMI_CHECK_REG_4)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, (1 << 6), pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SDS_RX_SYM_ERR_0)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, (1 << 7), pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SDS_RX_SYM_ERR_1)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, (1 << 8), pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == SAMOVE)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, RTL8373_ISR_EXT_MISC_ISR_EXT_SAMOVE_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == AUTO_REC)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, RTL8373_ISR_EXT_MISC_ISR_EXT_AUTO_REC_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;
        
        }
        else if (interrupt == ACL)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, RTL8373_ISR_EXT_MISC_ISR_EXT_ACL_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;

        }
        else if (interrupt == INCPU)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, RTL8373_ISR_EXT_MISC_ISR_EXT_ACL_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;

        }
        else if (interrupt == LOOP_DETEC)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, RTL8373_ISR_EXT_MISC_ISR_EXT_LOOP_DETECTION_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == METER_EXCEED)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, RTL8373_ISR_EXT_MISC_ISR_EXT_METER_EXCEED_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;
        }
        else if (interrupt == ROUT_PBUF)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, RTL8373_ISR_EXT_MISC_ISR_EXT_ROUT_PBUF_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;

        }
        else if (interrupt == PTP1588)
        {
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, RTL8373_ISR_EXT_MISC_ISR_EXT_PTP1588_OFFSET, pStatus)) != RT_ERR_OK)
                return retVal;

        }
    }

#endif    
    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_miscISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_miscISR_clear(rtk_uint32 type, interrupt_misc_t interrupt)
{
    rtk_api_ret_t retVal;

    if (type == 0)
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_INT_MISC_ADDR, interrupt, 1)) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_EXT_MISC_ADDR, interrupt, 1)) != RT_ERR_OK)
            return retVal;
    }


    return RT_ERR_OK;
}





/* Function Name:
 *      dal_rtl8373_glbISR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_glbISR_get(rtk_uint32 type, interrupt_glb_t interrupt, rtk_uint32* pStatus)
{
    rtk_api_ret_t retVal;

    /*internal interrupt*/
    if(type == 0)
    {
        if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_INT_GLB_ADDR, interrupt, pStatus)) != RT_ERR_OK)
            return retVal;

    }
    else  /*external interrupt*/
    {
        if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_EXT_GLB_ADDR, interrupt, pStatus)) != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}





/* Function Name:
 *      dal_rtl8373_IE_set
 * Description:
 *      Set switch interrupt enable
 * Input:
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_IE_set(rtk_uint32 enable)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_ISR_SW_INT_MODE_ADDR, RTL8373_ISR_SW_INT_MODE_SWITCH_IE_OFFSET, enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_IE_get
 * Description:
 *      Set switch interrupt enable
 * Input:
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_IE_get(rtk_uint32* pEnable)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_ISR_SW_INT_MODE_ADDR, RTL8373_ISR_SW_INT_MODE_SWITCH_IE_OFFSET, pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_portIntIMR_set
 * Description:
 *      Set per port interrupt IMR
 * Input:
 *      port: port id/ gphy id/ serdes id
 *      inttype: 0 internal interrupt; 1 external interupt
 *      int: per port interrupt
 *      enable: 0 disable IMR; 1 enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
rtk_api_ret_t dal_rtl8373_portIntIMR_set(rtk_port_t port, rtk_int_cpu_t inttype, rtk_int_type_t intnum, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /*interrnal interrupt*/
    if(intnum == INT_TYPE_LINK_CHANGE)
    {
        if ((retVal = dal_rtl8373_portLinkChgIMR_set(inttype, port, enable)) != RT_ERR_OK)
            return retVal;
    }
    else if(intnum == INT_TYPE_GPHY)
    {
        if ((retVal = dal_rtl8373_gphyIMR_set(inttype, port, enable)) != RT_ERR_OK)
            return retVal;
    }
    else if(intnum == INT_TYPE_LEARN_OVER)
    {
        if ((retVal = dal_rtl8373_portLrnOverIMR_set(inttype, port, enable)) != RT_ERR_OK)
            return retVal;
    }
    else if(intnum == INT_TYPE_RLFD)
    {
        if ((retVal = dal_rtl8373_portRLFDIMR_set(inttype, port, enable)) != RT_ERR_OK)
            return retVal;
    }
    else if(intnum == INT_TYPE_WOL)
    {
        if ((retVal = dal_rtl8373_portWolIMR_set(inttype, port, enable)) != RT_ERR_OK)
            return retVal;
    }
    else if(intnum == INT_TYPE_SDS_LINK_FAULT)
    {
        if ((retVal = dal_rtl8373_portSdsLnkFltIMR_set(inttype, port, enable)) != RT_ERR_OK)
            return retVal;
    }
    else if(intnum == INT_TYPE_SDS_UPDATE_PHY)
    {
        if ((retVal = dal_rtl8373_portSdsUpdPhyIMR_set(inttype, port, enable)) != RT_ERR_OK)
            return retVal;
    }

   
    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_portIntIMR_get
 * Description:
 *      Get per port interrupt IMR
 * Input:
 *      port: port id/ gphy id/ serdes id
 *      inttype: 0 internal interrupt; 1 external interupt
 *      int: per port interrupt
 *      enable: 0 disable IMR; 1 enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

rtk_api_ret_t dal_rtl8373_portIntIMR_get(rtk_port_t port, rtk_int_cpu_t inttype, rtk_int_type_t intnum, rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    /*interrnal interrupt*/
    if(intnum == INT_TYPE_LINK_CHANGE)
    {
        if ((retVal = dal_rtl8373_portLinkChgIMR_get(inttype, port, pEnable)) != RT_ERR_OK)
            return retVal;
    }
    else if(intnum == INT_TYPE_GPHY)
    {
        if ((retVal = dal_rtl8373_gphyIMR_get(inttype, port, pEnable)) != RT_ERR_OK)
            return retVal;
    }
    else if(intnum == INT_TYPE_LEARN_OVER)
    {
        if ((retVal = dal_rtl8373_portLrnOverIMR_get(inttype, port, pEnable)) != RT_ERR_OK)
            return retVal;
    }
    else if(intnum == INT_TYPE_RLFD)
    {
        if ((retVal = dal_rtl8373_portRLFDIMR_get(inttype, port, pEnable)) != RT_ERR_OK)
            return retVal;
    }
    else if(intnum == INT_TYPE_WOL)
    {
        if ((retVal = dal_rtl8373_portWolIMR_get(inttype, port, pEnable)) != RT_ERR_OK)
            return retVal;
    }
    else if(intnum == INT_TYPE_SDS_LINK_FAULT)
    {
        if ((retVal = dal_rtl8373_portSdsLnkFltIMR_get(inttype, port, pEnable)) != RT_ERR_OK)
            return retVal;
    }
    else if(intnum == INT_TYPE_SDS_UPDATE_PHY)
    {
        if ((retVal = dal_rtl8373_portSdsUpdPhyIMR_get(inttype, port, pEnable)) != RT_ERR_OK)
            return retVal;
    }

   
    return RT_ERR_OK;
}
































