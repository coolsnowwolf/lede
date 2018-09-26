#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mac80211

PKG_VERSION:=4.19-rc4-1
PKG_RELEASE:=1
PKG_SOURCE_URL:=https://hauke-m.de/files/
PKG_HASH:=5502add3ecfae4c75177f069450297d3e0d980f190206a7a71f3887c67cefee4

PKG_SOURCE:=backports-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/backports-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

PKG_DRIVERS = \
	adm8211 \
	airo \
	ath ath5k ath6kl ath6kl-sdio ath6kl-usb ath9k ath9k-common ath9k-htc ath10k \
	b43 b43legacy \
	carl9170 \
	hermes hermes-pci hermes-pcmcia hermes-plx\
	iwl-legacy iwl3945 iwl4965 iwlwifi \
	lib80211 \
	libipw ipw2100 ipw2200 \
	libertas-sdio libertas-usb libertas-spi \
	mac80211-hwsim \
	mt7601u \
	mwl8k mwifiex-pcie mwifiex-sdio \
	p54-common p54-pci p54-spi p54-usb \
	rsi91x rsi91x-usb rsi91x-sdio\
	rt2x00-lib rt2x00-pci rt2x00-usb \
	rt2400-pci rt2500-pci rt2500-usb \
	rt2800-lib rt2800-mmio rt2800-pci rt2800-soc rt2800-usb \
	rt61-pci rt73-usb \
	rtl8180 rtl8187 \
	rtlwifi rtlwifi-pci rtlwifi-btcoexist rtlwifi-usb rtl8192c-common \
	rtl8192ce rtl8192se rtl8192de rtl8192cu rtl8821ae \
	rtl8xxxu \
	wlcore wl12xx wl18xx \
	zd1211rw

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_kmod-mac80211 \
	$(patsubst %,CONFIG_PACKAGE_kmod-%,$(PKG_DRIVERS)) \
	CONFIG_PACKAGE_MAC80211_DEBUGFS \
	CONFIG_PACKAGE_MAC80211_MESH \
	CONFIG_PACKAGE_MAC80211_TRACING \
	CONFIG_PACKAGE_ATH_DEBUG \
	CONFIG_PACKAGE_ATH_DFS \
	CONFIG_PACKAGE_ATH_SPECTRAL \
	CONFIG_PACKAGE_ATH_DYNACK \
	CONFIG_PACKAGE_B43_DEBUG \
	CONFIG_PACKAGE_B43_PIO \
	CONFIG_PACKAGE_B43_PHY_G \
	CONFIG_PACKAGE_B43_PHY_N \
	CONFIG_PACKAGE_B43_PHY_LP \
	CONFIG_PACKAGE_B43_PHY_HT \
	CONFIG_PACKAGE_B43_BUSES_BCMA_AND_SSB \
	CONFIG_PACKAGE_B43_BUSES_BCMA \
	CONFIG_PACKAGE_B43_BUSES_SSB \
	CONFIG_PACKAGE_BRCM80211_DEBUG \
	CONFIG_PACKAGE_IWLWIFI_DEBUG \
	CONFIG_PACKAGE_IWLWIFI_DEBUGFS \
	CONFIG_PACKAGE_RT2X00_LIB_DEBUGFS \
	CONFIG_PACKAGE_RT2X00_DEBUG \
	CONFIG_PACKAGE_RTLWIFI_DEBUG \
	CONFIG_ATH9K_SUPPORT_PCOEM \
	CONFIG_ATH9K_TX99 \
	CONFIG_ATH10K_LEDS \
	CONFIG_ATH10K_THERMAL \
	CONFIG_ATH_USER_REGD \

include $(INCLUDE_DIR)/package.mk

WMENU:=Wireless Drivers

define KernelPackage/mac80211/Default
  SUBMENU:=$(WMENU)
  URL:=https://wireless.wiki.kernel.org/
  MAINTAINER:=Felix Fietkau <nbd@nbd.name>
endef

define KernelPackage/cfg80211
  $(call KernelPackage/mac80211/Default)
  TITLE:=cfg80211 - wireless configuration API
  DEPENDS+= +iw +wireless-regdb
  FILES:= \
	$(PKG_BUILD_DIR)/compat/compat.ko \
	$(PKG_BUILD_DIR)/net/wireless/cfg80211.ko
endef

define KernelPackage/cfg80211/description
cfg80211 is the Linux wireless LAN (802.11) configuration API.
endef

define KernelPackage/mac80211
  $(call KernelPackage/mac80211/Default)
  TITLE:=Linux 802.11 Wireless Networking Stack
  # +kmod-crypto-cmac is a runtime only dependency of net/mac80211/aes_cmac.c
  DEPENDS+= +kmod-cfg80211 +hostapd-common
  KCONFIG:=\
	CONFIG_AVERAGE=y
  FILES:= $(PKG_BUILD_DIR)/net/mac80211/mac80211.ko
  MENU:=1
endef

define KernelPackage/mac80211/config
  if PACKAGE_kmod-mac80211

	config PACKAGE_MAC80211_DEBUGFS
		bool "Export mac80211 internals in DebugFS"
		select KERNEL_DEBUG_FS
		default y
		help
		  Select this to see extensive information about
		  the internal state of mac80211 in debugfs.

	config PACKAGE_MAC80211_TRACING
		bool "Enable tracing (mac80211 and supported drivers)"
		select KERNEL_FTRACE
		select KERNEL_ENABLE_DEFAULT_TRACERS
		default n
		help
		  Select this to enable tracing of mac80211 and
		  related wifi drivers (using trace-cmd).

	config PACKAGE_MAC80211_MESH
		bool "Enable 802.11s mesh support"
		default y

  endif
endef

define KernelPackage/mac80211/description
Generic IEEE 802.11 Networking Stack (mac80211)
endef

define KernelPackage/adm8211
  $(call KernelPackage/mac80211/Default)
  TITLE:=ADMTek 8211 support
  DEPENDS+=@PCI_SUPPORT +kmod-mac80211 +kmod-eeprom-93cx6
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/admtek/adm8211.ko
  AUTOLOAD:=$(call AutoProbe,adm8211)
endef

define KernelPackage/airo
  $(call KernelPackage/mac80211/Default)
  TITLE:=Cisco Aironet driver
  DEPENDS+=@PCI_SUPPORT +@DRIVER_WEXT_SUPPORT +kmod-cfg80211 @TARGET_x86
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/cisco/airo.ko
  AUTOLOAD:=$(call AutoProbe,airo)
endef

define KernelPackage/airo/description
 Kernel support for Cisco Aironet cards
endef

define KernelPackage/ath/config
  if PACKAGE_kmod-ath
	config ATH_USER_REGD
		bool "Force Atheros drivers to respect the user's regdomain settings"
		default y
		help
		  Atheros' idea of regulatory handling is that the EEPROM of the card defines
		  the regulatory limits and the user is only allowed to restrict the settings
		  even further, even if the country allows frequencies or power levels that
		  are forbidden by the EEPROM settings.

		  Select this option if you want the driver to respect the user's decision about
		  regulatory settings.

	config PACKAGE_ATH_DEBUG
		bool "Atheros wireless debugging"
		help
		  Say Y, if you want to debug atheros wireless drivers.
		  Only ath9k & ath10k make use of this.

	config PACKAGE_ATH_DFS
		bool "Enable DFS support"
		default y
		help
		  Dynamic frequency selection (DFS) is required for most of the 5 GHz band
		  channels in Europe, US, and Japan.

		  Select this option if you want to use such channels.

	config PACKAGE_ATH_SPECTRAL
		bool "Atheros spectral scan support"
		depends on PACKAGE_ATH_DEBUG
		select KERNEL_RELAY
		help
		  Say Y to enable access to the FFT/spectral data via debugfs.

	config PACKAGE_ATH_DYNACK
		bool "Enable Dynack support"
		depends on PACKAGE_kmod-ath9k-common
		help
		  Enables support for Dynamic ACK estimation, which allows the fastest possible speed
		  at any distance automatically by increasing/decreasing the max frame ACK time for
		  the most remote station detected.  It can be enabled by using iw (iw phy0 set distance auto),
		  or by sending the NL80211_ATTR_WIPHY_DYN_ACK flag to mac80211 driver using netlink.

		  Select this option if you want to enable this feature

  endif
endef

define KernelPackage/ath
  $(call KernelPackage/mac80211/Default)
  TITLE:=Atheros common driver part
  DEPENDS+= @PCI_SUPPORT||USB_SUPPORT||TARGET_ar71xx||TARGET_ath79||TARGET_ath25 +kmod-mac80211
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath.ko
  MENU:=1
endef

define KernelPackage/ath/description
 This module contains some common parts needed by Atheros Wireless drivers.
endef

define KernelPackage/ath5k
  $(call KernelPackage/mac80211/Default)
  TITLE:=Atheros 5xxx wireless cards support
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/ath5k
  DEPENDS+= @PCI_SUPPORT||@TARGET_ath25 +kmod-ath +@DRIVER_11W_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath5k/ath5k.ko
  AUTOLOAD:=$(call AutoProbe,ath5k)
endef

define KernelPackage/ath5k/description
 This module adds support for wireless adapters based on
 Atheros 5xxx chipset.
endef

