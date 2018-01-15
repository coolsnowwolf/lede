
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ipset
PKG_VERSION:=6.34
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://ipset.netfilter.org
PKG_HASH:=d70e831b670b7aa25dde81fd994d3a7ce0c0e801559a557105576df66cd8d680

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=GPL-2.0

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ipset/Default
  SECTION:=net
  CATEGORY:=Network
  DEPENDS+= +kmod-ipt-ipset +libmnl
  TITLE:=IPset administration utility
  URL:=http://ipset.netfilter.org/
endef

define Package/ipset
$(call Package/ipset/Default)
  DEPENDS+= +libipset
endef

define Package/libipset
$(call Package/ipset/Default)
endef

CONFIGURE_ARGS += \
	--with-kbuild="$(LINUX_DIR)"

MAKE_FLAGS += \
	ARCH="$(LINUX_KARCH)" \
	SHELL="$(BASH)"

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libipset $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libipset.{a,so*} $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/lib/libipset.pc $(1)/usr/lib/pkgconfig/
endef

define Package/ipset/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/ipset $(1)/usr/sbin/
endef

define Package/libipset/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libipset*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,ipset))
$(eval $(call BuildPackage,libipset))
