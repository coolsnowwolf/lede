ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
WIFI_DRV_PATH := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/..
else
ifeq ($(CONFIG_DEFAULTS_KERNEL_4_4),y)
WIFI_DRV_PATH = drivers/net/wireless/mediatek
else
WIFI_DRV_PATH = drivers/net/wireless/mtk/mt7615e
endif
endif

EXTRA_CFLAGS = -I$(WIFI_DRV_PATH)/mt_wifi/include \
				-I$(WIFI_DRV_PATH)/mt_wifi/embedded/include \
				-I$(WIFI_DRV_PATH)/mt_wifi/ate/include \
				-I$(WIFI_DRV_PATH)/mt_wifi/include/mcu

DRV_NAME = mt_wifi
SRC_DIR = ../mt_wifi
SRC_EMBEDDED_DIR = ../mt_wifi/embedded
ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
EXTRA_CFLAGS += -DCONFIG_SUPPORT_OPENWRT
ifneq ($(CONFIG_RT_FIRST_CARD),)
EXTRA_CFLAGS += -DCONFIG_RT_FIRST_CARD=$(CONFIG_RT_FIRST_CARD)
EXTRA_CFLAGS += -DCONFIG_RT_FIRST_IF_RF_OFFSET=$(CONFIG_RT_FIRST_IF_RF_OFFSET)
endif
ifneq ($(CONFIG_RT_SECOND_CARD),)
EXTRA_CFLAGS += -DCONFIG_RT_SECOND_CARD=$(CONFIG_RT_SECOND_CARD)
EXTRA_CFLAGS += -DCONFIG_RT_SECOND_IF_RF_OFFSET=$(CONFIG_RT_SECOND_IF_RF_OFFSET)
endif
ifneq ($(CONFIG_RT_THIRD_CARD),)
EXTRA_CFLAGS += -DCONFIG_RT_THIRD_CARD=$(CONFIG_RT_THIRD_CARD)
EXTRA_CFLAGS += -DCONFIG_RT_THIRD_IF_RF_OFFSET=$(CONFIG_RT_THIRD_IF_RF_OFFSET)
endif
ifeq ($(CONFIG_FIRST_IF_EEPROM_PROM),y)
EXTRA_CFLAGS += -DCONFIG_RT_FIRST_CARD_EEPROM="\"prom\""
else
ifeq ($(CONFIG_FIRST_IF_EEPROM_EFUSE),y)
EXTRA_CFLAGS += -DCONFIG_RT_FIRST_CARD_EEPROM="\"efuse\""
else
ifeq ($(CONFIG_FIRST_IF_EEPROM_FLASH),y)
EXTRA_CFLAGS += -DCONFIG_RT_FIRST_CARD_EEPROM="\"flash\""
endif
endif
endif
ifeq ($(CONFIG_SECOND_IF_EEPROM_PROM),y)
EXTRA_CFLAGS += -DCONFIG_RT_SECOND_CARD_EEPROM="\"prom\""
else
ifeq ($(CONFIG_SECOND_IF_EEPROM_EFUSE),y)
EXTRA_CFLAGS += -DCONFIG_RT_SECOND_CARD_EEPROM="\"efuse\""
else
ifeq ($(CONFIG_SECOND_IF_EEPROM_FLASH),y)
EXTRA_CFLAGS += -DCONFIG_RT_SECOND_CARD_EEPROM="\"flash\""
endif
endif
endif
ifeq ($(CONFIG_THIRD_IF_EEPROM_PROM),y)
EXTRA_CFLAGS += -DCONFIG_RT_THIRD_CARD_EEPROM="\"prom\""
else
ifeq ($(CONFIG_THIRD_IF_EEPROM_EFUSE),y)
EXTRA_CFLAGS += -DCONFIG_RT_THIRD_CARD_EEPROM="\"efuse\""
else
ifeq ($(CONFIG_THIRD_IF_EEPROM_FLASH),y)
EXTRA_CFLAGS += -DCONFIG_RT_THIRD_CARD_EEPROM="\"flash\""
endif
endif
endif
endif

ifeq ($(CONFIG_FIRST_IF_EPAELNA),y)
EXTRA_CFLAGS += -DCONFIG_FIRST_IF_EPAELNA
else
ifeq ($(CONFIG_FIRST_IF_IPAILNA),y)
EXTRA_CFLAGS += -DCONFIG_FIRST_IF_IPAILNA
else
ifeq ($(CONFIG_FIRST_IF_IPAELNA),y)
EXTRA_CFLAGS += -DCONFIG_FIRST_IF_IPAELNA
endif
endif
endif
ifeq ($(CONFIG_SECOND_IF_EPAELNA),y)
EXTRA_CFLAGS += -DCONFIG_SECOND_IF_EPAELNA
else
ifeq ($(CONFIG_SECOND_IF_IPAILNA),y)
EXTRA_CFLAGS += -DCONFIG_SECOND_IF_IPAILNA
else
ifeq ($(CONFIG_SECOND_IF_IPAELNA),y)
EXTRA_CFLAGS += -DCONFIG_SECOND_IF_IPAELNA
endif
endif
endif
ifeq ($(CONFIG_THIRD_IF_EPAELNA),y)
EXTRA_CFLAGS += -DCONFIG_THIRD_IF_EPAELNA
else
ifeq ($(CONFIG_THIRD_IF_IPAILNA),y)
EXTRA_CFLAGS += -DCONFIG_THIRD_IF_IPAILNA
else
ifeq ($(CONFIG_THIRD_IF_IPAELNA),y)
EXTRA_CFLAGS += -DCONFIG_THIRD_IF_IPAELNA
endif
endif
endif

