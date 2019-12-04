ifeq ($(CONFIG_DEFAULTS_KERNEL_4_4),y)
WIFI_DRV_PATH = drivers/net/wireless/mediatek
else
WIFI_DRV_PATH = drivers/net/wireless
endif

EXTRA_CFLAGS = -I$(WIFI_DRV_PATH)/mt_wifi/include \
				-I$(WIFI_DRV_PATH)/mt_wifi/embedded/include \
				-I$(WIFI_DRV_PATH)/mt_wifi/ate/include

ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
EXTRA_CFLAGS = -I$(src)/../src/include \
				-I$(src)/../src/embedded/include \
				-I$(src)/../src/ate/include
DRV_NAME = mt7628
SRC_DIR = ../src/embedded
obj-m += $(DRV_NAME).o
else
EXTRA_CFLAGS = -I$(WIFI_DRV_PATH)/mt_wifi/include \
				-I$(WIFI_DRV_PATH)/mt_wifi/embedded/include \
				-I$(WIFI_DRV_PATH)/mt_wifi/ate/include
DRV_NAME = mt_wifi
SRC_DIR = ../mt_wifi
SRC_EMBEDDED_DIR = ../mt_wifi/embedded
obj-$(CONFIG_MT_STA_SUPPORT) += $(DRV_NAME).o
endif

ifeq ($(CONFIG_MTK_EMI_7622),y)
EXTRA_CFLAGS += -DCONFIG_ARCH_MT7622
endif

EXTRA_CFLAGS += -DWIFI_SYS_FW_V1
EXTRA_CFLAGS += -DWIFI_SYS_FW_V2

########################################################
# Common files
#		$(SRC_EMBEDDED_DIR)/common/cmm_mumimo.o\
########################################################
cmm_objs := $(SRC_EMBEDDED_DIR)/common/action.o\
		$(SRC_EMBEDDED_DIR)/common/ba_action.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hdev_ctrl.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hdev/radio_ctrl.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hdev/omac_ctrl.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hdev/wmm_ctrl.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hdev/wtbl_ctrl.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hdev/hdev_basic.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hw_ctrl.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hw_ctrl_ops_v1.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hw_ctrl_ops_v2.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hw_ctrl_cmd.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hw_ctrl_basic.o\
		$(SRC_DIR)/hw_ctrl/hw_init.o\
		$(SRC_EMBEDDED_DIR)/common/bcn.o\
		$(SRC_EMBEDDED_DIR)/security/cmm_aes.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_cfg.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_cmd.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_data.o\
		$(SRC_EMBEDDED_DIR)/common/qm.o\
		$(SRC_EMBEDDED_DIR)/common/fq_qm.o\
		$(SRC_EMBEDDED_DIR)/common/fp_fair_qm.o\
		$(SRC_EMBEDDED_DIR)/common/fp_qm.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_info.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_profile.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_radar.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_sync.o\
		$(SRC_EMBEDDED_DIR)/common/wifi_sys_info.o\
		$(SRC_EMBEDDED_DIR)/common/wifi_sys_notify.o\
		$(SRC_EMBEDDED_DIR)/wlan_config/config_basic.o\
		$(SRC_EMBEDDED_DIR)/wlan_config/config_phy.o\
		$(SRC_EMBEDDED_DIR)/wlan_config/config_ht.o\
		$(SRC_EMBEDDED_DIR)/wlan_config/config_vht.o\
		$(SRC_EMBEDDED_DIR)/mgmt/be_basic.o\
		$(SRC_EMBEDDED_DIR)/mgmt/be_phy.o\
		$(SRC_EMBEDDED_DIR)/mgmt/be_ht.o\
		$(SRC_EMBEDDED_DIR)/mgmt/be_vht.o\
                $(SRC_EMBEDDED_DIR)/mgmt/bss_ops.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_sanity.o\
		$(SRC_EMBEDDED_DIR)/common/vendor.o\
		$(SRC_EMBEDDED_DIR)/security/sae.o\
		$(SRC_EMBEDDED_DIR)/security/cmm_sec.o\
		$(SRC_EMBEDDED_DIR)/security/cmm_tkip.o\
		$(SRC_EMBEDDED_DIR)/security/cmm_wep.o\
		$(SRC_EMBEDDED_DIR)/security/cmm_wpa.o\
		$(SRC_EMBEDDED_DIR)/security/crypt_aes.o\
		$(SRC_EMBEDDED_DIR)/security/crypt_arc4.o\
		$(SRC_EMBEDDED_DIR)/security/crypt_hmac.o\
		$(SRC_EMBEDDED_DIR)/security/crypt_md5.o\
		$(SRC_EMBEDDED_DIR)/security/crypt_sha2.o\
		$(SRC_EMBEDDED_DIR)/common/eeprom.o\
		$(SRC_EMBEDDED_DIR)/common/mlme.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic.o\
		$(SRC_DIR)/hw_ctrl/cmm_chip.o\
		$(SRC_EMBEDDED_DIR)/mgmt/mgmt_ht.o\
		$(SRC_EMBEDDED_DIR)/mgmt/mgmt_dev.o\
		$(SRC_EMBEDDED_DIR)/common/rtmp_init.o\
		$(SRC_EMBEDDED_DIR)/common/rtmp_init_inf.o\
		$(SRC_EMBEDDED_DIR)/common/spectrum.o\
		$(SRC_EMBEDDED_DIR)/common/rtmp_timer.o\
		$(SRC_EMBEDDED_DIR)/common/rt_channel.o\
		$(SRC_EMBEDDED_DIR)/common/op_class.o\
		$(SRC_EMBEDDED_DIR)/common/rt_os_util.o\
		$(SRC_EMBEDDED_DIR)/common/scan.o\
		$(SRC_EMBEDDED_DIR)/common/sys_log.o\
		$(SRC_EMBEDDED_DIR)/common/txpower.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_info_element.o\
		$(SRC_DIR)/chips/rtmp_chip.o\
		$(SRC_EMBEDDED_DIR)/mgmt/mgmt_hw.o\
		$(SRC_EMBEDDED_DIR)/mgmt/mgmt_entrytb.o\
		$(SRC_EMBEDDED_DIR)/common/wdev.o\
		$(SRC_DIR)/os/linux/rt_profile.o\
		$(SRC_DIR)/os/linux/tm.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_rvr_dbg.o\
		$(SRC_DIR)/protocol/protection.o

