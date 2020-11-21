RT_DRV_DIR  := mt76x2
RT_DRV_PATH := ../$(RT_DRV_DIR)
RT_DRV_NAME := mt76x2_ap




obj-m += $(RT_DRV_NAME).o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/crypt_md5.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/crypt_sha2.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/crypt_hmac.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/crypt_aes.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/crypt_arc4.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/mlme.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/action.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/ba_action.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_wep.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_wpa.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_tkip.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_aes.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_data.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_data_pci.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_mac_pci.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_sync.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_sanity.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_info.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_cfg.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_profile.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_asic.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_cmd.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_radar.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rtmp_init.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rtmp_init_inf.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rtmp_timer.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rt_channel.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rt_os_util.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/spectrum.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/txpower.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/scan.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/ps.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/uapsd.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/eeprom.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/ee_flash.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/ee_efuse.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/ee_prom.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/tx_rx/wdev_tx.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/mgmt/mgmt_entrytb.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/mgmt/mgmt_ht.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/mac/rtmp_mac.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/mcu/mcu.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/phy/rf.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/phy/rt_rf.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/phy/phy.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/chips/rtmp_chip.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/rate_ctrl/ra_ctrl.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/rate_ctrl/alg_legacy.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/rate_ctrl/alg_grp.o
#$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/rate_ctrl/alg_ags.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_assoc.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_auth.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_connect.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_mlme.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_sanity.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_sync.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_wpa.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_data.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_autoChSel.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_qload.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_cfg.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/ap_ioctl.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rt_proc.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rt_linux.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rt_profile.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rt_main_dev.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rt_rbus_pci_drv.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rt_rbus_pci_util.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rt_pci_rbus.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/multi_main_dev.o

