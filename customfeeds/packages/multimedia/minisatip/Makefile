#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=minisatip
PKG_VERSION:=1.0.4
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/catalinii/minisatip/tar.gz/$(PKG_VERSION)?
PKG_HASH:=aa6ef68228a19faf04cf1283e994cae80134a01d9ee6bc45b9969d145b1281e5

PKG_MAINTAINER:=Daniel Kucera <github@danman.eu>
PKG_LICENSE:=GPL-2.0-or-later

PKG_BUILD_PARALLEL:=1
PKG_CONFIG_DEPENDS:= \
	CONFIG_BUILD_PATENTED \
	CONFIG_MINISATIP_AES \
	CONFIG_MINISATIP_CLIENT

include $(INCLUDE_DIR)/package.mk

define Package/minisatip
  TITLE:=SATIP server
  URL:=https://github.com/catalinii/minisatip
  SECTION:=multimedia
  CATEGORY:=Multimedia
  DEPENDS:=+MINISATIP_AES:libopenssl +BUILD_PATENTED:libdvbcsa
endef

define Package/minisatip/config
	if PACKAGE_minisatip

		config MINISATIP_AES
			bool "OpenSSL (AES as part of DVBAPI)"
			default n

		config MINISATIP_CLIENT
			bool "SatIP Client"
			default y

	endif
endef

CONFIGURE_ARGS += \
	--$(if $(CONFIG_BUILD_PATENTED),en,dis)able-dvbcsa \
	--$(if $(CONFIG_MINISATIP_AES),en,dis)able-dvbaes \
	--$(if $(CONFIG_MINISATIP_CLIENT),en,dis)able-satipc

define Package/minisatip/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/minisatip $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/minisatip/html/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/html/* $(1)/usr/share/minisatip/html
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/minisatip.init $(1)/etc/init.d/minisatip
endef

$(eval $(call BuildPackage,minisatip))
