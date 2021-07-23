include $(TOPDIR)/rules.mk

PKG_NAME:=microsocks
PKG_VERSION=1.0.2
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/rofl0r/microsocks/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=5ece77c283e71f73b9530da46302fdb4f72a0ae139aa734c07fe532407a6211a

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=lean

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/microsocks
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Web Servers/Proxies
	TITLE:=microsocks for OpenWRT
	DEPENDS:=+libpthread
endef

define Package/microsocks/description
	microsocks is a Tiny Proxy in C.
endef

define Package/microsocks/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/microsocks $(1)/usr/bin/microsocks
endef

$(eval $(call BuildPackage,microsocks))