### MT7620 (RBUS)
ifeq ($(CHIP_MT7620),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/chips/rt6352.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/chips/rtxx_dft.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rtmp_swmcu.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_rf_cal.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_loft_cal.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/phy/rlt_rf.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/phy/rtmp_phy.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/mac/ral_omac.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/rbus_main_dev.o
ifeq ($(CONFIG_RT_ATE),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ate/chips/rt6352_ate.o
endif
endif

### MT7602E/MT7612E (PCIe)

$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/chips/rt65xx.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/chips/mt76x2.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/mcu/mcu_and.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/phy/mt_rf.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/phy/rlt_phy.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/mac/ral_nmac.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/pci_main_dev.o
ifeq ($(CONFIG_RT_ATE),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ate/chips/mt76x2_ate.o
endif


ifeq ($(CONFIG_RT_ATE),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ate/common/rt_ate.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ate/common/ate_pci.o
ifeq ($(CONFIG_RT_QA),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ate/common/rt_qa.o
endif
endif

ifeq ($(CONFIG_AP_LED),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rt_led.o
endif

ifeq ($(CONFIG_AP_WDS),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_wds.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_wds_inf.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/client_wds.o
endif

ifeq ($(CONFIG_AP_MBSS),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_mbss.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_mbss_inf.o
endif

ifeq ($(CONFIG_AP_APCLI),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_apcli.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_apcli_inf.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/apcli_assoc.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/apcli_auth.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/apcli_ctrl.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/apcli_sync.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_mat.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_mat_iparp.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_mat_pppoe.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_mat_ipv6.o
ifeq ($(CONFIG_AP_APCLI_AUTO_BW),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/apcli_autobw.o
endif
ifeq ($(CONFIG_AP_MAC_REPEATER),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_repeater.o
endif
endif

ifeq ($(CONFIG_AP_MWDS),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/mwds.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/routing_tab.o
ifeq ($(CONFIG_AP_SMART_MESH),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/smart_mesh.o
endif
endif

ifeq ($(CONFIG_AP_80211AC_VHT),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/vht.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/mgmt/mgmt_vht.o
endif

ifeq ($(CONFIG_AP_GREENAP),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/hw_ctrl/greenap.o
endif

ifeq ($(CONFIG_AP_DLS),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_dls.o
endif

ifeq ($(CONFIG_AP_IDS),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_ids.o
endif

ifeq ($(CONFIG_AP_DFS),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_dfs.o
endif

ifeq ($(CONFIG_AP_CARRIER),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_cs.o
endif



ifeq ($(CONFIG_RT_WSC),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/wsc.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/wsc_tlv.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/crypt_dh.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/wsc_ufd.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/crypt_biginteger.o
ifeq ($(CONFIG_RT_WSC_V2),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/wsc_v2.o
endif
ifeq ($(CONFIG_RT_WSC_NFC),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/nfc.o
endif
endif

ifeq ($(CONFIG_RT_WAPI),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/wapi.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/wapi_sms4.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/wapi_crypt.o
endif

ifeq ($(CONFIG_RT_80211W_PMF),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/pmf.o
endif

ifeq ($(CONFIG_RT_IGMP_SNOOP),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/igmp_snoop.o
endif

ifeq ($(CONFIG_RT_NETIF_BLOCK),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/netif_block.o
endif

ifeq ($(CONFIG_RT_CSO),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/naf/net_acc.o
endif

ifeq ($(CONFIG_RT_VIDEO_TURBINE),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/cmm_video.o
endif

ifeq ($(CONFIG_RT_BAND_STEERING),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_band_steering.o
endif

ifeq ($(CONFIG_RT_DOT11R_FT),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/ft.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/ft_tlv.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/ft_iocl.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/ft_rc.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/ap/ap_ftkd.o
endif

ifeq ($(CONFIG_RT_DOT11K_RRM),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rrm_tlv.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rrm_sanity.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/rrm.o
endif

ifeq ($(CONFIG_RT_CFG80211),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/cfg80211/cfg80211_scan.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/cfg80211/cfg80211_rx.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/cfg80211/cfg80211_tx.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/cfg80211/cfg80211_inf.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/cfg80211/cfg80211_p2p.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/cfg80211/cfg80211_ap.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/cfg80211/cfg80211drv.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/cfg80211/cfg80211_util.o
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/os/linux/cfg80211/cfg80211.o
endif

ifeq ($(CONFIG_RT_DEBUG),y)
$(RT_DRV_NAME)-objs += $(RT_DRV_PATH)/common/sys_log.o
endif

###################
#  CFLAGS
##################

EXTRA_CFLAGS  = -Wall -Wstrict-prototypes -Wno-trigraphs -Wno-unused -Wno-declaration-after-statement
EXTRA_CFLAGS += -I$(src)/../mt76x2/include
ifeq ($(CONFIG_RT_ATE),y)
EXTRA_CFLAGS += -I$(src)/../mt76x2/ate/include 
endif
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT -DLINUX
EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT -DUAPSD_SUPPORT -DDOT11_N_SUPPORT -DDOT1X_SUPPORT
EXTRA_CFLAGS += -DAP_SCAN_SUPPORT -DSCAN_SUPPORT
#EXTRA_CFLAGS += -DAP_PARTIAL_SCAN_SUPPORT
EXTRA_CFLAGS += -DSTATS_COUNT_SUPPORT -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DRSSI_FEEDBACK
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC
EXTRA_CFLAGS += -DEAPOL_QUEUE_SUPPORT
EXTRA_CFLAGS += -DDATA_QUEUE_RESERVE
EXTRA_CFLAGS += -DFORCE_ANNOUNCE_CRITICAL_AMPDU
#EXTRA_CFLAGS += -DPS_ENTRY_MAITENANCE
#EXTRA_CFLAGS += -DSYSTEM_LOG_SUPPORT

ifeq ($(CONFIG_RT_FIRST_CARD),2860)
EXTRA_CFLAGS += -DRT_SOC_SUPPORT
endif

# do not lock CountryRegion from EEPROM
EXTRA_CFLAGS += -DEEPROM_COUNTRY_UNLOCK

# provide internal eFuse memory support
#EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT -DCAL_FREE_IC_SUPPORT

# provide information on the current STA population and traffic levels in the QBSS
#EXTRA_CFLAGS += -DAP_QLOAD_SUPPORT -DQLOAD_FUNC_BUSY_TIME_STATS

ifneq ($(CONFIG_RA_HW_NAT_WIFI),y)
EXTRA_CFLAGS += -DCONFIG_RA_NAT_NONE
endif

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS += -DWORKQUEUE_BH
endif

### MT7620 (RBUS)
ifeq ($(CHIP_MT7620),y)
EXTRA_CFLAGS += -DRT6352 -DCONFIG_SWMCU_SUPPORT
EXTRA_CFLAGS += -DRTMP_RBUS_SUPPORT -DRTMP_MAC_PCI
EXTRA_CFLAGS += -DRTMP_MAC -DRTMP_BBP -DRLT_RF -DRTMP_RF_RW_SUPPORT
EXTRA_CFLAGS += -DVCORECAL_SUPPORT
EXTRA_CFLAGS += -DLOFT_IQ_CAL_SUPPORT
EXTRA_CFLAGS += -DRTMP_TEMPERATURE_CALIBRATION
EXTRA_CFLAGS += -DFIFO_EXT_SUPPORT
EXTRA_CFLAGS += -DDYNAMIC_VGA_SUPPORT
#EXTRA_CFLAGS += -DMICROWAVE_OVEN_SUPPORT
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DPEER_DELBA_TX_ADAPT
EXTRA_CFLAGS += -DDROP_MASK_SUPPORT
EXTRA_CFLAGS += -DMULTI_CLIENT_SUPPORT
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT
#EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
ifeq ($(CONFIG_AP_RTMP_INTERNAL_TX_ALC),y)
EXTRA_CFLAGS += -DRTMP_INTERNAL_TX_ALC
endif
ifeq ($(CONFIG_AP_RTMP_TEMPERATURE_COMPENSATION),y)
EXTRA_CFLAGS += -DRTMP_TEMPERATURE_COMPENSATION
endif
ifeq ($(CONFIG_AP_ADJ_PWR_CONSUMPTION),y)
EXTRA_CFLAGS += -DADJUST_POWER_CONSUMPTION_SUPPORT
endif
ifeq ($(CONFIG_AP_INTERNAL_PA_EXTERNAL_LNA),y)
EXTRA_CFLAGS += -DRT6352_EL_SUPPORT
endif
ifeq ($(CONFIG_AP_EXTERNAL_PA_EXTERNAL_LNA),y)
EXTRA_CFLAGS += -DRT6352_EP_SUPPORT -DRT6352_EL_SUPPORT
endif
endif

### MT7602E/MT7612E (PCIe)
EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT
EXTRA_CFLAGS += -DREAD_MAC_FROM_EEPROM
EXTRA_CFLAGS +=-DCONFIG_RT_FIRST_IF_RF_OFFSET=0x0000
EXTRA_CFLAGS +=-DCONFIG_RT_FIRST_CARD=7603
EXTRA_CFLAGS +=-DCONFIG_RT_FIRST_CARD_EEPROM="flash"
EXTRA_CFLAGS +=-DCONFIG_RT_SECOND_IF_RF_OFFSET=0x8000
EXTRA_CFLAGS +=-DCONFIG_RT_SECOND_CARD_EEPROM="flash"
EXTRA_CFLAGS +=-DCONFIG_RT_SECOND_CARD=7612
EXTRA_CFLAGS += -DRT65xx -DMT76x2 -DMT7612 
#EXTRA_CFLAGS += -DCONFIG_FPGA_MODE
EXTRA_CFLAGS += -DRTMP_PCI_SUPPORT -DRTMP_MAC_PCI
EXTRA_CFLAGS += -DRLT_MAC -DRLT_BBP -DMT_RF -DRTMP_RF_RW_SUPPORT
EXTRA_CFLAGS += -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DRX_DMA_SCATTER -DCONFIG_ANDES_SUPPORT
EXTRA_CFLAGS += -DNEW_RATE_ADAPT_SUPPORT
#EXTRA_CFLAGS += -DNEW_RATE_ADAPT_QUICK_DOWN # has dropdown to CCK/0 issue on 5GHz
EXTRA_CFLAGS += -DFIFO_EXT_SUPPORT
EXTRA_CFLAGS += -DDYNAMIC_VGA_SUPPORT
#EXTRA_CFLAGS += -DMICROWAVE_OVEN_SUPPORT
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DPEER_DELBA_TX_ADAPT
EXTRA_CFLAGS += -DDROP_MASK_SUPPORT
EXTRA_CFLAGS += -DMULTI_CLIENT_SUPPORT
EXTRA_CFLAGS += -DRTMP_TEMPERATURE_TX_ALC
#EXTRA_CFLAGS += -DSPECIFIC_BCN_BUF_SUPPORT
#EXTRA_CFLAGS += -DSPECIFIC_TX_POWER_SUPPORT
#EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
#EXTRA_CFLAGS += -DDMA_BUSY_RESET
EXTRA_CFLAGS += -DDBG -DUAPSD_DEBUG
#EXTRA_CFLAGS += -DREAD_MAC_FROM_EEPROM
#EXTRA_CFLAGS += -DDBG_DIAGNOSE -DDBG_RX_MCS -DDBG_TX_MCS


ifeq ($(CONFIG_RT_ATE),y)
EXTRA_CFLAGS += -DRALINK_ATE -DCONFIG_RT2880_ATE_CMD_NEW
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT -DCAL_FREE_IC_SUPPORT
ifeq ($(CONFIG_RT_QA),y)
EXTRA_CFLAGS += -DRALINK_QA
endif
endif

ifeq ($(CONFIG_AP_DLS),y)
EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
endif

ifeq ($(CONFIG_AP_IDS),y)
EXTRA_CFLAGS += -DIDS_SUPPORT
endif

ifeq ($(CONFIG_AP_DFS),y)
EXTRA_CFLAGS += -DDFS_SUPPORT -DDFS_ATP_SUPPORT
endif

ifeq ($(CONFIG_AP_CARRIER),y)
EXTRA_CFLAGS += -DCARRIER_DETECTION_SUPPORT
endif

ifeq ($(CONFIG_AP_GREENAP),y)
EXTRA_CFLAGS += -DGREENAP_SUPPORT
endif

ifeq ($(CONFIG_AP_80211AC_VHT),y)
EXTRA_CFLAGS += -DDOT11_VHT_AC
endif



ifeq ($(CONFIG_AP_LED),y)
EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
ifeq ($(CONFIG_RT_WSC),y)
EXTRA_CFLAGS += -DWSC_LED_SUPPORT
endif
endif

ifeq ($(CONFIG_AP_LED_SOFT),y)
EXTRA_CFLAGS += -DLED_SOFT_SUPPORT
EXTRA_CFLAGS += -DLED_SOFT_BLINK_GPIO=$(CONFIG_AP_LED_SOFT_GPIO)
ifdef CONFIG_MT76X2_AP_LED_SOFT_GPIO_DEV1
EXTRA_CFLAGS += -DLED_SOFT_BLINK_GPIO_DEV1=$(CONFIG_AP_LED_SOFT_GPIO_DEV1)
else
EXTRA_CFLAGS += -DLED_SOFT_BLINK_GPIO_DEV1=-1
endif
endif

ifeq ($(CONFIG_AP_LLTD),y)
EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

ifeq ($(CONFIG_AP_WDS),y)
EXTRA_CFLAGS += -DWDS_SUPPORT
endif

ifeq ($(CONFIG_AP_MBSS),y)
EXTRA_CFLAGS += -DMBSS_SUPPORT
ifeq ($(CONFIG_AP_MBSS_NEW_MBSSID_MODE),y)
EXTRA_CFLAGS += -DNEW_MBSSID_MODE
EXTRA_CFLAGS += -DENHANCE_NEW_MBSSID_MODE
endif
endif

ifeq ($(CONFIG_AP_APCLI),y)
EXTRA_CFLAGS += -DAPCLI_SUPPORT -DMAT_SUPPORT -DTRAFFIC_BASED_TXOP
EXTRA_CFLAGS += -DAPCLI_AUTO_CONNECT_SUPPORT
EXTRA_CFLAGS += -DMAC_APCLI_SUPPORT
ifeq ($(CONFIG_AP_APCLI_AUTO_BW),y)
EXTRA_CFLAGS += -DAPCLI_AUTO_BW_SUPPORT
endif
ifeq ($(CONFIG_AP_MAC_REPEATER),y)
EXTRA_CFLAGS += -DMAC_REPEATER_SUPPORT
endif
ifeq ($(CONFIG_AP_APCLI_CERT),y)
EXTRA_CFLAGS += -DAPCLI_CERT_SUPPORT
endif
ifeq ($(CONFIG_AP_CON_WPS),y)
EXTRA_CFLAGS += -DCON_WPS -DCON_WPS_AP_SAME_UUID
endif
endif

ifeq ($(CONFIG_AP_AIRPLAY),y)
EXTRA_CFLAGS += -DAIRPLAY_SUPPORT
endif

ifeq ($(CONFIG_AP_MWDS),y)
EXTRA_CFLAGS += -DMWDS -DROUTING_TAB_SUPPORT
ifeq ($(CONFIG_AP_SMART_MESH),y)
EXTRA_CFLAGS += -DSMART_MESH -DSMART_MESH_MONITOR -DSMART_MESH_HIDDEN_WPS
endif
endif

ifeq ($(CONFIG_RT_WSC),y)
EXTRA_CFLAGS += -DWSC_AP_SUPPORT
ifeq ($(CONFIG_RT_WSC_V2),y)
EXTRA_CFLAGS += -DWSC_V2_SUPPORT
endif
ifeq ($(CONFIG_RT_WSC_NFC),y)
EXTRA_CFLAGS += -DWSC_NFC_SUPPORT
endif
endif

ifeq ($(CONFIG_RT_80211N_DRAFT3),y)
EXTRA_CFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(CONFIG_RT_WAPI),y)
EXTRA_CFLAGS += -DWAPI_SUPPORT
endif

ifeq ($(CONFIG_RT_80211W_PMF),y)
EXTRA_CFLAGS += -DDOT11W_PMF_SUPPORT -DSOFT_ENCRYPT
endif

ifeq ($(CONFIG_RT_ED_MONITOR),y)
EXTRA_CFLAGS += -DED_MONITOR -DED_SMART
endif

ifeq ($(CONFIG_RT_IGMP_SNOOP),y)
EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT
ifeq ($(CONFIG_AP_MWDS),y)
EXTRA_CFLAGS += -DIGMP_MESH
endif
endif

ifeq ($(CONFIG_RT_MCAST_RATE_SPECIFIC),y)
EXTRA_CFLAGS += -DMCAST_RATE_SPECIFIC
endif

ifeq ($(CONFIG_RT_VIDEO_TURBINE),y)
EXTRA_CFLAGS += -DVIDEO_TURBINE_SUPPORT
endif

ifeq ($(CONFIG_RT_BAND_STEERING),y)
EXTRA_CFLAGS += -DBAND_STEERING
endif

ifeq ($(CONFIG_RT_DOT11R_FT),y)
EXTRA_CFLAGS += -DDOT11R_FT_SUPPORT -DIAPP_SUPPORT
endif

ifeq ($(CONFIG_RT_DOT11K_RRM),y)
EXTRA_CFLAGS += -DDOT11K_RRM_SUPPORT -DAPPLE_11K_IOT
endif

ifeq ($(CONFIG_RT_HDR_TRANS),y)
EXTRA_CFLAGS += -DHDR_TRANS_SUPPORT
endif

ifeq ($(CONFIG_RT_CSO),y)
EXTRA_CFLAGS += -DCONFIG_CSO_SUPPORT -DCONFIG_TSO_SUPPORT
endif

ifeq ($(CONFIG_RT_DELAYED_TCP_ACK),y)
EXTRA_CFLAGS += -DDELAYED_TCP_ACK
endif

ifeq ($(CONFIG_RT_SINGLE_SKU),y)
EXTRA_CFLAGS += -DSINGLE_SKU_V2
endif

ifeq ($(CONFIG_RT_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS += -DMEMORY_OPTIMIZATION
endif

ifeq ($(CONFIG_RT_NETIF_BLOCK),y)
EXTRA_CFLAGS += -DBLOCK_NET_IF
endif

ifeq ($(CONFIG_RT_SNMP),y)
EXTRA_CFLAGS += -DSNMP_SUPPORT
endif

ifeq ($(CONFIG_RT_CFG80211),y)
EXTRA_CFLAGS += -DRT_CFG80211_SUPPORT -DEXT_BUILD_CHANNEL_LIST
endif

ifeq ($(CONFIG_RT_BIG_ENDIAN),y)
EXTRA_CFLAGS += -DRT_BIG_ENDIAN
endif

ifeq ($(CONFIG_RT_MC_SUPPORT),y)
EXTRA_CFLAGS += -DMULTIPLE_CARD_SUPPORT
endif
WFLAGS += -Wno-error=date-time

