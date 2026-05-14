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
 * Purpose : Mapper Layer is used to seperate different kind of software or hardware platform
 *
 * Feature : Just dispatch information to Multiplex layer
 *
 */
#ifndef __DAL_MAPPER_H__
#define __DAL_MAPPER_H__

/*
 * Include Files
 */
#include <rtk_types.h>
#include <rtk_error.h>
#include <rtk_switch.h>
#include <acl.h>
#include <vlan.h>
#include <svlan.h>
#include <mirror.h>
#include <cpuTag.h>
#include <qos.h>
#include <isolation.h>
#include <sharemeter.h>
#include <storm.h>
#include <igmp.h>
#include <dot1x.h>
#include <nic.h>
#include <ptp.h>
#include <l2.h>
#include <macsec.h>
#include <trunk.h>
#include <rldp.h>
#include <dos.h>
#include <gpio.h>
#include <i2c.h>
#include <rate.h>
#include <eee.h>
#include <interrupt.h>
#include <mib.h>
#include <rma.h>
#include <led.h>
#include <port.h>
#include <phy.h>


/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

typedef struct dal_mapper_s {

    /* switch */
    rtk_api_ret_t   (*switch_init)(void);
    rtk_api_ret_t   (*switch_portMaxPktLen_set)(rtk_port_t, rtk_switch_maxPktLen_linkSpeed_t, rtk_uint32);
    rtk_api_ret_t   (*switch_portMaxPktLen_get)(rtk_port_t, rtk_switch_maxPktLen_linkSpeed_t, rtk_uint32 *);
    rtk_api_ret_t   (*switch_maxPktLenCfg_set)(rtk_uint32, rtk_uint32);
    rtk_api_ret_t   (*switch_maxPktLenCfg_get)(rtk_uint32, rtk_uint32 *);
    rtk_api_ret_t   (*switch_greenEthernet_set)(rtk_enable_t);
    rtk_api_ret_t   (*switch_greenEthernet_get)(rtk_enable_t *);

    rtk_api_ret_t   (*fMdrv_miim_mmd_read)(rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 *);
    rtk_api_ret_t   (*fMdrv_miim_mmd_write)(rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 );
    rtk_api_ret_t   (*fMdrv_miim_mmd_readbits)(rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32,rtk_uint32 *);
    rtk_api_ret_t   (*fMdrv_miim_mmd_writebits)(rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 ,rtk_uint32);

    rtk_api_ret_t  (*fw_reset_flow_tgr_tgx)(rtk_uint32);
    rtk_api_ret_t  (*fw_reset_flow_tgr)(rtk_uint32);
    rtk_api_ret_t  (*fw_reset_flow_tgx)(rtk_uint32);
    rtk_api_ret_t  (*fw_reset_flow_tgr_8224)(rtk_uint32);
    rtk_api_ret_t  (*fw_reset_flow_8221B)(rtk_uint32);
    rtk_api_ret_t  (*rtl8224_top_reg_write)(rtk_uint32,  rtk_uint32);
    rtk_api_ret_t  (*rtl8224_top_reg_read)(rtk_uint32 ,  rtk_uint32* );
    rtk_api_ret_t  (*rtl8224_top_regbit_write)(rtk_uint32 ,  rtk_uint32 offset, rtk_uint32 );
    rtk_api_ret_t  (*rtl8224_top_regbit_read)(rtk_uint32 ,  rtk_uint32 offset, rtk_uint32* );
    rtk_api_ret_t  (*rtl8224_top_regbits_write)(rtk_uint32 ,  rtk_uint32 , rtk_uint32 );
    rtk_api_ret_t  (*rtl8224_top_regbits_read)(rtk_uint32 ,  rtk_uint32 , rtk_uint32* );
    rtk_api_ret_t  (*rtl8373_sds_reg_write)(rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 );
    rtk_api_ret_t  (*rtl8373_sds_reg_read)(rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 * );
    rtk_api_ret_t  (*rtl8373_sds_regbits_write)(rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 );
    rtk_api_ret_t  (*rtl8373_sds_regbits_read)(rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 * );
    rtk_api_ret_t  (*rtl8224_sds_reg_write)(rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 );
    rtk_api_ret_t  (*rtl8224_sds_reg_read)(rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 * );
    rtk_api_ret_t  (*rtl8224_sds_regbits_write)(rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 );
    rtk_api_ret_t  (*rtl8224_sds_regbits_read)(rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 * );
    rtk_api_ret_t  (*rtl8373_phy_regbits_write)(rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 );
    rtk_api_ret_t  (*rtl8373_phy_regbits_read)(rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 , rtk_uint32 * );
    ret_t (*rtl8373_setAsicRegBit)(rtk_uint32, rtk_uint32, rtk_uint32);
    ret_t (*rtl8373_getAsicRegBit)(rtk_uint32, rtk_uint32, rtk_uint32 *);
    ret_t (*rtl8373_setAsicRegBits)(rtk_uint32, rtk_uint32, rtk_uint32);
    ret_t (*rtl8373_getAsicRegBits)(rtk_uint32, rtk_uint32, rtk_uint32 *);
    ret_t (*rtl8373_setAsicReg)(rtk_uint32, rtk_uint32);
    ret_t (*rtl8373_getAsicReg)(rtk_uint32, rtk_uint32 *);
    /* cpuTag */
    rtk_api_ret_t (*cpuTag_externalCpuPort_set)(rtk_uint32);
    rtk_api_ret_t (*cpuTag_externalCpuPort_get)(rtk_uint32 *);
    rtk_api_ret_t (*cpuTag_tpid_set)(rtk_uint32);
    rtk_api_ret_t (*cpuTag_tpid_get)(rtk_uint32 *);
    rtk_api_ret_t (*cpuTag_enable_set)(rtk_cpu_type_t, rtk_enable_t);
    rtk_api_ret_t (*cpuTag_enable_get)(rtk_cpu_type_t, rtk_enable_t *);
    rtk_api_ret_t (*cpuTag_insertMode_set)(rtk_cpu_type_t,rtk_cpuTag_insertMode_t );
    rtk_api_ret_t (*cpuTag_insertMode_get)(rtk_cpu_type_t,rtk_cpuTag_insertMode_t *);
    rtk_api_ret_t (*cpuTag_awarePort_set)(rtk_portmask_t *);
    rtk_api_ret_t (*cpuTag_awarePort_get)(rtk_portmask_t *);
    rtk_api_ret_t (*cpuTag_priRemap_set)(rtk_cpu_type_t, rtk_pri_t, rtk_pri_t);
    rtk_api_ret_t (*cpuTag_priRemap_get)(rtk_cpu_type_t, rtk_pri_t, rtk_pri_t *);

    /* acl */
    rtk_api_ret_t (*filter_igrAcl_init)(void);
    rtk_api_ret_t (*filter_igrAcl_field_add)(rtk_filter_cfg_t *, rtk_filter_field_t *);
    rtk_api_ret_t (*filter_igrAcl_cfg_add)(rtk_filter_id_t, rtk_filter_cfg_t *, rtk_filter_action_t *, rtk_filter_number_t *);
    rtk_api_ret_t (*filter_igrAcl_cfg_del)(rtk_filter_id_t);
    rtk_api_ret_t (*filter_igrAcl_cfg_delAll)(void);
    rtk_api_ret_t (*filter_igrAcl_cfg_get)(rtk_filter_id_t, rtk_filter_cfg_raw_t *, rtk_filter_action_t *);
    rtk_api_ret_t (*filter_igrAcl_unmatchAction_set)(rtk_port_t, rtk_filter_unmatch_action_t);
    rtk_api_ret_t (*filter_igrAcl_unmatchAction_get)(rtk_port_t, rtk_filter_unmatch_action_t *);
    rtk_api_ret_t (*filter_igrAcl_state_set)(rtk_port_t, rtk_filter_state_t);
    rtk_api_ret_t (*filter_igrAcl_state_get)(rtk_port_t, rtk_filter_state_t *);
    rtk_api_ret_t (*filter_igrAcl_template_set)(rtk_filter_template_t *);
    rtk_api_ret_t (*filter_igrAcl_template_get)(rtk_filter_template_t *);
    rtk_api_ret_t (*filter_igrAcl_field_sel_set)(rtk_uint32, rtk_field_sel_t, rtk_uint32);
    rtk_api_ret_t (*filter_igrAcl_field_sel_get)(rtk_uint32, rtk_field_sel_t *, rtk_uint32 *);
    rtk_api_ret_t (*filter_iprange_set)(rtk_uint32, rtk_filter_iprange_t, ipaddr_t, ipaddr_t);
    rtk_api_ret_t (*filter_iprange_get)(rtk_uint32, rtk_filter_iprange_t *, ipaddr_t *, ipaddr_t *);
    rtk_api_ret_t (*filter_vidrange_set)(rtk_uint32, rtk_filter_vidrange_t, rtk_uint32, rtk_uint32);
    rtk_api_ret_t (*filter_vidrange_get)(rtk_uint32, rtk_filter_vidrange_t *, rtk_uint32 *, rtk_uint32 *);
    rtk_api_ret_t (*filter_portrange_set)(rtk_uint32, rtk_filter_portrange_t, rtk_uint32, rtk_uint32);
    rtk_api_ret_t (*filter_portrange_get)(rtk_uint32, rtk_filter_portrange_t *, rtk_uint32 *, rtk_uint32 *);
    rtk_api_ret_t (*filter_igrAcl_gpioPolarity_set)(rtk_uint32);
    rtk_api_ret_t (*filter_igrAcl_gpioPolarity_get)(rtk_uint32 *);
    rtk_api_ret_t (*filter_igrAcl_gpioEn_set)(rtk_uint32, rtk_enable_t);
    rtk_api_ret_t (*filter_igrAcl_gpioEn_get)(rtk_uint32, rtk_enable_t *);
    rtk_api_ret_t (*filter_igrAcl_tbl_rst)(void);

    /* mirror */
    rtk_api_ret_t (*mirror_set_en)(rtk_enable_t );
    rtk_api_ret_t (*mirror_setStatus_get)(rtk_enable_t *);
    rtk_api_ret_t (*mirror_entry_set)(rtk_port_mir_set_t *);
    rtk_api_ret_t (*mirror_entry_get)(rtk_port_mir_set_t *);
    rtk_api_ret_t (*mirror_portIso_set)(rtk_enable_t);
    rtk_api_ret_t (*mirror_portIso_get)(rtk_enable_t *);
    rtk_api_ret_t (*mirror_vlanLeaky_set)(rtk_enable_t , rtk_enable_t);
    rtk_api_ret_t (*mirror_vlanLeaky_get)(rtk_enable_t *, rtk_enable_t *);
    rtk_api_ret_t (*mirror_isolationLeaky_set)(rtk_enable_t, rtk_enable_t );
    rtk_api_ret_t (*mirror_isolationLeaky_get)(rtk_enable_t *, rtk_enable_t *);
    rtk_api_ret_t (*mirror_keep_set)(rtk_mirror_keep_t);
    rtk_api_ret_t (*mirror_keep_get)(rtk_mirror_keep_t *);
    rtk_api_ret_t (*mirror_override_set)(rtk_enable_t, rtk_enable_t, rtk_enable_t);
    rtk_api_ret_t (*mirror_override_get)(rtk_enable_t *, rtk_enable_t *, rtk_enable_t *);
    rtk_api_ret_t (*mirror_sampleRate_set)(rtk_uint32);
    rtk_api_ret_t (*mirror_sampleRate_get)(rtk_uint32 *);
    rtk_api_ret_t (*mirror_pktCnt_get)(rtk_uint32 *);
    rtk_api_ret_t (*mirror_samplePktCnt_get)(rtk_uint32 *);
    rtk_api_ret_t (*rspan_rxTag_en)(rtk_enable_t);
    rtk_api_ret_t (*rspan_rxTagEnSts_get)(rtk_enable_t *);
    rtk_api_ret_t (*rspan_tagCtxt_set)(rtk_rspan_tag_t *);
    rtk_api_ret_t (*rspan_tagCtxt_get)(rtk_rspan_tag_t *);
    rtk_api_ret_t (*rspan_tagAdd_set)(rtk_portmask_t);
    rtk_api_ret_t (*rspan_tagAdd_get)(rtk_portmask_t *);
    rtk_api_ret_t (*rspan_tagRemove_set)(rtk_enable_t);
    rtk_api_ret_t (*rspan_tagRemove_get)(rtk_enable_t *);

    rtk_api_ret_t (*port_isolation_set)(rtk_port_t, rtk_uint32 );
    rtk_api_ret_t (*port_isolation_get)(rtk_port_t, rtk_uint32 *);

    /* IGMP */
    rtk_api_ret_t (*igmp_init)(void);
    rtk_api_ret_t (*igmp_state_set)(rtk_enable_t);
    rtk_api_ret_t (*igmp_state_get)(rtk_enable_t *);
    rtk_api_ret_t (*igmp_static_router_port_set)(rtk_portmask_t *);
    rtk_api_ret_t (*igmp_static_router_port_get)(rtk_portmask_t *);
    rtk_api_ret_t (*igmp_protocol_set)(rtk_port_t, rtk_igmp_protocol_t, rtk_igmp_action_t);
    rtk_api_ret_t (*igmp_protocol_get)(rtk_port_t, rtk_igmp_protocol_t, rtk_igmp_action_t *);
    rtk_api_ret_t (*igmp_fastLeave_set)(rtk_enable_t);
    rtk_api_ret_t (*igmp_fastLeave_get)(rtk_enable_t *);
    rtk_api_ret_t (*igmp_maxGroup_set)(rtk_port_t, rtk_uint32);
    rtk_api_ret_t (*igmp_maxGroup_get)(rtk_port_t, rtk_uint32 *);
    rtk_api_ret_t (*igmp_currentGroup_get)(rtk_port_t, rtk_uint32 *);
    rtk_api_ret_t (*igmp_tableFullAction_set)(rtk_igmp_tableFullAction_t);
    rtk_api_ret_t (*igmp_tableFullAction_get)(rtk_igmp_tableFullAction_t *);
    rtk_api_ret_t (*igmp_checksumErrorAction_set)(rtk_igmp_checksumErrorAction_t);
    rtk_api_ret_t (*igmp_checksumErrorAction_get)(rtk_igmp_checksumErrorAction_t *);
    rtk_api_ret_t (*igmp_leaveTimer_set)(rtk_uint32);
    rtk_api_ret_t (*igmp_leaveTimer_get)(rtk_uint32 *);
    rtk_api_ret_t (*igmp_queryInterval_set)(rtk_uint32);
    rtk_api_ret_t (*igmp_queryInterval_get)(rtk_uint32 *);
    rtk_api_ret_t (*igmp_robustness_set)(rtk_uint32);
    rtk_api_ret_t (*igmp_robustness_get)(rtk_uint32 *);
    rtk_api_ret_t (*igmp_dynamicRouterPortAllow_set)(rtk_portmask_t *);
    rtk_api_ret_t (*igmp_dynamicRouterPortAllow_get)(rtk_portmask_t *);
    rtk_api_ret_t (*igmp_dynamicRouterPort_get)(rtk_igmp_dynamicRouterPort_t *);
    rtk_api_ret_t (*igmp_suppressionEnable_set)(rtk_enable_t, rtk_enable_t);
    rtk_api_ret_t (*igmp_suppressionEnable_get)(rtk_enable_t *, rtk_enable_t *);
    rtk_api_ret_t (*igmp_portRxPktEnable_set)(rtk_port_t, rtk_igmp_rxPktEnable_t *);
    rtk_api_ret_t (*igmp_portRxPktEnable_get)(rtk_port_t, rtk_igmp_rxPktEnable_t *);
    rtk_api_ret_t (*igmp_groupInfo_get)(rtk_uint32, rtk_igmp_groupInfo_t *);
    rtk_api_ret_t (*igmp_ReportLeaveFwdAction_set)(rtk_igmp_ReportLeaveFwdAct_t);
    rtk_api_ret_t (*igmp_ReportLeaveFwdAction_get)(rtk_igmp_ReportLeaveFwdAct_t *);
    rtk_api_ret_t (*igmp_dropLeaveZeroEnable_set)(rtk_enable_t);
    rtk_api_ret_t (*igmp_dropLeaveZeroEnable_get)(rtk_enable_t *);
    rtk_api_ret_t (*igmp_bypassGroupRange_set)(rtk_igmp_bypassGroup_t, rtk_enable_t);
    rtk_api_ret_t (*igmp_bypassGroupRange_get)(rtk_igmp_bypassGroup_t, rtk_enable_t *);

    /* Storm */
    rtk_api_ret_t (*rate_stormControlMeterIdx_set)(rtk_port_t, rtk_rate_storm_group_t, rtk_uint32);
    rtk_api_ret_t (*rate_stormControlMeterIdx_get)(rtk_port_t, rtk_rate_storm_group_t, rtk_uint32 *);
    rtk_api_ret_t (*rate_stormControlPortEnable_set)(rtk_port_t, rtk_rate_storm_group_t, rtk_enable_t);
    rtk_api_ret_t (*rate_stormControlPortEnable_get)(rtk_port_t, rtk_rate_storm_group_t, rtk_enable_t *);
    rtk_api_ret_t (*storm_bypass_set)(rtk_storm_bypass_t, rtk_enable_t);
    rtk_api_ret_t (*storm_bypass_get)(rtk_storm_bypass_t, rtk_enable_t *);
    rtk_api_ret_t (*rate_stormControlExtPortmask_set)(rtk_uint32);
    rtk_api_ret_t (*rate_stormControlExtPortmask_get)(rtk_uint32 *);
    rtk_api_ret_t (*rate_stormControlExtEnable_set)(rtk_rate_storm_group_t, rtk_enable_t);
    rtk_api_ret_t (*rate_stormControlExtEnable_get)(rtk_rate_storm_group_t, rtk_enable_t *);
    rtk_api_ret_t (*rate_stormControlExtMeterIdx_set)(rtk_rate_storm_group_t, rtk_uint32);
    rtk_api_ret_t (*rate_stormControlExtMeterIdx_get)(rtk_rate_storm_group_t, rtk_uint32 *);

    /* Rate */
    rtk_api_ret_t (*rate_shareMeter_set)(rtk_meter_id_t, rtk_meter_type_t, rtk_rate_t, rtk_enable_t);
    rtk_api_ret_t (*rate_shareMeter_get)(rtk_meter_id_t, rtk_meter_type_t *, rtk_rate_t *, rtk_enable_t *);
    rtk_api_ret_t (*rate_shareMeterBucket_set)(rtk_meter_id_t, rtk_uint32);
    rtk_api_ret_t (*rate_shareMeterBucket_get)(rtk_meter_id_t, rtk_uint32 *);
    rtk_api_ret_t (*rate_shareMeterExceedStatus_set)(rtk_uint32);
    rtk_api_ret_t (*rate_shareMeterExceedStatus_get)(rtk_uint32, rtk_uint32 *);
    rtk_api_ret_t (*rate_shareMeterICPUExceedStatus_set)(rtk_uint32);
    rtk_api_ret_t (*rate_shareMeterICPUExceedStatus_get)(rtk_uint32, rtk_uint32 *);

    /*QoS*/
    rtk_api_ret_t (*qos_init)(void);
    rtk_api_ret_t (*qos_priSel_set)(rtk_qos_priDecTbl_t, rtk_priority_select_t *);
    rtk_api_ret_t (*qos_priSel_get)(rtk_qos_priDecTbl_t, rtk_priority_select_t *);
    rtk_api_ret_t (*qos_1pPriRemap_set)(rtk_pri_t, rtk_pri_t);
    rtk_api_ret_t (*qos_1pPriRemap_get)(rtk_pri_t, rtk_pri_t *);
    rtk_api_ret_t (*qos_1pRemarkSrcSel_set)(rtk_qos_1pRmkSrc_t );
    rtk_api_ret_t (*qos_1pRemarkSrcSel_get)(rtk_qos_1pRmkSrc_t *);
    rtk_api_ret_t (*qos_dscpPriRemap_set)(rtk_dscp_t, rtk_pri_t );
    rtk_api_ret_t (*qos_dscpPriRemap_get)(rtk_dscp_t, rtk_pri_t *);
    rtk_api_ret_t (*qos_rspanpriRemap_set)(rtk_pri_t, rtk_pri_t);
    rtk_api_ret_t (*qos_rspanpriRemap_get)(rtk_pri_t, rtk_pri_t *);
    rtk_api_ret_t (*qos_portPri_set)(rtk_port_t, rtk_pri_t ) ;
    rtk_api_ret_t (*qos_portPri_get)(rtk_port_t, rtk_pri_t *) ;
    rtk_api_ret_t (*qos_priMap_set)(rtk_port_t, rtk_qos_pri2queue_t *);
    rtk_api_ret_t (*qos_priMap_get)(rtk_port_t, rtk_qos_pri2queue_t *);
    rtk_api_ret_t (*qos_schedulingQueue_set)(rtk_port_t, rtk_qos_queue_weights_t *);
    rtk_api_ret_t (*qos_schedulingQueue_get)(rtk_port_t, rtk_qos_queue_weights_t *);
    rtk_api_ret_t (*qos_1pRemarkEnable_set)(rtk_port_t, rtk_enable_t);
    rtk_api_ret_t (*qos_1pRemarkEnable_get)(rtk_port_t, rtk_enable_t *);
    rtk_api_ret_t (*qos_1pRemark_set)(rtk_pri_t, rtk_pri_t);
    rtk_api_ret_t (*qos_1pRemark_get)(rtk_pri_t, rtk_pri_t *);
    rtk_api_ret_t (*qos_dscpRemarkEnable_set)(rtk_port_t, rtk_enable_t);
    rtk_api_ret_t (*qos_dscpRemarkEnable_get)(rtk_port_t, rtk_enable_t *);
    rtk_api_ret_t (*qos_intpri2dscpRemark_set)(rtk_pri_t , rtk_dscp_t );
    rtk_api_ret_t (*qos_intpri2dscpRemark_get)(rtk_pri_t, rtk_dscp_t *);
    rtk_api_ret_t (*qos_dscp2dscpRemark_set)(rtk_pri_t , rtk_dscp_t );
    rtk_api_ret_t (*qos_dscp2dscpRemark_get)(rtk_pri_t, rtk_dscp_t *);
    rtk_api_ret_t (*qos_dscpRemarkSrcSel_set)(rtk_qos_dscpRmkSrc_t);
    rtk_api_ret_t (*qos_dscpRemarkSrcSel_get)(rtk_qos_dscpRmkSrc_t *);
    rtk_api_ret_t (*qos_portPriSelIndex_set)(rtk_port_t, rtk_qos_priDecTbl_t);
    rtk_api_ret_t (*qos_portPriSelIndex_get)(rtk_port_t, rtk_qos_priDecTbl_t *);
    rtk_api_ret_t (*qos_schedulingType_set)(rtk_port_t, rtk_qos_scheduling_type_t);
    rtk_api_ret_t (*qos_schedulingType_get)(rtk_port_t, rtk_qos_scheduling_type_t *);

    /*VLAN*/
    rtk_api_ret_t (*vlan_init)(void);
    rtk_api_ret_t (*vlan_set)(rtk_vlan_t, rtk_vlan_entry_t *);
    rtk_api_ret_t (*vlan_get)(rtk_vlan_t, rtk_vlan_entry_t *);
    rtk_api_ret_t (*vlan_egrFilterEnable_set)(rtk_enable_t);
    rtk_api_ret_t (*vlan_egrFilterEnable_get)(rtk_enable_t *);
    rtk_api_ret_t (*vlan_portPvid_set)(rtk_port_t, rtk_vlan_t);
    rtk_api_ret_t (*vlan_portPvid_get)(rtk_port_t, rtk_vlan_t *);
    rtk_api_ret_t (*vlan_portIgrFilterEnable_set)(rtk_port_t, rtk_enable_t);
    rtk_api_ret_t (*vlan_portIgrFilterEnable_get)(rtk_port_t, rtk_enable_t *);
    rtk_api_ret_t (*vlan_portAcceptFrameType_set)(rtk_port_t, rtk_vlan_acceptFrameType_t);
    rtk_api_ret_t (*vlan_portAcceptFrameType_get)(rtk_port_t, rtk_vlan_acceptFrameType_t *);
    rtk_api_ret_t (*vlan_tagMode_set)(rtk_port_t, rtk_vlan_egressTagMode_t);
    rtk_api_ret_t (*vlan_tagMode_get)(rtk_port_t, rtk_vlan_egressTagMode_t *);
    rtk_api_ret_t (*vlan_transparent_set)(rtk_port_t, rtk_portmask_t *);
    rtk_api_ret_t (*vlan_transparent_get)(rtk_port_t , rtk_portmask_t *);
    rtk_api_ret_t (*vlan_keep_set)(rtk_port_t, rtk_portmask_t *);
    rtk_api_ret_t (*vlan_keep_get)(rtk_port_t, rtk_portmask_t *);
    rtk_api_ret_t (*vlan_stg_set)(rtk_vlan_t, rtk_stp_msti_id_t);
    rtk_api_ret_t (*vlan_stg_get)(rtk_vlan_t, rtk_stp_msti_id_t *);
    rtk_api_ret_t (*vlan_portFid_set)(rtk_port_t port, rtk_enable_t, rtk_fid_t);
    rtk_api_ret_t (*vlan_portFid_get)(rtk_port_t port, rtk_enable_t *, rtk_fid_t *);
    rtk_api_ret_t (*vlan_reservedVidAction_set)(rtk_vlan_resVidAction_t, rtk_vlan_resVidAction_t);
    rtk_api_ret_t (*vlan_reservedVidAction_get)(rtk_vlan_resVidAction_t *, rtk_vlan_resVidAction_t *);
    rtk_api_ret_t (*vlan_realKeepRemarkEnable_set)(rtk_enable_t );
    rtk_api_ret_t (*vlan_realKeepRemarkEnable_get)(rtk_enable_t *);
    rtk_api_ret_t (*vlan_disL2Learn_entry_set)(rtk_vlan_disL2_learn_t *);
    rtk_api_ret_t (*vlan_disL2Learn_entry_get)(rtk_uint32, rtk_vlan_disL2_learn_t *);
    rtk_api_ret_t (*vlan_reset)(void);

    /*dot1x*/
    rtk_api_ret_t (*dot1x_unauthPacketOper_set)(rtk_port_t, rtk_dot1x_unauth_action_t);
    rtk_api_ret_t (*dot1x_unauthPacketOper_get)(rtk_port_t, rtk_dot1x_unauth_action_t *);
   // rtk_api_ret_t (*dot1x_eapolFrame2CpuEnable_set)(rtk_enable_t);
    //rtk_api_ret_t (*dot1x_eapolFrame2CpuEnable_get)(rtk_enable_t *);
    rtk_api_ret_t (*dot1x_trap2CPU_Sel_set)(rtk_dot1x_cpu_select_t );
    rtk_api_ret_t (*dot1x_trap2CPU_Sel_get)(rtk_dot1x_cpu_select_t *);
    rtk_api_ret_t (*dot1x_trap_priority_set)(rtk_pri_t );
    rtk_api_ret_t (*dot1x_trap_priority_get)(rtk_pri_t *);
    rtk_api_ret_t (*dot1x_portBasedEnable_set)(rtk_port_t , rtk_enable_t);
    rtk_api_ret_t (*dot1x_portBasedEnable_get)(rtk_port_t , rtk_enable_t *);
    rtk_api_ret_t (*dot1x_portBasedAuthStatus_set)(rtk_port_t, rtk_dot1x_auth_status_t);
    rtk_api_ret_t (*dot1x_portBasedAuthStatus_get)(rtk_port_t, rtk_dot1x_auth_status_t *);
    rtk_api_ret_t (*dot1x_portBasedDirection_set)(rtk_port_t, rtk_dot1x_direction_t);
    rtk_api_ret_t (*dot1x_portBasedDirection_get)(rtk_port_t, rtk_dot1x_direction_t *);
    rtk_api_ret_t (*dot1x_macBasedEnable_set)(rtk_port_t, rtk_enable_t);
    rtk_api_ret_t (*dot1x_macBasedEnable_get)(rtk_port_t , rtk_enable_t *);
    rtk_api_ret_t (*dot1x_macBasedAuthMac_add)(rtk_port_t, rtk_mac_t *, rtk_fid_t);
    rtk_api_ret_t (*dot1x_macBasedAuthMac_del)(rtk_port_t, rtk_mac_t *, rtk_fid_t);
    rtk_api_ret_t (*dot1x_macBasedDirection_set)(rtk_dot1x_direction_t);
    rtk_api_ret_t (*dot1x_macBasedDirection_get)(rtk_dot1x_direction_t *);
    rtk_api_ret_t (*dot1x_guestVlan_set)(rtk_vlan_t );
    rtk_api_ret_t (*dot1x_guestVlan_get)(rtk_vlan_t *);
    rtk_api_ret_t (*dot1x_guestVlan2Auth_set)(rtk_enable_t);
    rtk_api_ret_t (*dot1x_guestVlan2Auth_get)(rtk_enable_t *);

    /*MACsec*/
    rtk_api_ret_t (*macsec_enable_set)(rtk_uint32, rtk_uint32, rtk_uint32);
    rtk_api_ret_t(*macsec_enable_get)(rtk_uint32, rtk_uint32 *, rtk_uint32 *);
    rtk_api_ret_t(*macsec_egress_set)(rtk_uint32 , rtk_uint32 , rtk_uint32);
    rtk_api_ret_t(*macsec_egress_get)(rtk_uint32 , rtk_uint32 , rtk_uint32 *);
    rtk_api_ret_t(*macsec_ingress_set)(rtk_uint32 , rtk_uint32 , rtk_uint32);
    rtk_api_ret_t(*macsec_ingress_get)(rtk_uint32 , rtk_uint32 , rtk_uint32 *);
    rtk_api_ret_t(*macsec_rxgating_set)(rtk_uint32);
    rtk_api_ret_t(*macsec_rxgating_cancel)(rtk_uint32);
    rtk_api_ret_t(*macsec_txgating_set)(rtk_uint32);
    rtk_api_ret_t(*macsec_txgating_cancel)(rtk_uint32);
    rtk_api_ret_t(*macsec_rxIPbypass_set)(rtk_uint32, rtk_uint32);
    rtk_api_ret_t(*macsec_rxIPbypass_get)(rtk_uint32, rtk_uint32 *);
    rtk_api_ret_t(*macsec_txIPbypass_set)(rtk_uint32, rtk_uint32);
    rtk_api_ret_t(*macsec_txIPbypass_get)(rtk_uint32, rtk_uint32 *);
    rtk_api_ret_t(*macsec_rxbypass_set)(rtk_uint32, rtk_uint32);
    rtk_api_ret_t(*macsec_rxbypass_get)(rtk_uint32, rtk_uint32 *);
    rtk_api_ret_t(*macsec_txbypass_set)(rtk_uint32, rtk_uint32);
    rtk_api_ret_t(*macsec_txbypass_get)(rtk_uint32, rtk_uint32 *);
    rtk_api_ret_t(*wrapper_int_control_set)(rtk_uint32, rtk_macsec_int_type_t, rtk_enable_t);
    rtk_api_ret_t(*wrapper_int_control_get)(rtk_uint32, rtk_macsec_int_type_t, rtk_enable_t *);
    rtk_api_ret_t(*wrapper_int_status_set)(rtk_uint32, rtk_uint32);
    rtk_api_ret_t(*wrapper_int_status_get)(rtk_uint32, rtk_uint32 *);
    rtk_api_ret_t(*wrapper_mib_reset)(rtk_uint32, rtk_uint32);
    rtk_api_ret_t(*wrapper_mib_counter)(rtk_uint32, RTL8373_WRAPPER_MIBCOUNTER, rtk_uint64 *);
    rtk_api_ret_t(*macsec_ipg_len_set)(rtk_uint32, rtk_uint32);
    rtk_api_ret_t(*macsec_ipg_len_get)(rtk_uint32, rtk_uint32 *);
    rtk_api_ret_t(*macsec_ipg_mode_set)(rtk_uint32, rtk_uint32);
    rtk_api_ret_t(*macsec_ipg_mode_get)(rtk_uint32, rtk_uint32 *);
    rtk_api_ret_t(*macsec_eth_set)(rtk_uint32, rtk_uint32, rtk_uint32);
    rtk_api_ret_t(*macsec_eth_get)(rtk_uint32, rtk_uint32, rtk_uint32 *);
    rtk_api_ret_t(*macsec_init)(rtk_uint32);

    /*SVLAN*/
    rtk_api_ret_t (*svlan_init)(void);
    rtk_api_ret_t (*svlan_servicePort_add)(rtk_port_t );
    rtk_api_ret_t (*svlan_servicePort_get)(rtk_portmask_t *);
    rtk_api_ret_t (*svlan_servicePort_del)(rtk_port_t);
    rtk_api_ret_t (*svlan_tpidEntry_set)(rtk_uint32);
    rtk_api_ret_t (*svlan_tpidEntry_get)(rtk_uint32 *);
    rtk_api_ret_t (*svlan_priorityRef_set)(rtk_svlan_pri_ref_t);
    rtk_api_ret_t (*svlan_priorityRef_get)(rtk_svlan_pri_ref_t *);
    //rtk_api_ret_t (*svlan_memberPortEntry_set)(rtk_uint32, rtk_svlan_memberCfg_t *);
    //rtk_api_ret_t (*svlan_memberPortEntry_get)(rtk_uint32, rtk_svlan_memberCfg_t *);
    //rtk_api_ret_t (*svlan_memberPortEntry_adv_set)(rtk_uint32, rtk_svlan_memberCfg_t *);
    //rtk_api_ret_t (*svlan_memberPortEntry_adv_get)(rtk_uint32, rtk_svlan_memberCfg_t *);
    rtk_api_ret_t (*svlan_defaultSvlan_set)(rtk_port_t, rtk_vlan_t);
    rtk_api_ret_t (*svlan_defaultSvlan_get)(rtk_port_t, rtk_vlan_t *);
    rtk_api_ret_t (*svlan_c2s_add)(rtk_vlan_t, rtk_port_t, rtk_vlan_t);
    rtk_api_ret_t (*svlan_c2s_del)(rtk_vlan_t, rtk_port_t);
    rtk_api_ret_t (*svlan_c2s_get)(rtk_vlan_t, rtk_port_t, rtk_vlan_t *);
    rtk_api_ret_t (*svlan_untag_action_set)(rtk_svlan_untag_action_t, rtk_vlan_t);
    rtk_api_ret_t (*svlan_untag_action_get)(rtk_svlan_untag_action_t *, rtk_vlan_t *);
    rtk_api_ret_t (*svlan_trapPri_set)(rtk_pri_t);
    rtk_api_ret_t (*svlan_trapPri_get)(rtk_pri_t *);
    rtk_api_ret_t (*svlan_unassign_action_set)(rtk_svlan_unassign_action_t);
    rtk_api_ret_t (*svlan_unassign_action_get)(rtk_svlan_unassign_action_t *);
    rtk_api_ret_t (*svlan_trapCpuMsk_set)(rtk_uint32);
    rtk_api_ret_t (*svlan_trapCpuMsk_get)(rtk_uint32 *);

    /*NIC*/
    rtk_api_ret_t (*nic_rst_set)(void);
    rtk_api_ret_t (*nic_txStopAddr_set)( rtk_uint32 );
    rtk_api_ret_t (*nic_txStopAddr_get)(rtk_uint32 *);
    rtk_api_ret_t (*nic_rxStopAddr_set)( rtk_uint32 );
    rtk_api_ret_t (*nic_rxStopAddr_get)(rtk_uint32 *);
    rtk_api_ret_t (*nic_swRxCurPktAddr_get)(rtk_uint32 *);
    rtk_api_ret_t (*nic_rxReceivedPktLen_get)(rtk_uint32 *);
    rtk_api_ret_t (*nic_txAvailSpace_get)(rtk_uint32 *);
    rtk_api_ret_t (*nic_moduleEn_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_moduleEn_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_rxEn_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_rxEn_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_txEn_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_txEn_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_rxRemoveCrc_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_rxRemoveCrc_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_rxPaddingEn_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_rxPaddingEn_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_rxFreeSpaceThd_set)(rtk_uint32 );
    rtk_api_ret_t (*nic_rxFreeSpaceThd_get)(rtk_uint32 *);
    rtk_api_ret_t (*nic_rxCrcErrEn_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_rxCrcErrEn_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_rxL3CrcErrEn_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_rxL3CrcErrEn_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_rxL4CrcErrEn_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_rxL4CrcErrEn_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_rxArpEn_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_rxArpEn_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_rxAllPktEn_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_rxAllPktEn_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_rxPhyPktSel_set)(rtk_nic_rxpps_t );
    rtk_api_ret_t (*nic_rxPhyPktSel_get)(rtk_nic_rxpps_t *);
    rtk_api_ret_t (*nic_rxMultiPktEn_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_rxMultiPktEn_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_rxBcPktEn_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_rxBcPktEn_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_mcHashFltrEn_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_mcHashFltrEn_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_phyPktHashFltrEn_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_phyPktHashFltrEn_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_mcHashTblVal_set)(rtk_nic_hashValType_t, rtk_uint32 );
    rtk_api_ret_t (*nic_mcHashTblVal_get)(rtk_nic_hashValType_t, rtk_uint32 *);
    rtk_api_ret_t (*nic_phyPktHashTblVal_set)(rtk_nic_hashValType_t, rtk_uint32 );
    rtk_api_ret_t (*nic_phyPktHashTblVal_get)(rtk_nic_hashValType_t, rtk_uint32 *);
    rtk_api_ret_t (*nic_rxMTU_set)(rtk_nic_RxMTU_t );
    rtk_api_ret_t (*nic_rxMTU_get)(rtk_nic_RxMTU_t *);
    rtk_api_ret_t (*nic_loopbackEn_set)(rtk_enable_t );
    rtk_api_ret_t (*nic_loopbackEn_get)(rtk_enable_t *);
    rtk_api_ret_t (*nic_interruptEn_set)(rtk_enable_t, rtk_enable_t );
    rtk_api_ret_t (*nic_interruptEn_get)(rtk_enable_t *, rtk_enable_t *);
    rtk_api_ret_t (*nic_interruptStatus_get)(rtk_uint32 * , rtk_uint32 *);
    rtk_api_ret_t (*nic_interruptStatus_clear)(rtk_uint32 , rtk_uint32 );

 /* Time */
    rtk_api_ret_t   (*time_init)(rtk_portmask_t);
    rtk_api_ret_t   (*time_portPtpbypassptpEn_get)(rtk_port_t , rtk_enable_t *);
    rtk_api_ret_t   (*time_portPtpbypassptpEn_set)(rtk_port_t , rtk_enable_t );
    rtk_api_ret_t   (*time_portPtpEnable_get)(rtk_port_t ,rtk_ptp_header_t, rtk_enable_t *);
    rtk_api_ret_t   (*time_portPtpEnable_set)(rtk_port_t ,rtk_ptp_header_t,rtk_enable_t );
    rtk_api_ret_t   (*time_portPtpOper_triger)(void);
    
    rtk_api_ret_t   (*time_portRefTime_get)( rtk_time_timeStamp_t *);
    rtk_api_ret_t   (*time_portRefTime_set)( rtk_time_timeStamp_t ,rtk_enable_t);
    rtk_api_ret_t   (*time_portRefTimeAdjust_set)(  rtk_uint32 , rtk_time_timeStamp_t, rtk_enable_t apply);
    rtk_api_ret_t   (*time_portRefTimeEnable_get)(  rtk_enable_t *);
    rtk_api_ret_t   (*time_portRefTimeEnable_set)( rtk_enable_t );



   rtk_api_ret_t   (*time_portPtpVlanTpid_get)( rtk_vlanType_t , rtk_uint32 , rtk_uint32 *);
    rtk_api_ret_t   (*time_portPtpVlanTpid_set)(rtk_vlanType_t , rtk_uint32 , rtk_uint32 );
    rtk_api_ret_t   (*time_portPtpOper_get)(rtk_time_operCfg_t *);
    rtk_api_ret_t   (*time_portPtpOper_set)(  rtk_time_operCfg_t );
    rtk_api_ret_t   (*time_portPtpLatchTime_get)( rtk_time_timeStamp_t *);
    rtk_api_ret_t   (*time_portPtpRefTimeFreqCfg_get)(rtk_uint32 *, rtk_uint32 *);
    rtk_api_ret_t   (*time_portPtpRefTimeFreqCfg_set)(rtk_uint32, rtk_enable_t);
    rtk_api_ret_t   (*time_portPtpTxInterruptStatus_get)( rtk_uint32 *);
    rtk_api_ret_t   (*time_portPtpTxTimestampFifo_get)(rtk_time_txTimeEntry_t *);
    rtk_api_ret_t   (*time_portPtp1PPSOutput_get)(rtk_uint32 *, rtk_enable_t *);
    rtk_api_ret_t   (*time_portPtp1PPSOutput_set)( rtk_uint32, rtk_enable_t);
    rtk_api_ret_t   (*time_portPtpClockOutput_get)( rtk_time_clkOutput_t*);
    rtk_api_ret_t   (*time_portPtpClockOutput_set)( rtk_time_clkOutput_t );
    rtk_api_ret_t   (*time_portPtpClkSrcCtrl_get)( rtk_enable_t *);
    rtk_api_ret_t   (*time_portPtpClkSrcCtrl_set)( rtk_enable_t );
    rtk_api_ret_t   (*time_portPtpToddelay_get)( rtk_uint32 *);
    rtk_api_ret_t   (*time_portPtpToddelay_set)( rtk_uint32 );
    rtk_api_ret_t   (*time_portPtpOutputSigSel_get)( rtk_time_outSigSel_t *);
    rtk_api_ret_t   (*time_portPtpOutputSigSel_set)( rtk_time_outSigSel_t);
    rtk_api_ret_t   (*time_portPtpPortctrl_get)( rtk_port_t, rtk_ptp_port_ctrl_t*);
    rtk_api_ret_t   (*time_portPtpPortctrl_set)(rtk_port_t, rtk_ptp_port_ctrl_t);



         /*PTP*/
    rtk_api_ret_t (*ptp_intControl_set)(rtk_ptp_intType_t , rtk_enable_t);
    rtk_api_ret_t (*ptp_intControl_get)(rtk_ptp_intType_t,rtk_enable_t *);
    rtk_api_ret_t (*ptp_intStatus_get)( rtk_ptp_intStatus_t *);
    rtk_api_ret_t (*ptp_portTrap_set)(rtk_port_t,  rtk_ptp_porttrap_ctrl_t *);
    rtk_api_ret_t (*ptp_portTrap_get)(rtk_port_t, rtk_ptp_porttrap_ctrl_t *);
#if 1
    /*RLDP*/
    rtk_api_ret_t (*rldp_config_set)(rtk_rldp_config_t *);
    rtk_api_ret_t (*rldp_config_get)(rtk_rldp_config_t *);
    rtk_api_ret_t (*rldp_portConfig_set)(rtk_port_t, rtk_rldp_portConfig_t *);
    rtk_api_ret_t (*rldp_portConfig_get)(rtk_port_t, rtk_rldp_portConfig_t *);
    rtk_api_ret_t (*rldp_status_get)(rtk_rldp_status_t *);
    rtk_api_ret_t (*rldp_portStatus_get)(rtk_port_t, rtk_rldp_portStatus_t *);
    rtk_api_ret_t (*rldp_portStatus_set)(rtk_port_t, rtk_rldp_portStatus_t *);
    rtk_api_ret_t (*rldp_portLoopPair_get)(rtk_port_t, rtk_portmask_t *);
#endif
    /*trunk*/
    rtk_api_ret_t (*trunk_port_set)(rtk_trunk_group_t, rtk_portmask_t *);
    rtk_api_ret_t (*trunk_port_get)(rtk_trunk_group_t, rtk_portmask_t *);
    rtk_api_ret_t (*trunk_distributionAlgorithm_set)(rtk_trunk_group_t, rtk_uint32);
    rtk_api_ret_t (*trunk_distributionAlgorithm_get)(rtk_trunk_group_t, rtk_uint32 *);
    rtk_api_ret_t (*trunk_trafficSeparate_set)(rtk_trunk_group_t, rtk_trunk_separateType_t);
    rtk_api_ret_t (*trunk_trafficSeparate_get)(rtk_trunk_group_t, rtk_trunk_separateType_t *);
    rtk_api_ret_t (*trunk_mode_set)(rtk_trunk_mode_t);
    rtk_api_ret_t (*trunk_mode_get)(rtk_trunk_mode_t *);
    rtk_api_ret_t (*trunk_trafficPause_set)(rtk_trunk_group_t, rtk_enable_t);
    rtk_api_ret_t (*trunk_trafficPause_get)(rtk_trunk_group_t, rtk_enable_t *);
    rtk_api_ret_t (*trunk_hashMappingTable_set)(rtk_trunk_group_t, rtk_trunk_hashVal2Port_t *);
    rtk_api_ret_t (*trunk_hashMappingTable_get)(rtk_trunk_group_t, rtk_trunk_hashVal2Port_t *);
    rtk_api_ret_t (*trunk_portQueueEmpty_get)(rtk_portmask_t *);

	/* interrupt */
    rtk_api_ret_t (*int_enable)(rtk_enable_t);
    rtk_api_ret_t (*int_polarity_set)(rtk_int_polarity_t);
    rtk_api_ret_t (*int_polarity_get)(rtk_int_polarity_t *);
    rtk_api_ret_t (*int_control_set)(rtk_port_t, rtk_int_cpu_t, rtk_int_type_t, rtk_enable_t);
    rtk_api_ret_t (*int_control_get)(rtk_port_t, rtk_int_cpu_t, rtk_int_type_t, rtk_enable_t *);
    rtk_api_ret_t (*int_miscIMR_set)(rtk_uint32 , interrupt_misc_t , rtk_uint32 );
    rtk_api_ret_t (*int_miscIMR_get)(rtk_uint32 , interrupt_misc_t , rtk_uint32* );
    rtk_api_ret_t (*int_miscISR_clear)(rtk_uint32 , interrupt_misc_t );
    rtk_api_ret_t (*int_miscISR_get)(rtk_uint32 , interrupt_misc_t , rtk_uint32* );

	/*RMA*/
	rtk_api_ret_t (*rma_set)(rtk_uint32, rtk_rmaParam_t *);
    rtk_api_ret_t (*rma_get)(rtk_uint32, rtk_rmaParam_t *);

	/*led*/
	rtk_api_ret_t   (*led_blinkRate_set)(rtk_led_blink_rate_t);
    rtk_api_ret_t   (*led_blinkRate_get)(rtk_led_blink_rate_t *);
    rtk_api_ret_t   (*led_groupConfig_set)(rtk_led_set_t, rtk_uint32, rtk_led_config_t *);
    rtk_api_ret_t   (*led_portSelConfig_set)(rtk_port_t, rtk_led_set_t);
    rtk_api_ret_t   (*led_portSelConfig_get)(rtk_port_t, rtk_led_set_t * );
#if 0
    /*leaky*/
    rtk_api_ret_t (*leaky_vlan_set)(rtk_leaky_type_t, rtk_enable_t);
    rtk_api_ret_t (*leaky_vlan_get)(rtk_leaky_type_t, rtk_enable_t *);
    rtk_api_ret_t (*leaky_portIsolation_set)(rtk_leaky_type_t, rtk_enable_t);
    rtk_api_ret_t (*leaky_portIsolation_get)(rtk_leaky_type_t, rtk_enable_t *);

    /* led */
    rtk_api_ret_t   (*led_enable_set)(rtk_led_group_t, rtk_portmask_t *);
    rtk_api_ret_t   (*led_enable_get)(rtk_led_group_t, rtk_portmask_t *);
    rtk_api_ret_t   (*led_operation_set)(rtk_led_operation_t );
    rtk_api_ret_t   (*led_operation_get)(rtk_led_operation_t *);
    rtk_api_ret_t   (*led_modeForce_set)(rtk_port_t, rtk_led_group_t, rtk_led_force_mode_t);
    rtk_api_ret_t   (*led_modeForce_get)(rtk_port_t, rtk_led_group_t, rtk_led_force_mode_t *);
    rtk_api_ret_t   (*led_blinkRate_set)(rtk_led_blink_rate_t);
    rtk_api_ret_t   (*led_blinkRate_get)(rtk_led_blink_rate_t *);
    rtk_api_ret_t   (*led_groupConfig_set)(rtk_led_group_t, rtk_led_congig_t);
    rtk_api_ret_t   (*led_groupConfig_get)(rtk_led_group_t, rtk_led_congig_t *);
    rtk_api_ret_t   (*led_groupAbility_set)(rtk_led_group_t, rtk_led_ability_t *);
    rtk_api_ret_t   (*led_groupAbility_get)(rtk_led_group_t, rtk_led_ability_t *);
    rtk_api_ret_t   (*led_serialMode_set)(rtk_led_active_t);
    rtk_api_ret_t   (*led_serialMode_get)(rtk_led_active_t *);
    rtk_api_ret_t   (*led_OutputEnable_set)(rtk_enable_t);
    rtk_api_ret_t   (*led_OutputEnable_get)(rtk_enable_t *);
    rtk_api_ret_t   (*led_serialModePortmask_set)(rtk_led_serialOutput_t, rtk_portmask_t *);
    rtk_api_ret_t   (*led_serialModePortmask_get)(rtk_led_serialOutput_t *, rtk_portmask_t *);

    /* oam */
    rtk_api_ret_t (*oam_init)(void);
    rtk_api_ret_t (*oam_state_set)(rtk_enable_t);
    rtk_api_ret_t (*oam_state_get)(rtk_enable_t *);
    rtk_api_ret_t (*oam_parserAction_set)(rtk_port_t, rtk_oam_parser_act_t );
    rtk_api_ret_t (*oam_parserAction_get)(rtk_port_t, rtk_oam_parser_act_t *);
    rtk_api_ret_t (*oam_multiplexerAction_set)(rtk_port_t, rtk_oam_multiplexer_act_t );
    rtk_api_ret_t (*oam_multiplexerAction_get)(rtk_port_t, rtk_oam_multiplexer_act_t *);

    rtk_api_ret_t (*port_rgmiiDelayExt_set)(rtk_port_t, rtk_data_t, rtk_data_t);
    rtk_api_ret_t (*port_rgmiiDelayExt_get)(rtk_port_t, rtk_data_t *, rtk_data_t *);
    rtk_api_ret_t (*port_phyEnableAll_set)(rtk_enable_t);
    rtk_api_ret_t (*port_phyEnableAll_get)(rtk_enable_t *);
    rtk_api_ret_t (*port_efid_set)(rtk_port_t, rtk_data_t);
    rtk_api_ret_t (*port_efid_get)(rtk_port_t, rtk_data_t *);
    rtk_api_ret_t (*port_phyComboPortMedia_set)(rtk_port_t, rtk_port_media_t);
    rtk_api_ret_t (*port_phyComboPortMedia_get)(rtk_port_t, rtk_port_media_t *);
    rtk_api_ret_t (*port_rtctEnable_set)(rtk_portmask_t *);
    rtk_api_ret_t (*port_rtctDisable_set)(rtk_portmask_t *);
    rtk_api_ret_t (*port_rtctResult_get)(rtk_port_t, rtk_rtctResult_t *);
    rtk_api_ret_t (*port_sds_reset)(rtk_port_t);
    rtk_api_ret_t (*port_sgmiiLinkStatus_get)(rtk_port_t, rtk_data_t *, rtk_data_t *, rtk_port_linkStatus_t *);
    rtk_api_ret_t (*port_sgmiiNway_set)(rtk_port_t, rtk_enable_t);
    rtk_api_ret_t (*port_sgmiiNway_get)(rtk_port_t, rtk_enable_t *);
    rtk_api_ret_t (*port_fiberAbilityExt_set)(rtk_port_t, rtk_uint32, rtk_uint32);
    rtk_api_ret_t (*port_fiberAbilityExt_get)(rtk_port_t, rtk_uint32 *, rtk_uint32 *);
#endif
    
    rtk_api_ret_t (*port_autoDos_set)(rtk_port_autoDosType_t, rtk_enable_t);
    rtk_api_ret_t (*port_autoDos_get)(rtk_port_autoDosType_t, rtk_enable_t *);
#if 0
    /* stat */
    rtk_api_ret_t (*stat_global_reset)(void);
    rtk_api_ret_t (*stat_port_reset)(rtk_port_t);
    rtk_api_ret_t (*stat_queueManage_reset)(void);
    rtk_api_ret_t (*stat_global_get)(rtk_stat_global_type_t, rtk_stat_counter_t *);
    rtk_api_ret_t (*stat_global_getAll)(rtk_stat_global_cntr_t *);
    rtk_api_ret_t (*stat_port_get)(rtk_port_t, rtk_stat_port_type_t, rtk_stat_counter_t *);
    rtk_api_ret_t (*stat_port_getAll)(rtk_port_t, rtk_stat_port_cntr_t *);
    rtk_api_ret_t (*stat_logging_counterCfg_set)(rtk_uint32, rtk_logging_counter_mode_t, rtk_logging_counter_type_t);
    rtk_api_ret_t (*stat_logging_counterCfg_get)(rtk_uint32, rtk_logging_counter_mode_t *, rtk_logging_counter_type_t *);
    rtk_api_ret_t (*stat_logging_counter_reset)(rtk_uint32);
    rtk_api_ret_t (*stat_logging_counter_get)(rtk_uint32, rtk_uint32 *);
    rtk_api_ret_t (*stat_lengthMode_set)(rtk_stat_lengthMode_t, rtk_stat_lengthMode_t);
    rtk_api_ret_t (*stat_lengthMode_get)(rtk_stat_lengthMode_t *, rtk_stat_lengthMode_t *);
#endif
    /* l2 */
    rtk_api_ret_t (*l2_init)(void);
    rtk_api_ret_t (*l2_addr_add)(rtk_mac_t *, rtk_l2_ucastAddr_t *);
    rtk_api_ret_t (*l2_addr_get)(rtk_mac_t *, rtk_l2_ucastAddr_t *);
    rtk_api_ret_t (*l2_addr_next_get)(rtk_l2_read_method_t, rtk_port_t, rtk_uint32 *, rtk_l2_ucastAddr_t *);
    rtk_api_ret_t (*l2_addr_del)(rtk_mac_t *, rtk_l2_ucastAddr_t *);
    rtk_api_ret_t (*l2_mcastAddr_add)(rtk_l2_mcastAddr_t *);
    rtk_api_ret_t (*l2_mcastAddr_get)(rtk_l2_mcastAddr_t *);
    rtk_api_ret_t (*l2_mcastAddr_next_get)(rtk_uint32 *, rtk_l2_mcastAddr_t *);
    rtk_api_ret_t (*l2_mcastAddr_del)(rtk_l2_mcastAddr_t *);
    rtk_api_ret_t (*l2_ipMcastAddr_add)(rtk_l2_ipMcastAddr_t *);
    rtk_api_ret_t (*l2_ipMcastAddr_get)(rtk_l2_ipMcastAddr_t *);
    rtk_api_ret_t (*l2_ipMcastAddr_next_get)(rtk_uint32 *, rtk_l2_ipMcastAddr_t *);
    rtk_api_ret_t (*l2_ipMcastAddr_del)(rtk_l2_ipMcastAddr_t *);
    rtk_api_ret_t (*l2_ucastAddr_flush)(rtk_l2_flushCfg_t *);
    rtk_api_ret_t (*l2_table_clear)(void);
    rtk_api_ret_t (*l2_table_clearStatus_get)(rtk_l2_clearStatus_t *);
    rtk_api_ret_t (*l2_flushLinkDownPortAddrEnable_set)(rtk_enable_t);
    rtk_api_ret_t (*l2_flushLinkDownPortAddrEnable_get)(rtk_enable_t *);
    rtk_api_ret_t (*l2_agingEnable_set)(rtk_port_t, rtk_enable_t);
    rtk_api_ret_t (*l2_agingEnable_get)(rtk_port_t, rtk_enable_t *);
    rtk_api_ret_t (*l2_limitLearningCnt_set)(rtk_port_t, rtk_mac_cnt_t);
    rtk_api_ret_t (*l2_limitLearningCnt_get)(rtk_port_t, rtk_mac_cnt_t *);
    rtk_api_ret_t (*l2_limitSystemLearningCnt_set)(rtk_mac_cnt_t);
    rtk_api_ret_t (*l2_limitSystemLearningCnt_get)(rtk_mac_cnt_t *);
    rtk_api_ret_t (*l2_limitLearningCntAction_set)(rtk_port_t, rtk_l2_limitLearnCntAction_t);
    rtk_api_ret_t (*l2_limitLearningCntAction_get)(rtk_port_t, rtk_l2_limitLearnCntAction_t *);
    rtk_api_ret_t (*l2_limitSystemLearningCntAction_set)(rtk_l2_limitLearnCntAction_t);
    rtk_api_ret_t (*l2_limitSystemLearningCntAction_get)(rtk_l2_limitLearnCntAction_t *);
    rtk_api_ret_t (*l2_limitSystemLearningCntPortMask_set)(rtk_portmask_t *);
    rtk_api_ret_t (*l2_limitSystemLearningCntPortMask_get)(rtk_portmask_t *);
    rtk_api_ret_t (*l2_learningCnt_get)(rtk_port_t port, rtk_mac_cnt_t *);
    rtk_api_ret_t (*l2_floodPortMask_set)(rtk_l2_flood_type_t, rtk_portmask_t *);
    rtk_api_ret_t (*l2_floodPortMask_get)(rtk_l2_flood_type_t, rtk_portmask_t *);
    rtk_api_ret_t (*l2_localPktPermit_set)(rtk_port_t, rtk_enable_t);
    rtk_api_ret_t (*l2_localPktPermit_get)(rtk_port_t, rtk_enable_t *);
    rtk_api_ret_t (*l2_aging_set)(rtk_l2_age_time_t);
    rtk_api_ret_t (*l2_aging_get)(rtk_l2_age_time_t *);
    rtk_api_ret_t (*l2_ipMcastAddrLookup_set)(rtk_l2_ipmc_lookup_type_t);
    rtk_api_ret_t (*l2_ipMcastAddrLookup_get)(rtk_l2_ipmc_lookup_type_t *);
    rtk_api_ret_t (*l2_ipMcastForwardRouterPort_set)(rtk_enable_t);
    rtk_api_ret_t (*l2_ipMcastForwardRouterPort_get)(rtk_enable_t *);
    rtk_api_ret_t (*l2_ipMcastGroupEntry_add)(ipaddr_t, rtk_portmask_t *);
    rtk_api_ret_t (*l2_ipMcastGroupEntry_del)(ipaddr_t);
    rtk_api_ret_t (*l2_ipMcastGroupEntry_get)(ipaddr_t, rtk_portmask_t *);
    rtk_api_ret_t (*l2_entry_get)(rtk_l2_addr_table_t *);
    rtk_api_ret_t (*l2_lookupHitIsolationAction_set)(rtk_l2_lookupHitIsolationAction_t);
    rtk_api_ret_t (*l2_lookupHitIsolationAction_get)(rtk_l2_lookupHitIsolationAction_t *);

    rtk_api_ret_t (*l2_unknownUnicastPktAction_set)(rtk_port_t, rtk_uint32);
    rtk_api_ret_t (*l2_unknownUnicastPktAction_get)(rtk_port_t, rtk_uint32 *);
    rtk_api_ret_t (*l2_unknownMulticastPktAction_set)(rtk_port_t, rtk_uint32);
    rtk_api_ret_t (*l2_unknownMulticastPktAction_get)(rtk_port_t, rtk_uint32 *);
    rtk_api_ret_t (*l2_unknownV4McPktAction_set)(rtk_port_t, rtk_uint32);
    rtk_api_ret_t (*l2_unknownV4McPktAction_get)(rtk_port_t, rtk_uint32 *);
    rtk_api_ret_t (*l2_unknownV6McPktAction_set)(rtk_port_t, rtk_uint32);
    rtk_api_ret_t (*l2_unknownV6McPktAction_get)(rtk_port_t, rtk_uint32 *);
    
 
    rtk_api_ret_t (*l2_unmatchMacMoving_set)(rtk_port_t, rtk_enable_t);
    rtk_api_ret_t (*l2_unmatchMacMoving_get)(rtk_port_t, rtk_enable_t *);
    rtk_api_ret_t (*l2_unknownMcastPktAction_set)(rtk_port_t, rtk_uint32, rtk_uint32);
    rtk_api_ret_t (*l2_unknownMcastPktAction_get)(rtk_port_t, rtk_uint32, rtk_uint32 *);

    rtk_api_ret_t (*l2_portUnmatchMacPktAction_set)(rtk_port_t, rtk_uint32);
    rtk_api_ret_t (*l2_portUnmatchMacPktAction_get)(rtk_port_t, rtk_uint32 *);

    /*gpio*/
    rtk_api_ret_t (*gpio_muxSel_set)(rtk_uint32);
    rtk_api_ret_t (*gpio_muxSel_get)(rtk_uint32, rtk_enable_t *);
    rtk_api_ret_t (*gpio_groupVal_write)(rtk_gpio_groupReg_t, rtk_uint32);
    rtk_api_ret_t (*gpio_groupVal_read)(rtk_gpio_groupReg_t, rtk_uint32 *);
    rtk_api_ret_t (*gpio_pinVal_write)(rtk_uint32, rtk_gpio_level_t);
    rtk_api_ret_t (*gpio_pinVal_read)(rtk_uint32, rtk_gpio_level_t *pVal);
    rtk_api_ret_t (*gpio_pinDir_set)(rtk_uint32, rtk_gpio_direction_t );
    rtk_api_ret_t (*gpio_pinDir_get)(rtk_uint32, rtk_gpio_direction_t *);
    rtk_api_ret_t (*gpio_groupDir_get)(rtk_gpio_groupReg_t , rtk_uint32 *);

	/*I2C*/

	rtk_api_ret_t (*i2c_init)(rtk_i2c_sclClockRate_t, rtk_uint32);
	rtk_api_ret_t (*i2c_readMode_set)(rtk_i2c_readMode_t);
	rtk_api_ret_t (*i2c_readMode_get)(rtk_i2c_readMode_t *);
	rtk_api_ret_t (*i2c_gpioPinGroup_set)(rtk_uint32, rtk_uint32);
	rtk_api_ret_t (*i2c_gpioPinGroup_get)(rtk_uint32 *, rtk_uint32 *);
	rtk_api_ret_t (*i2c_data_read)(rtk_uint32, rtk_uint32, rtk_uint32, rtk_uint32 *);
	rtk_api_ret_t (*i2c_data_write)(rtk_uint32, rtk_uint32, rtk_uint32, rtk_uint32 *);

	/*Rate : ingress BW & egress queue BW*/
	rtk_api_ret_t (*rate_igrBwCtrlPortEn_set)(rtk_port_t, rtk_enable_t);
    rtk_api_ret_t (*rate_igrBwCtrlPortEn_get)(rtk_port_t, rtk_enable_t *);
	rtk_api_ret_t (*rate_igrBwCtrlRate_set)(rtk_port_t, rtk_rate_t,  rtk_enable_t);
    rtk_api_ret_t (*rate_igrBwCtrlRate_get)(rtk_port_t, rtk_rate_t *, rtk_enable_t *);
	rtk_api_ret_t (*rate_igrBwCtrlIfg_set)(rtk_enable_t);
    rtk_api_ret_t (*rate_igrBwCtrlIfg_get)(rtk_enable_t *);
	rtk_api_ret_t (*rate_igrBwCtrlCongestSts_get)(rtk_port_t, rtk_rate_igrBwCongestSts_t *);
    rtk_api_ret_t (*rate_egrBandwidthCtrlRate_set)(rtk_port_t, rtk_rate_t,  rtk_enable_t);
    rtk_api_ret_t (*rate_egrBandwidthCtrlRate_get)(rtk_port_t, rtk_rate_t *, rtk_enable_t *);
    rtk_api_ret_t (*rate_egrQueueBwCtrlEnable_set)(rtk_port_t, rtk_qid_t, rtk_enable_t);
    rtk_api_ret_t (*rate_egrQueueBwCtrlEnable_get)(rtk_port_t, rtk_qid_t, rtk_enable_t *);
    rtk_api_ret_t (*rate_egrQueueBwCtrlRate_set)(rtk_port_t, rtk_qid_t, rtk_rate_t);
    rtk_api_ret_t (*rate_egrQueueBwCtrlRate_get)(rtk_port_t, rtk_qid_t, rtk_rate_t *);

	/*EEE*/
	rtk_api_ret_t (*eee_init)(void);
    rtk_api_ret_t (*eee_macForceSpeedEn_set)(rtk_port_t, rtk_eee_speedInMacForceMode_t, rtk_enable_t);
    rtk_api_ret_t (*eee_macForceSpeedEn_get)(rtk_port_t, rtk_eee_speedInMacForceMode_t, rtk_enable_t *);
    rtk_api_ret_t (*eee_macForceAllSpeedEn_get)(rtk_port_t, rtk_uint32 *);
    rtk_api_ret_t (*eee_portTxRxEn_set)(rtk_port_t, rtk_enable_t, rtk_enable_t);
    rtk_api_ret_t (*eee_portTxRxEn_get)(rtk_port_t, rtk_enable_t *, rtk_enable_t *);

	/*mib*/
    rtk_api_ret_t (*stat_global_reset)(void);
    rtk_api_ret_t (*stat_port_reset)(rtk_port_t);
    rtk_api_ret_t (*stat_port_get)(rtk_port_t, rtk_stat_port_type_t, rtk_stat_counter_t *);
    rtk_api_ret_t (*stat_lengthMode_set)(rtk_stat_lengthMode_t, rtk_stat_lengthMode_t);
    rtk_api_ret_t (*stat_lengthMode_get)(rtk_stat_lengthMode_t *, rtk_stat_lengthMode_t *);

    /*port*/
    rtk_api_ret_t (*port_macForceLink_set)(rtk_port_t, rtk_port_ability_t *);
    rtk_api_ret_t (*port_macForceLink_get)(rtk_port_t, rtk_port_ability_t *);
    rtk_api_ret_t (*port_macStatus_get)(rtk_port_t, rtk_port_status_t *);
    rtk_api_ret_t (*port_macLocalLoopbackEnable_set)(rtk_port_t, rtk_enable_t);
    rtk_api_ret_t (*port_macLocalLoopbackEnable_get)(rtk_port_t, rtk_enable_t *);
    rtk_api_ret_t (*port_backpressureEnable_set)(rtk_port_t, rtk_enable_t);
    rtk_api_ret_t (*port_backpressureEnable_get)(rtk_port_t, rtk_enable_t *);
    rtk_api_ret_t (*port_rtct_init)(void);
    rtk_api_ret_t (*port_rtct_start)(rtk_uint32);
    rtk_api_ret_t (*port_rtctResult_get)(rtk_port_t, rtk_rtct_result_t *);
    rtk_api_ret_t (*port_sdsMode_set)(rtk_uint32, rtk_sds_mode_t);
    rtk_api_ret_t (*port_sdsMode_get)(rtk_uint32, rtk_sds_mode_t *);
    rtk_api_ret_t (*port_sdsNway_get)(rtk_uint32, rtk_sds_mode_t, rtk_enable_t*);
    rtk_api_ret_t (*port_sdsNway_set)(rtk_uint32, rtk_sds_mode_t, rtk_enable_t);

    rtk_api_ret_t (*port_extphyid_set)(rtk_uint32, rtk_uint32);
    rtk_api_ret_t (*port_extphyid_get)(rtk_uint32, rtk_uint32 *);
	/*phy*/
    rtk_api_ret_t (*phy_common_c45_an_restart)(rtk_port_t);
    rtk_api_ret_t (*phy_common_c45_autoNegoEnable_get)(rtk_port_t, rtk_enable_t *);
    rtk_api_ret_t (*phy_common_c45_autoNegoEnable_set)(rtk_port_t, rtk_enable_t);
    rtk_api_ret_t (*phy_autoNegoAbility_set)(rtk_port_t, rtk_port_phy_ability_t *);
    rtk_api_ret_t (*phy_common_c45_autoSpeed_set)(rtk_port_t, rtk_port_phy_ability_t *);

    rtk_api_ret_t (*phy_common_c45_speed_set)(rtk_port_t, rtk_port_speed_t);
    rtk_api_ret_t (*phy_common_c45_speed_get)(rtk_port_t, rtk_port_speed_t *);
    rtk_api_ret_t (*phy_common_c45_enable_set)(rtk_port_t, rtk_enable_t);
    rtk_api_ret_t (*phy_common_c45_enable_get)(rtk_port_t, rtk_enable_t *);
    rtk_api_ret_t (*phy_common_c45_duplex_set)(rtk_port_t, rtk_port_duplex_t);
    rtk_api_ret_t (*phy_common_c45_duplex_get)(rtk_port_t, rtk_port_duplex_t *);
    rtk_api_ret_t (*phy_common_c45_speedDuplexStatusResReg_get)(rtk_port_t, rtk_port_speed_t *, rtk_port_duplex_t *);
#if 0
    
    rtk_api_ret_t (*trap_lldpEnable_set)(rtk_enable_t);
    rtk_api_ret_t (*trap_lldpEnable_get)(rtk_enable_t *);
    rtk_api_ret_t (*trap_reasonTrapToCpuPriority_set)(rtk_trap_reason_type_t, rtk_pri_t);
    rtk_api_ret_t (*trap_reasonTrapToCpuPriority_get)(rtk_trap_reason_type_t, rtk_pri_t *);
    rtk_api_ret_t (*trap_rmaAction_set)(rtk_trap_type_t, rtk_trap_rma_action_t);
    rtk_api_ret_t (*trap_rmaAction_get)(rtk_trap_type_t, rtk_trap_rma_action_t *);
    rtk_api_ret_t (*trap_rmaKeepFormat_set)(rtk_trap_type_t, rtk_enable_t);
    rtk_api_ret_t (*trap_rmaKeepFormat_get)(rtk_trap_type_t, rtk_enable_t *);
#endif    
#if 0
    /* interrupt */
    rtk_api_ret_t (*int_polarity_set)(rtk_int_polarity_t);
    rtk_api_ret_t (*int_polarity_get)(rtk_int_polarity_t *);
    rtk_api_ret_t (*int_control_set)(rtk_int_type_t, rtk_enable_t);
    rtk_api_ret_t (*int_control_get)(rtk_int_type_t, rtk_enable_t *);
    rtk_api_ret_t (*int_status_set)(rtk_int_status_t *);
    rtk_api_ret_t (*int_status_get)(rtk_int_status_t *);
    rtk_api_ret_t (*int_advanceInfo_get)(rtk_int_advType_t, rtk_int_info_t *);

    /* port */
    rtk_api_ret_t (*port_phyAutoNegoAbility_set)(rtk_port_t, rtk_port_phy_ability_t *);
    rtk_api_ret_t (*port_phyAutoNegoAbility_get)(rtk_port_t, rtk_port_phy_ability_t *);
    rtk_api_ret_t (*port_phyForceModeAbility_set)(rtk_port_t, rtk_port_phy_ability_t *);
    rtk_api_ret_t (*port_phyForceModeAbility_get)(rtk_port_t, rtk_port_phy_ability_t *);
    rtk_api_ret_t (*port_phyStatus_get)(rtk_port_t, rtk_port_linkStatus_t *, rtk_port_speed_t *, rtk_port_duplex_t *);
    rtk_api_ret_t (*port_macForceLink_set)(rtk_port_t, rtk_port_mac_ability_t *);
    rtk_api_ret_t (*port_macForceLink_get)(rtk_port_t, rtk_port_mac_ability_t *);
    rtk_api_ret_t (*port_macForceLinkExt_set)(rtk_port_t, rtk_mode_ext_t, rtk_port_mac_ability_t *);
    rtk_api_ret_t (*port_macForceLinkExt_get)(rtk_port_t, rtk_mode_ext_t *, rtk_port_mac_ability_t *);
    
    
    rtk_api_ret_t (*port_phyReg_set)(rtk_port_t, rtk_port_phy_reg_t, rtk_port_phy_data_t);
    rtk_api_ret_t (*port_phyReg_get)(rtk_port_t, rtk_port_phy_reg_t, rtk_port_phy_data_t *);
    
    rtk_api_ret_t (*port_adminEnable_set)(rtk_port_t, rtk_enable_t);
    rtk_api_ret_t (*port_adminEnable_get)(rtk_port_t, rtk_enable_t *);
    #endif

} dal_mapper_t;


#endif /* __DAL_MAPPER_H __ */

