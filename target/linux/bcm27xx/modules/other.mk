# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2019 OpenWrt.org

define KernelPackage/pwm-raspberrypi-poe
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Raspberry Pi Firwmware PoE Hat PWM support
  KCONFIG:= \
    CONFIG_PWM=y \
    CONFIG_PWM_RASPBERRYPI_POE
  FILES:=$(LINUX_DIR)/drivers/pwm/pwm-raspberrypi-poe.ko
  AUTOLOAD:=$(call AutoLoad,20,pwm-raspberrypi-poe)
  DEPENDS:=@TARGET_bcm27xx +kmod-hwmon-pwmfan
endef

define KernelPackage/pwm-raspberrypi-poe/description
  This package provides Raspberry Pi Firwmware PoE Hat PWM support
endef

$(eval $(call KernelPackage,pwm-raspberrypi-poe))


define KernelPackage/smi-bcm2835
  SUBMENU:=$(OTHER_MENU)
  TITLE:=BCM2835 SMI driver
  KCONFIG:=CONFIG_BCM2835_SMI
  FILES:=$(LINUX_DIR)/drivers/misc/bcm2835_smi.ko
  AUTOLOAD:=$(call AutoLoad,20,bcm2835_smi)
  DEPENDS:=@TARGET_bcm27xx
endef

define KernelPackage/smi-bcm2835/description
  This package contains the Character device driver for Broadcom Secondary
  Memory Interface
endef

$(eval $(call KernelPackage,smi-bcm2835))


define KernelPackage/smi-bcm2835-dev
  SUBMENU:=$(OTHER_MENU)
  TITLE:=BCM2835 SMI device driver
  KCONFIG:=CONFIG_BCM2835_SMI_DEV
  FILES:=$(LINUX_DIR)/drivers/char/broadcom/bcm2835_smi_dev.ko
  AUTOLOAD:=$(call AutoLoad,21,bcm2835_smi_dev)
  DEPENDS:=@TARGET_bcm27xx +kmod-smi-bcm2835
endef

define KernelPackage/smi-bcm2835-dev/description
  This driver provides a character device interface (ioctl + read/write) to
  Broadcom's Secondary Memory interface. The low-level functionality is provided
  by the SMI driver itself.
endef

$(eval $(call KernelPackage,smi-bcm2835-dev))
