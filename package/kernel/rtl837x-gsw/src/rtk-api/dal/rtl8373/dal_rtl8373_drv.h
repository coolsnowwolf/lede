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
 * Purpose : RTL8371c switch high-level API
 *
 * Feature : The file includes phy module high-layer API defination
 *
 */

 #ifndef __DAL_RTL8373_DRV_H__
#define __DAL_RTL8373_DRV_H__

#define RTL8373_MAX_POLLCNT  1000UL


/* Function Name:
 *      dal_rtl8373_mdc_en
 * Description:
 *      Enbale MDC function.
 * Input:
 *      enable  - enable 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will enbale MDC function.
 */
extern rtk_api_ret_t dal_rtl8373_mdc_en(rtk_uint32 enable);

 /* Function Name:
 *      dal_rtl8373_phy_write
 * Description:
 *      Configure phy register data.
 * Input:
 *      phy_mask  - phy mask, bit[0:9]
 *      dev_addr   - device address
 *      reg_addr   - register address
 *      indata       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */
extern rtk_api_ret_t dal_rtl8373_phy_write(rtk_uint32 phy_mask, rtk_uint32 dev_addr, rtk_uint32 reg_addr, rtk_uint32 indata);

/* Function Name:
 *      dal_rtl8373_phy_read
 * Description:
 *      get phy register data.
 * Input:
 *      phy_id  - phy id
 *      dev_addr   - device address
 *      reg_addr   - register address
 * Output:
*      pdata       - phy data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get phy register data.
 */
extern rtk_api_ret_t dal_rtl8373_phy_read(rtk_uint32 phy_id, rtk_uint32 dev_addr, rtk_uint32 reg_addr, rtk_uint32 *pdata);
/* Function Name:
 *      dal_rtl8373_phy_readBits
 * Description:
 *      Configure phy register data.
 * Input:
 *      phy_id - phy id
 *      dev_addr   - device address
 *      reg_addr   - register address
 *      indata       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */
 
extern rtk_api_ret_t dal_rtl8373_phy_readBits(rtk_uint32 phy_id, rtk_uint32 dev_addr, rtk_uint32 reg_addr,  rtk_uint32 bitsMask,rtk_uint32 *pdata);

/* Function Name:
 *      rtk_port_phyReg_setBits
 * Description:
 *      Configure phy register data.
 * Input:
 *      phy_mask  - phy mask, bit[0:9]
 *      dev_addr   - device address
 *      reg_addr   - register address
 *      indata       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */
 
extern rtk_api_ret_t dal_rtl8373_phy_writeBits(rtk_uint32 phy_mask, rtk_uint32 dev_addr, rtk_uint32 reg_addr,  rtk_uint32 bitsMask,rtk_uint32 indata);
/* Function Name:
 *      dal_rtl8224_top_reg_write
 * Description:
 *      Set RTL8224 top register value.
 * Input:
 *      top_reg_addr   - top register address
 *      value               - register value
 * Output:
*      pdata       - None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will set phy register data.
 */

extern rtk_api_ret_t  dal_rtl8224_top_reg_write(rtk_uint32 top_reg_addr,  rtk_uint32 value);

/* Function Name:
 *      dal_rtl8224_top_reg_read
 * Description:
 *      Get RTL8224 top register value.
 * Input:
 *      top_reg_addr   - top register address
 *      pvalue               - register value
 * Output:
*      pdata       - None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get phy register data.
 */

extern rtk_api_ret_t  dal_rtl8224_top_reg_read(rtk_uint32 top_reg_addr,  rtk_uint32* pvalue);


/* Function Name:
 *      dal_rtl8224_top_regbit_write
 * Description:
 *      Set RTL8224 top register bit value.
 * Input:
 *      top_reg_addr   - top register address
 *      offet                - bit offset
 *      value               - register bit value
 * Output:
*      pdata       - None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will set phy register data.
 */

extern rtk_api_ret_t  dal_rtl8224_top_regbit_write(rtk_uint32 top_reg_addr,  rtk_uint32 offset, rtk_uint32 value);


/* Function Name:
 *      dal_rtl8224_top_regbit_read
 * Description:
 *      Get RTL8224 top register bit value.
 * Input:
 *      top_reg_addr   - top register address
 *      offet                - bit offset
 *      value               - register bit value
 * Output:
*      pdata       - None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get phy register data.
 */

