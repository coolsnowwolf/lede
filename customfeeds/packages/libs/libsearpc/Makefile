#
# Copyright (C) 2007-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libsearpc
PKG_VERSION:=3.2.0
PKG_RELEASE:=5

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/haiwen/libsearpc.git
PKG_SOURCE_VERSION:=d1fd7518a20b25766ff98d6fde6e40c63626ca0d
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)-$(call version_abbrev,$(PKG_SOURCE_VERSION))
PKG_MIRROR_HASH:=ee8b8b50b480afbcb71bd73cd9c2a96cfa72e9dafaf5773574adbf348e49a0d4

PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE.txt

PKG_FIXUP:=autoreconf
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PYTHON3_PKG_BUILD:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../../lang/python/python3-package.mk

define Package/libsearpc
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Seafile RPC Library
  URL:=https://seafile.com
  DEPENDS:=+glib2 +jansson $(ICONV_DEPENDS)
endef

define Package/libsearpc/description
  Searpc is a simple C language RPC framework based on GObject system.
  Searpc handles the serialization/deserialization part of RPC, the
  transport part is left to users.
endef

define Package/python3-searpc
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Python bindings for Searpc
  DEPENDS:=+libsearpc +python3-light +python3-logging
endef

define Package/python3-searpc/description
$(call Package/libsearpc/description)

  This package contains Python bindings for Searpc.
endef

CONFIGURE_ARGS += \
	--disable-compile-demo \
	--disable-server-pkg

CONFIGURE_VARS += \
	PYTHON="$(HOST_PYTHON3_BIN)"

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/{bin,include}
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsearpc.so* $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libsearpc.pc $(1)/usr/lib/pkgconfig/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/searpc-codegen.py $(1)/usr/bin/
endef

define Package/libsearpc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsearpc.so* $(1)/usr/lib/
endef

Py3Package/python3-searpc/install:=:

$(eval $(call BuildPackage,libsearpc))

$(eval $(call Py3Package,python3-searpc))
$(eval $(call BuildPackage,python3-searpc))
$(eval $(call BuildPackage,python3-searpc-src))
