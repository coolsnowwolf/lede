ccflags-y = -I$(src)/../mt7603_wifi/include \
		-I$(src)/../mt7603_wifi/ate/include
DRV_NAME = mt7603e
RT_WIFI_DIR = ../mt7603_wifi
WFLAGS += -Wno-error=incompatible-pointer-types

########################################################
# Common files
########################################################
cmm_objs := $(RT_WIFI_DIR)/common/crypt_md5.o\
						$(RT_WIFI_DIR)/common/crypt_sha2.o\
						$(RT_WIFI_DIR)/common/crypt_hmac.o\
						$(RT_WIFI_DIR)/common/crypt_aes.o\
						$(RT_WIFI_DIR)/common/crypt_arc4.o\
						$(RT_WIFI_DIR)/common/mlme.o\
						$(RT_WIFI_DIR)/common/cmm_wep.o\
						$(RT_WIFI_DIR)/common/action.o\
            $(RT_WIFI_DIR)/common/ba_action.o\
            $(RT_WIFI_DIR)/mgmt/mgmt_ht.o\
						$(RT_WIFI_DIR)/common/cmm_data.o\
						$(RT_WIFI_DIR)/common/rtmp_init.o\
						$(RT_WIFI_DIR)/common/rtmp_init_inf.o\
						$(RT_WIFI_DIR)/common/cmm_tkip.o\
						$(RT_WIFI_DIR)/common/cmm_aes.o\
						$(RT_WIFI_DIR)/common/cmm_sync.o\
						$(RT_WIFI_DIR)/common/eeprom.o\
						$(RT_WIFI_DIR)/common/cmm_sanity.o\
						$(RT_WIFI_DIR)/common/cmm_info.o\
						$(RT_WIFI_DIR)/common/cmm_cfg.o\
						$(RT_WIFI_DIR)/common/cmm_wpa.o\
						$(RT_WIFI_DIR)/common/cmm_radar.o\
						$(RT_WIFI_DIR)/common/spectrum.o\
						$(RT_WIFI_DIR)/common/rtmp_timer.o\
						$(RT_WIFI_DIR)/common/rt_channel.o\
						$(RT_WIFI_DIR)/common/rt_os_util.o\
						$(RT_WIFI_DIR)/common/cmm_profile.o\
						$(RT_WIFI_DIR)/common/scan.o\
						$(RT_WIFI_DIR)/common/vendor.o\
						$(RT_WIFI_DIR)/common/cmm_cmd.o\
						$(RT_WIFI_DIR)/common/sys_log.o\
						$(RT_WIFI_DIR)/common/txpower.o\
						$(RT_WIFI_DIR)/common/op_class.o\
						$(RT_WIFI_DIR)/chips/rtmp_chip.o\
						$(RT_WIFI_DIR)/mgmt/mgmt_hw.o\
						$(RT_WIFI_DIR)/mgmt/mgmt_entrytb.o\
						$(RT_WIFI_DIR)/tx_rx/wdev.o\
						$(RT_WIFI_DIR)/tx_rx/wdev_tx.o\
						$(RT_WIFI_DIR)/tx_rx/wdev_rx.o\
						$(RT_WIFI_DIR)/os/linux/rt_profile.o\
						$(RT_WIFI_DIR)/mgmt/mgmt_dev.o

########################################################
# Rate adaptation related files
########################################################
rate_objs := $(RT_WIFI_DIR)/rate_ctrl/ra_ctrl.o\
						$(RT_WIFI_DIR)/rate_ctrl/alg_legacy.o

ifeq ($(CONFIG_NEW_RATE_ADAPT_SUPPORT),y)
    EXTRA_CFLAGS += -DNEW_RATE_ADAPT_SUPPORT
    rate_objs += $(RT_WIFI_DIR)/rate_ctrl/alg_grp.o

    ifeq ($(CONFIG_AGS_SUPPORT),y)
        EXTRA_CFLAGS += -DAGS_ADAPT_SUPPORT
        rate_objs += $(RT_WIFI_DIR)/rate_ctrl/alg_ags.o
    endif
endif


########################################################
# ASIC related files
########################################################
asic_objs := $(RT_WIFI_DIR)/phy/phy.o

ifeq ($(CONFIG_RLT_MAC),y)
    EXTRA_CFLAGS += -DRLT_MAC

    asic_objs += $(RT_WIFI_DIR)/mac/ral_nmac.o
endif

ifeq ($RTMP_MAC),y)
    EXTRA_CFLAGS += -DRTMP_MAC

    asic_objs += $(RT_WIFI_DIR)/mac/ral_omac.o\
		$(RT_WIFI_DIR)/chips/rtxx_dft.o
endif


########################################################
#   Itnterface related source files
########################################################
inf_objs :=

ifeq ($(CONFIG_RTMP_PCI_SUPPORT),y)
    EXTRA_CFLAGS += -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT

    inf_objs += $(RT_WIFI_DIR)/common/cmm_mac_pci.o\
		$(RT_WIFI_DIR)/common/cmm_data_pci.o\
		$(RT_WIFI_DIR)/os/linux/pci_main_dev.o\
		$(RT_WIFI_DIR)/os/linux/rt_rbus_pci_drv.o\
		$(RT_WIFI_DIR)/hif/hif_pci.o\
		$(RT_WIFI_DIR)/common/cmm_mac_pci.o\
		$(RT_WIFI_DIR)/common/cmm_data_pci.o\
		$(RT_WIFI_DIR)/os/linux/rt_pci_rbus.o\
		$(RT_WIFI_DIR)/os/linux/rt_rbus_pci_drv.o\
		$(RT_WIFI_DIR)/os/linux/rt_rbus_pci_util.o
endif

ifeq ($(CONFIG_RTMP_USB_SUPPORT),y)
    EXTRA_CFLAGS += -DRTMP_MAC_USB -DRTMP_USB_SUPPORT

    inf_objs += $(RT_WIFI_DIR)/common/cmm_mac_usb.o\
		$(RT_WIFI_DIR)/common/rtusb_data.o\
		$(RT_WIFI_DIR)/common/cmm_usb_io.o\
		$(RT_WIFI_DIR)/common/cmm_data_usb.o\
		$(RT_WIFI_DIR)/common/rtusb_bulk.o\
		$(RT_WIFI_DIR)/os/linux/rt_usb.o\
		$(RT_WIFI_DIR)/common/mtusb_io.o\
		$(RT_WIFI_DIR)/common/rtusb_dev_id.o\
		$(RT_WIFI_DIR)/os/linux/rt_usb_util.o\
		$(RT_WIFI_DIR)/os/linux/usb_main_dev.o
endif

########################################################
# Spec feature related files
########################################################
spec_objs += $(RT_WIFI_DIR)/common/ps.o

