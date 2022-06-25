# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2016 OpenWrt.org
#

include $(TOPDIR)/rules.mk

PKG_NAME:=arptables
PKG_VERSION:=0.0.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://ftp.netfilter.org/pub/arptables
PKG_HASH:=4f9a0656ce5c90868f551cd4deeb2d04f33899667e1fb2818b64e432fe8f629c

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/arptables-legacy
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE:=ARP firewalling software
  DEPENDS:=+kmod-arptables
  URL:=https://git.netfilter.org/arptables/
  PROVIDES:=arptables
  ALTERNATIVES:=\
    200:/usr/sbin/arptables:/usr/sbin/arptables-legacy
endef

MAKE_FLAGS += \
	COPT_FLAGS="$(TARGET_CFLAGS) -D__OPTIMIZE__=1" \
	KERNEL_DIR="$(LINUX_DIR)"

define Package/arptables-legacy/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/arptables-legacy $(1)/usr/sbin/arptables-legacy
endef

$(eval $(call BuildPackage,arptables-legacy))
