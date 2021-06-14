#
# Copyright (C) 2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=armbian-firmware
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2021-05-05
PKG_SOURCE_URL:=https://github.com/armbian/firmware.git
PKG_SOURCE_VERSION:=761658e1701c77a0a84706754e6db1a25ee60b82
PKG_MIRROR_HASH:=b2db0195617a21ac8db4ce73c03ebec7feefa8df550c2da25544013ae5a32499

include $(INCLUDE_DIR)/package.mk

define Package/armbian-firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=https://github.com/armbian/firmware
endef

define Build/Compile
	true
endef

define Package/brcmfmac-firmware-43456-sdio
  $(Package/armbian-firmware-default)
  TITLE:=Broadcom BCM43456 FullMac SDIO firmware
endef

define Package/brcmfmac-firmware-43456-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43456-sdio.bin \
		$(1)/lib/firmware/brcm/brcmfmac43456-sdio.bin
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43456-sdio))

define Package/brcmfmac-nvram-43456-sdio
  $(Package/armbian-firmware-default)
  TITLE:=Broadcom BCM43456 NVRAM firmware
endef

define Package/brcmfmac-nvram-43456-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43456-sdio.txt \
		$(1)/lib/firmware/brcm/brcmfmac43456-sdio.txt
endef
$(eval $(call BuildPackage,brcmfmac-nvram-43456-sdio))
