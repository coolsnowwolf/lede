# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtl8261d
PKG_RELEASE:=1


PKG_LICENSE:=GPL-2.0-only
PKG_MAINTAINER:=StarField Xu <air_jinkela@163.com>

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/phy-rtl8261d
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL8261D 10 Gigabit PHY driver
  FILES:=$(PKG_BUILD_DIR)/src/rtl8261d.ko
  AUTOLOAD:=$(call AutoLoad,18,rtl8261d,1)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src $(PKG_BUILD_DIR)/
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/src" \
		CONFIG_REALTEK_PHY_RTL8261D=m \
		modules
endef

$(eval $(call KernelPackage,phy-rtl8261d))
