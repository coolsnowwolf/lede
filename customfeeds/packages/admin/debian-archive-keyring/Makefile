include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=debian-archive-keyring
PKG_VERSION:=2021.1.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=debian-archive-keyring_2021.1.1_all.deb
PKG_SOURCE_URL:=http://ftp.debian.org/debian/pool/main/d/debian-archive-keyring/
PKG_HASH:=56beca470dcd9b6d7e6c3c9e9d702101e01e9467e62810a8c357bd7b9c26251d
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=Unique
PKG_LICENSE_FILES:=usr/share/doc/debian-archive-keyring/copyright

include $(INCLUDE_DIR)/package.mk

define Package/debian-archive-keyring
  SECTION:=admin
  CATEGORY:=Administration
  TITLE:=Debian Archive keyring
  URL:=https://salsa.debian.org/release-team/debian-archive-keyring
  PKGARCH:=all
endef

define Build/Prepare
	$(AR) p $(DL_DIR)/$(PKG_SOURCE) data.tar.xz | $(TAR) -xJC $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/debian-archive-keyring/install
	$(INSTALL_DIR) $(1)/usr/share/keyrings
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usr/share/keyrings/*.gpg $(1)/usr/share/keyrings
endef

$(eval $(call BuildPackage,debian-archive-keyring))
