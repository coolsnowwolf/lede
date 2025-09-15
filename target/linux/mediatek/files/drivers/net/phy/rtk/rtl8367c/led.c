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
 * Feature : Here is a list of all functions and variables in LED module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <led.h>
#include <string.h>

#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_led.h>


/* Function Name:
 *      rtk_led_enable_set
 * Description:
 *      Set Led enable configuration
 * Input:
 *      group       - LED group id.
 *      pPortmask   - LED enable port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_MASK    - Error portmask
 * Note:
 *      The API can be used to enable LED per port per group.
 */
rtk_api_ret_t rtk_led_enable_set(rtk_led_group_t group, rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;
    rtk_port_t port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (group >= LED_GROUP_END)
        return RT_ERR_INPUT;

    RTK_CHK_PORTMASK_VALID(pPortmask);

    RTK_PORTMASK_SCAN((*pPortmask), port)
    {
        if(rtk_switch_isCPUPort(port) == RT_ERR_OK)
            return RT_ERR_PORT_MASK;
    }

    if((retVal = rtk_switch_portmask_L2P_get(pPortmask, &pmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicLedGroupEnable(group, pmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_enable_get
 * Description:
 *      Get Led enable configuration
 * Input:
 *      group - LED group id.
 * Output:
 *      pPortmask - LED enable port mask.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can be used to get LED enable status.
 */
rtk_api_ret_t rtk_led_enable_get(rtk_led_group_t group, rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (group >= LED_GROUP_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_getAsicLedGroupEnable(group, &pmask)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtk_switch_portmask_P2L_get(pmask, pPortmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_led_operation_set
 * Description:
 *      Set Led operation mode
 * Input:
 *      mode - LED operation mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can set Led operation mode.
 *      The modes that can be set are as following:
 *      - LED_OP_SCAN,
 *      - LED_OP_PARALLEL,
 *      - LED_OP_SERIAL,
 */
rtk_api_ret_t rtk_led_operation_set(rtk_led_operation_t mode)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ( mode >= LED_OP_END)
      return RT_ERR_INPUT;

    switch (mode)
    {
        case LED_OP_PARALLEL:
            regData = LEDOP_PARALLEL;
            break;
        case LED_OP_SERIAL:
            regData = LEDOP_SERIAL;
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
            break;
    }

    if ((retVal = rtl8367c_setAsicLedOperationMode(regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_operation_get
 * Description:
 *      Get Led operation mode
 * Input:
 *      None
 * Output:
 *      pMode - Support LED operation mode.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get Led operation mode.
 *      The modes that can be set are as following:
 *      - LED_OP_SCAN,
 *      - LED_OP_PARALLEL,
 *      - LED_OP_SERIAL,
 */
rtk_api_ret_t rtk_led_operation_get(rtk_led_operation_t *pMode)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMode)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicLedOperationMode(&regData)) != RT_ERR_OK)
        return retVal;

    if (regData == LEDOP_SERIAL)
        *pMode = LED_OP_SERIAL;
    else if (regData ==LEDOP_PARALLEL)
        *pMode = LED_OP_PARALLEL;
    else
       return RT_ERR_FAILED;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_modeForce_set
 * Description:
 *      Set Led group to configuration force mode
 * Input:
 *      port    - port ID
 *      group   - Support LED group id.
 *      mode    - Support LED force mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      The API can force to one force mode.
 *      The force modes that can be set are as following:
 *      - LED_FORCE_NORMAL,
 *      - LED_FORCE_BLINK,
 *      - LED_FORCE_OFF,
 *      - LED_FORCE_ON.
 */
rtk_api_ret_t rtk_led_modeForce_set(rtk_port_t port, rtk_led_group_t group, rtk_led_force_mode_t mode)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    /* No LED for CPU port */
    if(rtk_switch_isCPUPort(port) == RT_ERR_OK)
        return RT_ERR_PORT_ID;

    if (group >= LED_GROUP_END)
        return RT_ERR_INPUT;

    if (mode >= LED_FORCE_END)
        return RT_ERR_NOT_ALLOWED;

    if ((retVal = rtl8367c_setAsicForceLed(rtk_switch_port_L2P_get(port), group, mode)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_modeForce_get
 * Description:
 *      Get Led group to configuration force mode
 * Input:
 *      port  - port ID
 *      group - Support LED group id.
 *      pMode - Support LED force mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      The API can get forced Led group mode.
 *      The force modes that can be set are as following:
 *      - LED_FORCE_NORMAL,
 *      - LED_FORCE_BLINK,
 *      - LED_FORCE_OFF,
 *      - LED_FORCE_ON.
 */
rtk_api_ret_t rtk_led_modeForce_get(rtk_port_t port, rtk_led_group_t group, rtk_led_force_mode_t *pMode)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    /* No LED for CPU port */
    if(rtk_switch_isCPUPort(port) == RT_ERR_OK)
        return RT_ERR_PORT_ID;

    if (group >= LED_GROUP_END)
        return RT_ERR_INPUT;

    if (NULL == pMode)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicForceLed(rtk_switch_port_L2P_get(port), group, pMode)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_blinkRate_set
 * Description:
 *      Set LED blinking rate
 * Input:
 *      blinkRate - blinking rate.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      ASIC support 6 types of LED blinking rates at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms.
 */
rtk_api_ret_t rtk_led_blinkRate_set(rtk_led_blink_rate_t blinkRate)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (blinkRate >= LED_BLINKRATE_END)
        return RT_ERR_FAILED;

    if ((retVal = rtl8367c_setAsicLedBlinkRate(blinkRate)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_blinkRate_get
 * Description:
 *      Get LED blinking rate at mode 0 to mode 3
 * Input:
 *      None
 * Output:
 *      pBlinkRate - blinking rate.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      There are  6 types of LED blinking rates at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms.
 */
rtk_api_ret_t rtk_led_blinkRate_get(rtk_led_blink_rate_t *pBlinkRate)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pBlinkRate)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicLedBlinkRate(pBlinkRate)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_groupConfig_set
 * Description:
 *      Set per group Led to configuration mode
 * Input:
 *      group   - LED group.
 *      config  - LED configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can set LED indicated information configuration for each LED group with 1 to 1 led mapping to each port.
 *      - Definition  LED Statuses      Description
 *      - 0000        LED_Off           LED pin Tri-State.
 *      - 0001        Dup/Col           Collision, Full duplex Indicator.
 *      - 0010        Link/Act          Link, Activity Indicator.
 *      - 0011        Spd1000           1000Mb/s Speed Indicator.
 *      - 0100        Spd100            100Mb/s Speed Indicator.
 *      - 0101        Spd10             10Mb/s Speed Indicator.
 *      - 0110        Spd1000/Act       1000Mb/s Speed/Activity Indicator.
 *      - 0111        Spd100/Act        100Mb/s Speed/Activity Indicator.
 *      - 1000        Spd10/Act         10Mb/s Speed/Activity Indicator.
 *      - 1001        Spd100 (10)/Act   10/100Mb/s Speed/Activity Indicator.
 *      - 1010        LoopDetect        LoopDetect Indicator.
 *      - 1011        EEE               EEE Indicator.
 *      - 1100        Link/Rx           Link, Activity Indicator.
 *      - 1101        Link/Tx           Link, Activity Indicator.
 *      - 1110        Master            Link on Master Indicator.
 *      - 1111        Act               Activity Indicator. Low for link established.
 */
rtk_api_ret_t rtk_led_groupConfig_set(rtk_led_group_t group, rtk_led_congig_t config)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (LED_GROUP_END <= group)
        return RT_ERR_FAILED;

    if (LED_CONFIG_END <= config)
        return RT_ERR_FAILED;

    if ((retVal = rtl8367c_setAsicLedIndicateInfoConfig(group, config)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_groupConfig_get
 * Description:
 *      Get Led group configuration mode
 * Input:
 *      group - LED group.
 * Output:
 *      pConfig - LED configuration.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *       The API can get LED indicated information configuration for each LED group.
 */
rtk_api_ret_t rtk_led_groupConfig_get(rtk_led_group_t group, rtk_led_congig_t *pConfig)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (LED_GROUP_END <= group)
        return RT_ERR_FAILED;

    if(NULL == pConfig)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicLedIndicateInfoConfig(group, pConfig)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_groupAbility_set
 * Description:
 *      Configure per group Led ability
 * Input:
 *      group    - LED group.
 *      pAbility - LED ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      None.
 */

rtk_api_ret_t rtk_led_groupAbility_set(rtk_led_group_t group, rtk_led_ability_t *pAbility)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (LED_GROUP_END <= group)
        return RT_ERR_FAILED;

    if(pAbility == NULL)
        return RT_ERR_NULL_POINTER;

    if( (pAbility->link_10m >= RTK_ENABLE_END) || (pAbility->link_100m >= RTK_ENABLE_END)||
        (pAbility->link_500m >= RTK_ENABLE_END) || (pAbility->link_1000m >= RTK_ENABLE_END)||
        (pAbility->act_rx >= RTK_ENABLE_END) || (pAbility->act_tx >= RTK_ENABLE_END) )
    {
        return RT_ERR_INPUT;
    }

    if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_LED0_DATA_CTRL + (rtk_uint32)group, &regData)) != RT_ERR_OK)
        return retVal;

    if(pAbility->link_10m == ENABLED)
        regData |= 0x0001;
    else
        regData &= ~0x0001;

    if(pAbility->link_100m == ENABLED)
        regData |= 0x0002;
    else
        regData &= ~0x0002;

    if(pAbility->link_500m == ENABLED)
        regData |= 0x0004;
    else
        regData &= ~0x0004;

    if(pAbility->link_1000m == ENABLED)
        regData |= 0x0008;
    else
        regData &= ~0x0008;

    if(pAbility->act_rx == ENABLED)
        regData |= 0x0010;
    else
        regData &= ~0x0010;

    if(pAbility->act_tx == ENABLED)
        regData |= 0x0020;
    else
        regData &= ~0x0020;

    regData |= (0x0001 << 6);

    if ((retVal = rtl8367c_setAsicReg(RTL8367C_REG_LED0_DATA_CTRL + (rtk_uint32)group, regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_groupAbility_get
 * Description:
 *      Get per group Led ability
 * Input:
 *      group    - LED group.
 *      pAbility - LED ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      None.
 */

rtk_api_ret_t rtk_led_groupAbility_get(rtk_led_group_t group, rtk_led_ability_t *pAbility)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (LED_GROUP_END <= group)
        return RT_ERR_FAILED;

    if(pAbility == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicReg(RTL8367C_REG_LED0_DATA_CTRL + (rtk_uint32)group, &regData)) != RT_ERR_OK)
        return retVal;

    pAbility->link_10m = (regData & 0x0001) ? ENABLED : DISABLED;
    pAbility->link_100m = (regData & 0x0002) ? ENABLED : DISABLED;
    pAbility->link_500m = (regData & 0x0004) ? ENABLED : DISABLED;
    pAbility->link_1000m = (regData & 0x0008) ? ENABLED : DISABLED;
    pAbility->act_rx = (regData & 0x0010) ? ENABLED : DISABLED;
    pAbility->act_tx = (regData & 0x0020) ? ENABLED : DISABLED;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_led_serialMode_set
 * Description:
 *      Set Led serial mode active configuration
 * Input:
 *      active - LED group.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can set LED serial mode active configuration.
 */
rtk_api_ret_t rtk_led_serialMode_set(rtk_led_active_t active)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ( active >= LED_ACTIVE_END)
        return RT_ERR_INPUT;

     if ((retVal = rtl8367c_setAsicLedSerialModeConfig(active,1))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_serialMode_get
 * Description:
 *      Get Led group congiuration mode
 * Input:
 *      group - LED group.
 * Output:
 *      pConfig - LED configuration.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *       The API can get LED serial mode active configuration.
 */
rtk_api_ret_t rtk_led_serialMode_get(rtk_led_active_t *pActive)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pActive)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicLedSerialModeConfig(pActive,&regData))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_OutputEnable_set
 * Description:
 *      This API set LED I/O state.
 * Input:
 *      enabled     - LED I/O state
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error parameter
 * Note:
 *      This API set LED I/O state.
 */
rtk_api_ret_t rtk_led_OutputEnable_set(rtk_enable_t state)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (state >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicLedOutputEnable(state))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_OutputEnable_get
 * Description:
 *      This API get LED I/O state.
 * Input:
 *      None.
 * Output:
 *      pEnabled        - LED I/O state
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error parameter
 * Note:
 *      This API set current LED I/O  state.
 */
rtk_api_ret_t rtk_led_OutputEnable_get(rtk_enable_t *pState)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(pState == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicLedOutputEnable(pState))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_led_serialModePortmask_set
 * Description:
 *      This API configure Serial LED output Group and portmask
 * Input:
 *      output          - output group
 *      pPortmask       - output portmask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error parameter
 * Note:
 *      None.
 */
rtk_api_ret_t rtk_led_serialModePortmask_set(rtk_led_serialOutput_t output, rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(output >= SERIAL_LED_END)
        return RT_ERR_INPUT;

    if(pPortmask == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtk_switch_portmask_L2P_get(pPortmask, &pmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicLedSerialOutput((rtk_uint32)output, pmask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_serialModePortmask_get
 * Description:
 *      This API get Serial LED output Group and portmask
 * Input:
 *      None.
 * Output:
 *      pOutput         - output group
 *      pPortmask       - output portmask
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error parameter
 * Note:
 *      None.
 */
rtk_api_ret_t rtk_led_serialModePortmask_get(rtk_led_serialOutput_t *pOutput, rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(pOutput == NULL)
        return RT_ERR_NULL_POINTER;

    if(pPortmask == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicLedSerialOutput((rtk_uint32 *)pOutput, &pmask))!=RT_ERR_OK)
        return retVal;

    if((retVal = rtk_switch_portmask_P2L_get(pmask, pPortmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

