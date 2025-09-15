# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2016 Rafał Miłecki <rafal@milecki.pl>

define KernelPackage/phy-bcm-ns-usb2
  TITLE:=Broadcom Northstar USB 2.0 PHY Driver
  KCONFIG:=CONFIG_PHY_BCM_NS_USB2
  DEPENDS:=@TARGET_bcm53xx
  SUBMENU:=$(USB_MENU)
  FILES:=$(LINUX_DIR)/drivers/phy/broadcom/phy-bcm-ns-usb2.ko
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
  FILES:=$(LINUX_DIR)/drivers/phy/broadcom/phy-bcm-ns-usb3.ko
  AUTOLOAD:=$(call AutoLoad,45,phy-bcm-ns-usb3,1)
endef

define KernelPackage/phy-bcm-ns-usb3/description
  Support for Broadcom USB 3.0 PHY connected to the USB controller on Northstar
  family.
endef

$(eval $(call KernelPackage,phy-bcm-ns-usb3))

define KernelPackage/i2c-bcm-iproc
  TITLE:=Broadcom iProc I2C controller
  KCONFIG:= \
	CONFIG_I2C_BCM_IPROC \
	CONFIG_I2C_SLAVE_TESTUNIT=n
  DEPENDS:=@TARGET_bcm53xx +kmod-i2c-core
  SUBMENU:=$(I2C_MENU)
  FILES:=$(LINUX_DIR)/drivers/i2c/busses/i2c-bcm-iproc.ko
  AUTOLOAD:=$(call AutoLoad,59,i2c-bcm-iproc,1)
endef

define KernelPackage/i2c-bcm-iproc/description
 Kernel module for the Broadcom iProc I2C controller.
endef

$(eval $(call KernelPackage,i2c-bcm-iproc))
