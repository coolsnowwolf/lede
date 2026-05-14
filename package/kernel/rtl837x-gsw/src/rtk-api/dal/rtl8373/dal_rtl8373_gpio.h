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
 * Feature : The file includes GPIO module high-layer API defination
 *
 */

#ifndef  __DAL_RTL8373_GPIO_H__
#define  __DAL_RTL8373_GPIO_H__
#include <gpio.h>


/* Function Name:
 *      dal_rtl8373_gpio_muxSel_set
 * Description:
 *      enable gpio pin 
 * Input:
 *      gpioNum - GPIO pin number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
extern rtk_api_ret_t dal_rtl8373_gpio_muxSel_set(rtk_uint32 gpioNum);

/* Function Name:
 *      dal_rtl8373_gpio_muxSel_get
 * Description:
 *      Get gpio pin status 
 * Input:
 *      gpioNum - GPIO pin number
 * Output:
 *      *pStatus - status
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
extern rtk_api_ret_t dal_rtl8373_gpio_muxSel_get(rtk_uint32 gpioNum, rtk_enable_t *pStatus );

/* Function Name:
 *      dal_rtl8373_gpio_groupVal_write
 * Description:
 *      write 32bits gpio pin val
 * Input:
 *      idx - GPIO pin LSB 32bits or MSB 31bits
 *      val         - value
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
extern rtk_api_ret_t dal_rtl8373_gpio_groupVal_write(rtk_gpio_groupReg_t idx, rtk_uint32 val );

/* Function Name:
 *      dal_rtl8373_gpio_groupVal_read
 * Description:
 *      Read 32bits gpio pin val 
 * Input:
 *      idx - GPIO pin LSB 32bits or MSB 31bits
 * Output:
 *      *pVal - value
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
extern rtk_api_ret_t dal_rtl8373_gpio_groupVal_read(rtk_gpio_groupReg_t idx, rtk_uint32 *pVal);

/* Function Name:
 *      dal_rtl8373_gpio_pinVal_write
 * Description:
 *      write gpio pin val
 * Input:
 *      idx - GPIO pin LSB 32bits or MSB 31bits
 *      val         - value
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
extern rtk_api_ret_t dal_rtl8373_gpio_pinVal_write(rtk_uint32 gpioNum, rtk_gpio_level_t val);

/* Function Name:
 *      dal_rtl8373_gpio_pinVal_read
 * Description:
 *      Read gpio pin val 
 * Input:
 *      idx - GPIO pin LSB 32bits or MSB 31bits
 * Output:
 *      *pVal - value
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
extern rtk_api_ret_t dal_rtl8373_gpio_pinVal_read(rtk_uint32 gpioNum, rtk_gpio_level_t *pVal);

/* Function Name:
 *      dal_rtl8373_gpio_pinDir_set
 * Description:
 *      set gpio pin direction
 * Input:
 *      gpioNum - GPIO pin num
 *      dir  - GPIO pin direction
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
extern rtk_api_ret_t dal_rtl8373_gpio_pinDir_set(rtk_uint32 gpioNum, rtk_gpio_direction_t dir );

/* Function Name:
 *      dal_rtl8373_gpio_pinDir_get
 * Description:
 *      Get gpio pin direction
 * Input:
 *      gpioNum - GPIO pin num
 * Output:
 *      dir  - GPIO pin direction
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
extern rtk_api_ret_t dal_rtl8373_gpio_pinDir_get(rtk_uint32 gpioNum, rtk_gpio_direction_t *pDir);

/* Function Name:
 *      dal_rtl8373_gpio_groupDir_get
 * Description:
 *      Get gpio pin direction
 * Input:
 *      idx - GPIO pin LSB 32bits or MSB 31bits
 * Output:
 *      pDirVal  -  Group GPIO pin direction
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RANGE  - out of range
 * Note:
 *     
 */
extern rtk_api_ret_t dal_rtl8373_gpio_groupDir_get(rtk_gpio_groupReg_t idx, rtk_uint32 *pDirVal);


#endif /*  __DAL_RTL8373_GPIO_H__ */

