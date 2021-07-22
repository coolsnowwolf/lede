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
 * Purpose : RTL8367/RTL8367C switch high-level API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OAM (802.3ah) configuration
 *
 */

#ifndef __RTK_OAM_H__
#define __RTK_OAM_H__

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */


/*
 * Macro Declaration
 */

typedef enum rtk_oam_parser_act_e
{
    OAM_PARSER_ACTION_FORWARD = 0,
    OAM_PARSER_ACTION_LOOPBACK,
    OAM_PARSER_ACTION_DISCARD,
    OAM_PARSER_ACTION_END,

} rtk_oam_parser_act_t;

typedef enum rtk_oam_multiplexer_act_e
{
    OAM_MULTIPLEXER_ACTION_FORWARD = 0,
    OAM_MULTIPLEXER_ACTION_DISCARD,
    OAM_MULTIPLEXER_ACTION_CPUONLY,
    OAM_MULTIPLEXER_ACTION_END,

} rtk_oam_multiplexer_act_t;


/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_oam_init
 * Description:
 *      Initialize oam module.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize oam module before calling any oam APIs.
 */
extern rtk_api_ret_t rtk_oam_init(void);

/* Function Name:
 *      rtk_oam_state_set
 * Description:
 *      This API set OAM state.
 * Input:
 *      enabled     -OAMstate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error parameter
 * Note:
 *      This API set OAM state.
 */
extern rtk_api_ret_t rtk_oam_state_set(rtk_enable_t enabled);

/* Function Name:
 *      rtk_oam_state_get
 * Description:
 *      This API get OAM state.
 * Input:
 *      None.
 * Output:
 *      pEnabled        - H/W IGMP state
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error parameter
 * Note:
 *      This API set current OAM state.
 */
extern rtk_api_ret_t rtk_oam_state_get(rtk_enable_t *pEnabled);


/* Module Name : OAM */

/* Function Name:
 *      rtk_oam_parserAction_set
 * Description:
 *      Set OAM parser action
 * Input:
 *      port    - port id
 *      action  - parser action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
extern rtk_api_ret_t rtk_oam_parserAction_set(rtk_port_t port, rtk_oam_parser_act_t action);

/* Function Name:
 *      rtk_oam_parserAction_set
 * Description:
 *      Get OAM parser action
 * Input:
 *      port    - port id
 * Output:
 *      pAction  - parser action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
extern rtk_api_ret_t rtk_oam_parserAction_get(rtk_port_t port, rtk_oam_parser_act_t *pAction);


/* Function Name:
 *      rtk_oam_multiplexerAction_set
 * Description:
 *      Set OAM multiplexer action
 * Input:
 *      port    - port id
 *      action  - parser action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
extern rtk_api_ret_t rtk_oam_multiplexerAction_set(rtk_port_t port, rtk_oam_multiplexer_act_t action);

/* Function Name:
 *      rtk_oam_multiplexerAction_set
 * Description:
 *      Get OAM multiplexer action
 * Input:
 *      port    - port id
 * Output:
 *      pAction  - parser action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
extern rtk_api_ret_t rtk_oam_multiplexerAction_get(rtk_port_t port, rtk_oam_multiplexer_act_t *pAction);


#endif /* __RTK_OAM_H__ */

