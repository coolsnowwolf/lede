#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=chrony
PKG_VERSION:=4.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://download.tuxfamily.org/chrony/
PKG_HASH:=ed76f2d3f9347ac6221a91ad4bd553dd0565ac188cd7490d0801d08f7171164c

PKG_MAINTAINER:=Miroslav Lichvar <mlichvar0@gmail.com>
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:tuxfamily:chrony

PKG_BUILD_DEPENDS:=pps-tools

include $(INCLUDE_DIR)/package.mk

define Package/chrony/Default
  SUBMENU:=Time Synchronization
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libcap +libpthread
  USERID:=chrony=323:chrony=323
  TITLE:=A versatile NTP client and server
  URL:=http://chrony.tuxfamily.org/
  PROVIDES:=nts
endef

define Package/chrony
$(call Package/chrony/Default)
  TITLE+= (without NTS)
  VARIANT:=normal
endef

define Package/chrony-nts
$(call Package/chrony/Default)
  TITLE+= (with NTS)
  DEPENDS+= +libgnutls +ca-bundle
  VARIANT:=with-nts
endef

define Package/chrony/description
	An NTP client and server designed to perform well in a wide range
	of conditions. It can synchronize the system clock with NTP servers,
	reference clocks, and manual input using wristwatch and keyboard.
endef

Package/chrony-nts/description = $(Package/chrony/description)

define Package/chrony/conffiles
/etc/chrony/chrony.conf
/etc/config/chrony
endef

Package/chrony-nts/conffiles = $(Package/chrony/conffiles)

CONFIGURE_ARGS+= \
	--host-machine=$(shell echo $(GNU_TARGET_NAME) | sed -e 's/-.*//') \
	--host-release="" \
	--host-system=Linux \
	--sysconfdir=/etc/chrony \
	--prefix=/usr \
	--chronyrundir=/var/run/chrony \
	$(if $(findstring normal,$(BUILD_VARIANT)),--disable-nts,--enable,nts) \
	--disable-readline \
	--disable-rtc \
	--disable-sechash \
	--with-user=chrony

CONFIGURE_VARS+=CPPFLAGS=-DNDEBUG

define Package/chrony/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/chronyd $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/chronyc $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/chrony
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/chrony.hotplug $(1)/etc/hotplug.d/iface/20-chrony
	$(INSTALL_BIN) ./files/chronyd.init $(1)/etc/init.d/chronyd
	$(INSTALL_CONF) ./files/chrony.config $(1)/etc/config/chrony
	$(INSTALL_CONF) ./files/chrony.conf $(1)/etc/chrony/chrony.conf
endef

Package/chrony-nts/install= $(Package/chrony/install)

$(eval $(call BuildPackage,chrony))
$(eval $(call BuildPackage,chrony-nts))
