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
 * Feature : LED related functions
 *
 */
#include <rtl8367c_asicdrv_led.h>
/* Function Name:
 *      rtl8367c_setAsicLedIndicateInfoConfig
 * Description:
 *      Set Leds indicated information mode
 * Input:
 *      ledno   - LED group number. There are 1 to 1 led mapping to each port in each led group
 *      config  - Support 16 types configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      The API can set LED indicated information configuration for each LED group with 1 to 1 led mapping to each port.
 *      Definition        LED Statuses            Description
 *      0000        LED_Off                LED pin Tri-State.
 *      0001        Dup/Col                Collision, Full duplex Indicator. Blinking every 43ms when collision happens. Low for full duplex, and high for half duplex mode.
 *      0010        Link/Act               Link, Activity Indicator. Low for link established. Link/Act Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      0011        Spd1000                1000Mb/s Speed Indicator. Low for 1000Mb/s.
 *      0100        Spd100                 100Mb/s Speed Indicator. Low for 100Mb/s.
 *      0101        Spd10                  10Mb/s Speed Indicator. Low for 10Mb/s.
 *      0110        Spd1000/Act            1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      0111        Spd100/Act             100Mb/s Speed/Activity Indicator. Low for 100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      1000        Spd10/Act              10Mb/s Speed/Activity Indicator. Low for 10Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      1001        Spd100 (10)/Act        10/100Mb/s Speed/Activity Indicator. Low for 10/100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      1010        Fiber                  Fiber link Indicator. Low for Fiber.
 *      1011        Fault                  Auto-negotiation     Fault Indicator. Low for Fault.
 *      1100        Link/Rx                Link, Activity Indicator. Low for link established. Link/Rx Blinks every 43ms when the corresponding port is transmitting.
 *      1101        Link/Tx                Link, Activity Indicator. Low for link established. Link/Tx Blinks every 43ms when the corresponding port is receiving.
 *      1110        Master                 Link on Master Indicator. Low for link Master established.
 *      1111        LED_Force              Force LED output, LED output value reference
 */
