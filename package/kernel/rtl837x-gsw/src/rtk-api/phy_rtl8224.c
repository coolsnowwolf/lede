/*
 * Copyright (C) 2018 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 75479 $
 * $Date: 2017-01-20 15:17:16 +0800 (Fri, 20 Jan 2017) $
 *
 * Purpose : PHY 8284 Driver APIs.
 *
 * Feature : PHY 8284 Driver APIs
 *
 */
 #include <rtk_switch.h>
#include <rtk_error.h>
#include "miim.h"
#include "ptp.h"
#include "chip.h"
#include "port.h"
#include "phydef.h"
#include <phy_rtl8224.h>
#include <dal_rtl8373_ptp.h>
#include <rtl8373_reg_definition.h>
#include <string.h>
/*
 * Data Declaration
 */

static rtk_uint32   ptp_portmask = 0x0f;

/* Function Name:
 *      phy_8224_bypassptpEn_get
 * Description:
 *      Get PTP status of the specified port.
 * Input:
 *      port    - port id
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
ret_t phy_8224_bypassptpEn_get( rtk_port_t port, rtk_enable_t *pEnable)
{
    ret_t   retVal;
    
    if(pEnable==NULL)
        return RT_ERR_INPUT;
    
    retVal= rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_P0_MISC_CTRL_ADDR(port),RTL8373_P0_MISC_CTRL_P0_CFG_BYPASS_MASK,pEnable);
    if(retVal != RT_ERR_OK)
        return retVal;
     
     return RT_ERR_OK;
} 

/* Function Name:
 *      phy_8224_bypassptpEn_set
 * Description:
 *      Get PTP status of the specified port.
 * Input:
 *      port    - port id
 *      pEnable - status
 * Output:
  *     None 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT         - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
ret_t phy_8224_bypassptpEn_set( rtk_port_t port, rtk_enable_t pEnable)
{ 
    return rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_P0_MISC_CTRL_ADDR(port),RTL8373_P0_MISC_CTRL_P0_CFG_BYPASS_MASK,pEnable);
} 
/* Function Name:
 *      phy_8224_ptp_portmask
 * Description:
 *      PTP function initialization.
 * Input:
 *      portmask   range 0~0xf
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API is used to initialize PTP status.
 */
ret_t phy_8224_ptp_portmask(rtk_portmask_t portmask)
{    
     ptp_portmask=portmask.bits[0];
     return RT_ERR_OK;
}
/* Function Name:
 *      phy_8224_ptp_init
 * Description:
 *      PTP function initialization.
 * Input:
 *      ptpinternalpmask    port range 0~0xf
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API is used to initialize PTP status.
 */
ret_t phy_8224_ptp_init(  rtk_portmask_t ptppmask)
{
  
    rtk_port_t port;
    rtk_uint32 freq=0x10000000;//internal clock

    /* Check initialization state */
    phy_8224_ptp_portmask(ptppmask);

    for(port=UTP_PORT0;port<RTK_MAX_LOGICAL_PORT_ID;port++)
    {
         if ((1<<port) & ptp_portmask)
        {
            phy_8224_bypassptpEn_set(port,DISABLED);
        }
        else
        {
            /*to be added*/
        } 
    }
    phy_8224_ptp_ClkSrcCtrl_set(0);// 1-external,0=internal                                                          
    phy_8224_ptp_tpid_set( INNER_VLAN, 0, 0X8100);
    phy_8224_ptp_tpid_set( OUTER_VLAN, 0, 0X88a8);

    phy_8224_ptp_RefTimeFreqCfg_set(freq,ENABLED);
    
    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8224_ptp_tpid_set
 * Description:
 *      Configure PTP accepted outer & inner tag TPID.
 * Input:
 *      outerId - Ether type of S-tag frame parsing in PTP ports.
 *      innerId - Ether type of C-tag frame parsing in PTP ports.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      None
 */
ret_t phy_8224_ptp_tpid_set(rtk_vlanType_t type, rtk_uint32 idx, rtk_uint32 Tpid)
{
    ret_t retVal;
    
    /* Check initialization state */
    if((type >= VLAN_TYPE_END) || (idx > 4))
        return RT_ERR_INPUT;
       
    /* Check initialization state */
    RTK_CHK_INIT_STATE();
    
    if(type == OUTER_VLAN)
    {
        if((retVal = rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_OTAG_CONFIG0_ADDR+(4<<idx), Tpid)) != RT_ERR_OK)
             return retVal;
    }
    else if(type == INNER_VLAN)
    {
        if((retVal = rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_ITAG_CONFIG0_ADDR+(4<<idx), Tpid)) != RT_ERR_OK)
            return retVal;        
    }   
    
    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8224_ptp_tpid_get
 * Description:
 *      Get PTP accepted outer or inner tag TPID.
 * Input:
 *      type
 *      idx
 * Output:
 *      pTpid - Ether type of tag frame parsing in PTP ports.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */
ret_t phy_8224_ptp_tpid_get(rtk_vlanType_t type, rtk_uint32 idx, rtk_uint32 *pTpid)
{
    ret_t retVal;

    if((pTpid == NULL) || (type >= VLAN_TYPE_END) || (idx > 4))
        return RT_ERR_INPUT;
       
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(type == OUTER_VLAN)
    {
        if((retVal = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_OTAG_CONFIG0_ADDR+(4<<idx), pTpid)) != RT_ERR_OK)
             return retVal;
    }
    else if(type == INNER_VLAN)
    {
        if((retVal = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_ITAG_CONFIG0_ADDR+(4<<idx), pTpid)) != RT_ERR_OK)
            return retVal;        
    }
    
    return RT_ERR_OK;
    
}