########################################################
# SAE files
########################################################
ifeq ($(CONFIG_WPA3_SUPPORT),y)
   EXTRA_CFLAGS += -DDOT11_SAE_SUPPORT -DDOT11_SUITEB_SUPPORT
#    EXTRA_CFLAGS += -DDOT11_SAE_OPENSSL_BN

    cmm_objs += $(SRC_EMBEDDED_DIR)/security/crypt_bignum.o\
                $(SRC_EMBEDDED_DIR)/security/crypt_biginteger.o\
                $(SRC_EMBEDDED_DIR)/security/bn_lib.o\
                $(SRC_EMBEDDED_DIR)/security/ecc.o\
                $(SRC_EMBEDDED_DIR)/security/sae.o
endif

########################################################
# OWE related files
########################################################
ifeq ($(CONFIG_OWE_SUPPORT),y)
   EXTRA_CFLAGS += -DCONFIG_OWE_SUPPORT

    cmm_objs += $(SRC_EMBEDDED_DIR)/security/crypt_bignum.o\
                $(SRC_EMBEDDED_DIR)/security/crypt_biginteger.o\
                $(SRC_EMBEDDED_DIR)/security/bn_lib.o\
                $(SRC_EMBEDDED_DIR)/security/ecc.o\
                $(SRC_EMBEDDED_DIR)/security/owe.o

endif


########################################################
# Coex related files
########################################################
ifeq ($(CONFIG_COEX_SUPPORT),y)
   EXTRA_CFLAGS += -DCOEX_SUPPORT

    cmm_objs += $(SRC_EMBEDDED_DIR)/mcu/bt_coex.o
endif

########################################################
# Rate adaptation related files
########################################################
rate_objs := $(SRC_DIR)/rate_ctrl/ra_ctrl_mt.o\
		$(SRC_DIR)/rate_ctrl/ra_ctrl_mt_drv.o\
		$(SRC_DIR)/rate_ctrl/ra_wrapper_embedded.o\
		$(SRC_DIR)/rate_ctrl/ra_cfg.o

