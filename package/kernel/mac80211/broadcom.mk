PKG_DRIVERS += \
	b43 b43legacy brcmsmac brcmfmac brcmutil

PKG_CONFIG_DEPENDS += \
	CONFIG_PACKAGE_B43_DEBUG \
	CONFIG_PACKAGE_B43_PIO \
	CONFIG_PACKAGE_B43_PHY_G \
	CONFIG_PACKAGE_B43_PHY_N \
	CONFIG_PACKAGE_B43_PHY_LP \
	CONFIG_PACKAGE_B43_PHY_HT \
	CONFIG_PACKAGE_B43_BUSES_BCMA_AND_SSB \
	CONFIG_PACKAGE_B43_BUSES_BCMA \
	CONFIG_PACKAGE_B43_BUSES_SSB \
	CONFIG_PACKAGE_BRCM80211_DEBUG

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

config-$(CONFIG_LEDS_TRIGGERS) += B43_LEDS B43LEGACY_LEDS

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
  PKG_B43_FWV4_SOURCE_URL:=@OPENWRT
  PKG_B43_FWV4_HASH:=26a8c370f48fc129d0731cfd751c36cae1419b0bc8ca35781126744e60eae009
else
ifneq ($(CONFIG_B43_FW_4_178),)
  PKG_B43_FWV4_NAME:=broadcom-wl
  PKG_B43_FWV4_VERSION:=4.178.10.4
  PKG_B43_FWV4_OBJECT:=$(PKG_B43_FWV4_NAME)-$(PKG_B43_FWV4_VERSION)/linux/wl_apsta.o
  PKG_B43_FWV4_SOURCE:=$(PKG_B43_FWV4_NAME)-$(PKG_B43_FWV4_VERSION).tar.bz2
  PKG_B43_FWV4_SOURCE_URL:=@OPENWRT
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
  PKG_B43_FWV4_SOURCE_URL:=@OPENWRT
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
	@PCI_SUPPORT +kmod-mac80211 +kmod-lib-cordic \
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
	depends on !TARGET_bcm47xx && !TARGET_bcm63xx
	default PACKAGE_kmod-b43 if PACKAGE_B43_BUSES_BCMA_AND_SSB
	default PACKAGE_kmod-b43 if PACKAGE_B43_BUSES_SSB

config PACKAGE_B43_USE_BCMA
	select PACKAGE_kmod-bcma
	tristate
	depends on !TARGET_bcm47xx && !TARGET_bcm53xx
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
		default "5,6,7,8,9,10,11,13,15" if TARGET_bcm47xx_legacy
		default "16,28,29,30" if TARGET_bcm47xx_mips74k
		default "5,6,7,8,9,10,11,13,15,16,28,29,30"
		help
		  This is a comma separated list of core revision numbers.

		  Example (keep files for rev5 only):
		    5

		  Example (keep files for rev5 and rev11):
		    5,11

	config B43_FW_SQUASH_PHYTYPES
		string "PHY types to include"
		depends on B43_FW_SQUASH
		default "G,N,LP" if TARGET_bcm47xx_legacy
		default "N,HT" if TARGET_bcm47xx_mips74k
		default "G,N,LP,HT"
		help
		  This is a comma separated list of PHY types:
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
		depends on !TARGET_bcm47xx_legacy && !TARGET_bcm47xx_mips74k && !TARGET_bcm53xx
		bool "BCMA and SSB"

	config PACKAGE_B43_BUSES_BCMA
		depends on !TARGET_bcm47xx_legacy
		bool "BCMA only"

	config PACKAGE_B43_BUSES_SSB
		depends on !TARGET_bcm47xx_mips74k && !TARGET_bcm53xx
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
		default n if TARGET_bcm47xx_mips74k
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
		default n if TARGET_bcm47xx_mips74k
		default y
		help
		  Enable support for LP-PHY. This includes support for the following devices:
		  PCI: BCM4312
		  SoC: BCM5354

		  If unsure, say Y.

	config PACKAGE_B43_PHY_HT
		bool "Enable support for HT-PHYs"
		default n if TARGET_bcm47xx_legacy
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
  DEPENDS+= +kmod-mac80211 +!(TARGET_bcm47xx||TARGET_bcm63xx):kmod-ssb @!TARGET_bcm47xx_mips74k +b43legacy-firmware
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
  DEPENDS+= +kmod-mac80211 +@DRIVER_11N_SUPPORT +!TARGET_bcm47xx:kmod-bcma +kmod-lib-cordic +kmod-lib-crc8 +kmod-brcmutil +!BRCMSMAC_USE_FW_FROM_WL:brcmsmac-firmware
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
  DEPENDS+= @USB_SUPPORT +kmod-cfg80211 +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT \
  	+kmod-brcmutil +BRCMFMAC_SDIO:kmod-mmc @!TARGET_uml \
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
		default y if TARGET_bcm27xx
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
