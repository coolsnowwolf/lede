PKG_DRIVERS += \
	rt2x00-lib rt2x00-pci rt2x00-usb rt2x00-mmio \
	rt2800-lib rt2800-mmio rt2800-pci rt2800-soc rt2800-usb

PKG_CONFIG_DEPENDS += \
	CONFIG_PACKAGE_RT2X00_LIB_DEBUGFS \
	CONFIG_PACKAGE_RT2X00_DEBUG

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

define KernelPackage/rt2x00/Default
  $(call KernelPackage/mac80211/Default)
  TITLE:=Ralink Drivers for RT2x00 cards
endef

define KernelPackage/rt2x00-lib
$(call KernelPackage/rt2x00/Default)
  DEPENDS+= @(PCI_SUPPORT||USB_SUPPORT||TARGET_ramips) +kmod-mac80211
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
  DEPENDS+= @(PCI_SUPPORT||TARGET_ramips) +kmod-rt2x00-lib
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
  DEPENDS+= @(PCI_SUPPORT||USB_SUPPORT||TARGET_ramips) +kmod-rt2x00-lib +kmod-lib-crc-ccitt
  HIDDEN:=1
  TITLE+= (rt2800 LIB)
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/ralink/rt2x00/rt2800lib.ko
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
  DEPENDS+= @PCI_SUPPORT +kmod-rt2x00-pci +kmod-rt2800-lib +kmod-rt2800-mmio +kmod-eeprom-93cx6 +rt2800-pci-firmware
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