/* Function Name:
 *      phy_8224_ptp_Oper_get
 * Description:
 *      Get the PTP time operation configuration of specific port.
 * Input:
 *      port        - port ID
 * Output:
 *      pOperCfg    - pointer to PTP time operation configuraton
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
ret_t phy_8224_ptp_Oper_get(rtk_time_operCfg_t *pOperCfg)
{
        rtk_int32   ret = RT_ERR_OK;
        rtk_uint32  reg_val = 0;
        rtk_uint32  oper = 0;
    
        if((ret = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_OP_CTRL_ADDR,&reg_val)) != RT_ERR_OK)
            return ret;
        
        oper = (reg_val & RTL8373_PTP_TIME_OP_CTRL_CFG_GPI_OP_MASK) >> 4;
        switch (oper)
        {
            case 0:
                pOperCfg->oper = TIME_OPER_START;
                break;
            case 1:
                pOperCfg->oper = TIME_OPER_LATCH;
                break;
            case 2:
                pOperCfg->oper = TIME_OPER_STOP;
                break;
            case 3:
                pOperCfg->oper = TIME_OPER_CMD_EXEC;
                break;
            case 4:
                pOperCfg->oper = TIME_OPER_FREQ_APPLY;
                break;
            default:
                return RT_ERR_FAILED;
        }

        pOperCfg->rise_tri = (reg_val & RTL8373_PTP_TIME_OP_CTRL_CFG_GPI_RISE_TRIG_MASK) >> 3;
        pOperCfg->fall_tri = (reg_val & RTL8373_PTP_TIME_OP_CTRL_CFG_GPI_FALL_TRIG_MASK) >> 2;

        /*to be added phy*/
   
    return ret;
}

/* Function Name:
 *      phy_8224_ptp_Oper_set
 * Description:
 *      Set the PTP time operation configuration of specific port.
 * Input:
 *      port        - port ID
 *      pOperCfg    - pointer to PTP time operation configuraton
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
ret_t phy_8224_ptp_Oper_set( rtk_time_operCfg_t pOperCfg)
{
        rtk_int32   ret = RT_ERR_OK;
        rtk_uint32  reg_val = 0;
                
        switch (pOperCfg.oper)
        {
            case TIME_OPER_START:
                reg_val = 0 << 4;
                break;
            case TIME_OPER_LATCH:
                reg_val = 1 << 4;
                break;
            case TIME_OPER_STOP:
                reg_val = 2 << 4;
                break;
            case TIME_OPER_CMD_EXEC:
                reg_val = 3 << 4;
                break;
            case TIME_OPER_FREQ_APPLY:
                reg_val = 4 << 4;
                break;
            default:
                return RT_ERR_FAILED;
        }
 
        reg_val |= (pOperCfg.rise_tri == ENABLED) ? (RTL8373_PTP_TIME_OP_CTRL_CFG_GPI_RISE_TRIG_MASK) : (0);
        reg_val |= (pOperCfg.fall_tri == ENABLED) ? (RTL8373_PTP_TIME_OP_CTRL_CFG_GPI_FALL_TRIG_MASK) : (0);
        if((ret = rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_OP_CTRL_ADDR, reg_val)) != RT_ERR_OK)
            return ret;
 
        /*to be added phy*/
   
    return ret;
}

/* Function Name:
 *      dal_rtl8371c_ptp_LatchTime_get
 * Description:
 *      Get the PTP latched time of specific port by hardware.
 * Input:
 * Output:
 *      pLatchTime    - pointer to PTP time operation configuraton
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
ret_t phy_8224_ptp_LatchTime_get( rtk_time_timeStamp_t *pLatchTime)
{
    rtk_int32   ret = RT_ERR_OK;
    rtk_uint32  sec_l = 0;
    rtk_uint32  sec_m = 0;
    rtk_uint32  sec_h = 0;
    rtk_uint32  nsec_l = 0;
    rtk_uint32  nsec_h = 0;
    
    if((ret = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC_RD2_ADDR, &sec_h)) != RT_ERR_OK)
        return ret;
    if((ret = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC_RD1_ADDR, &sec_m)) != RT_ERR_OK)
        return ret;
    if((ret = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC_RD0_ADDR, &sec_l)) != RT_ERR_OK)
        return ret;
    if((ret = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_NSEC_RD1_ADDR, &nsec_h)) != RT_ERR_OK)
        return ret;
    if((ret = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_NSEC_RD0_ADDR, &nsec_l)) != RT_ERR_OK)
        return ret;

    pLatchTime->sec = ((rtk_uint64)sec_h << 32) | ((rtk_uint64)sec_m << 16) | ((rtk_uint64)sec_l & 0xFFFF);
    pLatchTime->nsec = (((nsec_h & 0x3FFF) << 16) | (nsec_l & 0xFFFF));
  
        /*to be added phy*/
 
    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8224_ptp_refTimeOp_set
 * Description:
 *      Set the reference time of the specified device.
 * Input:
 *      extoption - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 * Note:
 *      need gpio triger
 */
ret_t phy_8224_ptp_refTimeOp_set(rtk_uint32 extoption)
{   
    if(extoption>=PTP_TIME_CMD_END)
         return RT_ERR_INPUT;
    
   return rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_CRTL_ADDR,RTL8373_PTP_TIME_CRTL_PTP_TIME_CMD_MASK, extoption);         
}
/* Function Name:
 *      dal_rtl8371c_ptp_refTime_set
 * Description:
 *      Set the reference time of the specified device.
 * Input:
 *      type
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 * Note:
 *      need gpio triger
 */
