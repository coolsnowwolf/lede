/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 */

/*
 * Include Files
 */
#if (!defined(CONFIG_DAL_RTL8373) && !defined(CONFIG_DAL_RTL8370UG))
#define CONFIG_DAL_ALL
#endif

#include <dal/dal_mgmt.h>
#include <dal/dal_mapper.h>
#if defined(CONFIG_DAL_RTL8373) || defined(CONFIG_DAL_ALL)
#include <dal/rtl8373/dal_rtl8373_mapper.h>
#endif
//#if defined(CONFIG_DAL_RTL8370UG) || defined(CONFIG_DAL_ALL)
//#include <dal/rtl8370ug/dal_rtl8370ug_mapper.h>
//#endif

dal_mgmt_info_t         Mgmt_node;
dal_mgmt_info_t         *pMgmt_node = &Mgmt_node;

static dal_mapper_info_t dal_mapper_database[] =
{
    {CHIP_RTL8373,     NULL},
    {CHIP_RTL8372,     NULL},
};

rtk_int32 dal_mgmt_attachDevice(switch_chip_t switchChip)
{
    rtk_uint32  mapper_size=sizeof(dal_mapper_database)/sizeof(dal_mapper_info_t);
    rtk_uint32  mapper_index;

    /*mapper init*/
    for (mapper_index = 0; mapper_index < mapper_size; mapper_index++)
    {
        if (switchChip == dal_mapper_database[mapper_index].switchChip)
        {
#if defined(CONFIG_DAL_RTL8373) || defined(CONFIG_DAL_ALL)
            if (switchChip == CHIP_RTL8373)
            {
                dal_mapper_database[mapper_index].pMapper = dal_rtl8373_mapper_get();
                pMgmt_node->pMapper = dal_mapper_database[mapper_index].pMapper;
                return RT_ERR_OK;
            }
#endif
#if 0
#if defined(CONFIG_DAL_RTL8370UG) || defined(CONFIG_DAL_ALL)
            if (switchChip == CHIP_RTL8370UG)
            {
                dal_mapper_database[mapper_index].pMapper = dal_rtl8370ug_mapper_get();
                pMgmt_node->pMapper = dal_mapper_database[mapper_index].pMapper;
                return RT_ERR_OK;
            }
#endif
#endif
        }
    }

    return RT_ERR_CHIP_NOT_SUPPORTED;
}
