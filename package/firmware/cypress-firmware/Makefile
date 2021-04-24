#
# Copyright (C) 2019-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

UNPACK_CMD=unzip -q -p $(DL_DIR)/$(PKG_SOURCE) $(PKG_SOURCE_UNZIP) | gzip -dc | $(HOST_TAR) -C $(1) $(TAR_OPTIONS)

PKG_NAME:=cypress-firmware
PKG_VERSION:=v5.4.18-2021_0114
PKG_RELEASE:=1

PKG_SOURCE_UNZIP:=cypress-firmware-$(PKG_VERSION).tar.gz
PKG_SOURCE:=cypress-fmac-$(PKG_VERSION).zip
PKG_SOURCE_URL:=https://community.cypress.com/gfawx74859/attachments/gfawx74859/WiFiBluetoothLinux/1738/1/
PKG_HASH:=a3e914ed8e3b2e5057cdd3c6ab09ff2771a745bd767ba7aada33efe872b57899

PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/cypress-firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=https://community.cypress.com/community/linux
endef

define Build/Compile
	true
endef

# Cypress 43012 SDIO Firmware
define Package/cypress-firmware-43012-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW43012 FullMac SDIO firmware
endef

define Package/cypress-firmware-43012-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43012-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac43012-sdio.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43012-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac43012-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-43012-sdio))

# Cypress 43340 SDIO Firmware
define Package/cypress-firmware-43340-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW43340 FullMac SDIO firmware
endef

define Package/cypress-firmware-43340-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43340-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac43340-sdio.bin
endef

$(eval $(call BuildPackage,cypress-firmware-43340-sdio))

# Cypress 43362 SDIO Firmware
define Package/cypress-firmware-43362-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW43362 FullMac SDIO firmware
  PROVIDES:=brcmfmac-firmware-43362-sdio
  CONFLICTS:=brcmfmac-firmware-43362-sdio
endef

define Package/cypress-firmware-43362-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43362-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac43362-sdio.bin
endef

$(eval $(call BuildPackage,cypress-firmware-43362-sdio))

# Cypress 4339 SDIO Firmware
define Package/cypress-firmware-4339-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW4339 FullMac SDIO firmware
endef

define Package/cypress-firmware-4339-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4339-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac4339-sdio.bin
endef

$(eval $(call BuildPackage,cypress-firmware-4339-sdio))

# Cypress 43430 SDIO Firmware
define Package/cypress-firmware-43430-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW43430 FullMac SDIO firmware
  PROVIDES:=brcmfmac-firmware-43430-sdio
  CONFLICTS:=brcmfmac-firmware-43430-sdio
endef

define Package/cypress-firmware-43430-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43430-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac43430-sdio.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43430-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac43430-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-43430-sdio))

# Cypress 43455 SDIO Firmware
define Package/cypress-firmware-43455-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW43455 FullMac SDIO firmware
  PROVIDES:=brcmfmac-firmware-43455-sdio
  CONFLICTS:=brcmfmac-firmware-43455-sdio
endef

define Package/cypress-firmware-43455-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43455-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac43455-sdio.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43455-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac43455-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-43455-sdio))

# Cypress 4354 SDIO Firmware
define Package/cypress-firmware-4354-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW4354 FullMac SDIO firmware
endef

define Package/cypress-firmware-4354-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4354-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac4354-sdio.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4354-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac4354-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-4354-sdio))

# Cypress 4356 PCIe Firmware
define Package/cypress-firmware-4356-pcie
  $(Package/cypress-firmware-default)
  TITLE:=CYW4356 FullMac PCIe firmware
endef

define Package/cypress-firmware-4356-pcie/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4356-pcie.bin \
		$(1)/lib/firmware/brcm/brcmfmac4356-pcie.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4356-pcie.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac4356-pcie.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-4356-pcie))

# Cypress 4356 SDIO Firmware
define Package/cypress-firmware-4356-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW4356 FullMac SDIO firmware
endef

define Package/cypress-firmware-4356-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4356-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac4356-sdio.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4356-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac4356-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-4356-sdio))

# Cypress 43570 PCIe Firmware
define Package/cypress-firmware-43570-pcie
  $(Package/cypress-firmware-default)
  TITLE:=CYW43570 FullMac PCIe firmware
endef

define Package/cypress-firmware-43570-pcie/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43570-pcie.bin \
		$(1)/lib/firmware/brcm/brcmfmac43570-pcie.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43570-pcie.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac43570-pcie.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-43570-pcie))

# Cypress 4373 SDIO Firmware
define Package/cypress-firmware-4373-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW4373 FullMac SDIO firmware
endef

define Package/cypress-firmware-4373-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4373-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac4373-sdio.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4373-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac4373-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-4373-sdio))

# Cypress 4373 USB Firmware
define Package/cypress-firmware-4373-usb
  $(Package/cypress-firmware-default)
  TITLE:=CYW4373 FullMac USB firmware
endef

define Package/cypress-firmware-4373-usb/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4373-usb.bin \
		$(1)/lib/firmware/brcm/brcmfmac4373-usb.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4373.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac4373.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-4373-usb))

# Cypress 54591 PCIe Firmware
define Package/cypress-firmware-54591-pcie
  $(Package/cypress-firmware-default)
  TITLE:=CYW54591 FullMac PCIe firmware
endef

define Package/cypress-firmware-54591-pcie/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac54591-pcie.bin \
		$(1)/lib/firmware/brcm/brcmfmac54591-pcie.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac54591-pcie.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac54591-pcie.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-54591-pcie))

