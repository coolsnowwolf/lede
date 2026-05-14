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
 * Feature : The file includes Interrupt module high-layer API defination
 *
 */

#ifndef __RTK_API_INTERRUPT_H__
#define __RTK_API_INTERRUPT_H__


/*
 * Data Type Declaration
 */


typedef enum rtk_int_type_e
{
    INT_TYPE_LINK_CHANGE = 0,
    INT_TYPE_GPHY,
    INT_TYPE_LEARN_OVER,
    INT_TYPE_RLFD,
    INT_TYPE_WOL,
    INT_TYPE_SDS_LINK_FAULT,
    INT_TYPE_SDS_UPDATE_PHY,
    INT_TYPE_END
}rtk_int_type_t;

typedef enum rtk_int_cpu_e
{
    INT_CPU_INTERNAL = 0,
    INT_CPU_EXTERNAL,
    INT_CPU_END
}rtk_int_cpu_t;



typedef enum rtk_int_polarity_e
{
    INT_POLAR_HIGH = 0,
    INT_POLAR_LOW,
    INT_POSITIVE_PULSE,
    INT_NEGATIVE_PULSE,
    INT_POLAR_END
} rtk_int_polarity_t;

typedef enum interrupt_misc_e
{
    TM_HIGH=0,
    TM_LOW,
    SMI_CHECK_REG_0,
    SMI_CHECK_REG_1,
    SMI_CHECK_REG_2,
    SMI_CHECK_REG_3,
    SMI_CHECK_REG_4,
    SDS_RX_SYM_ERR_0,
    SDS_RX_SYM_ERR_1,
    SAMOVE = 10,
    AUTO_REC,
    ACL = 13,
    INCPU,
    LOOP_DETEC,
    METER_EXCEED,
    ROUT_PBUF,
    PTP1588,
    INTERRUPT_MISC_END
}interrupt_misc_t;

typedef enum interrupt_glb_e
{
    THERMAL_DET=0,
    SMI_CHK=2,
    SDS_RX_ERR=4,
    GPIO,
    GLB_SAMOVE=7,
    GLB_AUTO_REC,
    GLBACL=10,
    GLB_LOOP_DETEC=12,
    GLB_METER_EXCEED,
    PTP,
    GLB_ROUT_PBUF=24,
    SDS_UPD_PHY,
    SDS_LNK_FLT,
    WOL,
    RLFD,
    GPHY,
    LRN_OVER,
    LINK_CHG,
    INTERRUPT_GLB_END
}interrupt_glb_t;
/* Function Name:
 *      rtk_int_enable
 * Description:
 *      Enable interrupt function.
 * Input:
 *      enable - 0 disable, 1 enable.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can set interrupt polarity configuration.
 */
extern rtk_api_ret_t rtk_int_enable(rtk_enable_t enable);



/* Function Name:
 *      rtk_int_polarity_set
 * Description:
 *      Set interrupt polarity configuration.
 * Input:
 *      type - Interruptpolarity type.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can set interrupt polarity configuration.
 */
extern rtk_api_ret_t rtk_int_polarity_set(rtk_int_polarity_t type);

/* Function Name:
 *      rtk_int_polarity_get
 * Description:
 *      Get interrupt polarity configuration.
 * Input:
 *      None
 * Output:
 *      pType - Interruptpolarity type.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API can get interrupt polarity configuration.
 */
extern rtk_api_ret_t rtk_int_polarity_get(rtk_int_polarity_t *pType);

/* Function Name:
 *      rtk_portInt_control_set
 * Description:
 *      Set interrupt trigger status configuration.
 * Input:
 *      port - port id
 *      intcpu - 0 internal cpu interrupt   1 external cpu interrupt
 *      type - Interrupt type.
 *      enable - Interrupt status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      The API can set interrupt status configuration.
 *      The interrupt trigger status is shown in the following:
 *      - INT_TYPE_LINK_CHANGE,
 *      - INT_TYPE_GPHY,
 *      - INT_TYPE_LEARN_OVER,
 *      - INT_TYPE_RLFD,
 *      - INT_TYPE_WOL,
 *      - INT_TYPE_SDS_LINK_FAULT,
 *      - INT_TYPE_SDS_UPDATE_PHY,
 */
extern rtk_api_ret_t rtk_portInt_control_set(rtk_port_t port, rtk_int_cpu_t intcpu, rtk_int_type_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_portInt_control_get
 * Description:
 *      Get interrupt trigger status configuration.
 * Input:
 *      port - port id
 *      intcpu - 0 internal cpu interrupt   1 external cpu interrupt
 *      type - Interrupt type.
 *      enable - Interrupt status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      The API can set interrupt status configuration.
 *      The interrupt trigger status is shown in the following:
 *      - INT_TYPE_LINK_CHANGE,
 *      - INT_TYPE_GPHY,
 *      - INT_TYPE_LEARN_OVER,
 *      - INT_TYPE_RLFD,
 *      - INT_TYPE_WOL,
 *      - INT_TYPE_SDS_LINK_FAULT,
 *      - INT_TYPE_SDS_UPDATE_PHY,
 */

extern rtk_api_ret_t rtk_portInt_control_get(rtk_port_t port, rtk_int_cpu_t intcpu, rtk_int_type_t type, rtk_enable_t *pEnable);
/* Function Name:
 *      rtk_int_miscIMR_set
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
extern rtk_api_ret_t rtk_int_miscIMR_set(rtk_uint32 type, interrupt_misc_t interrupt, rtk_uint32 enable);



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

extern rtk_api_ret_t rtk_int_miscIMR_get(rtk_uint32 type, interrupt_misc_t interrupt, rtk_uint32* pEnable);


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

extern rtk_api_ret_t rtk_int_miscISR_clear(rtk_uint32 type, interrupt_misc_t interrupt);





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

extern rtk_api_ret_t rtk_int_miscISR_get(rtk_uint32 type, interrupt_misc_t interrupt, rtk_uint32* pStatus);

#endif /* __RTK_API_INTERRUPT_H*/

