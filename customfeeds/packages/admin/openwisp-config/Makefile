# openwisp.org
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
include $(TOPDIR)/rules.mk

PKG_NAME:=openwisp-config
PKG_SOURCE_VERSION:=0.5.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/openwisp/openwisp-config.git
PKG_MIRROR_HASH:=337a3a9542a0898da9f951256b0d19b6bc87ced98f4ec6dc9646172b551880ef
PKG_MAINTAINER:=Federico Capoano <f.capoano@openwisp.io>
PKG_LICENSE:=GPL3.0-or-later
PKGARCH:=all

include $(INCLUDE_DIR)/package.mk

define Package/openwisp-config/default
	TITLE:=Remote configuration management agent ($(2) variant)
	CATEGORY:=Administration
	SECTION:=admin
	SUBMENU:=openwisp
	DEPENDS:=+curl +lua +libuci-lua +luafilesystem +luci-lib-nixio $(3)
	VARIANT:=$(1)
	PKGARCH:=all
	URL:=http://openwisp.org
endef

Package/openwisp-config-openssl=$(call Package/openwisp-config/default,openssl,OpenSSL,+ca-certificates +libopenssl)
Package/openwisp-config-mbedtls=$(call Package/openwisp-config/default,mbedtls,mbedTLS,+ca-certificates +libmbedtls)
Package/openwisp-config-wolfssl=$(call Package/openwisp-config/default,wolfssl,WolfSSL,+ca-certificates +libwolfssl)
Package/openwisp-config-nossl=$(call Package/openwisp-config/default,nossl,No SSL)

define Build/Compile
endef

define Package/openwisp-config-$(BUILD_VARIANT)/conffiles
/etc/config/openwisp
endef

CONFIG_OPENWISP_UCI:=ssl

ifeq ($(BUILD_VARIANT),nossl)
    CONFIG_OPENWISP_UCI:=nossl
endif

define Package/openwisp-config-$(BUILD_VARIANT)/install
	$(INSTALL_DIR) \
		$(1)/usr/sbin \
		$(1)/etc/init.d \
		$(1)/etc/config \
		$(1)/etc/openwisp \
		$(1)/usr/lib/lua/openwisp

	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/openwisp-config/files/openwisp.agent \
		$(1)/usr/sbin/openwisp_config

	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/openwisp-config/files/openwisp.init \
		$(1)/etc/init.d/openwisp_config

	$(INSTALL_CONF) $(PKG_BUILD_DIR)/openwisp-config/files/openwisp-$(CONFIG_OPENWISP_UCI).config \
		$(1)/etc/config/openwisp

	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/openwisp-config/files/sbin/openwisp-reload-config \
		$(1)/usr/sbin/openwisp-reload-config

	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/openwisp-config/files/lib/openwisp/utils.lua \
		$(1)/usr/lib/lua/openwisp/utils.lua

	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/openwisp-config/files/lib/openwisp/net.lua \
		$(1)/usr/lib/lua/openwisp/net.lua

	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/openwisp-config/files/sbin/openwisp-store-unmanaged.lua \
		$(1)/usr/sbin/openwisp-store-unmanaged

	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/openwisp-config/files/sbin/openwisp-restore-unmanaged.lua \
		$(1)/usr/sbin/openwisp-restore-unmanaged

	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/openwisp-config/files/sbin/openwisp-remove-default-wifi.lua \
		$(1)/usr/sbin/openwisp-remove-default-wifi

	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/openwisp-config/files/sbin/openwisp-uci-autoname.lua \
		$(1)/usr/sbin/openwisp-uci-autoname

	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/openwisp-config/files/sbin/openwisp-update-config.lua \
		$(1)/usr/sbin/openwisp-update-config

	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/openwisp-config/files/sbin/openwisp-get-address.lua \
		$(1)/usr/sbin/openwisp-get-address

	$(CP) $(PKG_BUILD_DIR)/VERSION $(1)/etc/openwisp/
endef

$(eval $(call BuildPackage,openwisp-config-openssl))
$(eval $(call BuildPackage,openwisp-config-mbedtls))
$(eval $(call BuildPackage,openwisp-config-wolfssl))
$(eval $(call BuildPackage,openwisp-config-nossl))
