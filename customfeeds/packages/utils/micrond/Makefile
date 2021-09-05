include $(TOPDIR)/rules.mk

PKG_NAME:=micrond
PKG_VERSION:=1
PKG_RELEASE:=2
PKG_LICENSE:=BSD-2-clause

include $(INCLUDE_DIR)/package.mk

define Package/micrond
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Small Cron daemon providing a cron.d directory other packages can install their crontabs into
  DEPENDS:=
endef

define Package/micrond/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/micrond $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,micrond))
