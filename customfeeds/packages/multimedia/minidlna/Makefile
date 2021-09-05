#
# Copyright (C) 2010-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=minidlna
PKG_VERSION:=1.3.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/minidlna
PKG_HASH:=47d9b06b4c48801a4c1112ec23d24782728b5495e95ec2195bbe5c81bc2d3c63

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-or-later BSD-3-Clause
PKG_LICENSE_FILES:=COPYING LICENCE.miniupnpd

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/minidlna
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=UPnP A/V & DLNA Media Server
  URL:=http://minidlna.sourceforge.net/
  DEPENDS:= +libpthread +libexif +libjpeg +libsqlite3 +libffmpeg \
  	+libid3tag +libflac +libvorbis $(ICONV_DEPENDS) $(INTL_DEPENDS)
  USERID:=minidlna:minidlna
endef

define Package/minidlna/description
  MiniDLNA (aka ReadyDLNA) is server software with the aim of
  being fully compliant with DLNA/UPnP-AV clients.
endef

define Package/minidlna/conffiles
/etc/config/minidlna
endef

CONFIGURE_ARGS += \
	--with-libiconv-prefix="$(ICONV_PREFIX)" \
	--with-libintl-prefix="$(INTL_PREFIX)" \
	--with-os-name="OpenWrt Linux" \
	--with-os-version="$(LINUX_VERSION)" \
	--with-os-url="https://openwrt.org/" \
	--with-db-path="/var/run/minidlna" \
	--with-log-path="/var/log"

define Package/minidlna/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/minidlnad $(1)/usr/sbin/minidlnad
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/minidlna.init $(1)/etc/init.d/minidlna
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/minidlna.config $(1)/etc/config/minidlna
	$(INSTALL_DIR) $(1)/etc/sysctl.d
	$(INSTALL_CONF) ./files/minidlna.sysctl $(1)/etc/sysctl.d/30-minidlna.conf
endef

$(eval $(call BuildPackage,minidlna))
