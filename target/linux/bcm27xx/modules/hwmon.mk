# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2019 OpenWrt.org

define KernelPackage/hwmon-raspberrypi
  TITLE:=Raspberry Pi voltage monitor
  KCONFIG:=CONFIG_SENSORS_RASPBERRYPI_HWMON
  FILES:=$(LINUX_DIR)/drivers/hwmon/raspberrypi-hwmon.ko
  AUTOLOAD:=$(call AutoLoad,60,raspberrypi-hwmon)
  $(call AddDepends/hwmon,@TARGET_bcm27xx)
endef

define KernelPackage/hwmon-raspberrypi/description
  Kernel module for voltage sensor on the Raspberry Pi
endef

$(eval $(call KernelPackage,hwmon-raspberrypi))
