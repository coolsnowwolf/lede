#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=brcmfmac43569-firmware
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	TITLE:=brcmfmac43569-firmware
	DEPENDS:=+kmod-mwifiex-usb
	URL:=
endef

define Package/$(PKG_NAME)/description
  brcmfmac43569-firmware fw
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) ./files/lib/firmware/brcm/brcmfmac43569.bin $(1)/lib/firmware/brcm/brcmfmac43569.bin
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DIR) $(1)/lib/firmware/mrvl
	$(INSTALL_DATA) ./files/lib/firmware/mrvl/usb8797_uapsta.bin $(1)/lib/firmware/mrvl/usb8797_uapsta.bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