ret_t phy_8224_ptp_refTime_set(rtk_time_timeStamp_t timeStamp)
{
    ret_t retVal;
    rtk_uint32 sec_h, sec_m,sec_l, nsec8_h, nsec8_l;
    rtk_uint32 nano_second_8;
    rtk_uint32  busyFlag, count;
    /* Check initialization state */
    RTK_CHK_INIT_STATE();
        
    if (timeStamp.nsec > RTK_MAX_NUM_OF_NANO_SECOND)
        return RT_ERR_INPUT;

    sec_h = (timeStamp.sec>>32)& 0xFFFF;
    sec_m = (timeStamp.sec >>16)& 0xFFFF;
    sec_l = timeStamp.sec & 0xFFFF;
    nano_second_8 = timeStamp.nsec >> 3;
    nsec8_h = (nano_second_8 >>16) & 0xFFFF;
    nsec8_l = nano_second_8 &0xFFFF;

    if((retVal = rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_NSEC1_ADDR, nsec8_h)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_NSEC0_ADDR, nsec8_l)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC2_ADDR, sec_h)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC1_ADDR, sec_m)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC0_ADDR, sec_l)) != RT_ERR_OK)
        return retVal;

   if((retVal = rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_CRTL_ADDR,RTL8373_PTP_TIME_CRTL_PTP_TIME_CMD_MASK, PTP_TIME_WRITE)) != RT_ERR_OK)
        return retVal;    
    if((retVal = rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_NSEC1_ADDR,RTL8373_PTP_TIME_NSEC1_CFG_TOD_VALID_MASK,ENABLED)) != RT_ERR_OK)
        return retVal;    

    if((retVal = rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_CRTL_ADDR,RTL8373_PTP_TIME_CRTL_PTP_TIME_EXEC_MASK, 1)) != RT_ERR_OK)
             return retVal;

        count = 0;
        do {
        if((retVal = rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_CRTL_ADDR, RTL8373_PTP_TIME_CRTL_PTP_TIME_EXEC_MASK, &busyFlag)) != RT_ERR_OK)
            return retVal;
        count++;
        } while ((busyFlag != 0)&&(count<5));

        if (busyFlag != 0)
        return RT_ERR_BUSYWAIT_TIMEOUT;
    
    return RT_ERR_OK;
}


/* Function Name:
 *      phy_8224_ptp_refTime_get
 * Description:
 *      Get the reference time of the specified device by software.
 * Input:
 * Output:
 *      pTimeStamp - pointer buffer of the reference time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8371
 * Note:
 *      need gpio triger
 */
ret_t phy_8224_ptp_refTime_get(rtk_time_timeStamp_t *pTimeStamp)
{
    ret_t retVal;
    rtk_uint32 sec_h, sec_m,sec_l;
    rtk_uint32 nsec8_h, nsec8_l;
    rtk_uint32 nano_second_8;
    rtk_uint32  busyFlag, count;
    
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(pTimeStamp == NULL)
    {
         return RT_ERR_INPUT;
    }   
     if((retVal = rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_CRTL_ADDR,RTL8373_PTP_TIME_CRTL_PTP_TIME_CMD_MASK, PTP_TIME_READ)) != RT_ERR_OK)
        return retVal;

     if((retVal = rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_CRTL_ADDR,RTL8373_PTP_TIME_CRTL_PTP_TIME_EXEC_MASK, 1)) != RT_ERR_OK)
             return retVal;

    count = 0;
    do {
    if((retVal = rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_CRTL_ADDR, RTL8373_PTP_TIME_CRTL_PTP_TIME_EXEC_MASK, &busyFlag)) != RT_ERR_OK)
        return retVal;
    count++;
    } while ((busyFlag != 0)&&(count<5));

    if (busyFlag != 0)
    return RT_ERR_BUSYWAIT_TIMEOUT;
    
    if((retVal = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC_RD2_ADDR, &sec_h)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC_RD1_ADDR, &sec_m)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC_RD0_ADDR, &sec_l)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_NSEC_RD1_ADDR, &nsec8_h)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_NSEC_RD0_ADDR, &nsec8_l)) != RT_ERR_OK)
        return retVal;

    pTimeStamp->sec= ((rtk_uint64)sec_h<<32) | ((rtk_uint64)sec_m<<16) |(rtk_uint64)sec_l;
    nano_second_8 = (nsec8_h<<16) | nsec8_l;
    pTimeStamp->nsec= nano_second_8<<3;

/*to be added*/  
    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8224_ptp_refTimeAdjust_set
 * Description:
 *      Adjust the reference time.
 * Input:
 *      sign      - significant
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      sign=0 for positive adjustment, sign=1 for negative adjustment.
 */
 ret_t phy_8224_ptp_refTimeAdjust_set( rtk_ptp_sys_adjust_t sign, rtk_time_timeStamp_t timeStamp)
{
    ret_t retVal;
    rtk_uint32 sec_h, sec_m,sec_l, nsec8_h, nsec8_l;
    rtk_uint32 nano_second_8;
    rtk_uint64  sec;
    rtk_uint32  nsec=0;
    
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (timeStamp.nsec > DAL_RTL8373_MAX_NUM_OF_NANO_SECOND)
        return RT_ERR_INPUT;

    if (sign >= SYS_ADJUST_END)
        return RT_ERR_INPUT;

     if (sign == SYS_ADJUST_MINUS)
    {
        sec = 0;
    }
    else
    {
        /* adjust Timer of PHY */
        sec = 0 - (timeStamp.sec + 1);
        nsec = 1000000000 - timeStamp.nsec;
    }


    sec_h = (rtk_uint32)(sec >>32)& 0xFFFF;
    sec_m = (rtk_uint32)(sec  >>16)& 0xFFFF;
    sec_l = sec  & 0xFFFF;
    nano_second_8 = nsec  >> 3;
    nsec8_h = (nano_second_8 >>16) & 0xFFFF;
    nsec8_l = nano_second_8 &0xFFFF;
    
    if((retVal =  rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC2_ADDR, sec_h)) != RT_ERR_OK)
        return retVal;
    if((retVal =  rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC1_ADDR, sec_m)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC0_ADDR, sec_l)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_NSEC1_ADDR, nsec8_h)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_NSEC0_ADDR, nsec8_l)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}