extern rtk_api_ret_t  dal_rtl8224_top_regbit_read(rtk_uint32 top_reg_addr,  rtk_uint32 offset, rtk_uint32* pvalue);


/* Function Name:
 *      dal_rtl8224_top_regbits_write
 * Description:
 *      Set RTL8224 top register bits value.
 * Input:
 *      top_reg_addr   - top register address
 *      bitmask          - bit mask
 *      value               - register bits value
 * Output:
*      pdata       - None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will set phy register data.
 */

extern rtk_api_ret_t  dal_rtl8224_top_regbits_write(rtk_uint32 top_reg_addr,  rtk_uint32 bitmask, rtk_uint32 value);



/* Function Name:
 *      dal_rtl8224_top_regbits_read
 * Description:
 *      Get RTL8224 top register bits value.
 * Input:
 *      top_reg_addr   - top register address
 *      bitmask          - bit mask
 *      value               - register bits value
 * Output:
*      pdata       - None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get phy register data.
 */

extern rtk_api_ret_t  dal_rtl8224_top_regbits_read(rtk_uint32 top_reg_addr,  rtk_uint32 bitmask, rtk_uint32* pvalue);

/* Function Name:
 *      dal_rtl8373_sds_reg_write
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      regdata       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

extern rtk_api_ret_t dal_rtl8373_sds_reg_write(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 regdata);


/* Function Name:
 *      dal_rtl8373_sds_reg_read
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      pdata       - output data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

extern rtk_api_ret_t dal_rtl8373_sds_reg_read(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 * pdata);



/* Function Name:
 *      dal_rtl8373_sds_regbits_write
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      bitmask   - bits mask
 *      value       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

extern rtk_api_ret_t dal_rtl8373_sds_regbits_write(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 bitmask, rtk_uint32 value);


/* Function Name:
 *      dal_rtl8373_sds_regbis_read
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      pvalue       - output data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

extern rtk_api_ret_t dal_rtl8373_sds_regbits_read(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 bitmask, rtk_uint32 * pvalue);




/* Function Name:
 *      dal_rtl8224_sdsreg_write
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      regdata       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

extern rtk_api_ret_t dal_rtl8224_sds_reg_write(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 regdata);


/* Function Name:
 *      dal_rtl8224_sdsreg_read
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      pdata       - output data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

extern rtk_api_ret_t dal_rtl8224_sds_reg_read(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 * pdata);


/* Function Name:
 *      dal_rtl8224_sds_regbits_write
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      bitmask   - bits mask
 *      value       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

extern rtk_api_ret_t dal_rtl8224_sds_regbits_write(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 bitmask, rtk_uint32 value);


/* Function Name:
 *      dal_rtl8224_sds_regbis_read
 * Description:
 *      Configure phy register data.
 * Input:
 *      sds_index  - sds index 0 ~ 1
 *      sds_page   - page
 *      sds_geg   - register address
 *      pvalue       - output data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

extern rtk_api_ret_t dal_rtl8224_sds_regbits_read(rtk_uint32 sds_index, rtk_uint32 sds_page, rtk_uint32 sds_reg, rtk_uint32 bitmask, rtk_uint32 * pvalue);

/* Function Name:
 *      dal_rtl8373_phy_regbits_write
 * Description:
 *      Configure phy register data.
 * Input:
 *      phy_mask  - phy mask, bit[0:9]
 *      dev_addr   - device address
 *      reg_addr   - register address
 *      bitmask    - bits mask
 *      indata       - input data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will configure phy register data.
 */

extern rtk_api_ret_t dal_rtl8373_phy_regbits_write(rtk_uint32 phy_mask, rtk_uint32 dev_addr, rtk_uint32 reg_addr, rtk_uint32 bitmask, rtk_uint32 value);

/* Function Name:
 *      dal_rtl8373_phy_regbits_read
 * Description:
 *      get phy register data.
 * Input:
 *      phy_id  - phy id
 *      dev_addr   - device address
 *      reg_addr   - register address
 *      bitmask    - bits mask
 * Output:
 *      pdata       - phy data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM    - Invalid queue number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API will get phy register data.
 */

extern rtk_api_ret_t dal_rtl8373_phy_regbits_read(rtk_uint32 phy_id, rtk_uint32 dev_addr, rtk_uint32 reg_addr, rtk_uint32 bitmask, rtk_uint32 * pvalue);


#endif

