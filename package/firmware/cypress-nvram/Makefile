# SPDX-License-Identifier: GPL-2.0-or-later

include $(TOPDIR)/rules.mk

PKG_NAME:=cypress-nvram
PKG_RELEASE:=7

PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/cypress-nvram-default
  SECTION:=firmware
  CATEGORY:=Firmware
endef

define Build/Compile
	true
endef

# Cypress 4339 SDIO NVRAM
define Package/cypress-nvram-4339-sdio
  $(Package/cypress-nvram-default)
  TITLE:=BCM4339 SDIO NVRAM
  CONFLICTS:=brcmfmac-nvram-4339-sdio
endef

define Package/cypress-nvram-4339-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		./files/brcmfmac4339-sdio.AP6335.txt \
		$(1)/lib/firmware/brcm/
	$(LN) \
		brcmfmac4339-sdio.AP6335.txt \
		$(1)/lib/firmware/brcm/brcmfmac4339-sdio.technexion,imx7d-pico-pi.txt
endef

$(eval $(call BuildPackage,cypress-nvram-4339-sdio))
