include $(TOPDIR)/rules.mk

PKG_NAME:=yubico-pam
PKG_VERSION:=2.27
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=pam_yubico-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://developers.yubico.com/yubico-pam/Releases
PKG_HASH:=63d02788852644d871746e1a7a1d16c272c583c226f62576f5ad232a6a44e18c
PKG_BUILD_DIR:=$(BUILD_DIR)/pam_yubico-$(PKG_VERSION)

PKG_MAINTAINER:=Stuart B. Wilkins <stuwilkins@mac.com>
PKG_LICENSE_FILES:=COPYING
PKG_LICENSE:=BSD-2-Clause

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/yubico-pam
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=The Yubico PAM module
	URL:=https://developers.yubico.com/yubico-pam/
	DEPENDS:=+ykclient +ykpers +libyubikey +libpam +curl
endef

define Package/yubico-pam/description
	The Yubico PAM module provides an easy way to integrate the YubiKey
	into your existing user authentication infrastructure.
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--disable-static \
	--without-ldap

define Package/yubico-pam/install
	$(INSTALL_DIR) $(1)/usr/lib/security
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/security/pam_yubico.so $(1)/usr/lib/security
endef

$(eval $(call BuildPackage,yubico-pam))
