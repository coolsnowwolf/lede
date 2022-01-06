/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * Purpose : RTL8367/RTL8367C switch high-level API
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

typedef enum rtk_led_config_e
{
    LED_CONFIG_LEDOFF=0,
    LED_CONFIG_DUPCOL,
    LED_CONFIG_LINK_ACT,
    LED_CONFIG_SPD1000,
    LED_CONFIG_SPD100,
    LED_CONFIG_SPD10,
    LED_CONFIG_SPD1000ACT,
    LED_CONFIG_SPD100ACT,
    LED_CONFIG_SPD10ACT,
    LED_CONFIG_SPD10010ACT,
    LED_CONFIG_LOOPDETECT,
    LED_CONFIG_EEE,
    LED_CONFIG_LINKRX,
    LED_CONFIG_LINKTX,
    LED_CONFIG_MASTER,
    LED_CONFIG_ACT,
    LED_CONFIG_END,
}rtk_led_congig_t;

typedef struct rtk_led_ability_s
{
    rtk_enable_t link_10m;
    rtk_enable_t link_100m;
    rtk_enable_t link_500m;
    rtk_enable_t link_1000m;
    rtk_enable_t act_rx;
    rtk_enable_t act_tx;
}rtk_led_ability_t;

typedef enum rtk_led_blink_rate_e
{
    LED_BLINKRATE_32MS=0,
    LED_BLINKRATE_64MS,
    LED_BLINKRATE_128MS,
    LED_BLINKRATE_256MS,
    LED_BLINKRATE_512MS,
    LED_BLINKRATE_1024MS,
    LED_BLINKRATE_48MS,
    LED_BLINKRATE_96MS,
    LED_BLINKRATE_END,
}rtk_led_blink_rate_t;

typedef enum rtk_led_group_e
{
    LED_GROUP_0 = 0,
    LED_GROUP_1,
    LED_GROUP_2,
    LED_GROUP_END
}rtk_led_group_t;


typedef enum rtk_led_force_mode_e
{
    LED_FORCE_NORMAL=0,
    LED_FORCE_BLINK,
    LED_FORCE_OFF,
    LED_FORCE_ON,
    LED_FORCE_END
}rtk_led_force_mode_t;

typedef enum rtk_led_serialOutput_e
{
    SERIAL_LED_NONE = 0,
    SERIAL_LED_0,
    SERIAL_LED_0_1,
    SERIAL_LED_0_2,
    SERIAL_LED_END,
}rtk_led_serialOutput_t;


/* Function Name:
 *      rtk_led_enable_set
 * Description:
 *      Set Led enable congiuration
 * Input:
 *      group       - LED group id.
 *      pPortmask    - LED enable port mask.
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
extern rtk_api_ret_t rtk_led_enable_set(rtk_led_group_t group, rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_led_enable_get
 * Description:
 *      Get Led enable congiuration
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
extern rtk_api_ret_t rtk_led_enable_get(rtk_led_group_t group, rtk_portmask_t *pPortmask);

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
extern rtk_api_ret_t rtk_led_operation_set(rtk_led_operation_t mode);

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
extern rtk_api_ret_t rtk_led_operation_get(rtk_led_operation_t *pMode);

/* Function Name:
 *      rtk_led_modeForce_set
 * Description:
 *      Set Led group to congiuration force mode
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
extern rtk_api_ret_t rtk_led_modeForce_set(rtk_port_t port, rtk_led_group_t group, rtk_led_force_mode_t mode);

/* Function Name:
 *      rtk_led_modeForce_get
 * Description:
 *      Get Led group to congiuration force mode
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
extern rtk_api_ret_t rtk_led_modeForce_get(rtk_port_t port, rtk_led_group_t group, rtk_led_force_mode_t *pMode);

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
extern rtk_api_ret_t rtk_led_groupConfig_set(rtk_led_group_t group, rtk_led_congig_t config);

/* Function Name:
 *      rtk_led_groupConfig_get
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
 *       The API can get LED indicated information configuration for each LED group.
 */
extern rtk_api_ret_t rtk_led_groupConfig_get(rtk_led_group_t group, rtk_led_congig_t *pConfig);

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

extern rtk_api_ret_t rtk_led_groupAbility_set(rtk_led_group_t group, rtk_led_ability_t *pAbility);

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

extern rtk_api_ret_t rtk_led_groupAbility_get(rtk_led_group_t group, rtk_led_ability_t *pAbility);

/* Function Name:
 *      rtk_led_serialMode_set
 * Description:
 *      Set Led serial mode active congiuration
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
 *      The API can set LED serial mode active congiuration.
 */
extern rtk_api_ret_t rtk_led_serialMode_set(rtk_led_active_t active);

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
extern rtk_api_ret_t rtk_led_serialMode_get(rtk_led_active_t *pActive);

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
extern rtk_api_ret_t rtk_led_OutputEnable_set(rtk_enable_t state);


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
extern rtk_api_ret_t rtk_led_OutputEnable_get(rtk_enable_t *pState);

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
extern rtk_api_ret_t rtk_led_serialModePortmask_set(rtk_led_serialOutput_t output, rtk_portmask_t *pPortmask);

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
extern rtk_api_ret_t rtk_led_serialModePortmask_get(rtk_led_serialOutput_t *pOutput, rtk_portmask_t *pPortmask);

#endif /* __RTK_API_LED_H__ */
