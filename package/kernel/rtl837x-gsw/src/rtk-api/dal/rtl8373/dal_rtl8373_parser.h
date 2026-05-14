#ifndef __DAL_RTL8373_PARSER_H__
#define __DAL_RTL8373_PARSER_H__


/* Function Name:
 *      dal_rtl8373_ignrOUI_set
 * Description:
 *      Get ignore OUI
 * Input:
 *      None
 * Output:
 *      pEnabled     - ignore OUI enable/disable
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ignrOUI_set(rtk_uint32 enabled);



/* Function Name:
 *      dal_rtl8373_ignrOUI_get
 * Description:
 *      Get ignore OUI
 * Input:
 *      None
 * Output:
 *      pEnabled     - ignore OUI enable/disable
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_ignrOUI_get(rtk_uint32 *pEnabled);



/* Function Name:
 *      dal_rtl8373_rxReason_get
 * Description:
 *      Get ignore OUI
 * Input:
 *      None
 * Output:
 *      pReason     - rx drop reason
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_rxReason_get(rtk_uint32 port, rtk_uint32 *pReason);



/* Function Name:
 *      dal_rtl8373_fieldSelector_set
 * Description:
 *      Get field selector setting
 * Input:
 *      None
 * Output:
 *      index    - 0~15
 *      format   0 ~ 7
 *      offset    0 ~ 172
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_fieldSelector_set(rtk_uint32 index, rtk_uint32 format, rtk_uint32 offset);


/* Function Name:
 *      dal_rtl8373_fieldSelector_get
 * Description:
 *      Get field selector setting
 * Input:
 *      None
 * Output:
 *      index    - 0~15
 *      pFormat   0 ~ 7
 *      pOffset    0 ~ 172
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_fieldSelector_get(rtk_uint32 index, rtk_uint32 *pFormat, rtk_uint32 *pOffset);



#endif

