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
 * $Revision: 39583 $
 * $Date: 2013-05-20 16:59:23 +0800 (星期一, 20 五月 2013) $
 *
 * Purpose : RTK switch high-level API for RTL8367/RTL8367C
 * Feature : Here is a list of all functions and variables in time module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <ptp.h>
#include <string.h>
#include <dal/dal_mgmt.h>

/* Function Name:
 *      rtk_ptp_init
 * Description:
 *      PTP function initialization.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API is used to initialize PTP status.
 */
rtk_api_ret_t rtk_time_init(rtk_portmask_t ptpinternalpmask)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_init)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->time_init(ptpinternalpmask);
    RTK_API_UNLOCK();

    return retVal;
}
/* Function Name:
 *      rtk_time_portPtpbypassEnable_get
 * Description:
 *      Get PTP status of the specified port.
 * Input:
 *      port    - port id
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT         - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *      None
 */
rtk_api_ret_t rtk_time_portPtpbypassEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->time_portPtpbypassptpEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->time_portPtpbypassptpEn_get(port, pEnable);
    RTK_API_UNLOCK();
    return retVal;
} 

/* Function Name:
 *      rtk_time_portPtpbypassEnable_set
 * Description:
 *      Set PTP status of the specified port.
 * Input:
 *      port   - port id
 *      enable - status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT     - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *      None
 */
rtk_api_ret_t rtk_time_portPtpbypassEnable_set(rtk_port_t port, rtk_enable_t enable)
{
     rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->time_portPtpbypassptpEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->time_portPtpbypassptpEn_set(port, enable);
    RTK_API_UNLOCK();
    return retVal;

} /* end of rtk_time_portPtpEnable_set */
/* Function Name:
 *      rtk_time_portPtpEnable_get
 * Description:
 *      Get PTP status of the specified port.
 * Input:
 *      port    - port id
 *      type    - PTP_ETH or UDP
 *      enable - status
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT         - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      
 * Note:
 *      None
 * Changes:
 *      None
 */
