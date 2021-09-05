include $(TOPDIR)/rules.mk

PKG_NAME:=ykclient
PKG_VERSION:=2.15
PKG_RELEASE:=2

PKG_SOURCE:=ykclient-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://developers.yubico.com/yubico-c-client/Releases/
PKG_HASH:=f461cdefe7955d58bbd09d0eb7a15b36cb3576b88adbd68008f40ea978ea5016
PKG_MAINTAINER:=Stuart B. Wilkins <stuwilkins@mac.com>
PKG_LICENSE_FILES:=COPYING
PKG_LICENSE:=BSD-2-Clause
PKG_BUILD_DEPENDS:=curl

include $(INCLUDE_DIR)/package.mk

define Package/ykclient
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=The Yubico yubikey c client library
  URL:=https://developers.yubico.com/yubico-c-client/
  DEPENDS:=curl
endef

define Package/ykclient/description
	YubiKey C Client Library (libykclient)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include
	$(CP) $(PKG_BUILD_DIR)/ykclient*.h $(STAGING_DIR)/usr/include
	$(INSTALL_DIR) $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/.libs/libykclient.so* $(STAGING_DIR)/usr/lib
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--disable-static

define Package/ykclient/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/.libs/ykclient $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/.libs/libykclient.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,ykclient))