ifeq ($(CONFIG_NEW_RATE_ADAPT_SUPPORT),y)
    EXTRA_CFLAGS += -DNEW_RATE_ADAPT_SUPPORT
    rate_objs += $(SRC_DIR)/rate_ctrl/alg_grp_mt.o
    rate_objs += $(SRC_DIR)/rate_ctrl/ra_table.o
endif

ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
	EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
endif

########################################################
# ASIC related files
########################################################
asic_objs := $(SRC_DIR)/phy/phy.o

########################################################
# Spec feature related files
########################################################
spec_objs += $(SRC_EMBEDDED_DIR)/common/ps.o

ifeq ($(CONFIG_UAPSD),y)
    EXTRA_CFLAGS += -DUAPSD_SUPPORT -DUAPSD_DEBUG
    spec_objs += $(SRC_EMBEDDED_DIR)/common/uapsd.o
endif

ifeq ($(CONFIG_MT_MAC),y)
	spec_objs += $(SRC_EMBEDDED_DIR)/common/mt_ps.o
	spec_objs += $(SRC_EMBEDDED_DIR)/common/mt_io.o
	spec_objs += $(SRC_EMBEDDED_DIR)/common/txs.o
endif

# WSC
ifeq ($(CONFIG_WSC_INCLUDED),y)
    EXTRA_CFLAGS += -DWSC_INCLUDED
#-DWSC_SINGLE_TRIGGER

    ifneq ($(CONFIG_MT_AP_SUPPORT),)
        EXTRA_CFLAGS += -DWSC_AP_SUPPORT
    endif

    ifneq ($(CONFIG_MT_STA_SUPPORT),)
        EXTRA_CFLAGS += -DWSC_STA_SUPPORT
    endif

    spec_objs += $(SRC_EMBEDDED_DIR)/common/wsc.o\
                    $(SRC_EMBEDDED_DIR)/common/wsc_tlv.o\
                    $(SRC_EMBEDDED_DIR)/security/crypt_dh.o\
                    $(SRC_EMBEDDED_DIR)/security/crypt_biginteger.o\
#                    $(SRC_EMBEDDED_DIR)/common/wsc_ufd.o

    ifeq ($(CONFIG_WSC_V2_SUPPORT),y)
        EXTRA_CFLAGS += -DWSC_V2_SUPPORT
        spec_objs += $(SRC_EMBEDDED_DIR)/common/wsc_v2.o
    endif
endif

# VHT
ifeq ($(CONFIG_DOT11_VHT_AC),y)
    EXTRA_CFLAGS += -DDOT11_VHT_AC
    spec_objs += $(SRC_EMBEDDED_DIR)/mgmt/mgmt_vht.o\
            $(SRC_EMBEDDED_DIR)/common/vht.o
	# WFA VHT R2 PF
	ifeq ($(CONFIG_WFA_VHT_R2_PF),y)
		EXTRA_CFLAGS += -DWFA_VHT_R2_PF
	endif
endif

# WAPI
ifeq ($(CONFIG_WAPI_SUPPORT),y)
    EXTRA_CFLAGS += -DWAPI_SUPPORT
    ifeq ($(CONFIG_RALINK_RT3052),y)
        EXTRA_CFLAGS += -DSOFT_ENCRYPT
    endif

  spec_objs += $(SRC_EMBEDDED_DIR)/security/wapi.o\
                $(SRC_EMBEDDED_DIR)/security/wapi_sms4.o\
		$(SRC_EMBEDDED_DIR)/security/wapi_crypt.o

endif

# ACM
ifeq ($(CONFIG_WMM_ACM_SUPPORT),y)
    EXTRA_CFLAGS += -DWMM_ACM_SUPPORT

    spec_objs += $(SRC_EMBEDDED_DIR)/common/acm_edca.o\
            $(SRC_EMBEDDED_DIR)/common/acm_comm.o\
            $(SRC_EMBEDDED_DIR)/common/acm_iocl.o
endif

