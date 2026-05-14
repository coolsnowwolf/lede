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
 * Feature : Here is a list of all functions and variables in port module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal_rtl8373_port.h>
#include <dal_rtl8373_switch.h>
#include <dal_rtl8373_drv.h>
#include <rtl8373_asicdrv.h>
#include <string.h>

rtk_uint32 PORT3_PHYAD=0;
rtk_uint32 PORT8_PHYAD=0;
rtk_uint16 rtl8373_rtct_patch[][2]=
{
    {0x81A3,0x2E},
    {0x81A4,0xE0},
    {0x81A5,0x2E},
    {0x81A6,0xE0},
    {0x81A8,0x1D},
    {0x81A9,0x00},
    {0x81AF,0x2D},
    {0x81B0,0x05},
    {0x81B2,0x09},
    {0x81B3,0x1F},
    {0x81BC,0x1D},
    {0x81BD,0x00},
    {0x81BE,0x00},
    {0x81BF,0xEA},
    {0x81C0,0x03},
    {0x81C1,0xCA},
    {0x81C2,0x1D},
    {0x81C3,0x00},
    {0x81C4,0x00},
    {0x81C5,0x3B},
    {0x81C6,0x00},
    {0x81C7,0x1A},
    {0x81C8,0x00},
    {0x81C9,0x54},
    {0x81CA,0xFF},
    {0x81CB,0xD5},
    {0x81CC,0x07},
    {0x81CD,0xFA},
    {0x81CE,0xFF},
    {0x81CF,0x14},
    {0x81D0,0x00},
    {0x81D1,0x74},
    {0x81D2,0xFF},
    {0x81D3,0xDB},
    {0x81D4,0x09},
    {0x81D5,0xF0},
    {0x81D6,0xFF},
    {0x81D7,0x97},
    {0x81D8,0xFF},
    {0x81D9,0xC2},
    {0x81DA,0x00},
    {0x81DB,0x28},
    {0x81DC,0xF1},
    {0x81DD,0xA0},
    {0x81DE,0x00},
    {0x81DF,0x94},
    {0x81E0,0x00},
    {0x81E1,0x76},
    {0x81E2,0xFF},
    {0x81E3,0xB8},
    {0x81E4,0xEA},
    {0x81E5,0xDD},
    {0x81E6,0x01},
    {0x81E7,0x6B},
    {0x81E8,0xFF},
    {0x81E9,0xAA},
    {0x81EA,0x00},
    {0x81EB,0x06},
    {0x81EC,0x05},
    {0x81ED,0x0A},
    {0x81EE,0xFF},
    {0x81EF,0x9D},
    {0x81F0,0x00},
    {0x81F1,0x10},
    {0x81F2,0x00},
    {0x81F3,0x05},
    {0x81F4,0x01},
    {0x81F5,0xE0},
    {0x81F6,0x00},
    {0x81F7,0x00},
    {0x81F8,0x00},
    {0x81F9,0x00},
    {0x81FA,0x00},
    {0x81FB,0x00},
};






