
/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Use to Management each device
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Initialize system
 *           2) Initialize device
 *           3) Mangement Devices
 *
 */
#ifndef __DAL_MGMT_H__
#define __DAL_MGMT_H__

/*
 * Include Files
 */
#include <rtk_types.h>
#include <rtk_error.h>
#include <dal/dal_mapper.h>

/*
 * Symbol Definition
 */
typedef struct dal_mgmt_info_s
{
    dal_mapper_t    *pMapper;
} dal_mgmt_info_t;

typedef struct dal_mapper_info_s
{
    switch_chip_t   switchChip;
    dal_mapper_t    *pMapper;
} dal_mapper_info_t;

/*
 * Data Declaration
 */
extern dal_mgmt_info_t      *pMgmt_node;

/*
 * Macro Definition
 */
#define RT_MGMT       pMgmt_node
#define RT_MAPPER     RT_MGMT->pMapper

/*
 * Function Declaration
 */

/* Module Name : */

/* Function Name:
 *      dal_mgmt_attachDevice
 * Description:
 *      Attach device(semaphore, database clear)
 * Input:
 *      switchChip - switch type
 * Output:
 *      None
 * Return:
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 * Note:
 *      RTK must call this function before do other kind of action.
 */
extern rtk_int32
dal_mgmt_attachDevice(switch_chip_t switchChip);

#endif /* __DAL_MGMT_H__ */


