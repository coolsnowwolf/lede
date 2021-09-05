#
# Copyright (C) 2012-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=acpid
PKG_VERSION:=2.0.32
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@SF/acpid2
PKG_HASH:=f2d2d30b3edc3234bd82f6f7186699a6aa3c85c8d20bc4e30e9b3c68a1ed157e

PKG_MAINTAINER:=Thomas Heil <heil@terminal-consulting.de>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:tedfelix:acpid

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/acpid
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=The ACPI Daemon (acpid) With Netlink Support
  URL:=https://sourceforge.net/projects/acpid2/
  DEPENDS:=+kmod-input-evdev
endef

define Package/acpid/description
  The ACPI Daemon (acpid) With Netlink Support
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

define Package/acpid/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/acpid $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kacpimon $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/acpi_listen $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/acpi/events
	$(INSTALL_CONF) ./files/default $(1)/etc/acpi/events/default
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_CONF) ./files/acpid.init $(1)/etc/init.d/acpid
	chmod 0750 $(1)/etc/init.d/acpid
	$(INSTALL_DIR) $(1)/etc/hotplug.d/input/
	$(INSTALL_CONF) ./files/acpid.hotplug $(1)/etc/hotplug.d/input/
endef

define Package/acpid/postinst
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] || {
	echo "waiting for input devices to come up"
	/etc/init.d/acpid enable
	sleep 5
	/etc/init.d/acpid start
	echo "please try the power button"
	exit 0
}
endef

$(eval $(call BuildPackage,acpid))
