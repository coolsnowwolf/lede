# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2013 OpenWrt.org
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnl
PKG_VERSION:=3.11.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/thom311/libnl/releases/download/libnl$(subst .,_,$(PKG_VERSION))
PKG_HASH:=2a56e1edefa3e68a7c00879496736fdbf62fc94ed3232c0baba127ecfa76874d

PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:libnl_project:libnl

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

PKG_BUILD_FLAGS:=gc-sections

include $(INCLUDE_DIR)/package.mk

define Package/libnl/default
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=http://www.infradead.org/~tgr/libnl/
  ABI_VERSION:=200
endef

define Package/libnl-core
$(call Package/libnl/default)
  TITLE:=Core Netlink Library
  DEPENDS:=+libpthread
endef

define Package/libnl-genl
$(call Package/libnl/default)
  TITLE:=Generic Netlink Library
  DEPENDS:=+libnl-core
endef

define Package/libnl-route
$(call Package/libnl/default)
  TITLE:=Routing Netlink Library
  DEPENDS:=+libnl-core
endef

define Package/libnl-nf
$(call Package/libnl/default)
  TITLE:=Netfilter Netlink Library
  DEPENDS:=+libnl-route
endef

define Package/libnl-cli
$(call Package/libnl/default)
  TITLE:=CLI Netlink Library
  DEPENDS:=+libnl-genl +libnl-nf
endef

define Package/libnl
$(call Package/libnl/default)
  TITLE:=Full Netlink Library
  DEPENDS:=+libnl-genl +libnl-route +libnl-nf +libnl-cli
endef

define Package/libnl-core/description
 Common code for all netlink libraries
endef

define Package/libnl-genl/description
 Generic Netlink Library Functions
endef

define Package/libnl-route/description
 Routing Netlink Library Functions
endef

define Package/libnl-nf/description
 Netfilter Netlink Library Functions
endef

define Package/libnl-cli/description
 CLI Netlink Library Functions
endef

define Package/libnl/description
 Socket handling, connection management, sending and receiving of data,
 message construction and parsing, object caching system, etc.
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--disable-debug

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/libnl3 $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libnl3/* $(1)/usr/include/libnl3/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig

	# Copy symlinks
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-3.so $(1)/usr/lib/libnl.so
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-genl-3.so $(1)/usr/lib/libnl-genl.so
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-nf-3.so $(1)/usr/lib/libnl-nf.so
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-route-3.so $(1)/usr/lib/libnl-route.so
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-cli-3.so $(1)/usr/lib/libnl-cli.so
endef

define Package/libnl-core/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-3.so.* $(1)/usr/lib/
endef

define Package/libnl-genl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-genl-3.so.* $(1)/usr/lib/
endef

define Package/libnl-route/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-route-3.so.* $(1)/usr/lib/
endef

define Package/libnl-nf/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-nf-3.so.* $(1)/usr/lib/
endef

define Package/libnl-cli/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-cli-3.so.* $(1)/usr/lib/
endef

define Package/libnl/install
	:
endef

$(eval $(call BuildPackage,libnl-core))
$(eval $(call BuildPackage,libnl-genl))
$(eval $(call BuildPackage,libnl-route))
$(eval $(call BuildPackage,libnl-nf))
$(eval $(call BuildPackage,libnl-cli))
$(eval $(call BuildPackage,libnl))