/* Function Name:
 *      phy_8224_ptp_TxTimestampFifo_get
 * Description:
 *      Get the top entry from PTP Tx timstamp FIFO on the dedicated port from the specified device.
 * Input:
 *      port        - port id
 * Output:
 *      pTimeEntry  - pointer buffer of TIME timestamp entry
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
ret_t phy_8224_ptp_TxTimestampFifo_get(rtk_time_txTimeEntry_t *pTimeEntry)
{
    ret_t   retVal = RT_ERR_OK;
    rtk_uint32  reg_val = 0;
 //   rtk_uint32  reg_msk = 0;
    rtk_uint32  sec_l = 0;
    rtk_uint32  sec_m = 0;
    rtk_uint32  sec_h = 0;
    rtk_uint32 regData,count,busyFlag;
 //   rtl8371c_ptp_tx_time_stamp_t txtimestamp;
   
        if((retVal = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TX_TIMESTAMP_RD0_ADDR,&reg_val)) != RT_ERR_OK)
            return retVal;

        pTimeEntry->valid = (reg_val & RTL8373_PTP_TX_TIMESTAMP_RD0_RD_TX_TIMESTAMP_VALID_MASK)? 1:0;
        pTimeEntry->port = (reg_val & RTL8373_PTP_TX_TIMESTAMP_RD0_RD_PORT_ID_MASK) >> 8;
        pTimeEntry->msg_type = (reg_val & RTL8373_PTP_TX_TIMESTAMP_RD0_RD_MSG_TYPE_MASK) >> 6;
        pTimeEntry->seqId = (reg_val & RTL8373_PTP_TX_TIMESTAMP_RD0_RD_SEQ_ID_H_MASK) << 10;

        if((retVal = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TX_TIMESTAMP_RD1_ADDR,&reg_val)) != RT_ERR_OK)
            return retVal;

        pTimeEntry->seqId |= (reg_val & RTL8373_PTP_TX_TIMESTAMP_RD1_RD_SEQ_ID_L_MASK) >> 6;
        pTimeEntry->txTime.sec = (reg_val & RTL8373_PTP_TX_TIMESTAMP_RD1_RD_TX_TIMESTAMP_SEC_H_MASK) << 2;

        if((retVal = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TX_TIMESTAMP_RD2_ADDR,&reg_val)) != RT_ERR_OK)
            return retVal;

        pTimeEntry->txTime.sec |= (reg_val & RTL8373_PTP_TX_TIMESTAMP_RD2_RD_TX_TIMESTAMP_SEC_L_MASK) >> 14;
        pTimeEntry->txTime.nsec = (reg_val & RTL8373_PTP_TX_TIMESTAMP_RD2_RD_TX_TIMESTAMP_NSEC_H_MASK) << 16;

        if((retVal = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TX_TIMESTAMP_RD3_ADDR,&reg_val)) != RT_ERR_OK)
            return retVal;

         pTimeEntry->txTime.nsec |= (reg_val & RTL8373_PTP_TX_TIMESTAMP_RD3_RD_TX_TIMESTAMP_NSEC_L_MASK);

      //  rtl8371c_getAsicEavSysTime(&Sec,&NanoSec);/* 8 bits sec is not overflow yet */

          regData = (PTP_TIME_READ<<RTL8373_PTP_TIME_CRTL_PTP_TIME_EXEC_OFFSET) | 1;

        if((retVal = rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_CRTL_ADDR, regData)) != RT_ERR_OK)
            return retVal;

        count = 0;
        do {
            if((retVal = rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_CRTL_ADDR, RTL8373_PTP_TIME_CRTL_PTP_TIME_EXEC_MASK, &busyFlag)) != RT_ERR_OK)
                return retVal;
            count++;
        } while ((busyFlag != 0)&&(count<5));

        if (busyFlag != 0)
            return RT_ERR_BUSYWAIT_TIMEOUT;

        if((retVal = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC_RD2_ADDR, &sec_h)) != RT_ERR_OK)
            return retVal;
        if((retVal = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC_RD1_ADDR, &sec_m)) != RT_ERR_OK)
            return retVal;
        if((retVal = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_SEC_RD0_ADDR, &sec_l)) != RT_ERR_OK)
            return retVal;
        
        if((sec_l&0xff)>=pTimeEntry->txTime.sec)
        {
              pTimeEntry->txTime.sec |= ((rtk_uint64)sec_h << 32) | ((rtk_uint64)sec_m << 16) | ((rtk_uint64)sec_l & 0xFF00);
        }
        else
        {
            pTimeEntry->txTime.sec |= ((rtk_uint64)sec_h << 32) | ((rtk_uint64)sec_m << 16) | ((rtk_uint64)sec_l & 0xFF00);
            pTimeEntry->txTime.sec -= 0x100;
        }
 
 
        /*to be added phy*/   

    return RT_ERR_OK;
}


