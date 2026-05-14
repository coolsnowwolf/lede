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
 * Feature : The file includes MIB module high-layer API defination
 *
 */

#ifndef __RTK_API_RMA_H__
#define __RTK_API_RMA_H__


enum RTK_RMAOP
{
    RMAOP_FORWARD = 0,
    RMAOP_TRAP_TO_CPU,
    RMAOP_DROP,
    RMAOP_FORWARD_EXCLUDE_CPU,
    RMAOP_END
};





typedef struct  rtk_rmaParam_s{

    rtk_uint32 operation;
    rtk_uint32 discard_storm_filter;
    rtk_uint32 trap_priority;
    rtk_uint32 keep_format;
    rtk_uint32 vlan_leaky;
    rtk_uint32 portiso_leaky;

}rtk_rmaParam_t;


/* Function Name:
 *      rtk_rma_set
 * Description:
 *      Set RMA action, priority & leaky function.
 * Input:
 *      rmaAddr: 0x00 ~ 0x2f
 *      rmaParam:
 *            operation;
 *            discard_storm_filter;
 *            trap_priority;
 *            keep_format;
 *            vlan_leaky;
 *            portiso_leaky;
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Reset MIB counter of ports. API will use global reset while port mask is all-ports.
 */
extern rtk_api_ret_t rtk_rma_set(rtk_uint32 rmaAddr, rtk_rmaParam_t* rmaParam);


/* Function Name:
 *      rtk_rma_get
 * Description:
 *      Get RMA action, priority & leaky function.
 * Input:
 *      rmaAddr: 0x00 ~ 0x2f
 *      rmaParam:
 *            operation;
 *            discard_storm_filter;
 *            trap_priority;
 *            keep_format;
 *            vlan_leaky;
 *            portiso_leaky;
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Reset MIB counter of ports. API will use global reset while port mask is all-ports.
 */
extern rtk_api_ret_t rtk_rma_get(rtk_uint32 rmaAddr, rtk_rmaParam_t* rmaParam);




#endif /* __RTK_API_RMA_H__ */