ret_t rtk_time_portPtpEnable_get( rtk_port_t port, rtk_ptp_header_t type,rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    
    if (NULL == RT_MAPPER->time_portPtpEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->time_portPtpEnable_get(port, type,pEnable);
    RTK_API_UNLOCK();
    return retVal;
}
/* Function Name:
 *      rtk_time_portPtpEnable_set
 * Description:
 *      Get PTP status of the specified port.
 * Input:
 *      port    - port id
 *      type    - PTP_ETH or UDP
 *      enable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT         - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      
 * Note:
 *      None
 * Changes:
 *      None
 */
ret_t rtk_time_portPtpEnable_set( rtk_port_t port, rtk_ptp_header_t type,rtk_enable_t Enable)
{
    rtk_api_ret_t retVal;
    
    if (NULL == RT_MAPPER->time_portPtpEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->time_portPtpEnable_set(port, type,Enable);
    RTK_API_UNLOCK();
    return retVal;
}
/* Function Name:
 *      rtk_time_portRefTime_get
 * Description:
 *      Get the reference time of the specified port.
 * Input:
 *      port       - port id, it should be base port of PHY
 * Output:
 *      pTimeStamp - pointer buffer of the reference time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *          Add port parameter.
 */
rtk_api_ret_t rtk_time_portRefTime_get( rtk_time_timeStamp_t *pTimeStamp)
{
    rtk_api_ret_t retVal;
    
    if (NULL == RT_MAPPER->time_portRefTime_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->time_portRefTime_get(pTimeStamp);
    RTK_API_UNLOCK();
    return retVal;
} 



/* Function Name:
 *      rtk_time_portRefTime_set
 * Description:
 *      Set the reference time of the specified portmask.
 * Input:
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *          Add portmask parameter.
 */
rtk_api_ret_t rtk_time_portRefTime_set( rtk_time_timeStamp_t timeStamp,rtk_enable_t apply)
{
    rtk_api_ret_t retVal;
    
    if (NULL == RT_MAPPER->time_portRefTime_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->time_portRefTime_set( timeStamp,apply);
    RTK_API_UNLOCK();
    return retVal;
   
} /* end of rtk_time_portRefTime_set */


/* Function Name:
 *      rtk_time_portPtpOper_triger
 * Description:
 *      Set the PTP time operation triger.
 * Input:
 *      triType     
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
rtk_api_ret_t rtk_time_portPtpOper_triger(void)
{
    rtk_api_ret_t retVal;
    
    if (NULL == RT_MAPPER->time_portRefTime_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->time_portPtpOper_triger( );
    RTK_API_UNLOCK();
    return retVal;

}
/* Function Name:
 *      rtk_time_portRefTimeAdjust_set
 * Description:
 *      Adjust the reference time of portmask.
 * Input:
 *      portmask    - portmask, it should be base ports of PHYs
 *      sign      - significant
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8371
 * Note:
 *      sign=0 for positive adjustment, sign=1 for negative adjustment.
 * Changes:
 *          Change name from rtk_time_refTimeAdjust_set.
 *          Add portmask parameter.
 */
rtk_api_ret_t rtk_time_portRefTimeAdjust_set(rtk_uint32 sign, rtk_time_timeStamp_t timeStamp, rtk_enable_t apply)
{
    rtk_api_ret_t retVal;
    
    if (NULL == RT_MAPPER->time_portRefTimeAdjust_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->time_portRefTimeAdjust_set( sign, timeStamp,  apply);
    RTK_API_UNLOCK();
    return retVal;

} /* end of rtk_time_refTimeAdjust_set */


/* Function Name:
 *      rtk_time_portRefTimeEnable_get
 * Description:
 *      Get the enable state of reference time of the specified port.
 * Input:
 *      port    - port id, it should be base port of PHY
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *     8371
 * Note:
 *      None
 * Changes:
 *          Change name from rtk_time_refTimeEnable_get.
 *          Add port parameter.
 */
rtk_api_ret_t rtk_time_portRefTimeEnable_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portRefTimeEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->time_portRefTimeEnable_get(pEnable);
    RTK_API_UNLOCK();
    return retVal;

} /* end of rtk_time_portRefTimeEnable_get */


/* Function Name:
 *      rtk_time_portRefTimeEnable_set
 * Description:
 *      Set the enable state of reference time of the specified portmask.
 * Input:
 *      portmask    - portmask, it should be base ports of PHYs
 *      enable - status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *          Change name from rtk_time_refTimeEnable_set.
 *          Add portmask parameter.
 */
rtk_api_ret_t rtk_time_portRefTimeEnable_set( rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portRefTimeEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->time_portRefTimeEnable_set(enable);
    RTK_API_UNLOCK();
    return retVal;

} /* end of rtk_time_portRefTimeEnable_set */



/* Function Name:
 *      rtk_time_portPtpVlanTpid_get
 * Description:
 *      Get the VLAN TPID of specific port.
 * Input:
 *      port   - port ID
 *      type   - outer or inner VLAN
 *      idx    - TPID entry index
 * Output:
 *      pTpid  - pointer to TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtpVlanTpid_get(  rtk_vlanType_t type, rtk_uint32 idx, rtk_uint32 *pTpid)
{
       /* function body */
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtpVlanTpid_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpVlanTpid_get( type, idx, pTpid);
    RTK_API_UNLOCK();
    return retVal;
} /* end of rtk_time_portPtpVlanTpid_get */


/* Function Name:
 *      rtk_time_portPtpVlanTpid_set
 * Description:
 *      Set the VLAN TPID of specific port.
 * Input:
 *      unit    - unit id
 *      port   - port ID
 *      type   - outer or inner VLAN
 *      idx     - TPID entry index
 *      tpid    - VLAN TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtpVlanTpid_set( rtk_vlanType_t type, rtk_uint32 idx, rtk_uint32 tpid)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtpVlanTpid_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpVlanTpid_set( type, idx, tpid);
    RTK_API_UNLOCK();
    return retVal;

} /* end of rtk_time_portPtpVlanTpid_get */

/* Function Name:
 *      rtk_time_portPtpOper_get
 * Description:
 *      Get the PTP time operation configuration of specific port.
 * Input:
 *      port   - port ID
 * Output:
 *      pOperCfg  - pointer to PTP time operation configuraton
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtpOper_get(rtk_time_operCfg_t *pOperCfg)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtpOper_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpOper_get(pOperCfg);
    RTK_API_UNLOCK();
    return retVal;
    
} /* end of rtk_time_portPtpOper_get */


/* Function Name:
 *      rtk_time_portPtpOper_set
 * Description:
 *      Set the PTP time operation configuration of specific port.
 * Input:
 *      port   - port ID
 *      pOperCfg  - pointer to PTP time operation configuraton
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtpOper_set( rtk_time_operCfg_t pOperCfg)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtpOper_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpOper_set(pOperCfg);
    RTK_API_UNLOCK();
    return retVal;

} /* end of rtk_time_portPtpOper_set */


/* Function Name:
 *      rtk_time_portPtpLatchTime_get
 * Description:
 *      Get the PTP latched time of specific port.
 * Input:
 *      port   - port ID
 * Output:
 *      pOperCfg  - pointer to PTP time operation configuraton
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8371
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtpLatchTime_get(rtk_time_timeStamp_t *pLatchTime)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtpLatchTime_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpLatchTime_get( pLatchTime);
    RTK_API_UNLOCK();
    return retVal;

} /* end of rtk_time_portPtpLatchTime_get */


/* Function Name:
 *      rtk_time_portPtpRefTimeFreqCfg_get
 * Description:
 *      Get the frequency of reference time of the specified port.
 * Input:
 * Output:
 *      pFreqCfg    - pointer to configured reference time frequency
 *      pFreqCur    - pointer to current reference time frequency
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *  phy
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtpRefTimeFreqCfg_get(rtk_uint32 *pFreqCfg, rtk_uint32 *pFreqCur)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtpRefTimeFreqCfg_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpRefTimeFreqCfg_get(pFreqCfg, pFreqCur);
    RTK_API_UNLOCK();
    return retVal;
}

/* Function Name:
 *      rtk_time_portPtpRefTimeFreqCfg_set
 * Description:
 *      Set the frequency of reference time of the specified port.
 * Input:
 *      port        - port id
 *      freq        - reference time frequency
 *      apply       - if the frequency is applied immediately
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 *      phy
 * Note:
 *      0x80000000   external clock
 *      0x10000000   internal clock
 * Changes:
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtpRefTimeFreqCfg_set(rtk_uint32 freq, rtk_enable_t apply)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtpRefTimeFreqCfg_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpRefTimeFreqCfg_set(freq, apply);
    RTK_API_UNLOCK();
    return retVal;
}


/* Function Name:
 *      rtk_time_portPtpTxTimestampFifo_get
 * Description:
 *      Get the top entry from PTP Tx timstamp FIFO on the dedicated port from the specified device. of the specified port.
 * Input:
 *      port        - port id
 * Output:
 *      pTimeEntry  - pointer buffer of TIME timestamp entry
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      phy
 * Note:
 *      None
 * Changes:
 *      [SDK_NEXT_RELEASE_VERSION]
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtpTxTimestampFifo_get( rtk_time_txTimeEntry_t *pTimeEntry)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtpTxTimestampFifo_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpTxTimestampFifo_get(pTimeEntry);
    RTK_API_UNLOCK();
    return retVal;
}


/* Function Name:
 *      rtk_time_portPtp1PPSOutput_get
 * Description:
 *      Get the 1 PPS output configuration of the specified port.
 * Input:
 *      port        - port id
 * Output:
 *      pPulseWidth - pointer to 1 PPS pulse width
 *      pEnable     - pointer to 1 PPS output enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      phy
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtp1PPSOutput_get( rtk_uint32 *pPulseWidth, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtp1PPSOutput_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtp1PPSOutput_get( pPulseWidth, pEnable);
    RTK_API_UNLOCK();
    return retVal;

}


/* Function Name:
 *      rtk_time_portPtp1PPSOutput_set
 * Description:
 *      Set the 1 PPS output configuration of the specified port.
 * Input:
 *      port        - port id
 *      pulseWidth  - pointer to 1 PPS pulse width
 *      enable      - enable 1 PPS output
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 *
 * Note:
 *      None
 * Changes:
 *      [SDK_NEXT_RELEASE_VERSION]
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtp1PPSOutput_set( rtk_uint32 pulseWidth, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtp1PPSOutput_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtp1PPSOutput_set(pulseWidth, enable);
    RTK_API_UNLOCK();
    return retVal;

}
/* Function Name:
 *      rtk_time_portPtpClkSrcCtrl_get
 * Description:
 *      Set PTP time Clock source selection
 * Input:
 *      clksrc
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 * 0: internal clock (Internal PLL, 1GMHz)
 * 1: external clock, refer to cfg_ext_clk_src)
 */
rtk_api_ret_t rtk_time_portPtpClkSrcCtrl_get(rtk_enable_t* clksrc)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtpClkSrcCtrl_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpClkSrcCtrl_get(clksrc);
    RTK_API_UNLOCK();
    return retVal;

}


/* Function Name:
 *      rtk_time_portPtpClkSrcCtrl_set
 * Description:
 *      Set PTP time Clock source selection
 * Input:
 *      clksrc
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 * 0: internal clock (Internal PLL, 1GMHz)
 * 1: external clock, refer to cfg_ext_clk_src)
 */
rtk_api_ret_t rtk_time_portPtpClkSrcCtrl_set( rtk_enable_t clksrc)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtpClkSrcCtrl_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpClkSrcCtrl_set(clksrc);
    RTK_API_UNLOCK();
    return retVal;

}
/* Function Name:
 *      rtk_time_portPtptoddelay_get
 * Description:
 *     
 * Input:
 *      port        - port id
 * Output:
 *      toddelay
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtptoddelay_get(rtk_uint32* toddelay)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtp1PPSOutput_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpToddelay_get(toddelay);
    RTK_API_UNLOCK();
    return retVal;

}


/* Function Name:
 *      rtk_time_portPtptoddelay_set
 * Description:
 *      
 * Input:
 *     toddelay
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Applicable:
 *
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtptoddelay_set( rtk_uint32 toddelay)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtp1PPSOutput_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpToddelay_set(toddelay);
    RTK_API_UNLOCK();
    return retVal;

}
/* Function Name:
 *      rtk_time_portPtpClockOutput_get
 * Description:
 *      Get the clock output configuration of the specified port.
 * Input:
 *      port        - port id
 * Output:
 *      pClkOutput  - pointer to clock output configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8218E
 * Note:
 *      None
 * Changes:
 *      [SDK_NEXT_RELEASE_VERSION]
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtpClockOutput_get(rtk_time_clkOutput_t *pClkOutput)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtpClockOutput_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpClockOutput_get(pClkOutput);
    RTK_API_UNLOCK();
    return retVal;

}


/* Function Name:
 *      rtk_time_portPtpClockOutput_set
 * Description:
 *      Set the clock output configuration of the specified port.
 * Input:
 *      pClkOutput  - pointer to clock output configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtpClockOutput_set( rtk_time_clkOutput_t pClkOutput)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtpClockOutput_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpClockOutput_set( pClkOutput);
    RTK_API_UNLOCK();
    
    return retVal;
}

/* Function Name:
 *      rtk_time_portPtpPortctrl_set
 * Description:
 *      Set the PTP port ability.
 * Input:
 *      portcfg  -  portrole,linkdelay,awaysts
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      phy
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtpPortctrl_set( rtk_port_t port,rtk_ptp_port_ctrl_t portcfg)
{
   rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtpPortctrl_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpPortctrl_set(port,portcfg);
    RTK_API_UNLOCK();
    return retVal;
}


/* Function Name:
  *      Get the PTP rtk_time_portPtpPortctrl_get ability.
 * Description:
 *      Get the PTP port ability..
 * Input:
 * Output:
 *      portcfg
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - input parameter out of range
 * Note:
 *      None
 * Changes:
 *          New added function.
 */
rtk_api_ret_t rtk_time_portPtpPortctrl_get( rtk_port_t port, rtk_ptp_port_ctrl_t *portcfg)
{
   rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->time_portPtpPortctrl_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    /* function body */
    retVal = RT_MAPPER->time_portPtpPortctrl_get(port,portcfg);
    RTK_API_UNLOCK();
    return retVal;
}

/* Function Name:
 *      rtk_ptp_intControl_set
 * Description:
 *      Set PTP interrupt trigger status configuration.
 * Input:
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
 *      The API can set PTP interrupt status configuration.
 *      The interrupt trigger status is shown in the following:
 */
rtk_api_ret_t rtk_time_PtpIntControl_set(rtk_ptp_intType_t type,rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->ptp_intControl_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->ptp_intControl_set(type,enable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_ptp_intControl_get
 * Description:
 *      Get PTP interrupt trigger status configuration.
 * Input:
 * Output:
 *      pEnable - Interrupt status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get interrupt status configuration.
 *      The interrupt trigger status is shown in the following:
 */
rtk_api_ret_t rtk_time_PtpIntControl_get(rtk_ptp_intType_t type,rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->ptp_intControl_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->ptp_intControl_get(type,pEnable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_ptp_intStatus_get
 * Description:
 *      Get PTP port interrupt trigger status.
 * Input:
 *      port           - physical port
 * Output:
 *      pStatusMask - Interrupt status bit mask.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get interrupt trigger status when interrupt happened.
 *      The interrupt trigger status is shown in the following:
 *      PTP_INT_TYPE_1PPS = 0,
 *      PTP_INT_TYPE_TOD_DONE,
 *      PTP_INT_TYPE_TXTIME_EMPTY,
 *
 */
rtk_api_ret_t rtk_time_PtpIntStatus_get(rtk_ptp_intStatus_t *pStatusMask)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->ptp_intStatus_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->ptp_intStatus_get(pStatusMask);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_time_portPtpTrap_set
 * Description:
 *      Set PTP packet trap of the specified port.
 * Input:
 *      enable - status
 *      port   - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT     - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
rtk_api_ret_t rtk_time_portPtpTrap_set( rtk_port_t port, rtk_ptp_porttrap_ctrl_t *trapctrl )
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->ptp_portTrap_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->ptp_portTrap_set(port,trapctrl);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_time_portPtpTrap_get
 * Description:
 *      Get PTP packet trap of the specified port.
 * Input:
 *      port   - port id
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT         - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
rtk_api_ret_t rtk_time_portPtpTrap_get(rtk_port_t port, rtk_ptp_porttrap_ctrl_t *trapctrl )
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->ptp_portTrap_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->ptp_portTrap_get(port,trapctrl);
    RTK_API_UNLOCK();

    return retVal;
}


