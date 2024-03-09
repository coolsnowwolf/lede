# SPDX-License-Identifier: GPL-2.0-or-later

include $(TOPDIR)/rules.mk

PKG_NAME:=cypress-firmware
PKG_VERSION:=5.10.9-2022_0909
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/Infineon/ifx-linux-firmware/
PKG_MIRROR_HASH:=944faae3a80013f1a963b6692d7f50a38c97edcf91fd163de521df755e6922b5
PKG_SOURCE_VERSION:=release-v$(PKG_VERSION)

PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>
PKG_LICENSE_FILES:=LICENCE

include $(INCLUDE_DIR)/package.mk

define Package/cypress-firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=https://community.infineon.com/
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
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43012-sdio.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43012-sdio.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac43012-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac43012-sdio.bin
	$(LN) \
		../cypress/cyfmac43012-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac43012-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-43012-sdio))

# Cypress 43340 SDIO Firmware
define Package/cypress-firmware-43340-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW43340 FullMac SDIO firmware
endef

define Package/cypress-firmware-43340-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43340-sdio.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac43340-sdio.bin \
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
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43362-sdio.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac43362-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac43362-sdio.bin
endef

$(eval $(call BuildPackage,cypress-firmware-43362-sdio))

# Cypress 4339 SDIO Firmware
define Package/cypress-firmware-4339-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW4339 FullMac SDIO firmware
  PROVIDES:=brcmfmac-firmware-4339-sdio
  CONFLICTS:=brcmfmac-firmware-4339-sdio
endef

define Package/cypress-firmware-4339-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4339-sdio.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac4339-sdio.bin \
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
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43430-sdio.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43430-sdio.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac43430-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac43430-sdio.bin
	$(LN) \
		../cypress/cyfmac43430-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac43430-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-43430-sdio))

# Cypress 43439 SDIO Firmware
define Package/cypress-firmware-43439-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW43439 FullMac SDIO firmware
endef

define Package/cypress-firmware-43439-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43439-sdio.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43439-sdio.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac43439-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac43439-sdio.bin
	$(LN) \
		../cypress/cyfmac43439-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac43439-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-43439-sdio))

# Cypress 43455 SDIO Firmware
define Package/cypress-firmware-43455-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW43455 FullMac SDIO firmware
  PROVIDES:=brcmfmac-firmware-43455-sdio
  CONFLICTS:=brcmfmac-firmware-43455-sdio
endef

define Package/cypress-firmware-43455-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43455-sdio.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43455-sdio.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac43455-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac43455-sdio.bin
	$(LN) \
		../cypress/cyfmac43455-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac43455-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-43455-sdio))

# Cypress 4354 SDIO Firmware
define Package/cypress-firmware-4354-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW4354 FullMac SDIO firmware
endef

define Package/cypress-firmware-4354-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4354-sdio.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4354-sdio.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac4354-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac4354-sdio.bin
	$(LN) \
		../cypress/cyfmac4354-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac4354-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-4354-sdio))

# Cypress 4356 PCIe Firmware
define Package/cypress-firmware-4356-pcie
  $(Package/cypress-firmware-default)
  TITLE:=CYW4356 FullMac PCIe firmware
endef

define Package/cypress-firmware-4356-pcie/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4356-pcie.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4356-pcie.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac4356-pcie.bin \
		$(1)/lib/firmware/brcm/brcmfmac4356-pcie.bin
	$(LN) \
		../cypress/cyfmac4356-pcie.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac4356-pcie.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-4356-pcie))

# Cypress 4356 SDIO Firmware
define Package/cypress-firmware-4356-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW4356 FullMac SDIO firmware
endef

define Package/cypress-firmware-4356-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4356-sdio.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4356-sdio.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac4356-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac4356-sdio.bin
	$(LN) \
		../cypress/cyfmac4356-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac4356-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-4356-sdio))

# Cypress 43570 PCIe Firmware
define Package/cypress-firmware-43570-pcie
  $(Package/cypress-firmware-default)
  TITLE:=CYW43570 FullMac PCIe firmware
endef

