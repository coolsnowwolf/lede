include $(TOPDIR)/rules.mk

PKG_NAME:=jsonfilter
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/jsonpath.git
PKG_SOURCE_DATE:=2018-02-04
PKG_SOURCE_VERSION:=c7e938d6582a436dddc938539e72dd1320625c54
PKG_MIRROR_HASH:=0601b4d7aa5ee096e99388a57cb0701673ab58fccd6ed2984a2abbd4f846e045
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
  URL:=$(PKG_SOURCE_URL)
endef

define Package/jsonfilter/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/jsonpath $(1)/usr/bin/jsonfilter
endef

$(eval $(call BuildPackage,jsonfilter))
