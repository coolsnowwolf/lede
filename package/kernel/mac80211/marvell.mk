PKG_DRIVERS += \
	libertas-sdio libertas-usb libertas-spi \
	mwl8k mwifiex-pcie mwifiex-sdio

config-$(call config_package,libertas-sdio) += LIBERTAS LIBERTAS_SDIO
config-$(call config_package,libertas-usb) += LIBERTAS LIBERTAS_USB
config-$(call config_package,libertas-spi) += LIBERTAS LIBERTAS_SPI
config-$(call config_package,mwl8k) += MWL8K
config-$(call config_package,mwifiex-pcie) += MWIFIEX MWIFIEX_PCIE
config-$(call config_package,mwifiex-sdio) += MWIFIEX MWIFIEX_SDIO

define KernelPackage/libertas-usb
  $(call KernelPackage/mac80211/Default)
  DEPENDS+= @USB_SUPPORT +kmod-cfg80211 +kmod-usb-core +kmod-lib80211 +@DRIVER_WEXT_SUPPORT +libertas-usb-firmware
  TITLE:=Marvell 88W8015 Wireless Driver
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/libertas/libertas.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/libertas/usb8xxx.ko
  AUTOLOAD:=$(call AutoProbe,libertas usb8xxx)
endef

define KernelPackage/libertas-sdio
  $(call KernelPackage/mac80211/Default)
  DEPENDS+= +kmod-cfg80211 +kmod-lib80211 +kmod-mmc +@DRIVER_WEXT_SUPPORT @!TARGET_uml +libertas-sdio-firmware
  TITLE:=Marvell 88W8686 Wireless Driver
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/libertas/libertas.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/libertas/libertas_sdio.ko
  AUTOLOAD:=$(call AutoProbe,libertas libertas_sdio)
endef

define KernelPackage/libertas-spi
  $(call KernelPackage/mac80211/Default)
  SUBMENU:=Wireless Drivers
  DEPENDS+= +kmod-cfg80211 +kmod-lib80211 +@DRIVER_WEXT_SUPPORT @!TARGET_uml +libertas-spi-firmware
  KCONFIG := \
	CONFIG_SPI=y \
	CONFIG_SPI_MASTER=y
  TITLE:=Marvell 88W8686 SPI Wireless Driver
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/libertas/libertas.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/libertas/libertas_spi.ko
  AUTOLOAD:=$(call AutoProbe,libertas libertas_spi)
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

