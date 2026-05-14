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
 * Feature : The file includes QoS module high-layer API defination
 *
 */

#ifndef __RTK_API_PORT_H__
#define __RTK_API_PORT_H__

typedef enum rtk_port_speed_e
{
    PORT_SPEED_10M = 0,
    PORT_SPEED_100M,
    PORT_SPEED_1000M,
    PORT_SPEED_500M,
    PORT_SPEED_10G,
    PORT_SPEED_2500M,
    PORT_SPEED_5G,
    PORT_SPEED_END
} rtk_port_speed_t;

typedef enum rtk_port_duplex_e
{
    PORT_HALF_DUPLEX = 0,
    PORT_FULL_DUPLEX,
    PORT_DUPLEX_END
} rtk_port_duplex_t;

typedef enum rtk_port_linkStatus_e
{
    PORT_LINKDOWN = 0,
    PORT_LINKUP,
    PORT_LINKSTATUS_END
} rtk_port_linkStatus_t;

typedef enum rtk_port_media_e
{
    PORT_MEDIA_UTP = 0,
    PORT_MEDIA_FIBER,
    PORT_MEDIA_END
}rtk_port_media_t;

typedef enum rtk_sds_mode_e 
{
    SERDES_10GQXG,
    SERDES_10GUSXG = 0xD,
    SERDES_10GR = 0x1A,
    SERDES_HSG = 0x12,
    SERDES_2500BASEX = 0x16,
    SERDES_SG = 2,
    SERDES_1000BASEX = 4,
    SERDES_100FX = 5,  
    SERDES_OFF = 0x1F,
    SERDES_8221B = 0x21,
    SERDES_ON = 0x22,
    SERDES_END
}rtk_sds_mode_t;

typedef struct  rtk_port_ability_s{
    rtk_enable_t forcemode;
    rtk_enable_t txpause;
    rtk_enable_t rxpause;
    rtk_port_linkStatus_t link;
    rtk_port_duplex_t duplex;
    rtk_port_speed_t speed;
    rtk_port_media_t media;
    rtk_enable_t smi_force_fc;
}rtk_port_ability_t;



typedef struct  rtk_port_status_s{
    rtk_uint32 txpause;
    rtk_uint32 rxpause;
    rtk_uint32 link;
    rtk_uint32 duplex;
    rtk_uint32 speed;
    rtk_uint32 media;
    rtk_uint32 eee;
    rtk_uint32 master;
    rtk_uint32 master_slave;

}rtk_port_status_t;


typedef struct rtct_result_s
{
	rtk_uint32      channelABusy;
    rtk_uint32      channelBBusy;
    rtk_uint32      channelCBusy;
    rtk_uint32      channelDBusy;

	rtk_uint32      channelAMisOpen;
    rtk_uint32      channelBMisOpen;
    rtk_uint32      channelCMisOpen;
    rtk_uint32      channelDMisOpen;

	rtk_uint32      channelAMisShort;
    rtk_uint32      channelBMisShort;
    rtk_uint32      channelCMisShort;
    rtk_uint32      channelDMisShort;

	rtk_uint32      channelAOpen;
    rtk_uint32      channelBOpen;
    rtk_uint32      channelCOpen;
    rtk_uint32      channelDOpen;

	rtk_uint32      channelAShort;
    rtk_uint32      channelBShort;
    rtk_uint32      channelCShort;
    rtk_uint32      channelDShort;

    rtk_uint32      channelANormal;
    rtk_uint32      channelBNormal;
    rtk_uint32      channelCNormal;
    rtk_uint32      channelDNormal;

    rtk_uint32      channelADone;
    rtk_uint32      channelBDone;
    rtk_uint32      channelCDone;
    rtk_uint32      channelDDone;

    rtk_uint32      channelAInterShort;
    rtk_uint32      channelBInterShort;
    rtk_uint32      channelCInterShort;
    rtk_uint32      channelDInterShort;
} rtk_rtct_result_t;


