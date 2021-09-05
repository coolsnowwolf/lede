#
# Copyright (C) 2012-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=aria2
PKG_VERSION:=1.35.0
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/aria2/aria2/releases/download/release-$(PKG_VERSION)/
PKG_HASH:=1e2b7fd08d6af228856e51c07173cfcf987528f1ac97e04c5af4a47642617dfd
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

PKG_MAINTAINER:=Imre Kaloz <kaloz@openwrt.org>, \
	Hsing-Wang Liao <kuoruan@gmail.com>
PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:tatsuhiro_tsujikawa:aria2

PKG_CONFIG_DEPENDS := \
	CONFIG_ARIA2_NOSSL \
	CONFIG_ARIA2_OPENSSL \
	CONFIG_ARIA2_GNUTLS \
	CONFIG_ARIA2_NOCRYPTO \
	CONFIG_ARIA2_NETTLE \
	CONFIG_ARIA2_LIBGCRYPT \
	CONFIG_ARIA2_LIBXML2 \
	CONFIG_ARIA2_EXPAT \
	CONFIG_ARIA2_GMP \
	CONFIG_ARIA2_BITTORRENT \
	CONFIG_ARIA2_METALINK \
	CONFIG_ARIA2_SFTP \
	CONFIG_ARIA2_ASYNC_DNS \
	CONFIG_ARIA2_COOKIE \
	CONFIG_ARIA2_WEBSOCKET

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/aria2/config
  source "$(SOURCE)/Config.in"
endef

define Package/aria2
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=File Transfer
  TITLE:=lightweight download utility
  URL:=https://aria2.github.io/
  DEPENDS:=+zlib +libstdcpp +ARIA2_OPENSSL:libopenssl +ARIA2_GNUTLS:libgnutls \
	+ARIA2_NETTLE:libnettle +ARIA2_LIBGCRYPT:libgcrypt +ARIA2_GMP:libgmp \
	+ARIA2_LIBXML2:libxml2 +ARIA2_EXPAT:libexpat +ARIA2_SFTP:libssh2 \
	+ARIA2_ASYNC_DNS:libcares +ARIA2_COOKIE:libsqlite3
  USERID:=aria2=6800:aria2=6800
endef

define Package/aria2/description
  aria2 is a lightweight multi-protocol & multi-source command-line download
  utility
endef

CONFIGURE_ARGS += \
	--disable-nls \
	$(if $(CONFIG_ARIA2_NOSSL),--disable,--enable)-ssl \
	$(if $(CONFIG_ARIA2_BITTORRENT),--enable,--disable)-bittorrent \
	$(if $(CONFIG_ARIA2_METALINK),--enable,--disable)-metalink \
	$(if $(CONFIG_ARIA2_WEBSOCKET),--enable,--disable)-websocket \
	$(if $(CONFIG_ARIA2_OPENSSL),--with,--without)-openssl \
	$(if $(CONFIG_ARIA2_GNUTLS),--with,--without)-gnutls \
	$(if $(CONFIG_ARIA2_NETTLE),--with,--without)-libnettle \
	$(if $(CONFIG_ARIA2_LIBGCRYPT),--with,--without)-libgcrypt \
	$(if $(CONFIG_ARIA2_GMP),--with,--without)-libgmp \
	$(if $(CONFIG_ARIA2_LIBXML2),--with,--without)-libxml2 \
	$(if $(CONFIG_ARIA2_EXPAT),--with,--without)-libexpat \
	$(if $(CONFIG_ARIA2_SFTP),--with,--without)-libssh2 \
	$(if $(CONFIG_ARIA2_ASYNC_DNS),--with,--without)-libcares \
	$(if $(CONFIG_ARIA2_COOKIE),--with,--without)-sqlite3 \
	--without-libuv \
	--with-libz

TARGET_CXXFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections -flto

define Package/aria2/conffiles
/etc/config/aria2
endef

define Package/aria2/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/aria2c $(1)/usr/bin

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/aria2.init $(1)/etc/init.d/aria2

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/aria2.conf $(1)/etc/config/aria2
endef

$(eval $(call BuildPackage,aria2))