static ret_t _rtl8373_portlink_get(rtk_uint32 port, rtk_uint32* link)
{
    rtk_uint32 regData = 0;
    ret_t retVal;

    retVal = rtl8373_getAsicReg(RTL8373_MAC_LINK_STS_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicReg(RTL8373_MAC_LINK_STS_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *link = (regData >> port) & 1;
    
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portFrcAblitiy_set
 * Description:
 *      Set port force ability
 * Input:
 *      port     - 0 ~ 9
 *      ability    - link, speed, duplex, rxpause, txpause
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

ret_t dal_rtl8373_portFrcAbility_set(rtk_uint32 port, rtk_port_ability_t* ability)
{
    rtk_uint32 regData = 0;
    rtk_uint32 tmp;
    ret_t retVal;
    if((ability->forcemode>=RTK_ENABLE_END)||(ability->link>=PORT_LINKSTATUS_END)||(ability->duplex>=PORT_DUPLEX_END)||
        (ability->speed >=PORT_SPEED_END)||(ability->txpause>=RTK_ENABLE_END)||(ability->rxpause>=RTK_ENABLE_END)||
        (ability->smi_force_fc>=RTK_ENABLE_END)||(ability->media>=PORT_MEDIA_END))
        return RT_ERR_INPUT;
  
    /*if port now is link up,  force linkdown first, then set other ability*/
    _rtl8373_portlink_get(port, &tmp);
    if(tmp == 1)
    {
        retVal = rtl8373_setAsicRegBit(RTL8373_MAC_FORCE_MODE_CTRL0_ADDR(port), RTL8373_MAC_FORCE_MODE_CTRL0_FORCE_LINK_EN_OFFSET, 0);
        if(retVal != RT_ERR_OK)
            return retVal;

        retVal = rtl8373_setAsicRegBit(RTL8373_MAC_FORCE_MODE_CTRL0_ADDR(port), RTL8373_MAC_FORCE_MODE_CTRL0_MAC_FORCE_EN_OFFSET, 1);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    regData = ability->forcemode & 1;
    regData |= (ability->link << 1);
    regData |= (ability->duplex << 2);
    regData |= ((ability->speed & 0xf) << 3);
    regData |= (ability->txpause << 7);
    regData |= (ability->rxpause << 8);
    regData |= (ability->smi_force_fc << 9);
    regData |= (ability->media << 10);

    return rtl8373_setAsicRegBits(RTL8373_MAC_FORCE_MODE_CTRL0_ADDR(port), 0x7ff, regData);
}


/* Function Name:
 *      dal_rtl8373_portFrcAblitiy_get
 * Description:
 *      Get port force ability
 * Input:
 *      port     - 0 ~ 9
 *      ability    - link, speed, duplex, rxpause, txpause
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

ret_t dal_rtl8373_portFrcAbility_get(rtk_uint32 port, rtk_port_ability_t* ability)
{
    rtk_uint32 regData = 0;
    ret_t retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_MAC_FORCE_MODE_CTRL0_ADDR(port), 0x7ff, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;
    
    ability->forcemode = regData & 1;
    ability->link = (regData >> 1) & 1;
    ability->duplex = (regData >> 2) & 1;
    ability->speed = (regData >> 3) & 0xf;
    ability->txpause = (regData >> 7) & 1;
    ability->rxpause = (regData >> 8) & 1;
	ability->smi_force_fc = (regData >> 9) & 1;
    ability->media = (regData >> 10) & 1;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_portStatus_get
 * Description:
 *      Get port status
 * Input:
 *      port     - 0 ~ 9
 *      ability    - link, speed, duplex, rxpause, txpause
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

ret_t dal_rtl8373_portStatus_get(rtk_uint32 port, rtk_port_status_t* status)
{
    rtk_uint32 regData = 0;
    ret_t retVal;
    /*link status*/
    retVal = rtl8373_getAsicReg(RTL8373_MAC_LINK_STS_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicReg(RTL8373_MAC_LINK_STS_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    status->link = (regData >> port) & 1;

    /*speed status*/
    retVal = rtl8373_getAsicRegBits(RTL8373_MAC_LINK_SPD_STS_ADDR(port), RTL8373_MAC_LINK_SPD_STS_SPD_STS_9_0_MASK(port), &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    status->speed= regData;

    /*duplex status*/
    retVal = rtl8373_getAsicReg(RTL8373_MAC_LINK_DUP_STS_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    status->duplex= (regData >> port) & 1;

    /*tx pause status*/
    retVal = rtl8373_getAsicReg(RTL8373_MAC_TX_PAUSE_STS_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    status->txpause= (regData >> port) & 1;

    /*rx pause status*/
    retVal = rtl8373_getAsicReg(RTL8373_MAC_TX_PAUSE_STS_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    status->rxpause= (regData >> port) & 1;

    /*media status*/
    retVal = rtl8373_getAsicReg(RTL8373_MAC_LINK_MEDIA_STS_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    status->media= (regData >> port) & 1;

    /*eee status*/
    retVal = rtl8373_getAsicReg(RTL8373_MAC_EEE_ABLTY_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    status->eee= (regData >> port) & 1;

    /*master status*/
    retVal = rtl8373_getAsicReg(RTL8373_MAC_MSTR_SLV_STS_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    status->master= (regData >> port) & 1;

    /*master slave nway*/
    retVal = rtl8373_getAsicReg(RTL8373_MAC_MSTR_SLV_FAULT_STS_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    status->master_slave= (regData >> port) & 1;


    return RT_ERR_OK;
    
}


/* Function Name:
 *      dal_rtl8373_portMaxLen_set
 * Description:
 *      Set port rx max length
 * Input:
 *      port     - 0 ~ 9
 *      type    - 0 FE:10M,100M,   1 giga:1G, 2.5G, 5G, 10G
 *      len      - max length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

ret_t dal_rtl8373_portMaxLen_set(rtk_uint32 port, rtk_uint32 type, rtk_uint32 len)
{
    ret_t retVal;


    if(type == 0)
    {
        retVal = rtl8373_setAsicRegBits(RTL8373_MAC_L2_PORT_MAX_LEN_CTRL_ADDR(port), RTL8373_MAC_L2_PORT_MAX_LEN_CTRL_MAX_LEN_100M_10M_SEL_MASK, len);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else if(type == 1)
    {
        retVal = rtl8373_setAsicRegBits(RTL8373_MAC_L2_PORT_MAX_LEN_CTRL_ADDR(port), RTL8373_MAC_L2_PORT_MAX_LEN_CTRL_MAX_LEN_1G_2P5G_5G_10G_SEL_MASK, len);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else 
        return RT_ERR_INPUT;
    
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portMaxLen_get
 * Description:
 *      Get port rx max length
 * Input:
 *      port     - 0 ~ 9
 *      type    - 0 FE:10M,100M,   1 giga:1G, 2.5G, 5G, 10G
 *      pLen    - max length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

ret_t dal_rtl8373_portMaxLen_get(rtk_uint32 port, rtk_uint32 type, rtk_uint32* pLen)
{
    ret_t retVal;


    if(type == 0)
    {
        retVal = rtl8373_getAsicRegBits(RTL8373_MAC_L2_PORT_MAX_LEN_CTRL_ADDR(port), RTL8373_MAC_L2_PORT_MAX_LEN_CTRL_MAX_LEN_100M_10M_SEL_MASK, pLen);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else if(type == 1)
    {
        retVal = rtl8373_getAsicRegBits(RTL8373_MAC_L2_PORT_MAX_LEN_CTRL_ADDR(port), RTL8373_MAC_L2_PORT_MAX_LEN_CTRL_MAX_LEN_1G_2P5G_5G_10G_SEL_MASK, pLen);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else 
        return RT_ERR_INPUT;
    
    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_portMaxLenIncTag_set
 * Description:
 *      Set port rx max length include tag length: cputag ctag, stag
 * Input:
 *      port     - 0 ~ 9
 *      enaleb  - 1: include tag length,   0: not include tag length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

ret_t dal_rtl8373_portMaxLenIncTag_set(rtk_uint32 port, rtk_uint32 enable)
{


    return rtl8373_setAsicRegBit(RTL8373_MAC_L2_PORT_MAX_LEN_CTRL_ADDR(port), RTL8373_MAC_L2_PORT_MAX_LEN_CTRL_MAX_LEN_TAG_INC_OFFSET, enable);

}


/* Function Name:
 *      dal_rtl8373_portMaxLenIncTag_get
 * Description:
 *      Get port rx max length include tag length: cputag ctag, stag
 * Input:
 *      port     - 0 ~ 9
 *      enaleb  - 1: include tag length,   0: not include tag length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

ret_t dal_rtl8373_portMaxLenIncTag_get(rtk_uint32 port, rtk_uint32* pEnable)
{

    return rtl8373_getAsicRegBit(RTL8373_MAC_L2_PORT_MAX_LEN_CTRL_ADDR(port), RTL8373_MAC_L2_PORT_MAX_LEN_CTRL_MAX_LEN_TAG_INC_OFFSET, pEnable);

}


/* Function Name:
 *      dal_rtl8373_portLoopbackEn_set
 * Description:
 *      Set port mac tx loopback to rx
 * Input:
 *      port     - 0 ~ 9
 *      enable    - 0 disable loopback,   1 enable loopback
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

ret_t dal_rtl8373_portLoopbackEn_set(rtk_uint32 port, rtk_enable_t enable)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBit(RTL8373_MAC_L2_PORT_CTRL_ADDR(port), RTL8373_MAC_L2_PORT_CTRL_CFG_PORT_L_LPBK_OFFSET, enable);
    if(retVal != RT_ERR_OK)
        return retVal;

    
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portLoopbackEn_get
 * Description:
 *      Get port mac tx loopback to rx
 * Input:
 *      port     - 0 ~ 9
 *      pEnable    - 0 disable loopback,   1 enable loopback
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

ret_t dal_rtl8373_portLoopbackEn_get(rtk_uint32 port, rtk_enable_t * pEnable)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBit(RTL8373_MAC_L2_PORT_CTRL_ADDR(port), RTL8373_MAC_L2_PORT_CTRL_CFG_PORT_L_LPBK_OFFSET, pEnable);
    if(retVal != RT_ERR_OK)
        return retVal;

    
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portBackpressureEn_set
 * Description:
 *      Set port half back pressure enable
 * Input:
 *      port     - 0 ~ 9
 *      enable    - 0 disable backpressure,   1 enable backpressure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

ret_t dal_rtl8373_portBackpressureEn_set(rtk_uint32 port, rtk_enable_t enable)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBit(RTL8373_MAC_PORT_CTRL_ADDR(port), RTL8373_MAC_PORT_CTRL_BKPRES_EN_OFFSET, enable);
    if(retVal != RT_ERR_OK)
        return retVal;

    
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_portBackpressureEn_get
 * Description:
 *      Get port half back pressure enable
 * Input:
 *      port     - 0 ~ 9
 *      pEnable    - 0 disable backpressure,   1 enable backpressure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

ret_t dal_rtl8373_portBackpressureEn_get(rtk_uint32 port, rtk_enable_t * pEnable)
{
    ret_t retVal;


    retVal = rtl8373_getAsicRegBit(RTL8373_MAC_PORT_CTRL_ADDR(port), RTL8373_MAC_PORT_CTRL_BKPRES_EN_OFFSET, pEnable);
    if(retVal != RT_ERR_OK)
        return retVal;

    
    return RT_ERR_OK;
}




/* Function Name:
 *      dal_rtl8373_port_extphyid_set
 * Description:
 *      Set   smi address ,SMI Address, default value equals to port number
 * Input:
 *      sdsid     - 0,1
 *      phyid   - get from phy strap pin
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

ret_t dal_rtl8373_port_extphyid_set(rtk_uint32 sdsid,  rtk_uint32 phyid)
{
    ret_t retVal;
    
    RTK_CHK_INIT_STATE();
    if(sdsid == SERDES_ID0)
    {
        retVal =rtl8373_setAsicRegBits(RTL8373_SMI_MAC_TYPE_CTRL_ADDR, RTL8373_SMI_MAC_TYPE_CTRL_MAC_PORT3_TYPE_MASK, 1);
        if(retVal != RT_ERR_OK)
            return retVal;
        retVal = rtl8373_setAsicRegBits(RTL8373_SMI_PORT0_5_ADDR_CTRL_ADDR, RTL8373_SMI_PORT0_5_ADDR_CTRL_PORT3_ADDR_MASK, phyid);
        if(retVal != RT_ERR_OK)
            return retVal;
        PORT3_PHYAD=phyid;
    }
    else if(sdsid == SERDES_ID1)
    {
        retVal =rtl8373_setAsicRegBits(RTL8373_SMI_MAC_TYPE_CTRL_ADDR, RTL8373_SMI_MAC_TYPE_CTRL_MAC_PORT8_TYPE_MASK, 1);
        if(retVal != RT_ERR_OK)
            return retVal;
        retVal = rtl8373_setAsicRegBits(RTL8373_SMI_PORT6_9_ADDR_CTRL_ADDR, RTL8373_SMI_PORT6_9_ADDR_CTRL_PORT8_ADDR_MASK, phyid);
        if(retVal != RT_ERR_OK)
            return retVal;
       PORT8_PHYAD=phyid;
    }
    else 
        return RT_ERR_INPUT;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_port_extphyid_get
 * Description:
 *      Get   smi address ,SMI Address, default value equals to port number
 * Input:
 *      sdsid     - 0,1
 *      phyid   - get smi address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

ret_t dal_rtl8373_port_extphyid_get(rtk_uint32 sdsid,  rtk_uint32 *phyid)
{
    ret_t retVal;

    if(phyid == NULL)
        return RT_ERR_INPUT;
    
    if(sdsid == SERDES_ID0)
    {

        retVal = rtl8373_getAsicRegBits(RTL8373_SMI_PORT0_5_ADDR_CTRL_ADDR, RTL8373_SMI_PORT0_5_ADDR_CTRL_PORT3_ADDR_MASK, phyid);
        if(retVal != RT_ERR_OK)
            return retVal;

    }
    else if(sdsid == SERDES_ID1)
    {

        retVal = rtl8373_getAsicRegBits(RTL8373_SMI_PORT6_9_ADDR_CTRL_ADDR, RTL8373_SMI_PORT6_9_ADDR_CTRL_PORT8_ADDR_MASK, phyid);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else 
        return RT_ERR_INPUT;
    
    return RT_ERR_OK;
}
/* Function Name:
 *      dal_rtl8373_port_sdsNway_set
 * Description:
 *      Configure serdes port Nway state
 * Input:
 *      sdsId        - serdes ID
 *      sdsMode      - serdes mode
 *      enable       - 1:enable nway, 0:disable nway
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 * Note:
 *      The API configure SERDES port Nway state
 */
rtk_api_ret_t dal_rtl8373_port_sdsNway_set(rtk_uint32 sdsId, rtk_sds_mode_t sdsMode, rtk_enable_t enable)
{
    switch(sdsMode)
	{
		case SERDES_100FX:
		case SERDES_10GR:			 
			 break;
		case SERDES_1000BASEX:
		case SERDES_2500BASEX:
		case SERDES_SG:
		case SERDES_HSG:
			if(enable)
			{
				dal_rtl8373_sds_regbits_write(sdsId, 0, 2, 0x3<<8, 0x3);
				dal_rtl8373_sds_regbits_write(sdsId, 0, 4, 0x3<<1, 0x3); //force enable
			}
			else
			{
				dal_rtl8373_sds_regbits_write(sdsId, 0, 2, 0x3<<8, 0x1);
				dal_rtl8373_sds_regbits_write(sdsId, 0, 4, 0x3<<1, 0x3);
			}
			break;  
		case SERDES_10GUSXG:
		case SERDES_10GQXG:
			if(enable)
			{
				dal_rtl8373_sds_regbits_write(sdsId, 7, 17, 0xf<<0, 0xf);
			}
			else
			{

				dal_rtl8373_sds_regbits_write(sdsId, 7, 17, 0xf<<0, 0x0);
			}
			break;  
		default:
		 	return RT_ERR_INPUT;
			break;  
	 }	
	return RT_ERR_OK;
}
/* Function Name:
 *      dal_rtl8373_port_sdsNway_get
 * Description:
 *      Get serdes Nway
 * Input:
 *      sdsid        - serdes ID
 *      pState       - Nway state
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 * Note:
 *      The API configure serdes port Nway state
 */
rtk_api_ret_t dal_rtl8373_port_sdsNway_get(rtk_uint32 sdsId, rtk_sds_mode_t sdsMode, rtk_enable_t *pState)
{
    rtk_uint32 regData, regData1;
    
    switch(sdsMode)
    {
    	case SERDES_100FX:
    	case SERDES_10GR:			 
    		 break;
    	case SERDES_1000BASEX:
    	case SERDES_2500BASEX:
    	        dal_rtl8373_sds_regbits_read(sdsId, 0, 2, 0x3<<8, &regData);
    	        if(regData == 0x3) //bit8=1
    	        {    	           
                    *pState = ENABLED;                  
    	        }
    	        else if(regData == 0x1) //bit8=1
    	        {    	           
                    *pState = DISABLED;                  
    	        }
    	        else if((regData == 0x0) ||(regData == 0x2)) //bit8=0
    	        {
    	            dal_rtl8373_sds_regbits_read(sdsId, 2, 0, 0x1<<12, &regData1);
    	            if(regData1 == 0x1)
    			*pState = ENABLED;
                   else if(regData1 == 0x0)
                   	*pState = DISABLED;
                   else
                   	return RT_ERR_FAILED;
    	        }
    	        else
                   	return RT_ERR_FAILED;

    	        break;
    	case SERDES_SG:
    	case SERDES_HSG:
    		dal_rtl8373_sds_regbits_read(sdsId, 0, 2, 0x3<<8, &regData);
    	        if(regData == 0x3) //bit8=1
    	        {    	           
                    *pState = ENABLED;                  
    	        }
    	        else if(regData == 0x1) //bit8=1
    	        {    	           
                    *pState = DISABLED;                  
    	        }
    	        else if((regData == 0x0) ||(regData == 0x2))//bit8=0
    	        {
    			*pState = ENABLED;                   
    	        }
    	        else
                   	return RT_ERR_FAILED;
    		break;  
    	case SERDES_10GUSXG:
    	case SERDES_10GQXG:
    		dal_rtl8373_sds_regbits_read(sdsId, 7, 17, 0xf<<0, &regData);
    		if(regData == 0xf)
    			*pState = ENABLED;
    		else if(regData == 0)
    			*pState = DISABLED;
    		else
    			return RT_ERR_FAILED;
    		break;  
    	default:
    	 	return RT_ERR_INPUT;
    		break;  
     }	
	return RT_ERR_OK;
}
/* Function Name:
 *      dal_rtl8373_sdsMode_set
 * Description:
 *      Set sds mode
 * Input:
 *      sdsid     - 0 ~ 1
 *      mode  - 
 *      SERDES_10GQXG,
 *      SERDES_10GUSXG,
 *      SERDES_10GR,
 *      SERDES_HSG,
 *      SERDES_2500BASEX,
 *      SERDES_SG,
 *      SERDES_1000BASEX,
 *      SERDES_100FX,    
 *      SERDES_OFF, 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_NOT_ALLOWED      - mode is not support
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_sdsMode_set(rtk_uint32 sdsid, rtk_sds_mode_t mode)
{
    ret_t retVal;
    rtk_uint32 regdata, chiptype;

    RTK_CHK_INIT_STATE();

    if((retVal = rtl8373_getAsicReg(0x4, &regdata)) != RT_ERR_OK)
        return retVal;

    regdata = regdata >> 8;

    if(regdata == 0x837300)
    {
        if((sdsid == 0) && (mode != SERDES_10GQXG) && (mode != SERDES_OFF))
            return RT_ERR_NOT_ALLOWED;
        chiptype = CHIP_RTL8373_MODE;
    }
    else if (regdata == 0x837200)
        chiptype = CHIP_RTL8372_MODE;
	else if (regdata == 0x822400)
	{
        chiptype = CHIP_RTL8224_MODE;
    }
    else if (regdata == 0x837370)
    {
        chiptype = CHIP_RTL8373N_MODE;		 
    }
    else if (regdata == 0x837270)
    {
        chiptype = CHIP_RTL8372N_MODE;
    }
    else if (regdata == 0x822470)
    {
        chiptype = CHIP_RTL8224N_MODE;		 
    }
    else if (regdata == 0x8366A8)
    {
        chiptype = CHIP_RTL8366U_MODE;
    }    
    else 
        return RT_ERR_CHIP_NOT_FOUND;

    if(sdsid==SERDES_ID0)
    {
        if (mode == SERDES_8221B)
        {
            rtl8373_setAsicRegBits(RTL8373_SMI_PORT0_5_ADDR_CTRL_ADDR, RTL8373_SMI_PORT0_5_ADDR_CTRL_PORT3_ADDR_MASK, PORT3_PHYAD);
            cfg_rl6637_sds_mode(UTP_PORT3, 1);  //here add 8221B phy patch
            SDS_MODE_SET_SW(chiptype, sdsid, SERDES_HSG);
            SDS_MODE_SET_SW(chiptype, sdsid, SERDES_SG);
            
            //rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR,RTL8373_SDS_MODE_SEL_SDS0_MODE_SEL_MASK,0x1F);
            rtl8373_setAsicRegBit(RTL8373_SMI_PORT_POLLING_SEL_ADDR, RTL8373_SMI_PORT_POLLING_SEL_SMI_POLLING_SEL3_OFFSET, ENABLED); // RTL8372 set port3  polling internal resolution reg

        }
        else if(mode == SERDES_OFF)
        {
            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<4, 0x3);
            delay_loop(10);
            
            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<4, 0x1);
            delay_loop(100);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<6, 0x1);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<6, 0x3);
            delay_loop(100);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<10, 0x3);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<10, 0x1);
                        
            delay_loop(100);
        }
        else if(mode == SERDES_ON)  //sds on
        {
            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<10, 0x1);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<10, 0x3);
            delay_loop(100);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<10, 0x0);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<6, 0x3);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<6, 0x1);
            delay_loop(100);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<6, 0x0);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<4, 0x1);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<4, 0x3);
            delay_loop(100);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<4, 0x0);
            delay_loop(100);
        }
        else
        {

            rtl8373_setAsicRegBit(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_CFG_MAC3_8221B_OFFSET, DISABLED);
            //rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR,RTL8373_SDS_MODE_SEL_SDS0_MODE_SEL_MASK,0x1F);
            delay_loop(1000);
            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<4, 0x3);
            delay_loop(10);
            
            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<4, 0x1);
            delay_loop(100);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<6, 0x1);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<6, 0x3);
            delay_loop(100);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<10, 0x3);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<10, 0x1);
                        
            delay_loop(100);
            SDS_MODE_SET_SW(chiptype, sdsid, mode);         
        }
    }
    if(sdsid==SERDES_ID1)
    {
        if (mode == SERDES_8221B)
        {
            rtl8373_setAsicRegBits(RTL8373_SMI_PORT6_9_ADDR_CTRL_ADDR, RTL8373_SMI_PORT6_9_ADDR_CTRL_PORT8_ADDR_MASK, PORT8_PHYAD);
            cfg_rl6637_sds_mode(UTP_PORT8, 1);  //here add 8221B phy patch 
            SDS_MODE_SET_SW(chiptype, sdsid, SERDES_HSG);
            SDS_MODE_SET_SW(chiptype, sdsid, SERDES_SG);
            
            //rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR,RTL8373_SDS_MODE_SEL_SDS1_MODE_SEL_MASK,0x1F);
            rtl8373_setAsicRegBit(RTL8373_SMI_PORT_POLLING_SEL_ADDR, RTL8373_SMI_PORT_POLLING_SEL_SMI_POLLING_SEL8_OFFSET, ENABLED); // RTL8372 set port8  polling internal resolution reg
        }
        else if(mode == SERDES_OFF)
        {
            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<4, 0x3);
            delay_loop(10);
            
            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<4, 0x1);
            delay_loop(100);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<6, 0x1);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<6, 0x3);
            delay_loop(100);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<10, 0x3);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<10, 0x1);
                        
            delay_loop(100);
        }
        else if(mode == SERDES_ON)  //sds on
        {
            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<10, 0x1);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<10, 0x3);
            delay_loop(100);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<10, 0x0);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<6, 0x3);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<6, 0x1);
            delay_loop(100);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<6, 0x0);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<4, 0x1);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<4, 0x3);
            delay_loop(100);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<4, 0x0);
            delay_loop(100);
        }
        else
        {

            rtl8373_setAsicRegBit(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_CFG_MAC8_8221B_OFFSET, DISABLED);
            //rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR,RTL8373_SDS_MODE_SEL_SDS1_MODE_SEL_MASK,0x1F);
            delay_loop(1000);
            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<4, 0x3);
            delay_loop(10);
            
            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<4, 0x1);
            delay_loop(100);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<6, 0x1);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<6, 0x3);
            delay_loop(100);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<10, 0x3);
            delay_loop(10);

            dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0x00, 0x3<<10, 0x1);
            
            delay_loop(100);
            SDS_MODE_SET_SW(chiptype, sdsid, mode);         
        }
    }

    delay_loop(50);
    if((mode == SERDES_10GQXG)||(mode == SERDES_10GR) || (mode == SERDES_10GUSXG))
        fw_reset_flow_tgr(sdsid);
    else
        fw_reset_flow_tgx(sdsid);

    delay_loop(50);

    return RT_ERR_OK;

}



/* Function Name:
 *      dal_rtl8373_sdsMode_get
 * Description:
 *      Get sds mode
 * Input:
 *      sdsid     - 0 ~ 1
 *      pMode  - 
 *      SERDES_10GQXG,
 *      SERDES_10GUSXG,
 *      SERDES_10GR,
 *      SERDES_HSG,
 *      SERDES_2500BASEX,
 *      SERDES_SG,
 *      SERDES_1000BASEX,
 *      SERDES_100FX,    
 *      SERDES_OFF, 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_NOT_ALLOWED      - mode is not support
 * Note:
 *      None
 */

rtk_api_ret_t dal_rtl8373_sdsMode_get(rtk_uint32 sdsid, rtk_sds_mode_t * pMode)
{
    rtk_uint32 regdata, regdata2, subtype;
    
    if(sdsid == 0)
    {
        rtl8373_getAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_MODE_SEL_MASK,&regdata);
        rtl8373_getAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_USX_SUB_MODE_MASK,&subtype);
        dal_rtl8373_sds_reg_read(sdsid, 0x20, 0x00,&regdata2);
        
    }
    else if(sdsid == 1)
    {
        rtl8373_getAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS1_MODE_SEL_MASK,&regdata);
        rtl8373_getAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS1_USX_SUB_MODE_MASK,&subtype);
        dal_rtl8373_sds_reg_read(sdsid, 0x20, 0x00,&regdata2);
    }
    else 
        return RT_ERR_INPUT;
    
    if ((((regdata2 >> 4) & 0x3) == 0x1) && (((regdata2 >> 6) & 0x3) == 0x3) && (((regdata2 >> 10) & 0x3) == 0x1) )
    {
        *pMode = SERDES_OFF;
    }
    else
    {
        if(regdata == 0xD)
        {
            /*USXG*/
            if(subtype == 0)
                *pMode = SERDES_10GUSXG;
            else if (subtype == 2)
                *pMode = SERDES_10GQXG;
            else
                return RT_ERR_FAILED;
        }
        else if (regdata == 0x1A)
            *pMode = SERDES_10GR;    
        else if (regdata == 0x12)
                *pMode = SERDES_HSG;
        else if (regdata == 0x16)
                *pMode = SERDES_2500BASEX;
        else if (regdata == 0x2)
                *pMode = SERDES_SG;
        else if (regdata == 0x4)
                *pMode = SERDES_1000BASEX;
        else if (regdata == 0x5)
                *pMode = SERDES_100FX;
        else if (regdata == 0x1F)
                *pMode = SERDES_OFF;
    }
    return RT_ERR_OK;    
}


/* Function Name:
 *      dal_rtl8373_rtct_init
 * Description:
 *      Init RTCT
 * Input:
 *      None.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid port mask
 * Note:
 *      RTCT test takes 4.8 seconds at most.
 */

ret_t dal_rtl8373_rtct_init(void)
{
    rtk_uint32 phyid, len, regaddr, regdata, i;
    rtk_uint32 rg_dll_one_ch_en, pn_rate, rg_biquad_coef_sel, enlongpn, rg_rtct_hpf_coef_sel, reg_pnrate; 


    for(phyid = 4; phyid < 8; phyid++)
    {
        dal_rtl8373_phy_regbits_write(1<<phyid, 31, 0xa46a, 3, 0x1);
        dal_rtl8373_phy_regbits_write(1<<phyid, 31, 0xa422, 1, 0x0);
    }

    rg_dll_one_ch_en = 1;
    pn_rate = 1;
    rg_biquad_coef_sel = 0;
    enlongpn = 0;
    rg_rtct_hpf_coef_sel = 2;

    dal_rtl8373_phy_regbits_write(1<<phyid, 31, 0xc010, 1<<9, rg_dll_one_ch_en);
    dal_rtl8373_phy_regbits_write(1<<phyid, 31, 0xc010, 1<<10, pn_rate);
    dal_rtl8373_phy_regbits_write(1<<phyid, 31, 0xc010, 3<<13, rg_biquad_coef_sel);
    dal_rtl8373_phy_regbits_write(1<<phyid, 31, 0xc000, 1<<0, enlongpn);
    dal_rtl8373_phy_regbits_write(1<<phyid, 31, 0xc020, 7<<1, rg_rtct_hpf_coef_sel);


    dal_rtl8373_phy_regbits_read(phyid, 31, 0xc010, 1<<10, &reg_pnrate);

    len= sizeof(rtl8373_rtct_patch)/4;

    for(i=0;i<len;i++)
    {
        regaddr=rtl8373_rtct_patch[i][0];
        regdata=rtl8373_rtct_patch[i][1];        
        uc1_sram_write_8b(phyid,regaddr,regdata);    
                            
    }
    
	return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_rtct_start
 * Description:
 *      Set RTCT start
 * Input:
 *      portmask    - Port mask of RTCT enabled (4 - 7)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid port mask
 * Note:
 *      RTCT test takes 4.8 seconds at most.
 */
ret_t dal_rtl8373_rtct_start(rtk_uint32 phymask)
{
    rtk_uint32  port, i, rtct_done, count;

    for(port = 0; port <= 8 ; port++)
    {
        if(phymask & (0x0001 << port))
        {
            dal_rtl8373_phy_regbits_write(1<<port, 31, 0xa400, 1 << 9, 1);

            for(i=0; i<10000; i++);

            dal_rtl8373_phy_regbits_write(1<<port, 31, 0xa422, 1 << 1, 1);
            dal_rtl8373_phy_regbits_write(1<<port, 31, 0xa400, 1 << 15, 0);
             
            dal_rtl8373_phy_regbits_read(port, 31, 0xa400, 1 << 15, &rtct_done);
            if(rtct_done != 0)
                return RT_ERR_FAILED;

            dal_rtl8373_phy_regbits_write(1<<port, 31, 0xa422, 0xf << 4, 0xf);
            dal_rtl8373_phy_regbits_write(1<<port, 31, 0xa422, 1, 1);

            count = 1000;
            do
            {
                dal_rtl8373_phy_regbits_read(port, 31, 0xa400, 1 << 15, &rtct_done);
                count--;
                if (count == 0)
                    break;
            }while(rtct_done!=1);

            if(count == 0)
                return RT_ERR_BUSYWAIT_TIMEOUT;
        }
        return RT_ERR_OK;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_rtct_status_get
 * Description:
 *      Get RTCT result
 * Input:
 *      phyid    - Port id of RTCT result (4 - 7)
 *      pResult  - result of RTCT
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      RTCT test takes 4.8 seconds at most.
 */
ret_t dal_rtl8373_rtct_status_get(rtk_uint32 phyid, rtk_rtct_result_t * pResult)
{
    rtk_uint32  channalA, channalB, channalC, channalD;

	if(pResult == NULL)
		return RT_ERR_NULL_POINTER;

	channalA = uc1_sram_read_8b(phyid, 0x8026);
	channalB = uc1_sram_read_8b(phyid, 0x802A);
	channalC = uc1_sram_read_8b(phyid, 0x802E);
	channalD = uc1_sram_read_8b(phyid, 0x8032);

	pResult->channelABusy = channalA & 1;
	pResult->channelAMisOpen = (channalA >> 1) & 1;
	pResult->channelAMisShort = (channalA >> 2) & 1;
	pResult->channelAOpen = (channalA >> 3) & 1;
	pResult->channelAShort = (channalA >> 4) & 1;
	pResult->channelANormal = (channalA >> 5) & 1;
	pResult->channelADone = (channalA >> 6) & 1;
	pResult->channelAInterShort = (channalA >> 7) & 1;

	pResult->channelBBusy = channalB & 1;
	pResult->channelBMisOpen = (channalB >> 1) & 1;
	pResult->channelBMisShort = (channalB >> 2) & 1;
	pResult->channelBOpen = (channalB >> 3) & 1;
	pResult->channelBShort = (channalB >> 4) & 1;
	pResult->channelBNormal = (channalB >> 5) & 1;
	pResult->channelBDone = (channalB >> 6) & 1;
	pResult->channelBInterShort = (channalB >> 7) & 1;

	pResult->channelCBusy = channalC & 1;
	pResult->channelCMisOpen = (channalC >> 1) & 1;
	pResult->channelCMisShort = (channalC >> 2) & 1;
	pResult->channelCOpen = (channalC >> 3) & 1;
	pResult->channelCShort = (channalC >> 4) & 1;
	pResult->channelCNormal = (channalC >> 5) & 1;
	pResult->channelCDone = (channalC >> 6) & 1;
	pResult->channelCInterShort = (channalC >> 7) & 1;

	pResult->channelDBusy = channalD & 1;
	pResult->channelDMisOpen = (channalD >> 1) & 1;
	pResult->channelDMisShort = (channalD >> 2) & 1;
	pResult->channelDOpen = (channalD >> 3) & 1;
	pResult->channelDShort = (channalD >> 4) & 1;
	pResult->channelDNormal = (channalD >> 5) & 1;
	pResult->channelDDone = (channalD >> 6) & 1;
	pResult->channelDInterShort = (channalD >> 7) & 1;

	return RT_ERR_OK;

}


/* Function Name:
 *      dal_rtl8373_iol_fix_pattern
 * Description:
 *      Set IOL fix pattern.
 * Input:
 *      port  - port id 0 - 8 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */

rtk_api_ret_t dal_rtl8373_iol_fix_pattern(rtk_uint32 port)
{
    rtk_uint32 phymask;
    rtk_api_ret_t retVal;

    phymask = 1 << port;

    if ((retVal = dal_rtl8373_phy_write(phymask, 31, 0xc804, 0x0015)) != RT_ERR_OK)
        return retVal;

    
    if ((retVal = dal_rtl8373_phy_write(phymask, 31, 0xc800, 0xFF21)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_iol_random_pattern
 * Description:
 *      Set IOL ramdom pattern.
 * Input:
 *      port  - port id 0 - 8 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */

rtk_api_ret_t dal_rtl8373_iol_random_pattern(rtk_uint32 port)
{
    rtk_uint32 phymask;
    rtk_api_ret_t retVal;

    phymask = 1 << port;

    if ((retVal = dal_rtl8373_phy_write(phymask, 31, 0xc804, 0x0115)) != RT_ERR_OK)
        return retVal;

    
    if ((retVal = dal_rtl8373_phy_write(phymask, 31, 0xc800, 0x5A21)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_iol_10M_mode
 * Description:
 *      Set IOL 10M mode.
 * Input:
 *      port  - port id 0 - 8 
 *      mode - 0:mdi   1:mdix
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */

rtk_api_ret_t dal_rtl8373_iol_10M_mode(rtk_uint32 port, rtk_uint32 mode)
{
    rtk_uint32 phymask;
    rtk_api_ret_t retVal;

    phymask = 1 << port;

    if ((retVal = dal_rtl8373_phy_write(phymask, 1, 0, 0)) != RT_ERR_OK)
        return retVal;
    
    if ((retVal = dal_rtl8373_phy_regbits_write(phymask, 7, 0, 1 << 12, 0)) != RT_ERR_OK)
        return retVal;

    if ((retVal = dal_rtl8373_phy_regbits_write(phymask, 31, 0xA412, 7 << 13, 0)) != RT_ERR_OK)
        return retVal;

    if(mode == IOL_MODE_MDI)
    {
        if ((retVal = dal_rtl8373_phy_write(phymask, 31, 0xA430, 0x239A)) != RT_ERR_OK)
            return retVal;
    }
    else if(mode == IOL_MODE_MDIX)
    {
        if ((retVal = dal_rtl8373_phy_write(phymask, 31, 0xA430, 0x229A)) != RT_ERR_OK)
            return retVal;
    }
    else 
        return RT_ERR_INPUT;


    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_iol_100M_mode
 * Description:
 *      Set IOL 100M mode.
 * Input:
 *      port  - port id 0 - 8 
 *      mode - 0:mdi   1:mdix
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */

rtk_api_ret_t dal_rtl8373_iol_100M_mode(rtk_uint32 port, rtk_uint32 mode)
{
    rtk_uint32 phymask;
    rtk_api_ret_t retVal;

    phymask = 1 << port;

    if ((retVal = dal_rtl8373_phy_write(phymask, 1, 0, 0x2000)) != RT_ERR_OK)
        return retVal;

    if ((retVal = dal_rtl8373_phy_write(phymask, 7, 0, 0x2000)) != RT_ERR_OK)
        return retVal;
    
    if ((retVal = dal_rtl8373_phy_write(phymask, 31, 0xA412, 0x0200)) != RT_ERR_OK)
        return retVal;

    if(mode == IOL_MODE_MDI)
    {
        if ((retVal = dal_rtl8373_phy_write(phymask, 31, 0xA430, 0x239A)) != RT_ERR_OK)
            return retVal;
    }
    else if(mode == IOL_MODE_MDIX)
    {
        if ((retVal = dal_rtl8373_phy_write(phymask, 31, 0xA430, 0x229A)) != RT_ERR_OK)
            return retVal;
    }
    else 
        return RT_ERR_INPUT;


    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_iol_giga_mode
 * Description:
 *      Set IOL Giga mode.
 * Input:
 *      port  - port id 0 - 8 
 *      mode - 1 - 4
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */

rtk_api_ret_t dal_rtl8373_iol_giga_mode(rtk_uint32 port, rtk_uint32 mode)
{
    rtk_uint32 phymask;
    rtk_api_ret_t retVal;

    phymask = 1 << port;

    if ((retVal = dal_rtl8373_phy_write(phymask, 1, 0, 0xA058)) != RT_ERR_OK)
        return retVal;

    if ((retVal = dal_rtl8373_phy_write(phymask, 7, 0, 0x3000)) != RT_ERR_OK)
        return retVal;
    

    if(mode == 1)
    {
        if ((retVal = dal_rtl8373_phy_write(phymask, 31, 0xA412, 0x3040)) != RT_ERR_OK)
            return retVal;
    }
    else if(mode == 2)
    {
        if ((retVal = dal_rtl8373_phy_write(phymask, 31, 0xA412, 0x5040)) != RT_ERR_OK)
            return retVal;
    }
    else if(mode == 3)
    {
        if ((retVal = dal_rtl8373_phy_write(phymask, 31, 0xA412, 0x7040)) != RT_ERR_OK)
            return retVal;
    }
    else if(mode == 4)
    {
        if ((retVal = dal_rtl8373_phy_write(phymask, 31, 0xA412, 0x9040)) != RT_ERR_OK)
            return retVal;
    }
    else 
        return RT_ERR_INPUT;


    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_iol_2p5G_mode2
 * Description:
 *      Set IOL 2.5G mode2.
 * Input:
 *      port  - port id 0 - 8 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */

rtk_api_ret_t dal_rtl8373_iol_2p5G_mode2(rtk_uint32 port)
{
    rtk_uint32 phymask;
    rtk_api_ret_t retVal;

    phymask = 1 << port;

    if ((retVal = dal_rtl8373_phy_write(phymask, 1, 0x84, 0x4400)) != RT_ERR_OK)
        return retVal;

    
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_iol_2p5G_mode3 and mode1
 * Description:
 *      Set IOL 2.5G mode3.
 * Input:
 *      port  - port id 0 - 8 
 *      lp     - link partner port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */

rtk_api_ret_t dal_rtl8373_iol_2p5G_mode3(rtk_uint32 port, rtk_uint32 lp)
{
    rtk_uint32 phymask, lpmask;
    rtk_api_ret_t retVal;

    phymask = 1 << port;
    lpmask = 1 << lp;

    if ((retVal = dal_rtl8373_phy_write(phymask, 1, 0x84, 0x6400)) != RT_ERR_OK)
        return retVal;
    
    if ((retVal = dal_rtl8373_phy_write(lpmask, 1, 0x84, 0x2400)) != RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_iol_2p5G_mode4
 * Description:
 *      Set IOL 2.5G mode4.
 * Input:
 *      port  - port id 0 - 8 
 *      tone - 1-5
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */

rtk_api_ret_t dal_rtl8373_iol_2p5G_mode4(rtk_uint32 port, rtk_uint32 tone)
{
    rtk_uint32 phymask;
    rtk_api_ret_t retVal;

    phymask = 1 << port;

    if(tone == 1)
    {
        if ((retVal = dal_rtl8373_phy_write(phymask, 1, 0x84, 0x8400)) != RT_ERR_OK)
            return retVal;
    }       
    else if(tone == 2)
    {
        if ((retVal = dal_rtl8373_phy_write(phymask, 1, 0x84, 0x8800)) != RT_ERR_OK)
            return retVal;
    }
    else if(tone == 3)
    {
        if ((retVal = dal_rtl8373_phy_write(phymask, 1, 0x84, 0x9000)) != RT_ERR_OK)
            return retVal;
    }
    else if(tone == 4)
    {
        if ((retVal = dal_rtl8373_phy_write(phymask, 1, 0x84, 0x9400)) != RT_ERR_OK)
            return retVal;
    }
    else if(tone == 5)
    {
        if ((retVal = dal_rtl8373_phy_write(phymask, 1, 0x84, 0x9800)) != RT_ERR_OK)
            return retVal;
    }
    else 
        return RT_ERR_INPUT;

    
    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_iol_2p5G_mode5
 * Description:
 *      Set IOL 2.5G mode5.
 * Input:
 *      port  - port id 0 - 8 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */

rtk_api_ret_t dal_rtl8373_iol_2p5G_mode5(rtk_uint32 port)
{
    rtk_uint32 phymask;
    rtk_api_ret_t retVal;

    phymask = 1 << port;

    if ((retVal = dal_rtl8373_phy_write(phymask, 1, 0x84, 0xA400)) != RT_ERR_OK)
        return retVal;

    
    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_iol_2p5G_mode6
 * Description:
 *      Set IOL 2.5G mode6.
 * Input:
 *      port  - port id 0 - 8 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */

rtk_api_ret_t dal_rtl8373_iol_2p5G_mode6(rtk_uint32 port)
{
    rtk_uint32 phymask;
    rtk_api_ret_t retVal;

    phymask = 1 << port;

    if ((retVal = dal_rtl8373_phy_write(phymask, 31, 0xA412, 0x0200)) != RT_ERR_OK)
        return retVal;

    if ((retVal = dal_rtl8373_phy_write(phymask, 7, 0x0, 0x3000)) != RT_ERR_OK)
        return retVal;

    if ((retVal = dal_rtl8373_phy_write(phymask, 1, 0x0, 0x2058)) != RT_ERR_OK)
        return retVal;

    if ((retVal = dal_rtl8373_phy_write(phymask, 1, 0x84, 0xC400)) != RT_ERR_OK)
        return retVal;

    
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_iol_2p5G_mode7
 * Description:
 *      Set IOL 2.5G mode7.
 * Input:
 *      port  - port id 0 - 8 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */

rtk_api_ret_t dal_rtl8373_iol_2p5G_mode7(rtk_uint32 port)
{
    rtk_uint32 phymask;
    rtk_api_ret_t retVal;    

    phymask = 1 << port;
    
    if ((retVal = dal_rtl8373_phy_write(phymask, 1, 0x84, 0xE400)) != RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}
/* Function Name:
 *      dal_rtl8373_iol_sd
 * Description:
 *      Set sds test mode.
 * Input:
 *      chip  - 8373/8373N/8372/8372N/8366U/8224 
 *      prbs_tx -prbs31/prbs9/tx_8081
 *      sdsid -0/1
 *      on_off -0/1
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */
rtk_api_ret_t dal_rtl8373_iol_sd(rtk_uint32 chip, rtk_uint32 prbs_tx, rtk_uint32 sdsid, rtk_uint32 on_off)
{
    //rtk_api_ret_t retVal;

    switch(chip)
    {
        case CHIP_RTL8372_MODE :
        case CHIP_RTL8373_MODE :
        case CHIP_RTL8366U_MODE:
        case CHIP_RTL8372N_MODE:
        case CHIP_RTL8373N_MODE:	
            if(prbs_tx == 31) //prbs31
            {
                if(on_off == 1)
                {
                     dal_rtl8373_sds_regbits_write(sdsid, 5, 0xa, 0x30, 3);
                     dal_rtl8373_sds_regbits_write(sdsid, 6, 0x00, 0x6000, 3);
                     dal_rtl8373_sds_regbits_write(sdsid, 6, 0x00, 0x1f00, 0x1f);
                     dal_rtl8373_sds_regbits_write(sdsid, 6, 0x0e, 0xffff, 0x1ff);
                     dal_rtl8373_sds_regbits_write(sdsid, 6, 0x0f, 0xffff, 0x1ff);
                }
                else
                {
                    dal_rtl8373_sds_reg_write(sdsid, 0x1e, 0xd, 0);
                    dal_rtl8373_sds_reg_write(sdsid, 0x05, 0x0a, 0);
                    dal_rtl8373_sds_reg_write(sdsid, 0x06, 0x00, 0x1f00);
                    dal_rtl8373_sds_reg_write(sdsid, 0x06, 0x0e, 0x1ff);
                    dal_rtl8373_sds_reg_write(sdsid, 0x06, 0x0f, 0x1ff);    
                }
            }
            else if(prbs_tx == 9) //prbs9
            {
                if (on_off == 1)
                {           
                     dal_rtl8373_sds_regbits_write(sdsid,5, 0xa, 0xc0, 3);
                     dal_rtl8373_sds_regbits_write(sdsid,6, 0x00, 0x6000,3);
                     dal_rtl8373_sds_regbits_write(sdsid,6, 0x00, 0x1f00, 0x1f);
                     dal_rtl8373_sds_regbits_write(sdsid,6, 0x0e, 0xffff, 0x1ff);
                     dal_rtl8373_sds_regbits_write(sdsid,6, 0x0f, 0xffff, 0x1ff);
                   
                }
                else
                {
                    dal_rtl8373_sds_reg_write(sdsid, 0x1e, 0xd, 0);
                    dal_rtl8373_sds_reg_write(sdsid, 0x05, 0x0a, 0);
                    dal_rtl8373_sds_reg_write(sdsid, 0x06, 0x00, 0x1f00);
                    dal_rtl8373_sds_reg_write(sdsid, 0x06, 0x0e, 0x1ff);
                    dal_rtl8373_sds_reg_write(sdsid, 0x06, 0x0f, 0x1ff);                    
                
                }
            }
            else if (prbs_tx == 8) //tx_8081
           {                
                if (on_off == 1)
                {
                    dal_rtl8373_sds_reg_write(sdsid, 0x1e, 0x0d, 0x0040);
                    dal_rtl8373_sds_reg_write(sdsid, 0x5, 0x0a, 0x0002);
                    dal_rtl8373_sds_reg_write(sdsid, 0x6, 0x00, 0x6080); //[expr ((8<<4)+24576)]
                }
                else
                {
                    dal_rtl8373_sds_reg_write(sdsid, 0x5, 0x0a, 0);
                }
            }
            break;
        case CHIP_RTL8224_MODE:
        case CHIP_RTL8224N_MODE:
            if(prbs_tx == 31) //prbs31
            {
                if(on_off == 1)
                {
                     dal_rtl8224_sds_regbits_write(sdsid, 5, 0xa, 0x30, 3);
                     dal_rtl8224_sds_regbits_write(sdsid, 6, 0x00, 0x6000, 3);
                     dal_rtl8224_sds_regbits_write(sdsid, 6, 0x00, 0x1f00, 0x1f);
                     dal_rtl8224_sds_regbits_write(sdsid, 6, 0x0e, 0xffff, 0x1ff);
                     dal_rtl8224_sds_regbits_write(sdsid, 6, 0x0f, 0xffff, 0x1ff);
                }
                else
                {
                    dal_rtl8224_sds_reg_write(sdsid, 0x1e, 0xd, 0);
                    dal_rtl8224_sds_reg_write(sdsid, 0x05, 0x0a, 0);
                    dal_rtl8224_sds_reg_write(sdsid, 0x06, 0x00, 0x1f00);
                    dal_rtl8224_sds_reg_write(sdsid, 0x06, 0x0e, 0x1ff);
                    dal_rtl8224_sds_reg_write(sdsid, 0x06, 0x0f, 0x1ff);    
                }
            }
            else if(prbs_tx == 9) //prbs9
            {
                if (on_off == 1)
                {           
                     dal_rtl8224_sds_regbits_write(sdsid,5, 0xa, 0xc0, 3);
                     dal_rtl8224_sds_regbits_write(sdsid,6, 0x00, 0x6000,3);
                     dal_rtl8224_sds_regbits_write(sdsid,6, 0x00, 0x1f00, 0x1f);
                     dal_rtl8224_sds_regbits_write(sdsid,6, 0x0e, 0xffff, 0x1ff);
                     dal_rtl8224_sds_regbits_write(sdsid,6, 0x0f, 0xffff, 0x1ff);
                   
                }
                else
                {
                    dal_rtl8224_sds_reg_write(sdsid, 0x1e, 0xd, 0);
                    dal_rtl8224_sds_reg_write(sdsid, 0x05, 0x0a, 0);
                    dal_rtl8224_sds_reg_write(sdsid, 0x06, 0x00, 0x1f00);
                    dal_rtl8224_sds_reg_write(sdsid, 0x06, 0x0e, 0x1ff);
                    dal_rtl8224_sds_reg_write(sdsid, 0x06, 0x0f, 0x1ff);                    
                
                }
            }
            else if (prbs_tx == 8) //tx_8081
           {                
                if (on_off == 1)
                {
                    dal_rtl8224_sds_reg_write(sdsid, 0x1e, 0x0d, 0x0040);
                    dal_rtl8224_sds_reg_write(sdsid, 0x5, 0x0a, 0x0002);
                    dal_rtl8224_sds_reg_write(sdsid, 0x6, 0x00, 0x6080); //[expr ((8<<4)+24576)]
                }
                else
                {
                    dal_rtl8224_sds_reg_write(sdsid, 0x5, 0x0a, 0);
                }
            }
            break;
            default :     
                return RT_ERR_OK;
            break;
    }

    return RT_ERR_OK;
}
/* Function Name:
 *      dal_rtl8373_iol_pre_amp
 * Description:
 *      Set sds test mode.
 * Input:
 *      chip  - 8373/8373N/8372/8372N/8366U/8224 
 *      pre   -0~63
 *      sdsid -0/1
 *      endis -0/1
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */
rtk_api_ret_t dal_rtl8373_iol_pre_amp(rtk_uint32 chip, rtk_uint32 pre, rtk_uint32 sdsid, rtk_uint32 endis)
{
    rtk_uint32 sds_mode, sds_page;

    rtl8373_getAsicRegBits (0x7b3c, 0x1f<<(16*sdsid), &sds_mode);
     if ((sds_mode == SERDES_10GUSXG) || (sds_mode == SERDES_10GR)) //10g
     {
        sds_page = 0x2e;
     }
     else if((sds_mode == SERDES_HSG) || (sds_mode == SERDES_2500BASEX))
    {
        sds_page = 0x28;
     }
     else if((sds_mode == SERDES_SG) || (sds_mode == SERDES_1000BASEX))
    {
        sds_page = 0x24;
     }
    else if(sds_mode == SERDES_100FX)
    {
        sds_page = 0x26;
     }
    else
    {
        sds_page = 0x2e;
    }
    switch(chip)
    {
        case CHIP_RTL8372_MODE :
        case CHIP_RTL8373_MODE :
        case CHIP_RTL8366U_MODE:
        case CHIP_RTL8372N_MODE:
        case CHIP_RTL8373N_MODE:	
            dal_rtl8373_sds_regbits_write(sdsid, sds_page, 0x7, 0x4, endis);
            dal_rtl8373_sds_regbits_write(sdsid, sds_page, 0x7, 0x1f8, pre);
            break;

        case CHIP_RTL8224_MODE:
        case CHIP_RTL8224N_MODE:    
            dal_rtl8224_sds_regbits_write(sdsid, sds_page, 0x7, 0x4, endis);
            dal_rtl8224_sds_regbits_write(sdsid, sds_page, 0x7, 0x1f8, pre);
            break;
        default :     
                return RT_ERR_OK;
            break;
    }

        return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_iol_main_amp
 * Description:
 *      Set sds test mode.
 * Input:
 *      chip  - 8373/8373N/8372/8372N/8366U/8224 
 *      main   -0~63
 *      sdsid -0/1
 *      boost -0/1
 *      endis -0/1
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */
rtk_api_ret_t dal_rtl8373_iol_main_amp(rtk_uint32 chip, rtk_uint32 main, rtk_uint32 sdsid,rtk_uint32 boost, rtk_uint32 endis)
{
    rtk_uint32 sds_mode, sds_page;

    rtl8373_getAsicRegBits (0x7b3c, 0x1f<<(16*sdsid), &sds_mode);
     if ((sds_mode == SERDES_10GUSXG) || (sds_mode == SERDES_10GR)) //10g
     {
        sds_page = 0x2e;
     }
     else if((sds_mode == SERDES_HSG) || (sds_mode == SERDES_2500BASEX))
    {
        sds_page = 0x28;
     }
     else if((sds_mode == SERDES_SG) || (sds_mode == SERDES_1000BASEX))
    {
        sds_page = 0x24;
     }
    else if(sds_mode == SERDES_100FX)
    {
        sds_page = 0x26;
     }
    else
    {
        sds_page = 0x2e;
    }
    switch(chip)
    {
        case CHIP_RTL8372_MODE :
        case CHIP_RTL8373_MODE :
        case CHIP_RTL8366U_MODE:
        case CHIP_RTL8372N_MODE:
        case CHIP_RTL8373N_MODE:	
            dal_rtl8373_sds_regbits_write(sdsid, sds_page, 0x7, 0x200, endis);
            dal_rtl8373_sds_regbits_write(sdsid, sds_page, 0x5, 0x20, boost);
            dal_rtl8373_sds_regbits_write(sdsid, sds_page, 0x7, 0xfc00, main);
            break;

        case CHIP_RTL8224_MODE:
        case CHIP_RTL8224N_MODE:    
            dal_rtl8224_sds_regbits_write(sdsid, sds_page, 0x7, 0x200, endis);
            dal_rtl8224_sds_regbits_write(sdsid, sds_page, 0x5, 0x20, boost);
            dal_rtl8224_sds_regbits_write(sdsid, sds_page, 0x7, 0xfc00, main);
            break;
        default :     
                return RT_ERR_OK;
            break;    
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_iol_post_amp
 * Description:
 *      Set sds test mode.
 * Input:
 *      chip  - 8373/8373N/8372/8372N/8366U/8224 
 *      post   -0~63
 *      sdsid -0/1
 *      endis -0/1
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */
rtk_api_ret_t dal_rtl8373_iol_post_amp(rtk_uint32 chip, rtk_uint32 post, rtk_uint32 sdsid, rtk_uint32 endis)
{
    rtk_uint32 sds_mode, sds_page;

    rtl8373_getAsicRegBits (0x7b3c, 0x1f<<(16*sdsid), &sds_mode);
     if ((sds_mode == SERDES_10GUSXG) || (sds_mode == SERDES_10GR)) //10g
     {
        sds_page = 0x2e;
     }
     else if((sds_mode == SERDES_HSG) || (sds_mode == SERDES_2500BASEX))
    {
        sds_page = 0x28;
     }
     else if((sds_mode == SERDES_SG) || (sds_mode == SERDES_1000BASEX))
    {
        sds_page = 0x24;
     }
    else if(sds_mode == SERDES_100FX)
    {
        sds_page = 0x26;
     }
    else
    {
        sds_page = 0x2e;
    }
    
    switch(chip)
    {
        case CHIP_RTL8372_MODE :
        case CHIP_RTL8373_MODE :
        case CHIP_RTL8366U_MODE:
        case CHIP_RTL8372N_MODE:
        case CHIP_RTL8373N_MODE:	
            dal_rtl8373_sds_regbits_write(sdsid, sds_page, 0x6, 0x8, endis);
            dal_rtl8373_sds_regbits_write(sdsid, sds_page, 0x6, 0xfc00, post);
            break;

        case CHIP_RTL8224_MODE:
        case CHIP_RTL8224N_MODE:    
            dal_rtl8224_sds_regbits_write(sdsid, sds_page, 0x6, 0x8, endis);
            dal_rtl8224_sds_regbits_write(sdsid, sds_page, 0x6, 0xfc00, post);
            break;
            
        default :     
                return RT_ERR_OK;
            break;
    }

        return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_iol_txz0
 * Description:
 *      Set sds test mode.
 * Input:
 *      chip  - 8373/8373N/8372/8372N/8366U/8224 
 *      sdsid -0/1
 *      txz0 -0~15
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */
rtk_api_ret_t dal_rtl8373_iol_txz0(rtk_uint32 chip,  rtk_uint32 sdsid, rtk_uint32 txz0)
{
    rtk_uint32 sds_mode, sds_page;

    rtl8373_getAsicRegBits (0x7b3c, 0x1f<<(16*sdsid), &sds_mode);
     if ((sds_mode == SERDES_10GUSXG) || (sds_mode == SERDES_10GR)) //10g
     {
        sds_page = 0x2e;
     }
     else if((sds_mode == SERDES_HSG) || (sds_mode == SERDES_2500BASEX))
    {
        sds_page = 0x28;
     }
     else if((sds_mode == SERDES_SG) || (sds_mode == SERDES_1000BASEX))
    {
        sds_page = 0x24;
     }
    else if(sds_mode == SERDES_100FX)
    {
        sds_page = 0x26;
     }
    else
    {
        sds_page = 0x2e;
    }
    
    switch(chip)
    {
        case CHIP_RTL8372_MODE :
        case CHIP_RTL8373_MODE :
        case CHIP_RTL8366U_MODE:
        case CHIP_RTL8372N_MODE:
        case CHIP_RTL8373N_MODE:	
            
            dal_rtl8373_sds_regbits_write(sdsid, sds_page, 0xb, 0x780, txz0);  
            break;

        case CHIP_RTL8224_MODE:
        case CHIP_RTL8224N_MODE:    
            dal_rtl8224_sds_regbits_write(sdsid, sds_page, 0xb, 0x780, txz0);
            break;
            
        default :     
                return RT_ERR_OK;
            break;
    }

        return RT_ERR_OK;
}

void SDS_DEBUG_PAD_OUT(rtk_uint32 sdsid, rtk_uint32 chip)
{
    rtk_uint32 DBG_CTRL_ADR0 = 0xc0b0;
    rtk_uint32 DBG_CTRL_ADR1 = 0xc0b4;
    rtk_uint32 DBG_CTRL_ADR2 = 0xc0b8;
    rtk_uint32 DBG_CTRL_ADR3 = 0xc0bc;

    rtk_uint32 DBG_CTRL_SEL0 = 0xc0c0;
    rtk_uint32 DBG_CTRL_SEL1 = 0xc0c4;
    rtk_uint32 DBG_CTRL_SEL2 = 0xc0c8;
    rtk_uint32 DBG_CTRL_SEL3 = 0xc0cc;
    //rtk_uint32 DBG_PAD_CTRL  = 0x34;

    if((chip == CHIP_RTL8373_MODE) ||(chip == CHIP_RTL8372_MODE)
        ||(chip == CHIP_RTL8373N_MODE)||(chip == CHIP_RTL8372N_MODE)||(chip == CHIP_RTL8366U_MODE))
    {
        switch (sdsid){
            case 0:
                rtl8373_setAsicReg(DBG_CTRL_ADR0, 0x0010);
                rtl8373_setAsicReg(DBG_CTRL_ADR1, 0x0010);
                rtl8373_setAsicReg(DBG_CTRL_ADR2, 0x0010);
                rtl8373_setAsicReg(DBG_CTRL_ADR3, 0x0010);
                break;

             case 1:
                rtl8373_setAsicReg(DBG_CTRL_ADR0, 0x0020);
                rtl8373_setAsicReg(DBG_CTRL_ADR1, 0x0020);
                rtl8373_setAsicReg(DBG_CTRL_ADR2, 0x0020);
                rtl8373_setAsicReg(DBG_CTRL_ADR3, 0x0020);
                break;
        }

        rtl8373_setAsicRegBits(DBG_CTRL_SEL0, 0x3, 0x0);
        rtl8373_setAsicRegBits(DBG_CTRL_SEL1, 0x3, 0x1);
        rtl8373_setAsicRegBits(DBG_CTRL_SEL2, 0x3, 0x2);
        rtl8373_setAsicRegBits(DBG_CTRL_SEL3, 0x3, 0x3);
        
    }
    else if((chip == CHIP_RTL8224_MODE) ||(chip == CHIP_RTL8224N_MODE))
    {
        switch (sdsid){

            case 0:
                dal_rtl8224_top_reg_write(DBG_CTRL_ADR0, 0x0010);
                dal_rtl8224_top_reg_write(DBG_CTRL_ADR1, 0x0010);
                dal_rtl8224_top_reg_write(DBG_CTRL_ADR2, 0x0010);
                dal_rtl8224_top_reg_write(DBG_CTRL_ADR3, 0x0010);
                break;

             case 1:
                dal_rtl8224_top_reg_write(DBG_CTRL_ADR0, 0x0020);
                dal_rtl8224_top_reg_write(DBG_CTRL_ADR1, 0x0020);
                dal_rtl8224_top_reg_write(DBG_CTRL_ADR2, 0x0020);
                dal_rtl8224_top_reg_write(DBG_CTRL_ADR3, 0x0020);
                break;
        }

        dal_rtl8224_top_regbits_write(DBG_CTRL_SEL0, 0x3, 0x0);
        dal_rtl8224_top_regbits_write(DBG_CTRL_SEL1, 0x3, 0x1);
        dal_rtl8224_top_regbits_write(DBG_CTRL_SEL2, 0x3, 0x2);
        dal_rtl8224_top_regbits_write(DBG_CTRL_SEL3, 0x3, 0x3);
    }
}

void WTGSDS_DEBUG_SEL(rtk_uint32 sdsid, rtk_uint32 sel, rtk_uint32 chip)
{
    if((chip == CHIP_RTL8373_MODE) ||(chip == CHIP_RTL8372_MODE)
        ||(chip == CHIP_RTL8373N_MODE)||(chip == CHIP_RTL8372N_MODE)||(chip == CHIP_RTL8366U_MODE))
    {
        dal_rtl8373_sds_reg_write(sdsid, 0x1f, 2, sel);        
    } 
    else if((chip == CHIP_RTL8224_MODE) ||(chip == CHIP_RTL8224N_MODE))
    {
        dal_rtl8224_sds_reg_write(sdsid, 0x1f, 2, sel);
    }
}

rtk_uint32 SDS_BUDEG_OUT_DATA(rtk_uint32 chip)
{
    rtk_uint32 DBG_CTRL_VAL = 0xc0d0;
    rtk_uint32 ret;

    if((chip == CHIP_RTL8373_MODE) ||(chip == CHIP_RTL8372_MODE)
        ||(chip == CHIP_RTL8373N_MODE)||(chip == CHIP_RTL8372N_MODE)||(chip == CHIP_RTL8366U_MODE))
    {
        rtl8373_getAsicReg(DBG_CTRL_VAL, &ret);        
    }
    else if((chip == CHIP_RTL8224_MODE) ||(chip == CHIP_RTL8224N_MODE))
    {
        dal_rtl8224_top_reg_read(DBG_CTRL_VAL, &ret);
    }
    
    return ret;
}

void EyeMonitorMDIOCLK(rtk_uint32 sdsid, rtk_uint32 chip)
{
    if((chip == CHIP_RTL8373_MODE) ||(chip == CHIP_RTL8372_MODE)
        ||(chip == CHIP_RTL8373N_MODE)||(chip == CHIP_RTL8372N_MODE)||(chip == CHIP_RTL8366U_MODE))
    {
            dal_rtl8373_sds_regbits_write(sdsid, 0x36, 0x19, 0x1,  1);
            dal_rtl8373_sds_regbits_write(sdsid, 0x36, 0x19, 0x1,  0);
            dal_rtl8373_sds_regbits_write(sdsid, 0x36, 0x19, 0x1,  1);
     }
    else if((chip == CHIP_RTL8224_MODE) ||(chip == CHIP_RTL8224N_MODE))
    {
            dal_rtl8224_sds_regbits_write(sdsid, 0x36, 0x19, 0x1,  1);
            dal_rtl8224_sds_regbits_write(sdsid, 0x36, 0x19, 0x1,  0);
            dal_rtl8224_sds_regbits_write(sdsid, 0x36, 0x19, 0x1,  1);
    }
}

rtk_uint32 dump_EYE_DATA(rtk_uint32 sdsid, rtk_uint32 dbg_out, rtk_uint32 chip)
{
    rtk_uint32 ret;
    rtk_uint32 tmp;
    if (dbg_out != 0)
    {
        SDS_DEBUG_PAD_OUT(sdsid, chip); 
        WTGSDS_DEBUG_SEL(sdsid, 31, chip); //#31 -> DEBUG_OUT

        if ((chip == CHIP_RTL8373_MODE) ||(chip == CHIP_RTL8372_MODE)
            ||(chip == CHIP_RTL8373N_MODE)||(chip == CHIP_RTL8372N_MODE)||(chip == CHIP_RTL8366U_MODE))
        {
            dal_rtl8373_sds_regbits_write(sdsid, 0x36, 0x05, 0x7800, 0xc); 
            dal_rtl8373_sds_regbits_write(sdsid, 0x36, 0x12, 0x7e0,  0);
        }
        else if((chip == CHIP_RTL8224_MODE) ||(chip == CHIP_RTL8224N_MODE))
        {
            dal_rtl8224_sds_regbits_write(sdsid, 0x36, 0x05, 0x7800, 0xc);
            dal_rtl8224_sds_regbits_write(sdsid, 0x36, 0x12, 0x7e0,  0);
        }
        
    }

    tmp = SDS_BUDEG_OUT_DATA(chip);   

    ret = tmp & 0xffff;    

    return ret;
}


/* Function Name:
 *      dal_rtl8373_iol_eye
 * Description:
 *      eye monitor , ’Î∂‘ RTL8373/2 s0/1  º∞ RTL8373_ind_ac_RTL8224 s0.
 * Input:
 *      chip  - 8373/8373N/8372/8372N/8366U/8224 
 *      sdsid -0/1
 *      frame -0~x
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */
rtk_api_ret_t dal_rtl8373_iol_eye(rtk_uint32 chip,  rtk_uint32 sdsid, rtk_uint32 frame)
{
    rtk_uint32 i, j, k = 0;
    rtk_uint32 data0, timeout, start = 0;
    
     if ((chip == CHIP_RTL8373_MODE) ||(chip == CHIP_RTL8372_MODE)
            ||(chip == CHIP_RTL8373N_MODE)||(chip == CHIP_RTL8372N_MODE)||(chip == CHIP_RTL8366U_MODE))
     {  
             dal_rtl8373_sds_regbits_write(sdsid, 0x21, 0x11, 0x1, 1);
             dal_rtl8373_sds_regbits_write(sdsid, 0x21, 0x11, 0x1, 0);
             dal_rtl8373_sds_regbits_write(sdsid, 0x21, 0x11, 0x1, 1);

            dal_rtl8373_sds_regbits_write(sdsid, 0x36, 0x10, 0x3f, 0x3f);
            dal_rtl8373_sds_regbits_write(sdsid, 0x2f, 0x16, 0x7e0, 0x3f);
            
       }
      else if((chip == CHIP_RTL8224_MODE) ||(chip == CHIP_RTL8224N_MODE))
      {
            rtlglue_printf("set 8224 eye monitor,sdsid: %d, ,frame: %d\n", sdsid, frame);
            dal_rtl8224_sds_regbits_write(sdsid, 0x21, 0x11, 0x1,  1); 
            dal_rtl8224_sds_regbits_write(sdsid, 0x21, 0x11, 0x1,  0);
            dal_rtl8224_sds_regbits_write(sdsid, 0x21, 0x11, 0x1,  1);
    						
            dal_rtl8224_sds_regbits_write(sdsid, 0x36, 0x10, 0x3f, 0x3f);
            dal_rtl8224_sds_regbits_write(sdsid, 0x2f, 0x16, 0x7e0, 0x3f);
      }

    for(i=0; i<64;i++)//64
    {
        if ((chip == CHIP_RTL8373_MODE) ||(chip == CHIP_RTL8372_MODE)
            ||(chip == CHIP_RTL8373N_MODE)||(chip == CHIP_RTL8372N_MODE)||(chip == CHIP_RTL8366U_MODE))
        {
            dal_rtl8373_sds_regbits_write(sdsid, 0x2f, 0x16, 0x7e0, i);
        }
        else if((chip == CHIP_RTL8224_MODE) ||(chip == CHIP_RTL8224N_MODE))
        {
            dal_rtl8224_sds_regbits_write(sdsid, 0x2f, 0x16, 0x7e0, i);
        }

        for(j=0; j<64;j++) //64
        {
            if ((chip == CHIP_RTL8373_MODE) ||(chip == CHIP_RTL8372_MODE)
            ||(chip == CHIP_RTL8373N_MODE)||(chip == CHIP_RTL8372N_MODE)||(chip == CHIP_RTL8366U_MODE))
            {
                dal_rtl8373_sds_regbits_write(sdsid, 0x36, 0x10, 0x3f, j);
           }
           else if((chip == CHIP_RTL8224_MODE) ||(chip == CHIP_RTL8224N_MODE))
           {
                dal_rtl8224_sds_regbits_write(sdsid, 0x36, 0x10, 0x3f, j);
           }

           EyeMonitorMDIOCLK(sdsid, chip);
           //data1 = dump_EYE_DATA(sdsid, 1, chip); //dbg_out Œ™string

           for(k=0; k<frame; k++)
           {                               

               EyeMonitorMDIOCLK(sdsid, chip);
               data0 = dump_EYE_DATA(sdsid, 0, chip);

               if ((data0 == 0) && (start == 1))
               {
                    timeout++; 
                    if (timeout > 2)
                    {
                        break; 
                    }
                    else
                    {
                        continue;

                    }
               }
               else
               {
                    start = 0;
               }

                if(data0 !=0)
                {
                    rtlglue_printf("[%d, %d : %d]\n",i , j, data0);                    
               }
               
           }

        }
    }   

        return RT_ERR_OK;
}