ret_t rtl8367c_setAsicLedIndicateInfoConfig(rtk_uint32 ledno, rtk_uint32 config)
{
    ret_t   retVal;
    CONST rtk_uint16 bits[RTL8367C_LEDGROUPNO] = {RTL8367C_LED0_CFG_MASK, RTL8367C_LED1_CFG_MASK, RTL8367C_LED2_CFG_MASK};

    if(ledno >= RTL8367C_LEDGROUPNO)
        return RT_ERR_OUT_OF_RANGE;

    if(config >= LEDCONF_END)
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_LED_CONFIGURATION, RTL8367C_LED_CONFIG_SEL_OFFSET, 0);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_LED_CONFIGURATION, bits[ledno], config);
}
/* Function Name:
 *      rtl8367c_getAsicLedIndicateInfoConfig
 * Description:
 *      Get Leds indicated information mode
 * Input:
 *      ledno   - LED group number. There are 1 to 1 led mapping to each port in each led group
 *      pConfig     - Support 16 types configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLedIndicateInfoConfig(rtk_uint32 ledno, rtk_uint32* pConfig)
{
    CONST rtk_uint16 bits[RTL8367C_LEDGROUPNO]= {RTL8367C_LED0_CFG_MASK, RTL8367C_LED1_CFG_MASK, RTL8367C_LED2_CFG_MASK};

    if(ledno >= RTL8367C_LEDGROUPNO)
        return RT_ERR_OUT_OF_RANGE;

    /* Get register value */
    return rtl8367c_getAsicRegBits(RTL8367C_REG_LED_CONFIGURATION, bits[ledno], pConfig);
}
/* Function Name:
 *      rtl8367c_setAsicLedGroupMode
 * Description:
 *      Set Led Group mode
 * Input:
 *      mode    - LED mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLedGroupMode(rtk_uint32 mode)
{
    ret_t retVal;

    /* Invalid input parameter */
    if(mode >= RTL8367C_LED_MODE_END)
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_LED_CONFIGURATION, RTL8367C_LED_CONFIG_SEL_OFFSET, 1);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_LED_CONFIGURATION, RTL8367C_DATA_LED_MASK, mode);
}
/* Function Name:
 *      rtl8367c_getAsicLedGroupMode
 * Description:
 *      Get Led Group mode
 * Input:
 *      pMode   - LED mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLedGroupMode(rtk_uint32* pMode)
{
    ret_t retVal;
    rtk_uint32 regData;

    retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_LED_CONFIGURATION, RTL8367C_LED_CONFIG_SEL_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    if(regData!=1)
        return RT_ERR_FAILED;

    return rtl8367c_getAsicRegBits(RTL8367C_REG_LED_CONFIGURATION, RTL8367C_DATA_LED_MASK, pMode);
}
/* Function Name:
 *      rtl8367c_setAsicForceLeds
 * Description:
 *      Set group LED mode
 * Input:
 *      port    - Physical port number (0~7)
 *      group   - LED group number
 *      mode    - LED mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicForceLed(rtk_uint32 port, rtk_uint32 group, rtk_uint32 mode)
{
    rtk_uint16 regAddr;
    ret_t retVal;

    /* Invalid input parameter */
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(group >= RTL8367C_LEDGROUPNO)
        return RT_ERR_OUT_OF_RANGE;

    if(mode >= LEDFORCEMODE_END)
        return RT_ERR_OUT_OF_RANGE;
    /* Set Related Registers */
    if(port < 8){
        regAddr = RTL8367C_LED_FORCE_MODE_BASE + (group << 1);
        if((retVal = rtl8367c_setAsicRegBits(regAddr, 0x3 << (port * 2), mode)) != RT_ERR_OK)
            return retVal;
    }else if(port >= 8){
        regAddr = RTL8367C_REG_CPU_FORCE_LED0_CFG1 + (group << 1);
        if((retVal = rtl8367c_setAsicRegBits(regAddr, 0x3 << ((port-8) * 2), mode)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicForceLed
 * Description:
 *      Get group LED mode
 * Input:
 *      port    - Physical port number (0~7)
 *      group   - LED group number
 *      pMode   - LED mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicForceLed(rtk_uint32 port, rtk_uint32 group, rtk_uint32* pMode)
{
    rtk_uint16 regAddr;
    ret_t retVal;

    /* Invalid input parameter */
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(group >= RTL8367C_LEDGROUPNO)
        return RT_ERR_INPUT;

    /* Get Related Registers */
    if(port < 8){
        regAddr = RTL8367C_LED_FORCE_MODE_BASE + (group << 1);
        if((retVal = rtl8367c_getAsicRegBits(regAddr, 0x3 << (port * 2), pMode)) != RT_ERR_OK)
            return retVal;
    }else if(port >= 8){
        regAddr = RTL8367C_REG_CPU_FORCE_LED0_CFG1 + (group << 1);
        if((retVal = rtl8367c_getAsicRegBits(regAddr, 0x3 << ((port-8) * 2), pMode)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicForceGroupLed
 * Description:
 *      Turn on/off Led of all ports
 * Input:
 *      group   - LED group number
 *      mode    - 0b00:normal mode, 0b01:force blink, 0b10:force off, 0b11:force on
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicForceGroupLed(rtk_uint32 groupmask, rtk_uint32 mode)
{
    ret_t retVal;
    rtk_uint32 i,bitmask;
    CONST rtk_uint16 bits[3]= {0x0004,0x0010,0x0040};

    /* Invalid input parameter */
    if(groupmask > RTL8367C_LEDGROUPMASK)
        return RT_ERR_OUT_OF_RANGE;

    if(mode >= LEDFORCEMODE_END)
        return RT_ERR_OUT_OF_RANGE;

    bitmask = 0;
    for(i = 0; i <  RTL8367C_LEDGROUPNO; i++)
    {
        if(groupmask & (1 << i))
        {
            bitmask = bitmask | bits[i];
        }

    }

    retVal = rtl8367c_setAsicRegBits(RTL8367C_LED_FORCE_CTRL, RTL8367C_LED_FORCE_MODE_MASK, bitmask);

    retVal = rtl8367c_setAsicRegBits(RTL8367C_LED_FORCE_CTRL, RTL8367C_FORCE_MODE_MASK, mode);

    if(LEDFORCEMODE_NORMAL == mode)
        retVal = rtl8367c_setAsicRegBits(RTL8367C_LED_FORCE_CTRL, RTL8367C_LED_FORCE_MODE_MASK, 0);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicForceGroupLed
 * Description:
 *      Turn on/off Led of all ports
 * Input:
 *      group   - LED group number
 *      pMode   - 0b00:normal mode, 0b01:force blink, 0b10:force off, 0b11:force on
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicForceGroupLed(rtk_uint32* groupmask, rtk_uint32* pMode)
{
    ret_t retVal;
    rtk_uint32 i,regData;
    CONST rtk_uint16 bits[3] = {0x0004,0x0010,0x0040};

    /* Get Related Registers */
    if((retVal = rtl8367c_getAsicRegBits(RTL8367C_LED_FORCE_CTRL, RTL8367C_LED_FORCE_MODE_MASK, &regData)) != RT_ERR_OK)
        return retVal;

    for(i = 0; i< RTL8367C_LEDGROUPNO; i++)
    {
        if((regData & bits[i]) == bits[i])
        {
            *groupmask = *groupmask | (1 << i);
        }
    }

    return rtl8367c_getAsicRegBits(RTL8367C_LED_FORCE_CTRL, RTL8367C_FORCE_MODE_MASK, pMode);
}
/* Function Name:
 *      rtl8367c_setAsicLedBlinkRate
 * Description:
 *      Set led blinking rate at mode 0 to mode 3
 * Input:
 *      blinkRate   - Support 6 blink rates
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      LED blink rate can be at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms
 */
ret_t rtl8367c_setAsicLedBlinkRate(rtk_uint32 blinkRate)
{
    if(blinkRate >= LEDBLINKRATE_END)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_LED_MODE, RTL8367C_SEL_LEDRATE_MASK, blinkRate);
}
/* Function Name:
 *      rtl8367c_getAsicLedBlinkRate
 * Description:
 *      Get led blinking rate at mode 0 to mode 3
 * Input:
 *      pBlinkRate  - Support 6 blink rates
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLedBlinkRate(rtk_uint32* pBlinkRate)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_LED_MODE, RTL8367C_SEL_LEDRATE_MASK, pBlinkRate);
}
/* Function Name:
 *      rtl8367c_setAsicLedForceBlinkRate
 * Description:
 *      Set LEd blinking rate for force mode led
 * Input:
 *      blinkRate   - Support 6 blink rates
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLedForceBlinkRate(rtk_uint32 blinkRate)
{
    if(blinkRate >= LEDFORCERATE_END)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_LED_MODE, RTL8367C_FORCE_RATE_MASK, blinkRate);
}
/* Function Name:
 *      rtl8367c_getAsicLedForceBlinkRate
 * Description:
 *      Get LED blinking rate for force mode led
 * Input:
 *      pBlinkRate  - Support 6 blink rates
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLedForceBlinkRate(rtk_uint32* pBlinkRate)
{
     return rtl8367c_getAsicRegBits(RTL8367C_REG_LED_MODE, RTL8367C_FORCE_RATE_MASK, pBlinkRate);
}

/*
@func ret_t | rtl8367c_setAsicLedGroupEnable | Turn on/off Led of all system ports
@parm rtk_uint32 | group | LED group id.
@parm rtk_uint32 | portmask | LED port mask.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off leds of dedicated port while indicated information configuration of LED group is set to force mode.
 */
ret_t rtl8367c_setAsicLedGroupEnable(rtk_uint32 group, rtk_uint32 portmask)
{
    ret_t retVal;
    rtk_uint32 regAddr;
    rtk_uint32 regDataMask;

    if ( group >= RTL8367C_LEDGROUPNO )
        return RT_ERR_INPUT;

    regAddr = RTL8367C_REG_PARA_LED_IO_EN1 + group/2;
    regDataMask = 0xFF << ((group%2)*8);
    retVal = rtl8367c_setAsicRegBits(regAddr, regDataMask, portmask&0xff);
    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8367C_REG_PARA_LED_IO_EN3;
    regDataMask = 0x3 << (group*2);
    retVal = rtl8367c_setAsicRegBits(regAddr, regDataMask, (portmask>>8)&0x7);
    if(retVal != RT_ERR_OK)
        return retVal;


    return RT_ERR_OK;
}

/*
@func ret_t | rtl8367c_getAsicLedGroupEnable | Get on/off status of Led of all system ports
@parm rtk_uint32 | group | LED group id.
@parm rtk_uint32 | *portmask | LED port mask.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off leds of dedicated port while indicated information configuration of LED group is set to force mode.
 */
ret_t rtl8367c_getAsicLedGroupEnable(rtk_uint32 group, rtk_uint32 *portmask)
{
    ret_t retVal;
    rtk_uint32 regAddr;
    rtk_uint32 regDataMask,regData;

    if ( group >= RTL8367C_LEDGROUPNO )
        return RT_ERR_INPUT;

    regAddr = RTL8367C_REG_PARA_LED_IO_EN1 + group/2;
    regDataMask = 0xFF << ((group%2)*8);
    retVal = rtl8367c_getAsicRegBits(regAddr, regDataMask, portmask);
    if(retVal != RT_ERR_OK)
        return retVal;


    regAddr = RTL8367C_REG_PARA_LED_IO_EN3;
    regDataMask = 0x3 << (group*2);
    retVal = rtl8367c_getAsicRegBits(regAddr, regDataMask, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *portmask = (regData << 8) | *portmask;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8367c_setAsicLedOperationMode | Set LED operation mode
@parm rtk_uint32 | mode | LED mode. 1:scan mode 1, 2:parallel mode, 3:mdx mode (serial mode)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off led serial mode and set signal to active high/low.
 */
ret_t rtl8367c_setAsicLedOperationMode(rtk_uint32 mode)
{
    ret_t retVal;

    /* Invalid input parameter */
    if( mode >= LEDOP_END)
        return RT_ERR_INPUT;

    switch(mode)
    {
        case LEDOP_PARALLEL:
            if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_LED_SYS_CONFIG, RTL8367C_LED_SELECT_OFFSET, 0))!=  RT_ERR_OK)
                return retVal;
            /*Disable serial CLK mode*/
            if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_SCAN0_LED_IO_EN1,RTL8367C_LED_SERI_CLK_EN_OFFSET, 0))!=  RT_ERR_OK)
                return retVal;
            /*Disable serial DATA mode*/
            if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_SCAN0_LED_IO_EN1,RTL8367C_LED_SERI_DATA_EN_OFFSET, 0))!=  RT_ERR_OK)
                return retVal;
            break;
        case LEDOP_SERIAL:
            if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_LED_SYS_CONFIG, RTL8367C_LED_SELECT_OFFSET, 1))!=  RT_ERR_OK)
                return retVal;
            /*Enable serial CLK mode*/
            if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_SCAN0_LED_IO_EN1,RTL8367C_LED_SERI_CLK_EN_OFFSET, 1))!=  RT_ERR_OK)
                return retVal;
            /*Enable serial DATA mode*/
            if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_SCAN0_LED_IO_EN1,RTL8367C_LED_SERI_DATA_EN_OFFSET, 1))!=  RT_ERR_OK)
                return retVal;
            break;
        default:
            return RT_ERR_INPUT;
            break;
    }

    return RT_ERR_OK;
}


