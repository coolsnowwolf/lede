# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

GPIO_MENU:=GPIO support

define KernelPackage/gpio-amd-fch
  SUBMENU:=$(GPIO_MENU)
  DEPENDS:=@GPIO_SUPPORT @TARGET_x86
  TITLE:=GPIO support for AMD Fusion Controller Hub (G-series SOCs)
  KCONFIG:=CONFIG_GPIO_AMD_FCH
  FILES:=$(LINUX_DIR)/drivers/gpio/gpio-amd-fch.ko
  AUTOLOAD:=$(call AutoLoad,25,gpio-amd-fch,1)
endef

define KernelPackage/gpio-amd-fch/description
  This option enables driver for GPIO on AMDs Fusion Controller Hub,
  as found on G-series SOCs (eg. GX-412TC)
endef

$(eval $(call KernelPackage,gpio-amd-fch))


define KernelPackage/gpio-beeper
  SUBMENU:=$(GPIO_MENU)
  TITLE:=GPIO beeper support
  DEPENDS:=+kmod-input-core
  KCONFIG:= \
	CONFIG_INPUT_MISC=y \
	CONFIG_INPUT_GPIO_BEEPER
  FILES:= \
	$(LINUX_DIR)/drivers/input/misc/gpio-beeper.ko
  AUTOLOAD:=$(call AutoLoad,50,gpio-beeper)
endef

define KernelPackage/gpio-beeper/description
 This enables playing beeps through an GPIO-connected buzzer
endef

$(eval $(call KernelPackage,gpio-beeper))


define KernelPackage/gpio-cascade
  SUBMENU:=$(GPIO_MENU)
  TITLE:=Generic GPIO cascade
  KCONFIG:=CONFIG_GPIO_CASCADE
  DEPENDS:=@GPIO_SUPPORT +kmod-mux-core
  FILES:=$(LINUX_DIR)/drivers/gpio/gpio-cascade.ko
  AUTOLOAD:=$(call AutoLoad,29,gpio-cascade,1)
endef

define KernelPackage/gpio-cascade/description
  Kernel module for Generic GPIO cascade
endef

$(eval $(call KernelPackage,gpio-cascade))


define KernelPackage/gpio-f7188x
  SUBMENU:=$(GPIO_MENU)
  TITLE:=Fintek F718xx/F818xx GPIO Support
  DEPENDS:=@GPIO_SUPPORT @TARGET_x86
  KCONFIG:=CONFIG_GPIO_F7188X
  FILES:=$(LINUX_DIR)/drivers/gpio/gpio-f7188x.ko
  AUTOLOAD:=$(call AutoProbe,gpio-f7188x)
endef

define KernelPackage/gpio-f7188x/description
  Kernel module for the GPIOs found on many Fintek Super-IO chips.
endef

$(eval $(call KernelPackage,gpio-f7188x))


define KernelPackage/gpio-it87
  SUBMENU:=$(GPIO_MENU)
  DEPENDS:=@GPIO_SUPPORT @TARGET_x86
  TITLE:=GPIO support for IT87xx Super I/O chips
  KCONFIG:=CONFIG_GPIO_IT87
  FILES:=$(LINUX_DIR)/drivers/gpio/gpio-it87.ko
  AUTOLOAD:=$(call AutoLoad,25,gpio-it87,1)
endef

define KernelPackage/gpio-it87/description
  This driver is tested with ITE IT8728 and IT8732 Super I/O chips, and
  supports the IT8761E, IT8613, IT8620E, and IT8628E Super I/O chips as
  well.
endef

$(eval $(call KernelPackage,gpio-it87))


define KernelPackage/gpio-nxp-74hc164
  SUBMENU:=$(GPIO_MENU)
  TITLE:=NXP 74HC164 GPIO expander support
  KCONFIG:=CONFIG_GPIO_74X164
  FILES:=$(LINUX_DIR)/drivers/gpio/gpio-74x164.ko
  AUTOLOAD:=$(call AutoProbe,gpio-74x164)
endef

define KernelPackage/gpio-nxp-74hc164/description
 Kernel module for NXP 74HC164 GPIO expander
endef

$(eval $(call KernelPackage,gpio-nxp-74hc164))


define KernelPackage/gpio-pca953x
  SUBMENU:=$(GPIO_MENU)
  DEPENDS:=@GPIO_SUPPORT +kmod-i2c-core +kmod-regmap-i2c
  TITLE:=PCA95xx, TCA64xx, and MAX7310 I/O ports
  KCONFIG:=CONFIG_GPIO_PCA953X \
	   CONFIG_GPIO_PCA953X_IRQ=y
  FILES:=$(LINUX_DIR)/drivers/gpio/gpio-pca953x.ko
  AUTOLOAD:=$(call AutoLoad,55,gpio-pca953x)
endef

define KernelPackage/gpio-pca953x/description
 Kernel module for MAX731{0,2,3,5}, PCA6107, PCA953{4-9}, PCA955{4-7},
 PCA957{4,5} and TCA64{08,16} I2C GPIO expanders
endef

$(eval $(call KernelPackage,gpio-pca953x))


define KernelPackage/gpio-pcf857x
  SUBMENU:=$(GPIO_MENU)
  DEPENDS:=@GPIO_SUPPORT +kmod-i2c-core
  TITLE:=PCX857x, PCA967x and MAX732X I2C GPIO expanders
  KCONFIG:=CONFIG_GPIO_PCF857X
  FILES:=$(LINUX_DIR)/drivers/gpio/gpio-pcf857x.ko
  AUTOLOAD:=$(call AutoLoad,55,gpio-pcf857x)
endef

define KernelPackage/gpio-pcf857x/description
 Kernel module for PCF857x, PCA{85,96}7x, and MAX732[89] I2C GPIO expanders
endef

$(eval $(call KernelPackage,gpio-pcf857x))
