# Copyright (C) 2020 Stijn Tintel <stijn@linux-ipv6.be>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=readsb
PKG_VERSION:=3.9.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/Mictronics/readsb/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=234e2f5f45d69d8be376272e9a08be54bdc48ed5ad7ebad2c4374bbd754d9426

PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING LICENSE

PKG_MAINTAINER:=Stijn Tintel <stijn@linux-ipv6.be>

PKG_BUILD_DEPENDS:=!USE_GLIBC:argp-standalone
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/readsb/default
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libncurses
  TITLE:=Mode-S/ADSB/TIS decoder for various devices
  URL:=https://github.com/Mictronic/readsb
endef

define Package/readsb
  $(call Package/readsb/default)
  TITLE+= (readsb)
  DEPENDS+=+librtlsdr
endef

define Package/viewadsb
  $(call Package/readsb/default)
  TITLE+= (viewadsb)
endef

MAKE_FLAGS += \
	BLADERF=no \
	CFLAGS="$(TARGET_CFLAGS)" \
	RTLSDR=yes \
	UNAME="Linux"

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += \
	$(if $(CONFIG_USE_GLIBC),,-largp) \
	-Wl,--gc-sections,--as-needed

define Package/readsb/conffiles
/etc/config/readsb
endef

define Package/readsb/install
	$(INSTALL_DIR) $(1)/etc/{config,init.d} $(1)/usr/bin
	$(INSTALL_CONF) ./files/readsb.config $(1)/etc/config/readsb
	$(INSTALL_BIN) ./files/readsb.init $(1)/etc/init.d/readsb
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/readsb $(1)/usr/bin
endef

define Package/viewadsb/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/viewadsb $(1)/usr/bin
endef

$(eval $(call BuildPackage,readsb))
$(eval $(call BuildPackage,viewadsb))