#PMF
ifeq ($(CONFIG_DOT11W_PMF_SUPPORT),y)
    EXTRA_CFLAGS += -DDOT11W_PMF_SUPPORT

    spec_objs += $(SRC_EMBEDDED_DIR)/security/pmf.o
endif

# LLTD
ifeq ($(CONFIG_LLTD_SUPPORT),y)
    EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

ifeq ($(CONFIG_PASSPOINT_R2),y)
	EXTRA_CFLAGS += -DCONFIG_DOT11U_INTERWORKING -DCONFIG_DOT11V_WNM -DCONFIG_HOTSPOT -DCONFIG_HOTSPOT_R2

	spec_objs += $(SRC_EMBEDDED_DIR)/common/wnm.o\
				$(SRC_EMBEDDED_DIR)/common/gas.o\
				$(SRC_EMBEDDED_DIR)/common/hotspot.o
endif

# FT
#ifeq ($(CONFIG_RT2860V2_80211R_FT),y)
#dot11_ft_objs += $(SRC_EMBEDDED_DIR)/common/ft.o\
#                    $(SRC_EMBEDDED_DIR)/common/ft_tlv.o\
#                    $(SRC_EMBEDDED_DIR)/common/ft_ioctl.o\
#                    $(SRC_EMBEDDED_DIR)/common/ft_rc.o\
#                    $(SRC_EMBEDDED_DIR)/ap/ap_ftkd.o
#endif

