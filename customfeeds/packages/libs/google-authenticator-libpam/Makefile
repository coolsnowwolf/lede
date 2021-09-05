#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=google-authenticator-libpam
PKG_VERSION:=1.09
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/google/google-authenticator-libpam/tar.gz/$(PKG_VERSION)?
PKG_HASH:=ab1d7983413dc2f11de2efa903e5c326af8cb9ea37765dacb39949417f7cd037

PKG_MAINTAINER:=Lucian Cristian <lucian.cristian@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/google-authenticator-libpam
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libpam +libqrencode
  TITLE:=Google Authenticator PAM module
  URL:=https://github.com/google/google-authenticator-libpam
endef

define Package/google-authenticator-libpam/description
    Google Authenticator PAM module
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/security
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/security/* $(1)/usr/lib/security/
endef

define Package/google-authenticator-libpam/install
	$(INSTALL_DIR) $(1)/usr/lib/security
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/security/*.so* \
	    $(1)/usr/lib/security/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,google-authenticator-libpam))
