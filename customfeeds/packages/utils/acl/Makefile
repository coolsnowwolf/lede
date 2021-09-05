#
# Copyright (C) 2014-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=acl
PKG_VERSION:=2.3.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://git.savannah.nongnu.org/cgit/acl.git/snapshot
PKG_HASH:=8cad1182cc5703c3e8bf7a220fc267f146246f088d1ba5dd72d8b02736deedcc
PKG_MAINTAINER:=Maxim Storchak <m.storchak@gmail.com>

PKG_LICENSE:=LGPL-2.1 GPL-2.0
PKG_LICENSE_FILES:=doc/COPYING doc/COPYING.LGPL

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/acl/Default
  TITLE:=Access control list (ACL) manipulation
  URL:=https://savannah.nongnu.org/projects/acl
  SUBMENU:=Filesystem
endef

define Package/acl/Default/description
  Access control list support
endef

define Package/acl
$(call Package/acl/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE+=utils
  DEPENDS:=+libacl
endef

define Package/libacl
$(call Package/acl/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+=library
  DEPENDS:=+libattr
endef

define Package/libacl/description
$(call Package/acl/Default/description)
 This package provides libacl
endef

define Package/acl/description
$(call Package/acl/Default/description)
 This package provides ACL manipulation utilities
 - chacl
 - getfacl
 - setfacl
endef

CONFIGURE_ARGS += --enable-static --enable-shared

define Package/acl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

define Package/libacl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

define Build/InstallDev
	mkdir -p $(1)/usr/include
	mkdir -p $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/{include,lib} $(1)/usr/
endef

$(eval $(call BuildPackage,acl))
$(eval $(call BuildPackage,libacl))
