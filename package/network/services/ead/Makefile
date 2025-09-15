#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ead
PKG_RELEASE:=1

PKG_BUILD_DEPENDS:=libpcap
PKG_BUILD_DIR:=$(BUILD_DIR)/ead

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/ead
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=Emergency Access Daemon
  URL:=http://bridge.sourceforge.net/
endef

define Package/ead/description
  Provides remote access to your device even if IP and firewall
  configuration settings are defunct
endef

CONFIGURE_PATH = tinysrp

TARGET_CFLAGS += \
	-I$(PKG_BUILD_DIR) \
	-I$(PKG_BUILD_DIR)/tinysrp \
	$(TARGET_CPPFLAGS)

MAKE_FLAGS += \
	CONFIGURE_ARGS="$(CONFIGURE_ARGS)" \
	LIBS_EADCLIENT="$(PKG_BUILD_DIR)/tinysrp/libtinysrp.a" \
	LIBS_EAD="$(PKG_BUILD_DIR)/tinysrp/libtinysrp.a $(STAGING_DIR)/usr/lib/libpcap.a" \
	CFLAGS="$(TARGET_CFLAGS)"

define Package/ead/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ead $(1)/sbin/
endef

$(eval $(call BuildPackage,ead))
