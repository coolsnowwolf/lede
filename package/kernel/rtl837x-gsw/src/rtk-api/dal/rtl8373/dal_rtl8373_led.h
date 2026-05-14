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
 * Purpose : RTL8373 switch high-level API
 *
 * Feature : The file includes LED module high-layer API defination
 *
 */

#ifndef __DAL_RTL8373_LED_H__
#define __DAL_RTL8373_LED_H__
#include <led.h>

/* Function Name:
 *      dal_rtl8373_led_config_set
 * Description:
 *      Set Led congiuration
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
 *                      led_two_pair_2p5g; * Output:
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
extern rtk_api_ret_t dal_rtl8373_led_config_set(rtk_led_set_t setid, rtk_uint32 ledid, rtk_led_config_t *pConfig);


/* Function Name:
 *      dal_rtl8373_portLedConfig_set
 * Description:
 *      Select led config for per port
 * Input:
 *      port       -  port 0 ~ 8
 *      setid       -  4 leds, 0 ~ 3
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_MASK    - Error portmask
 * Note:
 *      The API can be used to set LED per port config.
 */
extern rtk_api_ret_t dal_rtl8373_portLedConfig_set(rtk_port_t port, rtk_led_set_t setid);


/* Function Name:
 *      dal_rtl8373_portLedConfig_get
 * Description:
 *      Get led config for per port
 * Input:
 *      port       -  port 0 ~ 8
 *      setid       -  4 leds, 0 ~ 3
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_MASK    - Error portmask
 * Note:
 *      The API can be used to get  LED per port config.
 */
extern rtk_api_ret_t dal_rtl8373_portLedConfig_get(rtk_port_t port, rtk_led_set_t * pSetid);




/* Function Name:
 *      dal_rtl8373_led_blinkRate_set
 * Description:
 *      Set Led blink rate
 * Input:
 *      rate      -  
 *                   LED_BLINKRATE_32MS=1,
 *                   LED_BLINKRATE_64MS,
 *                   LED_BLINKRATE_128MS,
 *                   LED_BLINKRATE_256MS,
 *                   LED_BLINKRATE_512MS,
 *                   LED_BLINKRATE_1024MS,.
 *                   LED_BLINKRATE_END, *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_MASK    - Error portmask
 * Note:
 *      The API can be used to enable LED per port per group.
 */
extern rtk_api_ret_t dal_rtl8373_led_blinkRate_set(rtk_led_blink_rate_t rate);

/* Function Name:
 *      dal_rtl8373_led_blinkRate_get
 * Description:
 *      Set Led blink rate
 * Input:
 *      pRate      -  
 *                   LED_BLINKRATE_32MS=1,
 *                   LED_BLINKRATE_64MS,
 *                   LED_BLINKRATE_128MS,
 *                   LED_BLINKRATE_256MS,
 *                   LED_BLINKRATE_512MS,
 *                   LED_BLINKRATE_1024MS,.
 *                   LED_BLINKRATE_END, *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_MASK    - Error portmask
 * Note:
 *      The API can be used to enable LED per port per group.
 */
extern rtk_api_ret_t dal_rtl8373_led_blinkRate_get(rtk_led_blink_rate_t * pRate);



#endif /* __DAL_RTL8373_LED_H__ */