/*
@func ret_t | rtl8367c_getAsicLedOperationMode | Get LED OP mode setup
@parm rtk_uint32*| mode | LED mode. 1:scan mode 1, 2:parallel mode, 3:mdx mode (serial mode)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can get LED serial mode setup and get signal active high/low.
 */
ret_t rtl8367c_getAsicLedOperationMode(rtk_uint32 *mode)
{
    ret_t retVal;
    rtk_uint32 regData;

    if((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_LED_SYS_CONFIG, RTL8367C_LED_SELECT_OFFSET, &regData))!=  RT_ERR_OK)
        return retVal;

    if (regData == 1)
        *mode = LEDOP_SERIAL;
    else if (regData == 0)
        *mode = LEDOP_PARALLEL;
    else
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8367c_setAsicLedSerialModeConfig | Set LED serial mode
@parm rtk_uint32 | active | Active High or Low.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off led serial mode and set signal to active high/low.
 */
ret_t rtl8367c_setAsicLedSerialModeConfig(rtk_uint32 active, rtk_uint32 serimode)
{
    ret_t retVal;

    /* Invalid input parameter */
    if( active >= LEDSERACT_MAX)
        return RT_ERR_INPUT;
    if( serimode >= LEDSER_MAX)
        return RT_ERR_INPUT;

    /* Set Active High or Low */
    if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_LED_SYS_CONFIG, RTL8367C_SERI_LED_ACT_LOW_OFFSET, active)) !=  RT_ERR_OK)
        return retVal;

    /*set to 8G mode (not 16G mode)*/
    if((retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_LED_MODE, RTL8367C_DLINK_TIME_OFFSET, serimode))!=  RT_ERR_OK)
        return retVal;


    return RT_ERR_OK;
}


