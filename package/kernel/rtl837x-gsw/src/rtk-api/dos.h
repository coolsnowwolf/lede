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
 * Feature : The file includes Trunk module high-layer TRUNK defination
 *
 */

#ifndef __RTK_API_DOS_H__
#define __RTK_API_DOS_H__


typedef enum rtk_port_autoDosType_e
{
    AUTODOS_DAEQSA = 0,
    AUTODOS_LANDATTACKS_V4,
    AUTODOS_LANDATTACKS_V6,
    AUTODOS_BLATATTACKS,
    AUTODOS_SYNFINSCAN,
    AUTODOS_XMASCAN,
    AUTODOS_NULLSCAN,
    AUTODOS_SYN1024,
    AUTODOS_TCPSHORTHDR,
    AUTODOS_TCPFRAGERROR,
    AUTODOS_ICMPFRAGMENT,
    AUTODOS_END,

} rtk_port_autoDosType_t;

/* Function Name:
 *      rtk_port_autoDos_set
 * Description:
 *      Set Auto Dos state
 * Input:
 *      type        - Auto DoS type
 *      state       - 1: Eanble(Drop), 0: Disable(Forward)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 * Note:
 *      The API can set Auto Dos state
 */
extern rtk_api_ret_t rtk_port_autoDos_set(rtk_port_autoDosType_t type, rtk_enable_t state);

/* Function Name:
 *      rtk_port_autoDos_get
 * Description:
 *      Get Auto Dos state
 * Input:
 *      type        - Auto DoS type
 * Output:
 *      pState      - 1: Eanble(Drop), 0: Disable(Forward)
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_NULL_POINTER         - Null Pointer
 * Note:
 *      The API can get Auto Dos state
 */
extern rtk_api_ret_t rtk_port_autoDos_get(rtk_port_autoDosType_t type, rtk_enable_t *pState);


#endif /* __RTK_API_TRUNK_H__ */
