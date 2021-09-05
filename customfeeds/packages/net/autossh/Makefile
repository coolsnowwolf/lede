#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=autossh
PKG_VERSION:=1.4g
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=https://www.harding.motd.ca/autossh/
PKG_HASH:=5fc3cee3361ca1615af862364c480593171d0c54ec156de79fc421e31ae21277
PKG_LICENSE:=0BSD

include $(INCLUDE_DIR)/package.mk

define Package/autossh
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Autossh client
  URL:=https://www.harding.motd.ca/autossh/
  MAINTAINER:=Christian Beier <cb@shoutrlabs.com>
  SUBMENU:=SSH
endef

define Build/Compile
	$(call Build/Compile/Default, -f Makefile \
		CFLAGS="$(TARGET_CFLAGS) -Wall -D\"SSH_PATH=\\\"\$$$$(SSH)\\\"\" -D\"VER=\\\"\$$$$(VER)\\\"\"" \
		all \
	)
endef

define Package/autossh/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/autossh $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/autossh.init $(1)/etc/init.d/autossh
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/autossh.config $(1)/etc/config/autossh
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) ./files/autossh.hotplug $(1)/etc/hotplug.d/iface/20-autossh
endef

define Package/autossh/conffiles
/etc/config/autossh
endef

$(eval $(call BuildPackage,autossh))