/*
@func ret_t | rtl8367c_getAsicLedSerialModeConfig | Get LED serial mode setup
@parm rtk_uint32*| active | Active High or Low.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can get LED serial mode setup and get signal active high/low.
 */
ret_t rtl8367c_getAsicLedSerialModeConfig(rtk_uint32 *active, rtk_uint32 *serimode)
{
    ret_t retVal;

    if((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_LED_SYS_CONFIG, RTL8367C_SERI_LED_ACT_LOW_OFFSET, active))!=  RT_ERR_OK)
        return retVal;

    /*get to 8G mode (not 16G mode)*/
    if((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_LED_MODE, RTL8367C_DLINK_TIME_OFFSET, serimode))!=  RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/*
@func ret_t | rtl8367c_setAsicLedOutputEnable | Set LED output enable
@parm rtk_uint32 | enabled | enable or disalbe.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off LED output Enable
 */
ret_t rtl8367c_setAsicLedOutputEnable(rtk_uint32 enabled)
{
    ret_t retVal;
    rtk_uint32 regdata;

    if (enabled == 1)
        regdata = 0;
    else
        regdata = 1;

    /* Enable/Disable H/W IGMP/MLD */
    retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_LED_SYS_CONFIG, RTL8367C_LED_IO_DISABLE_OFFSET, regdata);

    return retVal;
}


