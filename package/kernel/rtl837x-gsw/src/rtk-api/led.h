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

#ifndef __RTK_API_LED_H__
#define __RTK_API_LED_H__

typedef enum rtk_led_operation_e
{
    LED_OP_SCAN=0,
    LED_OP_PARALLEL,
    LED_OP_SERIAL,
    LED_OP_END,
}rtk_led_operation_t;


typedef enum rtk_led_active_e
{
    LED_ACTIVE_HIGH=0,
    LED_ACTIVE_LOW,
    LED_ACTIVE_END,
}rtk_led_active_t;


typedef struct rtk_led_ability_s
{
    rtk_enable_t link_10m;
    rtk_enable_t link_100m;
    rtk_enable_t link_500m;
    rtk_enable_t link_1000m;
    rtk_enable_t link_2500m;
    rtk_enable_t act_rx;
    rtk_enable_t act_tx;
}rtk_led_ability_t;

typedef enum rtk_led_blink_rate_e
{
    LED_BLINKRATE_32MS=1,
    LED_BLINKRATE_64MS,
    LED_BLINKRATE_128MS,
    LED_BLINKRATE_256MS,
    LED_BLINKRATE_512MS,
    LED_BLINKRATE_1024MS,
    LED_BLINKRATE_END,
}rtk_led_blink_rate_t;

typedef enum rtk_led_set_e
{
    LED_SET_0 = 0,
    LED_SET_1,
    LED_SET_2,
    LED_SET_3,
    LED_SET_END
}rtk_led_set_t;


typedef struct rtk_led_config_e
{
    rtk_uint8 led_2p5g;
    rtk_uint8 led_two_pair_1g;
    rtk_uint8 led_1g;
    rtk_uint8 led_500m;
    rtk_uint8 led_100m;
    rtk_uint8 led_10m;
    rtk_uint8 led_link;
    rtk_uint8 led_link_flash;
    rtk_uint8 led_act;
    rtk_uint8 led_rx;
    rtk_uint8 led_tx;
    rtk_uint8 led_col;
    rtk_uint8 led_duplex;
    rtk_uint8 led_training;
    rtk_uint8 led_master;
    rtk_uint8 led_10g;
    rtk_uint8 led_two_pair_5g;
    rtk_uint8 led_5g;
    rtk_uint8 led_two_pair_2p5g;
}rtk_led_config_t;


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
extern rtk_api_ret_t rtk_led_blinkRate_set(rtk_led_blink_rate_t blinkRate);

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
extern rtk_api_ret_t rtk_led_blinkRate_get(rtk_led_blink_rate_t *pBlinkRate);

/* Function Name:
 *      rtk_led_groupConfig_set
 * Description:
 *      Set per group Led to congiuration mode
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
extern rtk_api_ret_t rtk_led_groupConfig_set(rtk_led_set_t setid, rtk_uint32 ledid, rtk_led_config_t * config);

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
extern rtk_api_ret_t rtk_led_portSelConfig_set(rtk_port_t port, rtk_led_set_t setid);


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
extern rtk_api_ret_t rtk_led_portSelConfig_get(rtk_port_t port, rtk_led_set_t * setid);


#endif /* __RTK_API_LED_H__ */