ifeq ($(CONFIG_UAPSD),y)
    EXTRA_CFLAGS += -DUAPSD_SUPPORT -DUAPSD_DEBUG
     spec_objs += $(RT_WIFI_DIR)/common/uapsd.o
endif

#ifeq ($(CONFIG_RLT_MAC),y)
#	spec_objs += $(RT_WIFI_DIR)/common/ral_ps.o
#endif

#ifeq ($(CONFIG_RTMP_MAC),y)
#	spec_objs += $(RT_WIFI_DIR)/common/ral_ps.o
#endif

ifeq ($(CONFIG_MT_MAC),y)
	EXTRA_CFLAGS += -DMT_PS
	spec_objs += $(RT_WIFI_DIR)/common/mt_ps.o
	spec_objs += $(RT_WIFI_DIR)/common/mt_io.o
	spec_objs += $(RT_WIFI_DIR)/tx_rx/txs.o
endif

# WSC
ifeq ($(CONFIG_WSC_INCLUDED),y)
    EXTRA_CFLAGS += -DWSC_INCLUDED -DWSC_SINGLE_TRIGGER

	ifneq ($(CONFIG_USER_GOAHEAD_HTTPD),)
        EXTRA_CFLAGS += -DSDK_GOAHEAD_HTTPD
    endif
    
    ifneq ($(CONFIG_USER_LIGHTY),)
        EXTRA_CFLAGS += -DSDK_USER_LIGHTY
    endif

    ifneq ($(CONFIG_RLT_AP_SUPPORT),)
        EXTRA_CFLAGS += -DWSC_AP_SUPPORT
    endif

    ifneq ($(CONFIG_RLT_STA_SUPPORT),)
        EXTRA_CFLAGS += -DWSC_STA_SUPPORT
    endif

    spec_objs += $(RT_WIFI_DIR)/common/wsc.o\
                    $(RT_WIFI_DIR)/common/wsc_tlv.o\
                    $(RT_WIFI_DIR)/common/crypt_dh.o\
                    $(RT_WIFI_DIR)/common/crypt_biginteger.o\
                    $(RT_WIFI_DIR)/common/wsc_ufd.o

    ifeq ($(CONFIG_WSC_V2_SUPPORT),y)
        EXTRA_CFLAGS += -DWSC_V2_SUPPORT
        spec_objs += $(RT_WIFI_DIR)/common/wsc_v2.o
    endif
endif

# VHT
#ifeq ($(CONFIG_DOT11_VHT_AC),y)
#    EXTRA_CFLAGS += -DDOT11_VHT_AC
#    spec_objs += $(RT_WIFI_DIR)/mgmt/mgmt_vht.o\
#            $(RT_WIFI_DIR)/common/vht.o
#endif

# WAPI
ifeq ($(CONFIG_WAPI_SUPPORT),y)
    EXTRA_CFLAGS += -DWAPI_SUPPORT
#    ifeq ($(CONFIG_RALINK_RT3052),y)
#        EXTRA_CFLAGS += -DSOFT_ENCRYPT
#    endif

  spec_objs += $(RT_WIFI_DIR)/common/wapi.o\
                $(RT_WIFI_DIR)/common/wapi_sms4.o\
		$(RT_WIFI_DIR)/common/wapi_crypt.o
		
endif

# ACM
ifeq ($(CONFIG_WMM_ACM_SUPPORT),y)
    EXTRA_CFLAGS += -DWMM_ACM_SUPPORT

    spec_objs += $(RT_WIFI_DIR)/common/acm_edca.o\
            $(RT_WIFI_DIR)/common/acm_comm.o\
            $(RT_WIFI_DIR)/common/acm_iocl.o
endif

#PMF
ifeq ($(CONFIG_DOT11W_PMF_SUPPORT),y)
    EXTRA_CFLAGS += -DDOT11W_PMF_SUPPORT -DSOFT_ENCRYPT
    EXTRA_CFLAGS += -DAPCLI_DOT11W_PMF_SUPPORT
    spec_objs += $(RT_WIFI_DIR)/common/pmf.o
endif

ifeq ($(CONFIG_EASY_MOD_SUPPORT),y)
    EXTRA_CFLAGS += -DWH_EZ_SETUP -DNEW_CONNECTION_ALGO -DEZ_NETWORK_MERGE_SUPPORT -DEZ_DUAL_BAND_SUPPORT -DEZ_MOD_SUPPORT
    EXTRA_CFLAGS += -DEZ_ROAM_SUPPORT -DDUAL_CHIP -DEZ_API_SUPPORT -DEZ_PUSH_BW_SUPPORT -DEZ_REGROUP_SUPPORT -DIF_UP_DOWN
    EXTRA_CFLAGS += -Idrivers/net/wireless/easy_setup/ez_mod
    spec_objs += $(RT_WIFI_DIR)/easy_setup/mt7603_driver_specific_func.o
    spec_objs += $(RT_WIFI_DIR)/easy_setup/mt7603_chip_ops_api.o
    spec_objs += $(RT_WIFI_DIR)/easy_setup/ez_hooks.o
endif

ifeq ($(CONFIG_EASY_DFS_SUPPORT),y)
ccflags-y += -DEZ_DFS_SUPPORT 
endif
ifeq ($(CONFIG_MWDS_SUPPORT),y)
    EXTRA_CFLAGS += -DMWDS -DA4_CONN -DROUTING_TAB_SUPPORT
    spec_objs += $(RT_WIFI_DIR)/common/mwds.o
    spec_objs += $(RT_WIFI_DIR)/common/a4_conn.o
    spec_objs += $(RT_WIFI_DIR)/common/routing_tab.o
endif

