/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 76336 $
 * $Date: 2017-03-09 10:41:21 +0800 (週四, 09 三月 2017) $
 *
 * Purpose : RTK switch high-level API
 * Feature : Here is a list of all functions and variables in this module.
 *
 */

#ifndef __RTK_SWITCH_H__
#define __RTK_SWITCH_H__

#include <rtk_types.h>

#define UNDEFINE_PHY_PORT   (0xFF)
#define RTK_SWITCH_PORT_NUM (32)

#define MAXPKTLEN_CFG_ID_MAX (1)

#define RTK_SWITCH_MAX_PKTLEN (0x3FFF)

typedef enum init_state_e
{
    INIT_NOT_COMPLETED = 0,
    INIT_COMPLETED,
    INIT_STATE_END
} init_state_t;

typedef enum switch_chip_e
{
    CHIP_RTL8367C = 0,
    CHIP_RTL8370B,
    CHIP_RTL8364B,
    CHIP_RTL8363SC_VB,
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
    rtk_uint32      hsg_logical_port;
    rtk_uint32      sg_logical_portmask;
    rtk_uint32      max_meter_id;
    rtk_uint32      max_lut_addr_num;
    rtk_uint32      trunk_group_mask;

}rtk_switch_halCtrl_t;

typedef enum rtk_switch_maxPktLen_linkSpeed_e {
     MAXPKTLEN_LINK_SPEED_FE = 0,
     MAXPKTLEN_LINK_SPEED_GE,
     MAXPKTLEN_LINK_SPEED_END,
} rtk_switch_maxPktLen_linkSpeed_t;


/* UTIL MACRO */
#define RTK_CHK_INIT_STATE()                                \
    do                                                      \
    {                                                       \
        if(rtk_switch_initialState_get() != INIT_COMPLETED) \
        {                                                   \
            return RT_ERR_NOT_INIT;                         \
        }                                                   \
    }while(0)

#define RTK_CHK_PORT_VALID(__port__)                            \
    do                                                          \
    {                                                           \
        if(rtk_switch_logicalPortCheck(__port__) != RT_ERR_OK)  \
        {                                                       \
            return RT_ERR_PORT_ID;                              \
        }                                                       \
    }while(0)

#define RTK_CHK_PORT_IS_UTP(__port__)                           \
    do                                                          \
    {                                                           \
        if(rtk_switch_isUtpPort(__port__) != RT_ERR_OK)         \
        {                                                       \
            return RT_ERR_PORT_ID;                              \
        }                                                       \
    }while(0)

#define RTK_CHK_PORT_IS_EXT(__port__)                           \
    do                                                          \
    {                                                           \
        if(rtk_switch_isExtPort(__port__) != RT_ERR_OK)         \
        {                                                       \
            return RT_ERR_PORT_ID;                              \
        }                                                       \
    }while(0)

#define RTK_CHK_PORT_IS_COMBO(__port__)                         \
    do                                                          \
    {                                                           \
        if(rtk_switch_isComboPort(__port__) != RT_ERR_OK)       \
        {                                                       \
            return RT_ERR_PORT_ID;                              \
        }                                                       \
    }while(0)

#define RTK_CHK_PORT_IS_PTP(__port__)                           \
    do                                                          \
    {                                                           \
        if(rtk_switch_isPtpPort(__port__) != RT_ERR_OK)         \
        {                                                       \
            return RT_ERR_PORT_ID;                              \
        }                                                       \
    }while(0)

#define RTK_CHK_PORTMASK_VALID(__portmask__)                        \
    do                                                              \
    {                                                               \
        if(rtk_switch_isPortMaskValid(__portmask__) != RT_ERR_OK)   \
        {                                                           \
            return RT_ERR_PORT_MASK;                                \
        }                                                           \
    }while(0)

#define RTK_CHK_PORTMASK_VALID_ONLY_UTP(__portmask__)               \
    do                                                              \
    {                                                               \
        if(rtk_switch_isPortMaskUtp(__portmask__) != RT_ERR_OK)     \
        {                                                           \
            return RT_ERR_PORT_MASK;                                \
        }                                                           \
    }while(0)

#define RTK_CHK_PORTMASK_VALID_ONLY_EXT(__portmask__)               \
    do                                                              \
    {                                                               \
        if(rtk_switch_isPortMaskExt(__portmask__) != RT_ERR_OK)     \
        {                                                           \
            return RT_ERR_PORT_MASK;                                \
        }                                                           \
    }while(0)

