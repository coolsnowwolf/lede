#
# Copyright (C) 2019 Xue Liu <liuxuenetmail@gmail>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lora-gateway-hal
PKG_VERSION:=5.0.1
PKG_RELEASE:=2

PKG_SOURCE_URL:=https://codeload.github.com/Lora-net/lora_gateway/tar.gz/v$(PKG_VERSION)?
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=1a0447d5e8183d08e6dce5f739f6872b9c57824b98f4078830d5ee21b15782c1
PKG_MAINTAINER:=Xue Liu <liuxuenetmail@gmail.com>
PKG_LICENSE_FILES:=LICENSE
PKG_BUILD_DIR:=$(BUILD_DIR)/lora_gateway-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libloragw
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Driver/HAL library for Semtech SX1301
  URL:=https://www.semtech.com/products/wireless-rf/lora-gateways/sx1301
  DEPENDS:=+kmod-spi-dev @(!PACKAGE_lora-picogw-hal)
endef

define Package/libloragw/description
  Driver/HAL library for Semtech SX1301 multi-channel modem and
  SX1257/SX1255 RF transceivers.
endef

define Package/libloragw/config
	source "$(SOURCE)/Config.in"
endef

define Package/libloragw-tests
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=LoRaWAN
  TITLE:=Test programs for libloragw to check functionality
  DEPENDS:=+libloragw
endef

define Package/libloragw-utils
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=LoRaWAN
  TITLE:=Utility programs for libloragw
  DEPENDS:=+libloragw
endef

CMAKE_OPTIONS += \
		-DSPI_DEV_PATH:FILEPATH=$(CONFIG_SX1301_SPI_PATH) \
		-DSPI_SPEED:STRING=$(CONFIG_SX1301_SPI_SPEED) \
		-Dlora_gateway_build_shared_libs=ON

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/libloragw
	$(CP) $(PKG_BUILD_DIR)/libloragw/inc/loragw_* $(1)/usr/include/libloragw
	$(CP) $(PKG_BUILD_DIR)/libloragw/config.h $(1)/usr/include/libloragw
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libloragw/libloragw.so* $(1)/usr/lib/
	$(LN) libloragw.so.0 $(1)/usr/lib/libloragw.so
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_BUILD_DIR)/loragw.pc  $(1)/usr/lib/pkgconfig/
endef

define Package/libloragw/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libloragw/libloragw.so.* $(1)/usr/lib/
endef

define Package/libloragw-tests/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libloragw/test* $(1)/usr/sbin
endef

define Package/libloragw-utils/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/util_lbt_test $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/util_pkt_logger $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/util_spectral_scan $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/util_tx_continuous $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/util_spi_stress $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/util_tx_test $(1)/usr/sbin
endef

$(eval $(call BuildPackage,libloragw))
$(eval $(call BuildPackage,libloragw-tests))
$(eval $(call BuildPackage,libloragw-utils))
