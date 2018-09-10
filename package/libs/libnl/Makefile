#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnl
PKG_VERSION:=3.4.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/thom311/libnl/releases/download/libnl3_4_0
PKG_HASH:=b7287637ae71c6db6f89e1422c995f0407ff2fe50cecd61a312b6a9b0921f5bf
PKG_LICENSE:=LGPL-2.1

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/libnl/default
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=http://www.infradead.org/~tgr/libnl/
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

define Package/libnl
$(call Package/libnl/default)
  TITLE:=Full Netlink Library
  DEPENDS:=+libnl-genl +libnl-route +libnl-nf
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

define Package/libnl/description
 Socket handling, connection management, sending and receiving of data,
 message construction and parsing, object caching system, etc.
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections $(FPIC)

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
endef

define Package/libnl-core/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-3.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-3.so $(1)/usr/lib/libnl.so
endef

define Package/libnl-genl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-genl-3.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-genl-3.so $(1)/usr/lib/libnl-genl.so
endef

define Package/libnl-route/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-route-3.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-route-3.so $(1)/usr/lib/libnl-route.so
endef

define Package/libnl-nf/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-nf-3.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnl-nf-3.so $(1)/usr/lib/libnl-nf.so
endef

define Package/libnl/install
	:
endef

$(eval $(call BuildPackage,libnl-core))
$(eval $(call BuildPackage,libnl-genl))
$(eval $(call BuildPackage,libnl-route))
$(eval $(call BuildPackage,libnl-nf))
$(eval $(call BuildPackage,libnl))
