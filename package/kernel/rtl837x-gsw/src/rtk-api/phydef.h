/*
 * Copyright (C) 2009-2016 Realtek Semiconductor Corp.
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
 * Purpose : PHY symbol and data type definition in the SDK.
 *
 * Feature : PHY symbol and data type definition
 *
 */

#ifndef __HAL_PHY_PHYDEF_H__
#define __HAL_PHY_PHYDEF_H__
#include "ptp.h"
/* definition phy driver structure */
typedef struct rt_phydrv_s
{
    rtk_int32   phydrv_index;
    ret_t   (*fPhydrv_init)(rtk_port_t);
  
    ret_t   (*fPhydrv_ptpRefTime_get)(rtk_time_timeStamp_t *);
    ret_t   (*fPhydrv_ptpRefTime_set)(rtk_time_timeStamp_t);
    ret_t   (*fPhydrv_PtpbypassptpEn_get)(rtk_port_t , rtk_enable_t *);
    ret_t   (*fPhydrv_PtpbypassptpEn_set)(rtk_port_t , rtk_enable_t );
    ret_t   (*fPhydrv_PtpEnable_get)(rtk_port_t ,rtk_ptp_header_t, rtk_enable_t *);
    ret_t   (*fPhydrv_PtpEnable_set)(rtk_port_t ,rtk_ptp_header_t,rtk_enable_t );
    ret_t   (*fPhydrv_RefTimeAdjust_set)(  rtk_uint32 , rtk_time_timeStamp_t );
    ret_t   (*fPhydrv_RefTimeEnable_get)(  rtk_enable_t *);
    ret_t   (*fPhydrv_RefTimeEnable_set)( rtk_enable_t );

    ret_t   (*fPhydrv_PtpVlanTpid_get)( rtk_vlanType_t , rtk_uint32 , rtk_uint32 *);
    ret_t   (*fPhydrv_PtpVlanTpid_set)(rtk_vlanType_t , rtk_uint32 , rtk_uint32 );
    ret_t   (*fPhydrv_PtpOper_get)(rtk_time_operCfg_t *);
    ret_t   (*fPhydrv_PtpOper_set)(  rtk_time_operCfg_t );
    ret_t   (*fPhydrv_PtpLatchTime_get)( rtk_time_timeStamp_t *);
    ret_t   (*fPhydrv_PtpRefTimeFreqCfg_get)(rtk_uint32 *, rtk_uint32 *);
    ret_t   (*fPhydrv_PtpRefTimeFreqCfg_set)(rtk_uint32, rtk_enable_t);
    ret_t   (*fPhydrv_PtpTxInterruptStatus_get)( rtk_uint32 *);
    ret_t   (*fPhydrv_PtpTxTimestampFifo_get)(rtk_time_txTimeEntry_t *);
    ret_t   (*fPhydrv_Ptp1PPSOutput_get)(rtk_uint32 *, rtk_enable_t *);
    ret_t   (*fPhydrv_Ptp1PPSOutput_set)( rtk_uint32, rtk_enable_t);
    ret_t   (*fPhydrv_PtpClockOutput_get)( rtk_time_clkOutput_t*);
    ret_t   (*fPhydrv_PtpClockOutput_set)( rtk_time_clkOutput_t );
    ret_t   (*fPhydrv_PtpToddelay_get)( rtk_uint32 *);
    ret_t   (*fPhydrv_PtpToddelay_set)( rtk_uint32 );
    ret_t   (*fPhydrv_PtpOutputSigSel_get)( rtk_time_outSigSel_t *);
    ret_t   (*fPhydrv_PtpOutputSigSel_set)( rtk_time_outSigSel_t);
    ret_t   (*fPhydrv_PtpPortctrl_get)( rtk_port_t, rtk_ptp_port_ctrl_t*);
    ret_t   (*fPhydrv_PtpPortctrl_set)(rtk_port_t, rtk_ptp_port_ctrl_t);
} rt_phydrv_t;

typedef ret_t (*phy_ident_f)( rtk_uint32 model_id, rtk_uint32 rev_id);

//typedef void  (*fPhydrv_mapperInit_f)(rt_phydrv_t* pPhydrv);
typedef rt_phydrv_t *(*fPhydrv_mapperInit_f)(void);

/* Definition family ID */
#define RTL8371_FAMILY_ID           (0x83710000)
#define RTK_PHYINFO_FLAG_NONE                       (0x0)
#define PHY_MODEL_ID_NULL               (0x0)
typedef enum phy_type_e {
    RTK_PHYTYPE_NONE = 0,       /* no phy connected */
    RTK_PHYTYPE_RTL8224,
    RTK_PHYTYPE_RTL8221B,
    RTK_PHYTYPE_RTL8218E,
    RTK_PHYTYPE_SERDES,
    RTK_PHYTYPE_UNKNOWN,        /* phy connected, but unknown type */
    RTK_PHYTYPE_INVALID,        /* somehting wrong in hardware profile */
    RTK_PHYTYPE_END,
} phy_type_t;

/* enum for supported phy driver */
typedef enum rt_supported_phydrv_e
{
    RT_PHYDRV_RTL8224,
    RT_PHYDRV_END
} rt_supported_phydrv_t;
typedef struct rt_phyInfo_s
{
    rtk_uint32  phy_num;    /* PHY number in the chip, like 8218 is 8, 8224C is 4 */
    rtk_uint32 eth_type; /* PHY speed*/
} rt_phyInfo_t;

/* definition phy control structure */
typedef struct rt_phyctrl_s
{
    phy_ident_f chk_func;
    rtk_uint32      phy_model_id;
    rtk_uint32      phy_rev_id;
    phy_type_t  phyType;
    rt_phydrv_t *pPhydrv;
    fPhydrv_mapperInit_f mapperInit_func;
    rtk_uint32      drv_rev_id;
    rt_phyInfo_t *pPhyInfo;
} rt_phyctrl_t;

#endif
