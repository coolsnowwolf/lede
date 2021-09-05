include $(TOPDIR)/rules.mk

PKG_NAME:=device-observatory
PKG_VERSION:=1.2.0
PKG_RELEASE:=7

PKG_SOURCE_URL:=https://codeload.github.com/mwarning/device-observatory/tar.gz/v$(PKG_VERSION)?
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=83b3f362f154a427abbd3af31b3c2dda9983cdc15f6b833d804727ef0fbdc72e

PKG_LICENSE:=GPL-3.0-or-later
PKG_BUILD_DEPENDS:=vim/host

include $(INCLUDE_DIR)/package.mk

define Package/device-observatory
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=device-observatory
	MAINTAINER:=Moritz Warning <moritzwarning@web.de>
	URL:=https://github.com/mwarning/device-observatory/
	DEPENDS:=+iw +libpcap +libmicrohttpd
endef

define Package/device-observatory/conffiles
/etc/config/device-observatory
endef

define Package/device-observatory/description
	Show information about connected devices and connections to increase security awareness.
endef

define Package/device-observatory/install
	$(CP) files/* $(1)
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/device-observatory $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/device-observatory
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/misc/macdb.txt $(1)/usr/share/device-observatory/macdb.txt
endef

$(eval $(call BuildPackage,device-observatory))