#define RTK_CHK_TRUNK_GROUP_VALID(__grpId__)                        \
    do                                                              \
    {                                                               \
        if(rtk_switch_isValidTrunkGrpId(__grpId__) != RT_ERR_OK)    \
        {                                                           \
            return RT_ERR_LA_TRUNK_ID;                              \
        }                                                           \
    }while(0)

#define RTK_PORTMASK_IS_PORT_SET(__portmask__, __port__)    (((__portmask__).bits[0] & (0x00000001 << __port__)) ? 1 : 0)
#define RTK_PORTMASK_IS_EMPTY(__portmask__)                 (((__portmask__).bits[0] == 0) ? 1 : 0)
#define RTK_PORTMASK_CLEAR(__portmask__)                    ((__portmask__).bits[0] = 0)
#define RTK_PORTMASK_PORT_SET(__portmask__, __port__)       ((__portmask__).bits[0] |= (0x00000001 << __port__))
#define RTK_PORTMASK_PORT_CLEAR(__portmask__, __port__)     ((__portmask__).bits[0] &= ~(0x00000001 << __port__))
#define RTK_PORTMASK_ALLPORT_SET(__portmask__)              (rtk_switch_logPortMask_get(&__portmask__))
#define RTK_PORTMASK_SCAN(__portmask__, __port__)           for(__port__ = 0; __port__ < RTK_SWITCH_PORT_NUM; __port__++)  if(RTK_PORTMASK_IS_PORT_SET(__portmask__, __port__))
#define RTK_PORTMASK_COMPARE(__portmask_A__, __portmask_B__)    ((__portmask_A__).bits[0] - (__portmask_B__).bits[0])

#define RTK_SCAN_ALL_PHY_PORTMASK(__port__)                 for(__port__ = 0; __port__ < RTK_SWITCH_PORT_NUM; __port__++)  if( (rtk_switch_phyPortMask_get() & (0x00000001 << __port__)))
#define RTK_SCAN_ALL_LOG_PORT(__port__)                     for(__port__ = 0; __port__ < RTK_SWITCH_PORT_NUM; __port__++)  if( rtk_switch_logicalPortCheck(__port__) == RT_ERR_OK)
#define RTK_SCAN_ALL_LOG_PORTMASK(__portmask__)             for((__portmask__).bits[0] = 0; (__portmask__).bits[0] < 0x7FFFF; (__portmask__).bits[0]++)  if( rtk_switch_isPortMaskValid(&__portmask__) == RT_ERR_OK)

/* Port mask definition */
#define RTK_PHY_PORTMASK_ALL                                (rtk_switch_phyPortMask_get())

/* Port definition*/
#define RTK_MAX_LOGICAL_PORT_ID                             (rtk_switch_maxLogicalPort_get())

/* Function Name:
 *      rtk_switch_probe
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
extern rtk_api_ret_t rtk_switch_probe(switch_chip_t *pSwitchChip);

/* Function Name:
 *      rtk_switch_initialState_set
 * Description:
 *      Set initial status
 * Input:
 *      state   - Initial state;
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Initialized
 *      RT_ERR_FAILED   - Uninitialized
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_initialState_set(init_state_t state);

/* Function Name:
 *      rtk_switch_initialState_get
 * Description:
 *      Get initial status
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      INIT_COMPLETED     - Initialized
 *      INIT_NOT_COMPLETED - Uninitialized
 * Note:
 *
 */
extern init_state_t rtk_switch_initialState_get(void);