/*
@func ret_t | rtl8367c_getAsicLedOutputEnable | Get LED serial mode setup
@parm rtk_uint32*| active | Active High or Low.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can get LED serial mode setup and get signal active high/low.
 */
ret_t rtl8367c_getAsicLedOutputEnable(rtk_uint32 *ptr_enabled)
{
    ret_t retVal;
    rtk_uint32 regdata;

    retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_LED_SYS_CONFIG, RTL8367C_LED_IO_DISABLE_OFFSET, &regdata);
    if (retVal != RT_ERR_OK)
        return retVal;

    if (regdata == 1)
        *ptr_enabled = 0;
    else
        *ptr_enabled = 1;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicLedSerialOutput
 * Description:
 *      Set serial LED output group and portmask.
 * Input:
 *      output      - Serial LED output group
 *      pmask       - Serial LED output portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLedSerialOutput(rtk_uint32 output, rtk_uint32 pmask)
{
    ret_t retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_SERIAL_LED_CTRL, RTL8367C_SERIAL_LED_GROUP_NUM_MASK, output);
    if (retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_SERIAL_LED_CTRL, RTL8367C_SERIAL_LED_PORT_EN_MASK, pmask);
    if (retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicLedSerialOutput
 * Description:
 *      Get serial LED output group and portmask.
 * Input:
 *      None
 * Output:
 *      pOutput      - Serial LED output group
 *      pPmask       - Serial LED output portmask
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLedSerialOutput(rtk_uint32 *pOutput, rtk_uint32 *pPmask)
{
    ret_t retVal;

    if(pOutput == NULL)
        return RT_ERR_NULL_POINTER;

    if(pPmask == NULL)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_SERIAL_LED_CTRL, RTL8367C_SERIAL_LED_GROUP_NUM_MASK, pOutput);
    if (retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_SERIAL_LED_CTRL, RTL8367C_SERIAL_LED_PORT_EN_MASK, pPmask);
    if (retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