define Package/cypress-firmware-43570-pcie/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43570-pcie.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac43570-pcie.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac43570-pcie.bin \
		$(1)/lib/firmware/brcm/brcmfmac43570-pcie.bin
	$(LN) \
		../cypress/cyfmac43570-pcie.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac43570-pcie.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-43570-pcie))

# Cypress 4373 PCIe Firmware
define Package/cypress-firmware-4373-pcie
  $(Package/cypress-firmware-default)
  TITLE:=CYW4373 FullMac PCIe firmware
endef

define Package/cypress-firmware-4373-pcie/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4373-pcie.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4373-pcie.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac4373-pcie.bin \
		$(1)/lib/firmware/brcm/brcmfmac4373-pcie.bin
	$(LN) \
		../cypress/cyfmac4373-pcie.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac4373-pcie.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-4373-pcie))

# Cypress 4373 SDIO Firmware
define Package/cypress-firmware-4373-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW4373 FullMac SDIO firmware
endef

define Package/cypress-firmware-4373-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4373-sdio.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4373-sdio.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac4373-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac4373-sdio.bin
	$(LN) \
		../cypress/cyfmac4373-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac4373-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-4373-sdio))

# Cypress 4373 USB Firmware
define Package/cypress-firmware-4373-usb
  $(Package/cypress-firmware-default)
  TITLE:=CYW4373 FullMac USB firmware
endef

define Package/cypress-firmware-4373-usb/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4373-usb.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac4373.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac4373-usb.bin \
		$(1)/lib/firmware/brcm/brcmfmac4373-usb.bin
	$(LN) \
		../cypress/cyfmac4373.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac4373.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-4373-usb))

# Cypress 54591 PCIe Firmware
define Package/cypress-firmware-54591-pcie
  $(Package/cypress-firmware-default)
  TITLE:=CYW54591 FullMac PCIe firmware
endef

define Package/cypress-firmware-54591-pcie/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac54591-pcie.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac54591-pcie.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac54591-pcie.bin \
		$(1)/lib/firmware/brcm/brcmfmac54591-pcie.bin
	$(LN) \
		../cypress/cyfmac54591-pcie.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac54591-pcie.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-54591-pcie))

# Cypress 54591 SDIO Firmware
define Package/cypress-firmware-54591-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW54591 FullMac SDIO firmware
endef

define Package/cypress-firmware-54591-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac54591-sdio.bin \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac54591-sdio.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac54591-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac54591-sdio.bin
	$(LN) \
		../cypress/cyfmac54591-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac54591-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-54591-sdio))

# Cypress 55560 PCIe Firmware
define Package/cypress-firmware-55560-pcie
  $(Package/cypress-firmware-default)
  TITLE:=CYW55560 FullMac PCIe firmware
endef

define Package/cypress-firmware-55560-pcie/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac55560-pcie.trxse \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac55560-pcie.trxse \
		$(1)/lib/firmware/brcm/brcmfmac55560-pcie.trxse
endef

$(eval $(call BuildPackage,cypress-firmware-55560-pcie))

# Cypress 55572 PCIe Firmware
define Package/cypress-firmware-55572-pcie
  $(Package/cypress-firmware-default)
  TITLE:=CYW55572 FullMac PCIe firmware
endef

define Package/cypress-firmware-55572-pcie/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac55572-pcie.trxse \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac55572-pcie.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac55572-pcie.trxse \
		$(1)/lib/firmware/brcm/brcmfmac55572-pcie.trxse
	$(LN) \
		../cypress/cyfmac55572-pcie.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac55572-pcie.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-55572-pcie))

# Cypress 55572 SDIO Firmware
define Package/cypress-firmware-55572-sdio
  $(Package/cypress-firmware-default)
  TITLE:=CYW55572 FullMac SDIO firmware
endef

define Package/cypress-firmware-55572-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/cypress
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac55572-sdio.trxse \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/firmware/cyfmac55572-sdio.clm_blob \
		$(1)/lib/firmware/cypress/
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(LN) \
		../cypress/cyfmac55572-sdio.trxse \
		$(1)/lib/firmware/brcm/brcmfmac55572-sdio.trxse
	$(LN) \
		../cypress/cyfmac55572-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac55572-sdio.clm_blob
endef

$(eval $(call BuildPackage,cypress-firmware-55572-sdio))