/* Function Name:
 *      rtk_setAsicPHYReg
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
extern rtk_api_ret_t rtk_port_phyReg_set(rtk_uint32 phy_mask, rtk_uint32 dev_addr, rtk_uint32 reg_addr, rtk_uint32 indata);

/* Function Name:
 *      rtk_getAsicPHYReg
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
 
extern rtk_api_ret_t rtk_port_phyReg_get(rtk_uint32 phy_id, rtk_uint32 dev_addr, rtk_uint32 reg_addr, rtk_uint32 *pdata);

/* Function Name:
 *      rtk_port_phyReg_getBits
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
 
extern rtk_api_ret_t rtk_port_phyReg_getBits(rtk_uint32 phy_id, rtk_uint32 dev_addr, rtk_uint32 reg_addr,  rtk_uint32 bitsMask,rtk_uint32 *pdata);


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
 
extern rtk_api_ret_t rtk_port_phyReg_setBits(rtk_uint32 phy_mask, rtk_uint32 dev_addr, rtk_uint32 reg_addr,  rtk_uint32 bitsMask,rtk_uint32 indata);

/* Function Name:
 *      rtk_port_macForceLink_set
 * Description:
 *      Set port force linking configuration.
 * Input:
 *      port            - port id.
 *      pPortability    - port ability configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can set Port/MAC force mode properties.
 */
extern rtk_api_ret_t rtk_port_macForceLink_set(rtk_port_t port, rtk_port_ability_t *pPortability);


/* Function Name:
 *      rtk_port_macForceLink_get
 * Description:
 *      Get port force linking configuration.
 * Input:
 *      port - Port id.
 * Output:
 *      pPortability - port ability configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can get Port/MAC force mode properties.
 */
extern rtk_api_ret_t rtk_port_macForceLink_get(rtk_port_t port, rtk_port_ability_t *pPortability);


/* Function Name:
 *      rtk_port_macStatus_get
 * Description:
 *      Get port link status.
 * Input:
 *      port - Port id.
 * Output:
 *      pPortstatus - port ability configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get Port/PHY properties.
 */
extern rtk_api_ret_t rtk_port_macStatus_get(rtk_port_t port, rtk_port_status_t *pPortstatus);


/* Function Name:
 *      rtk_port_macLocalLoopbackEnable_set
 * Description:
 *      Set Port Local Loopback. (Redirect TX to RX.)
 * Input:
 *      port    - Port id.
 *      enable  - Loopback state, 0:disable, 1:enable
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can enable/disable Local loopback in MAC.
 *      For UTP port, This API will also enable the digital
 *      loopback bit in PHY register for sync of speed between
 *      PHY and MAC. For EXT port, users need to force the
 *      link state by themself.
 */
extern rtk_api_ret_t rtk_port_macLocalLoopbackEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_macLocalLoopbackEnable_get
 * Description:
 *      Get Port Local Loopback. (Redirect TX to RX.)
 * Input:
 *      port    - Port id.
 * Output:
 *      pEnable  - Loopback state, 0:disable, 1:enable
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
extern rtk_api_ret_t rtk_port_macLocalLoopbackEnable_get(rtk_port_t port, rtk_enable_t *pEnable);


/* Function Name:
 *      rtk_port_backpressureEnable_set
 * Description:
 *      Set the half duplex backpressure enable status of the specific port.
 * Input:
 *      port    - port id.
 *      enable  - Back pressure status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set the half duplex backpressure enable status of the specific port.
 *      The half duplex backpressure enable status of the port is as following:
 *      - DISABLE(Defer)
 *      - ENABLE (Backpressure)
 */
extern rtk_api_ret_t rtk_port_backpressureEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_backpressureEnable_get
 * Description:
 *      Get the half duplex backpressure enable status of the specific port.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get the half duplex backpressure enable status of the specific port.
 *      The half duplex backpressure enable status of the port is as following:
 *      - DISABLE(Defer)
 *      - ENABLE (Backpressure)
 */
extern rtk_api_ret_t rtk_port_backpressureEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_rtctEnable_set
 * Description:
 *      Enable RTCT test
 * Input:
 *      pPortmask    - Port mask of RTCT enabled port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_MASK        - Invalid port mask.
 * Note:
 *      The API can enable RTCT Test
 */
