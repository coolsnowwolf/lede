# Copyright (C) 2016 LEDE project

include $(TOPDIR)/rules.mk

PKG_NAME:=lede-keyring
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/keyring.git
PKG_SOURCE_DATE:=2017-01-20
PKG_SOURCE_VERSION:=a50b7529880988ca96e72dede0279ff139a8ab1a
PKG_MIRROR_HASH:=811ba79ba71925e949d2c690db7d7b031ac1dd965aa831ca9b6d9d70f5657254

PKG_MAINTAINER:=John Crispin <john@phrozen.org>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/lede-keyring
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=LEDE Developer Keyring
  URL:=http://www.lede-project.org
endef

define Package/lede-keyring/description
  The keyring of with the developer using and gpg public keys.
endef

Build/Compile=

define Package/lede-keyring/install
	$(INSTALL_DIR) $(1)/etc/opkg/keys/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usign/* $(1)/etc/opkg/keys/
endef

$(eval $(call BuildPackage,lede-keyring))
