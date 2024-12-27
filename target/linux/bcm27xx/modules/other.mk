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


define KernelPackage/rp1
  SUBMENU:=$(OTHER_MENU)
  TITLE:=RP1 firmware
  KCONFIG:=CONFIG_FIRMWARE_RP1
  FILES:=$(LINUX_DIR)/drivers/firmware/rp1.ko
  AUTOLOAD:=$(call AutoLoad,21,rp1)
  DEPENDS:=@TARGET_bcm27xx_bcm2712
endef

define KernelPackage/rp1/description
  This driver provides a firmware interface to the RP1 processor using shared
  memory and a mailbox.
endef

$(eval $(call KernelPackage,rp1))


define KernelPackage/rp1-pio
  SUBMENU:=$(OTHER_MENU)
  TITLE:=RP1 PIO block support
  KCONFIG:=CONFIG_RP1_PIO
  FILES:=$(LINUX_DIR)/drivers/misc/rp1-pio.ko
  AUTOLOAD:=$(call AutoLoad,21,rp1-pio)
  DEPENDS:=@TARGET_bcm27xx_bcm2712 +kmod-rp1
endef

define KernelPackage/rp1-pio/description
  Driver providing control of the Raspberry Pi PIO block, as found in RP1
endef

$(eval $(call KernelPackage,rp1-pio))


define KernelPackage/pwm-pio-rp1
  SUBMENU:=$(OTHER_MENU)
  TITLE:=RP1 PWM support
  KCONFIG:=CONFIG_PWM_PIO_RP1
  FILES:=$(LINUX_DIR)/drivers/pwm/pwm-pio-rp1.ko
  AUTOLOAD:=$(call AutoLoad,21,pwm-pio-rp1)
  DEPENDS:=@TARGET_bcm27xx_bcm2712 +kmod-rp1-pio
endef

define KernelPackage/pwm-pio-rp1/description
  Enables precise control of PWM signals for tasks like motor control,
  LED dimming, and audio signal generation. Leveraging PIO allows for
  higher accuracy and flexibility in PWM signal generation compared
  to traditional hardware timers.
endef

$(eval $(call KernelPackage,pwm-pio-rp1))


define KernelPackage/ws2812-pio-rp1
  SUBMENU:=$(OTHER_MENU)
  TITLE:=RP1 PIO-base WS2812 driver
  KCONFIG:=CONFIG_WS2812_PIO_RP1
  FILES:=$(LINUX_DIR)/drivers/misc/ws2812-pio-rp1.ko
  AUTOLOAD:=$(call AutoLoad,21,ws2812-pio-rp1)
  DEPENDS:=@TARGET_bcm27xx_bcm2712 +kmod-rp1-pio
endef

define KernelPackage/ws2812-pio-rp1/description
  Driver for the WS2812 (NeoPixel) LEDs using the RP1 PIO hardware.
  The driver creates a character device to which rgbw pixels may be
  written. Single-byte writes to offset 0 set the brightness at runtime.
endef

$(eval $(call KernelPackage,ws2812-pio-rp1))


define KernelPackage/rp1-mailbox
  SUBMENU:=$(OTHER_MENU)
  TITLE:=RP1 mailbox IPC driver
  KCONFIG:=CONFIG_MBOX_RP1
  FILES:=$(LINUX_DIR)/drivers/mailbox/rp1-mailbox.ko
  AUTOLOAD:=$(call AutoLoad,21,rp1-mailbox)
  DEPENDS:=@TARGET_bcm27xx_bcm2712
endef

define KernelPackage/rp1-mailbox/description
  This is a RP1 mailbox IPC driver.
endef

$(eval $(call KernelPackage,rp1-mailbox))
