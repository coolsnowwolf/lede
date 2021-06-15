#
# Copyright (C) 2010-2016 Jo-Philipp Wich <jo@mein.io>
#
# This is free software, licensed under the GPL 2 license.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libiwinfo
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/iwinfo.git
PKG_SOURCE_DATE:=2021-06-09
PKG_SOURCE_VERSION:=c0414642fead263a4a6a686ad3cb7e965ec8a23a
PKG_MIRROR_HASH:=c5686bbae86753c53db03a686b034bbb80d31107cc359ebd8522ea1c82db35ea
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=GPL-2.0

PKG_FLAGS := nonshared

PKG_CONFIG_DEPENDS := \
	CONFIG_PACKAGE_kmod-brcm-wl \
	CONFIG_PACKAGE_kmod-brcm-wl-mini \
	CONFIG_PACKAGE_kmod-brcm-wl-mimo \
	CONFIG_PACKAGE_kmod-cfg80211

IWINFO_ABI_VERSION:=20210430

include $(INCLUDE_DIR)/package.mk


define Package/libiwinfo
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Generalized Wireless Information Library (iwinfo)
  DEPENDS:=+PACKAGE_kmod-cfg80211:libnl-tiny +libuci +libubus
  ABI_VERSION:=$(IWINFO_ABI_VERSION)
endef

define Package/libiwinfo/description
  Wireless information library with consistent interface for proprietary Broadcom,
  nl80211 and wext driver interfaces.
endef


define Package/libiwinfo-lua
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=libiwinfo Lua binding
  DEPENDS:=+libiwinfo +liblua
endef

define Package/libiwinfo-lua/description
  This is the Lua binding for the iwinfo library. It provides access to all enabled
  backends.
endef


define Package/iwinfo
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Generalized Wireless Information utility
  DEPENDS:=+libiwinfo
endef

define Package/iwinfo/description
  Command line frontend for the wireless information library.
endef


define Build/Configure
endef

IWINFO_BACKENDS := \
	$(if $(CONFIG_PACKAGE_kmod-brcm-wl),wl) \
	$(if $(CONFIG_PACKAGE_kmod-brcm-wl-mini),wl) \
	$(if $(CONFIG_PACKAGE_kmod-brcm-wl-mimo),wl) \
	$(if $(CONFIG_PACKAGE_kmod-cfg80211),nl80211)

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(STAGING_DIR)/usr/include \
	-D_GNU_SOURCE

MAKE_FLAGS += \
	FPIC="$(FPIC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	BACKENDS="$(IWINFO_BACKENDS)" \
	SOVERSION="$(IWINFO_ABI_VERSION)"

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/iwinfo
	$(CP) $(PKG_BUILD_DIR)/include/iwinfo.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/include/iwinfo/* $(1)/usr/include/iwinfo/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libiwinfo.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iwinfo.so $(1)/usr/lib/lua/iwinfo.so
endef

define Package/libiwinfo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libiwinfo.so.$(IWINFO_ABI_VERSION) $(1)/usr/lib/libiwinfo.so.$(IWINFO_ABI_VERSION)
	$(LN) libiwinfo.so.$(IWINFO_ABI_VERSION) $(1)/usr/lib/libiwinfo.so
	$(INSTALL_DIR) $(1)/usr/share/libiwinfo
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/hardware.txt $(1)/usr/share/libiwinfo/hardware.txt
endef

define Package/libiwinfo-lua/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iwinfo.so $(1)/usr/lib/lua/iwinfo.so
endef

define Package/iwinfo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iwinfo $(1)/usr/bin/iwinfo
endef

$(eval $(call BuildPackage,libiwinfo))
$(eval $(call BuildPackage,libiwinfo-lua))
$(eval $(call BuildPackage,iwinfo))
