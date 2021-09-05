#
# Copyright (C) 2009-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=transmission
PKG_VERSION:=2.94
PKG_RELEASE:=8

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GITHUB/transmission/transmission-releases/master
PKG_HASH:=35442cc849f91f8df982c3d0d479d650c6ca19310a994eccdaa79a4af3916b7d

PKG_MAINTAINER:=Rosen Penev <rosenp@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:transmissionbt:transmission

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/transmission/template
  SUBMENU:=BitTorrent
  SECTION:=net
  CATEGORY:=Network
  TITLE:=BitTorrent client
  URL:=https://www.transmissionbt.com
  DEPENDS:=+libcurl +libevent2 +libminiupnpc +libnatpmp +libpthread +librt +zlib
endef

define Package/transmission-daemon/Default
  $(call Package/transmission/template)
  USERID:=transmission=224:transmission=224
endef

define Package/transmission-daemon-openssl
  $(call Package/transmission-daemon/Default)
  TITLE+= (with OpenSSL)
  DEPENDS+=+libopenssl
  VARIANT:=openssl
endef

define Package/transmission-daemon-mbedtls
  $(call Package/transmission-daemon/Default)
  TITLE+= (with mbed TLS)
  DEPENDS+=+libmbedtls
  VARIANT:=mbedtls
endef

define Package/transmission-cli-openssl
  $(call Package/transmission/template)
  TITLE+= (with OpenSSL)
  DEPENDS+=+libopenssl
  VARIANT:=openssl
endef

define Package/transmission-cli-mbedtls
  $(call Package/transmission/template)
  TITLE+= (with mbed TLS)
  DEPENDS+=+libmbedtls
  VARIANT:=mbedtls
endef

define Package/transmission-remote-openssl
  $(call Package/transmission/template)
  TITLE+= (with OpenSSL)
  DEPENDS+=+libopenssl
  VARIANT:=openssl
endef

define Package/transmission-remote-mbedtls
  $(call Package/transmission/template)
  TITLE+= (with mbed TLS)
  DEPENDS+=+libmbedtls
  VARIANT:=mbedtls
endef

define Package/transmission-web
  $(call Package/transmission/template)
  TITLE+= (webinterface)
  DEPENDS:=+transmission-daemon-openssl
  PKGARCH:=all
endef


define Package/transmission-daemon/Default/description
 Transmission is a simple BitTorrent client.
 It features a very simple, intuitive interface
 on top on an efficient, cross-platform back-end.
 This package contains the daemon itself.
endef
Package/transmission-daemon-openssl/description = $(Package/transmission-daemon/Default/description)
Package/transmission-daemon-mbedtls/description = $(Package/transmission-daemon/Default/description)

define Package/transmission-cli/Default/description
 CLI utilities for transmission.
endef
Package/transmission-cli-openssl/description = $(Package/transmission-cli/Default/description)
Package/transmission-cli-mbedtls/description = $(Package/transmission-cli/Default/description)

define Package/transmission-remote/Default/description
 CLI remote interface for transmission.
endef
Package/transmission-remote-openssl/description = $(Package/transmission-remote/Default/description)
Package/transmission-remote-mbedtls/description = $(Package/transmission-remote/Default/description)

define Package/transmission-web/description
 Webinterface resources for transmission.
endef

define Package/transmission-daemon-openssl/conffiles
/etc/config/transmission
endef
Package/transmission-daemon-mbedtls/conffiles = $(Package/transmission-daemon-openssl/conffiles)

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections -Wl,--as-needed

CONFIGURE_ARGS += \
	--enable-cli \
	--enable-daemon \
	--enable-external-natpmp \
	--enable-largefile \
	--enable-lightweight \
	--without-gtk \
	--without-kqueue \
	--without-systemd-daemon

ifeq ($(BUILD_VARIANT),mbedtls)
  CONFIGURE_ARGS += --with-crypto=polarssl
  CONFIGURE_VARS += \
	MBEDTLS_CFLAGS="-I$(STAGING_DIR)/usr/include/mbedtls" \
	MBEDTLS_LIBS="-lmbedtls -lmbedcrypto"
else
  CONFIGURE_ARGS += --with-crypto=openssl
endif

define Package/transmission-daemon-openssl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/transmission-daemon $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) files/transmission.init $(1)/etc/init.d/transmission
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) files/transmission.config $(1)/etc/config/transmission
	$(INSTALL_DIR) $(1)/etc/sysctl.d/
	$(INSTALL_CONF) files/transmission.sysctl $(1)/etc/sysctl.d/20-transmission.conf
endef
Package/transmission-daemon-mbedtls/install = $(Package/transmission-daemon-openssl/install)

define Package/transmission-cli-openssl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/transmission-cli \
			$(PKG_INSTALL_DIR)/usr/bin/transmission-create \
			$(PKG_INSTALL_DIR)/usr/bin/transmission-edit \
			$(PKG_INSTALL_DIR)/usr/bin/transmission-show \
			$(1)/usr/bin/
endef
Package/transmission-cli-mbedtls/install = $(Package/transmission-cli-openssl/install)

define Package/transmission-remote-openssl/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/transmission-remote $(1)/usr/bin/
endef
Package/transmission-remote-mbedtls/install = $(Package/transmission-remote-openssl/install)

define Package/transmission-web/install
	$(INSTALL_DIR) $(1)/usr/share/transmission
	$(CP) $(PKG_INSTALL_DIR)/usr/share/transmission/web $(1)/usr/share/transmission/
endef

$(eval $(call BuildPackage,transmission-daemon-openssl))
#$(eval $(call BuildPackage,transmission-daemon-mbedtls))
$(eval $(call BuildPackage,transmission-cli-openssl))
#$(eval $(call BuildPackage,transmission-cli-mbedtls))
$(eval $(call BuildPackage,transmission-remote-openssl))
#$(eval $(call BuildPackage,transmission-remote-mbedtls))
$(eval $(call BuildPackage,transmission-web))