/* Function Name:
 *      phy_8224_ptp_1PPSOutput_get
 * Description:
 *      Get 1 PPS output configuration of the specified port.
 * Input:
 *      type        - phy or mac
 * Output:
 *      pPulseWidth - pointer to 1 PPS pulse width, unit: 10 ms
 *      pEnable     - pointer to 1 PPS output enable status
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
ret_t phy_8224_ptp_1PPSOutput_get( rtk_uint32 *pPulseWidth, rtk_enable_t *pEnable)
{
    ret_t   ret ;
    rtk_uint32  reg_val = 0;

    ret=rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_PPS_CTRL_ADDR,&reg_val);
    
    if (reg_val & RTL8373_PTP_PPS_CTRL_CFG_PPS_EN_MASK)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    *pPulseWidth = reg_val & RTL8373_PTP_PPS_CTRL_CFG_PPS_WIDTH_MASK;

    /*to be added phy*/


    return ret;
}


/* Function Name:
 *      phy_8224_ptp_1PPSOutput_set
 * Description:
 *      Set 1 PPS output configuration of the specified port.
 * Input:
 *      type        - phy or mac
 *      pulseWidth  - pointer to 1 PPS pulse width, unit: 10 ms
 *      enable      - enable 1 PPS output
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
ret_t phy_8224_ptp_1PPSOutput_set( rtk_uint32 pulseWidth, rtk_enable_t enable)
{
    rtk_uint32  reg_val = 0;

    if (pulseWidth > RTL8373_MAX_PPS_WIDTH)
        return RT_ERR_OUT_OF_RANGE;
    
    reg_val = (enable << RTL8373_PTP_PPS_CTRL_CFG_PPS_EN_OFFSET) | (pulseWidth);
    return rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_PPS_CTRL_ADDR,reg_val); 
}


/* Function Name:
 *      phy_8224_ptp_ClockOutput_get
 * Description:
 *      Get clock output configuration of the specified port.
 * Input:
 *      pClkOutput   -pClkOutput
 * Output:
 *      pClkOutput  - pointer to clock output configuration
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
ret_t phy_8224_ptp_ClockOutput_get(  rtk_time_clkOutput_t *pClkOutput)
{
        ret_t   ret = RT_ERR_OK;
        rtk_uint32  reg_val = 0;
        rtk_uint32  sec_l = 0;
        rtk_uint32  sec_m = 0;
        rtk_uint32  sec_h = 0;
        rtk_uint32  nsec_l = 0;
        rtk_uint32  nsec_h = 0;

        rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_SEC0_ADDR, &sec_l);
        rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_SEC1_ADDR, &sec_m);
        rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_SEC2_ADDR, &sec_h);
        rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_NSEC0_ADDR, &nsec_l);
        rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_NSEC1_ADDR, &nsec_h);

        pClkOutput->startTime.sec = ((rtk_uint64)sec_h << 32) | ((rtk_uint64)sec_m << 16) | ((rtk_uint64)sec_l & 0xFFFF);
        pClkOutput->startTime.nsec = (((nsec_h & 0x3FFF) << 16) | (nsec_l & 0xFFFF));

        rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_CTRL_ADDR, &reg_val);

       if (reg_val & RTL8373_PTP_CLKOUT_CTRL_CFG_PULSE_MODE_MASK)
            pClkOutput->mode = PTP_CLK_OUT_PULSE;
        else
            pClkOutput->mode = PTP_CLK_OUT_REPEAT;

        if (reg_val & RTL8373_PTP_CLKOUT_CTRL_CFG_CLKOUT_EN_MASK)
            pClkOutput->enable = ENABLED;
        else
            pClkOutput->enable = DISABLED;

        if (reg_val & RTL8373_PTP_CLKOUT_CTRL_RD_CLKOUT_RUN_MASK)
            pClkOutput->runing = TRUE;
        else
            pClkOutput->runing = FALSE;

        rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_HALF_PERD_NS_L_ADDR, &nsec_l);
        rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_HALF_PERD_NS_H_ADDR, &nsec_h);

        pClkOutput->halfPeriodNsec = (((nsec_h & 0x3FFF) << 16) | (nsec_l & 0xFFFF));

        /*to be added phy*/
  
    return ret;
}


/* Function Name:
 *      phy_8224_ptp_ClockOutput_set
 * Description:
 *      Set 1 PPS output configuration of the specified port.
 * Input:
*       type            - phy or mac
 *      pClkOutput  - pointer to clock output configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
ret_t phy_8224_ptp_ClockOutput_set( rtk_time_clkOutput_t pClkOutput)
{
    rtk_uint32  reg_val = 0;
    rtk_uint32  sec_l = 0;
    rtk_uint32  sec_m = 0;
    rtk_uint32  sec_h = 0;
    rtk_uint32  nsec_l = 0;
    rtk_uint32  nsec_h = 0;
    
 
        /* adjust Timer of PHY */
        sec_l = (pClkOutput.startTime.sec) & 0xFFFF;
        sec_m = ((pClkOutput.startTime.sec) >> 16) & 0xFFFF;
        sec_h = ((pClkOutput.startTime.sec) >> 32) & 0xFFFF;
        /* convert nsec to 8nsec */
        nsec_l = pClkOutput.startTime.nsec & 0xFFFF;
        nsec_h = pClkOutput.startTime.nsec >> 16;

        rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_SEC0_ADDR, sec_l);
        rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_SEC1_ADDR, sec_m);
        rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_SEC2_ADDR, sec_h);
        rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_NSEC0_ADDR, nsec_l);
        rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_NSEC1_ADDR, nsec_h);

        nsec_l = pClkOutput.halfPeriodNsec & 0xFFFF;
        nsec_h = pClkOutput.halfPeriodNsec >> 16;

        rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_HALF_PERD_NS_L_ADDR, nsec_l);
        rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_HALF_PERD_NS_H_ADDR, nsec_h);


        reg_val = (pClkOutput.mode << RTL8373_PTP_CLKOUT_CTRL_CFG_PULSE_MODE_OFFSET) | (pClkOutput.enable << RTL8373_PTP_CLKOUT_CTRL_CFG_CLKOUT_EN_OFFSET);

       rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_CLKOUT_CTRL_ADDR, reg_val);

 
    return RT_ERR_OK;

}

