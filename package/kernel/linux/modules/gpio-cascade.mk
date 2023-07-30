# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

MENU_TITLE:=GPIO support

define KernelPackage/gpio-cascade
  SUBMENU:=$(MENU_TITLE)
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