# RR
#ifeq ($(CONFIG_RT2860V2_80211K_RR),y)
#dot11k_rr_objs += $(SRC_EMBEDDED_DIR)/common/rrm_tlv.o\
#                    $(SRC_EMBEDDED_DIR)/common/rrm_sanity.o\
#                    $(SRC_EMBEDDED_DIR)/common/rrm.o
#endif
#SNIFFER
ifeq ($(CONFIG_SNIFFER_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_SNIFFER_SUPPORT

    spec_objs += $(MT_WIFI_DIR)/common/sniffer_radiotap.o\
                 $(MT_WIFI_DIR)/common/sniffer_prism.o
endif

#CFG
ifeq ($(CONFIG_CFG80211_SUPPORT),y)
    EXTRA_CFLAGS += -DRT_CFG80211_SUPPORT -DEXT_BUILD_CHANNEL_LIST -DWPA_SUPPLICANT_SUPPORT

    spec_objs += $(SRC_DIR)/os/linux/cfg80211/cfg80211_scan.o\
        $(SRC_DIR)/os/linux/cfg80211/cfg80211_rx.o\
        $(SRC_DIR)/os/linux/cfg80211/cfg80211_tx.o\
        $(SRC_DIR)/os/linux/cfg80211/cfg80211_inf.o\
        $(SRC_DIR)/os/linux/cfg80211/cfg80211_ap.o\
        $(SRC_DIR)/os/linux/cfg80211/cfg80211drv.o\
                $(SRC_DIR)/os/linux/cfg80211/cfg80211_util.o\
                $(SRC_DIR)/os/linux/cfg80211/cfg80211.o
endif

#
# Common Feature related files
#
func_objs :=

ifeq ($(CONFIG_IGMP_SNOOP_SUPPORT),y)
    EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT

    func_objs += $(SRC_EMBEDDED_DIR)/common/igmp_snoop.o
endif

ifeq ($(CONFIG_BLOCK_NET_IF),y)
    EXTRA_CFLAGS += -DBLOCK_NET_IF

    func_objs += $(SRC_EMBEDDED_DIR)/common/netif_block.o
endif

ifeq ($(CONFIG_SINGLE_SKU),y)
    EXTRA_CFLAGS += -DSINGLE_SKU
    ifeq ($(CONFIG_RALINK_RT6352),y)
        EXTRA_CFLAGS += -DSINGLE_SKU_V2
    endif
endif

ifeq ($(CONFIG_RT2860V2_AP_VIDEO_TURBINE),y)
    EXTRA_CFLAGS += -DVIDEO_TURBINE_SUPPORT

    func_objs += $(SRC_EMBEDDED_DIR)/common/cmm_video.o
endif


ifeq ($(CONFIG_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
    ifeq ($(CONFIG_WSC_INCLUDED),y)
        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
    endif

    func_objs += $(SRC_EMBEDDED_DIR)/common/rt_led.o
endif


########################################################
# STA feature related files
########################################################
sta_objs := \
	$(SRC_EMBEDDED_DIR)/sta/assoc.o\
	$(SRC_EMBEDDED_DIR)/sta/auth.o\
	$(SRC_EMBEDDED_DIR)/sta/auth_rsp.o\
	$(SRC_EMBEDDED_DIR)/sta/sync.o\
	$(SRC_EMBEDDED_DIR)/sta/sanity.o\
	$(SRC_EMBEDDED_DIR)/sta/sta_data.o\
	$(SRC_EMBEDDED_DIR)/sta/connect.o\
    $(SRC_EMBEDDED_DIR)/sta/sta_sec.o\
	$(SRC_EMBEDDED_DIR)/sta/wpa.o\
	$(SRC_EMBEDDED_DIR)/sta/sta_cfg.o\
	$(SRC_EMBEDDED_DIR)/sta/sta.o

ifeq ($(CONFIG_ATE_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_ATE -DCONFIG_QA -DCONFIG_RT2880_ATE_CMD_NEW -DCONFIG_HW_HAL_OFFLOAD  #-DCSD_VERIFICATION -DINTERNAL_CAPTURE_SUPPORT
endif

ifeq ($(CONFIG_ICAP_SUPPORT),y)
		EXTRA_CFLAGS += -DINTERNAL_CAPTURE_SUPPORT
endif


ifeq ($(CONFIG_RTMP_FLASH_SUPPORT),y)
    EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT
endif

ifeq ($(CONFIG_RTMP_TEMPERATURE_TX_ALC),y)
    EXTRA_CFLAGS += -DRTMP_TEMPERATURE_TX_ALC
endif

ifeq ($(CONFIG_TXBF_SUPPORT),y)
    EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DTXBF_SUPPORT -DVHT_TXBF_SUPPORT
    sta_objs += $(SRC_DIR)/txbf/cmm_txbf_mt.o\
                    $(SRC_DIR)/txbf/cmm_txbf_cal_mt.o\
                    $(SRC_DIR)/txbf/txbf_wrapper_embedded.o
endif

ifeq ($(CONFIG_CON_WPS_SUPPORT), y)
    EXTRA_CFLAGS += -DCON_WPS -DCON_WPS_AP_SAME_UUID
endif

ifeq ($(CONFIG_DFS_SUPPORT),y)
    EXTRA_CFLAGS += -DDFS_SUPPORT
    EXTRA_CFLAGS += -DDFS_ATP_SUPPORT
    ap_objs += $(SRC_EMBEDDED_DIR)/common/cmm_dfs.o
endif

ifeq ($(CONFIG_RT2860V2_AP_CARRIER),y)
    EXTRA_CFLAGS += -DCARRIER_DETECTION_SUPPORT

    ap_objs += $(SRC_EMBEDDED_DIR)/common/cmm_cs.o
endif

ifeq ($(CONFIG_RT2860V2_MCAST_RATE_SPECIFIC),y)
    EXTRA_CFLAGS += -DMCAST_RATE_SPECIFIC
endif

ifeq ($(CONFIG_AIRPLAY_SUPPORT),y)
    EXTRA_CFLAGS += -DAIRPLAY_SUPPORT
endif

ifeq ($(CONFIG_ETH_CONVERT_SUPPORT),y)
    EXTRA_CFLAGS += -DETH_CONVERT_SUPPORT -DMAT_SUPPORT
    sta_objs += $(SRC_EMBEDDED_DIR)/common/cmm_mat.o\
    		$(SRC_EMBEDDED_DIR)/common/cmm_mat_iparp.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_mat_pppoe.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_mat_ipv6.o
endif

########################################################
# Linux system related files
########################################################
os_objs := $(SRC_DIR)/os/linux/rt_proc.o\
            $(SRC_DIR)/os/linux/rt_linux.o\
            $(SRC_DIR)/os/linux/rt_profile.o\
            $(SRC_DIR)/os/linux/rt_txrx_hook.o\
            $(SRC_DIR)/os/linux/rt_main_dev.o\
            $(SRC_DIR)/os/linux/sta_ioctl.o\

ifeq ($(CONFIG_MULTI_INF_SUPPORT),y)
    EXTRA_CFLAGS += -DMULTI_INF_SUPPORT

    os_objs += $(SRC_DIR)/os/linux/multi_main_dev.o
endif

ifeq ($(CONFIG_WIFI_WORK_QUEUE_BH),y)
    EXTRA_CFLAGS += -DWORKQUEUE_BH
endif

ifeq ($(CONFIG_KTHREAD),y)
    EXTRA_CFLAGS += -DKTHREAD_SUPPORT
endif


########################################################
# chip related files
########################################################
ifeq ($(CONFIG_FIRST_IF_MT7628),y)
EXTRA_CFLAGS += -DMT7628 -DMT_BBP -DMT_RF -DRTMP_RBUS_SUPPORT -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT
EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT -DDMA_SCH_SUPPORT -DRTMP_EFUSE_SUPPORT
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC
EXTRA_CFLAGS += -DNEW_MBSSID_MODE -DENHANCE_NEW_MBSSID_MODE
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DFIFO_EXT_SUPPORT
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT
EXTRA_CFLAGS += -DUSE_BMC -DTHERMAL_PROTECT_SUPPORT -DCAL_FREE_IC_SUPPORT

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_smac.o
endif

chip_objs += $(SRC_DIR)/chips/mt7628.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt.o\
		$(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
		$(SRC_DIR)/mac/mt_mac.o\
		$(SRC_DIR)/mcu/mcu.o\
		$(SRC_DIR)/mcu/andes_core.o\
		$(SRC_DIR)/mcu/andes_mt.o\
		$(SRC_DIR)/phy/mt_rf.o\
		$(SRC_DIR)/phy/rf.o\
		$(SRC_DIR)/phy/mt_phy.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
		$(SRC_EMBEDDED_DIR)/common/hif_pci.o\
		$(SRC_EMBEDDED_DIR)/common/ee_prom.o\
		$(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
		$(SRC_DIR)/os/linux/rbus_main_dev.o
endif


ifeq ($(CONFIG_SECOND_IF_MT7637E),y)
EXTRA_CFLAGS += -DMT7637 -DMT_BBP -DMT_RF -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DA_BAND_SUPPORT -DDMA_SCH_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
#EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT
# -DCUT_THROUGH
#EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
#EXTRA_CFLAGS += -DDBDC_MODE
#EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_smac.o
endif

chip_objs += $(SRC_DIR)/chips/mt7637.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt_fw.o\
		$(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
		$(SRC_DIR)/mac/mt_mac.o\
                $(SRC_EMBEDDED_DIR)/mcu/mcu.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_core.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_mt.o \
		$(SRC_DIR)/mcu/mt_cmd.o\
		$(SRC_DIR)/mcu/fwdl_mt.o\
		$(SRC_DIR)/mcu/fw_cmd.o\
		$(SRC_DIR)/mcu/fwdl.o\
                $(SRC_DIR)/phy/mt_rf.o\
                $(SRC_DIR)/phy/rf.o\
                $(SRC_DIR)/phy/mt_phy.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
		$(SRC_DIR)/protocol/tmr.o\
		$(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
                $(SRC_DIR)/os/linux/pci_main_dev.o
endif


ifeq ($(CONFIG_CHIP_MT7615E),y)
EXTRA_CFLAGS += -DMT7615 -DMT_BBP -DMT_RF -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DRX_SCATTER
ifeq ($(CONFIG_G_BAND_256QAM_SUPPORT), y)
EXTRA_CFLAGS += -DG_BAND_256QAM
ifeq ($(CONFIG_BRCM_256QAM_SUPPORT), y)
EXTRA_CFLAGS += -DBRCM_256QAM
endif
endif
EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
EXTRA_CFLAGS += -DDBDC_MODE
EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
EXTRA_CFLAGS += -DCONFIG_KEEP_ALIVE_OFFLOAD -DSTA_LP_PHASE_2_SUPPORT -DCONFIG_PM_BIT_HW_MODE
EXTRA_CFLAGS += -DMT_WOW_SUPPORT -DCONFIG_PM
EXTRA_CFLAGS += -DRATE_ADAPTION
EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif
ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
#	EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_dmac.o
endif

chip_objs += $(SRC_DIR)/chips/mt7615.o\
		$(SRC_DIR)/chips/mt7615_dbg.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt_dmac.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt_fw.o\
		$(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
		$(SRC_DIR)/mac/mt_dmac.o\
                $(SRC_EMBEDDED_DIR)/mcu/mcu.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_core.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_mt.o \
		$(SRC_DIR)/mcu/mt_cmd.o\
		$(SRC_DIR)/mcu/fwdl_mt.o\
		$(SRC_DIR)/mcu/fw_cmd.o\
		$(SRC_DIR)/mcu/fwdl.o\
                $(SRC_DIR)/phy/mt_rf.o\
                $(SRC_DIR)/phy/rf.o\
                $(SRC_DIR)/phy/mt_phy.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
		$(SRC_EMBEDDED_DIR)/common/cut_through.o\
		$(SRC_DIR)/protocol/tmr.o\
		$(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
                $(SRC_DIR)/os/linux/pci_main_dev.o
endif


ifeq ($(CONFIG_FIRST_IF_MT7622),y)
EXTRA_CFLAGS += -DMT7622 -DMT_BBP -DMT_RF -DRTMP_RBUS_SUPPORT -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI
EXTRA_CFLAGS += -DMT_DMAC
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
#EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DCONFIG_KEEP_ALIVE_OFFLOAD -DSTA_LP_PHASE_2_SUPPORT -DCONFIG_PM_BIT_HW_MODE
EXTRA_CFLAGS += -DMT_WOW_SUPPORT -DCONFIG_PM -DHOST_RESUME_DONE_ACK_SUPPORT
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC
EXTRA_CFLAGS += -DCONFIG_HW_HAL_OFFLOAD
#EXTRA_CFLAGS += -DCONFIG_FPGA_MODE
#EXTRA_CFLAGS += -DMT7622_FPGA
#EXTRA_CFLAGS += -DCONFIG_CSO_SUPPORT
#EXTRA_CFLAGS += -DMSP_UTILITY
EXTRA_CFLAGS += -DRATE_ADAPTION
EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
CONFIG_PROPRIETART_DRIVER=n

ifeq ($(CONFIG_PLOPRIETART_DRIVER),y)
EXTRA_CFLAGS += -DCONFIG_PROPRIETARY_DRIVER
CONFIG_PLATFORM_DRV_SUPPORT=n
else
CONFIG_PLATFORM_DRV_SUPPORT=y
endif

ifeq ($(CONFIG_PLATFORM_DRV_SUPPROT),y)
EXTRA_CFLAGS += -DPLATFORM_DRIVER_SUPPORT
endif

EXTRA_CFLAGS += -Werror
ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
        EXTRA_CFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif
ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
        EXTRA_CFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
#        EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_dmac.o
endif

chip_objs += $(SRC_DIR)/chips/mt7622.o\
		$(SRC_DIR)/chips/mt7622_dbg.o\
                $(SRC_DIR)/hw_ctrl/cmm_asic_mt_dmac.o\
                $(SRC_DIR)/hw_ctrl/cmm_asic_mt_fw.o\
                $(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
                $(SRC_DIR)/mac/mt_dmac.o\
                $(SRC_EMBEDDED_DIR)/mcu/mcu.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_core.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_mt.o \
                $(SRC_DIR)/mcu/mt_cmd.o\
                $(SRC_DIR)/mcu/fwdl_mt.o\
                $(SRC_DIR)/mcu/fw_cmd.o\
                $(SRC_DIR)/mcu/fwdl.o\
                $(SRC_DIR)/phy/mt_rf.o\
                $(SRC_DIR)/phy/rf.o\
                $(SRC_DIR)/phy/mt_phy.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
                $(SRC_EMBEDDED_DIR)/common/cut_through.o\
                $(SRC_DIR)/protocol/tmr.o\
                $(SRC_DIR)/os/linux/rt_rbus_pci_drv.o
endif

ifeq ($(CONFIG_RTMP_FLASH_SUPPORT)$(CONFIG_MTD),yy)
ifeq ($(CONFIG_DEFAULTS_KERNEL_4_4),y)
ifeq ($(CONFIG_MTD_SPI_NOR),y)
EXTRA_CFLAGS += -DCONFIG_WIFI_MTD
else
ifeq ($(CONFIG_MTD_NAND),y)
EXTRA_CFLAGS += -DCONFIG_WIFI_MTD
endif
endif
endif
endif

ifeq ($(CONFIG_DEFAULTS_MEDIATEK_MT7622),y)
EXTRA_CFLAGS += -DCONFIG_WIFI_MTD
endif


ifeq ($(CONFIG_PLATFORM_DRV_SUPPORT),y)
chip_objs += $(SRC_DIR)/os/linux/wbsys_main_dev.o
else
chip_objs += $(SRC_DIR)/os/linux/rbus_prop_dev.o
endif

#
# Root
#

$(DRV_NAME)-objs += $(sta_objs) $(cmm_objs) $(asic_objs) $(chip_objs) $(rate_objs)\
                    $(spec_objs) $(func_objs) $(os_objs)

$(DRV_NAME)-objs += $(SRC_EMBEDDED_DIR)/common/eeprom.o\
					$(SRC_EMBEDDED_DIR)/common/ee_flash.o\
					$(SRC_EMBEDDED_DIR)/common/ee_efuse.o

$(DRV_NAME)-objs += $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o
$(DRV_NAME)-objs += $(SRC_EMBEDDED_DIR)/common/hif_pci.o

$(DRV_NAME)-objs += $(SRC_DIR)/os/linux/rt_pci_rbus.o\
                    $(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
                    $(SRC_DIR)/os/linux/rt_rbus_pci_util.o

ifeq ($(CONFIG_ATE_SUPPORT),y)
$(DRV_NAME)-objs += $(SRC_DIR)/ate/ate_agent.o\
                    $(SRC_DIR)/ate/testmode_ioctl.o\
		    $(SRC_DIR)/ate/mt_mac/mt_testmode.o\
 		    $(SRC_DIR)/ate/LoopBack.o
endif


###################
#  CFLAGS
##################
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT  -DLINUX \
              -Wstrict-prototypes -Wno-trigraphs -Wall -Wno-unused-but-set-variable

#-Werror
#-DDBG_DIAGNOSE -DDBG_RX_MCS -DDBG_TX_MCS

EXTRA_CFLAGS += -DCONFIG_STA_SUPPORT -DSCAN_SUPPORT -DDBG
#-DCONFIG_AP_SUPPORT -DSCAN_SUPPORT -DAP_SCAN_SUPPORT
EXTRA_CFLAGS += -DDOT11_N_SUPPORT -DDOT11N_DRAFT3 -DSTATS_COUNT_SUPPORT
# -DIAPP_SUPPORT -DDOT1X_SUPPORT
#EXTRA_CFLAGS += -DRALINK_ATE -DRALINK_QA -DCONFIG_RT2880_ATE_CMD_NEW
ifneq ($(CONFIG_RA_HW_NAT_WIFI_NEW_ARCH),y)
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
#EXTRA_CFLAGS += -DDBG
endif
EXTRA_CFLAGS += -DDBG

ifeq ($(CONFIG_RTMP_INTERNAL_TX_ALC),y)
EXTRA_CFLAGS += -DRTMP_INTERNAL_TX_ALC
endif

ifeq ($(CONFIG_SUPPORT_DYNAMIC_TXOP),y)
EXTRA_CFLAGS += -DTXOP_ARBITER
endif

ifeq ($(CONFIG_RALINK_MT7621),y)
EXTRA_CFLAGS += -DRACTRL_LIMIT_MAX_PHY_RATE=1300
endif

EXTRA_CFLAGS += -DIP_ASSEMBLY
EXTRA_CFLAGS += -Inet/nat

MODULE_FLAGS=$(EXTRA_CFLAGS)
export MODULE_FLAGS
obj-m+=$(SRC_EMBEDDED_DIR)/tools/plug_in/