/* Function Name:
 *      phy_8224_ptp_portctrl_set
 * Description:
 *      Get enable status for PTP transparent clock of the specified port.
 * Input:
 *      port    - port id
 *      portcfg   -port role/udp_en/eth_en/always_ts
 * Output:
 *      
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
ret_t phy_8224_ptp_portctrl_set( rtk_port_t port, rtk_ptp_port_ctrl_t portcfg)
{
    ret_t   retVal = RT_ERR_OK;
    rtk_uint32 link_delay_l,link_delay_h; 
    
   if ((1<<port) & ptp_portmask)
    {
        if((retVal = rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_P0_PORT_CTRL_ADDR(port), RTL8373_P0_PORT_CTRL_P0_PTP_ROLE_MASK,portcfg.portrole)) != RT_ERR_OK)
            return retVal;
        if((retVal = rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_P0_PORT_CTRL_ADDR(port), RTL8373_P0_PORT_CTRL_P0_CFG_UDP_EN_MASK,portcfg.udp_en)) != RT_ERR_OK)
            return retVal;
        if((retVal = rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_P0_PORT_CTRL_ADDR(port), RTL8373_P0_PORT_CTRL_P0_CFG_ETH_EN_MASK,portcfg.eth_en)) != RT_ERR_OK)
            return retVal;
        if((retVal = rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_P0_PORT_CTRL_ADDR(port), RTL8373_P0_PORT_CTRL_P0_CFG_ALWAYS_TS_MASK,portcfg.always_ts_en)) != RT_ERR_OK)
            return retVal;
        
        link_delay_l= portcfg.link_delay & 0x3FF;
        link_delay_h = (portcfg.link_delay>>10) & 0xFFFF;
        
         if((retVal = rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_P0_LINK_DELAY_H_ADDR(port), RTL8373_P0_LINK_DELAY_H_P0_CFG_LINK_DELAY_H_MASK,link_delay_h)) != RT_ERR_OK)
            return retVal;
         if((retVal = rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_P0_PORT_CTRL_ADDR(port), RTL8373_P0_PORT_CTRL_P0_CFG_LINK_DELAY_L_MASK,link_delay_l)) != RT_ERR_OK)
            return retVal;
    }

    
    return retVal;
}


/* Function Name:
 *      phy_8224_ptp_portctrl_get
 * Description:
 *      Get enable status for PTP transparent clock of the specified port.
 * Input:
 *      port    - port id
 * Output:
 *      portcfg   -port role/udp_en/eth_en/always_ts
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
ret_t phy_8224_ptp_portctrl_get( rtk_port_t port, rtk_ptp_port_ctrl_t *pportcfg)
{
    ret_t   retVal = RT_ERR_OK;
    rtk_uint32 link_delay_l=0;
    rtk_uint32 link_delay_h=0;
    rtk_uint32  portrole = 0;
    rtk_uint32  udp_en = 0;
    rtk_uint32  eth_en = 0;
    rtk_uint32  always_ts_en = 0;

    if(pportcfg==NULL)
        return RT_ERR_INPUT;
    if ((1<<port) & ptp_portmask)
    {
        if((retVal = rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_P0_PORT_CTRL_ADDR(port), RTL8373_P0_PORT_CTRL_P0_PTP_ROLE_MASK,&portrole)) != RT_ERR_OK)
            return retVal;
        if((retVal = rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_P0_PORT_CTRL_ADDR(port), RTL8373_P0_PORT_CTRL_P0_CFG_UDP_EN_MASK,&udp_en)) != RT_ERR_OK)
            return retVal;
        if((retVal = rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_P0_PORT_CTRL_ADDR(port), RTL8373_P0_PORT_CTRL_P0_CFG_ETH_EN_MASK,&eth_en)) != RT_ERR_OK)
            return retVal;
        if((retVal = rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_P0_PORT_CTRL_ADDR(port), RTL8373_P0_PORT_CTRL_P0_CFG_ALWAYS_TS_MASK,&always_ts_en)) != RT_ERR_OK)
            return retVal;        
        if((retVal = rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_P0_LINK_DELAY_H_ADDR(port), RTL8373_P0_LINK_DELAY_H_P0_CFG_LINK_DELAY_H_MASK,&link_delay_h)) != RT_ERR_OK)
            return retVal;
        if((retVal = rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_P0_PORT_CTRL_ADDR(port), RTL8373_P0_PORT_CTRL_P0_CFG_LINK_DELAY_L_MASK,&link_delay_l)) != RT_ERR_OK)
            return retVal;
        
         pportcfg->always_ts_en=always_ts_en;
         pportcfg->eth_en=eth_en;
         pportcfg->udp_en=udp_en;
         pportcfg->link_delay=link_delay_l|(link_delay_h<<10);
         pportcfg->portrole=portrole;
   }

   
    return retVal;
}



/* Function Name:
 *      phy_8224_ptp_TxImbal_set
 * Description:
 *      Set TX/RX timer value compensation..
 * Input:
 *      port    - port id
 *      TxImbal  - TX timer value compensation
 *      RxImbal  - RX timer value compensation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      unit: 1 ns
 */
