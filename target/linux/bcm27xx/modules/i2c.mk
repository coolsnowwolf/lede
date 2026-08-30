# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2019 OpenWrt.org

I2C_BCM2835_MODULES:=\
  CONFIG_I2C_BCM2835:drivers/i2c/busses/i2c-bcm2835

define KernelPackage/i2c-bcm2835
  $(call i2c_defaults,$(I2C_BCM2835_MODULES),59)
  TITLE:=Broadcom BCM2835 I2C master controller driver
  DEPENDS:=@TARGET_bcm27xx +kmod-i2c-core
endef

define KernelPackage/i2c-bcm2835/description
  This package contains the Broadcom 2835 I2C master controller driver
endef

$(eval $(call KernelPackage,i2c-bcm2835))


I2C_BRCMSTB_MODULES:=\
  CONFIG_I2C_BRCMSTB:drivers/i2c/busses/i2c-brcmstb

define KernelPackage/i2c-brcmstb
  $(call i2c_defaults,$(I2C_BRCMSTB_MODULES),59)
  TITLE:=Broadcom BRCMSTB I2C master controller driver
  DEPENDS:=@TARGET_bcm27xx +kmod-i2c-core
endef

define KernelPackage/i2c-brcmstb/description
  This package contains the BRCM Settop/DSL I2C master controller driver
endef

$(eval $(call KernelPackage,i2c-brcmstb))


define KernelPackage/bcm27xx-i2c
  SUBMENU:=$(I2C_MENU)
  TITLE:=I2C support for bcm27xx boards
  DEPENDS:=@TARGET_bcm27xx +kmod-i2c-bcm2835 \
	+TARGET_bcm27xx_bcm2711:kmod-i2c-brcmstb \
	+TARGET_bcm27xx_bcm2712:kmod-i2c-brcmstb \
	+TARGET_bcm27xx_bcm2712:kmod-i2c-designware-platform
endef

define KernelPackage/bcm27xx-i2c/description
 Pulls in the correct I2C kernel modules for whichever bcm27xx
 board you're building for, without needing to know which specific
 driver combination your particular Pi/CM generation requires.
endef

$(eval $(call KernelPackage,bcm27xx-i2c))