#support built-in/ko for Openwrt/Linux SDK
obj-$(CONFIG_MT_AP_SUPPORT) += $(DRV_NAME).o

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
		$(SRC_EMBEDDED_DIR)/common/fp_qm.o\
		$(SRC_EMBEDDED_DIR)/common/fq_qm.o\
		$(SRC_EMBEDDED_DIR)/common/fp_fair_qm.o\
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
		$(SRC_EMBEDDED_DIR)/common/cmm_sanity.o\
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
                $(SRC_EMBEDDED_DIR)/mgmt/bss_ops.o\
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
		$(SRC_EMBEDDED_DIR)/common/icap.o\
		$(SRC_EMBEDDED_DIR)/common/vendor.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_info_element.o\
		$(SRC_DIR)/chips/rtmp_chip.o\
		$(SRC_EMBEDDED_DIR)/mgmt/mgmt_hw.o\
		$(SRC_EMBEDDED_DIR)/mgmt/mgmt_entrytb.o\
		$(SRC_EMBEDDED_DIR)/common/wdev.o\
		$(SRC_DIR)/os/linux/rt_profile.o\
		$(SRC_DIR)/os/linux/tm.o\
		$(SRC_DIR)/txpwr/single_sku.o\
		$(SRC_EMBEDDED_DIR)/common/ra_ac_q_mgmt.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_rvr_dbg.o\
		$(SRC_DIR)/protocol/protection.o\
		$(SRC_EMBEDDED_DIR)/common/misc_app.o

EXTRA_CFLAGS += -DMULTI_PROFILE
cmm_objs += $(SRC_EMBEDDED_DIR)/common/multi_profile.o

ifeq ($(CONFIG_DEFAULT_5G_PROFILE),y)
EXTRA_CFLAGS += -DDEFAULT_5G_PROFILE
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

#ifeq ($(CONFIG_NEW_RATE_ADAPT_SUPPORT),y)
#    EXTRA_CFLAGS += -DNEW_RATE_ADAPT_SUPPORT
#    rate_objs += $(SRC_DIR)/rate_ctrl/alg_grp_mt.o
#    rate_objs += $(SRC_DIR)/rate_ctrl/ra_table.o
#endif

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
    EXTRA_CFLAGS += -DWSC_INCLUDED -DWSC_SINGLE_TRIGGER

    ifneq ($(CONFIG_USER_GOAHEAD_HTTPD),)
        EXTRA_CFLAGS += -DSDK_GOAHEAD_HTTPD
    endif

    ifneq ($(CONFIG_USER_LIGHTY),)
        EXTRA_CFLAGS += -DSDK_USER_LIGHTY
    endif

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
                    $(SRC_EMBEDDED_DIR)/common/wsc_ufd.o

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

# WPA3
ifeq ($(CONFIG_WPA3_SUPPORT),y)
    EXTRA_CFLAGS += -DDOT11_SAE_SUPPORT -DDOT11_SUITEB_SUPPORT -DDOT11W_PMF_SUPPORT
    EXTRA_CFLAGS += -DDOT11_SAE_OPENSSL_BN

  spec_objs += $(SRC_EMBEDDED_DIR)/security/sae.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/crypt_bignum.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/crypt_biginteger.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/bn_lib.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/ecc.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/pmf.o

  ifeq ($(CONFIG_APCLI_SUPPORT),y)
      EXTRA_CFLAGS += -DAPCLI_SAE_SUPPORT
  endif

endif

