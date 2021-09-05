# Copyright (C) 2020 xiaorouji
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ssocks
PKG_VERSION:=0.0.14
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/tostercx/ssocks.git
PKG_MIRROR_HASH:=0ce65cb7aaea78c97bba9d62af3b40ed0d422938f67cb44a24d522608cfd097c
PKG_SOURCE_VERSION:=c2024789c1ee076d171fd6061d7c133302216ea7
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_INSTALL:=1

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Network
  TITLE:=sSocks Relay
  URL:=https://github.com/tostercx/ssocks
  DEPENDS:=+libopenssl
endef

define Package/$(PKG_NAME)/description
sSocks is a package which contains: a socks5 server implements RFC 1928 (SOCKS V5) and RFC 1929 (Authentication for SOCKS V5), a reverse socks server and client, a netcat like tool and a socks5 relay.
endef

define Package/$(PKG_NAME)d
  $(call Package/$(PKG_NAME))
  TITLE:=sSocks Server
endef

Package/$(PKG_NAME)d/description = $(Package/$(PKG_NAME)/description)

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/$(PKG_NAME)$(2) $(1)/usr/bin/
endef

define Package/$(PKG_NAME)d/install
	$(call Package/$(PKG_NAME)/install,$(1),d)
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)d))