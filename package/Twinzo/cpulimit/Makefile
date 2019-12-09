include $(TOPDIR)/rules.mk

PKG_NAME:=cpulimit
PKG_VERSION:=cabeb9947c
PKG_RELEASE:=1
PKG_REV:=cabeb9947ccddd9a6e6ba14503e2a33063ac1b21

#PKG_SOURCE_PROTO:=git
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
#PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
#PKG_SOURCE_URL:=https://github.com/opsengine/cpulimit.git
#PKG_SOURCE_VERSION:=$(PKG_REV)

include $(INCLUDE_DIR)/package.mk

define Package/cpulimit
  CATEGORY:=Utilities
  TITLE:=cpulimit
  URL:=https://github.com/opsengine/cpulimit.git
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/cpulimit/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cpulimit $(1)/usr/bin
endef


$(eval $(call BuildPackage,cpulimit))
