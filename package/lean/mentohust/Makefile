#
# Copyright (C) 2006-2011 Xmlad.com
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mentohust
PKG_VERSION:=4.0
PKG_RELEASE:=1
PKG_REV=857f5a1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-r$(PKG_REV).tar.bz2
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_URL:=https://github.com/hyrathb/mentohust.git

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/mentohust
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=+libpcap
	TITLE:=An CERNET client daemon
	URL:=https://github.com/hyrathb/mentohust
	SUBMENU:=CERNET
endef

define Package/mentohust/description
An CERNET client daemon,
Most usually used in China collages.
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	# $(SED) 's/dhclient/udhcpc -i/g' $(PKG_BUILD_DIR)/src/myconfig.c
endef

define Build/Configure
	cd $(PKG_BUILD_DIR) && sh ./autogen.sh
	$(call Build/Configure/Default)
endef

CONFIGURE_ARGS += \
	--disable-encodepass \
	--disable-notify

# XXX: CFLAGS are already set by Build/Compile/Default
MAKE_FLAGS+= \
	OFLAGS=""

define Package/mentohust/conffiles
/etc/mentohust.conf
endef

define Package/mentohust/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mentohust $(1)/usr/sbin/
	chmod 755 $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/mentohust.conf $(1)/etc/mentohust.conf.template
endef

$(eval $(call BuildPackage,mentohust))
