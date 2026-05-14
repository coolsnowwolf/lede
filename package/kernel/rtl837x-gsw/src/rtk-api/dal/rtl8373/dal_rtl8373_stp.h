#ifndef __DAL_RTL8373_STP_H__
#define __DAL_RTL8373_STP_H__

#include <rtk_switch.h>
#include <rtk_error.h>
#include <rtl8373_asicdrv.h>
#include <string.h>

enum RTL8373_MSTP_STATE
{
    MSTP_DISABLE = 0,
    MSTP_BLOCKING,
    MSTP_LEARNING,
    MSTP_FORWARDING,
    MSTP_END
};

/* Function Name:
 *      dal_rtl8373_asicMstpPortStatus_set
 * Description:
 *      Set MSTP port status
 * Input:
 *      fid     - mstp index
 *      port     - 
 *      statys  - 0 disable  1 blocking     2 learning    3 forwarding
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicMstpPortStatus_set(rtk_uint32 fid, rtk_uint32 port, rtk_uint32 status);



/* Function Name:
 *      dal_rtl8373_asicMstpPortStatus_get
 * Description:
 *      Get MSTP port status
 * Input:
 *      fid     - mstp index
 *      port     - 
 *      *pStatys  - 0 disable  1 blocking     2 learning    3 forwarding
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicMstpPortStatus_get(rtk_uint32 fid, rtk_uint32 port, rtk_uint32* pStatus);



#endif