# OWE
ifeq ($(CONFIG_OWE_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_OWE_SUPPORT -DDOT11W_PMF_SUPPORT
    EXTRA_CFLAGS += -DDOT11_SAE_OPENSSL_BN

  spec_objs += $(SRC_EMBEDDED_DIR)/security/crypt_bignum.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/crypt_biginteger.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/bn_lib.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/ecc.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/owe.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/pmf.o

  ifeq ($(CONFIG_APCLI_SUPPORT),y)
      EXTRA_CFLAGS += -DAPCLI_OWE_SUPPORT
  endif

endif

# ACM
ifeq ($(CONFIG_WMM_ACM_SUPPORT),y)
    EXTRA_CFLAGS += -DWMM_ACM_SUPPORT

    spec_objs += $(SRC_EMBEDDED_DIR)/common/acm_edca.o\
            $(SRC_EMBEDDED_DIR)/common/acm_comm.o\
            $(SRC_EMBEDDED_DIR)/common/acm_iocl.o
endif

#PMF
ifeq ($(CONFIG_MT_MAC),y)
ifeq ($(CONFIG_DOT11W_PMF_SUPPORT),y)
    EXTRA_CFLAGS += -DDOT11W_PMF_SUPPORT

    spec_objs += $(SRC_EMBEDDED_DIR)/security/pmf.o
endif
endif

# LLTD
ifeq ($(CONFIG_LLTD_SUPPORT),y)
    EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

#RRM
ifeq ($(CONFIG_DOT11K_RRM_SUPPORT),y)
    EXTRA_CFLAGS += -DDOT11K_RRM_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT -DAPPLE_11K_IOT
    spec_objs += $(SRC_EMBEDDED_DIR)/common/rrm_tlv.o \
				 $(SRC_EMBEDDED_DIR)/common/rrm.o \
				 $(SRC_EMBEDDED_DIR)/common/rrm_sanity.o
endif
#EXTRA_CFLAGS += -DHOSTAPD_11K_SUPPORT
# TPC
ifeq ($(CONFIG_TPC_SUPPORT),y)
    EXTRA_CFLAGS += -DTPC_SUPPORT
endif

# FTM
ifeq ($(CONFIG_FTM_SUPPORT),y)
    EXTRA_CFLAGS += -DFTM_SUPPORT -DFTM_INITIATOR
    EXTRA_CFLAGS += -DTMR_SUPPORT
    EXTRA_CFLAGS += -DDOT11K_RRM_SUPPORT

    spec_objs += $(SRC_EMBEDDED_DIR)/common/ftm.o
    spec_objs += $(SRC_DIR)/protocol/tmr.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/rrm_tlv.o \
				 $(SRC_EMBEDDED_DIR)/common/rrm.o \
				 $(SRC_EMBEDDED_DIR)/common/rrm_sanity.o
endif

ifeq ($(CONFIG_PASSPOINT_R2),y)
	EXTRA_CFLAGS += -DCONFIG_DOT11U_INTERWORKING -DCONFIG_DOT11V_WNM -DCONFIG_HOTSPOT -DCONFIG_HOTSPOT_R2 -DWAPP_SUPPORT

	spec_objs += $(SRC_EMBEDDED_DIR)/common/wnm.o\
				$(SRC_EMBEDDED_DIR)/common/gas.o\
				$(SRC_EMBEDDED_DIR)/common/hotspot.o
        spec_objs += $(SRC_EMBEDDED_DIR)/wapp/wapp.o
endif

ifeq ($(CONFIG_DSCP_QOS_MAP_SUPPORT),y)
	EXTRA_CFLAGS += -DDSCP_QOS_MAP_SUPPORT
endif

ifeq ($(CONFIG_DSCP_PRI_SUPPORT),y)
	EXTRA_CFLAGS += -DDSCP_PRI_SUPPORT
endif

ifeq ($(CONFIG_MIN_PHY_RATE_SUPPORT),y)
	EXTRA_CFLAGS += -DMIN_PHY_RATE_SUPPORT
endif

ifeq ($(CONFIG_FAST_UP_RATE_SUPPORT),y)
	EXTRA_CFLAGS += -DFAST_UP_RATE_SUPPORT
endif

ifeq ($(CONFIG_WNM_SUPPORT),y)
        EXTRA_CFLAGS += -DCONFIG_DOT11V_WNM

        spec_objs += $(SRC_EMBEDDED_DIR)/common/wnm.o
endif

ifeq ($(CONFIG_INTERWORKING),y)
        EXTRA_CFLAGS += -DCONFIG_DOT11U_INTERWORKING
        spec_objs += $(SRC_EMBEDDED_DIR)/common/gas.o
endif

# MBO
ifeq ($(CONFIG_MBO_SUPPORT),y)
    EXTRA_CFLAGS += -DMBO_SUPPORT -DWAPP_SUPPORT
    spec_objs += $(SRC_EMBEDDED_DIR)/common/mbo.o
    spec_objs += $(SRC_EMBEDDED_DIR)/wapp/wapp.o
endif

ifeq ($(CONFIG_TRACE_TCP_PKT),y)
       EXTRA_CFLAGS += -DTRACELOG_TCP_PKT
endif

ifeq ($(CONFIG_BACKGROUND_SCAN_SUPPORT),y)
    EXTRA_CFLAGS += -DBACKGROUND_SCAN_SUPPORT

    spec_objs += $(SRC_EMBEDDED_DIR)/common/bgnd_scan.o
endif

ifeq ($(CONFIG_MT_DFS_SUPPORT),y)
	EXTRA_CFLAGS += -DMT_DFS_SUPPORT -DCUSTOMISE_RDD_THRESHOLD_SUPPORT -DRDM_FALSE_ALARM_DEBUG_SUPPORT

	spec_objs += $(SRC_EMBEDDED_DIR)/common/cmm_rdm_mt.o
endif

ifeq ($(CONFIG_WIFI_EAP_FEATURE),y)
	EXTRA_CFLAGS += -DWIFI_EAP_FEATURE
endif

# FT
ifeq ($(CONFIG_DOT11R_FT_SUPPORT),y)
EXTRA_CFLAGS += -DDOT11R_FT_SUPPORT
dot11_ft_objs += $(SRC_EMBEDDED_DIR)/common/ft.o\
                    $(SRC_EMBEDDED_DIR)/common/ft_tlv.o\
                    $(SRC_EMBEDDED_DIR)/common/ft_iocl.o\
                    $(SRC_EMBEDDED_DIR)/common/ft_rc.o\
                    $(SRC_EMBEDDED_DIR)/ap/ap_ftkd.o
## Preserve R1KH during Radio ON/OFF
ifeq ($(CONFIG_MBO_SUPPORT),y)
EXTRA_CFLAGS += -DFT_R1KH_KEEP
endif
endif

#SNIFFER
ifeq ($(CONFIG_SNIFFER_SUPPORT),y)
    EXTRA_CFLAGS += -DSNIFFER_SUPPORT

    spec_objs += $(SRC_EMBEDDED_DIR)/common/sniffer_radiotap.o\
                 $(SRC_EMBEDDED_DIR)/common/sniffer_prism.o
endif

#EAP-TXRX Statsitics
ifeq ($(CONFIG_TXRX_STAT_SUPPORT),y)
EXTRA_CFLAGS += -DTXRX_STAT_SUPPORT
endif

#VENDOR_FEATURE11
ifeq ($(CONFIG_VENDOR_FEATURE11_SUPPORT),y)
EXTRA_CFLAGS += -DVENDOR_FEATURE11_SUPPORT -DTXRX_STAT_SUPPORT
endif

#HOSTAPD_MAP_SUPPORT
ifeq ($(CONFIG_HOSTAPD_MAP_SUPPORT),y)
	EXTRA_CFLAGS += -DHOSTAPD_MAP_SUPPORT
endif

#ENTERPRISE_AP
ifeq ($(CONFIG_ENTERPRISE_AP_SUPPORT),y)
	EXTRA_CFLAGS += -DHOSTAPD_11R_SUPPORT
	EXTRA_CFLAGS += -DMBSS_DTIM_SUPPORT
	ifeq ($(CONFIG_VLAN_SUPPORT),y)
		EXTRA_CFLAGS += -DVLAN_SUPPORT
	endif
	ifeq ($(CONFIG_RADIUS_MAC_AUTH_SUPPORT),y)
		EXTRA_CFLAGS += -DRADIUS_MAC_AUTH_SUPPORT
	endif
	ifeq ($(CONFIG_DYNAMIC_VLAN_SUPPORT),y)
		EXTRA_CFLAGS += -DDYNAMIC_VLAN_SUPPORT
	endif

endif

#CFG
ifeq ($(CONFIG_CFG80211_SUPPORT),y)
    EXTRA_CFLAGS += -DRT_CFG80211_SUPPORT -DEXT_BUILD_CHANNEL_LIST
    #WPA3 SUPPORT
	ifneq ($(CONFIG_HOSTAPD_MAP_SUPPORT),y)
		EXTRA_CFLAGS += -DHOSTAPD_SAE_SUPPORT -DHOSTAPD_OWE_SUPPORT -DHOSTAPD_SUITEB_SUPPORT
	endif
	ifeq ($(CONFIG_CUSTOMISED_HOSTAPD_SUPPORT),y)
		EXTRA_CFLAGS += -DDISABLE_HOSTAPD_BEACON -DDISABLE_HOSTAPD_PROBE_RESP -DHOSTAPD_AUTO_CH_SUPPORT
	endif
	ifeq ($(CONFIG_WDS_STA_SUPPORT),y)
		ifneq ($(CONFIG_HOSTAPD_MAP_SUPPORT),y)
			EXTRA_CFLAGS += -DAPCLI_AS_WDS_STA_SUPPORT
		endif
	endif
	ifeq ($(CONFIG_APCLI_STA_SUPPORT),y)
		ifneq ($(CONFIG_HOSTAPD_MAP_SUPPORT),y)
			EXTRA_CFLAGS += -DAPCLI_CFG80211_SUPPORT
			spec_objs += $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211_apcli.o
		endif
	endif
    spec_objs += $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211_scan.o\
		        $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211_rx.o\
        		$(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211_tx.o\
		        $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211_inf.o\
        		$(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211_ap.o\
		        $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211drv.o\
                $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211_util.o\
                $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211.o

endif

ifeq ($(CONFIG_MAP_SUPPORT),y)
	EXTRA_CFLAGS += -DA4_CONN -DROUTING_TAB_SUPPORT -DWAPP_SUPPORT -DCONFIG_MAP_SUPPORT -DOFFCHANNEL_SCAN_FEATURE
	spec_objs += $(SRC_EMBEDDED_DIR)/common/map.o
	spec_objs += $(SRC_EMBEDDED_DIR)/common/a4_conn.o
	spec_objs += $(SRC_EMBEDDED_DIR)/common/routing_tab.o
	spec_objs += $(SRC_EMBEDDED_DIR)/wapp/wapp.o
endif

ifeq ($(CONFIG_RALINK_TIMER_WDG),y)
    EXTRA_CFLAGS += -DSDK_TIMER_WDG
endif

ifeq ($(CONFIG_OFFCHANNEL_SCAN_FEATURE),y)
    EXTRA_CFLAGS += -DOFFCHANNEL_SCAN_FEATURE
endif

ifeq ($(CONFIG_VENDOR_FEATURE10_SUPPORT),y)
    EXTRA_CFLAGS += -DFOLLOW_HIDDEN_SSID_FEATURE
    EXTRA_CFLAGS += -DCONVERTER_MODE_SWITCH_SUPPORT
    ifeq ($(CONFIG_MT_DFS_SUPPORT),y)
        EXTRA_CFLAGS += -DDFS_VENDOR10_CUSTOM_FEATURE
    endif
    ifeq ($(CONFIG_MCAST_RATE_SPECIFIC),y)
        EXTRA_CFLAGS += -DMCAST_BCAST_RATE_SET_SUPPORT
    endif
    ifeq ($(CONFIG_IGMP_SNOOP_SUPPORT),y)
        EXTRA_CFLAGS += -DIGMP_TVM_SUPPORT
    endif
    ifeq ($(CONFIG_APCLI_SUPPORT),y)
        EXTRA_CFLAGS += -DVENDOR10_CUSTOM_RSSI_FEATURE
	EXTRA_CFLAGS += -DBW_VENDOR10_CUSTOM_FEATURE
    endif
endif

ifeq ($(CONFIG_MT_DFS_SUPPORT),y)
    EXTRA_CFLAGS += -DONDEMAND_DFS
endif

ifeq ($(CONFIG_MTK_WD_KICKER),y)
    ifeq ($(CONFIG_MTK_WD_KICKER_WCN),y)
        EXTRA_CFLAGS += -DSDK_TIMER_WDG
    endif
endif

#ACL_BLK_CONUT
#EXTRA_CFLAGS += -DACL_BLK_COUNT_SUPPORT

#RATE_PRIOR_SUPPORT
#EXTRA_CFLAGS += -DRATE_PRIOR_SUPPORT

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

ifeq ($(CONFIG_SMART_CARRIER_SENSE_SUPPORT),y)
    EXTRA_CFLAGS += -DSMART_CARRIER_SENSE_SUPPORT

        spec_objs += $(SRC_EMBEDDED_DIR)/common/scs.o
endif

########################################################
# AP feature related files
########################################################
ap_objs := $(SRC_EMBEDDED_DIR)/ap/ap.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_assoc.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_auth.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_mlme.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_sanity.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_sync.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_wpa.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_sec.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_data.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_autoChSel.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_qload.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_cfg.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_nps.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_vow.o\
            $(SRC_DIR)/os/linux/ap_ioctl.o

ifeq ($(CONFIG_MBSS_SUPPORT),y)
    EXTRA_CFLAGS += -DMBSS_SUPPORT

    ifeq ($(CONFIG_NEW_MBSSID_MODE),y)
        EXTRA_CFLAGS += -DNEW_MBSSID_MODE
        ifeq ($(CONFIG_ENHANCE_NEW_MBSSID_MODE),y)
            EXTRA_CFLAGS += -DENHANCE_NEW_MBSSID_MODE
        endif
    endif

    ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_mbss.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_mbss_inf.o

ifeq ($(CONFIG_MBSS_AS_WDS_AP_SUPPORT),y)
EXTRA_CFLAGS += -DMBSS_AS_WDS_AP_SUPPORT
EXTRA_CFLAGS += -DCLIENT_WDS
ap_objs += $(SRC_EMBEDDED_DIR)/common/client_wds.o
endif
endif


ifeq ($(CONFIG_WDS_SUPPORT),y)
    EXTRA_CFLAGS += -DWDS_SUPPORT

    ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_wds.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_wds_inf.o\
            $(SRC_EMBEDDED_DIR)/common/client_wds.o
endif

ifeq ($(CONFIG_APCLI_SUPPORT),y)
    EXTRA_CFLAGS += -DAPCLI_SUPPORT -DMAT_SUPPORT -DMULTI_APCLI_SUPPORT -DAPCLI_AUTO_CONNECT_SUPPORT  -DAPCLI_AUTO_BW_TMP
    #EXTRA_CFLAGS += -DAPCLI_CERT_SUPPORT -DAPCLI_CONNECTION_TRIAL
    EXTRA_CFLAGS += -DDHCP_UC_SUPPORT
    EXTRA_CFLAGS += -DFAST_EAPOL_WAR
    EXTRA_CFLAGS += -DBT_APCLI_SUPPORT
    ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_apcli.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_apcli_inf.o\
            $(SRC_EMBEDDED_DIR)/ap/apcli_assoc.o\
            $(SRC_EMBEDDED_DIR)/ap/apcli_auth.o\
            $(SRC_EMBEDDED_DIR)/ap/apcli_ctrl.o\
            $(SRC_EMBEDDED_DIR)/ap/apcli_sync.o\
            $(SRC_EMBEDDED_DIR)/common/cmm_mat.o\
            $(SRC_EMBEDDED_DIR)/common/cmm_mat_iparp.o\
            $(SRC_EMBEDDED_DIR)/common/cmm_mat_pppoe.o\
            $(SRC_EMBEDDED_DIR)/common/cmm_mat_ipv6.o

    ifeq ($(CONFIG_APCLI_CERT_SUPPORT),y)
        EXTRA_CFLAGS += -DAPCLI_CERT_SUPPORT
    endif

    ifeq ($(CONFIG_MAC_REPEATER_SUPPORT),y)
        EXTRA_CFLAGS += -DMAC_REPEATER_SUPPORT

        ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_repeater.o
    endif

endif

ifeq ($(CONFIG_IDS_SUPPORT),y)
    EXTRA_CFLAGS += -DIDS_SUPPORT

    ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_ids.o
endif

ifeq ($(CONFIG_NINTENDO_AP),y)
    EXTRA_CFLAGS += -DNINTENDO_AP

    ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_nintendo.o
endif

#ifeq ($(CONFIG_COC_SUPPORT),y)
#   EXTRA_CFLAGS += -DGREENAP_SUPPORT -DCOC_SUPPORT
#
#    ap_objs += $(SRC_DIR)/../hw_ctrl/greenap.o
#endif

ifeq ($(CONFIG_GREENAP_SUPPORT),y)
   EXTRA_CFLAGS += -DGREENAP_SUPPORT

    ap_objs += $(SRC_DIR)/hw_ctrl/greenap.o
endif

ifeq ($(CONFIG_PCIE_ASPM_DYM_CTRL_SUPPORT),y)
   EXTRA_CFLAGS += -DPCIE_ASPM_DYM_CTRL_SUPPORT
endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_ATE -DCONFIG_QA -DCONFIG_RT2880_ATE_CMD_NEW -DCONFIG_HW_HAL_OFFLOAD  #-DCSD_VERIFICATION -DINTERNAL_CAPTURE_SUPPORT
endif

ifeq ($(CONFIG_ICAP_SUPPORT),y)
		EXTRA_CFLAGS += -DINTERNAL_CAPTURE_SUPPORT
endif

ifeq ($(CONFIG_SPECTRUM_SUPPORT),y)
		EXTRA_CFLAGS += -DWIFI_SPECTRUM_SUPPORT
endif

ifeq ($(CONFIG_RTMP_FLASH_SUPPORT),y)
    EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT
endif

ifeq ($(CONFIG_PRE_CAL_TRX_SET1_SUPPORT),y)
    EXTRA_CFLAGS += -DPRE_CAL_TRX_SET1_SUPPORT
endif

ifeq ($(CONFIG_RLM_CAL_CACHE_SUPPORT),y)
    EXTRA_CFLAGS += -DRLM_CAL_CACHE_SUPPORT
endif

ifeq ($(CONFIG_PRE_CAL_TRX_SET2_SUPPORT),y)
    EXTRA_CFLAGS += -DPRE_CAL_TRX_SET2_SUPPORT
endif

ifeq ($(CONFIG_CAL_BIN_FILE_SUPPORT),y)
    EXTRA_CFLAGS += -DCAL_BIN_FILE_SUPPORT
endif

ifeq ($(CONFIG_RF_LOCKDOWN_SUPPORT),y)
    EXTRA_CFLAGS += -DRF_LOCKDOWN
endif

ifeq ($(CONFIG_LINK_TEST_SUPPORT),y)
    EXTRA_CFLAGS += -DLINK_TEST_SUPPORT
endif

ifeq ($(CONFIG_RTMP_TEMPERATURE_TX_ALC),y)
    EXTRA_CFLAGS += -DRTMP_TEMPERATURE_TX_ALC
endif

ifeq ($(CONFIG_TXBF_SUPPORT),y)
    EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DTXBF_SUPPORT -DVHT_TXBF_SUPPORT
    ap_objs += $(SRC_DIR)/txbf/cmm_txbf_mt.o\
                    $(SRC_DIR)/txbf/cmm_txbf_cal_mt.o\
                    $(SRC_DIR)/txbf/txbf_wrapper_embedded.o
endif

ifeq ($(CONFIG_MUMIMO_SUPPORT),y)
   EXTRA_CFLAGS += -DCFG_SUPPORT_MU_MIMO
    ap_objs += $(SRC_EMBEDDED_DIR)/common/cmm_mumimo.o
    ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_mumimo.o
endif

ifeq ($(CONFIG_MU_RA_SUPPORT),y)
   EXTRA_CFLAGS += -DCFG_SUPPORT_MU_MIMO_RA
   ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_mura.o
endif

ifeq ($(CONFIG_CON_WPS_SUPPORT), y)
    EXTRA_CFLAGS += -DCON_WPS -DCON_WPS_AP_SAME_UUID
endif

ifeq ($(CONFIG_MCAST_RATE_SPECIFIC),y)
    EXTRA_CFLAGS += -DMCAST_RATE_SPECIFIC
endif

ifeq ($(CONFIG_AIRPLAY_SUPPORT),y)
    EXTRA_CFLAGS += -DAIRPLAY_SUPPORT
endif

ifeq ($(CONFIG_BAND_STEERING),y)
    EXTRA_CFLAGS += -DBAND_STEERING
    ap_objs +=  $(SRC_EMBEDDED_DIR)/ap/ap_band_steering.o
endif

ifeq ($(CONFIG_AIR_MONITOR),y)
    EXTRA_CFLAGS += -DAIR_MONITOR
endif

ifeq ($(CONFIG_EASY_SETUP_SUPPORT),y)
$(DRV_NAME)-objs += $(SRC_EMBEDDED_DIR)/easy_setup/ez_cmm.o
endif

ifeq ($(CONFIG_MWDS),y)
    ap_objs += $(SRC_EMBEDDED_DIR)/common/mwds.o
    ap_objs += $(SRC_EMBEDDED_DIR)/common/a4_conn.o
    ap_objs += $(SRC_EMBEDDED_DIR)/common/routing_tab.o
endif

ifeq ($(CONFIG_EVENT_NOTIFIER_SUPPORT),y)
    ap_objs += $(SRC_EMBEDDED_DIR)/common/event_notifier.o
endif

ifeq ($(CONFIG_VOW_SUPPORT),y)
    EXTRA_CFLAGS += -DVOW_SUPPORT
endif

ifeq ($(CONFIG_RED_SUPPORT),y)
    EXTRA_CFLAGS += -DRED_SUPPORT
endif

ifeq ($(CONFIG_FQ_SCH_SUPPORT),y)
    EXTRA_CFLAGS += -DFQ_SCH_SUPPORT
endif

#EXTRA_CFLAGS += -DCHANNEL_SWITCH_MONITOR_CONFIG
#ccflags-y += -DACS_CTCC_SUPPORT
########################################################
# Linux system related files
########################################################
os_objs := $(SRC_DIR)/os/linux/rt_proc.o\
            $(SRC_DIR)/os/linux/rt_linux.o\
            $(SRC_DIR)/os/linux/rt_profile.o\
            $(SRC_DIR)/os/linux/rt_main_dev.o

ifeq ($(CONFIG_WLAN_HOOK),y)
	EXTRA_CFLAGS +=-DRTMP_WLAN_HOOK_SUPPORT
	os_objs += $(SRC_DIR)/os/linux/rt_txrx_hook.o
endif

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

ifeq ($(CONFIG_LINUX_NET_TXQ_SUPPORT),y)
EXTRA_CFLAGS += -DLINUX_NET_TXQ_SUPPORT
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
		$(SRC_DIR)/chips/mt7628_dbg.o\
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
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTAR_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
# -DCUT_THROUGH
#EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
#EXTRA_CFLAGS += -DDBDC_MODE
#EXTRA_CFLAGS += -DNEED_ROM_PATCH
#EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_smac.o
endif

chip_objs += $(SRC_DIR)/chips/mt7637.o\
		$(SRC_DIR)/chips/mt7637_dbg.o\
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

ifeq ($(CONFIG_DBDC_MODE),y)
EXTRA_CFLAGS += -DDBDC_MODE
endif

ifeq ($(CONFIG_CHIP_MT7615E),y)
EXTRA_CFLAGS += -DMT7615 -DMT_BBP -DMT_RF -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DERR_RECOVERY
#EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
#EXTRA_CFLAGS += -DDBDC_MODE
EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DTXPWRMANUAL
EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DNEWSEC
EXTRA_CFLAGS += -DSINGLE_SKU_V2
EXTRA_CFLAGS += -DTX_POWER_CONTROL_SUPPORT
EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC
EXTRA_CFLAGS += -DCONFIG_HW_HAL_OFFLOAD
EXTRA_CFLAGS += -DRX_SCATTER
EXTRA_CFLAGS += -DPKT_BUDGET_CTRL_SUPPORT
EXTRA_CFLAGS += -DRATE_ADAPTION
EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DCONFIG_TX_DELAY
EXTRA_CFLAGS += -DRELEASE_EXCLUDE
EXTRA_CFLAGS += -DPRE_CAL_TRX_SET1_SUPPORT
EXTRA_CFLAGS += -DRLM_CAL_CACHE_SUPPORT
EXTRA_CFLAGS += -DPRE_CAL_TRX_SET2_SUPPORT
ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
CONFIG_PROPRIETARY_DRIVER=y
ifeq ($(CONFIG_PROPRIETARY_DRIVER),y)
EXTRA_CFLAGS += -DCONFIG_PROPRIETARY_DRIVER
CONFIG_PLATFORM_DRV_SUPPORT=n
else
CONFIG_PLATFORM_DRV_SUPPORT=y
endif
endif

ifeq ($(CONFIG_CUSTOMER_VENDOR_IE_SUPPORT),y)
    EXTRA_CFLAGS += -DCUSTOMER_VENDOR_IE_SUPPORT
endif
ifeq ($(CONFIG_G_BAND_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DG_BAND_256QAM
ifeq ($(CONFIG_BRCM_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DBRCM_256QAM
endif
endif
EXTRA_CFLAGS += -Wframe-larger-than=4096
ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif
ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
#	EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_TCP_RACK_SUPPORT),y)
	EXTRA_CFLAGS += -DREDUCE_TCP_ACK_SUPPORT
endif

ifeq ($(CONFIG_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
#    ifeq ($(CONFIG_WSC_INCLUDED),y)
#        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
#    endif

    func_objs += $(SRC_EMBEDDED_DIR)/common/rt_led.o
endif

ifeq ($(CONFIG_RADIUS_ACCOUNTING_SUPPORT),y)
	EXTRA_CFLAGS += -DRADIUS_ACCOUNTING_SUPPORT
endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_dmac.o
endif

ifeq ($(CONFIG_FDB_SUPPORT),y)
    EXTRA_CFLAGS += -DMT_FDB
    chip_objs += $(SRC_DIR)/mcu/mt_fdb.o
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
                $(SRC_DIR)/phy/rlm_cal_cache.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
		$(SRC_EMBEDDED_DIR)/common/cut_through.o\
		$(SRC_DIR)/protocol/tmr.o\
		$(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
                $(SRC_DIR)/os/linux/pci_main_dev.o
endif

ifeq ($(CONFIG_CHIP_MT7663E),y)
EXTRA_CFLAGS += -DMT7663 -DMT_BBP -DMT_RF -DRTMP_PCI_SUPPORT -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI
EXTRA_CFLAGS += -DMT_DMAC
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
#EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
#EXTRA_CFLAGS += -DERR_RECOVERY
#EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
#EXTRA_CFLAGS += -DDBDC_MODE
EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DNEWSEC
#EXTRA_CFLAGS += -DSINGLE_SKU_V2
#EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC
EXTRA_CFLAGS += -DCONFIG_HW_HAL_OFFLOAD
WFLAGS += -DCONFIG_FPGA_MODE
#EXTRA_CFLAGS += -DRX_SCATTER
EXTRA_CFLAGS += -DPKT_BUDGET_CTRL_SUPPORT
EXTRA_CFLAGS += -DHW_TX_AMSDU_SUPPORT
#WFLAGS += -DCONFIG_CSO_SUPPORT
#WFLAGS += -DMSP_UTILITY
ifeq ($(CONFIG_G_BAND_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DG_BAND_256QAM
ifeq ($(CONFIG_BRCM_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DBRCM_256QAM
endif
endif

#EXTRA_CFLAGS += -Wframe-larger-than=4096
WFLAGS += -Werror

ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif
ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
	EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
	EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_TCP_RACK_SUPPORT),y)
	EXTRA_CFLAGS += -DREDUCE_TCP_ACK_SUPPORT
endif

ifeq ($(CONFIG_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
#    ifeq ($(CONFIG_WSC_INCLUDED),y)
#        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
#    endif

    func_objs += $(SRC_EMBEDDED_DIR)/common/rt_led.o
endif

ifeq ($(CONFIG_RADIUS_ACCOUNTING_SUPPORT),y)
	EXTRA_CFLAGS += -DRADIUS_ACCOUNTING_SUPPORT
endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_dmac.o
endif

ifeq ($(CONFIG_FDB_SUPPORT),y)
    EXTRA_CFLAGS += -DMT_FDB
    chip_objs += $(SRC_DIR)/mcu/mt_fdb.o
endif

chip_objs += $(SRC_DIR)/chips/mt7663.o\
		$(SRC_DIR)/chips/mt7663_dbg.o\
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
                $(SRC_DIR)/phy/rlm_cal_cache.o\
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
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
#EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
EXTRA_CFLAGS += -DUNIFY_FW_CMD
EXTRA_CFLAGS += -DTXPWRMANUAL
#EXTRA_CFLAGS += -DTMR_VERIFY
EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DETSI_RX_BLOCKER_SUPPORT
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DNEWSEC
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC
EXTRA_CFLAGS += -DCONFIG_HW_HAL_OFFLOAD
EXTRA_CFLAGS += -DRX_SCATTER
#EXTRA_CFLAGS += -DCONFIG_FPGA_MODE
#EXTRA_CFLAGS += -DCONFIG_CSO_SUPPORT
EXTRA_CFLAGS += -DPKT_BUDGET_CTRL_SUPPORT
#EXTRA_CFLAGS += -DMSP_UTILITY
EXTRA_CFLAGS += -DRATE_ADAPTION
EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DCONFIG_TX_DELAY
EXTRA_CFLAGS += -DDBG_AMSDU
EXTRA_CFLAGS += -DPRE_CAL_MT7622_SUPPORT
EXTRA_CFLAGS += -DTXSTAT_2040BW_24G_SUPPORT
CONFIG_PROPRIETARY_DRIVER=y

ifeq ($(CONFIG_PROPRIETARY_DRIVER),y)
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
ifeq ($(CONFIG_VHT_TXBF_2G_EPIGRAM_IE_SUPPORT),y)
        EXTRA_CFLAGS += -DVHT_TXBF_2G_EPIGRAM_IE
endif

#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
#        EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_dmac.o
endif

ifeq ($(CONFIG_VOW_SUPPORT),y)
ifeq ($(CONFIG_RED_SUPPORT),y)
EXTRA_CFLAGS += -DFQ_SCH_SUPPORT
endif
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
                $(SRC_DIR)/phy/rlm_cal_cache.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
                $(SRC_EMBEDDED_DIR)/common/cut_through.o\
                $(SRC_DIR)/protocol/tmr.o\
                $(SRC_DIR)/os/linux/rt_rbus_pci_drv.o
endif

ifeq ($(CONFIG_RTMP_FLASH_SUPPORT)$(CONFIG_MTD),yy)
ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
ifeq ($(CONFIG_MTD_SPI_NOR),y)
EXTRA_CFLAGS += -DCONFIG_WIFI_MTD
else
ifeq ($(CONFIG_MTD_NAND),y)
EXTRA_CFLAGS += -DCONFIG_WIFI_MTD
endif
endif
else
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

$(DRV_NAME)-objs += $(ap_objs) $(cmm_objs) $(asic_objs) $(chip_objs) $(rate_objs)\
                    $(spec_objs) $(func_objs) $(os_objs) $(dot11_ft_objs)

$(DRV_NAME)-objs += $(SRC_EMBEDDED_DIR)/common/eeprom.o\
					$(SRC_EMBEDDED_DIR)/common/ee_flash.o\
					$(SRC_EMBEDDED_DIR)/common/ee_efuse.o

$(DRV_NAME)-objs += $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o
$(DRV_NAME)-objs += $(SRC_EMBEDDED_DIR)/common/hif_pci.o

$(DRV_NAME)-objs += $(SRC_DIR)/os/linux/rt_pci_rbus.o\
                    $(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
                    $(SRC_DIR)/os/linux/rt_rbus_pci_util.o\
                    #$(SRC_DIR)/os/linux/rbus_main_dev.o

ifeq ($(CONFIG_ATE_SUPPORT),y)
$(DRV_NAME)-objs += $(SRC_DIR)/ate/ate_agent.o\
                    $(SRC_DIR)/ate/testmode_ioctl.o\
		    $(SRC_DIR)/ate/mt_mac/mt_testmode.o\
 		    $(SRC_DIR)/ate/LoopBack.o
endif


ifeq ($(CONFIG_TCP_RACK_SUPPORT),y)
$(DRV_NAME)-objs += $(SRC_EMBEDDED_DIR)/common/cmm_tcprack.o
endif

ifeq ($(CONFIG_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
#    ifeq ($(CONFIG_WSC_INCLUDED),y)
#        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
#    endif

    func_objs += $(SRC_EMBEDDED_DIR)/common/rt_led.o
endif

###################
#  CFLAGS
##################
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT  -DLINUX \
              -Wstrict-prototypes -Wno-trigraphs -Wall -Wno-unused-but-set-variable

EXTRA_CFLAGS += -DOLDSEC

#-Werror
#-DDBG_DIAGNOSE -DDBG_RX_MCS -DDBG_TX_MCS

EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT -DSCAN_SUPPORT -DAP_SCAN_SUPPORT
EXTRA_CFLAGS += -DDOT11_N_SUPPORT -DDOT11N_DRAFT3 -DSTATS_COUNT_SUPPORT -DIAPP_SUPPORT -DDOT1X_SUPPORT
#EXTRA_CFLAGS += -DRALINK_ATE -DRALINK_QA -DCONFIG_RT2880_ATE_CMD_NEW

ifeq ($(CONFIG_FAST_NAT_SUPPORT),y)
EXTRA_CFLAGS += -DCONFIG_FAST_NAT_SUPPORT
endif

#provide busy time statistics for every TBTT */
#EXTRA_CFLAGS += -DQLOAD_FUNC_BUSY_TIME_STATS

# provide busy time alarm mechanism
# use the function to avoid to locate in some noise environments
#EXTRA_CFLAGS += -DQLOAD_FUNC_BUSY_TIME_ALARM

ifeq ($(CONFIG_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS += -DMEMORY_OPTIMIZATION
else
#EXTRA_CFLAGS += -DDBG
endif
EXTRA_CFLAGS += -DDBG

ifeq ($(CONFIG_RTMP_INTERNAL_TX_ALC),y)
EXTRA_CFLAGS += -DRTMP_INTERNAL_TX_ALC
endif

ifeq ($(CONFIG_RALINK_MT7621),y)
EXTRA_CFLAGS += -DRACTRL_LIMIT_MAX_PHY_RATE=1300
endif

ifeq ($(CONFIG_FAST_NAT_SUPPORT),y)
ifeq ($(CONFIG_WLAN_HOOK),y)
ifeq ($(CONFIG_WHNAT_SUPPORT),y)
EXTRA_CFLAGS += -DWHNAT_SUPPORT
else
ifeq ($(CONFIG_WHNAT_SUPPORT),m)
EXTRA_CFLAGS += -DWHNAT_SUPPORT
endif
endif
endif
endif

ifeq ($(CONFIG_EASY_SETUP_SUPPORT),y)
EXTRA_CFLAGS += -DWH_EZ_SETUP
EXTRA_CFLAGS += -DNEW_CONNECTION_ALGO
EXTRA_CFLAGS += -DEZ_NETWORK_MERGE_SUPPORT
EXTRA_CFLAGS += -DEZ_DUAL_BAND_SUPPORT
EXTRA_CFLAGS += -DEZ_ROAM_SUPPORT
EXTRA_CFLAGS += -DDUAL_CHIP
EXTRA_CFLAGS += -DEZ_API_SUPPORT
EXTRA_CFLAGS += -DEZ_PUSH_BW_SUPPORT
endif

ifeq ($(CONFIG_MWDS),y)
EXTRA_CFLAGS += -DMWDS -DA4_CONN -DROUTING_TAB_SUPPORT
endif

ifeq ($(CONFIG_EVENT_NOTIFIER_SUPPORT),y)
EXTRA_CFLAGS += -DWH_EVENT_NOTIFIER
endif

#ifeq ($(CONFIG_WIFI_FWD_SUPPORT),y)
EXTRA_CFLAGS += -DCONFIG_WIFI_PKT_FWD
#endif

#ifeq ($(CONFIG_MTFWD_SUPPORT),y)
EXTRA_CFLAGS += -DMTFWD
#endif

ifeq ($(CONFIG_ROAMING_ENHANCE_SUPPORT),y)
EXTRA_CFLAGS += -DROAMING_ENHANCE_SUPPORT
endif

EXTRA_CFLAGS += -DIP_ASSEMBLY
EXTRA_CFLAGS += -DOUI_CHECK_SUPPORT
EXTRA_CFLAGS += -DHTC_DECRYPT_IOT

EXTRA_CFLAGS += -Inet/nat
#EXTRA_CFLAGS += -DDBG_STARVATION
ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
EXTRA_CFLAGS += -DMT_WIFI_MODULE
else
ifeq ($(CONFIG_MT_WIFI),m)
EXTRA_CFLAGS += -DMT_WIFI_MODULE
endif
endif

MODULE_FLAGS=$(EXTRA_CFLAGS)
export MODULE_FLAGS
obj-m+=$(SRC_EMBEDDED_DIR)/plug_in/
