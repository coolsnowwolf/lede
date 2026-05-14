#ifndef __DAL_RTL8373_DOS_H__
#define __DAL_RTL8373_DOS_H__

#include "dos.h"


/* Function Name:
 *      rtl8373_setAsicDos
 * Description:
 *      Set asic dos configuration
 * Input:
 *      index     - dos type
 *      enable     - 1:enable, 0:disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicDos_set(rtk_port_autoDosType_t index, rtk_uint32 enable);



/* Function Name:
 *      rtl8373_getAsicDos
 * Description:
 *      Get asic dos configuration
 * Input:
 *      index     - dos type
 *      pEnable     - 1:enable, 0:disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicDos_get(rtk_port_autoDosType_t index, rtk_uint32* pEnable);

#endif

