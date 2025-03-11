# SPDX-License-Identifier: GPL-2.0-only

OTHER_MENU:=Other modules

I2C_MT7621_MODULES:= \
  CONFIG_I2C_MT7621:drivers/i2c/busses/i2c-mt7621

define KernelPackage/i2c-an7581
  SUBMENU:=$(OTHER_MENU)
  $(call i2c_defaults,$(I2C_MT7621_MODULES),79)
  TITLE:=Airoha I2C Controller
  DEPENDS:=+kmod-i2c-core \
	  @(TARGET_airoha_an7581)
endef

define KernelPackage/i2c-an7581/description
 Kernel modules for enable mt7621 i2c controller.
endef

$(eval $(call KernelPackage,i2c-an7581))


define KernelPackage/pwm-an7581
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Airoha EN7581 PWM
  DEPENDS:=@(TARGET_airoha_an7581)
  KCONFIG:= \
        CONFIG_PWM=y \
        CONFIG_PWM_AIROHA=y \
        CONFIG_PWM_SYSFS=y
  FILES:= \
        $(LINUX_DIR)/drivers/pwm/pwm-airoha.ko
  AUTOLOAD:=$(call AutoProbe,pwm-airoha)
endef

define KernelPackage/pwm-an7581/description
 Kernel module to use the PWM channel on Airoha SoC
endef

$(eval $(call KernelPackage,pwm-an7581))


