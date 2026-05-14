/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision$
 * $Date$
 *
 * Purpose : Definition function prototype of RTK switch API.
 *
 * Feature : Function prototype definition
 *
 */

#ifndef __CHIP_H__
#define __CHIP_H__


#include <rtk_types.h>
#include "phydef.h"

#define UNDEFINE_PHY_PORT   (0xFF)
#define RTK_SWITCH_PORT_NUM (10)
#define  RTK_MAX_PHYNUM 1
typedef enum switch_chip_e
{
    CHIP_RTL8373 = 0,
    CHIP_RTL8224 = 1,
	CHIP_RTL8372,
	CHIP_RTL8373N,
	CHIP_RTL8221B,
	CHIP_RTL8366U,
	CHIP_RTL8372N,
	CHIP_RTL8224N,
    CHIP_END
}switch_chip_t;

typedef enum port_type_e
{
    UTP_PORT = 0,
    EXT_PORT,
    UNKNOWN_PORT = 0xFF,
    PORT_TYPE_END
}port_type_t;

typedef struct rtk_switch_halCtrl_s
{
    switch_chip_t   switch_type;
    rtk_uint32      l2p_port[RTK_SWITCH_PORT_NUM];
    rtk_uint32      p2l_port[RTK_SWITCH_PORT_NUM];
    port_type_t     log_port_type[RTK_SWITCH_PORT_NUM];
    rtk_uint32      ptp_port[RTK_SWITCH_PORT_NUM];
    rtk_uint32      valid_portmask;
    rtk_uint32      valid_utp_portmask;
    rtk_uint32      valid_ext_portmask;
    rtk_uint32      valid_cpu_portmask;
    rtk_uint32      min_phy_port;
    rtk_uint32      max_phy_port;
    rtk_uint32      phy_portmask;
    rtk_uint32      combo_logical_port;
    rtk_uint32      hsg_logical_portmask;
    rtk_uint32      sg_logical_portmask;
    rtk_uint32      max_meter_id;
    rtk_uint32      max_lut_addr_num;
    rtk_uint32      trunk_group_mask;
    rtk_uint32      packet_buffer_page_num;
    rt_phyctrl_t    *pPhy_ctrl;

}rtk_switch_halCtrl_t;



typedef enum rt_port_ethType_id_e
{
    HWP_FE_ID = 0,
    HWP_GE_ID,
    HWP_2_5GE_ID,
    HWP_XGE_ID,
    HWP_SXGE_ID,
    HWP_ETHTYPE_ID_END,
}rt_port_ethType_id_t;
/* port ethernet type */
typedef enum rt_port_ethType_e
{
    HWP_FE             = (0x1 << HWP_FE_ID),          /* Fast ethernet port */
    HWP_GE             = (0x1 << HWP_GE_ID),          /* Giga ethernet port */
    HWP_2_5GE          = (0x1 << HWP_2_5GE_ID),       /* 2.5 Giga ethernet port */
    HWP_XGE            = (0x1 << HWP_XGE_ID),         /* 10 Giga ethernet port */
    HWP_SXGE           = (0x1 << HWP_SXGE_ID),        /* Cascade Stack port */
    HWP_ETHTYPE_END    = (0x1 << HWP_ETHTYPE_ID_END), /* Ether type end*/
}rt_port_ethType_t;
/* Function Name:
 *      switch_probe
 * Description:
 *      Probe switch
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Switch probed
 *      RT_ERR_FAILED   - Switch Unprobed.
 * Note:
 *
 */
extern rtk_api_ret_t switch_probe(switch_chip_t *pSwitchChip);

/* Function Name:
 *      hal_find_device
 * Description:
 *      Find the mac chip from SDK supported mac device lists.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      NULL        - Not found
 *      Otherwise   - Pointer of mac chip HAL structure that found
 * Note:
 *
 */
extern rtk_switch_halCtrl_t *hal_find_device(void);

#endif /* End of __CHIP_H__ */
