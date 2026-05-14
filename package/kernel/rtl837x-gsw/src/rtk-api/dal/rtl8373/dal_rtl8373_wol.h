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
 * Purpose : RTK switch high-level API for RTL8367/RTL8373
 * Feature : Here is a list of all functions and variables in TRUNK module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <rtl8373_asicdrv.h>
#include <string.h>



/* Function Name:
 *      dal_rtl8373_wolState_set
 * Description:
 *      Set wol function enable
 * Input:
 *      enable : 1 enable wol;   0: disable wol
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_wolState_set(rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_wolState_sget
 * Description:
 *      Set wol function enable
 * Input:
 *      pEnable : 1 enable wol;   0: disable wol
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_wolState_get(rtk_uint32* pEnable);


/* Function Name:
 *      dal_rtl8373_wolPortmsk_get
 * Description:
 *      Set wol port mask
 * Input:
 *      pMask: port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_wolPortmsk_get(rtk_uint32* pMask);




/* Function Name:
 *      dal_rtl8373_wolMac_set
 * Description:
 *      Set wol port mask
 * Input:
 *      pMac: wol mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_wolMac_set(rtk_mac_t *pMac);


/* Function Name:
 *      dal_rtl8373_wolMac_get
 * Description:
 *      Set wol port mask
 * Input:
 *      pMac: wol mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_wolMac_get(rtk_mac_t *pMac);




