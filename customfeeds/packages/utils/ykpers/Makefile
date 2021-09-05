include $(TOPDIR)/rules.mk

PKG_NAME:=ykpers
PKG_VERSION:=1.20.0
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://developers.yubico.com/yubikey-personalization/Releases
PKG_HASH:=0ec84d0ea862f45a7d85a1a3afe5e60b8da42df211bb7d27a50f486e31a79b93

PKG_MAINTAINER:=Stuart B. Wilkins <stuwilkins@mac.com>
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ykpers
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=The Yubico personalization package
  URL:=https://developers.yubico.com/yubikey-personalization/
  DEPENDS:=+curl +libyubikey +libjson-c +libusb-1.0
endef

define Package/ykpers/description
	The YubiKey Personalization package contains a library and command
	line tool used to personalize (i.e., set a AES key) YubiKeys.
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--disable-static

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/ykpers-1
	$(CP) $(PKG_INSTALL_DIR)/usr/include/ykpers-1/*.h $(STAGING_DIR)/usr/include/ykpers-1
	$(INSTALL_DIR) $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libykpers-1.so* $(STAGING_DIR)/usr/lib
	$(INSTALL_DIR) $(STAGING_DIR)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/ykpers-1.pc $(STAGING_DIR)/usr/lib/pkgconfig
endef

define Package/ykpers/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ykchalresp $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ykinfo $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ykpersonalize $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libykpers-1.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,ykpers))
