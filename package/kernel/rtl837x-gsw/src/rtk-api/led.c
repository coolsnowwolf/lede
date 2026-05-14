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
 * Feature : Here is a list of all functions and variables in LED module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <led.h>
#include <string.h>
#include <dal/dal_mgmt.h>



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
 *      ASIC support 6 types of LED blinking rates at 32ms, 64ms, 128ms, 256ms, 512ms and 1024ms.
 */
rtk_api_ret_t rtk_led_blinkRate_set(rtk_led_blink_rate_t blinkRate)
{
     rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->led_blinkRate_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->led_blinkRate_set(blinkRate);    
    RTK_API_UNLOCK();

    return retVal;
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
 *      There are  6 types of LED blinking rates at32ms, 64ms, 128ms, 256ms, 512ms and 1024ms.
 */
rtk_api_ret_t rtk_led_blinkRate_get(rtk_led_blink_rate_t *pBlinkRate)
{
     rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->led_blinkRate_get)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->led_blinkRate_get(pBlinkRate);    
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_led_groupConfig_set
 * Description:
 *      Set per group Led to congiuration mode
 * Input:
 *      setid       -  4 groups led setting 0 ~ 3
 *      ledid       -  4 leds, 0 ~ 3
 *      pConfig  .
 *                      led_2p5g;
 *                      led_two_pair_1g;
 *                      led_1g;
 *                      led_500m;
 *                      led_100m;
 *                      led_10m;
 *                      led_link;
 *                      led_link_flash;
 *                      led_act;
 *                      led_rx;
 *                      led_tx;
 *                      led_col;
 *                      led_duplex;
 *                      led_training;
 *                      led_master;
 *                      led_10g;
 *                      led_two_pair_5g;
 *                      led_5g; 
 *                      led_two_pair_2p5g; 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can be used to enable LED per port per group.
 */

rtk_api_ret_t rtk_led_groupConfig_set(rtk_led_set_t setid, rtk_uint32 ledid, rtk_led_config_t * config)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->led_groupConfig_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->led_groupConfig_set(setid, ledid, config);    
    RTK_API_UNLOCK();

    return retVal;
}


/* Function Name:
 *      rtk_led_portSelConfig_set
 * Description:
 *      Set led group connfig for per port
 * Input:
 *      port   - port id 0 ~ 8
 *      setid  - led config group id 0 ~ 3 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      ASIC support 6 types of LED blinking rates at 32ms, 64ms, 128ms, 256ms, 512ms and 1024ms.
 */
rtk_api_ret_t rtk_led_portSelConfig_set(rtk_port_t port, rtk_led_set_t setid)
{
     rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->led_portSelConfig_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->led_portSelConfig_set(port, setid);    
    RTK_API_UNLOCK();

    return retVal;
}


/* Function Name:
 *      rtk_led_portSelConfig_get
 * Description:
 *      Get led group connfig for per port
 * Input:
 *      port   - port id 0 ~ 8
 *      setid  - led config group id 0 ~ 3 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      ASIC support 6 types of LED blinking rates at 32ms, 64ms, 128ms, 256ms, 512ms and 1024ms.
 */
rtk_api_ret_t rtk_led_portSelConfig_get(rtk_port_t port, rtk_led_set_t * setid)
{
     rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->led_portSelConfig_get)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->led_portSelConfig_get(port, setid);    
    RTK_API_UNLOCK();

    return retVal;
}

