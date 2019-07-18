#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/pwm-bcm2835
  SUBMENU:=$(OTHER_MENU)
  TITLE:=BCM2835 PWM driver
  KCONFIG:= \
    CONFIG_PWM=y \
    CONFIG_PWM_BCM2835
  FILES:=$(LINUX_DIR)/drivers/pwm/pwm-bcm2835.ko
  AUTOLOAD:=$(call AutoLoad,60,pwm-bcm2835)
  DEPENDS:=@TARGET_brcm2708
endef

define KernelPackage/pwm-bcm2835/description
  This package contains the PWM framework driver for BCM2835 controller (Raspberry Pi)
endef

$(eval $(call KernelPackage,pwm-bcm2835))


define KernelPackage/random-bcm2835
  SUBMENU:=$(OTHER_MENU)
  TITLE:=BCM2835 HW Random Number Generator
  KCONFIG:= \
    CONFIG_HW_RANDOM_BCM2835
  FILES:=$(LINUX_DIR)/drivers/char/hw_random/bcm2835-rng.ko
  AUTOLOAD:=$(call AutoLoad,11,bcm2835-rng)
  DEPENDS:=@TARGET_brcm2708 +kmod-random-core
endef

define KernelPackage/random-bcm2835/description
  This package contains the Broadcom 2835 HW random number generator driver
endef

$(eval $(call KernelPackage,random-bcm2835))


define KernelPackage/smi-bcm2835
  SUBMENU:=$(OTHER_MENU)
  TITLE:=BCM2835 SMI driver
  KCONFIG:=CONFIG_BCM2835_SMI
  FILES:=$(LINUX_DIR)/drivers/misc/bcm2835_smi.ko
  AUTOLOAD:=$(call AutoLoad,20,bcm2835_smi)
  DEPENDS:=@TARGET_brcm2708
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
  DEPENDS:=@TARGET_brcm2708 +kmod-smi-bcm2835
endef

define KernelPackage/smi-bcm2835-dev/description
  This driver provides a character device interface (ioctl + read/write) to
  Broadcom's Secondary Memory interface. The low-level functionality is provided
  by the SMI driver itself.
endef

$(eval $(call KernelPackage,smi-bcm2835-dev))
