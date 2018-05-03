#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

LEDS_MENU:=LED modules

define KernelPackage/leds-gpio
  SUBMENU:=$(LEDS_MENU)
  TITLE:=GPIO LED support
  DEPENDS:= @GPIO_SUPPORT
  KCONFIG:=CONFIG_LEDS_GPIO
  FILES:=$(LINUX_DIR)/drivers/leds/leds-gpio.ko
  AUTOLOAD:=$(call AutoLoad,60,leds-gpio,1)
endef

define KernelPackage/leds-gpio/description
 Kernel module for LEDs on GPIO lines
endef

$(eval $(call KernelPackage,leds-gpio))

LED_TRIGGER_DIR=$(LINUX_DIR)/drivers/leds/trigger

define KernelPackage/ledtrig-heartbeat
  SUBMENU:=$(LEDS_MENU)
  TITLE:=LED Heartbeat Trigger
  KCONFIG:=CONFIG_LEDS_TRIGGER_HEARTBEAT
  FILES:=$(LED_TRIGGER_DIR)/ledtrig-heartbeat.ko
  AUTOLOAD:=$(call AutoLoad,50,ledtrig-heartbeat)
endef

define KernelPackage/ledtrig-heartbeat/description
 Kernel module that allows LEDs to blink like heart beat
endef

$(eval $(call KernelPackage,ledtrig-heartbeat))


define KernelPackage/ledtrig-gpio
  SUBMENU:=$(LEDS_MENU)
  TITLE:=LED GPIO Trigger
  KCONFIG:=CONFIG_LEDS_TRIGGER_GPIO
  FILES:=$(LED_TRIGGER_DIR)/ledtrig-gpio.ko
  AUTOLOAD:=$(call AutoLoad,50,ledtrig-gpio)
endef

define KernelPackage/ledtrig-gpio/description
 Kernel module that allows LEDs to be controlled by gpio events
endef

$(eval $(call KernelPackage,ledtrig-gpio))


define KernelPackage/ledtrig-netdev
  SUBMENU:=$(LEDS_MENU)
  TITLE:=LED NETDEV Trigger
  KCONFIG:=CONFIG_LEDS_TRIGGER_NETDEV
  FILES:=$(LINUX_DIR)/drivers/leds/ledtrig-netdev.ko
  AUTOLOAD:=$(call AutoLoad,50,ledtrig-netdev)
endef

define KernelPackage/ledtrig-netdev/description
 Kernel module to drive LEDs based on network activity
endef

$(eval $(call KernelPackage,ledtrig-netdev))


define KernelPackage/ledtrig-default-on
  SUBMENU:=$(LEDS_MENU)
  TITLE:=LED Default ON Trigger
  KCONFIG:=CONFIG_LEDS_TRIGGER_DEFAULT_ON
  FILES:=$(LED_TRIGGER_DIR)/ledtrig-default-on.ko
  AUTOLOAD:=$(call AutoLoad,50,ledtrig-default-on,1)
endef

define KernelPackage/ledtrig-default-on/description
 Kernel module that allows LEDs to be initialised in the ON state
endef

$(eval $(call KernelPackage,ledtrig-default-on))


define KernelPackage/ledtrig-timer
  SUBMENU:=$(LEDS_MENU)
  TITLE:=LED Timer Trigger
  KCONFIG:=CONFIG_LEDS_TRIGGER_TIMER
  FILES:=$(LED_TRIGGER_DIR)/ledtrig-timer.ko
  AUTOLOAD:=$(call AutoLoad,50,ledtrig-timer,1)
endef

define KernelPackage/ledtrig-timer/description
 Kernel module that allows LEDs to be controlled by a programmable timer
 via sysfs
endef

$(eval $(call KernelPackage,ledtrig-timer))


define KernelPackage/ledtrig-transient
  SUBMENU:=$(LEDS_MENU)
  TITLE:=LED Transient Trigger
  KCONFIG:=CONFIG_LEDS_TRIGGER_TRANSIENT
  FILES:=$(LED_TRIGGER_DIR)/ledtrig-transient.ko
  AUTOLOAD:=$(call AutoLoad,50,ledtrig-transient,1)
endef

define KernelPackage/ledtrig-transient/description
 Kernel module that allows LEDs one time activation of a transient state.
endef

$(eval $(call KernelPackage,ledtrig-transient))


define KernelPackage/ledtrig-oneshot
  SUBMENU:=$(LEDS_MENU)
  TITLE:=LED One-Shot Trigger
  KCONFIG:=CONFIG_LEDS_TRIGGER_ONESHOT
  FILES:=$(LED_TRIGGER_DIR)/ledtrig-oneshot.ko
  AUTOLOAD:=$(call AutoLoad,50,ledtrig-oneshot)
endef

define KernelPackage/ledtrig-oneshot/description
 Kernel module that allows LEDs to be triggered by sporadic events in
 one-shot pulses
endef

$(eval $(call KernelPackage,ledtrig-oneshot))


define KernelPackage/leds-pca963x
  SUBMENU:=$(LEDS_MENU)
  TITLE:=PCA963x LED support
  DEPENDS:=+kmod-i2c-core
  KCONFIG:=CONFIG_LEDS_PCA963X
  FILES:=$(LINUX_DIR)/drivers/leds/leds-pca963x.ko
  AUTOLOAD:=$(call AutoLoad,60,leds-pca963x,1)
endef

define KernelPackage/leds-pca963x/description
 Driver for the NXP PCA963x I2C LED controllers.
endef

$(eval $(call KernelPackage,leds-pca963x))
