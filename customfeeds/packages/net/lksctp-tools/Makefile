#
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lksctp-tools
PKG_VERSION:=1.0.18
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/sctp/lksctp-tools/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=3e9ab5b3844a8b65fc8152633aafe85f406e6da463e53921583dfc4a443ff03a

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/lksctp-tools/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=SCTP user-land
  URL:=https://github.com/sctp/lksctp-tools
endef

define Package/libsctp
$(call Package/lksctp-tools/Default)
  SUBMENU:=Networking
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= library
  DEPENDS:=+kmod-sctp
endef

define Package/sctp
$(call Package/lksctp-tools/Default)
  TITLE+= (meta)
  DEPENDS:=+libsctp +sctp-tools
  BUILDONLY:=1
endef

define Package/sctp-tools
$(call Package/lksctp-tools/Default)
  TITLE+= tools
  DEPENDS:=+libsctp
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/netinet
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/include/netinet/sctp.h \
		$(1)/usr/include/netinet
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libsctp.{a,so*} \
		$(1)/usr/lib/
endef

define Package/libsctp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libsctp.so.* \
		$(1)/usr/lib/
endef

define Package/sctp-tools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/checksctp \
		$(1)/usr/bin/
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/sctp_{darn,status,test} \
		$(1)/usr/bin/
endef

$(eval $(call BuildPackage,libsctp))
$(eval $(call BuildPackage,sctp))
$(eval $(call BuildPackage,sctp-tools))