define KernelPackage/ath6kl
  $(call KernelPackage/mac80211/Default)
  TITLE:=Atheros FullMAC wireless devices (common code for ath6kl_sdio and ath6kl_usb)
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/ath6kl
  HIDDEN:=1
  DEPENDS+= +kmod-ath +@DRIVER_11N_SUPPORT
  FILES:= $(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath6kl/ath6kl_core.ko
endef

define KernelPackage/ath6kl-sdio
  $(call KernelPackage/mac80211/Default)
  TITLE:=Atheros 802.11n SDIO wireless cards support
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/ath6kl
  DEPENDS+= +kmod-mmc +kmod-ath6kl
  FILES:= $(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath6kl/ath6kl_sdio.ko
  AUTOLOAD:=$(call AutoProbe,ath6kl_sdio)
endef

define KernelPackage/ath6kl-sdio/description
This module adds support for wireless adapters based on
Atheros IEEE 802.11n AR6003 and AR6004 family of chipsets.
endef

define KernelPackage/ath6kl-usb
  $(call KernelPackage/mac80211/Default)
  TITLE:=Atheros 802.11n USB wireless cards support
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/ath6kl
  DEPENDS+= @USB_SUPPORT +kmod-usb-core +kmod-ath6kl
  FILES:= $(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath6kl/ath6kl_usb.ko
  AUTOLOAD:=$(call AutoProbe,ath6kl_usb)
endef

define KernelPackage/ath6kl-usb/description
This module adds support for wireless adapters based on the
Atheros IEEE 802.11n AR6004 chipset.
endef

define KernelPackage/ath9k-common
  $(call KernelPackage/mac80211/Default)
  TITLE:=Atheros 802.11n wireless devices (common code for ath9k and ath9k_htc)
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/ath9k
  HIDDEN:=1
  DEPENDS+= @PCI_SUPPORT||USB_SUPPORT||TARGET_ar71xx||TARGET_ath79 +kmod-ath +@DRIVER_11N_SUPPORT +@DRIVER_11W_SUPPORT
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath9k/ath9k_common.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath9k/ath9k_hw.ko
endef

define KernelPackage/ath9k
  $(call KernelPackage/mac80211/Default)
  TITLE:=Atheros 802.11n PCI wireless cards support
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/ath9k
  DEPENDS+= @PCI_SUPPORT||TARGET_ar71xx||TARGET_ath79 +kmod-ath9k-common
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath9k/ath9k.ko
  AUTOLOAD:=$(call AutoProbe,ath9k)
endef

define KernelPackage/ath9k/description
This module adds support for wireless adapters based on
Atheros IEEE 802.11n AR5008 and AR9001 family of chipsets.
endef

define KernelPackage/ath9k/config

	config ATH9K_SUPPORT_PCOEM
		bool "Support chips used in PC OEM cards"
		depends on PACKAGE_kmod-ath9k

       config ATH9K_TX99
               bool "Enable TX99 support (WARNING: testing only, breaks normal operation!)"
               depends on PACKAGE_kmod-ath9k

	config ATH9K_UBNTHSR
		bool "Support for Ubiquiti UniFi Outdoor+ access point"
		depends on PACKAGE_kmod-ath9k && (TARGET_ar71xx_generic||TARGET_ath79)
		default y

endef

define KernelPackage/ath9k-htc
  $(call KernelPackage/mac80211/Default)
  TITLE:=Atheros 802.11n USB device support
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/ath9k
  DEPENDS+= @USB_SUPPORT +kmod-ath9k-common +kmod-usb-core +ath9k-htc-firmware
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath9k/ath9k_htc.ko
  AUTOLOAD:=$(call AutoProbe,ath9k_htc)
endef

define KernelPackage/ath9k-htc/description
This module adds support for wireless adapters based on
Atheros USB AR9271 and AR7010 family of chipsets.
endef

define KernelPackage/ath10k
  $(call KernelPackage/mac80211/Default)
  TITLE:=Atheros 802.11ac wireless cards support
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/ath10k
  DEPENDS+= @PCI_SUPPORT +kmod-ath +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT +@DRIVER_11W_SUPPORT \
	+ATH10K_THERMAL:kmod-hwmon-core +ATH10K_THERMAL:kmod-thermal
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath10k/ath10k_core.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/ath/ath10k/ath10k_pci.ko
  AUTOLOAD:=$(call AutoProbe,ath10k_pci)
endef

define KernelPackage/ath10k/description
This module adds support for wireless adapters based on
Atheros IEEE 802.11ac family of chipsets. For now only
PCI is supported.
endef

define KernelPackage/ath10k/config

       config ATH10K_LEDS
               bool "Enable LED support"
               default y
               depends on PACKAGE_kmod-ath10k

       config ATH10K_THERMAL
               bool "Enable thermal sensors and throttling support"
               depends on PACKAGE_kmod-ath10k

endef

#Broadcom firmware
ifneq ($(CONFIG_B43_FW_6_30),)
  PKG_B43_FWV4_NAME:=broadcom-wl
  PKG_B43_FWV4_VERSION:=6.30.163.46
  PKG_B43_FWV4_OBJECT:=$(PKG_B43_FWV4_NAME)-$(PKG_B43_FWV4_VERSION).wl_apsta.o
  PKG_B43_FWV4_SOURCE:=$(PKG_B43_FWV4_NAME)-$(PKG_B43_FWV4_VERSION).tar.bz2
  PKG_B43_FWV4_SOURCE_URL:=http://www.lwfinger.com/b43-firmware/
  PKG_B43_FWV4_HASH:=a07c3b6b277833c7dbe61daa511f908cd66c5e2763eb7a0859abc36cd9335c2d
else
ifneq ($(CONFIG_B43_FW_5_10),)
  PKG_B43_FWV4_NAME:=broadcom-wl
  PKG_B43_FWV4_VERSION:=5.10.56.27.3
  PKG_B43_FWV4_OBJECT:=$(PKG_B43_FWV4_NAME)-$(PKG_B43_FWV4_VERSION)/driver/wl_apsta/wl_prebuilt.o
  PKG_B43_FWV4_SOURCE:=$(PKG_B43_FWV4_NAME)-$(PKG_B43_FWV4_VERSION)_mipsel.tar.bz2
  PKG_B43_FWV4_SOURCE_URL:=http://mirror2.openwrt.org/sources/
  PKG_B43_FWV4_HASH:=26a8c370f48fc129d0731cfd751c36cae1419b0bc8ca35781126744e60eae009
else
ifneq ($(CONFIG_B43_FW_4_178),)
  PKG_B43_FWV4_NAME:=broadcom-wl
  PKG_B43_FWV4_VERSION:=4.178.10.4
  PKG_B43_FWV4_OBJECT:=$(PKG_B43_FWV4_NAME)-$(PKG_B43_FWV4_VERSION)/linux/wl_apsta.o
  PKG_B43_FWV4_SOURCE:=$(PKG_B43_FWV4_NAME)-$(PKG_B43_FWV4_VERSION).tar.bz2
  PKG_B43_FWV4_SOURCE_URL:=http://mirror2.openwrt.org/sources/
  PKG_B43_FWV4_HASH:=32f6ad98facbb9045646fdc8b54bb03086d204153253f9c65d0234a5d90ae53f
else
ifneq ($(CONFIG_B43_FW_5_100_138),)
  PKG_B43_FWV4_NAME:=broadcom-wl
  PKG_B43_FWV4_VERSION:=5.100.138
  PKG_B43_FWV4_OBJECT:=$(PKG_B43_FWV4_NAME)-$(PKG_B43_FWV4_VERSION)/linux/wl_apsta.o
  PKG_B43_FWV4_SOURCE:=$(PKG_B43_FWV4_NAME)-$(PKG_B43_FWV4_VERSION).tar.bz2
  PKG_B43_FWV4_SOURCE_URL:=http://www.lwfinger.com/b43-firmware/
  PKG_B43_FWV4_HASH:=f1e7067aac5b62b67b8b6e4c517990277804339ac16065eb13c731ff909ae46f
else
  PKG_B43_FWV4_NAME:=broadcom-wl
  PKG_B43_FWV4_VERSION:=4.150.10.5
  PKG_B43_FWV4_OBJECT:=$(PKG_B43_FWV4_NAME)-$(PKG_B43_FWV4_VERSION)/driver/wl_apsta_mimo.o
  PKG_B43_FWV4_SOURCE:=$(PKG_B43_FWV4_NAME)-$(PKG_B43_FWV4_VERSION).tar.bz2
  PKG_B43_FWV4_SOURCE_URL:=http://mirror2.openwrt.org/sources/
  PKG_B43_FWV4_HASH:=a9f4e276a4d8d3a1cd0f2eb87080ae89b77f0a7140f06d4e9e2135fc44fdd533
endif
endif
endif
endif
ifneq ($(CONFIG_B43_OPENFIRMWARE),)
  PKG_B43_FWV4_NAME:=broadcom-wl
  PKG_B43_FWV4_VERSION:=5.2
  PKG_B43_FWV4_OBJECT:=openfwwf-$(PKG_B43_FWV4_VERSION)
  PKG_B43_FWV4_SOURCE:=openfwwf-$(PKG_B43_FWV4_VERSION).tar.gz
  PKG_B43_FWV4_SOURCE_URL:=http://netweb.ing.unibs.it/~openfwwf/firmware
  PKG_B43_FWV4_HASH:=9de03320083201080b2e94b81637ac07a159cf4e6f3481383e1a217e627bc0dc
endif


define Download/b43
  FILE:=$(PKG_B43_FWV4_SOURCE)
  URL:=$(PKG_B43_FWV4_SOURCE_URL)
  HASH:=$(PKG_B43_FWV4_HASH)
endef
$(eval $(call Download,b43))

define KernelPackage/b43
  $(call KernelPackage/mac80211/Default)
  TITLE:=Broadcom 43xx wireless support
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/b43
  KCONFIG:= \
  	CONFIG_HW_RANDOM=y
  # Depend on PCI_SUPPORT to make sure we can select kmod-bcma or kmod-ssb
  DEPENDS += \
	@PCI_SUPPORT +kmod-mac80211 \
	$(if $(CONFIG_PACKAGE_B43_USE_SSB),+kmod-ssb) \
	$(if $(CONFIG_PACKAGE_B43_USE_BCMA),+kmod-bcma)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/broadcom/b43/b43.ko
  AUTOLOAD:=$(call AutoProbe,b43)
  MENU:=1
endef

define KernelPackage/b43/config

config PACKAGE_B43_USE_SSB
	select PACKAGE_kmod-ssb
	tristate
	depends on !TARGET_brcm47xx && !TARGET_brcm63xx
	default PACKAGE_kmod-b43 if PACKAGE_B43_BUSES_BCMA_AND_SSB
	default PACKAGE_kmod-b43 if PACKAGE_B43_BUSES_SSB

config PACKAGE_B43_USE_BCMA
	select PACKAGE_kmod-bcma
	tristate
	depends on !TARGET_brcm47xx && !TARGET_bcm53xx
	default PACKAGE_kmod-b43 if PACKAGE_B43_BUSES_BCMA_AND_SSB
	default PACKAGE_kmod-b43 if PACKAGE_B43_BUSES_BCMA

  if PACKAGE_kmod-b43

	choice
		prompt "b43 firmware version"
		default B43_FW_5_100_138
		help
		  This option allows you to select the version of the b43 firmware.

	config B43_FW_4_150
		bool "Firmware 410.2160 from driver 4.150.10.5 (old stable)"
		help
		  Old stable firmware for BCM43xx devices.

		  If unsure, select this.

	config B43_FW_4_178
		bool "Firmware 478.104 from driver 4.178.10.4"
		help
		  Older firmware for BCM43xx devices.

		  If unsure, select the "stable" firmware.

	config B43_FW_5_10
		bool "Firmware 508.1084 from driver 5.10.56.27"
		help
		  Older firmware for BCM43xx devices.

		  If unsure, select the "stable" firmware.

	config B43_FW_5_100_138
		bool "Firmware 666.2 from driver 5.100.138 (stable)"
		help
		  The currently default firmware for BCM43xx devices.

		  This firmware currently gets most of the testing and is needed for some N-PHY devices.

		  If unsure, select the this firmware.

	config B43_FW_6_30
		bool "Firmware 784.2 from driver 6.30.163.46 (experimental)"
		help
		  Newer experimental firmware for BCM43xx devices.

		  This firmware is mostly untested.

		  If unsure, select the "stable" firmware.

	config B43_OPENFIRMWARE
		bool "Open FirmWare for WiFi networks"
		help
		  Opensource firmware for BCM43xx devices.

		  Do _not_ select this, unless you know what you are doing.
		  The Opensource firmware is not suitable for embedded devices, yet.
		  It does not support QoS, which is bad for AccessPoints.
		  It does not support hardware crypto acceleration, which is a showstopper
		  for embedded devices with low CPU resources.

		  If unsure, select the "stable" firmware.

	endchoice

	config B43_FW_SQUASH
		bool "Remove unnecessary firmware files"
		depends on !B43_OPENFIRMWARE
		default y
		help
		  This options allows you to remove unnecessary b43 firmware files
		  from the final rootfs image. This can reduce the rootfs size by
		  up to 200k.

		  If unsure, say Y.

	config B43_FW_SQUASH_COREREVS
		string "Core revisions to include"
		depends on B43_FW_SQUASH
		default "5,6,7,8,9,10,11,13,15" if TARGET_brcm47xx_legacy
		default "16,28,29,30" if TARGET_brcm47xx_mips74k
		default "5,6,7,8,9,10,11,13,15,16,28,29,30"
		help
		  This is a comma seperated list of core revision numbers.

		  Example (keep files for rev5 only):
		    5

		  Example (keep files for rev5 and rev11):
		    5,11

	config B43_FW_SQUASH_PHYTYPES
		string "PHY types to include"
		depends on B43_FW_SQUASH
		default "G,N,LP" if TARGET_brcm47xx_legacy
		default "N,HT" if TARGET_brcm47xx_mips74k
		default "G,N,LP,HT"
		help
		  This is a comma seperated list of PHY types:
		    A  => A-PHY
		    AG => Dual A-PHY G-PHY
		    G  => G-PHY
		    LP => LP-PHY
		    N  => N-PHY
		    HT  => HT-PHY
		    LCN  => LCN-PHY
		    LCN40  => LCN40-PHY
		    AC  => AC-PHY

		  Example (keep files for G-PHY only):
		    G

		  Example (keep files for G-PHY and N-PHY):
		    G,N

	choice
		prompt "Supported buses"
		default PACKAGE_B43_BUSES_BCMA_AND_SSB
		help
		  This allows choosing buses that b43 should support.

	config PACKAGE_B43_BUSES_BCMA_AND_SSB
		depends on !TARGET_brcm47xx_legacy && !TARGET_brcm47xx_mips74k && !TARGET_bcm53xx
		bool "BCMA and SSB"

	config PACKAGE_B43_BUSES_BCMA
		depends on !TARGET_brcm47xx_legacy
		bool "BCMA only"

	config PACKAGE_B43_BUSES_SSB
		depends on !TARGET_brcm47xx_mips74k && !TARGET_bcm53xx
		bool "SSB only"

	endchoice

	config PACKAGE_B43_DEBUG
		bool "Enable debug output and debugfs for b43"
		default n
		help
		  Enable additional debug output and runtime sanity checks for b43
		  and enables the debugfs interface.

		  If unsure, say N.

	config PACKAGE_B43_PIO
		bool "Enable support for PIO transfer mode"
		default n
		help
		  Enable support for using PIO instead of DMA. Unless you have DMA
		  transfer problems you don't need this.

		  If unsure, say N.

	config PACKAGE_B43_PHY_G
		bool "Enable support for G-PHYs"
		default n if TARGET_brcm47xx_mips74k
		default y
		help
		  Enable support for G-PHY. This includes support for the following devices:
		  PCI: BCM4306, BCM4311, BCM4318
		  SoC: BCM5352E, BCM4712

		  If unsure, say Y.

	config PACKAGE_B43_PHY_N
		bool "Enable support for N-PHYs"
		default y
		help
		  Enable support for N-PHY. This includes support for the following devices:
		  PCI: BCM4321, BCM4322, BCM43222, BCM43224, BCM43225
		  SoC: BCM4716, BCM4717, BCM4718

		  Currently only 11g speed is available.

		  If unsure, say Y.

	config PACKAGE_B43_PHY_LP
		bool "Enable support for LP-PHYs"
		default n if TARGET_brcm47xx_mips74k
		default y
		help
		  Enable support for LP-PHY. This includes support for the following devices:
		  PCI: BCM4312
		  SoC: BCM5354

		  If unsure, say Y.

	config PACKAGE_B43_PHY_HT
		bool "Enable support for HT-PHYs"
		default n if TARGET_brcm47xx_legacy
		default y
		help
		  Enable support for HT-PHY. This includes support for the following devices:
		  PCI: BCM4331

		  Currently only 11g speed is available.

		  If unsure, say Y.

	config PACKAGE_B43_PHY_LCN
		bool "Enable support for LCN-PHYs"
		depends on BROKEN
		default n
		help
		  Currently broken.

		  If unsure, say N.

  endif
endef

define KernelPackage/b43/description
Kernel module for Broadcom 43xx wireless support (mac80211 stack) new
endef

define KernelPackage/b43legacy
  $(call KernelPackage/mac80211/Default)
  TITLE:=Broadcom 43xx-legacy wireless support
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/b43
  KCONFIG:= \
  	CONFIG_HW_RANDOM=y
  DEPENDS+= +kmod-mac80211 +!(TARGET_brcm47xx||TARGET_brcm63xx):kmod-ssb +b43legacy-firmware
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/broadcom/b43legacy/b43legacy.ko
  AUTOLOAD:=$(call AutoProbe,b43legacy)
  MENU:=1
endef

define KernelPackage/b43legacy/description
Kernel module for Broadcom 43xx-legacy wireless support (mac80211 stack) new
endef


define KernelPackage/brcmutil
  $(call KernelPackage/mac80211/Default)
  TITLE:=Broadcom IEEE802.11n common driver parts
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/brcm80211
  DEPENDS+=@PCI_SUPPORT||USB_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/broadcom/brcm80211/brcmutil/brcmutil.ko
  AUTOLOAD:=$(call AutoProbe,brcmutil)
  MENU:=1
endef

define KernelPackage/brcmutil/description
 This module contains some common parts needed by Broadcom Wireless drivers brcmsmac and brcmfmac.
endef

define KernelPackage/brcmutil/config
  if PACKAGE_kmod-brcmutil

	config PACKAGE_BRCM80211_DEBUG
		bool "Broadcom wireless driver debugging"
		help
		  Say Y, if you want to debug brcmsmac and brcmfmac wireless driver.

  endif
endef

PKG_BRCMSMAC_FW_NAME:=broadcom-wl
PKG_BRCMSMAC_FW_VERSION:=5.100.138
PKG_BRCMSMAC_FW_OBJECT:=$(PKG_BRCMSMAC_FW_NAME)-$(PKG_BRCMSMAC_FW_VERSION)/linux/wl_apsta.o
PKG_BRCMSMAC_FW_SOURCE:=$(PKG_BRCMSMAC_FW_NAME)-$(PKG_BRCMSMAC_FW_VERSION).tar.bz2
PKG_BRCMSMAC_FW_SOURCE_URL:=http://www.lwfinger.com/b43-firmware/
PKG_BRCMSMAC_FW_HASH:=f1e7067aac5b62b67b8b6e4c517990277804339ac16065eb13c731ff909ae46f

define Download/brcmsmac
  FILE:=$(PKG_BRCMSMAC_FW_SOURCE)
  URL:=$(PKG_BRCMSMAC_FW_SOURCE_URL)
  HASH:=$(PKG_BRCMSMAC_FW_HASH)
endef
$(eval $(call Download,brcmsmac))

define KernelPackage/brcmsmac
  $(call KernelPackage/mac80211/Default)
  TITLE:=Broadcom IEEE802.11n PCIe SoftMAC WLAN driver
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/brcm80211
  DEPENDS+= +kmod-mac80211 +@DRIVER_11N_SUPPORT +!TARGET_brcm47xx:kmod-bcma +kmod-lib-cordic +kmod-lib-crc8 +kmod-brcmutil +!BRCMSMAC_USE_FW_FROM_WL:brcmsmac-firmware
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/broadcom/brcm80211/brcmsmac/brcmsmac.ko
  AUTOLOAD:=$(call AutoProbe,brcmsmac)
  MENU:=1
endef

define KernelPackage/brcmsmac/description
 Kernel module for Broadcom IEEE802.11n PCIe Wireless cards
endef

define KernelPackage/brcmsmac/config
  if PACKAGE_kmod-brcmsmac

	config BRCMSMAC_USE_FW_FROM_WL
		bool "Use firmware extracted from broadcom proprietary driver"
		default y
		help
		  Instead of using the official brcmsmac firmware a firmware
		  version 666.2 extracted from the proprietary Broadcom driver
		  is used. This is needed to get core rev 17 used in bcm4716
		  to work.

		  If unsure, say Y.

  endif
endef


define KernelPackage/brcmfmac
  $(call KernelPackage/mac80211/Default)
  TITLE:=Broadcom IEEE802.11n USB FullMAC WLAN driver
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/brcm80211
  DEPENDS+= @USB_SUPPORT +kmod-cfg80211 +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT +kmod-brcmutil \
	+BRCMFMAC_SDIO:kmod-mmc @!TARGET_uml \
	+BRCMFMAC_USB:kmod-usb-core +BRCMFMAC_USB:brcmfmac-firmware-usb
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/broadcom/brcm80211/brcmfmac/brcmfmac.ko
  AUTOLOAD:=$(call AutoProbe,brcmfmac)
endef

define KernelPackage/brcmfmac/description
 Kernel module for Broadcom IEEE802.11n USB Wireless cards
endef

define KernelPackage/brcmfmac/config
  if PACKAGE_kmod-brcmfmac

	config BRCMFMAC_SDIO
		bool "Enable SDIO bus interface support"
		default y if TARGET_brcm2708
		default y if TARGET_sunxi
		default n
		help
		  Enable support for cards attached to an SDIO bus.
		  Select this option only if you are sure that your
		  board has a Broadcom wireless chip atacched to
		  that bus.

	config BRCMFMAC_USB
		bool "Enable USB bus interface support"
		depends on USB_SUPPORT
		default y
		help
		  Supported USB connected chipsets:
		  BCM43235, BCM43236, BCM43238 (all in revision 3 only)
		  BCM43143, BCM43242, BCM43566, BCM43569

	config BRCMFMAC_PCIE
		bool "Enable PCIE bus interface support"
		depends on PCI_SUPPORT
		default y
		help
		  Supported PCIe connected chipsets:
		  BCM4354, BCM4356, BCM43567, BCM43570, BCM43602

  endif
endef


define KernelPackage/carl9170
  $(call KernelPackage/mac80211/Default)
  TITLE:=Driver for Atheros AR9170 USB sticks
  DEPENDS:=@USB_SUPPORT +kmod-mac80211 +kmod-ath +kmod-usb-core +kmod-input-core +@DRIVER_11N_SUPPORT +carl9170-firmware
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ath/carl9170/carl9170.ko
  AUTOLOAD:=$(call AutoProbe,carl9170)
endef


define KernelPackage/hermes
  $(call KernelPackage/mac80211/Default)
  TITLE:=Hermes 802.11b chipset support
  DEPENDS:=@PCI_SUPPORT||PCMCIA_SUPPORT +kmod-cfg80211 +@DRIVER_WEXT_SUPPORT +kmod-crypto-michael-mic
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intersil/orinoco/orinoco.ko
  AUTOLOAD:=$(call AutoProbe,orinoco)
endef

define KernelPackage/hermes/description
 Kernel support for Hermes 802.11b chipsets
endef

define KernelPackage/hermes-pci
  $(call KernelPackage/mac80211/Default)
  TITLE:=Intersil Prism 2.5 PCI support
  DEPENDS:=@PCI_SUPPORT +kmod-hermes
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intersil/orinoco/orinoco_pci.ko
  AUTOLOAD:=$(call AutoProbe,orinoco_pci)
endef

define KernelPackage/hermes-pci/description
 Kernel modules for Intersil Prism 2.5 PCI support
endef

define KernelPackage/hermes-plx
  $(call KernelPackage/mac80211/Default)
  TITLE:=PLX9052 based PCI adaptor
  DEPENDS:=@PCI_SUPPORT +kmod-hermes
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intersil/orinoco/orinoco_plx.ko
  AUTOLOAD:=$(call AutoProbe,orinoco_plx)
endef

define KernelPackage/hermes-plx/description
 Kernel modules for Hermes in PLX9052 based PCI adaptors
endef

define KernelPackage/hermes-pcmcia
  $(call KernelPackage/mac80211/Default)
  TITLE:=Hermes based PCMCIA adaptors
  DEPENDS:=@PCMCIA_SUPPORT +kmod-hermes
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intersil/orinoco/orinoco_cs.ko
  AUTOLOAD:=$(call AutoProbe,orinoco_cs)
endef

define KernelPackage/hermes-pcmcia/description
 Kernel modules for Hermes based PCMCIA adaptors
endef


define KernelPackage/iwlwifi
  $(call KernelPackage/mac80211/Default)
  DEPENDS:= +kmod-mac80211 @PCI_SUPPORT +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT @!LINUX_3_18
  TITLE:=Intel AGN Wireless support
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/intel/iwlwifi/iwlwifi.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/intel/iwlwifi/dvm/iwldvm.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/intel/iwlwifi/mvm/iwlmvm.ko
  AUTOLOAD:=$(call AutoProbe,iwlwifi iwldvm iwlmvm)
  MENU:=1
endef

define KernelPackage/iwlwifi/description
 iwlwifi kernel module for
 Intel Wireless WiFi Link 6250AGN Adapter
 Intel 6000 Series Wi-Fi Adapters (6200AGN and 6300AGN)
 Intel WiFi Link 1000BGN
 Intel Wireless WiFi 5150AGN
 Intel Wireless WiFi 5100AGN, 5300AGN, and 5350AGN
 Intel 6005 Series Wi-Fi Adapters
 Intel 6030 Series Wi-Fi Adapters
 Intel Wireless WiFi Link 6150BGN 2 Adapter
 Intel 100 Series Wi-Fi Adapters (100BGN and 130BGN)
 Intel 2000 Series Wi-Fi Adapters
 Intel 7260 Wi-Fi Adapter
 Intel 3160 Wi-Fi Adapter
 Intel 7265 Wi-Fi Adapter
 Intel 8260 Wi-Fi Adapter
 Intel 3165 Wi-Fi Adapter
endef

define KernelPackage/iwlwifi/config
  if PACKAGE_kmod-iwlwifi

	config PACKAGE_IWLWIFI_DEBUG
		bool "Enable full debugging output in the iwlwifi driver"
		default n
		help
		  This option will enable debug tracing output for the iwlwifi drivers

		  This will result in the kernel module being ~100k larger.  You can
		  control which debug output is sent to the kernel log by setting the
		  value in

			/sys/module/iwlwifi/parameters/debug

		  This entry will only exist if this option is enabled.

		  To set a value, simply echo an 8-byte hex value to the same file:

			  % echo 0x43fff > /sys/module/iwlwifi/parameters/debug

		  You can find the list of debug mask values in:
			  drivers/net/wireless/intel/iwlwifi/iwl-debug.h

		  If this is your first time using this driver, you should say Y here
		  as the debug information can assist others in helping you resolve
		  any problems you may encounter.

	config PACKAGE_IWLWIFI_DEBUGFS
	        bool "iwlwifi debugfs support"
		depends on PACKAGE_MAC80211_DEBUGFS
		default n
		help
		  Enable creation of debugfs files for the iwlwifi drivers. This
		  is a low-impact option that allows getting insight into the
		  driver's state at runtime.

  endif
endef

define KernelPackage/iwl-legacy
  $(call KernelPackage/mac80211/Default)
  DEPENDS:= +kmod-mac80211 @PCI_SUPPORT
  TITLE:=Intel legacy Wireless support
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intel/iwlegacy/iwlegacy.ko
  AUTOLOAD:=$(call AutoProbe,iwlegacy)
endef

define KernelPackage/iwl-legacy/description
 iwl-legacy kernel module for legacy Intel wireless support
endef

define KernelPackage/iwl3945
  $(call KernelPackage/mac80211/Default)
  DEPENDS:= +kmod-mac80211 +kmod-iwl-legacy +iwl3945-firmware
  TITLE:=Intel iwl3945 Wireless support
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intel/iwlegacy/iwl3945.ko
  AUTOLOAD:=$(call AutoProbe,iwl3945)
endef

define KernelPackage/iwl3945/description
 iwl3945 kernel module for Intel 3945 support
endef

define KernelPackage/iwl4965
  $(call KernelPackage/mac80211/Default)
  DEPENDS:= +kmod-mac80211 +kmod-iwl-legacy +@DRIVER_11N_SUPPORT +iwl4965-firmware
  TITLE:=Intel iwl4965 Wireless support
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intel/iwlegacy/iwl4965.ko
  AUTOLOAD:=$(call AutoProbe,iwl4965)
endef

define KernelPackage/iwl4965/description
 iwl4965 kernel module for Intel 4965 support
endef


define KernelPackage/lib80211
  $(call KernelPackage/mac80211/Default)
  TITLE:=802.11 Networking stack
  DEPENDS:=+kmod-cfg80211 +kmod-crypto-hash
  FILES:= \
	$(PKG_BUILD_DIR)/net/wireless/lib80211.ko \
	$(PKG_BUILD_DIR)/net/wireless/lib80211_crypt_wep.ko \
	$(PKG_BUILD_DIR)/net/wireless/lib80211_crypt_ccmp.ko \
	$(PKG_BUILD_DIR)/net/wireless/lib80211_crypt_tkip.ko
  AUTOLOAD:=$(call AutoProbe, \
	lib80211 \
	lib80211_crypt_wep \
	lib80211_crypt_ccmp \
	lib80211_crypt_tkip \
  )
endef

define KernelPackage/lib80211/description
 Kernel modules for 802.11 Networking stack
 Includes:
 - lib80211
 - lib80211_crypt_wep
 - lib80211_crypt_tkip
 - lib80211_crytp_ccmp
endef


define KernelPackage/libipw
  $(call KernelPackage/mac80211/Default)
  TITLE:=libipw for ipw2100 and ipw2200
  DEPENDS:=@PCI_SUPPORT +kmod-crypto-michael-mic +kmod-crypto-ecb +kmod-lib80211 +kmod-cfg80211 +@DRIVER_WEXT_SUPPORT @!BIG_ENDIAN @!LINUX_3_18
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intel/ipw2x00/libipw.ko
  AUTOLOAD:=$(call AutoProbe,libipw)
endef

define KernelPackage/libipw/description
 Hardware independent IEEE 802.11 networking stack for ipw2100 and ipw2200.
endef

IPW2100_NAME:=ipw2100-fw
IPW2100_VERSION:=1.3

define Download/ipw2100
  URL:= \
	https://src.fedoraproject.org/repo/pkgs/ipw2100-firmware/ipw2100-fw-1.3.tgz/46aa75bcda1a00efa841f9707bbbd113/ \
	https://archlinux.mirror.pkern.at/other/packages/ipw2100-fw/ \
	http://mirror.ox.ac.uk/sites/ftp.openbsd.org/pub/OpenBSD/distfiles/firmware/ \
	http://firmware.openbsd.org/firmware-dist/
  FILE:=$(IPW2100_NAME)-$(IPW2100_VERSION).tgz
  HASH:=e1107c455e48d324a616b47a622593bc8413dcce72026f72731c0b03dae3a7a2
endef
$(eval $(call Download,ipw2100))

define KernelPackage/ipw2100
  $(call KernelPackage/mac80211/Default)
  TITLE:=Intel IPW2100 driver
  DEPENDS:=@PCI_SUPPORT +kmod-libipw
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intel/ipw2x00/ipw2100.ko
  AUTOLOAD:=$(call AutoProbe,ipw2100)
endef

define KernelPackage/ipw2100/description
 Kernel support for Intel IPW2100
 Includes:
 - ipw2100
endef

IPW2200_NAME:=ipw2200-fw
IPW2200_VERSION:=3.1

define Download/ipw2200
  URL:= \
	https://src.fedoraproject.org/repo/pkgs/ipw2200-firmware/ipw2200-fw-3.1.tgz/eaba788643c7cc7483dd67ace70f6e99/ \
	https://archlinux.mirror.pkern.at/other/packages/ipw2200-fw/ \
	http://mirror.ox.ac.uk/sites/ftp.openbsd.org/pub/OpenBSD/distfiles/firmware/ \
	http://firmware.openbsd.org/firmware-dist/
  FILE:=$(IPW2200_NAME)-$(IPW2200_VERSION).tgz
  HASH:=c6818c11c18cc030d55ff83f64b2bad8feef485e7742f84f94a61d811a6258bd
endef
$(eval $(call Download,ipw2200))

define KernelPackage/ipw2200
  $(call KernelPackage/mac80211/Default)
  TITLE:=Intel IPW2200 driver
  DEPENDS:=@PCI_SUPPORT +kmod-libipw
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intel/ipw2x00/ipw2200.ko
  AUTOLOAD:=$(call AutoProbe,ipw2200)
endef

define KernelPackage/ipw2200/description
 Kernel support for Intel IPW2200
 Includes:
 - ipw2200
endef


define KernelPackage/libertas-usb
  $(call KernelPackage/mac80211/Default)
  DEPENDS+= @USB_SUPPORT +kmod-cfg80211 +kmod-usb-core +kmod-lib80211 +@DRIVER_WEXT_SUPPORT +libertas-usb-firmware @!LINUX_3_18
  TITLE:=Marvell 88W8015 Wireless Driver
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/libertas/libertas.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/libertas/usb8xxx.ko
  AUTOLOAD:=$(call AutoProbe,libertas usb8xxx)
endef

define KernelPackage/libertas-sdio
  $(call KernelPackage/mac80211/Default)
  DEPENDS+= +kmod-cfg80211 +kmod-lib80211 +kmod-mmc +@DRIVER_WEXT_SUPPORT @!TARGET_uml +libertas-sdio-firmware @!LINUX_3_18
  TITLE:=Marvell 88W8686 Wireless Driver
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/libertas/libertas.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/libertas/libertas_sdio.ko
  AUTOLOAD:=$(call AutoProbe,libertas libertas_sdio)
endef

define KernelPackage/libertas-spi
  $(call KernelPackage/mac80211/Default)
  SUBMENU:=Wireless Drivers
  DEPENDS+= +kmod-cfg80211 +kmod-lib80211 +@DRIVER_WEXT_SUPPORT @!TARGET_uml +libertas-spi-firmware @!LINUX_3_18
  KCONFIG := \
	CONFIG_SPI=y \
	CONFIG_SPI_MASTER=y
  TITLE:=Marvell 88W8686 SPI Wireless Driver
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/libertas/libertas.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/libertas/libertas_spi.ko
  AUTOLOAD:=$(call AutoProbe,libertas libertas_spi)
endef

define KernelPackage/mac80211-hwsim
  $(call KernelPackage/mac80211/Default)
  TITLE:=mac80211 HW simulation device
  DEPENDS+= +kmod-mac80211 +@DRIVER_11AC_SUPPORT +@DRIVER_11N_SUPPORT +@DRIVER_11W_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/mac80211_hwsim.ko
  AUTOLOAD:=$(call AutoProbe,mac80211_hwsim)
endef


define KernelPackage/mt7601u
  $(call KernelPackage/mac80211/Default)
  TITLE:=MT7601U-based USB dongles Wireless Driver
  DEPENDS+= +kmod-mac80211 +@DRIVER_11N_SUPPORT @USB_SUPPORT +kmod-usb-core +mt7601u-firmware
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/mediatek/mt7601u/mt7601u.ko
  AUTOLOAD:=$(call AutoProbe,mt7601u)
endef


define KernelPackage/mwl8k
  $(call KernelPackage/mac80211/Default)
  TITLE:=Driver for Marvell TOPDOG 802.11 Wireless cards
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/mwl8k
  DEPENDS+= @PCI_SUPPORT +kmod-mac80211 +@DRIVER_11N_SUPPORT +mwl8k-firmware
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/mwl8k.ko
  AUTOLOAD:=$(call AutoProbe,mwl8k)
endef

define KernelPackage/mwl8k/description
 Kernel modules for Marvell TOPDOG 802.11 Wireless cards
endef


define KernelPackage/mwifiex-pcie
  $(call KernelPackage/mac80211/Default)
  TITLE:=Driver for Marvell 802.11n/802.11ac PCIe Wireless cards
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/mwifiex
  DEPENDS+= @PCI_SUPPORT +kmod-mac80211 +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT +mwifiex-pcie-firmware
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/mwifiex/mwifiex.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/mwifiex/mwifiex_pcie.ko
  AUTOLOAD:=$(call AutoProbe,mwifiex_pcie)
endef

define KernelPackage/mwifiex-pcie/description
 Kernel modules for Marvell 802.11n/802.11ac PCIe Wireless cards
endef

define KernelPackage/mwifiex-sdio
  $(call KernelPackage/mac80211/Default)
  TITLE:=Driver for Marvell 802.11n/802.11ac SDIO Wireless cards
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/mwifiex
  DEPENDS+= +kmod-mmc +kmod-mac80211 +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT +mwifiex-sdio-firmware
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/mwifiex/mwifiex.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/mwifiex/mwifiex_sdio.ko
  AUTOLOAD:=$(call AutoProbe,mwifiex_sdio)
endef

define KernelPackage/mwifiex-sdio/description
 Kernel modules for Marvell 802.11n/802.11ac SDIO Wireless cards
endef

define KernelPackage/p54/Default
  $(call KernelPackage/mac80211/Default)
  TITLE:=Prism54 Drivers
endef

define KernelPackage/p54/description
  Kernel module for Prism54 chipsets (mac80211)
endef

define KernelPackage/p54-common
  $(call KernelPackage/p54/Default)
  DEPENDS+= @PCI_SUPPORT||@USB_SUPPORT||@TARGET_omap24xx +kmod-mac80211 +kmod-lib-crc-ccitt
  TITLE+= (COMMON)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intersil/p54/p54common.ko
endef

define KernelPackage/p54-pci
  $(call KernelPackage/p54/Default)
  TITLE+= (PCI)
  DEPENDS+= @PCI_SUPPORT +kmod-p54-common +p54-pci-firmware
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intersil/p54/p54pci.ko
  AUTOLOAD:=$(call AutoProbe,p54pci)
endef

define KernelPackage/p54-usb
  $(call KernelPackage/p54/Default)
  TITLE+= (USB)
  DEPENDS+= @USB_SUPPORT +kmod-usb-core +kmod-p54-common +p54-usb-firmware
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intersil/p54/p54usb.ko
  AUTOLOAD:=$(call AutoProbe,p54usb)
endef

define KernelPackage/p54-spi
  $(call KernelPackage/p54/Default)
  TITLE+= (SPI)
  DEPENDS+= @TARGET_omap24xx +kmod-p54-common +p54-spi-firmware
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intersil/p54/p54spi.ko
  AUTOLOAD:=$(call AutoProbe,p54spi)
endef

define KernelPackage/rsi91x
  $(call KernelPackage/mac80211/Default)
  TITLE:=Redpine Signals Inc 91x WLAN driver support
  DEPENDS+= +kmod-mac80211 +rs9113-firmware
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/rsi/rsi_91x.ko
endef

define KernelPackage/rsi91x-usb
  $(call KernelPackage/mac80211/Default)
  TITLE:=Redpine Signals USB bus support
  DEPENDS+= +kmod-mac80211 +kmod-usb2 +kmod-rsi91x +rs9113-firmware
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/rsi/rsi_usb.ko
  AUTOLOAD:=$(call AutoProbe,rsi_usb)
endef

define KernelPackage/rsi91x-sdio
  $(call KernelPackage/mac80211/Default)
  TITLE:=Redpine Signals SDIO bus support
  DEPENDS+= +kmod-mac80211 +kmod-mmc +kmod-rsi91x +rs9113-firmware
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/rsi/rsi_sdio.ko
  AUTOLOAD:=$(call AutoProbe,rsi_sdio)
endef



define KernelPackage/rt2x00/Default
  $(call KernelPackage/mac80211/Default)
  TITLE:=Ralink Drivers for RT2x00 cards
endef

define KernelPackage/rt2x00-lib
$(call KernelPackage/rt2x00/Default)
  DEPENDS+= @(PCI_SUPPORT||USB_SUPPORT||TARGET_ramips) +kmod-mac80211 +kmod-lib-crc-itu-t
  TITLE+= (LIB)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt2x00lib.ko
  MENU:=1
endef

define KernelPackage/rt2x00-lib/config
  if PACKAGE_kmod-rt2x00-lib

	config PACKAGE_RT2X00_LIB_DEBUGFS
		bool "Enable rt2x00 debugfs support"
		depends on PACKAGE_MAC80211_DEBUGFS
		help
		  Enable creation of debugfs files for the rt2x00 drivers.
		  These debugfs files support both reading and writing of the
		  most important register types of the rt2x00 hardware.

	config PACKAGE_RT2X00_DEBUG
		bool "Enable rt2x00 debug output"
		help
		  Enable debugging output for all rt2x00 modules

  endif
endef

define KernelPackage/rt2x00-mmio
$(call KernelPackage/rt2x00/Default)
  DEPENDS+= @(PCI_SUPPORT||TARGET_ramips) +kmod-rt2x00-lib +kmod-eeprom-93cx6
  HIDDEN:=1
  TITLE+= (MMIO)
  FILES:= $(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt2x00mmio.ko
endef

define KernelPackage/rt2x00-pci
$(call KernelPackage/rt2x00/Default)
  DEPENDS+= @PCI_SUPPORT +kmod-rt2x00-mmio +kmod-rt2x00-lib
  HIDDEN:=1
  TITLE+= (PCI)
  FILES:= $(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt2x00pci.ko
  AUTOLOAD:=$(call AutoProbe,rt2x00pci)
endef

define KernelPackage/rt2x00-usb
$(call KernelPackage/rt2x00/Default)
  DEPENDS+= @USB_SUPPORT +kmod-rt2x00-lib +kmod-usb-core
  HIDDEN:=1
  TITLE+= (USB)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt2x00usb.ko
  AUTOLOAD:=$(call AutoProbe,rt2x00usb)
endef

define KernelPackage/rt2800-lib
$(call KernelPackage/rt2x00/Default)
  DEPENDS+= @(PCI_SUPPORT||USB_SUPPORT||TARGET_ramips) +kmod-rt2x00-lib +kmod-lib-crc-ccitt +@DRIVER_11N_SUPPORT
  HIDDEN:=1
  TITLE+= (rt2800 LIB)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt2800lib.ko
endef

define KernelPackage/rt2400-pci
$(call KernelPackage/rt2x00/Default)
  DEPENDS+= @PCI_SUPPORT +kmod-rt2x00-pci
  TITLE+= (RT2400 PCI)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt2400pci.ko
  AUTOLOAD:=$(call AutoProbe,rt2400pci)
endef

define KernelPackage/rt2500-pci
$(call KernelPackage/rt2x00/Default)
  DEPENDS+= @PCI_SUPPORT +kmod-rt2x00-pci
  TITLE+= (RT2500 PCI)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt2500pci.ko
  AUTOLOAD:=$(call AutoProbe,rt2500pci)
endef

define KernelPackage/rt2500-usb
$(call KernelPackage/rt2x00/Default)
  DEPENDS+= @USB_SUPPORT +kmod-rt2x00-usb
  TITLE+= (RT2500 USB)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt2500usb.ko
  AUTOLOAD:=$(call AutoProbe,rt2500usb)
endef

define KernelPackage/rt2800-mmio
$(call KernelPackage/rt2x00/Default)
  TITLE += (RT28xx/RT3xxx MMIO)
  DEPENDS += +kmod-rt2800-lib +kmod-rt2x00-mmio
  HIDDEN:=1
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt2800mmio.ko
endef

define KernelPackage/rt2800-soc
$(call KernelPackage/rt2x00/Default)
  DEPENDS += @(TARGET_ramips_rt288x||TARGET_ramips_rt305x||TARGET_ramips_rt3883||TARGET_ramips_mt7620) +kmod-rt2800-mmio +kmod-rt2800-lib
  TITLE += (RT28xx/RT3xxx SoC)
  FILES := \
	$(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt2x00soc.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt2800soc.ko
  AUTOLOAD:=$(call AutoProbe,rt2800soc)
endef

define KernelPackage/rt2800-pci
$(call KernelPackage/rt2x00/Default)
  DEPENDS+= @PCI_SUPPORT +kmod-rt2x00-pci +kmod-rt2800-lib +kmod-rt2800-mmio +rt2800-pci-firmware
  TITLE+= (RT2860 PCI)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt2800pci.ko
  AUTOLOAD:=$(call AutoProbe,rt2800pci)
endef

define KernelPackage/rt2800-usb
$(call KernelPackage/rt2x00/Default)
  DEPENDS+= @USB_SUPPORT +kmod-rt2x00-usb +kmod-rt2800-lib +kmod-lib-crc-ccitt +rt2800-usb-firmware
  TITLE+= (RT2870 USB)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt2800usb.ko
  AUTOLOAD:=$(call AutoProbe,rt2800usb)
endef


define KernelPackage/rt61-pci
$(call KernelPackage/rt2x00/Default)
  DEPENDS+= @PCI_SUPPORT +kmod-rt2x00-pci +rt61-pci-firmware
  TITLE+= (RT2x61 PCI)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt61pci.ko
  AUTOLOAD:=$(call AutoProbe,rt61pci)
endef

define KernelPackage/rt73-usb
  $(call KernelPackage/rt2x00/Default)
  DEPENDS+= @USB_SUPPORT +kmod-rt2x00-usb +rt73-usb-firmware
  TITLE+= (RT73 USB)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt73usb.ko
  AUTOLOAD:=$(call AutoProbe,rt73usb)
endef


define KernelPackage/rtl818x/Default
  $(call KernelPackage/mac80211/Default)
  TITLE:=Realtek Drivers for RTL818x devices
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/rtl8187
  DEPENDS+= +kmod-eeprom-93cx6 +kmod-mac80211
endef

define KernelPackage/rtl8180
  $(call KernelPackage/rtl818x/Default)
  DEPENDS+= @PCI_SUPPORT
  TITLE+= (RTL8180 PCI)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtl818x/rtl8180/rtl818x_pci.ko
  AUTOLOAD:=$(call AutoProbe,rtl818x_pci)
endef

define KernelPackage/rtl8187
$(call KernelPackage/rtl818x/Default)
  DEPENDS+= @USB_SUPPORT +kmod-usb-core
  TITLE+= (RTL8187 USB)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtl818x/rtl8187/rtl8187.ko
  AUTOLOAD:=$(call AutoProbe,rtl8187)
endef

define KernelPackage/rtlwifi/config
	config PACKAGE_RTLWIFI_DEBUG
		bool "Realtek wireless debugging"
		depends on PACKAGE_kmod-rtlwifi
		help
		  Say Y, if you want to debug realtek wireless drivers.

endef

define KernelPackage/rtlwifi
  $(call KernelPackage/mac80211/Default)
  TITLE:=Realtek common driver part
  DEPENDS+= @(PCI_SUPPORT||USB_SUPPORT) +kmod-mac80211 +@DRIVER_11N_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtlwifi/rtlwifi.ko
  HIDDEN:=1
endef

define KernelPackage/rtlwifi-pci
  $(call KernelPackage/mac80211/Default)
  TITLE:=Realtek common driver part (PCI support)
  DEPENDS+= @PCI_SUPPORT +kmod-rtlwifi
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtlwifi/rtl_pci.ko
  AUTOLOAD:=$(call AutoProbe,rtl_pci)
  HIDDEN:=1
endef

define KernelPackage/rtlwifi-btcoexist
  $(call KernelPackage/mac80211/Default)
  TITLE:=Realtek BT coexist support
  DEPENDS+= +kmod-rtlwifi
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtlwifi/btcoexist/btcoexist.ko
  AUTOLOAD:=$(call AutoProbe,btcoexist)
  HIDDEN:=1
endef

define KernelPackage/rtlwifi-usb
  $(call KernelPackage/mac80211/Default)
  TITLE:=Realtek common driver part (USB support)
  DEPENDS+= @USB_SUPPORT +kmod-usb-core +kmod-rtlwifi
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtlwifi/rtl_usb.ko
  AUTOLOAD:=$(call AutoProbe,rtl_usb)
  HIDDEN:=1
endef

define KernelPackage/rtl8192c-common
  $(call KernelPackage/mac80211/Default)
  TITLE:=Realtek RTL8192CE/RTL8192CU common support module
  DEPENDS+= +kmod-rtlwifi
  FILES:= $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtlwifi/rtl8192c/rtl8192c-common.ko
  HIDDEN:=1
endef

define KernelPackage/rtl8192ce
  $(call KernelPackage/mac80211/Default)
  TITLE:=Realtek RTL8192CE/RTL8188CE support
  DEPENDS+= +kmod-rtlwifi-pci +kmod-rtl8192c-common +rtl8192ce-firmware
  FILES:= $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtlwifi/rtl8192ce/rtl8192ce.ko
  AUTOLOAD:=$(call AutoProbe,rtl8192ce)
endef

define KernelPackage/rtl8192se
  $(call KernelPackage/mac80211/Default)
  TITLE:=Realtek RTL8192SE/RTL8191SE support
  DEPENDS+= +kmod-rtlwifi-pci +rtl8192se-firmware
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtlwifi/rtl8192se/rtl8192se.ko
  AUTOLOAD:=$(call AutoProbe,rtl8192se)
endef

define KernelPackage/rtl8192de
  $(call KernelPackage/mac80211/Default)
  TITLE:=Realtek RTL8192DE/RTL8188DE support
  DEPENDS+= +kmod-rtlwifi-pci +rtl8192de-firmware
  FILES:= $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtlwifi/rtl8192de/rtl8192de.ko
  AUTOLOAD:=$(call AutoProbe,rtl8192de)
endef

define KernelPackage/rtl8192cu
  $(call KernelPackage/mac80211/Default)
  TITLE:=Realtek RTL8192CU/RTL8188CU support
  DEPENDS+= +kmod-rtlwifi-usb +kmod-rtl8192c-common +rtl8192cu-firmware
  FILES:= $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtlwifi/rtl8192cu/rtl8192cu.ko
  AUTOLOAD:=$(call AutoProbe,rtl8192cu)
endef

define KernelPackage/rtl8821ae
  $(call KernelPackage/mac80211/Default)
  TITLE:=Realtek RTL8821AE support
  DEPENDS+= +kmod-rtlwifi-btcoexist +kmod-rtlwifi-pci +rtl8821ae-firmware
  FILES:= $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtlwifi/rtl8821ae/rtl8821ae.ko
  AUTOLOAD:=$(call AutoProbe,rtl8821ae)
endef

define KernelPackage/rtl8xxxu
  $(call KernelPackage/mac80211/Default)
  TITLE:=alternative Realtek RTL8XXXU support
  DEPENDS+= @USB_SUPPORT +kmod-usb-core +kmod-mac80211
  FILES:= $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtl8xxxu/rtl8xxxu.ko
  AUTOLOAD:=$(call AutoProbe,rtl8xxxu)
endef

define KernelPackage/rtl8xxxu/description
  This is an alternative driver for various Realtek RTL8XXX
  parts written to utilize the Linux mac80211 stack.
  The driver is known to work with a number of RTL8723AU,
  RL8188CU, RTL8188RU, RTL8191CU, and RTL8192CU devices

  This driver is under development and has a limited feature
  set. In particular it does not yet support 40MHz channels
  and power management. However it should have a smaller
  memory footprint than the vendor drivers and benetifs
  from the in kernel mac80211 stack.

  It can coexist with drivers from drivers/staging/rtl8723au,
  drivers/staging/rtl8192u, and drivers/net/wireless/rtlwifi,
  but you will need to control which module you wish to load.

  RTL8XXXU_UNTESTED is enabled
  This option enables detection of Realtek 8723/8188/8191/8192 WiFi
  USB devices which have not been tested directly by the driver
  author or reported to be working by third parties.

  Please report your results!
endef


define KernelPackage/wlcore
  $(call KernelPackage/mac80211/Default)
  TITLE:=TI common driver part
  DEPENDS+= +kmod-mmc +kmod-mac80211 +@DRIVER_11N_SUPPORT
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/ti/wlcore/wlcore.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/ti/wlcore/wlcore_sdio.ko
  AUTOLOAD:=$(call AutoProbe,wlcore wlcore_sdio)
endef

define KernelPackage/wlcore/description
 This module contains some common parts needed by TI Wireless drivers.
endef

define KernelPackage/wl12xx
  $(call KernelPackage/mac80211/Default)
  TITLE:=Driver for TI WL12xx
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/wl12xx
  DEPENDS+= +kmod-wlcore +wl12xx-firmware
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ti/wl12xx/wl12xx.ko
  AUTOLOAD:=$(call AutoProbe,wl12xx)
endef

define KernelPackage/wl12xx/description
 Kernel modules for TI WL12xx
endef

define KernelPackage/wl18xx
  $(call KernelPackage/mac80211/Default)
  TITLE:=Driver for TI WL18xx
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/wl18xx
  DEPENDS+= +kmod-wlcore +wl18xx-firmware
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ti/wl18xx/wl18xx.ko
  AUTOLOAD:=$(call AutoProbe,wl18xx)
endef

define KernelPackage/wl18xx/description
 Kernel modules for TI WL18xx
endef


ZD1211FW_NAME:=zd1211-firmware
ZD1211FW_VERSION:=1.4
define Download/zd1211rw
  FILE:=$(ZD1211FW_NAME)-$(ZD1211FW_VERSION).tar.bz2
  URL:=@SF/zd1211/
  HASH:=866308f6f59f7075f075d4959dff2ede47735c751251fecd1496df1ba4d338e1
endef
$(eval $(call Download,zd1211rw))

define KernelPackage/zd1211rw
  $(call KernelPackage/mac80211/Default)
  TITLE:=Zydas ZD1211 support
  DEPENDS+= @USB_SUPPORT +kmod-usb-core +kmod-mac80211
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/zydas/zd1211rw/zd1211rw.ko
  AUTOLOAD:=$(call AutoProbe,zd1211rw)
endef



config_package=$(if $(CONFIG_PACKAGE_kmod-$(1)),m)

config-y:= \
	WLAN \
	NL80211_TESTMODE \
	CFG80211_WEXT \
	CFG80211_CERTIFICATION_ONUS \
	MAC80211_RC_MINSTREL \
	MAC80211_RC_MINSTREL_HT \
	MAC80211_RC_MINSTREL_VHT \
	MAC80211_RC_DEFAULT_MINSTREL \
	WLAN_VENDOR_ADMTEK \
	WLAN_VENDOR_ATH \
	WLAN_VENDOR_ATMEL \
	WLAN_VENDOR_BROADCOM \
	WLAN_VENDOR_CISCO \
	WLAN_VENDOR_INTEL \
	WLAN_VENDOR_INTERSIL \
	WLAN_VENDOR_MARVELL \
	WLAN_VENDOR_MEDIATEK \
	WLAN_VENDOR_RALINK \
	WLAN_VENDOR_REALTEK \
	WLAN_VENDOR_RSI \
	WLAN_VENDOR_ST \
	WLAN_VENDOR_TI \
	WLAN_VENDOR_ZYDAS \

config-$(call config_package,cfg80211) += CFG80211

config-$(call config_package,mac80211) += MAC80211
config-$(CONFIG_PACKAGE_MAC80211_MESH) += MAC80211_MESH
ifdef CONFIG_PACKAGE_MAC80211_DEBUGFS
  config-y += \
	CFG80211_DEBUGFS \
	MAC80211_DEBUGFS \
	ATH9K_DEBUGFS \
	ATH9K_HTC_DEBUGFS \
	ATH10K_DEBUGFS \
	CARL9170_DEBUGFS \
	ATH5K_DEBUG \
	ATH6KL_DEBUG
endif

ifdef CONFIG_PACKAGE_MAC80211_TRACING
  config-y += \
	ATH10K_TRACING \
	ATH6KL_TRACING \
	ATH_TRACEPOINTS \
	WIL6210_TRACING \
	ATH5K_TRACER \
	IWLWIFI_DEVICE_TRACING
endif

config-$(call config_package,lib80211) += LIB80211 LIB80211_CRYPT_WEP LIB80211_CRYPT_CCMP LIB80211_CRYPT_TKIP

config-$(call config_package,airo) += AIRO

config-$(call config_package,ath) += ATH_CARDS ATH_COMMON ATH_REG_DYNAMIC_USER_REG_HINTS
config-$(CONFIG_PACKAGE_ATH_DEBUG) += ATH_DEBUG ATH10K_DEBUG ATH9K_STATION_STATISTICS
config-$(CONFIG_PACKAGE_ATH_DFS) += ATH9K_DFS_CERTIFIED ATH10K_DFS_CERTIFIED
config-$(CONFIG_PACKAGE_ATH_SPECTRAL) += ATH9K_COMMON_SPECTRAL ATH10K_SPECTRAL
config-$(CONFIG_PACKAGE_ATH_DYNACK) += ATH9K_DYNACK
config-$(call config_package,ath9k) += ATH9K
config-$(call config_package,ath9k-common) += ATH9K_COMMON
config-$(CONFIG_TARGET_ar71xx) += ATH9K_AHB
config-$(CONFIG_TARGET_ath79) += ATH9K_AHB
config-$(CONFIG_TARGET_ipq40xx) += ATH10K_AHB
config-$(CONFIG_PCI) += ATH9K_PCI
config-$(CONFIG_ATH_USER_REGD) += ATH_USER_REGD
config-$(CONFIG_ATH9K_SUPPORT_PCOEM) += ATH9K_PCOEM
config-$(CONFIG_ATH9K_TX99) += ATH9K_TX99
config-$(CONFIG_ATH9K_UBNTHSR) += ATH9K_UBNTHSR
config-$(CONFIG_ATH10K_LEDS) += ATH10K_LEDS
config-$(CONFIG_ATH10K_THERMAL) += ATH10K_THERMAL

config-$(call config_package,ath9k-htc) += ATH9K_HTC
config-$(call config_package,ath10k) += ATH10K ATH10K_PCI

config-$(call config_package,ath5k) += ATH5K
ifdef CONFIG_TARGET_ath25
  config-y += ATH5K_AHB
else
  config-y += ATH5K_PCI
endif

config-$(call config_package,ath6kl) += ATH6KL
config-$(call config_package,ath6kl-sdio) += ATH6KL_SDIO
config-$(call config_package,ath6kl-usb) += ATH6KL_USB

config-$(call config_package,carl9170) += CARL9170

config-$(call config_package,b43) += B43
config-$(CONFIG_PACKAGE_B43_BUSES_BCMA_AND_SSB) += B43_BUSES_BCMA_AND_SSB
config-$(CONFIG_PACKAGE_B43_BUSES_BCMA) += B43_BUSES_BCMA
config-$(CONFIG_PACKAGE_B43_BUSES_SSB) += B43_BUSES_SSB
config-$(CONFIG_PACKAGE_B43_PHY_G) += B43_PHY_G
config-$(CONFIG_PACKAGE_B43_PHY_N) += B43_PHY_N
config-$(CONFIG_PACKAGE_B43_PHY_LP) += B43_PHY_LP
config-$(CONFIG_PACKAGE_B43_PHY_HT) += B43_PHY_HT
config-$(CONFIG_PACKAGE_B43_PIO) += B43_PIO
config-$(CONFIG_PACKAGE_B43_DEBUG) += B43_DEBUG

config-$(call config_package,b43legacy) += B43LEGACY
config-y += B43LEGACY_DMA_MODE

config-$(call config_package,brcmutil) += BRCMUTIL
config-$(call config_package,brcmsmac) += BRCMSMAC
config-$(call config_package,brcmfmac) += BRCMFMAC
config-$(CONFIG_BRCMFMAC_SDIO) += BRCMFMAC_SDIO
config-$(CONFIG_BRCMFMAC_USB) += BRCMFMAC_USB
config-$(CONFIG_BRCMFMAC_PCIE) += BRCMFMAC_PCIE
config-$(CONFIG_PACKAGE_BRCM80211_DEBUG) += BRCMDBG

config-$(call config_package,mac80211-hwsim) += MAC80211_HWSIM
config-$(call config_package,mt7601u) += MT7601U
config-y += WL_MEDIATEK

config-$(call config_package,rt2x00-lib) += RT2X00 RT2X00_LIB
config-$(call config_package,rt2x00-pci) += RT2X00_LIB_PCI
config-$(call config_package,rt2x00-mmio) += RT2X00_LIB_MMIO
config-$(call config_package,rt2x00-usb) += RT2X00_LIB_USB
config-$(CONFIG_PACKAGE_RT2X00_LIB_DEBUGFS) += RT2X00_LIB_DEBUGFS
config-$(CONFIG_PACKAGE_RT2X00_DEBUG) += RT2X00_DEBUG

config-$(call config_package,rt2400-pci) += RT2400PCI
config-$(call config_package,rt2500-pci) += RT2500PCI
config-$(call config_package,rt2500-usb) += RT2500USB
config-$(call config_package,rt61-pci) += RT61PCI
config-$(call config_package,rt73-usb) += RT73USB

config-$(call config_package,rt2800-lib) += RT2800_LIB

config-$(call config_package,rt2800-soc) += RT2800SOC
config-$(call config_package,rt2800-pci) += RT2800PCI
config-y += RT2800PCI_RT33XX RT2800PCI_RT35XX RT2800PCI_RT53XX RT2800PCI_RT3290

config-$(call config_package,rt2800-usb) += RT2800USB
config-y += RT2800USB_RT33XX RT2800USB_RT35XX RT2800USB_RT3573 RT2800USB_RT53XX RT2800USB_RT55XX RT2800USB_UNKNOWN

config-$(call config_package,iwl-legacy) += IWLEGACY
config-$(call config_package,iwl3945) += IWL3945
config-$(call config_package,iwl4965) += IWL4965
config-$(call config_package,iwlwifi) += IWLWIFI IWLDVM IWLMVM
config-$(CONFIG_PACKAGE_IWLWIFI_DEBUG)+= IWLWIFI_DEBUG
config-$(CONFIG_PACKAGE_IWLWIFI_DEBUGFS)+= IWLWIFI_DEBUGFS

config-$(call config_package,libipw) += LIBIPW
config-$(call config_package,ipw2100) += IPW2100
config-$(call config_package,ipw2200) += IPW2200

config-$(call config_package,p54-common) += P54_COMMON
config-$(call config_package,p54-pci) += P54_PCI
config-$(call config_package,p54-usb) += P54_USB
config-$(call config_package,p54-spi) += P54_SPI

config-$(call config_package,hermes) += HERMES
config-$(call config_package,hermes-pci) += PCI_HERMES
config-$(call config_package,hermes-plx) += PLX_HERMES
config-$(call config_package,hermes-pcmcia) += PCMCIA_HERMES
config-y += HERMES_PRISM

config-$(call config_package,adm8211) += ADM8211
config-$(call config_package,libertas-sdio) += LIBERTAS LIBERTAS_SDIO
config-$(call config_package,libertas-usb) += LIBERTAS LIBERTAS_USB
config-$(call config_package,libertas-spi) += LIBERTAS LIBERTAS_SPI
config-$(call config_package,mwl8k) += MWL8K
config-$(call config_package,mwifiex-pcie) += MWIFIEX MWIFIEX_PCIE
config-$(call config_package,mwifiex-sdio) += MWIFIEX MWIFIEX_SDIO
config-$(call config_package,rtl8180) += RTL8180
config-$(call config_package,rtl8187) += RTL8187
config-$(call config_package,wlcore) += WLCORE WLCORE_SDIO
config-$(call config_package,wl12xx) += WL12XX
config-$(call config_package,wl18xx) += WL18XX
config-y += WL_TI WILINK_PLATFORM_DATA
config-$(call config_package,zd1211rw) += ZD1211RW
config-$(call config_package,rsi91x) += RSI_91X
config-$(call config_package,rsi91x-usb) += RSI_USB
config-$(call config_package,rsi91x-sdio) += RSI_SDIO

config-$(call config_package,rtlwifi) += RTL_CARDS RTLWIFI
config-$(call config_package,rtlwifi-pci) += RTLWIFI_PCI
config-$(call config_package,rtlwifi-btcoexist) += RTLBTCOEXIST
config-$(call config_package,rtlwifi-usb) += RTLWIFI_USB
config-$(call config_package,rtl8192c-common) += RTL8192C_COMMON
config-$(call config_package,rtl8192ce) += RTL8192CE
config-$(call config_package,rtl8192se) += RTL8192SE
config-$(call config_package,rtl8192de) += RTL8192DE
config-$(call config_package,rtl8192cu) += RTL8192CU
config-$(call config_package,rtl8821ae) += RTL8821AE
config-$(CONFIG_PACKAGE_RTLWIFI_DEBUG) += RTLWIFI_DEBUG

config-$(call config_package,rtl8xxxu) += RTL8XXXU
config-y += RTL8XXXU_UNTESTED

config-$(CONFIG_LEDS_TRIGGERS) += MAC80211_LEDS B43_LEDS B43LEGACY_LEDS

MAKE_OPTS:= -C "$(PKG_BUILD_DIR)" \
	$(KERNEL_MAKE_FLAGS) \
	EXTRA_CFLAGS="-I$(PKG_BUILD_DIR)/include $(IREMAP_CFLAGS)" \
	KLIB_BUILD="$(LINUX_DIR)" \
	MODPROBE=true \
	KLIB=$(TARGET_MODULES_DIR) \
	KERNEL_SUBLEVEL=$(lastword $(subst ., ,$(KERNEL_PATCHVER))) \
	KBUILD_LDFLAGS_MODULE_PREREQ=

define ConfigVars
$(subst $(space),,$(foreach opt,$(config-$(1)),CPTCFG_$(opt)=$(1)
))
endef

define mac80211_config
$(call ConfigVars,m)$(call ConfigVars,y)
endef
$(eval $(call shexport,mac80211_config))

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	$(PKG_UNPACK)
	$(Build/Patch)
	$(TAR) -C $(PKG_BUILD_DIR) -xzf $(DL_DIR)/$(IPW2100_NAME)-$(IPW2100_VERSION).tgz
	$(TAR) -C $(PKG_BUILD_DIR) -xzf $(DL_DIR)/$(IPW2200_NAME)-$(IPW2200_VERSION).tgz
	$(TAR) -C $(PKG_BUILD_DIR) -xjf $(DL_DIR)/$(ZD1211FW_NAME)-$(ZD1211FW_VERSION).tar.bz2
	rm -rf \
		$(PKG_BUILD_DIR)/include/linux/ssb \
		$(PKG_BUILD_DIR)/include/linux/bcma \
		$(PKG_BUILD_DIR)/include/net/bluetooth

	rm -f \
		$(PKG_BUILD_DIR)/include/linux/cordic.h \
		$(PKG_BUILD_DIR)/include/linux/crc8.h \
		$(PKG_BUILD_DIR)/include/linux/eeprom_93cx6.h \
		$(PKG_BUILD_DIR)/include/linux/wl12xx.h \
		$(PKG_BUILD_DIR)/include/linux/spi/libertas_spi.h \
		$(PKG_BUILD_DIR)/include/net/ieee80211.h \
		$(PKG_BUILD_DIR)/backport-include/linux/bcm47xx_nvram.h

	echo 'compat-wireless-$(PKG_VERSION)-$(PKG_RELEASE)-$(REVISION)' > $(PKG_BUILD_DIR)/compat_version
endef

ifneq ($(CONFIG_PACKAGE_kmod-cfg80211)$(CONFIG_PACKAGE_kmod-lib80211),)
 define Build/Compile/kmod
	rm -rf $(PKG_BUILD_DIR)/modules
	+$(MAKE) $(PKG_JOBS) $(MAKE_OPTS) modules
 endef
endif

#do not Build/Configure for EXTERNAL KERNEL
ifeq ($(strip $(CONFIG_EXTERNAL_KERNEL_TREE)),"")
  ifeq ($(strip $(CONFIG_KERNEL_GIT_CLONE_URI)),"")
    define Build/Configure
	  cmp $(PKG_BUILD_DIR)/include/linux/ath9k_platform.h $(LINUX_DIR)/include/linux/ath9k_platform.h
	  cmp $(PKG_BUILD_DIR)/include/linux/ath5k_platform.h $(LINUX_DIR)/include/linux/ath5k_platform.h
	  cmp $(PKG_BUILD_DIR)/include/linux/rt2x00_platform.h $(LINUX_DIR)/include/linux/rt2x00_platform.h
    endef
  endif
endif

define Build/Patch
	$(if $(QUILT),rm -rf $(PKG_BUILD_DIR)/patches; mkdir -p $(PKG_BUILD_DIR)/patches)
	$(call PatchDir,$(PKG_BUILD_DIR),$(PATCH_DIR)/build,build/)
	$(call PatchDir,$(PKG_BUILD_DIR),$(PATCH_DIR)/subsys,subsys/)
	$(call PatchDir,$(PKG_BUILD_DIR),$(PATCH_DIR)/ath,ath/)
	$(call PatchDir,$(PKG_BUILD_DIR),$(PATCH_DIR)/rt2x00,rt2x00/)
	$(call PatchDir,$(PKG_BUILD_DIR),$(PATCH_DIR)/mwl,mwl/)
	$(call PatchDir,$(PKG_BUILD_DIR),$(PATCH_DIR)/brcm,brcm/)
	$(if $(QUILT),touch $(PKG_BUILD_DIR)/.quilt_used)
endef

define Quilt/Refresh/Package
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(PATCH_DIR)/build,build/)
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(PATCH_DIR)/subsys,subsys/)
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(PATCH_DIR)/ath,ath/)
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(PATCH_DIR)/rt2x00,rt2x00/)
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(PATCH_DIR)/mwl,mwl/)
	$(call Quilt/RefreshDir,$(PKG_BUILD_DIR),$(PATCH_DIR)/brcm,brcm/)
endef

define Build/Compile
	$(SH_FUNC) var2file "$(call shvar,mac80211_config)" $(PKG_BUILD_DIR)/.config
	$(MAKE) $(MAKE_OPTS) allnoconfig
	$(call Build/Compile/kmod)
endef

define Build/InstallDev
	mkdir -p \
		$(1)/usr/include/mac80211 \
		$(1)/usr/include/mac80211-backport \
		$(1)/usr/include/mac80211/ath \
		$(1)/usr/include/net/mac80211
	$(CP) $(PKG_BUILD_DIR)/net/mac80211/*.h $(PKG_BUILD_DIR)/include/* $(1)/usr/include/mac80211/
	$(CP) $(PKG_BUILD_DIR)/backport-include/* $(1)/usr/include/mac80211-backport/
	$(CP) $(PKG_BUILD_DIR)/net/mac80211/rate.h $(1)/usr/include/net/mac80211/
	$(CP) $(PKG_BUILD_DIR)/drivers/net/wireless/ath/*.h $(1)/usr/include/mac80211/ath/
	rm -f $(1)/usr/include/mac80211-backport/linux/module.h
endef


define KernelPackage/b43/install
	rm -rf $(1)/lib/firmware/
ifeq ($(CONFIG_B43_OPENFIRMWARE),y)
	tar xzf "$(DL_DIR)/$(PKG_B43_FWV4_SOURCE)" -C "$(PKG_BUILD_DIR)"
else
	tar xjf "$(DL_DIR)/$(PKG_B43_FWV4_SOURCE)" -C "$(PKG_BUILD_DIR)"
endif
	$(INSTALL_DIR) $(1)/lib/firmware/
ifeq ($(CONFIG_B43_OPENFIRMWARE),y)
	$(MAKE) -C "$(PKG_BUILD_DIR)/$(PKG_B43_FWV4_OBJECT)/"
	$(INSTALL_DIR) $(1)/lib/firmware/b43-open/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(PKG_B43_FWV4_OBJECT)/ucode5.fw $(1)/lib/firmware/b43-open/ucode5.fw
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(PKG_B43_FWV4_OBJECT)/b0g0bsinitvals5.fw $(1)/lib/firmware/b43-open/b0g0bsinitvals5.fw
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(PKG_B43_FWV4_OBJECT)/b0g0initvals5.fw $(1)/lib/firmware/b43-open/b0g0initvals5.fw
else
	b43-fwcutter -w $(1)/lib/firmware/ $(PKG_BUILD_DIR)/$(PKG_B43_FWV4_OBJECT)
endif
ifneq ($(CONFIG_B43_FW_SQUASH),)
	b43-fwsquash.py "$(CONFIG_B43_FW_SQUASH_PHYTYPES)" "$(CONFIG_B43_FW_SQUASH_COREREVS)" "$(1)/lib/firmware/b43"
endif
endef

define KernelPackage/brcmsmac/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
ifeq ($(CONFIG_BRCMSMAC_USE_FW_FROM_WL),y)
	tar xjf "$(DL_DIR)/$(PKG_BRCMSMAC_FW_SOURCE)" -C "$(PKG_BUILD_DIR)"
	b43-fwcutter --brcmsmac -w $(1)/lib/firmware/ $(PKG_BUILD_DIR)/$(PKG_BRCMSMAC_FW_OBJECT)
endif
endef

define KernelPackage/cfg80211/install
	$(INSTALL_DIR) $(1)/lib/wifi $(1)/lib/netifd/wireless
	$(INSTALL_DATA) ./files/lib/wifi/mac80211.sh $(1)/lib/wifi
	$(INSTALL_BIN) ./files/lib/netifd/wireless/mac80211.sh $(1)/lib/netifd/wireless
	$(INSTALL_DIR) $(1)/etc/hotplug.d/ieee80211
	$(INSTALL_DATA) ./files/mac80211.hotplug $(1)/etc/hotplug.d/ieee80211/10-wifi-detect
endef

define KernelPackage/ipw2100/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/ipw2100-$(IPW2100_VERSION)*.fw $(1)/lib/firmware
endef

define KernelPackage/ipw2200/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(IPW2200_NAME)-$(IPW2200_VERSION)/ipw2200*.fw $(1)/lib/firmware
endef

define KernelPackage/zd1211rw/install
	$(INSTALL_DIR) $(1)/lib/firmware/zd1211
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(ZD1211FW_NAME)/zd1211* $(1)/lib/firmware/zd1211
endef


$(eval $(call KernelPackage,adm8211))
$(eval $(call KernelPackage,airo))
$(eval $(call KernelPackage,ath))
$(eval $(call KernelPackage,ath10k))
$(eval $(call KernelPackage,ath5k))
$(eval $(call KernelPackage,ath6kl))
$(eval $(call KernelPackage,ath6kl-sdio))
$(eval $(call KernelPackage,ath6kl-usb))
$(eval $(call KernelPackage,ath9k))
$(eval $(call KernelPackage,ath9k-common))
$(eval $(call KernelPackage,ath9k-htc))
$(eval $(call KernelPackage,b43))
$(eval $(call KernelPackage,b43legacy))
$(eval $(call KernelPackage,brcmsmac))
$(eval $(call KernelPackage,brcmfmac))
$(eval $(call KernelPackage,brcmutil))
$(eval $(call KernelPackage,carl9170))
$(eval $(call KernelPackage,cfg80211))
$(eval $(call KernelPackage,hermes))
$(eval $(call KernelPackage,hermes-pci))
$(eval $(call KernelPackage,hermes-plx))
$(eval $(call KernelPackage,hermes-pcmcia))
$(eval $(call KernelPackage,iwlwifi))
$(eval $(call KernelPackage,iwl-legacy))
$(eval $(call KernelPackage,iwl4965))
$(eval $(call KernelPackage,iwl3945))
$(eval $(call KernelPackage,lib80211))
$(eval $(call KernelPackage,libertas-usb))
$(eval $(call KernelPackage,libertas-sdio))
$(eval $(call KernelPackage,libertas-spi))
$(eval $(call KernelPackage,libipw))
$(eval $(call KernelPackage,ipw2100))
$(eval $(call KernelPackage,ipw2200))
$(eval $(call KernelPackage,mac80211))
$(eval $(call KernelPackage,mac80211-hwsim))
$(eval $(call KernelPackage,mt7601u))
$(eval $(call KernelPackage,mwl8k))
$(eval $(call KernelPackage,mwifiex-pcie))
$(eval $(call KernelPackage,mwifiex-sdio))
$(eval $(call KernelPackage,p54-common))
$(eval $(call KernelPackage,p54-pci))
$(eval $(call KernelPackage,p54-usb))
$(eval $(call KernelPackage,p54-spi))
$(eval $(call KernelPackage,rsi91x))
$(eval $(call KernelPackage,rsi91x-usb))
$(eval $(call KernelPackage,rsi91x-sdio))
$(eval $(call KernelPackage,rt2x00-lib))
$(eval $(call KernelPackage,rt2x00-mmio))
$(eval $(call KernelPackage,rt2x00-pci))
$(eval $(call KernelPackage,rt2x00-usb))
$(eval $(call KernelPackage,rt2800-lib))
$(eval $(call KernelPackage,rt2400-pci))
$(eval $(call KernelPackage,rt2500-pci))
$(eval $(call KernelPackage,rt2500-usb))
$(eval $(call KernelPackage,rt2800-mmio))
$(eval $(call KernelPackage,rt2800-soc))
$(eval $(call KernelPackage,rt2800-pci))
$(eval $(call KernelPackage,rt2800-usb))
$(eval $(call KernelPackage,rt61-pci))
$(eval $(call KernelPackage,rt73-usb))
$(eval $(call KernelPackage,rtl8180))
$(eval $(call KernelPackage,rtl8187))
$(eval $(call KernelPackage,rtlwifi))
$(eval $(call KernelPackage,rtlwifi-pci))
$(eval $(call KernelPackage,rtlwifi-btcoexist))
$(eval $(call KernelPackage,rtlwifi-usb))
$(eval $(call KernelPackage,rtl8192c-common))
$(eval $(call KernelPackage,rtl8192ce))
$(eval $(call KernelPackage,rtl8192se))
$(eval $(call KernelPackage,rtl8192de))
$(eval $(call KernelPackage,rtl8192cu))
$(eval $(call KernelPackage,rtl8821ae))
$(eval $(call KernelPackage,rtl8xxxu))
$(eval $(call KernelPackage,wlcore))
$(eval $(call KernelPackage,wl12xx))
$(eval $(call KernelPackage,wl18xx))
$(eval $(call KernelPackage,zd1211rw))
