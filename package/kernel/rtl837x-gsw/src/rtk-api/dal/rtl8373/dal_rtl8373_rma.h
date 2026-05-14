#ifndef __DAL_RTL8373_RMA_H__
#define __DAL_RTL8373_RMA_H__

#include <rma.h>


#define RTL8373_RMAMAX                     0x2F




/* Function Name:
 *      dal_rtl8373_asicRma_set
 * Description:
 *      Set reserved multicast address for CPU trapping
 * Input:
 *      index     - reserved multicast LSB byte, 0x00~0x2F is available value
 *      pRmacfg     - type of RMA for trapping frame type setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicRma_set(rtk_uint32 index, rtk_rmaParam_t* pRmacfg);



/* Function Name:
 *      dal_rtl8373_asicRma_get
 * Description:
 *      Get reserved multicast address for CPU trapping
 * Input:
 *      index     - reserved multicast LSB byte, 0x00~0x2F is available value
 *      rmacfg     - type of RMA for trapping frame type setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicRma_get(rtk_uint32 index, rtk_rmaParam_t* pRmacfg);



/* Function Name:
 *      dal_rtl8373_asicRmaCdp_set
 * Description:
 *      Set CDP(Cisco Discovery Protocol) for CPU trapping
 * Input:
 *      pRmacfg     - type of RMA for trapping frame type setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicRmaCdp_set(rtk_rmaParam_t* pRmacfg);


/* Function Name:
 *      dal_rtl8373_asicRmaCdp_get
 * Description:
 *      Get CDP(Cisco Discovery Protocol) for CPU trapping
 * Input:
 *      None
 * Output:
 *      pRmacfg     - type of RMA for trapping frame type setting
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicRmaCdp_get(rtk_rmaParam_t* pRmacfg);



/* Function Name:
 *      dal_rtl8373_asicRmaCsstp_set
 * Description:
 *      Set CSSTP(Cisco Shared Spanning Tree Protocol) for CPU trapping
 * Input:
 *      pRmacfg     - type of RMA for trapping frame type setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicRmaCsstp_set(rtk_rmaParam_t* pRmacfg);



/* Function Name:
 *      dal_rtl8373_asicRmaCsstp_get
 * Description:
 *      Get CSSTP(Cisco Shared Spanning Tree Protocol) for CPU trapping
 * Input:
 *      None
 * Output:
 *      pRmacfg     - type of RMA for trapping frame type setting
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicRmaCsstp_get(rtk_rmaParam_t* pRmacfg);




/* Function Name:
 *      dal_rtl8373_asicRmaLldp_set
 * Description:
 *      Set LLDP for CPU trapping
 * Input:
 *      pRmacfg     - type of RMA for trapping frame type setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicRmaLldp_set(rtk_uint32 enabled, rtk_rmaParam_t* pRmacfg);



/* Function Name:
 *      dal_rtl8373_asicRmaLldp_get
 * Description:
 *      Get LLDP for CPU trapping
 * Input:
 *      None
 * Output:
 *      pRmacfg     - type of RMA for trapping frame type setting
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicRmaLldp_get(rtk_uint32 *pEnabled, rtk_rmaParam_t* pRmacfg);



/* Function Name:
 *      dal_rtl8373_asicRmaTrapPri_set
 * Description:
 *      Set RMA function trap priority
 * Input:
 *      pri     - trap priority for all RMA
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */

extern ret_t dal_rtl8373_asicRmaTrapPri_set(rtk_uint32 pri);


/* Function Name:
 *      dal_rtl8373_asicRmaTrapPri_get
 * Description:
 *      Set RMA function trap priority
 * Input:
 *      pri     - trap priority for all RMA
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */

extern ret_t dal_rtl8373_asicRmaTrapPri_get(rtk_uint32 * pri);



#endif

