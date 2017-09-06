#
# Copyright (C) 2016 Rafał Miłecki <rafal@milecki.pl>
#
# This is free software, licensed under the GNU General Public License v2.
#

define KernelPackage/phy-bcm-ns-usb2
  TITLE:=Broadcom Northstar USB 2.0 PHY Driver
  KCONFIG:=CONFIG_PHY_BCM_NS_USB2
  DEPENDS:=@TARGET_bcm53xx
  SUBMENU:=$(USB_MENU)
  FILES:=$(LINUX_DIR)/drivers/phy/phy-bcm-ns-usb2.ko
  AUTOLOAD:=$(call AutoLoad,45,phy-bcm-ns-usb2,1)
endef

define KernelPackage/phy-bcm-ns-usb2/description
  Support for Broadcom USB 2.0 PHY connected to the USB controller on Northstar
  family.
endef

$(eval $(call KernelPackage,phy-bcm-ns-usb2))

define KernelPackage/phy-bcm-ns-usb3
  TITLE:=Broadcom Northstar USB 3.0 PHY Driver
  KCONFIG:=CONFIG_PHY_BCM_NS_USB3
  DEPENDS:=@TARGET_bcm53xx
  SUBMENU:=$(USB_MENU)
  FILES:=$(LINUX_DIR)/drivers/phy/phy-bcm-ns-usb3.ko
  AUTOLOAD:=$(call AutoLoad,45,phy-bcm-ns-usb3,1)
endef

define KernelPackage/phy-bcm-ns-usb3/description
  Support for Broadcom USB 3.0 PHY connected to the USB controller on Northstar
  family.
endef

$(eval $(call KernelPackage,phy-bcm-ns-usb3))
