# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

MENU_TITLE:=Multiplexer Support

define KernelPackage/mux-core
  SUBMENU:=$(MENU_TITLE)
  TITLE:=Multiplexer Support
  KCONFIG:=CONFIG_MULTIPLEXER
  FILES:=$(LINUX_DIR)/drivers/mux/mux-core.ko
  AUTOLOAD:=$(call AutoLoad,25,mux-core,1)
endef

define KernelPackage/mux-core/description
  Kernel module for multiplexer support
endef

$(eval $(call KernelPackage,mux-core))

define KernelPackage/mux-gpio
  SUBMENU:=$(MENU_TITLE)
  TITLE:=GPIO-controlled Multiplexer controller
  KCONFIG:=CONFIG_MUX_GPIO
  DEPENDS:=@GPIO_SUPPORT kmod-mux-core
  FILES:=$(LINUX_DIR)/drivers/mux/mux-gpio.ko
  AUTOLOAD:=$(call AutoLoad,25,mux-gpio,1)
endef

define KernelPackage/mux-gpio/description
  Kernel modules for GPIO-controlled Multiplexer controller
endef

$(eval $(call KernelPackage,mux-gpio))