ret_t  phy_8224_ptp_TxImbal_set(rtk_port_t port, rtk_uint32 TxImbal,rtk_uint32 RxImbal)
{
    ret_t   ret ;

    ret= rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_P0_TX_IMBAL_ADDR(port),RTL8373_P0_TX_IMBAL_P0_TX_IMBAL_MASK, TxImbal);
    if(ret!=RT_ERR_OK)
        return ret;
    ret= rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_P0_RX_IMBAL_ADDR(port),RTL8373_P0_RX_IMBAL_P0_RX_IMBAL_MASK, RxImbal);
    if(ret!=RT_ERR_OK)
        return ret;
        
    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8224_ptp_TxImbal_get
 * Description:
 *      Get TX/RX timer value compensation..
 * Input:
 *      port    - port id
 *      PTxImbal  - TX timer value compensation
 *      pRxImbal  - RX timer value compensation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      unit: 1 ns
 */
ret_t phy_8224_ptp_TxImbal_get(rtk_port_t port, rtk_uint32 *pTxImbal,rtk_uint32 *pRxImbal)
{
    ret_t   ret ;

    if ((pTxImbal==NULL)||(pRxImbal==NULL))
        return RT_ERR_INPUT;

     ret= rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_P0_TX_IMBAL_ADDR(port),RTL8373_P0_TX_IMBAL_P0_TX_IMBAL_MASK, pTxImbal);
     if(ret!=RT_ERR_OK)
        return ret;
     ret=rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_P0_RX_IMBAL_ADDR(port),RTL8373_P0_RX_IMBAL_P0_RX_IMBAL_MASK, pRxImbal);
      if(ret!=RT_ERR_OK)
        return ret;
      
      return RT_ERR_OK;
}

/* Function Name:
 *      phy_8224_ptp_phyidtoportid_set
 * Description:
 *      Set Packet TX port ID..
 * Input:
 *      port    - port id
 *      ptp_portino  - TX timer value compensation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      unit: 1 ns
 */
ret_t phy_8224_ptp_phyidtoportid_set(rtk_port_t port, rtk_port_t ptp_portino)
{   
   return rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_P0_PTP_PORTID_ADDR(port), ptp_portino);
}

/* Function Name:
 *      phy_8224_ptp_phyidtoptpid_get
 * Description:
 *      Get Packet TX port ID..
 * Input:
 *      port    - port id
 *      TxImbal  - TX timer value compensation
 *      RxImbal  - RX timer value compensation
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      unit: 1 ns
 */
ret_t phy_8224_ptp_phyidtoptpid_get(rtk_port_t port, rtk_port_t *pptp_portino)
{
     return rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_P0_PTP_PORTID_ADDR(port), pptp_portino);
  
}
/* Function Name:
 *      phy_8224_ptp_PPSLatchTime_get
 * Description:
 *      Set toddelay.
 * Input:
 *      type    - accessType
 * Output:
 *      pLatchTime  - latch time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      mac mode only
 */
ret_t phy_8224_ptp_PPSLatchTime_get( rtk_time_timeStamp_t *pLatchTime)
{
    ret_t   ret = RT_ERR_OK;
    rtk_uint32  sec_l = 0;
    rtk_uint32  sec_m = 0;
    rtk_uint32  sec_h = 0;
    rtk_uint32  nsec_l = 0;
    rtk_uint32  nsec_h = 0;

    if((ret = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PPS_IN_LATCH_TIME_SEC_H_ADDR, &sec_h)) != RT_ERR_OK)
        return ret;
    if((ret = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PPS_IN_LATCH_TIME_SEC_M_ADDR, &sec_m)) != RT_ERR_OK)
        return ret;
    if((ret = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PPS_IN_LATCH_TIME_SEC_L_ADDR, &sec_l)) != RT_ERR_OK)
        return ret;
    if((ret = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PPS_IN_LATCH_TIME_NSEC_H_ADDR, &nsec_h)) != RT_ERR_OK)
        return ret;
    if((ret = rtk_port_phyReg_get(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PPS_IN_LATCH_TIME_NSEC_L_ADDR, &nsec_l)) != RT_ERR_OK)
        return ret;

    pLatchTime->sec = ((rtk_uint64)sec_h << 32) | ((rtk_uint64)sec_m << 16) | ((rtk_uint64)sec_l & 0xFFFF);
    pLatchTime->nsec = (((nsec_h & 0x3FFF) << 16) | (nsec_l & 0xFFFF));
 

    return ret;
}
/* Function Name:
 *      phy_8224_ptp_RefTimeFreqCfg_set
 * Description:
 *      Set the frequency of reference time of PHY of the specified port.
 * Input:
 *      freq    - reference time frequency
 *      apply
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      The frequency configuration decides the reference time tick frequency.
 *      The default value is 0x10000000.
 *      If it is configured to 0x8000000, the tick frequency would be half of default.
 *      If it is configured to 0x20000000, the tick frequency would be one and half times of default.
 */
