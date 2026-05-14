
/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 */

/*
 * Include Files
 */
#include <dal/dal_mapper.h>
#include <dal/rtl8373/dal_rtl8373_mapper.h>
#include <dal/rtl8373/dal_rtl8373_switch.h>
#include <dal/rtl8373/dal_rtl8373_vlan.h>
#include <dal/rtl8373/dal_rtl8373_svlan.h>
#include <dal/rtl8373/dal_rtl8373_acl.h>
#include <dal/rtl8373/dal_rtl8373_mirror.h>
#include <dal/rtl8373/dal_rtl8373_cpuTag.h>
#include <dal/rtl8373/dal_rtl8373_isolation.h>
#include <dal/rtl8373/dal_rtl8373_qos.h>
#include <dal/rtl8373/dal_rtl8373_sharemeter.h>
#include <dal/rtl8373/dal_rtl8373_storm.h>
#include <dal/rtl8373/dal_rtl8373_dot1x.h>
#include <dal/rtl8373/dal_rtl8373_nic.h>
#include <dal_rtl8373_igmp.h>
#include <dal/rtl8373/dal_rtl8373_lut.h>
#include <dal/rtl8373/dal_rtl8373_macsec.h>
#include <dal/rtl8373/dal_rtl8373_trunk.h>
#include <dal/rtl8373/dal_rtl8373_rtkpp.h>
#include <dal/rtl8373/dal_rtl8373_dos.h>
#include <dal/rtl8373/dal_rtl8373_gpio.h>
#include <dal/rtl8373/dal_rtl8373_ptp.h>
#include <dal/rtl8373/dal_rtl8373_drv.h>
#include <dal/rtl8373/dal_rtl8373_i2c.h>
#include <dal/rtl8373/dal_rtl8373_rate.h>
#include <dal/rtl8373/dal_rtl8373_eee.h>
#include <dal/rtl8373/dal_rtl8373_interrupt.h>
#include <dal/rtl8373/dal_rtl8373_mib.h>
#include <dal/rtl8373/dal_rtl8373_rma.h>
#include <dal/rtl8373/dal_rtl8373_led.h>
#include <dal/rtl8373/dal_rtl8373_port.h>
#include <dal/rtl8373/dal_rtl8373_phy.h>

