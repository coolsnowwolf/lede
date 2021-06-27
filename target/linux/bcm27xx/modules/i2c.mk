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
