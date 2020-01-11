#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cypress-nvram
PKG_SOURCE_DATE:=2019-09-03
PKG_SOURCE_VERSION:=e7b78df22f2a0c5f56abb7b5880661611de35e5f
PKG_MIRROR_HASH:=1cb20a749696852be0a512d51961365dd9c031362af0af1a2b9f5a3fb894885f
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/openwrt/cypress-nvram.git

PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/cypress-nvram-default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=https://community.cypress.com/community/linux
endef

define Build/Compile
	true
endef

# Cypress 43430 SDIO Raspberry Pi 3B NVRAM
define Package/cypress-nvram-43430-sdio-rpi-3b
  $(Package/cypress-nvram-default)
  TITLE:=CYW43430 NVRAM for Raspberry Pi 3B
  DEPENDS:=@TARGET_brcm2708
endef

define Package/cypress-nvram-43430-sdio-rpi-3b/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcmfmac43430-sdio.raspberrypi,3-model-b.txt \
		$(1)/lib/firmware/brcm/brcmfmac43430-sdio.raspberrypi,3-model-b.txt
endef

$(eval $(call BuildPackage,cypress-nvram-43430-sdio-rpi-3b))

# Cypress 43430 SDIO Raspberry Pi Zero W NVRAM
define Package/cypress-nvram-43430-sdio-rpi-zero-w
  $(Package/cypress-nvram-default)
  TITLE:=CYW43430 NVRAM for Raspberry Pi Zero W
  DEPENDS:=@TARGET_brcm2708
endef

define Package/cypress-nvram-43430-sdio-rpi-zero-w/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcmfmac43430-sdio.raspberrypi,model-zero-w.txt \
		$(1)/lib/firmware/brcm/brcmfmac43430-sdio.raspberrypi,model-zero-w.txt
endef

$(eval $(call BuildPackage,cypress-nvram-43430-sdio-rpi-zero-w))

# Cypress 43455 SDIO Raspberry Pi 3B+ NVRAM
define Package/cypress-nvram-43455-sdio-rpi-3b-plus
  $(Package/cypress-nvram-default)
  TITLE:=CYW43455 NVRAM for Raspberry Pi 3B+
  DEPENDS:=@TARGET_brcm2708
endef

define Package/cypress-nvram-43455-sdio-rpi-3b-plus/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcmfmac43455-sdio.raspberrypi,3-model-b-plus.txt \
		$(1)/lib/firmware/brcm/brcmfmac43455-sdio.raspberrypi,3-model-b-plus.txt
endef

$(eval $(call BuildPackage,cypress-nvram-43455-sdio-rpi-3b-plus))

# Cypress 43455 SDIO Raspberry Pi 4B NVRAM
define Package/cypress-nvram-43455-sdio-rpi-4b
  $(Package/cypress-nvram-default)
  TITLE:=CYW43455 NVRAM for Raspberry Pi 4B
  DEPENDS:=@TARGET_brcm2708
endef

define Package/cypress-nvram-43455-sdio-rpi-4b/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcmfmac43455-sdio.raspberrypi,4-model-b.txt \
		$(1)/lib/firmware/brcm/brcmfmac43455-sdio.raspberrypi,4-model-b.txt
endef

$(eval $(call BuildPackage,cypress-nvram-43455-sdio-rpi-4b))