ifeq ($(CONFIG_PUSH_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_PUSH_SUPPORT 

    spec_objs += $(RT_WIFI_DIR)/common/event_notifier.o
endif


ifeq ($(CONFIG_AIR_MONITOR_SUPPORT),y)
    EXTRA_CFLAGS += -DAIR_MONITOR
endif

ifeq ($(CONFIG_STA_FORCE_ROAM_SUPPORT),y)
    EXTRA_CFLAGS += -DSTA_FORCE_ROAM_SUPPORT
endif

ifeq ($(CONFIG_ROAMING_ENHANCE_SUPPORT),y)
    EXTRA_CFLAGS += -DROAMING_ENHANCE_SUPPORT
endif

ifeq ($(CONFIG_WIFI_FWD_UPDATED),y)
    EXTRA_CFLAGS += -DWIFI_FWD_UPDATED
endif

ifeq ($(CONFIG_WIFI_PKT_FWD),m)
    EXTRA_CFLAGS += -DCONFIG_WIFI_PKT_FWD
endif

#ifneq ($(CONFIG_MTFWD_SUPPORT),)
    EXTRA_CFLAGS += -DMTFWD
#endif

ifeq ($(CONFIG_EVENT_NOTIFIER_SUPPORT),y)
    EXTRA_CFLAGS += -DWH_EVENT_NOTIFIER
    spec_objs += $(RT_WIFI_DIR)/common/event_notifier.o
endif

# 11N BSS CoEx
ifeq ($(CONFIG_DOT11N_DRAFT3),y)
    EXTRA_CFLAGS += -DDOT11N_DRAFT3
endif

# LLTD
ifeq ($(CONFIG_LLTD_SUPPORT),y)
    EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

# FT
ifeq ($(CONFIG_DOT11R_FT_SUPPORT),y)
EXTRA_CFLAGS += -DDOT11R_FT_SUPPORT
spec_objs += $(RT_WIFI_DIR)/common/ft.o\
                    $(RT_WIFI_DIR)/common/ft_tlv.o\
                    $(RT_WIFI_DIR)/common/ft_iocl.o\
                    $(RT_WIFI_DIR)/common/ft_rc.o\
                    $(RT_WIFI_DIR)/ap/ap_ftkd.o
endif

# RR
ifeq ($(CONFIG_DOT11K_RRM_SUPPORT),y)
	EXTRA_CFLAGS += -DDOT11K_RRM_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT -DAPPLE_11K_IOT
spec_objs += $(RT_WIFI_DIR)/common/rrm_tlv.o\
                    $(RT_WIFI_DIR)/common/rrm_sanity.o\
                    $(RT_WIFI_DIR)/common/rrm.o
endif

# WNM
ifeq ($(CONFIG_WNM_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_DOT11V_WNM 

    spec_objs += $(RT_WIFI_DIR)/common/wnm.o
endif

ifeq ($(CONFIG_HOTSPOT),y)
	EXTRA_CFLAGS += -DDOT11U_INTERWORKING -DCONFIG_DOT11V_WNM -DCONFIG_HOTSPOT -DWAPP_SUPPORT

	spec_objs += $(RT_WIFI_DIR)/common/wnm.o\
				$(RT_WIFI_DIR)/common/gas.o\
				$(RT_WIFI_DIR)/common/hotspot.o\
				$(RT_WIFI_DIR)/wapp/wapp.o
ifeq ($(CONFIG_PASSPOINT_R2),y)
ccflags-y += -DCONFIG_HOTSPOT_R2
endif
endif

ifeq ($(CONFIG_SNIFFER_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_SNIFFER_SUPPORT

    spec_objs += $(RT_WIFI_DIR)/sniffer/sniffer_radiotap.o\
		 $(RT_WIFI_DIR)/sniffer/sniffer_prism.o
endif

#CFG
ifeq ($(CONFIG_CFG80211_SUPPORT),y)
    EXTRA_CFLAGS += -DRT_CFG80211_SUPPORT -DEXT_BUILD_CHANNEL_LIST

    spec_objs += $(RT_WIFI_DIR)/os/linux/cfg80211/cfg80211_scan.o\
        $(RT_WIFI_DIR)/os/linux/cfg80211/cfg80211_rx.o\
        $(RT_WIFI_DIR)/os/linux/cfg80211/cfg80211_tx.o\
        $(RT_WIFI_DIR)/os/linux/cfg80211/cfg80211_inf.o\
        $(RT_WIFI_DIR)/os/linux/cfg80211/cfg80211_ap.o\
        $(RT_WIFI_DIR)/os/linux/cfg80211/cfg80211drv.o\
		$(RT_WIFI_DIR)/os/linux/cfg80211/cfg80211_util.o\
		$(RT_WIFI_DIR)/os/linux/cfg80211/cfg80211.o		
endif

#porting from 7615 start.
ifeq ($(CONFIG_INTERWORKING),y)
        EXTRA_CFLAGS += -DDOT11U_INTERWORKING
        spec_objs += $(RT_WIFI_DIR)/common/gas.o
endif

# MBO
ifeq ($(CONFIG_MBO_SUPPORT),y)
    EXTRA_CFLAGS += -DMBO_SUPPORT -DWAPP_SUPPORT

    spec_objs += $(RT_WIFI_DIR)/common/mbo.o
    spec_objs +=  $(RT_WIFI_DIR)/wapp/wapp.o
endif

ifeq ($(CONFIG_MAP_SUPPORT),y)
    EXTRA_CFLAGS += -DA4_CONN -DROUTING_TAB_SUPPORT -DWAPP_SUPPORT -DMAP_SUPPORT
    spec_objs += $(RT_WIFI_DIR)/common/map.o
    spec_objs += $(RT_WIFI_DIR)/common/a4_conn.o
    spec_objs += $(RT_WIFI_DIR)/common/routing_tab.o
    spec_objs +=  $(RT_WIFI_DIR)/wapp/wapp.o
endif

#ifeq ($(CONFIG_WAPP),y)
#    EXTRA_CFLAGS += -DWAPP_SUPPORT
#    spec_objs +=  $(RT_WIFI_DIR)/wapp/wapp.o
#endif
#porting from 7615 end.

ifeq ($(CONFIG_SNIFFER_MIB_CMD),y)
    EXTRA_CFLAGS += -DSNIFFER_MIB_CMD
    spec_objs += $(RT_WIFI_DIR)/ap/sniffer_mib.o
endif

ifeq ($(CONFIG_ALL_NET_EVENT),y)
    EXTRA_CFLAGS += -DALL_NET_EVENT
    spec_objs += $(RT_WIFI_DIR)/common/all_net_event.o
endif

ifeq ($(CONFIG_ACL_V2_SUPPORT),y)
    EXTRA_CFLAGS += -DACL_V2_SUPPORT
    spec_objs += $(RT_WIFI_DIR)/ap/ap_acl_v2.o
endif


#
# Common Feature related files
#
func_objs :=

ifeq ($(CONFIG_IGMP_SNOOP_SUPPORT),y)
    EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT

    func_objs += $(RT_WIFI_DIR)/common/igmp_snoop.o
endif

ifeq ($(CONFIG_BLOCK_NET_IF),y)
    EXTRA_CFLAGS += -DBLOCK_NET_IF

    func_objs += $(RT_WIFI_DIR)/common/netif_block.o
endif

ifeq ($(HAS_MAX_CONTINOUS_TX_CNT_SUPPORT),y)
EXTRA_CFLAGS += -DMAX_CONTINOUS_TX_CNT
endif

ifeq ($(CONFIG_SINGLE_SKU_V2),y)
    EXTRA_CFLAGS += -DSINGLE_SKU_V2
endif
ifeq ($(CONFIG_SMART_CARRIER_SENSE_SUPPORT),y)
    EXTRA_CFLAGS += -DSMART_CARRIER_SENSE_SUPPORT
endif



ifeq ($(CONFIG_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
    ifeq ($(CONFIG_WSC_INCLUDED),y)
        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
    endif

    func_objs += $(RT_WIFI_DIR)/common/rt_led.o
endif


########################################################
# AP feature related files
########################################################
ap_objs := $(RT_WIFI_DIR)/ap/ap.o\
            $(RT_WIFI_DIR)/ap/ap_assoc.o\
            $(RT_WIFI_DIR)/ap/ap_auth.o\
            $(RT_WIFI_DIR)/ap/ap_connect.o\
            $(RT_WIFI_DIR)/ap/ap_mlme.o\
            $(RT_WIFI_DIR)/ap/ap_sanity.o\
            $(RT_WIFI_DIR)/ap/ap_sync.o\
            $(RT_WIFI_DIR)/ap/ap_wpa.o\
            $(RT_WIFI_DIR)/ap/ap_data.o\
            $(RT_WIFI_DIR)/ap/ap_autoChSel.o\
            $(RT_WIFI_DIR)/ap/ap_qload.o\
            $(RT_WIFI_DIR)/ap/ap_cfg.o\
            $(RT_WIFI_DIR)/ap/ap_nps.o\
            $(RT_WIFI_DIR)/os/linux/ap_ioctl.o            
#EXTRA_CFLAGS += -DANDLINK_FEATURE_SUPPORT

ifeq ($(CONFIG_QOS_DLS_SUPPORT),y)
    EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
    ap_objs += $(RT_WIFI_DIR)/ap/ap_dls.o
endif

ifeq ($(CONFIG_MBSS_SUPPORT),y)
    EXTRA_CFLAGS += -DMBSS_SUPPORT

    ifeq ($(CONFIG_NEW_MBSSID_MODE),y)
        EXTRA_CFLAGS += -DNEW_MBSSID_MODE
        ifeq ($(CONFIG_ENHANCE_NEW_MBSSID_MODE),y)
            EXTRA_CFLAGS += -DENHANCE_NEW_MBSSID_MODE
        endif
    endif

    ap_objs += $(RT_WIFI_DIR)/ap/ap_mbss.o\
            $(RT_WIFI_DIR)/ap/ap_mbss_inf.o
endif


ifeq ($(CONFIG_WDS_SUPPORT),y)
    EXTRA_CFLAGS += -DWDS_SUPPORT

    ap_objs += $(RT_WIFI_DIR)/ap/ap_wds.o\
            $(RT_WIFI_DIR)/ap/ap_wds_inf.o\
            $(RT_WIFI_DIR)/common/client_wds.o
endif

#CONFIG_MIXMODE_SUPPORT = y
ifeq ($(CONFIG_MIXMODE_SUPPORT),y)
    EXTRA_CFLAGS += -DMIXMODE_SUPPORT

    ap_objs += $(RT_WIFI_DIR)/ap/ap_mix_mode.o
endif
ifeq ($(CONFIG_APCLI_SUPPORT),y)
    EXTRA_CFLAGS += -DAPCLI_SUPPORT -DMAT_SUPPORT -DTRAFFIC_BASED_TXOP -DAPCLI_AUTO_CONNECT_SUPPORT
    ifeq ($(CONFIG_RALINK_MT7620),y)
        EXTRA_CFLAGS += -DMAC_APCLI_SUPPORT
    endif

    ifeq ($(CONFIG_MULTI_APCLI_SUPPORT),y)
        EXTRA_CFLAGS += -DMULTI_APCLI_SUPPORT
    endif

    ifeq ($(CONFIG_AIRPLAY_SUPPORT),y)
        EXTRA_CFLAGS += -DAIRPLAY_SUPPORT
    endif

    ap_objs += $(RT_WIFI_DIR)/ap/ap_apcli.o\
            $(RT_WIFI_DIR)/ap/ap_apcli_inf.o\
            $(RT_WIFI_DIR)/ap/apcli_assoc.o\
            $(RT_WIFI_DIR)/ap/apcli_auth.o\
            $(RT_WIFI_DIR)/ap/apcli_ctrl.o\
            $(RT_WIFI_DIR)/ap/apcli_sync.o\
            $(RT_WIFI_DIR)/common/cmm_mat.o\
            $(RT_WIFI_DIR)/common/cmm_mat_iparp.o\
            $(RT_WIFI_DIR)/common/cmm_mat_pppoe.o\
            $(RT_WIFI_DIR)/common/cmm_mat_ipv6.o

    ifeq ($(CONFIG_MAC_REPEATER_SUPPORT),y)
        EXTRA_CFLAGS += -DMAC_REPEATER_SUPPORT

        ap_objs += $(RT_WIFI_DIR)/ap/ap_repeater.o
    endif
endif

ifeq ($(CONFIG_IDS_SUPPORT),y)
    EXTRA_CFLAGS += -DIDS_SUPPORT

    ap_objs += $(RT_WIFI_DIR)/ap/ap_ids.o
endif

ifeq ($(CONFIG_NINTENDO_AP),y)
    EXTRA_CFLAGS += -DNINTENDO_AP

    ap_objs += $(RT_WIFI_DIR)/ap/ap_nintendo.o
endif

ifeq ($(CONFIG_COC_SUPPORT),y)
    EXTRA_CFLAGS += -DGREENAP_SUPPORT -DCOC_SUPPORT

    ap_objs += $(RT_WIFI_DIR)/hw_ctrl/greenap.o
endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_ATE -DCONFIG_QA -DCONFIG_RT2880_ATE_CMD_NEW
endif

ifeq ($(CONFIG_RTMP_FLASH_SUPPORT),y)
    EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT
endif

ifeq ($(CONFIG_TXBF_SUPPORT),y)
    EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DTXBF_SUPPORT -DVHT_TXBF_SUPPORT
    ap_objs += $(RT_WIFI_DIR)/common/cmm_txbf.o\
                    $(RT_WIFI_DIR)/common/cmm_txbf_cal.o
endif

ifeq ($(CONFIG_BAND_STEERING_7603),y)
    EXTRA_CFLAGS += -DBAND_STEERING
    ap_objs +=  $(RT_WIFI_DIR)/ap/ap_band_steering.o
endif

ifeq ($(CONFIG_DFS_SUPPORT),y)
    EXTRA_CFLAGS += -DDFS_SUPPORT
    EXTRA_CFLAGS += -DDFS_ATP_SUPPORT
    ap_objs += $(RT_WIFI_DIR)/common/cmm_dfs.o
endif

ifeq ($(CONFIG_RT2860V2_AP_CARRIER),y)
    EXTRA_CFLAGS += -DCARRIER_DETECTION_SUPPORT

    ap_objs += $(RT_WIFI_DIR)/common/cmm_cs.o
endif

ifeq ($(CONFIG_MCAST_RATE_SPECIFIC),y)
    EXTRA_CFLAGS += -DMCAST_RATE_SPECIFIC
endif

ifeq ($(CONFIG_PREVENT_ARP_SPOOFING_SUPPORT),y)
    EXTRA_CFLAGS += -DPREVENT_ARP_SPOOFING
endif

#ifeq ($(CONFIG_RT2860V2_AP_MESH),y)
#EXTRA_CFLAGS += -DMESH_SUPPORT -DINTEL_CMPC
#mesh_objs += $(RT_WIFI_DIR)/common/mesh_bmpkt.o\
#                    $(RT_WIFI_DIR)/common/mesh_ctrl.o\
#                    $(RT_WIFI_DIR)/common/mesh_link_mng.o\
#                    $(RT_WIFI_DIR)/common/mesh_sanity.o\
#                    $(RT_WIFI_DIR)/common/mesh_tlv.o\
#                    $(RT_WIFI_DIR)/common/mesh.o\
#                    $(RT_WIFI_DIR)/common/mesh_inf.o\
#                    $(RT_WIFI_DIR)/common/mesh_forwarding.o\
#                    $(RT_WIFI_DIR)/common/mesh_path_mng.o 
#endif


########################################################
# Linux system related files
########################################################
os_objs := $(RT_WIFI_DIR)/os/linux/rt_proc.o\
            $(RT_WIFI_DIR)/os/linux/rt_linux.o\
            $(RT_WIFI_DIR)/os/linux/rt_profile.o\
            $(RT_WIFI_DIR)/os/linux/rt_main_dev.o


ifeq ($(CONFIG_MULTI_INF_SUPPORT),y)
    EXTRA_CFLAGS += -DMULTI_INF_SUPPORT

    os_objs += $(RT_WIFI_DIR)/os/linux/multi_main_dev.o
endif

ifeq ($(CONFIG_WIFI_WORK_QUEUE_BH),y)
    EXTRA_CFLAGS += -DWORKQUEUE_BH
endif

ifeq ($(CONFIG_KTHREAD),y)
    EXTRA_CFLAGS += -DKTHREAD_SUPPORT
endif

ifeq ($(CONFIG_BTCOEX_CONCURRENT),y)
    EXTRA_CFLAGS += -DBTCOEX_CONCURRENT
endif


########################################################
# chip related files
########################################################
#ifeq ($(CONFIG_RALINK_RT2880),y)
#EXTRA_CFLAGS += -DRT2880

#chip_objs += $(RT_WIFI_DIR)/chips/rt2880.o
#endif

#ifeq ($(CONFIG_RT2860V2_AP_TXBF),y)
#EXTRA_CFLAGS += -DTXBF_SUPPORT
#endif

#ifeq ($(CONFIG_RALINK_RT2883),y)
#EXTRA_CFLAGS += -DRT2883 -DDOT11N_SS3_SUPPORT -DA_BAND_SUPPORT -DRTMP_RF_RW_SUPPORT

#chip_objs += $(RT_WIFI_DIR)/phy/rt_rf.o\
#            $(RT_WIFI_DIR)/chips/rt2883.o
#endif

#ifeq ($(CONFIG_RALINK_RT3883),y)
#EXTRA_CFLAGS += -DRT3883 -DDOT11N_SS3_SUPPORT -DA_BAND_SUPPORT -DRTMP_RF_RW_SUPPORT
#EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT -DCONFIG_SWMCU_SUPPORT
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT -DVCORECAL_SUPPORT
#EXTRA_CFLAGS += -DRANGE_EXT_SUPPORT
#EXTRA_CFLAGS += -DSTREAM_MODE_SUPPORT -DFIFO_EXT_SUPPORT

#chip_objs += $(RT_WIFI_DIR)/phy/rt_rf.o\
#                    $(RT_WIFI_DIR)/chips/rt3883.o
#                    #$(RT_WIFI_DIR)/ate/chips/rt3883_ate.o

#ifeq ($(CONFIG_TXBF_SUPPORT),y)
#chip_objs += $(RT_WIFI_DIR)/common/cmm_txbf.o\
#                    $(RT_WIFI_DIR)/common/cmm_txbf_cal.o
#endif
#endif

#ifeq ($(CONFIG_RALINK_RT3052),y)
#EXTRA_CFLAGS += -DSPECIFIC_VCORECAL_SUPPORT
#ifeq ($(CONFIG_RALINK_RT3350),y)
#EXTRA_CFLAGS += -DRT3350 -DRT305x -DRTMP_RF_RW_SUPPORT
#else
#EXTRA_CFLAGS += -DRT3052 -DRT305x -DRTMP_RF_RW_SUPPORT
#endif

#chip_objs += $(RT_WIFI_DIR)/phy/rt_rf.o\
#                    $(RT_WIFI_DIR)/chips/rt305x.o
#endif

#ifeq ($(CONFIG_RALINK_RT3352),y)
#EXTRA_CFLAGS += -DRT3352 -DRT305x -DRTMP_RF_RW_SUPPORT
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT -DVCORECAL_SUPPORT

#chip_objs += $(RT_WIFI_DIR)/phy/rt_rf.o\
#                    $(RT_WIFI_DIR)/chips/rt305x.o\
#                    $(RT_WIFI_DIR)/chips/rt3352.o
#endif

#ifeq ($(CONFIG_RT3x52),y)
#EXTRA_CFLAGS += -DRT3052 -DRT3352 -DRT305x -DRTMP_RF_RW_SUPPORT -DSPECIFIC_BCN_BUF_SUPPORT -DVCORECAL_SUPPORT

#chip_objs += $(RT_WIFI_DIR)/phy/rt_rf.o\
#                    $(RT_WIFI_DIR)/chips/rt305x.o\
#                    $(RT_WIFI_DIR)/chips/rt3352.o
#endif

#ifeq ($(CONFIG_RALINK_RT5350),y)
#EXTRA_CFLAGS += -DRT5350 -DRT305x -DRTMP_RF_RW_SUPPORT
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT -DVCORECAL_SUPPORT

#chip_objs += $(RT_WIFI_DIR)/phy/rt_rf.o\
#                    $(RT_WIFI_DIR)/chips/rt305x.o\
#                    $(RT_WIFI_DIR)/chips/rt5350.o
#endif
#ifeq ($(CONFIG_RALINK_RT6352),y)
#EXTRA_CFLAGS += -DRT6352 -DRTMP_RF_RW_SUPPORT -DRTMP_FLASH_SUPPORT -DCONFIG_SWMCU_SUPPORT
#EXTRA_CFLAGS += -DRTMP_RBUS_SUPPORT -DRTMP_MAC_PCI
#EXTRA_CFLAGS += -DRTMP_MAC -DRTMP_BBP -DRLT_RF
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT -DVCORECAL_SUPPORT
#EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC
#EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
#EXTRA_CFLAGS += -DLOFT_IQ_CAL_SUPPORT
#EXTRA_CFLAGS += -DRTMP_TEMPERATURE_CALIBRATION
#EXTRA_CFLAGS += -DRELEASE_EXCLUDE
#EXTRA_CFLAGS += -DENHANCE_NEW_MBSSID_MODE
#EXTRA_CFLAGS += -DSPECIFIC_TX_POWER_SUPPORT
#EXTRA_CFLAGS += -DFIFO_EXT_SUPPORT

#chip_objs += $(RT_WIFI_DIR)/phy/rt_rf.o\
#		$(RT_WIFI_DIR)/phy/rlt_rf.o\
#            $(RT_WIFI_DIR)/phy/rtmp_phy.o\
#            $(RT_WIFI_DIR)/chips/rt6352.o\
#            $(RT_WIFI_DIR)/common/cmm_rf_cal.o\
#            $(RT_WIFI_DIR)/common/cmm_loft_cal.o
#            #$(RT_WIFI_DIR)/ate/chips/rt6352_ate.o
#endif

#ifeq ($(CONFIG_RALINK_MT7610E),y)
#EXTRA_CFLAGS += -DRT65xx -DMT76x0 -DMT7610
#EXTRA_CFLAGS += -DRLT_MAC -DRLT_BBP -DRLT_RF -DRTMP_RF_RW_SUPPORT
#EXTRA_CFLAGS += -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT -DRX_DMA_SCATTER -DCONFIG_ANDES_SUPPORT -DRTMP_EFUSE_SUPPORT
#EXTRA_CFLAGS += -DA_BAND_SUPPORT -DSPECIFIC_BCN_BUF_SUPPORT -DNEW_MBSSID_MODE -DENHANCE_NEW_MBSSID_MODE
#EXTRA_CFLAGS += -DVCORECAL_SUPPORT -DMT76x0_TSSI_CAL_COMPENSATION
#EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC
#EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
#EXTRA_CFLAGS += -DRELEASE_EXCLUDE

#chip_objs += $(RT_WIFI_DIR)/phy/rt_rf.o\
#            $(RT_WIFI_DIR)/phy/rlt_rf.o\
#            $(RT_WIFI_DIR)/phy/rlt_phy.o\
#            $(RT_WIFI_DIR)/phy/rtmp_phy.o\
#            $(RT_WIFI_DIR)/chips/mt76x0.o\
#            $(RT_WIFI_DIR)/chips/rt65xx.o\
#            $(RT_WIFI_DIR)/common/ee_efuse.o\
#            $(RT_WIFI_DIR)/common/ee_prom.o\
#            $(RT_WIFI_DIR)/mcu/mcu_and.o\
#            $(RT_WIFI_DIR)/common/cmm_mac_pci.o\
#            $(RT_WIFI_DIR)/common/cmm_data_pci.o\
#            $(RT_WIFI_DIR)/os/linux/pci_main_dev.o\
#            $(RT_WIFI_DIR)/os/linux/rt_rbus_pci_drv.o
#            #$(RT_WIFI_DIR)/ate/chips/rt6590_ate.o
#endif

#ifeq ($(CONFIG_RALINK_MT7612E),y)
#EXTRA_CFLAGS += -DRT65xx -DMT76x2 -DMT7612
#EXTRA_CFLAGS += -DRLT_MAC -DRLT_BBP -DMT_RF -DRTMP_RF_RW_SUPPORT
#EXTRA_CFLAGS += -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT -DRX_DMA_SCATTER -DCONFIG_ANDES_SUPPORT -DRTMP_EFUSE_SUPPORT
#EXTRA_CFLAGS += -DA_BAND_SUPPORT -DSPECIFIC_BCN_BUF_SUPPORT -DNEW_MBSSID_MODE -DENHANCE_NEW_MBSSID_MODE
#EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC -DRTMP_EFUSE_SUPPORT -DCONFIG_FPGA_MODE
#EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY -DNEW_RATE_ADAPT_SUPPORT -DDYNAMIC_VGA_SUPPORT
#EXTRA_CFLAGS += -DRELEASE_EXCLUDE -DFIFO_EXT_SUPPORT
#EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DPEER_DELBA_TX_ADAPT
#EXTRA_CFLAGS += -DSPECIFIC_TX_POWER_SUPPORT

#chip_objs += $(RT_WIFI_DIR)/phy/rt_rf.o\
#            $(RT_WIFI_DIR)/phy/mt_rf.o\
#            $(RT_WIFI_DIR)/phy/rlt_phy.o\
#            $(RT_WIFI_DIR)/phy/rtmp_phy.o\
#            $(RT_WIFI_DIR)/chips/mt76x2.o\
#            $(RT_WIFI_DIR)/chips/rt65xx.o\
#            $(RT_WIFI_DIR)/common/ee_efuse.o\
#            $(RT_WIFI_DIR)/common/ee_prom.o\
#            $(RT_WIFI_DIR)/mcu/mcu_and.o\
#            $(RT_WIFI_DIR)/common/cmm_mac_pci.o\
#            $(RT_WIFI_DIR)/common/cmm_data_pci.o\
#            $(RT_WIFI_DIR)/os/linux/pci_main_dev.o\
#            $(RT_WIFI_DIR)/os/linux/rt_rbus_pci_drv.o
#endif

ifeq ($(CONFIG_RALINK_MT7603E),y)
ccflags-y += -DMT7603 -DMT_BBP -DMT_RF -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT -DMCS_LUT_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT -DCAL_FREE_IC_SUPPORT
EXTRA_CFLAGS += -DCONFIG_RT_FIRST_CARD=y
EXTRA_CFLAGS += -DCONFIG_RT_FIRST_IF_RF_OFFSET=0
EXTRA_CFLAGS += -DCONFIG_RT_FIRST_CARD_EEPROM="\"flash\""
EXTRA_CFLAGS += -DCONFIG_FIRST_IF_EPAELNA
ccflags-y += -DRELEASE_EXCLUDE
ccflags-y += -DMT_RF -DRTMP_RF_RW_SUPPORT
ccflags-y += -DRTMP_PCI_SUPPORT -DRX_DMA_SCATTER -DCONFIG_ANDES_SUPPORT -DRTMP_EFUSE_SUPPORT
ccflags-y += -DA_BAND_SUPPORT -DNEW_MBSSID_MODE -DENHANCE_NEW_MBSSID_MODE
ccflags-y += -DRESOURCE_PRE_ALLOC -DRTMP_EFUSE_SUPPORT -DCONFIG_FPGA_MODE
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY -DNEW_RATE_ADAPT_SUPPORT
EXTRA_CFLAGS += -DRELEASE_EXCLUDE -DFIFO_EXT_SUPPORT
EXTRA_CFLAGS += -DCONFIG_BA_REORDER_MONITOR
EXTRA_CFLAGS += -DUSE_BMC
EXTRA_CFLAGS += -DMT_PS -DDMA_RESET_SUPPORT -DTXRXCR_DEBUG_SUPPORT
EXTRA_CFLAGS += -DMT7603_E1
EXTRA_CFLAGS += -DMT7603_E2
EXTRA_CFLAGS += -DEDCCA_RB
#EXTRA_CFLAGS += -Os -DECONET_ALPHA_RELEASE
#EXTRA_CFLAGS += -DMEMORY_OPTIMIZATION
#Force Load F/W one time
EXTRA_CFLAGS += -DLOAD_FW_ONE_TIME
#support dedicate EAPOL QUEUE
EXTRA_CFLAGS += -DEAPOL_QUEUE_SUPPORT
ifeq ($(CONFIG_DMA_CACHE_OPTIMIZE),y)
    EXTRA_CFLAGS += -DDMA_CACHE_OPTIMIZE
endif
#support reserve high priority pkt queue
ccflags-y += -DDATA_QUEUE_RESERVE
ccflags-y += -DMULTI_CLIENT_SUPPORT
ccflags-y += -DSMART_CARRIER_SENSE_SUPPORT
#EXTRA_CFLAGS += -DDYNAMIC_WMM
#ccflags-y += -DINTERFERENCE_RA_SUPPORT

#Limit the global s/w queue instead of per sta s/w queue
ccflags-y += -DLIMIT_GLOBAL_SW_QUEUE

chip_objs += $(RT_WIFI_DIR)/chips/mt7603.o\
		$(RT_WIFI_DIR)/hw_ctrl/cmm_asic_mt.o\
		$(RT_WIFI_DIR)/hw_ctrl/cmm_chip_mt.o\
		$(RT_WIFI_DIR)/hw_ctrl/mt_gpio.o\
		$(RT_WIFI_DIR)/mac/mt_mac.o\
		$(RT_WIFI_DIR)/mcu/mcu.o\
		$(RT_WIFI_DIR)/mcu/andes_core.o\
		$(RT_WIFI_DIR)/mcu/andes_mt.o\
		$(RT_WIFI_DIR)/phy/mt_rf.o\
		$(RT_WIFI_DIR)/phy/rf.o\
		$(RT_WIFI_DIR)/phy/mt_phy.o\
		$(RT_WIFI_DIR)/common/ee_efuse.o\
		$(RT_WIFI_DIR)/common/ee_prom.o
endif

ifeq ($(CONFIG_RALINK_MT7603U),y)
EXTRA_CFLAGS += -DMT7603 -DMT_BBP -DMT_RF -DRTMP_RF_RW_SUPPORT
EXTRA_CFLAGS += -DMT_MAC -DMCS_LUT_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT
EXTRA_CFLAGS += -DNEW_MBSSID_MODE -DENHANCE_NEW_MBSSID_MODE
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC -DCONFIG_FPGA_MODE
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DRELEASE_EXCLUDE -DFIFO_EXT_SUPPORT
EXTRA_CFLAGS += -DRTMP_WLAN_HOOK_SUPPORT
EXTRA_CFLAGS += -DCONFIG_CALIBRATION_COLLECTION
EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
EXTRA_CFLAGS += -DRTMP_TIMER_TASK_SUPPORT
EXTRA_CFLAGS += -DMT_PS_CHECK
EXTRA_CFLAGS += -DUSE_BMC -DBCN_OFFLOAD_SUPPORT

EXTRA_CFLAGS += -DMT7603_E1
EXTRA_CFLAGS += -DMT7603_E2
#EXTRA_CFLAGS += -Os -DECONET_ALPHA_RELEASE
#EXTRA_CFLAGS += -DMEMORY_OPTIMIZATION
#Force Load F/W one time
EXTRA_CFLAGS += -DLOAD_FW_ONE_TIME

#Limit the global s/w queue instead of per sta s/w queue
#EXTRA_CFLAGS += -DLIMIT_GLOBAL_SW_QUEUE

chip_objs += $(RT_WIFI_DIR)/chips/mt7603.o\
		$(RT_WIFI_DIR)/hw_ctrl/cmm_asic_mt.o\
		$(RT_WIFI_DIR)/hw_ctrl/cmm_chip_mt.o\
		$(RT_WIFI_DIR)/hw_ctrl/mt_gpio.o\
		$(RT_WIFI_DIR)/mac/mt_mac.o\
		$(RT_WIFI_DIR)/mcu/mcu.o\
		$(RT_WIFI_DIR)/mcu/andes_core.o\
		$(RT_WIFI_DIR)/mcu/andes_mt.o\
		$(RT_WIFI_DIR)/phy/mt_rf.o\
		$(RT_WIFI_DIR)/phy/rf.o\
		$(RT_WIFI_DIR)/phy/mt_phy.o\
		$(RT_WIFI_DIR)/common/ee_efuse.o\
		$(RT_WIFI_DIR)/common/ee_prom.o
endif

#ifeq ($(CONFIG_RALINK_MT7620),y)
#EXTRA_CFLAGS += -DRT6352 \
#				-DRTMP_RF_RW_SUPPORT \
#				-DRF_BANK -DRTMP_FLASH_SUPPORT \
#				-DCONFIG_SWMCU_SUPPORT \
#				-DRTMP_MAC_PCI \
#				-DRTMP_BBP \
#				-DRTMP_RBUS_SUPPORT
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT -DVCORECAL_SUPPORT
#EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC
#EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
#EXTRA_CFLAGS += -DLOFT_IQ_CAL_SUPPORT
#EXTRA_CFLAGS += -DRTMP_TEMPERATURE_CALIBRATION
#EXTRA_CFLAGS += -DDYNAMIC_VGA_SUPPORT
#EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DPEER_DELBA_TX_ADAPT
#chip_objs += $(RT_WIFI_DIR)/os/linux/rbus_main_dev.o\
#			$(RT_WIFI_DIR)/phy/rf.o\
#			$(RT_WIFI_DIR)/phy/rt_rf.o\
#	        $(RT_WIFI_DIR)/common/cmm_rf_cal.o\
#			$(RT_WIFI_DIR)/phy/rtmp_phy.o\
#	        $(RT_WIFI_DIR)/hw_ctrl/cmm_asic.o\
#	        $(RT_WIFI_DIR)/hif/hif_pci.o\
#	        $(RT_WIFI_DIR)/mcu/mcu.o\
#	        $(RT_WIFI_DIR)/mac/rtmp_mac.o\
#	        $(RT_WIFI_DIR)/common/ee_prom.o\
#	        $(RT_WIFI_DIR)/common/ral_ps.o\
#	        $(RT_WIFI_DIR)/common/rtmp_swmcu.o\
#	        $(RT_WIFI_DIR)/common/cmm_loft_cal.o\
#			$(RT_WIFI_DIR)/chips/rt6352.o
## to check ##
#EXTRA_CFLAGS += -DRLT_RF
#chip_objs += $(RT_WIFI_DIR)/phy/rlt_rf.o
#endif

#
# Root 
#
ifeq ($(CONFIG_RALINK_MT7603E),y)
obj-m += $(DRV_NAME).o
endif

$(DRV_NAME)-objs += $(ap_objs) $(cmm_objs) $(asic_objs) $(chip_objs) $(rate_objs) $(inf_objs)\
                    $(spec_objs) $(func_objs) $(os_objs)

$(DRV_NAME)-objs += $(RT_WIFI_DIR)/common/eeprom.o\
                    $(RT_WIFI_DIR)/common/ee_flash.o

$(DRV_NAME)-objs += $(RT_WIFI_DIR)/common/cmm_mac_pci.o
$(DRV_NAME)-objs += $(RT_WIFI_DIR)/common/cmm_data_pci.o

$(DRV_NAME)-objs += $(RT_WIFI_DIR)/os/linux/rt_pci_rbus.o\
                    $(RT_WIFI_DIR)/os/linux/rt_rbus_pci_drv.o\
                    $(RT_WIFI_DIR)/os/linux/rt_rbus_pci_util.o\
                    #$(RT_WIFI_DIR)/os/linux/rbus_main_dev.o
ifeq ($(CONFIG_ATE_SUPPORT),y)
$(DRV_NAME)-objs += $(RT_WIFI_DIR)/ate/ate_agent.o\
                    $(RT_WIFI_DIR)/ate/qa_agent.o\
                    $(RT_WIFI_DIR)/ate/mt_mac/mt_ate.o
endif

###################
#  CFLAGS
##################
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT  -DLINUX \
               -Wall -Wstrict-prototypes -Wno-trigraphs -Wframe-larger-than=4096 
#-DDBG_DIAGNOSE -DDBG_RX_MCS -DDBG_TX_MCS
EXTRA_CFLAGS += -DCONFIG_RALINK_FLASH_API
EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT -DSCAN_SUPPORT -DAP_SCAN_SUPPORT
EXTRA_CFLAGS += -DDOT11_N_SUPPORT -DSTATS_COUNT_SUPPORT -DIAPP_SUPPORT -DDOT1X_SUPPORT
#EXTRA_CFLAGS += -DRALINK_ATE -DRALINK_QA -DCONFIG_RT2880_ATE_CMD_NEW
ifeq ($(CONFIG_RA_HW_NAT_WIFI_NEW_ARCH),n)
EXTRA_CFLAGS += -DCONFIG_RA_NAT_NONE
endif

#provide busy time statistics for every TBTT */
#EXTRA_CFLAGS += -DQLOAD_FUNC_BUSY_TIME_STATS 

# provide busy time alarm mechanism 
# use the function to avoid to locate in some noise environments 
#EXTRA_CFLAGS += -DQLOAD_FUNC_BUSY_TIME_ALARM

ifeq ($(CONFIG_RT2860V2_AUTO_CH_SELECT_ENCANCE),y)
EXTRA_CFLAGS += -DAUTO_CH_SELECT_ENHANCE
endif

ifeq ($(CONFIG_RT2860V2_SNMP),y)
EXTRA_CFLAGS += -DSNMP_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_32B_DESC),y)
EXTRA_CFLAGS += -DDESC_32B_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_HW_ANTENNA_DIVERSITY),y)
EXTRA_CFLAGS += -DHW_ANTENNA_DIVERSITY_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_EXT_CHANNEL_LIST),y)
EXTRA_CFLAGS += -DEXT_BUILD_CHANNEL_LIST
endif

ifeq ($(CONFIG_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS += -DMEMORY_OPTIMIZATION
else
EXTRA_CFLAGS += -DDBG
endif

ifeq ($(CONFIG_RTMP_INTERNAL_TX_ALC),y)
EXTRA_CFLAGS += -DRTMP_INTERNAL_TX_ALC
endif

ifeq ($(CONFIG_ROAMING_ENHANCE_SUPPORT),y)
ccflags-y += -DROAMING_ENHANCE_SUPPORT
endif

ifeq ($(CONFIG_MT7603E_WPA3_SUPPORT),y)
    EXTRA_CFLAGS += -DDOT11_SAE_SUPPORT
    SRC_EMBEDDED_DIR = $(RT_WIFI_DIR)
    spec_objs += $(SRC_EMBEDDED_DIR)/common/sae.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/crypt_bignum.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/crypt_biginteger.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/bn_lib.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/ecc.o
    EXTRA_CFLAGS += -DDOT11_SAE_OPENSSL_BN
    ifeq ($(CONFIG_MT7603E_APCLI_SUPPORT),y)
        EXTRA_CFLAGS += -DAPCLI_SAE_SUPPORT
    endif
endif

ifeq ($(CONFIG_MT7603E_OWE_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_OWE_SUPPORT -DDOT11_SAE_OPENSSL_BN
    SRC_EMBEDDED_DIR = $(RT_WIFI_DIR)
    spec_objs += $(SRC_EMBEDDED_DIR)/common/owe.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/crypt_bignum.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/crypt_biginteger.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/bn_lib.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/ecc.o
    ifeq ($(CONFIG_MT7603E_APCLI_SUPPORT),y)
        EXTRA_CFLAGS += -DAPCLI_OWE_SUPPORT
    endif
endif
EXTRA_CFLAGS += -DIP_ASSEMBLY
#ifeq ($(CONFIG_RT2860V2_AP_RTMP_TEMPERATURE_CALIBRATION),y)
#ifeq ($(CONFIG_RALINK_MT7620),y)
#EXTRA_CFLAGS   += -DRTMP_TEMPERATURE_CALIBRATION
#endif
#endif

EXTRA_CFLAGS += -DACS_CTCC_SUPPORT

#support 7603 SW ATF
EXTRA_CFLAGS += -DSW_ATF_SUPPORT

ifeq ($(CONFIG_SECURITY_IMPROVEMENT_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_SECURITY_IMPROVEMENT_SUPPORT
    EXTRA_CFLAGS += -DAPCLI_SECURITY_IMPROVEMENT_SUPPORT
endif



ifeq ($(CONFIG_NEW_BW2040_COEXIST_SUPPORT),y)
    EXTRA_CFLAGS += -DNEW_BW2040_COEXIST_SUPPORT
endif

MODULE_FLAGS=$(EXTRA_CFLAGS)
export MODULE_FLAGS
