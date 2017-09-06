include $(TOPDIR)/rules.mk

PKG_NAME:=jsonfilter
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(LEDE_GIT)/project/jsonpath.git
PKG_SOURCE_DATE:=2016-07-02
PKG_SOURCE_VERSION:=dea067ad67d977c247c300c06676a06adf21e0c7
PKG_MIRROR_HASH:=6c0e30da3f0c82527f9b5285d7c6ae61406732f2b0543b93131fe115ffc2987a
CMAKE_INSTALL:=1

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=ISC

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/jsonfilter
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libubox +libjson-c
  TITLE:=OpenWrt JSON filter utility
  URL:=http://git.openwrt.org/?p=project/jsonpath.git
endef

define Package/jsonfilter/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/jsonpath $(1)/usr/bin/jsonfilter
endef

$(eval $(call BuildPackage,jsonfilter))