#if 0
#include <dal/rtl8373/dal_rtl8373_leaky.h>
#include <dal/rtl8373/dal_rtl8373_rldp.h>
#include <dal/rtl8373/dal_rtl8373_led.h>
#include <dal/rtl8373/dal_rtl8373_stat.h>
#include <dal/rtl8373/dal_rtl8373_interrupt.h>
#include <dal/rtl8373/dal_rtl8373_oam.h>
#endif
/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
static dal_mapper_t dal_rtl8373_mapper =
{
    /* Switch */
    .switch_init = dal_rtl8373_switch_init,
    //.switch_portMaxPktLen_set = dal_rtl8373_portMaxLen_set,
    //.switch_portMaxPktLen_get = dal_rtl8373_portMaxLen_get,
    //.switch_maxPktLenCfg_set = dal_rtl8373_switch_maxPktLenCfg_set,
    //.switch_maxPktLenCfg_get = dal_rtl8373_switch_maxPktLenCfg_get,
    .switch_greenEthernet_set = NULL, //dal_rtl8373_switch_greenEthernet_set,
    .switch_greenEthernet_get = NULL, //dal_rtl8373_switch_greenEthernet_get,

    .fMdrv_miim_mmd_read = dal_rtl8373_phy_read,
    .fMdrv_miim_mmd_write = dal_rtl8373_phy_write,
    .fMdrv_miim_mmd_readbits = dal_rtl8373_phy_readBits,
    .fMdrv_miim_mmd_writebits = dal_rtl8373_phy_writeBits,

    .fw_reset_flow_tgr_tgx = fw_reset_flow_tgr_tgx,
    .fw_reset_flow_tgr = fw_reset_flow_tgr,
    .fw_reset_flow_tgx = fw_reset_flow_tgx,
    .fw_reset_flow_tgr_8224 = fw_reset_flow_tgr_8224,
    .fw_reset_flow_8221B = dal_rtl8373_fw_reset_flow_8221B,
    .rtl8224_top_reg_write = dal_rtl8224_top_reg_write,
    .rtl8224_top_reg_read= dal_rtl8224_top_reg_read,
    .rtl8224_top_regbit_write = dal_rtl8224_top_regbit_write,
    .rtl8224_top_regbit_read = dal_rtl8224_top_regbit_read,
    .rtl8224_top_regbits_write = dal_rtl8224_top_regbits_write,
    .rtl8224_top_regbits_read = dal_rtl8224_top_regbits_read,
    .rtl8373_sds_reg_write = dal_rtl8373_sds_reg_write,
    .rtl8373_sds_reg_read = dal_rtl8373_sds_reg_read,
    .rtl8373_sds_regbits_write = dal_rtl8373_sds_regbits_write,
    .rtl8373_sds_regbits_read = dal_rtl8373_sds_regbits_read,
    .rtl8224_sds_reg_write = dal_rtl8224_sds_reg_write,
    .rtl8224_sds_reg_read = dal_rtl8224_sds_reg_read,
    .rtl8224_sds_regbits_write = dal_rtl8224_sds_regbits_write,
    .rtl8224_sds_regbits_read = dal_rtl8224_sds_regbits_read,
    .rtl8373_phy_regbits_write = dal_rtl8373_phy_regbits_write,
    .rtl8373_phy_regbits_read = dal_rtl8373_phy_regbits_read,

    .rtl8373_setAsicRegBit = rtl8373_setAsicRegBit,
    .rtl8373_getAsicRegBit = rtl8373_getAsicRegBit,
    .rtl8373_setAsicRegBits = rtl8373_setAsicRegBits,
    .rtl8373_getAsicRegBits = rtl8373_getAsicRegBits,
    .rtl8373_setAsicReg = rtl8373_setAsicReg,
    .rtl8373_getAsicReg = rtl8373_getAsicReg,
    
    /* cpu */
    .cpuTag_externalCpuPort_set = dal_rtl8373_cpuTag_externalCpuPort_set,
    .cpuTag_externalCpuPort_get = dal_rtl8373_cpuTag_externalCpuPort_get,
    .cpuTag_tpid_set = dal_rtl8373_cpuTag_tpid_set,
    .cpuTag_tpid_get =  dal_rtl8373_cpuTag_tpid_get,
    .cpuTag_enable_set = dal_rtl8373_cpuTag_enable_set,
    .cpuTag_enable_get = dal_rtl8373_cpuTag_enable_get,
    .cpuTag_insertMode_set = dal_rtl8373_cpuTag_insertMode_set,
    .cpuTag_insertMode_get = dal_rtl8373_cpuTag_insertMode_get,
    .cpuTag_awarePort_set = dal_rtl8373_cpuTag_awarePort_set,
    .cpuTag_awarePort_get = dal_rtl8373_cpuTag_awarePort_get,
    .cpuTag_priRemap_set = dal_rtl8373_cpuTag_priRemap_set,
    .cpuTag_priRemap_get = dal_rtl8373_cpuTag_priRemap_get,

    /* acl */
    .filter_igrAcl_init = dal_rtl8373_igrAcl_init,
    .filter_igrAcl_field_add = dal_rtl8373_igrAcl_field_add,
    .filter_igrAcl_cfg_add = dal_rtl8373_igrAcl_cfg_add,
    .filter_igrAcl_cfg_del = dal_rtl8373_igrAcl_cfg_del,
    .filter_igrAcl_cfg_delAll = dal_rtl8373_igrAcl_cfg_delAll,
    .filter_igrAcl_cfg_get = dal_rtl8373_igrAcl_cfg_get,
    .filter_igrAcl_unmatchAction_set = dal_rtl8373_igrAcl_unmatchAction_set,
    .filter_igrAcl_unmatchAction_get = dal_rtl8373_igrAcl_unmatchAction_get,
    .filter_igrAcl_state_set = dal_rtl8373_igrAcl_state_set,
    .filter_igrAcl_state_get = dal_rtl8373_igrAcl_state_get,
    .filter_igrAcl_template_set = dal_rtl8373_igrAcl_template_set,
    .filter_igrAcl_template_get = dal_rtl8373_igrAcl_template_get,
    .filter_igrAcl_field_sel_set = dal_rtl8373_igrAcl_fieldSel_set,
    .filter_igrAcl_field_sel_get = dal_rtl8373_igrAcl_fieldSel_get,
    .filter_iprange_set = dal_rtl8373_igrAcl_ipRange_set,
    .filter_iprange_get = dal_rtl8373_igrAcl_ipRange_get,
    .filter_vidrange_set = dal_rtl8373_igrAcl_vidRange_set,
    .filter_vidrange_get = dal_rtl8373_igrAcl_vidRange_get,
    .filter_portrange_set = dal_rtl8373_igrAcl_portRange_set,
    .filter_portrange_get = dal_rtl8373_igrAcl_portRange_get,
    .filter_igrAcl_gpioPolarity_set = dal_rtl8373_igrAcl_gpioPolarity_set,
    .filter_igrAcl_gpioPolarity_get = dal_rtl8373_igrAcl_gpioPolarity_get,
    .filter_igrAcl_gpioEn_set = dal_rtl8373_igrAcl_gpioEn_set,
    .filter_igrAcl_gpioEn_get = dal_rtl8373_igrAcl_gpioEn_get,
    .filter_igrAcl_tbl_rst = dal_rtl8373_igrAcl_table_rst,

    /* mirror */
    .mirror_set_en = dal_rtl8373_mirror_set_en,
    .mirror_setStatus_get = dal_rtl8373_mirror_setStatus_get,
    .mirror_entry_set = dal_rtl8373_mirror_entry_set,
    .mirror_entry_get = dal_rtl8373_mirror_entry_get,
    .mirror_portIso_set = dal_rtl8373_mirror_portIso_set,
    .mirror_portIso_get = dal_rtl8373_mirror_portIso_get,
    .mirror_vlanLeaky_set = dal_rtl8373_mirror_vlanLeaky_set,
    .mirror_vlanLeaky_get = dal_rtl8373_mirror_vlanLeaky_get,
    .mirror_isolationLeaky_set = dal_rtl8373_mirror_isolationLeaky_set,
    .mirror_isolationLeaky_get = dal_rtl8373_mirror_isolationLeaky_get,
    .mirror_keep_set = dal_rtl8373_mirror_keep_set,
    .mirror_keep_get = dal_rtl8373_mirror_keep_get,
    .mirror_override_set = dal_rtl8373_mirror_override_set,
    .mirror_override_get = dal_rtl8373_mirror_override_get,
    .mirror_sampleRate_set = dal_rtl8373_mirror_sampeRate_set,
    .mirror_sampleRate_get = dal_rtl8373_mirror_sampeRate_get,
    .mirror_pktCnt_get = dal_rtl8373_mirror_pktCnt_get,
    .mirror_samplePktCnt_get = dal_rtl8373_mirror_samplePktCnt_get,
    .rspan_rxTag_en = dal_rtl8373_rspan_rxTag_en,
    .rspan_rxTagEnSts_get = dal_rtl8373_rspan_rxTagEnSts_get,
    .rspan_tagCtxt_set = dal_rtl8373_rspan_tagContext_set,
    .rspan_tagCtxt_get = dal_rtl8373_rspan_tagContext_get,
    .rspan_tagAdd_set = dal_rtl8373_rspan_tagAdd_set,
    .rspan_tagAdd_get = dal_rtl8373_rspan_tagAdd_get,
    .rspan_tagRemove_set = dal_rtl8373_rspan_tagRemove_set,
    .rspan_tagRemove_get = dal_rtl8373_rspan_tagRemove_get,

    .port_isolation_set = dal_rtl8373_port_isolation_set,
    .port_isolation_get = dal_rtl8373_port_isolation_get,


    /* Storm */
    .rate_stormControlMeterIdx_set = dal_rtl8373_rate_stormControlMeterIdx_set,
    .rate_stormControlMeterIdx_get = dal_rtl8373_rate_stormControlMeterIdx_get,
    .rate_stormControlPortEnable_set = dal_rtl8373_rate_stormControlPortEnable_set,
    .rate_stormControlPortEnable_get = dal_rtl8373_rate_stormControlPortEnable_get,
    .storm_bypass_set = dal_rtl8373_storm_bypass_set,
    .storm_bypass_get = dal_rtl8373_storm_bypass_get,
    .rate_stormControlExtPortmask_set = dal_rtl8373_rate_stormControlExtPortmask_set,
    .rate_stormControlExtPortmask_get = dal_rtl8373_rate_stormControlExtPortmask_get,
    .rate_stormControlExtEnable_set = dal_rtl8373_rate_stormControlExtEnable_set,
    .rate_stormControlExtEnable_get = dal_rtl8373_rate_stormControlExtEnable_get,
    .rate_stormControlExtMeterIdx_set = dal_rtl8373_rate_stormControlExtMeterIdx_set,
    .rate_stormControlExtMeterIdx_get = dal_rtl8373_rate_stormControlExtMeterIdx_get,

    /* Rate */
    .rate_shareMeter_set = dal_rtl8373_rate_shareMeter_set,
    .rate_shareMeter_get = dal_rtl8373_rate_shareMeter_get,
    .rate_shareMeterBucket_set = dal_rtl8373_rate_shareMeterBucket_set,
    .rate_shareMeterBucket_get = dal_rtl8373_rate_shareMeterBucket_get,
    .rate_shareMeterExceedStatus_set = dal_rtl8373_rate_shareMeterExceedStatus_set,
    .rate_shareMeterExceedStatus_get = dal_rtl8373_rate_shareMeterExceedStatus_get,
    .rate_shareMeterICPUExceedStatus_set = dal_rtl8373_rate_shareMeterICPUExceedStatus_set,
    .rate_shareMeterICPUExceedStatus_get = dal_rtl8373_rate_shareMeterICPUExceedStatus_get,
    /*QoS*/
    .qos_init = dal_rtl8373_qos_init,
    .qos_priSel_set = dal_rtl8373_qos_priSel_set,
    .qos_priSel_get = dal_rtl8373_qos_priSel_get,
    .qos_1pPriRemap_set = dal_rtl8373_qos_1pPriRemap_set,
    .qos_1pPriRemap_get = dal_rtl8373_qos_1pPriRemap_get,
    .qos_1pRemarkSrcSel_set = dal_rtl8373_qos_1pRemarkSrcSel_set,
    .qos_1pRemarkSrcSel_get = dal_rtl8373_qos_1pRemarkSrcSel_get,
    .qos_dscpPriRemap_set = dal_rtl8373_qos_dscpPriRemap_set,
    .qos_dscpPriRemap_get = dal_rtl8373_qos_dscpPriRemap_get,
    .qos_rspanpriRemap_set = dal_rtl8373_qos_RspanPriRemap_set,
    .qos_rspanpriRemap_get = dal_rtl8373_qos_RspanPriRemap_get,
    .qos_portPri_set = dal_rtl8373_qos_portPri_set,
    .qos_portPri_get = dal_rtl8373_qos_portPri_get,
    .qos_priMap_set = dal_rtl8373_qos_priMap_set,
    .qos_priMap_get = dal_rtl8373_qos_priMap_get,
    .qos_schedulingQueue_set = dal_rtl8373_qos_schedulingQueue_set,
    .qos_schedulingQueue_get = dal_rtl8373_qos_schedulingQueue_get,
    .qos_1pRemarkEnable_set = dal_rtl8373_qos_1pRemarkEnable_set,
    .qos_1pRemarkEnable_get = dal_rtl8373_qos_1pRemarkEnable_get,
    .qos_1pRemark_set = dal_rtl8373_qos_1pRemark_set,
    .qos_1pRemark_get = dal_rtl8373_qos_1pRemark_get,
    .qos_dscpRemarkEnable_set = dal_rtl8373_qos_dscpRemarkEnable_set,
    .qos_dscpRemarkEnable_get = dal_rtl8373_qos_dscpRemarkEnable_get,
    .qos_intpri2dscpRemark_set = dal_rtl8373_qos_intpri2dscp_Remark_set,
    .qos_intpri2dscpRemark_get = dal_rtl8373_qos_intpri2dscp_Remark_get,
    .qos_dscp2dscpRemark_set = dal_rtl8373_qos_dscp2dscp_Remark_set,
    .qos_dscp2dscpRemark_get = dal_rtl8373_qos_dscp2dscp_Remark_get,
    .qos_dscpRemarkSrcSel_set = dal_rtl8373_qos_dscpRemarkSrcSel_set,
    .qos_dscpRemarkSrcSel_get = dal_rtl8373_qos_dscpRemarkSrcSel_get,
    .qos_portPriSelIndex_set = dal_rtl8373_qos_portPriSelIndex_set,
    .qos_portPriSelIndex_get = dal_rtl8373_qos_portPriSelIndex_get,
    .qos_schedulingType_set = dal_rtl8373_qos_schedulingType_set,
    .qos_schedulingType_get = dal_rtl8373_qos_schedulingType_get,

    /*VLAN*/
    .vlan_init = dal_rtl8373_vlan_init,
    .vlan_set = dal_rtl8373_vlan_set,
    .vlan_get = dal_rtl8373_vlan_get,
    .vlan_egrFilterEnable_set = dal_rtl8373_vlan_egrFilterEnable_set,
    .vlan_egrFilterEnable_get = dal_rtl8373_vlan_egrFilterEnable_get,
    .vlan_portPvid_set = dal_rtl8373_vlan_portPvid_set,
    .vlan_portPvid_get = dal_rtl8373_vlan_portPvid_get,
    .vlan_portIgrFilterEnable_set = dal_rtl8373_vlan_portIgrFilterEnable_set,
    .vlan_portIgrFilterEnable_get = dal_rtl8373_vlan_portIgrFilterEnable_get,
    .vlan_portAcceptFrameType_set = dal_rtl8373_vlan_portAcceptFrameType_set,
    .vlan_portAcceptFrameType_get = dal_rtl8373_vlan_portAcceptFrameType_get,
    .vlan_tagMode_set = dal_rtl8373_vlan_tagMode_set,
    .vlan_tagMode_get = dal_rtl8373_vlan_tagMode_get,
    .vlan_transparent_set = dal_rtl8373_vlan_portTransparent_set,
    .vlan_transparent_get = dal_rtl8373_vlan_portTransparent_get,
    .vlan_keep_set = dal_rtl8373_vlan_keep_set,
    .vlan_keep_get = dal_rtl8373_vlan_keep_get,
    .vlan_stg_set = dal_rtl8373_vlan_stg_set,
    .vlan_stg_get = dal_rtl8373_vlan_stg_get,
    .vlan_portFid_set = dal_rtl8373_vlan_portFid_set,
    .vlan_portFid_get = dal_rtl8373_vlan_portFid_get,
    .vlan_reservedVidAction_set = dal_rtl8373_vlan_reservedVidAction_set,
    .vlan_reservedVidAction_get = dal_rtl8373_vlan_reservedVidAction_get,
    .vlan_realKeepRemarkEnable_set = dal_rtl8373_vlan_realKeepRemarkEnable_set,
    .vlan_realKeepRemarkEnable_get = dal_rtl8373_vlan_realKeepRemarkEnable_get,
    .vlan_disL2Learn_entry_set = dal_rtl8373_vlan_disL2Learn_entry_set,
    .vlan_disL2Learn_entry_get =  dal_rtl8373_vlan_disL2Learn_entry_get,
    .vlan_reset = dal_rtl8373_vlan_reset,

    /*dot1x*/
    .dot1x_unauthPacketOper_set = dal_rtl8373_dot1x_unauthPacketOper_set,
    .dot1x_unauthPacketOper_get = dal_rtl8373_dot1x_unauthPacketOper_get,
    //.dot1x_eapolFrame2CpuEnable_set = dal_rtl8373_dot1x_eapolFrame2CpuEnable_set,
    //.dot1x_eapolFrame2CpuEnable_get = dal_rtl8373_dot1x_eapolFrame2CpuEnable_get,
    .dot1x_trap2CPU_Sel_set = dal_rtl8373_dot1x_trap2CPU_Sel_set,
    .dot1x_trap2CPU_Sel_get = dal_rtl8373_dot1x_trap2CPU_Sel_get,
    .dot1x_trap_priority_set = dal_rtl8373_dot1x_trap_priority_set,
    .dot1x_trap_priority_get = dal_rtl8373_dot1x_trap_priority_get,
    .dot1x_portBasedEnable_set = dal_rtl8373_dot1x_portBasedEnable_set,
    .dot1x_portBasedEnable_get = dal_rtl8373_dot1x_portBasedEnable_get,
    .dot1x_portBasedAuthStatus_set = dal_rtl8373_dot1x_portBasedAuthStatus_set,
    .dot1x_portBasedAuthStatus_get = dal_rtl8373_dot1x_portBasedAuthStatus_get,
    .dot1x_portBasedDirection_set = dal_rtl8373_dot1x_portBasedDirection_set,
    .dot1x_portBasedDirection_get = dal_rtl8373_dot1x_portBasedDirection_get,
    .dot1x_macBasedEnable_set = dal_rtl8373_dot1x_macBasedEnable_set,
    .dot1x_macBasedEnable_get = dal_rtl8373_dot1x_macBasedEnable_get,
    .dot1x_macBasedAuthMac_add = dal_rtl8373_dot1x_macBasedAuthMac_add,
    .dot1x_macBasedAuthMac_del = dal_rtl8373_dot1x_macBasedAuthMac_del,
    .dot1x_macBasedDirection_set = dal_rtl8373_dot1x_macBasedDirection_set,
    .dot1x_macBasedDirection_get = dal_rtl8373_dot1x_macBasedDirection_get,
    .dot1x_guestVlan_set = dal_rtl8373_dot1x_guestVlan_set,
    .dot1x_guestVlan_get = dal_rtl8373_dot1x_guestVlan_get,
    .dot1x_guestVlan2Auth_set = dal_rtl8373_dot1x_guestVlan2Auth_set,
    .dot1x_guestVlan2Auth_get = dal_rtl8373_dot1x_guestVlan2Auth_get,

     /*MACsec*/
     .macsec_enable_set = dal_rtl8373_macsec_enable_set,
     .macsec_enable_get = dal_rtl8373_macsec_enable_get,
     .macsec_egress_set = dal_rtl8373_macsec_egress_set,
     .macsec_egress_get = dal_rtl8373_macsec_egress_get,
     .macsec_ingress_set = dal_rtl8373_macsec_ingress_set,
     .macsec_ingress_get = dal_rtl8373_macsec_ingress_get,
     .macsec_rxgating_set = dal_rtl8373_macsec_rxgating_set,
     .macsec_rxgating_cancel = dal_rtl8373_macsec_rxgating_cancel,
     .macsec_txgating_set = dal_rtl8373_macsec_txgating_set,
     .macsec_txgating_cancel = dal_rtl8373_macsec_txgating_cancel,
     .macsec_rxIPbypass_set = dal_rtl8373_macsec_rxIPbypass_set,
     .macsec_rxIPbypass_get = dal_rtl8373_macsec_rxIPbypass_get,
     .macsec_txIPbypass_set = dal_rtl8373_macsec_txIPbypass_set,
     .macsec_txIPbypass_get = dal_rtl8373_macsec_txIPbypass_get,
     .macsec_rxbypass_set = dal_rtl8373_macsec_rxbypass_set,
     .macsec_rxbypass_get = dal_rtl8373_macsec_rxbypass_get,
     .macsec_txbypass_set = dal_rtl8373_macsec_txbypass_set,
     .macsec_txbypass_get = dal_rtl8373_macsec_txbypass_get,
     .wrapper_int_control_set = dal_rtl8373_wrapper_int_control_set,
     .wrapper_int_control_get = dal_rtl8373_wrapper_int_control_get,
     .wrapper_int_status_set = dal_rtl8373_wrapper_int_status_set,
     .wrapper_int_status_get = dal_rtl8373_wrapper_int_status_get,
     .wrapper_mib_reset = dal_rtl8373_wrapper_mib_reset,
     .wrapper_mib_counter = dal_rtl8373_wrapper_mib_counter,
     .macsec_ipg_len_set = dal_rtl8373_macsec_ipg_len_set,
     .macsec_ipg_len_get = dal_rtl8373_macsec_ipg_len_get,
     .macsec_ipg_mode_set = dal_rtl8373_macsec_ipg_mode_set,
     .macsec_ipg_mode_get = dal_rtl8373_macsec_ipg_mode_get,
     .macsec_eth_set = dal_rtl8373_macsec_eth_set,
     .macsec_eth_get = dal_rtl8373_macsec_eth_get,
     .macsec_init = dal_rtl8373_macsec_init,

    /*SVLAN*/
    .svlan_init = dal_rtl8373_svlanInit,
    .svlan_servicePort_add = dal_rtl8373_svlanServicePort_add,
    .svlan_servicePort_get = dal_rtl8373_svlanServicePort_get,
    .svlan_servicePort_del = dal_rtl8373_svlanServicePort_del,
    .svlan_tpidEntry_set = dal_rtl8373_svlanTpid_set,
    .svlan_tpidEntry_get = dal_rtl8373_svlanTpid_get,
    .svlan_priorityRef_set = dal_rtl8373_svlanPriRef_set,
    .svlan_priorityRef_get = dal_rtl8373_svlanPriRef_get,
   // .svlan_memberPortEntry_set = dal_rtl8373_svlanMbrrPortEntry_set,
   // .svlan_memberPortEntry_get = dal_rtl8373_svlanmemberPortEntry_get,
    //.svlan_memberPortEntry_adv_set = NULL,
    //.svlan_memberPortEntry_adv_get = NULL,
    .svlan_defaultSvlan_set = dal_rtl8373_svlanDfltSvlan_set,
    .svlan_defaultSvlan_get = dal_rtl8373_svlanDfltSvlan_get,
    .svlan_c2s_add = dal_rtl8373_svlanC2S_add,
    .svlan_c2s_del = dal_rtl8373_svlanC2S_del,
    .svlan_c2s_get = dal_rtl8373_svlanC2S_get,
    .svlan_untag_action_set = dal_rtl8373_svlanUntagAction_set,
    .svlan_untag_action_get = dal_rtl8373_svlanUntagAction_get,
    .svlan_trapPri_set = dal_rtl8373_svlanTrapPri_set,
    .svlan_trapPri_get = dal_rtl8373_svlanTrapPri_get,
    .svlan_unassign_action_set = dal_rtl8373_svlanUnassignAction_set,
    .svlan_unassign_action_get = dal_rtl8373_svlanUnassignAction_get,
    .svlan_trapCpuMsk_set = dal_rtl8373_svlanTrapCpumsk_set,
    .svlan_trapCpuMsk_get = dal_rtl8373_svlanTrapCpumsk_get,
    
	/*NIC*/
    .nic_rst_set = dal_rtl8373_nic_rst_set,
    .nic_txStopAddr_set =  dal_rtl8373_nic_txStopAddr_set,
    .nic_txStopAddr_get = dal_rtl8373_nic_txStopAddr_get,
    .nic_rxStopAddr_set = dal_rtl8373_nic_rxStopAddr_set,
    .nic_rxStopAddr_get = dal_rtl8373_nic_rxStopAddr_get,
    .nic_swRxCurPktAddr_get = dal_rtl8373_nic_swRxCurPktAddr_get,
    .nic_rxReceivedPktLen_get = dal_rtl8373_nic_rxReceivedPktLen_get,
    .nic_txAvailSpace_get = dal_rtl8373_nic_txAvailSpace_get,
    .nic_moduleEn_set = dal_rtl8373_nic_moduleEn_set,
    .nic_moduleEn_get = dal_rtl8373_nic_moduleEn_get,
    .nic_rxEn_set = dal_rtl8373_nic_rxEn_set,
    .nic_rxEn_get = dal_rtl8373_nic_rxEn_get,
    .nic_txEn_set = dal_rtl8373_nic_txEn_set,
    .nic_txEn_get = dal_rtl8373_nic_txEn_get,
    .nic_rxRemoveCrc_set = dal_rtl8373_nic_rxRemoveCrc_set,
    .nic_rxRemoveCrc_get = dal_rtl8373_nic_rxRemoveCrc_get,
    .nic_rxPaddingEn_set = dal_rtl8373_nic_rxPaddingEn_set,
    .nic_rxPaddingEn_get = dal_rtl8373_nic_rxPaddingEn_get,
    .nic_rxFreeSpaceThd_set = dal_rtl8373_nic_rxFreeSpaceThd_set,
    .nic_rxFreeSpaceThd_get = dal_rtl8373_nic_rxFreeSpaceThd_get,
    .nic_rxCrcErrEn_set = dal_rtl8373_nic_rxCrcErrEn_set,
    .nic_rxCrcErrEn_get = dal_rtl8373_nic_rxCrcErrEn_get,
    .nic_rxL3CrcErrEn_set = dal_rtl8373_nic_rxL3CrcErrEn_set,
    .nic_rxL3CrcErrEn_get = dal_rtl8373_nic_rxL3CrcErrEn_get,
    .nic_rxL4CrcErrEn_set = dal_rtl8373_nic_rxL4CrcErrEn_set,
    .nic_rxL4CrcErrEn_get = dal_rtl8373_nic_rxL4CrcErrEn_get,
    .nic_rxArpEn_set = dal_rtl8373_nic_rxArpEn_set,
    .nic_rxArpEn_get = dal_rtl8373_nic_rxArpEn_get,
    .nic_rxAllPktEn_set = dal_rtl8373_nic_rxAllPktEn_set,
    .nic_rxAllPktEn_get = dal_rtl8373_nic_rxAllPktEn_get,
    .nic_rxPhyPktSel_set = dal_rtl8373_nic_rxPhyPktSel_set,
    .nic_rxPhyPktSel_get = dal_rtl8373_nic_rxPhyPktSel_get,
    .nic_rxMultiPktEn_set = dal_rtl8373_nic_rxMultiPktEn_set,
    .nic_rxMultiPktEn_get = dal_rtl8373_nic_rxMultiPktEn_get,
    .nic_rxBcPktEn_set = dal_rtl8373_nic_rxBcPktEn_set,
    .nic_rxBcPktEn_get = dal_rtl8373_nic_rxBcPktEn_get,
    .nic_mcHashFltrEn_set = dal_rtl8373_nic_mcHashFltrEn_set,
    .nic_mcHashFltrEn_get = dal_rtl8373_nic_mcHashFltrEn_get,
    .nic_phyPktHashFltrEn_set = dal_rtl8373_nic_phyPktHashFltrEn_set,
    .nic_phyPktHashFltrEn_get = dal_rtl8373_nic_phyPktHashFltrEn_get,
    .nic_mcHashTblVal_set = dal_rtl8373_nic_mcHashTblVal_set,
    .nic_mcHashTblVal_get = dal_rtl8373_nic_mcHashTblVal_get,
    .nic_phyPktHashTblVal_set = dal_rtl8373_nic_phyPktHashTblVal_set,
    .nic_phyPktHashTblVal_get = dal_rtl8373_nic_phyPktHashTblVal_get,
    .nic_rxMTU_set = dal_rtl8373_nic_rxMTU_set,
    .nic_rxMTU_get = dal_rtl8373_nic_rxMTU_get,
    .nic_loopbackEn_set = dal_rtl8373_nic_loopbackEn_set,
    .nic_loopbackEn_get = dal_rtl8373_nic_loopbackEn_get,
    .nic_interruptEn_set = dal_rtl8373_nic_interruptEn_set,
    .nic_interruptEn_get = dal_rtl8373_nic_interruptEn_get,
    .nic_interruptStatus_get = dal_rtl8373_nic_interruptStatus_get,
    .nic_interruptStatus_clear = dal_rtl8373_nic_interruptStatus_clear,

    /* IGMP */
    .igmp_init = dal_rtl8373_igmp_init,
    .igmp_state_set = dal_rtl8373_igmp_state_set,
    .igmp_state_get = dal_rtl8373_igmp_state_get,
    .igmp_static_router_port_set = dal_rtl8373_igmp_static_router_port_set,
    .igmp_static_router_port_get = dal_rtl8373_igmp_static_router_port_get,
    .igmp_protocol_set = dal_rtl8373_igmp_protocol_set,
    .igmp_protocol_get = dal_rtl8373_igmp_protocol_get,
    .igmp_fastLeave_set = dal_rtl8373_igmp_fastLeave_set,
    .igmp_fastLeave_get = dal_rtl8373_igmp_fastLeave_get,
    .igmp_maxGroup_set = dal_rtl8373_igmp_maxGroup_set,
    .igmp_maxGroup_get = dal_rtl8373_igmp_maxGroup_get,
    .igmp_currentGroup_get = dal_rtl8373_igmp_currentGroup_get,
    .igmp_tableFullAction_set = dal_rtl8373_igmp_tableFullAction_set,
    .igmp_tableFullAction_get = dal_rtl8373_igmp_tableFullAction_get,
    .igmp_checksumErrorAction_set = dal_rtl8373_igmp_checksumErrorAction_set,
    .igmp_checksumErrorAction_get = dal_rtl8373_igmp_checksumErrorAction_get,
    .igmp_leaveTimer_set = dal_rtl8373_igmp_leaveTimer_set,
    .igmp_leaveTimer_get = dal_rtl8373_igmp_leaveTimer_get,
    .igmp_queryInterval_set = dal_rtl8373_igmp_queryInterval_set,
    .igmp_queryInterval_get = dal_rtl8373_igmp_queryInterval_get,
    .igmp_robustness_set = dal_rtl8373_igmp_robustness_set,
    .igmp_robustness_get = dal_rtl8373_igmp_robustness_get,
    .igmp_dynamicRouterPortAllow_set = dal_rtl8373_igmp_dynamicRouterPortAllow_set,
    .igmp_dynamicRouterPortAllow_get = dal_rtl8373_igmp_dynamicRouterPortAllow_get,
    .igmp_dynamicRouterPort_get = dal_rtl8373_igmp_dynamicRouterPort_get,
    .igmp_suppressionEnable_set = dal_rtl8373_igmp_suppressionEnable_set,
    .igmp_suppressionEnable_get = dal_rtl8373_igmp_suppressionEnable_get,
    .igmp_portRxPktEnable_set = dal_rtl8373_igmp_portRxPktEnable_set,
    .igmp_portRxPktEnable_get = dal_rtl8373_igmp_portRxPktEnable_get,
    .igmp_groupInfo_get = dal_rtl8373_igmp_groupInfo_get,
    .igmp_ReportLeaveFwdAction_set = dal_rtl8373_igmp_ReportLeaveFwdAction_set,
    .igmp_ReportLeaveFwdAction_get = dal_rtl8373_igmp_ReportLeaveFwdAction_get,
    .igmp_dropLeaveZeroEnable_set = dal_rtl8373_igmp_dropLeaveZeroEnable_set,
    .igmp_dropLeaveZeroEnable_get = dal_rtl8373_igmp_dropLeaveZeroEnable_get,
    .igmp_bypassGroupRange_set = dal_rtl8373_igmp_bypassGroupRange_set,
    .igmp_bypassGroupRange_get = dal_rtl8373_igmp_bypassGroupRange_get,
#if 1
    /*RLDP*/
    .rldp_config_set = dal_rtl8373_rldp_config_set,
    .rldp_config_get = dal_rtl8373_rldp_config_get,
    .rldp_portConfig_set = dal_rtl8373_rldp_portConfig_set,
    .rldp_portConfig_get = dal_rtl8373_rldp_portConfig_get,
    .rldp_status_get = NULL,
    .rldp_portStatus_get = dal_rtl8373_rldp_portStatus_get,
    .rldp_portStatus_set = NULL,
    .rldp_portLoopPair_get = dal_rtl8373_rldp_portLoopPair_get,
#endif

#if 1
    /*trunk*/
    .trunk_port_set = dal_rtl8373_trunk_port_set,
    .trunk_port_get = dal_rtl8373_trunk_port_get,
    .trunk_distributionAlgorithm_set = dal_rtl8373_trunk_distributionAlgorithm_set,
    .trunk_distributionAlgorithm_get = dal_rtl8373_trunk_distributionAlgorithm_get,
    .trunk_trafficSeparate_set = dal_rtl8373_trunk_trafficSeparate_set,
    .trunk_trafficSeparate_get = dal_rtl8373_trunk_trafficSeparate_get,
    .trunk_mode_set = NULL,
    .trunk_mode_get = NULL,
    .trunk_trafficPause_set = dal_rtl8373_trunk_trafficPause_set,
    .trunk_trafficPause_get = dal_rtl8373_trunk_trafficPause_get,
    .trunk_hashMappingTable_set = NULL,
    .trunk_hashMappingTable_get = NULL,
    .trunk_portQueueEmpty_get = dal_rtl8373_trunk_portQueueEmpty_get,
#endif

    /* l2 */
    .l2_init = dal_rtl8373_l2_init,
    .l2_addr_add = dal_rtl8373_l2_addr_add,
    .l2_addr_get = dal_rtl8373_l2_addr_get,
    .l2_addr_next_get = dal_rtl8373_l2_addr_next_get,
    .l2_addr_del = dal_rtl8373_l2_addr_del,
    .l2_mcastAddr_add = dal_rtl8373_l2_mcastAddr_add,
    .l2_mcastAddr_get = dal_rtl8373_l2_mcastAddr_get,
    .l2_mcastAddr_next_get = dal_rtl8373_l2_mcastAddr_next_get,
    .l2_mcastAddr_del = dal_rtl8373_l2_mcastAddr_del,
    .l2_ipMcastAddr_add = dal_rtl8373_l2_ipMcastAddr_add,
    .l2_ipMcastAddr_get = dal_rtl8373_l2_ipMcastAddr_get,
    .l2_ipMcastAddr_next_get = dal_rtl8373_l2_ipMcastAddr_next_get,
    .l2_ipMcastAddr_del = dal_rtl8373_l2_ipMcastAddr_del,
    .l2_ucastAddr_flush = NULL,
    .l2_table_clear = dal_rtl8373_l2_table_clear,
    .l2_table_clearStatus_get = dal_rtl8373_l2_table_clearStatus_get,
    .l2_flushLinkDownPortAddrEnable_set = dal_rtl8373_l2_flushLinkDownPortAddrEnable_set,
    .l2_flushLinkDownPortAddrEnable_get = dal_rtl8373_l2_flushLinkDownPortAddrEnable_get,
    .l2_agingEnable_set = dal_rtl8373_l2_agingEnable_set,
    .l2_agingEnable_get = dal_rtl8373_l2_agingEnable_get,
    .l2_limitLearningCnt_set = dal_rtl8373_l2_limitLearningCnt_set,
    .l2_limitLearningCnt_get = dal_rtl8373_l2_limitLearningCnt_get,
    .l2_limitSystemLearningCnt_set = dal_rtl8373_l2_limitSystemLearningCnt_set,
    .l2_limitSystemLearningCnt_get = dal_rtl8373_l2_limitSystemLearningCnt_get,
    .l2_limitLearningCntAction_set = dal_rtl8373_l2_limitLearningCntAction_set,
    .l2_limitLearningCntAction_get = dal_rtl8373_l2_limitLearningCntAction_get,
    .l2_limitSystemLearningCntAction_set = dal_rtl8373_l2_limitSystemLearningCntAction_set,
    .l2_limitSystemLearningCntAction_get = dal_rtl8373_l2_limitSystemLearningCntAction_get,
    .l2_limitSystemLearningCntPortMask_set = dal_rtl8373_l2_limitSystemLearningCntPortMask_set,
    .l2_limitSystemLearningCntPortMask_get = dal_rtl8373_l2_limitSystemLearningCntPortMask_get,
    .l2_learningCnt_get = dal_rtl8373_l2_learningCnt_get,
    .l2_floodPortMask_set = dal_rtl8373_l2_floodPortMsk_set,
    .l2_floodPortMask_get = dal_rtl8373_l2_floodPortMsk_get,
    .l2_localPktPermit_set = dal_rtl8373_srcPortPermit_set,
    .l2_localPktPermit_get = dal_rtl8373_srcPortPermit_get,
    .l2_aging_set = dal_rtl8373_l2_ageout_timer_set,
    .l2_aging_get = dal_rtl8373_l2_ageout_timer_get,
    .l2_ipMcastAddrLookup_set = dal_rtl8373_l2_ipMcastAddrLookup_set,
    .l2_ipMcastAddrLookup_get = dal_rtl8373_l2_ipMcastAddrLookup_get,
    .l2_ipMcastForwardRouterPort_set = NULL,
    .l2_ipMcastForwardRouterPort_get = NULL,
    .l2_ipMcastGroupEntry_add = dal_rtl8373_l2_ipMcastGroupEntry_add,
    .l2_ipMcastGroupEntry_del = dal_rtl8373_l2_ipMcastGroupEntry_del,
    .l2_ipMcastGroupEntry_get = dal_rtl8373_l2_ipMcastGroupEntry_get,
    .l2_entry_get = dal_rtl8373_l2_entry_get,
    .l2_lookupHitIsolationAction_set = dal_rtl8373_l2_lookupHitIsolationAction_set,
    .l2_lookupHitIsolationAction_get = dal_rtl8373_l2_lookupHitIsolationAction_get,
    .l2_unknownUnicastPktAction_set = dal_rtl8373_l2_unknUc_action_set,
    .l2_unknownUnicastPktAction_get = dal_rtl8373_l2_unknUc_action_get,
    .l2_unknownMulticastPktAction_set = dal_rtl8373_l2_unknMc_action_set,
    .l2_unknownMulticastPktAction_get = dal_rtl8373_l2_unknMc_action_get,
    .l2_unknownV4McPktAction_set = dal_rtl8373_l2_unknV4Mc_action_set,
    .l2_unknownV4McPktAction_get = dal_rtl8373_l2_unknV4Mc_action_get,
    .l2_unknownV6McPktAction_set = dal_rtl8373_l2_unknV6Mc_action_set,
    .l2_unknownV6McPktAction_get = dal_rtl8373_l2_unknV6Mc_action_get,

    /*GPIO*/
    .gpio_muxSel_set = dal_rtl8373_gpio_muxSel_set,
    .gpio_muxSel_get = dal_rtl8373_gpio_muxSel_get,
    .gpio_groupVal_write = dal_rtl8373_gpio_groupVal_write,
    .gpio_groupVal_read = dal_rtl8373_gpio_groupVal_read,
    .gpio_pinVal_write = dal_rtl8373_gpio_pinVal_write,
    .gpio_pinVal_read = dal_rtl8373_gpio_pinVal_read,
    .gpio_pinDir_set = dal_rtl8373_gpio_pinDir_set,
    .gpio_pinDir_get = dal_rtl8373_gpio_pinDir_get,
    .gpio_groupDir_get = dal_rtl8373_gpio_groupDir_get,

    /*I2C*/
	.i2c_init = dal_rtl8373_i2c_init,
	.i2c_readMode_set = dal_rtl8373_i2c_readMode_set,
	.i2c_readMode_get = dal_rtl8373_i2c_readMode_get,
	.i2c_gpioPinGroup_set = dal_rtl8373_i2c_gpioPinGroup_set,
	.i2c_gpioPinGroup_get = dal_rtl8373_i2c_gpioPinGroup_get,
	.i2c_data_read = dal_rtl8373_i2c_data_read,
	.i2c_data_write = dal_rtl8373_i2c_data_write,

	/*Rate : ingress BW & egress queue BW*/
	.rate_igrBwCtrlPortEn_set = dal_rtl8373_rate_igrBwCtrlPortEn_set,
    .rate_igrBwCtrlPortEn_get = dal_rtl8373_rate_igrBwCtrlPortEn_get,
	.rate_igrBwCtrlRate_set = dal_rtl8373_rate_igrBwCtrlRate_set,
    .rate_igrBwCtrlRate_get = dal_rtl8373_rate_igrBwCtrlRate_get,
	.rate_igrBwCtrlIfg_set = dal_rtl8373_rate_igrBwCtrlIfg_set,
    .rate_igrBwCtrlIfg_get = dal_rtl8373_rate_igrBwCtrlIfg_get,
	.rate_igrBwCtrlCongestSts_get = dal_rtl8373_rate_igrBwCtrlCongestSts_get,
	.rate_egrBandwidthCtrlRate_set = dal_rtl8373_rate_egrBwCtrlRate_set,
    .rate_egrBandwidthCtrlRate_get = dal_rtl8373_rate_egrBwCtrlRate_get,
    .rate_egrQueueBwCtrlEnable_set = dal_rtl8373_rate_egrQueueMaxBwEn_set,
    .rate_egrQueueBwCtrlEnable_get = dal_rtl8373_rate_egrQueueMaxBwEn_get,
    .rate_egrQueueBwCtrlRate_set = dal_rtl8373_rate_egrQueueMaxBwRate_set,
    .rate_egrQueueBwCtrlRate_get = dal_rtl8373_rate_egrQueueMaxBwRate_get,


    /* eee */
    .eee_init = dal_rtl8373_eee_init,
    .eee_macForceSpeedEn_set = dal_rtl8373_eee_macForceSpeedEn_set,
    .eee_macForceSpeedEn_get =  dal_rtl8373_eee_macForceSpeedEn_get,
    .eee_macForceAllSpeedEn_get = dal_rtl8373_eee_macForceAllSpeedEn_get,
    .eee_portTxRxEn_set = dal_rtl8373_eee_portTxRxEn_set,
    .eee_portTxRxEn_get = dal_rtl8373_eee_portTxRxEn_get,

	/* interrupt */
    .int_enable = dal_rtl8373_IE_set,
    .int_polarity_set = dal_rtl8373_intMode_set,
    .int_polarity_get = dal_rtl8373_intMode_get,
    .int_control_set = dal_rtl8373_portIntIMR_set,
    .int_control_get = dal_rtl8373_portIntIMR_get,
    .int_miscIMR_set = dal_rtl8373_miscIMR_set,
    .int_miscIMR_get = dal_rtl8373_miscIMR_get,
    .int_miscISR_clear = dal_rtl8373_miscISR_clear,
    .int_miscISR_get = dal_rtl8373_miscISR_get,
	/* mib */
    .stat_global_reset = dal_rtl8373_globalMib_rst,
    .stat_port_reset = dal_rtl8373_portMib_rst,
    .stat_port_get = dal_rtl8373_portMib_read,
    .stat_lengthMode_set = dal_rtl8373_mibLength_set,
    .stat_lengthMode_get = dal_rtl8373_mibLength_get,

	/*RMA*/
	
	.rma_set = dal_rtl8373_asicRma_set,
	.rma_get = dal_rtl8373_asicRma_get,

	/*LED*/
	.led_blinkRate_set = dal_rtl8373_led_blinkRate_set,
    .led_blinkRate_get = dal_rtl8373_led_blinkRate_get,
    .led_groupConfig_set = dal_rtl8373_led_config_set,
    .led_portSelConfig_set = dal_rtl8373_portLedConfig_set,
    .led_portSelConfig_get = dal_rtl8373_portLedConfig_get,


	/*port*/
    .port_macForceLink_set = dal_rtl8373_portFrcAbility_set,
    .port_macForceLink_get = dal_rtl8373_portFrcAbility_get,
    .port_macStatus_get = dal_rtl8373_portStatus_get,
    .port_macLocalLoopbackEnable_set = dal_rtl8373_portLoopbackEn_set,
    .port_macLocalLoopbackEnable_get = dal_rtl8373_portLoopbackEn_get,
    .port_backpressureEnable_set = dal_rtl8373_portBackpressureEn_set,
    .port_backpressureEnable_get = dal_rtl8373_portBackpressureEn_get,
    .port_rtct_init = dal_rtl8373_rtct_init,
    .port_rtct_start = dal_rtl8373_rtct_start,
    .port_rtctResult_get = dal_rtl8373_rtct_status_get,
    .port_sdsMode_set = dal_rtl8373_sdsMode_set,
    .port_sdsMode_get = dal_rtl8373_sdsMode_get,
    .port_sdsNway_get = dal_rtl8373_port_sdsNway_get,
    .port_sdsNway_set = dal_rtl8373_port_sdsNway_set,
    .port_extphyid_set = dal_rtl8373_port_extphyid_set,
    .port_extphyid_get = dal_rtl8373_port_extphyid_get,
    
	/*phy*/
    .phy_common_c45_an_restart = dal_rlt8373_phy_common_c45_an_restart,
    .phy_common_c45_autoNegoEnable_get = dal_rlt8373_phy_common_c45_autoNegoEnable_get,
    .phy_common_c45_autoNegoEnable_set = dal_rlt8373_phy_common_c45_autoNegoEnable_set,
    .phy_autoNegoAbility_set = dal_rlt8373_phy_autoNegoAbility_set,
    .phy_common_c45_autoSpeed_set = dal_rlt8373_phy_common_c45_autoSpeed_set,

    .phy_common_c45_speed_set = dal_rlt8373_phy_common_c45_speed_set,
    .phy_common_c45_speed_get = dal_rlt8373_phy_common_c45_speed_get,
    .phy_common_c45_enable_set = dal_rlt8373_phy_common_c45_enable_set,
    .phy_common_c45_enable_get = dal_rlt8373_phy_common_c45_enable_get,
    .phy_common_c45_duplex_set = dal_rlt8373_phy_common_c45_duplex_set,
    .phy_common_c45_duplex_get = dal_rlt8373_phy_common_c45_duplex_get,
    .phy_common_c45_speedDuplexStatusResReg_get = dal_rlt8373_phy_common_c45_speedDuplexStatusResReg_get,
    
#if 0 

    /*leaky*/
    .leaky_vlan_set = dal_rtl8373_leaky_vlan_set,
    .leaky_vlan_get = dal_rtl8373_leaky_vlan_get,
    .leaky_portIsolation_set = dal_rtl8373_leaky_portIsolation_set,
    .leaky_portIsolation_get = dal_rtl8373_leaky_portIsolation_get,

    /* led */
    .led_enable_set = dal_rtl8373_led_enable_set,
    .led_enable_get = dal_rtl8373_led_enable_get,
    .led_operation_set = dal_rtl8373_led_operation_set,
    .led_operation_get = dal_rtl8373_led_operation_get,
    .led_modeForce_set = dal_rtl8373_led_modeForce_set,
    .led_modeForce_get = dal_rtl8373_led_modeForce_get,
    .led_blinkRate_set = dal_rtl8373_led_blinkRate_set,
    .led_blinkRate_get = dal_rtl8373_led_blinkRate_get,
    .led_groupConfig_set = dal_rtl8373_led_groupConfig_set,
    .led_groupConfig_get = dal_rtl8373_led_groupConfig_get,
    .led_groupAbility_set = dal_rtl8373_led_groupAbility_set,
    .led_groupAbility_get = dal_rtl8373_led_groupAbility_get,
    .led_serialMode_set = dal_rtl8373_led_serialMode_set,
    .led_serialMode_get = dal_rtl8373_led_serialMode_get,
    .led_OutputEnable_set = dal_rtl8373_led_OutputEnable_set,
    .led_OutputEnable_get = dal_rtl8373_led_OutputEnable_get,
    .led_serialModePortmask_set = NULL,
    .led_serialModePortmask_get = NULL,

    /* oam */
    .oam_init = NULL,
    .oam_state_set = NULL,
    .oam_state_get = NULL,
    .oam_parserAction_set = NULL,
    .oam_parserAction_get = NULL,
    .oam_multiplexerAction_set = NULL,
    .oam_multiplexerAction_get = NULL,

    /* stat */
    .stat_global_reset = dal_rtl8373_stat_global_reset,
    .stat_port_reset = dal_rtl8373_stat_port_reset,
    .stat_queueManage_reset = dal_rtl8373_stat_queueManage_reset,
    .stat_global_get = dal_rtl8373_stat_global_get,
    .stat_global_getAll = dal_rtl8373_stat_global_getAll,
    .stat_port_get = dal_rtl8373_stat_port_get,
    .stat_port_getAll = dal_rtl8373_stat_port_getAll,
    .stat_logging_counterCfg_set = dal_rtl8373_stat_logging_counterCfg_set,
    .stat_logging_counterCfg_get = dal_rtl8373_stat_logging_counterCfg_get,
    .stat_logging_counter_reset = dal_rtl8373_stat_logging_counter_reset,
    .stat_logging_counter_get = dal_rtl8373_stat_logging_counter_get,
    .stat_lengthMode_set = dal_rtl8373_stat_lengthMode_set,
    .stat_lengthMode_get = dal_rtl8373_stat_lengthMode_get,


    /* interrupt */
    .int_polarity_set = dal_rtl8373_int_polarity_set,
    .int_polarity_get = dal_rtl8373_int_polarity_get,
    .int_control_set = dal_rtl8373_int_control_set,
    .int_control_get = dal_rtl8373_int_control_get,
    .int_status_set = dal_rtl8373_int_status_set,
    .int_status_get = dal_rtl8373_int_status_get,
    .int_advanceInfo_get = dal_rtl8373_int_advanceInfo_get,

    /* port */
    .port_phyAutoNegoAbility_set = dal_rtl8373_port_phyAutoNegoAbility_set,
    .port_phyAutoNegoAbility_get = dal_rtl8373_port_phyAutoNegoAbility_get,
    .port_phyForceModeAbility_set = dal_rtl8373_port_phyForceModeAbility_set,
    .port_phyForceModeAbility_get = dal_rtl8373_port_phyForceModeAbility_get,
    .port_phyStatus_get = dal_rtl8373_port_phyStatus_get,
    
    .port_macForceLinkExt_set = dal_rtl8373_port_macForceLinkExt_set,
    .port_macForceLinkExt_get = dal_rtl8373_port_macForceLinkExt_get,
    
    
    .port_phyReg_set = dal_rtl8373_port_phyReg_set,
    .port_phyReg_get = dal_rtl8373_port_phyReg_get,
    
    .port_adminEnable_set = dal_rtl8373_port_adminEnable_set,
    .port_adminEnable_get = dal_rtl8373_port_adminEnable_get
    .port_rgmiiDelayExt_set = dal_rtl8373_port_rgmiiDelayExt_set,
    .port_rgmiiDelayExt_get = dal_rtl8373_port_rgmiiDelayExt_get,
    .port_phyEnableAll_set = dal_rtl8373_port_phyEnableAll_set,
    .port_phyEnableAll_get = dal_rtl8373_port_phyEnableAll_get,
    .port_efid_set = NULL,
    .port_efid_get = NULL,
    .port_phyComboPortMedia_set = dal_rtl8373_port_phyComboPortMedia_set,
    .port_phyComboPortMedia_get = dal_rtl8373_port_phyComboPortMedia_get,
    .port_rtctEnable_set = NULL,
    .port_rtctDisable_set = NULL,
    .port_rtctResult_get = NULL,
    .port_sds_reset = NULL,
    .port_sgmiiLinkStatus_get = dal_rtl8373_port_sgmiiLinkStatus_get,
    .port_sgmiiNway_set = dal_rtl8373_port_sgmiiNway_set,
    .port_sgmiiNway_get = dal_rtl8373_port_sgmiiNway_get,
    .port_fiberAbilityExt_set = NULL,
    .port_fiberAbilityExt_get = NULL,

#endif
    .port_autoDos_set = dal_rtl8373_asicDos_set,
    .port_autoDos_get = dal_rtl8373_asicDos_get,

 /*PTP*/
	.time_init = dal_rtl8373_ptp_init,
	.time_portPtpbypassptpEn_get = dal_rtl8373_bypassptpEn_get,
	.time_portPtpbypassptpEn_set = dal_rtl8373_bypassptpEn_set,
	.time_portPtpEnable_get = NULL,
	.time_portPtpEnable_set = NULL,
	.time_portPtpOper_triger =dal_rtl8373_ptp_Oper_triger,
	.time_portRefTime_get=dal_rtl8373_ptp_refTime_get,
	.time_portRefTime_set=dal_rtl8373_ptp_refTime_set,
	.time_portRefTimeAdjust_set = dal_rtl8373_ptp_refTimeAdjust_set,
	.time_portPtpVlanTpid_get = dal_rtl8373_ptp_tpid_get,
	.time_portPtpVlanTpid_set = dal_rtl8373_ptp_tpid_set,
	.time_portPtpOper_get = dal_rtl8373_ptp_Oper_get,
	.time_portPtpOper_set = dal_rtl8373_ptp_Oper_set,
	.time_portPtpLatchTime_get = dal_rtl8373_ptp_LatchTime_get,
	.time_portPtpRefTimeFreqCfg_get = dal_rtl8373_ptp_RefTimeFreqCfg_get,
	.time_portPtpRefTimeFreqCfg_set = dal_rtl8373_ptp_RefTimeFreqCfg_set,
	.time_portPtpClkSrcCtrl_set = dal_rtl8373_ptp_ClkSrcCtrl_set,
	.time_portPtpClkSrcCtrl_get = dal_rtl8373_ptp_ClkSrcCtrl_get,

	.time_portPtpTxTimestampFifo_get = dal_rtl8373_ptp_TxTimestampFifo_get ,
	.time_portPtp1PPSOutput_get = dal_rtl8373_ptp_1PPSOutput_get,
	.time_portPtp1PPSOutput_set = dal_rtl8373_ptp_1PPSOutput_set,
	.time_portPtpClockOutput_get = dal_rtl8373_ptp_ClockOutput_get,
	.time_portPtpClockOutput_set = dal_rtl8373_ptp_ClockOutput_set,
	.time_portPtpToddelay_set =dal_rtl8373_ptp_toddelay_set,
	.time_portPtpToddelay_get =dal_rtl8373_ptp_toddelay_get,
	.time_portPtpPortctrl_get = dal_rtl8373_ptp_portctrl_get,
	.time_portPtpPortctrl_set = dal_rtl8373_ptp_portctrl_set,
	.ptp_intControl_set = dal_rtl8373_ptp_intControl_set,
	.ptp_intControl_get = dal_rtl8373_ptp_intControl_get,
	.ptp_portTrap_set = dal_rtl8373_ptp_portTrap_set,
	.ptp_portTrap_get = dal_rtl8373_ptp_portTrap_get,
       .ptp_intStatus_get = dal_rtl8373_ptp_intStatus_get,



};

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */


/* Module Name    :  */

/* Function Name:
 *      dal_rtl8373_mapper_get
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
dal_mapper_t *dal_rtl8373_mapper_get(void)
{

    return &dal_rtl8373_mapper;
} /* end of dal_rtl8373_mapper_get */

