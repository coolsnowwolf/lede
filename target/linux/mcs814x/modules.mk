#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

define KernelPackage/mcs814x-wdt
  SUBMENU:=$(OTHER_MENU)
  DEPENDS:=@TARGET_mcs814x
  TITLE:=MCS814x watchdog driver
  KCONFIG:=CONFIG_MCS814X_WATCHDOG
  FILES:=$(LINUX_DIR)/drivers/$(WATCHDOG_DIR)/mcs814x_wdt.ko
  AUTOLOAD:=$(call AutoLoad,50,mcs814x_wdt)
endef

define KernelPackage/mcs814x-wdt/description
  Kernel module for the Moschip MCS814x watchdog timer.
endef

$(eval $(call KernelPackage,mcs814x-wdt))
