#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=brcmfmac-firmware-4366c0-pcie-vendor
PKG_VERSION:=1
PKG_RELEASE:=4

include $(INCLUDE_DIR)/package.mk

define Package/brcmfmac-firmware-4366c0-pcie-vendor
	SECTION:=firmware
	CATEGORY:=Firmware
	TITLE:=Broadcom 4366c0 FullMac PCIe firmware from vendor
	CONFLICTS:=brcmfmac-firmware-4366c0-pcie
endef

define Build/Compile
	true
endef

define Package/brcmfmac-firmware-4366c0-pcie-vendor/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) ./files/brcmfmac4366c-pcie.bin $(1)/lib/firmware/brcm/brcmfmac4366c-pcie.bin
endef

$(eval $(call BuildPackage,brcmfmac-firmware-4366c0-pcie-vendor))
