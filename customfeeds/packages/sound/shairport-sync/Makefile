#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
# updated to work with latest source from abrasive
#

include $(TOPDIR)/rules.mk

PKG_NAME:=shairport-sync
PKG_VERSION:=3.3.8
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/mikebrady/shairport-sync/tar.gz/$(PKG_VERSION)?
PKG_HASH:=c92f9a2d86dd1138673abc66e0010c94412ad6a46da8f36c3d538f4fa6b9faca

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>, \
		Mike Brady <mikebrady@eircom.net>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING LICENSES shairport.c

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/shairport-sync/default
  SECTION:=sound
  CATEGORY:=Sound
  TITLE:=AirPlay compatible audio player
  DEPENDS:=@AUDIO_SUPPORT +libpthread +alsa-lib +libconfig +libdaemon +libpopt
  PROVIDES:=shairport-sync
  URL:=https://github.com/mikebrady/shairport-sync
endef

define Package/shairport-sync-openssl
  $(Package/shairport-sync/default)
  TITLE+= (openssl)
  DEPENDS+= +libopenssl +libavahi-client +libsoxr
  VARIANT:=openssl
endef

define Package/shairport-sync-mbedtls
  $(Package/shairport-sync/default)
  TITLE+= (mbed TLS)
  DEPENDS+= +libmbedtls +libavahi-client +libsoxr
  VARIANT:=mbedtls
  DEFAULT_VARIANT:=1
endef

define Package/shairport-sync-mini
  $(Package/shairport-sync/default)
  TITLE+= (minimal)
  DEPENDS+= +libmbedtls
  VARIANT:=mini
endef

define Package/shairport-sync/default/description
  Shairport Sync plays audio from iTunes and AirPlay sources, including
  iOS devices, Quicktime Player and third party sources such as forkedDaapd.
  Audio played by a Shairport Sync-powered device stays synchronised with the source
  and hence with similar devices playing the same source.

  Shairport Sync does not support AirPlay video or photo streaming.
  Ensure Kernel Modules > Sound Support > kmod-sound-core is selected.
  Also select kmod-usb-audio if you want to use USB-connected sound cards.
endef
Package/shairport-sync-openssl/description = $(Package/shairport-sync/default/description)
Package/shairport-sync-mbedtls/description = $(Package/shairport-sync/default/description)

define Package/shairport-sync-mini/description
  $(Package/shairport-sync/default/description)

  Minimal version uses mbed TLS and does not include libsoxr and avahi support.
endef

CONFIGURE_ARGS += \
	--with-alsa \
	--with-libdaemon \
	--with-metadata

ifeq ($(BUILD_VARIANT),openssl)
  CONFIGURE_ARGS+= --with-ssl=openssl
endif

ifeq ($(BUILD_VARIANT),mbedtls)
  CONFIGURE_ARGS+= --with-ssl=mbedtls
endif

ifeq ($(BUILD_VARIANT),mini)
  CONFIGURE_ARGS+= --with-ssl=mbedtls --with-tinysvcmdns
else
  CONFIGURE_ARGS+= --with-avahi --with-soxr
endif

define Package/shairport-sync/default/conffiles
/etc/config/shairport-sync
/etc/shairport-sync.conf
endef

Package/shairport-sync-openssl/conffiles = $(Package/shairport-sync/default/conffiles)
Package/shairport-sync-mbedtls/conffiles = $(Package/shairport-sync/default/conffiles)
Package/shairport-sync-mini/conffiles = $(Package/shairport-sync/default/conffiles)

define Package/shairport-sync/default/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/shairport-sync $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/shairport-sync.conf $(1)/etc/shairport-sync.conf
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/shairport-sync.init $(1)/etc/init.d/shairport-sync
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/shairport-sync.config $(1)/etc/config/shairport-sync
endef

Package/shairport-sync-openssl/install = $(Package/shairport-sync/default/install)
Package/shairport-sync-mbedtls/install = $(Package/shairport-sync/default/install)
Package/shairport-sync-mini/install = $(Package/shairport-sync/default/install)

$(eval $(call BuildPackage,shairport-sync-openssl))
$(eval $(call BuildPackage,shairport-sync-mbedtls))
$(eval $(call BuildPackage,shairport-sync-mini))
