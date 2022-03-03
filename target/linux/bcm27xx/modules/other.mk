# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2019 OpenWrt.org

define KernelPackage/pwm-bcm2835
  SUBMENU:=$(OTHER_MENU)
  TITLE:=BCM2835 PWM driver
  KCONFIG:= \
    CONFIG_PWM=y \
    CONFIG_PWM_BCM2835
  FILES:=$(LINUX_DIR)/drivers/pwm/pwm-bcm2835.ko
  AUTOLOAD:=$(call AutoLoad,60,pwm-bcm2835)
  DEPENDS:=@TARGET_bcm27xx
endef

define KernelPackage/pwm-bcm2835/description
  This package contains the PWM framework driver for BCM2835 controller (Raspberry Pi)
endef

$(eval $(call KernelPackage,pwm-bcm2835))


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
