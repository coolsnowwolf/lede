include $(TOPDIR)/rules.mk

PKG_NAME:=libgabe
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/schanzen/libgabe/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=2a8c151a90c9ad8eaad073c8ad1482d66875e3433b0b4fd1e08424c0fc89e877

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libgabe
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+glib2 +libgcrypt +libgmp +libpbc
  TITLE:=Ciphertext-Policy Attribute-Based Encryption
  URL:=https://github.com/schanzen/libgabe
endef

define Package/libgabe/description
  This is a fork of the libbswabe library by John Bethencourt
  (http://hms.isi.jhu.edu/acsc/cpabe/) replacing openssl with libgcrypt
  and fixing some bugs.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

define Package/libgabe/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libgabe))