/* Function Name:
 *      rtk_switch_logicalPortCheck
 * Description:
 *      Check logical port ID.
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is correct
 *      RT_ERR_FAILED   - Port ID is not correct
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_logicalPortCheck(rtk_port_t logicalPort);

/* Function Name:
 *      rtk_switch_isUtpPort
 * Description:
 *      Check is logical port a UTP port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a UTP port
 *      RT_ERR_FAILED   - Port ID is not a UTP port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_isUtpPort(rtk_port_t logicalPort);

/* Function Name:
 *      rtk_switch_isExtPort
 * Description:
 *      Check is logical port a Extension port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a EXT port
 *      RT_ERR_FAILED   - Port ID is not a EXT port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_isExtPort(rtk_port_t logicalPort);

/* Function Name:
 *      rtk_switch_isHsgPort
 * Description:
 *      Check is logical port a HSG port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a HSG port
 *      RT_ERR_FAILED   - Port ID is not a HSG port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_isHsgPort(rtk_port_t logicalPort);

/* Function Name:
 *      rtk_switch_isSgmiiPort
 * Description:
 *      Check is logical port a SGMII port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a SGMII port
 *      RT_ERR_FAILED   - Port ID is not a SGMII port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_isSgmiiPort(rtk_port_t logicalPort);

/* Function Name:
 *      rtk_switch_isCPUPort
 * Description:
 *      Check is logical port a CPU port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a CPU port
 *      RT_ERR_FAILED   - Port ID is not a CPU port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_isCPUPort(rtk_port_t logicalPort);

/* Function Name:
 *      rtk_switch_isComboPort
 * Description:
 *      Check is logical port a Combo port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a combo port
 *      RT_ERR_FAILED   - Port ID is not a combo port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_isComboPort(rtk_port_t logicalPort);

/* Function Name:
 *      rtk_switch_ComboPort_get
 * Description:
 *      Get Combo port ID
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      Port ID of combo port
 * Note:
 *
 */
extern rtk_uint32 rtk_switch_ComboPort_get(void);

/* Function Name:
 *      rtk_switch_isPtpPort
 * Description:
 *      Check is logical port a PTP port
 * Input:
 *      logicalPort     - logical port ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Port ID is a PTP port
 *      RT_ERR_FAILED   - Port ID is not a PTP port
 *      RT_ERR_NOT_INIT - Not Initialize
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_isPtpPort(rtk_port_t logicalPort);

/* Function Name:
 *      rtk_switch_port_L2P_get
 * Description:
 *      Get physical port ID
 * Input:
 *      logicalPort       - logical port ID
 * Output:
 *      None
 * Return:
 *      Physical port ID
 * Note:
 *
 */
extern rtk_uint32 rtk_switch_port_L2P_get(rtk_port_t logicalPort);

/* Function Name:
 *      rtk_switch_port_P2L_get
 * Description:
 *      Get logical port ID
 * Input:
 *      physicalPort       - physical port ID
 * Output:
 *      None
 * Return:
 *      logical port ID
 * Note:
 *
 */
extern rtk_port_t rtk_switch_port_P2L_get(rtk_uint32 physicalPort);

/* Function Name:
 *      rtk_switch_isPortMaskValid
 * Description:
 *      Check portmask is valid or not
 * Input:
 *      pPmask       - logical port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - port mask is valid
 *      RT_ERR_FAILED       - port mask is not valid
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_isPortMaskValid(rtk_portmask_t *pPmask);

/* Function Name:
 *      rtk_switch_isPortMaskUtp
 * Description:
 *      Check all ports in portmask are only UTP port
 * Input:
 *      pPmask       - logical port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Only UTP port in port mask
 *      RT_ERR_FAILED       - Not only UTP port in port mask
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_isPortMaskUtp(rtk_portmask_t *pPmask);

/* Function Name:
 *      rtk_switch_isPortMaskExt
 * Description:
 *      Check all ports in portmask are only EXT port
 * Input:
 *      pPmask       - logical port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Only EXT port in port mask
 *      RT_ERR_FAILED       - Not only EXT port in port mask
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_isPortMaskExt(rtk_portmask_t *pPmask);

/* Function Name:
 *      rtk_switch_portmask_L2P_get
 * Description:
 *      Get physical portmask from logical portmask
 * Input:
 *      pLogicalPmask       - logical port mask
 * Output:
 *      pPhysicalPortmask   - physical port mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 *      RT_ERR_PORT_MASK    - Error port mask
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_portmask_L2P_get(rtk_portmask_t *pLogicalPmask, rtk_uint32 *pPhysicalPortmask);

/* Function Name:
 *      rtk_switch_portmask_P2L_get
 * Description:
 *      Get logical portmask from physical portmask
 * Input:
 *      physicalPortmask    - physical port mask
 * Output:
 *      pLogicalPmask       - logical port mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 *      RT_ERR_PORT_MASK    - Error port mask
 * Note:
 *
 */
extern rtk_api_ret_t rtk_switch_portmask_P2L_get(rtk_uint32 physicalPortmask, rtk_portmask_t *pLogicalPmask);

