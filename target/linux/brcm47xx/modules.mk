#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/bgmac
  TITLE:=Broadcom bgmac driver
  KCONFIG:=CONFIG_BGMAC CONFIG_BGMAC_BCMA
  DEPENDS:=@TARGET_brcm47xx @!TARGET_brcm47xx_legacy
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  FILES:= \
	$(LINUX_DIR)/drivers/net/ethernet/broadcom/bgmac-bcma.ko \
	$(LINUX_DIR)/drivers/net/ethernet/broadcom/bgmac-bcma-mdio.ko \
	$(LINUX_DIR)/drivers/net/ethernet/broadcom/bgmac.ko
  AUTOLOAD:=$(call AutoProbe,bgmac-bcma)
endef

define KernelPackage/bgmac/description
 Kernel modules for Broadcom bgmac Ethernet adapters.
endef

$(eval $(call KernelPackage,bgmac))
