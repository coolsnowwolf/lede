#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

I2C_LANTIQ_MODULES:= \
  CONFIG_I2C_LANTIQ:drivers/i2c/busses/i2c-lantiq

define KernelPackage/i2c-lantiq
  TITLE:=Lantiq I2C controller
  $(call i2c_defaults,$(I2C_LANTIQ_MODULES),52)
  DEPENDS:=kmod-i2c-core @TARGET_lantiq_falcon
endef

define KernelPackage/i2c-lantiq/description
  Kernel support for the Lantiq/Falcon I2C controller
endef

$(eval $(call KernelPackage,i2c-lantiq))