/* Function Name:
 *      rtk_switch_phyPortMask_get
 * Description:
 *      Get physical portmask
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0x00                - Not Initialize
 *      Other value         - Physical port mask
 * Note:
 *
 */
rtk_uint32 rtk_switch_phyPortMask_get(void);

/* Function Name:
 *      rtk_switch_logPortMask_get
 * Description:
 *      Get Logical portmask
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_NOT_INIT     - Not Initialize
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
rtk_api_ret_t rtk_switch_logPortMask_get(rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_switch_init
 * Description:
 *      Set chip to default configuration environment
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API can set chip registers to default configuration for different release chip model.
 */
extern rtk_api_ret_t rtk_switch_init(void);

/* Function Name:
 *      rtk_switch_portMaxPktLen_set
 * Description:
 *      Set Max packet length
 * Input:
 *      port    - Port ID
 *      speed   - Speed
 *      cfgId   - Configuration ID
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
extern rtk_api_ret_t rtk_switch_portMaxPktLen_set(rtk_port_t port, rtk_switch_maxPktLen_linkSpeed_t speed, rtk_uint32 cfgId);

/* Function Name:
 *      rtk_switch_portMaxPktLen_get
 * Description:
 *      Get Max packet length
 * Input:
 *      port    - Port ID
 *      speed   - Speed
 * Output:
 *      pCfgId  - Configuration ID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
extern rtk_api_ret_t rtk_switch_portMaxPktLen_get(rtk_port_t port, rtk_switch_maxPktLen_linkSpeed_t speed, rtk_uint32 *pCfgId);

/* Function Name:
 *      rtk_switch_maxPktLenCfg_set
 * Description:
 *      Set Max packet length configuration
 * Input:
 *      cfgId   - Configuration ID
 *      pktLen  - Max packet length
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
extern rtk_api_ret_t rtk_switch_maxPktLenCfg_set(rtk_uint32 cfgId, rtk_uint32 pktLen);

/* Function Name:
 *      rtk_switch_maxPktLenCfg_get
 * Description:
 *      Get Max packet length configuration
 * Input:
 *      cfgId   - Configuration ID
 *      pPktLen - Max packet length
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
extern rtk_api_ret_t rtk_switch_maxPktLenCfg_get(rtk_uint32 cfgId, rtk_uint32 *pPktLen);

/* Function Name:
 *      rtk_switch_greenEthernet_set
 * Description:
 *      Set all Ports Green Ethernet state.
 * Input:
 *      enable - Green Ethernet state.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - OK
 *      RT_ERR_FAILED   - Failed
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_ENABLE   - Invalid enable input.
 * Note:
 *      This API can set all Ports Green Ethernet state.
 *      The configuration is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern rtk_api_ret_t rtk_switch_greenEthernet_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_switch_greenEthernet_get
 * Description:
 *      Get all Ports Green Ethernet state.
 * Input:
 *      None
 * Output:
 *      pEnable - Green Ethernet state.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API can get Green Ethernet state.
 */
extern rtk_api_ret_t rtk_switch_greenEthernet_get(rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_switch_maxLogicalPort_get
 * Description:
 *      Get Max logical port ID
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      Max logical port
 * Note:
 *      This API can get max logical port
 */
extern rtk_port_t rtk_switch_maxLogicalPort_get(void);

/* Function Name:
 *      rtk_switch_maxMeterId_get
 * Description:
 *      Get Max Meter ID
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0x00                - Not Initialize
 *      Other value         - Max Meter ID
 * Note:
 *
 */
extern rtk_uint32 rtk_switch_maxMeterId_get(void);

/* Function Name:
 *      rtk_switch_maxLutAddrNumber_get
 * Description:
 *      Get Max LUT Address number
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      0x00                - Not Initialize
 *      Other value         - Max LUT Address number
 * Note:
 *
 */
extern rtk_uint32 rtk_switch_maxLutAddrNumber_get(void);

/* Function Name:
 *      rtk_switch_isValidTrunkGrpId
 * Description:
 *      Check if trunk group is valid or not
 * Input:
 *      grpId       - Group ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Trunk Group ID is valid
 *      RT_ERR_LA_TRUNK_ID  - Trunk Group ID is not valid
 * Note:
 *
 */
rtk_uint32 rtk_switch_isValidTrunkGrpId(rtk_uint32 grpId);

#endif
