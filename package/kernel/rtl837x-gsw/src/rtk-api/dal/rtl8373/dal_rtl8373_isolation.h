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
 * Feature : The file includes ISOLATION module high-layer API defination
 *
 */

 
/* Function Name:
 *      dal_rtl8373_port_isolation_set
 * Description:
 *      Set permitted port isolation portmask
 * Input:
 *      port            - Physical port number (0~9)
 *      permitPortmask  - port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_PORT_MASK    - Invalid portmask
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_port_isolation_set(rtk_port_t port, rtk_uint32 permitPortmask);

/* Function Name:
 *      dal_rtl8373_port_isolation_get
 * Description:
 *      Get permitted port isolation portmask
 * Input:
 *      port                - Physical port number (0~9)
 *      pPermitPortmask     - port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_port_isolation_get(rtk_port_t port, rtk_uint32 *pPermitPortmask);


