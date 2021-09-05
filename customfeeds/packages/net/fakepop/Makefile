#
# This software is licensed under the Public Domain.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fakepop
PKG_VERSION:=11
PKG_RELEASE:=1

PKG_MAINTAINER:=Marc Egerton <foxtrot@realloc.me>
PKG_LICENSE:=GPL-3.0-or-later

PKG_SOURCE:=fakepop_11.tar.gz
PKG_SOURCE_URL:=http://deb.debian.org/debian/pool/main/f/fakepop/
PKG_HASH:=9756259ade152f8dac78174019b4145b4e07738631055c34bc04e64f865393e7
# Downloaded .tar.gz contains "fakepop-10" instead of "fakepop-11"...
PKG_BUILD_DIR:=$(BUILD_DIR)/fakepop-10

include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/package.mk

define Package/fakepop
	SECTION:=net
	CATEGORY:=Network
	TITLE:=fakepop
	DEPENDS:=+glib2 +xinetd
endef

define Package/fakepop/description
	fake pop3 server which returns the same messages to all users
endef

define Package/fakepop/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fakepop $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/xinetd.d
	$(CP) ./files/fakepop $(1)/etc/xinetd.d/
	mkdir $(1)/etc/fakepop
endef

$(eval $(call BuildPackage,fakepop))
