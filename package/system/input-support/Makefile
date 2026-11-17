# SPDX-License-Identifier: GPL-2.0-only
include $(TOPDIR)/rules.mk

PKG_NAME:=input-support
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define Package/input-support
  SECTION:=hardware
  CATEGORY:=Hardware support
  TITLE:=Input device support
  DEPENDS:=+procd
  PKGARCH:=all
  USERID:=:input=41
endef

define Package/input-support/description
 Allocates the input group and grants it access to evdev character devices,
 using hotplug scripts supported by this procd version.
endef

define Build/Compile
endef

define Package/input-support/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/input
	$(INSTALL_DATA) ./files/40-input $(1)/etc/hotplug.d/input/40-input
endef

$(eval $(call BuildPackage,input-support))
