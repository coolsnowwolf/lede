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
 * $Revision$
 * $Date$
 *
 * Purpose : RTK switch high-level API for RTL8373
 * Feature : Here is a list of all functions and variables in port module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <port.h>
#include <rtl8373_asicdrv.h>
#include <string.h>
#include <dal/dal_mgmt.h>


/* Function Name:
 *      rtk_port_phyReg_set
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
rtk_api_ret_t rtk_port_phyReg_set(rtk_uint32 phy_mask, rtk_uint32 dev_addr, rtk_uint32 reg_addr, rtk_uint32 indata)
{
       rtk_api_ret_t retVal;
       
    if (NULL == RT_MAPPER->fMdrv_miim_mmd_write)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->fMdrv_miim_mmd_write(phy_mask,dev_addr,reg_addr,indata);
    RTK_API_UNLOCK();

    return retVal;

}

/* Function Name:
 *      rtk_port_phyReg_get
 * Description:
 *      Configure phy register data.
 * Input:
 *      phy_id  -  phy id
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
 
rtk_api_ret_t rtk_port_phyReg_get(rtk_uint32 phy_id, rtk_uint32 dev_addr, rtk_uint32 reg_addr, rtk_uint32 *pdata)
{
       rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->fMdrv_miim_mmd_read)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->fMdrv_miim_mmd_read(phy_id,dev_addr,reg_addr,pdata);
    RTK_API_UNLOCK();

    return retVal;

}

/* Function Name:
 *      rtk_port_phyReg_getBits
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
 
rtk_api_ret_t rtk_port_phyReg_getBits(rtk_uint32 phy_id, rtk_uint32 dev_addr, rtk_uint32 reg_addr,  rtk_uint32 bitsMask,rtk_uint32 *pdata)
{
        rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->fMdrv_miim_mmd_readbits)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->fMdrv_miim_mmd_readbits(phy_id,dev_addr,reg_addr,bitsMask,pdata);
    RTK_API_UNLOCK();

    return retVal;
}

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
 
rtk_api_ret_t rtk_port_phyReg_setBits(rtk_uint32 phy_mask, rtk_uint32 dev_addr, rtk_uint32 reg_addr,  rtk_uint32 bitsMask,rtk_uint32 indata)
{
       rtk_api_ret_t retVal;
       
    if (NULL == RT_MAPPER->fMdrv_miim_mmd_writebits)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->fMdrv_miim_mmd_writebits(phy_mask,dev_addr,reg_addr,bitsMask,indata);
    RTK_API_UNLOCK();

    return retVal;

}


/* Function Name:
 *      rtk_port_macForceLink_set
 * Description:
 *      Set port force linking configuration.
 * Input:
 *      port            - port id.
 *      pPortability    - port ability configuration
 *  0x0: 10Mbps;
 * 0x1:  100Mbps;
 * 0x2: 1000Mbps;
 * 0x3: 500Mbps;
 * 0x4: 10Gbps;
 * 0x5: 2.5Gbps;
 * 0x6: 5Gbps;
 * 0x7: two_pair 1Gbps;
 * 0x8: two_pair 2.5Gbps;
 * 0x9: two_pair 5Gbps;
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
rtk_api_ret_t rtk_port_macForceLink_set(rtk_port_t port, rtk_port_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->port_macForceLink_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_macForceLink_set(port, pPortability);
    RTK_API_UNLOCK();

    return retVal;
}

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
rtk_api_ret_t rtk_port_macForceLink_get(rtk_port_t port, rtk_port_ability_t *pPortability)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_macForceLink_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_macForceLink_get(port, pPortability);
    RTK_API_UNLOCK();

    return retVal;
}


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
rtk_api_ret_t rtk_port_macStatus_get(rtk_port_t port, rtk_port_status_t *pPortstatus)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_macStatus_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_macStatus_get(port, pPortstatus);
    RTK_API_UNLOCK();

    return retVal;
}


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
rtk_api_ret_t rtk_port_macLocalLoopbackEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_macLocalLoopbackEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_macLocalLoopbackEnable_set(port, enable);
    RTK_API_UNLOCK();

    return retVal;
}

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
rtk_api_ret_t rtk_port_macLocalLoopbackEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_macLocalLoopbackEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_macLocalLoopbackEnable_get(port, pEnable);
    RTK_API_UNLOCK();

    return retVal;
}


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
rtk_api_ret_t rtk_port_backpressureEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_backpressureEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_backpressureEnable_set(port, enable);
    RTK_API_UNLOCK();

    return retVal;
}

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
rtk_api_ret_t rtk_port_backpressureEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_backpressureEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_backpressureEnable_get(port, pEnable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_port_rtctEnable_set
 * Description:
 *      Enable RTCT test
 * Input:
 *      portmask    - Port mask of RTCT enabled port
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
rtk_api_ret_t rtk_port_rtctEnable_set(rtk_uint32 portmask)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_rtct_start)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_rtct_start(portmask);
    RTK_API_UNLOCK();

    return retVal;
}

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
rtk_api_ret_t rtk_port_rtct_init(void)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_rtct_init)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_rtct_init();
    RTK_API_UNLOCK();

    return retVal;
}



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
rtk_api_ret_t rtk_port_rtctResult_get(rtk_port_t port, rtk_rtct_result_t *pRtctResult)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_rtctResult_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_rtctResult_get(port, pRtctResult);
    RTK_API_UNLOCK();

    return retVal;
}



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
rtk_api_ret_t rtk_sdsMode_get(rtk_uint32 sdsId, rtk_sds_mode_t *pSdsMode)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_sdsMode_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_sdsMode_get(sdsId, pSdsMode);
    RTK_API_UNLOCK();

    return retVal;
}


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
rtk_api_ret_t rtk_sdsMode_set(rtk_uint32 sdsId, rtk_sds_mode_t sdsMode)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_sdsMode_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_sdsMode_set(sdsId, sdsMode);
    RTK_API_UNLOCK();

    return retVal;
}

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
rtk_api_ret_t rtk_port_sdsNway_get(rtk_uint32 sdsId, rtk_sds_mode_t sdsMode, rtk_enable_t *pState)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_sdsNway_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_sdsNway_get(sdsId, sdsMode, pState);
    RTK_API_UNLOCK();

    return retVal;
}


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
rtk_api_ret_t rtk_port_sdsNway_set(rtk_uint32 sdsId, rtk_sds_mode_t sdsMode, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_sdsNway_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_sdsNway_set(sdsId, sdsMode, enable);
    RTK_API_UNLOCK();

    return retVal;
}


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

rtk_api_ret_t rtk_port_extphyid_set(rtk_uint32 sdsid,  rtk_uint32 phyid)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_extphyid_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_extphyid_set(sdsid, phyid);
    RTK_API_UNLOCK();

    return retVal;
}


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

rtk_api_ret_t rtk_port_extphyid_get(rtk_uint32 sdsid,  rtk_uint32 *phyid)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_extphyid_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_extphyid_get(sdsid, phyid);
    RTK_API_UNLOCK();

    return retVal;
}