extern rtk_api_ret_t rtk_port_rtctEnable_set(rtk_uint32 portmask);

/* Function Name:
 *      rtk_port_rtct_init
 * Description:
 *      Init RTCT test
 * Input:
 *      pPortmask    - Port mask of RTCT disabled port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_MASK        - Invalid port mask.
 * Note:
 *      The API can disable RTCT Test
 */
extern rtk_api_ret_t rtk_port_rtct_init(void);


/* Function Name:
 *      rtk_port_rtctResult_get
 * Description:
 *      Get the result of RTCT test
 * Input:
 *      port        - Port ID
 * Output:
 *      pRtctResult - The result of RTCT result
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 *      RT_ERR_PHY_RTCT_NOT_FINISH  - Testing does not finish.
 * Note:
 *      The API can get RTCT test result.
 *      RTCT test may takes 4.8 seconds to finish its test at most.
 *      Thus, if this API return RT_ERR_PHY_RTCT_NOT_FINISH or
 *      other error code, the result can not be referenced and
 *      user should call this API again until this API returns
 *      a RT_ERR_OK.
 *      The result is stored at pRtctResult->ge_result
 *      pRtctResult->linkType is unused.
 *      The unit of channel length is 2.5cm. Ex. 300 means 300 * 2.5 = 750cm = 7.5M
 */
extern rtk_api_ret_t rtk_port_rtctResult_get(rtk_port_t port, rtk_rtct_result_t *pRtctResult);


/* Function Name:
 *      rtk_sdsMode_get
 * Description:
 *      Get sds mode
 * Input:
 *      sdsId        - serdes ID,0-1
 *      pSdsMode     - serdes Mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 *      RT_ERR_PHY_RTCT_NOT_FINISH  - Testing does not finish.
 * Note:
 */
extern rtk_api_ret_t rtk_sdsMode_get(rtk_uint32 sdsId, rtk_sds_mode_t *pSdsMode);


/* Function Name:
 *      rtk_sdsMode_set
 * Description:
 *      Set sds mode
 * Input:
 *      sdsId        - serdes ID,0-1
 *      sdsMode     - serdes Mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 *      RT_ERR_PHY_RTCT_NOT_FINISH  - Testing does not finish.
 * Note:
 */
extern rtk_api_ret_t rtk_sdsMode_set(rtk_uint32 sdsId, rtk_sds_mode_t sdsMode);

/* Function Name:
 *      rtk_port_sdsNway_get
 * Description:
 *      Get serdes Nway
 * Input:
 *      sdsid        - serdes ID
 *      pState       - Nway state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 *      RT_ERR_PHY_RTCT_NOT_FINISH  - Testing does not finish.
 * Note:
 */
extern rtk_api_ret_t rtk_port_sdsNway_get(rtk_uint32 sdsId, rtk_sds_mode_t sdsMode, rtk_enable_t *pState);

/* Function Name:
 *      rtk_port_sdsNway_set
 * Description:
 *      Set sds nway status
 * Input:
 *      sdsId        - serdes ID
 *      sdsMode      - serdes mode
 *      enable       - 1:enable nway, 0:disable nway
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 *      RT_ERR_PHY_RTCT_NOT_FINISH  - Testing does not finish.
 * Note:
 */
extern rtk_api_ret_t rtk_port_sdsNway_set(rtk_uint32 sdsId, rtk_sds_mode_t sdsMode, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_extphyid_set
 * Description:
 *      Set   smi address ,SMI Address, default value equals to port number
 * Input:
 *      sdsid     - 0,1
 *      phyid   - get from phy strap pin
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t rtk_port_extphyid_set(rtk_uint32 sdsid,  rtk_uint32 phyid);

/* Function Name:
 *      rtk_port_extphyid_get
 * Description:
 *      Get   smi address ,SMI Address, default value equals to port number
 * Input:
 *      sdsid     - 0,1
 *      phyid   - get smi address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */

rtk_api_ret_t rtk_port_extphyid_get(rtk_uint32 sdsid,  rtk_uint32 *phyid);

#endif