#ifndef __DAL_RTL8373_MIB_H__
#define __DAL_RTL8373_MIB_H__

#include <mib.h>




#define RTL8373_PORT_MIB_NUMBER    104


/* Function Name:
 *      dal_rtl8373_globalMib_rst
 * Description:
 *      global mib reset
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_globalMib_rst(void);


/* Function Name:
 *      dal_rtl8373_portMib_rst
 * Description:
 *      port mib reset
 * Input:
 *      port: the port to reset mib
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_portMib_rst(rtk_uint32 port);


/* Function Name:
 *      dal_rtl8373_dbgMib_rst
 * Description:
 *      debug mib reset
 * Input:
 *      port: the port to reset mib
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_dbgMib_rst(rtk_uint32 port);




/* Function Name:
 *      dal_rtl8373_dbgMib_rst
 * Description:
 *      debug mib reset
 * Input:
 *      port: the port to reset mib
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_portMib_read(rtk_uint32 portid, rtk_stat_port_type_t mibid, rtk_uint64 * pMibCounter);


/* Function Name:
 *      dal_rtl8373_mibLength_set
 * Description:
 *      Set mib counter length include tag or not
 * Input:
 *      txMode: tx include tag or not; rxMode: rx include tag or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_mibLength_set(rtk_stat_lengthMode_t txMode, rtk_stat_lengthMode_t rxMode);


/* Function Name:
 *      dal_rtl8373_mibLength_get
 * Description:
 *      Get mib counter length include tag or not
 * Input:
 *      txMode: tx include tag or not; rxMode: rx include tag or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_mibLength_get(rtk_stat_lengthMode_t* pTxMode, rtk_stat_lengthMode_t* pRxMode);




#endif

