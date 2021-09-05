#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fakeidentd
PKG_VERSION:=2.6
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).c
PKG_SOURCE_URL:=http://distfiles.gentoo.org/distfiles/
PKG_HASH:=75b321e5d9dddcea2b38035b62ac30f2175ed43f3f187be6da19aaa67c4fe165
PKG_MAINTAINER:=Daniel Gimpelevich <daniel@gimpelevich.san-francisco.ca.us>
PKG_LICENSE:=GPL-2.0-or-later
UNPACK_CMD=$(CP) $(DL_DIR)/$(PKG_SOURCE) $(1)/

include $(INCLUDE_DIR)/package.mk

define Package/fakeidentd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A static, secure identd.
  URL:=http://www.guru-group.fi/~too/sw/releases/
endef

define Package/fakeidentd/description
	A static secure identd, only one source file.
endef

define Build/Compile
	$(SHELL) "$(PKG_BUILD_DIR)/$(MAKE_PATH)/$(PKG_SOURCE)" \
	CC="$(TARGET_CC)" TRG="$(PKG_BUILD_DIR)/$(MAKE_PATH)/$(PKG_NAME)" \
	$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(TARGET_CPPFLAGS) $(EXTRA_CPPFLAGS) \
	$(TARGET_LDFLAGS) $(EXTRA_LDFLAGS) -DUSE_UNIX_OS
endef

define Package/fakeidentd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
endef

$(eval $(call BuildPackage,fakeidentd))
