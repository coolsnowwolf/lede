#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=swconfig
PKG_RELEASE:=12

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/swconfig
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libuci +libnl-tiny
  TITLE:=Switch configuration utility
endef

TARGET_CFLAGS += -flto
TARGET_LDFLAGS += -flto=jobserver

TARGET_CPPFLAGS := \
	-D_GNU_SOURCE \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(PKG_BUILD_DIR) \
	$(TARGET_CPPFLAGS) \
	-I$(LINUX_DIR)/user_headers/include

define Build/Compile
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		LIBS="$(TARGET_LDFLAGS) -lnl-tiny -lm -luci -lubox"
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/swlib.h $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libsw.a $(1)/usr/lib/
endef

define Package/swconfig/install
	$(INSTALL_DIR) $(1)/sbin $(1)/lib/network
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/swconfig $(1)/sbin/swconfig
	$(INSTALL_DATA) ./files/switch.sh $(1)/lib/network/
endef

$(eval $(call BuildPackage,swconfig))
