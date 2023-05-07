PKG_DRIVERS += \
	mwl8k mwifiex-pcie mwifiex-sdio

config-$(call config_package,mwl8k) += MWL8K
config-$(call config_package,mwifiex-pcie) += MWIFIEX MWIFIEX_PCIE
config-$(call config_package,mwifiex-sdio) += MWIFIEX MWIFIEX_SDIO

define KernelPackage/mwl8k
  $(call KernelPackage/mac80211/Default)
  TITLE:=Driver for Marvell TOPDOG 802.11 Wireless cards
  URL:=https://wireless.wiki.kernel.org/en/users/drivers/mwl8k
  DEPENDS+= @PCI_SUPPORT +kmod-mac80211 +mwl8k-firmware
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
  DEPENDS+= @PCI_SUPPORT +kmod-mac80211 +@DRIVER_11AC_SUPPORT +mwifiex-pcie-firmware
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
  DEPENDS+= +kmod-mmc +kmod-mac80211 +@DRIVER_11AC_SUPPORT +mwifiex-sdio-firmware
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/mwifiex/mwifiex.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/marvell/mwifiex/mwifiex_sdio.ko
  AUTOLOAD:=$(call AutoProbe,mwifiex_sdio)
endef

define KernelPackage/mwifiex-sdio/description
 Kernel modules for Marvell 802.11n/802.11ac SDIO Wireless cards
endef

