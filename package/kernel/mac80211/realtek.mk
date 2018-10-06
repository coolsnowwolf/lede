PKG_DRIVERS += \
	rtl8180 rtl8187 \
	rtlwifi rtlwifi-pci rtlwifi-btcoexist rtlwifi-usb rtl8192c-common \
	rtl8192ce rtl8192se rtl8192de rtl8192cu rtl8821ae \
	rtl8xxxu

config-$(call config_package,rtl8180) += RTL8180
config-$(call config_package,rtl8187) += RTL8187

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
