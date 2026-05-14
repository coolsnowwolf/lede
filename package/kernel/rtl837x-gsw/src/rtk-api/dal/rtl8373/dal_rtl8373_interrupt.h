#ifndef __DAL_RTL8373_INTERRUPT_H__
#define __DAL_RTL8373_INTERRUPT_H__

#include <interrupt.h>








/* Function Name:
 *      dal_rtl8373_intMode_set
 * Description:
 *      Set interrupt mode
 * Input:
 *      mode : 0 high;   1: low;  2 positive pulse;  3 negative pulse
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_intMode_set(rtk_int_polarity_t mode);



/* Function Name:
 *      dal_rtl8373_intMode_get
 * Description:
 *      Set interrupt mode
 * Input:
 *      pMode : 0 high;   1: low;  2 positive pulse;  3 negative pulse
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_intMode_get(rtk_int_polarity_t* pMode);



/* Function Name:
 *      dal_rtl8373_portLinkChgIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_portLinkChgIMR_set(rtk_uint32 type, rtk_uint32 port, rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_portLinkChgIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portLinkChgIMR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pEnable);


/* Function Name:
 *      dal_rtl8373_gphyIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      gphy: gphy id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_gphyIMR_set(rtk_uint32 type, rtk_uint32 phy, rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_gphyIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_gphyIMR_get(rtk_uint32 type, rtk_uint32 phy, rtk_uint32* pEnable);



/* Function Name:
 *      dal_rtl8373_portLrnOverIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_portLrnOverIMR_set(rtk_uint32 type, rtk_uint32 port, rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_portLrnOverIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portLrnOverIMR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pEnable);


/* Function Name:
 *      dal_rtl8373_portRLFDIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_portRLFDIMR_set(rtk_uint32 type, rtk_uint32 port, rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_portRLFDIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portRLFDIMR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pEnable);


/* Function Name:
 *      dal_rtl8373_portWolIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_portWolIMR_set(rtk_uint32 type, rtk_uint32 port, rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_portWolIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portWolIMR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pEnable);


/* Function Name:
 *      dal_rtl8373_portSdsLnkFltIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_portSdsLnkFltIMR_set(rtk_uint32 type, rtk_uint32 sds, rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_portSdsLnkFltIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portSdsLnkFltIMR_get(rtk_uint32 type, rtk_uint32 sds, rtk_uint32* pEnable);


/* Function Name:
 *      dal_rtl8373_portSdsUpdPhyIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_portSdsUpdPhyIMR_set(rtk_uint32 type, rtk_uint32 sds, rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_portSdsUpdPhyIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portSdsUpdPhyIMR_get(rtk_uint32 type, rtk_uint32 sds, rtk_uint32* pEnable);


/* Function Name:
 *      dal_rtl8373_gpioIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_gpioIMR_set(rtk_uint32 type, rtk_uint32 gpio, rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_gpioIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_gpioIMR_get(rtk_uint32 type, rtk_uint32 gpio, rtk_uint32* pEnable);


/* Function Name:
 *      dal_rtl8373_miscIMR_set
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_miscIMR_set(rtk_uint32 type, interrupt_misc_t interrupt, rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_miscIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_miscIMR_get(rtk_uint32 type, interrupt_misc_t interrupt, rtk_uint32* pEnable);




/* Function Name:
 *      dal_rtl8373_portLinkChgISR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     pStatus: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portLinkChgISR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pStatus);



/* Function Name:
 *      dal_rtl8373_gphyIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_gphyISR_get(rtk_uint32 type, rtk_uint32 phy, rtk_uint32* pStatus);




/* Function Name:
 *      dal_rtl8373_portLrnOverIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portLrnOverISR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pStatus);



/* Function Name:
 *      dal_rtl8373_portRLFDIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portRLFDISR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pStatus);



/* Function Name:
 *      dal_rtl8373_portWolIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portWolISR_get(rtk_uint32 type, rtk_uint32 port, rtk_uint32* pStatus);



/* Function Name:
 *      dal_rtl8373_portSdsLnkFltIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portSdsLnkFltISR_get(rtk_uint32 type, rtk_uint32 sds, rtk_uint32* pStatus);



/* Function Name:
 *      dal_rtl8373_portSdsUpdPhyIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portSdsUpdPhyISR_get(rtk_uint32 type, rtk_uint32 sds, rtk_uint32* pStatus);



/* Function Name:
 *      dal_rtl8373_gpioIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_gpioISR_get(rtk_uint32 type, rtk_uint32 gpio, rtk_uint32* pStatus);



/* Function Name:
 *      dal_rtl8373_miscIMR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_miscISR_get(rtk_uint32 type, interrupt_misc_t interrupt, rtk_uint32* pStatus);



/* Function Name:
 *      dal_rtl8373_glbISR_get
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_glbISR_get(rtk_uint32 type, interrupt_glb_t interrupt, rtk_uint32* pStatus);



/* Function Name:
 *      dal_rtl8373_IE_set
 * Description:
 *      Set switch interrupt enable
 * Input:
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_IE_set(rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_IE_get
 * Description:
 *      Set switch interrupt enable
 * Input:
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_IE_get(rtk_uint32* pEnable);




 /* Function Name:
  *      dal_rtl8373_portLinkChgISR_clear
  * Description:
  *      Set link change interrupt IMR
  * Input:
  *      type : 0 internal interrupt;   1: external interrupt
  *      port: port id
  *     pStatus: 0:disable,  1: enable
  * Output:
  *      None
  * Return:
  *      RT_ERR_OK           - OK
  *      RT_ERR_FAILED       - Failed
  *      RT_ERR_SMI          - SMI access error
  * Note:
   *      The API can set wol enable 
  */
 
 extern rtk_api_ret_t dal_rtl8373_portLinkChgISR_clear(rtk_uint32 type, rtk_uint32 port);
 
 
 
 /* Function Name:
  *      dal_rtl8373_gphyISR_clear
  * Description:
  *      Set link change interrupt IMR
  * Input:
  *      type : 0 internal interrupt;   1: external interrupt
  *      port: port id
  *     enable: 0:disable,  1: enable
  * Output:
  *      None
  * Return:
  *      RT_ERR_OK           - OK
  *      RT_ERR_FAILED       - Failed
  *      RT_ERR_SMI          - SMI access error
  * Note:
   *      The API can set wol enable 
  */
 
 extern rtk_api_ret_t dal_rtl8373_gphyISR_clear(rtk_uint32 type, rtk_uint32 phy);
 
 
 
 
 /* Function Name:
  *      dal_rtl8373_portLrnOverISR_clear
  * Description:
  *      Set link change interrupt IMR
  * Input:
  *      type : 0 internal interrupt;   1: external interrupt
  *      port: port id
  *     enable: 0:disable,  1: enable
  * Output:
  *      None
  * Return:

 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portLrnOverISR_clear(rtk_uint32 type, rtk_uint32 port);



/* Function Name:
 *      dal_rtl8373_portRLFDISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portRLFDISR_clear(rtk_uint32 type, rtk_uint32 port);



/* Function Name:
 *      dal_rtl8373_portRLFDISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portWolISR_clear(rtk_uint32 type, rtk_uint32 port);



/* Function Name:
 *      dal_rtl8373_portSdsLnkFltISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portSdsLnkFltISR_clear(rtk_uint32 type, rtk_uint32 sds);



/* Function Name:
 *      dal_rtl8373_portSdsUpdPhyISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_portSdsUpdPhyISR_clear(rtk_uint32 type, rtk_uint32 sds);



/* Function Name:
 *      dal_rtl8373_gpioISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_gpioISR_clear(rtk_uint32 type, rtk_uint32 gpio);



/* Function Name:
 *      dal_rtl8373_miscISR_clear
 * Description:
 *      Set link change interrupt IMR
 * Input:
 *      type : 0 internal interrupt;   1: external interrupt
 *      port: port id
 *     enable: 0:disable,  1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */

extern rtk_api_ret_t dal_rtl8373_miscISR_clear(rtk_uint32 type, interrupt_misc_t interrupt);

/* Function Name:
 *      dal_rtl8373_portIntIMR_set
 * Description:
 *      Set per port interrupt IMR
 * Input:
 *      port: port id/ gphy id/ serdes id
 *      inttype: 0 internal interrupt; 1 external interupt
 *      int: per port interrupt
 *      enable: 0 disable IMR; 1 enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_portIntIMR_set(rtk_port_t port, rtk_int_cpu_t inttype, rtk_int_type_t intnum, rtk_enable_t enable);




/* Function Name:
 *      dal_rtl8373_portIntIMR_get
 * Description:
 *      Get per port interrupt IMR
 * Input:
 *      port: port id/ gphy id/ serdes id
 *      inttype: 0 internal interrupt; 1 external interupt
 *      int: per port interrupt
 *      enable: 0 disable IMR; 1 enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
  *      The API can set wol enable 
 */
extern rtk_api_ret_t dal_rtl8373_portIntIMR_get(rtk_port_t port, rtk_int_cpu_t inttype, rtk_int_type_t intnum, rtk_enable_t* pEnable);




#endif