ret_t phy_8224_ptp_RefTimeFreqCfg_set(rtk_uint32 freq, rtk_enable_t apply)
{
   ret_t   ret;
  
    if((ret = rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_FREQ0_ADDR,RTL8373_PTP_TIME_FREQ0_CFG_PTP_TIME_FREQ0_MASK, freq&0xffff)) != RT_ERR_OK)
         return ret;
    if((ret = rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_FREQ1_ADDR,RTL8373_PTP_TIME_FREQ1_CFG_PTP_TIME_FREQ1_MASK, (freq>>16)&0xffff)) != RT_ERR_OK)
         return ret;
    if((ret = rtk_port_phyReg_set(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_APPLY_FREQ_ADDR,apply)) != RT_ERR_OK)
         return ret;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8224_ptp_RefTimeFreqCfg_get
 * Description:
 *      Set  ptp_RefTimeFreqCfg_get.
 * Input:
 *      None    
 * Output:
 *      cfgFreq
 *      curFreq
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 */
ret_t phy_8224_ptp_RefTimeFreqCfg_get(rtk_uint32 *cfgFreq,rtk_uint32 *curFreq)
{
    rtk_uint32 freqtmp;
     ret_t   ret;
    
     if((ret = rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_FREQ0_ADDR,RTL8373_PTP_TIME_FREQ0_CFG_PTP_TIME_FREQ0_MASK, cfgFreq)) != RT_ERR_OK)
           return ret;
     if((ret = rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_TIME_FREQ1_ADDR,RTL8373_PTP_TIME_FREQ1_CFG_PTP_TIME_FREQ1_MASK, &freqtmp)) != RT_ERR_OK)
            return ret;
     *cfgFreq|=(freqtmp<<16);

     if((ret = rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_CUR_TIME_FREQ0_ADDR,RTL8373_PTP_CUR_TIME_FREQ0_CUR_PTP_TIME_FREQ0_MASK, curFreq)) != RT_ERR_OK)
              return ret;
     if((ret = rtk_port_phyReg_getBits(RTL8224PHYID,RTL8224TOPDEVAD,RTL8373_PTP_CUR_TIME_FREQ1_ADDR,RTL8373_PTP_CUR_TIME_FREQ1_CUR_PTP_TIME_FREQ1_MASK, &freqtmp)) != RT_ERR_OK)
              return ret;
     *curFreq|=(freqtmp<<16);
      
      return RT_ERR_OK;
}


/* Function Name:
 *      phy_8224_ptp_ClkSrcCtrl_set
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
ret_t phy_8224_ptp_ClkSrcCtrl_set(rtk_uint32 clksrc)
{
    if (clksrc>1)
        return RT_ERR_INPUT;

     return rtk_port_phyReg_setBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_CLK_SRC_CTRL_ADDR, RTL8373_PTP_CLK_SRC_CTRL_CFG_CLK_SRC_MASK, clksrc);
  
}

/* Function Name:
 *      phy_8224_ptp_ClkSrcCtrl_set
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
ret_t phy_8224_ptp_ClkSrcCtrl_get(rtk_uint32 *pclksrc)
{
    if (pclksrc==NULL)
        return RT_ERR_INPUT;

     return rtk_port_phyReg_getBits(RTL8224BASEID,RTL8224TOPDEVAD,RTL8373_PTP_CLK_SRC_CTRL_ADDR, RTL8373_PTP_CLK_SRC_CTRL_CFG_CLK_SRC_MASK, pclksrc);
  
}
/* Function Name:
 *      phy_8224drv_mapperInit
 * Description:
 *      Initialize PHY 8224 driver.
 * Input:
 *      pPhydrv - pointer of phy driver
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
 /*
 * Data Declaration
 */
static rt_phydrv_t phy_8224drv_mapperInit =
{   
    .fPhydrv_ptpRefTime_set = phy_8224_ptp_refTime_set,
    .fPhydrv_PtpbypassptpEn_get = phy_8224_bypassptpEn_get,
    .fPhydrv_PtpbypassptpEn_set = phy_8224_bypassptpEn_set,

   .fPhydrv_RefTimeAdjust_set = phy_8224_ptp_refTimeAdjust_set,

   .fPhydrv_PtpVlanTpid_get = phy_8224_ptp_tpid_get,
   .fPhydrv_PtpVlanTpid_set = phy_8224_ptp_tpid_set,
   .fPhydrv_PtpOper_get = phy_8224_ptp_Oper_get,
   .fPhydrv_PtpOper_set = phy_8224_ptp_Oper_set,
   .fPhydrv_PtpLatchTime_get = phy_8224_ptp_LatchTime_get,
   .fPhydrv_PtpRefTimeFreqCfg_get = phy_8224_ptp_RefTimeFreqCfg_get,
   .fPhydrv_PtpRefTimeFreqCfg_set = phy_8224_ptp_RefTimeFreqCfg_set,
   .fPhydrv_PtpTxTimestampFifo_get = phy_8224_ptp_TxTimestampFifo_get ,
   .fPhydrv_Ptp1PPSOutput_get = phy_8224_ptp_1PPSOutput_get,
   .fPhydrv_Ptp1PPSOutput_set = phy_8224_ptp_1PPSOutput_set,
   .fPhydrv_PtpClockOutput_get = phy_8224_ptp_ClockOutput_get,
   .fPhydrv_PtpClockOutput_set = phy_8224_ptp_ClockOutput_set,


   .fPhydrv_PtpPortctrl_get = phy_8224_ptp_portctrl_get,
   .fPhydrv_PtpPortctrl_set = phy_8224_ptp_portctrl_set,
 
};

/* Function Name:
 *      phy_8224drv_mapper_get
 * Description:
 *      Get DAL mapper function
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      dal_mapper_t *     - mapper pointer
 * Note:
 */
rt_phydrv_t *phy_8224drv_mapper_get(void)
{
    return &phy_8224drv_mapperInit;
} /* end of dal_rtl8371c_mapper_get */


