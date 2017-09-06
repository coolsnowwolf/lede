include $(TOPDIR)/rules.mk

PKG_NAME:=mproxy
PKG_VERSION=1
PKG_RELEASE:=6

PKG_MAINTAINER:=lean
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE_PROTO:=git
#PKG_SOURCE_URL:=https://github.com/openwrt-develop/mproxy.git
#PKG_SOURCE_VERSION:=0e0e27192e91ff08f4e235065b6b24c642541334
PKG_SOURCE_URL:=https://github.com/lonsx/mproxy-mod.git
PKG_SOURCE_VERSION:=ea3abaa40c901da6bb8e655cde63d3cac28b83ec

PKG_SOURCE_SUBDIR:=$(PKG_NAME)
#PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_SOURCE:=$(PKG_NAME).$(PKG_VERSION).$(PKG_RELEASE).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/mproxy
	SECTION:=net
	CATEGORY:=Network
	TITLE:=mproxy for OpenWRT
	DEPENDS:=
endef

define Package/mproxy/description
	mproxy is a Tiny Proxy in C.
endef


define Package/mproxy/Prepare
  mkdir -p $(PKG_BUILD_DIR)
  $(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/mproxy/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mproxy $(1)/usr/bin/mproxy
endef

$(eval $(call BuildPackage,mproxy))

