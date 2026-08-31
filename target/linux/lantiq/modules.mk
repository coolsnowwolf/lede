# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2010 OpenWrt.org

I2C_LANTIQ_MODULES:= \
  CONFIG_I2C_LANTIQ:drivers/i2c/busses/i2c-lantiq

define KernelPackage/i2c-lantiq
  TITLE:=Lantiq I2C controller
  $(call i2c_defaults,$(I2C_LANTIQ_MODULES),52)
  DEPENDS:=+kmod-i2c-core @TARGET_lantiq_falcon
endef

define KernelPackage/i2c-lantiq/description
  Kernel support for the Lantiq/Falcon I2C controller
endef

$(eval $(call KernelPackage,i2c-lantiq))

define KernelPackage/dsa-gswip
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Lantiq xRx200/xRx300/xRx330 switch support
  DEPENDS:=@TARGET_lantiq_xrx200||TARGET_lantiq_xrx200_legacy
  KCONFIG:= \
	CONFIG_NET_DSA_LANTIQ_GSWIP \
	CONFIG_NET_DSA_TAG_GSWIP
  FILES:= \
	$(LINUX_DIR)/drivers/net/dsa/lantiq/lantiq_gswip.ko@ge6.18 \
	$(LINUX_DIR)/drivers/net/dsa/lantiq_gswip.ko@lt6.18 \
	$(LINUX_DIR)/net/dsa/tag_gswip.ko
  AUTOLOAD:=$(call AutoLoad,41,tag_gswip lantiq_gswip,1)
endef

define KernelPackage/dsa-gswip/description
 Lantiq xRx200/xRx300/xRx330 switch support
endef

$(eval $(call KernelPackage,dsa-gswip))
