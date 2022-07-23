#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=k3wifi
PKG_VERSION:=1
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/k3wifi
  TITLE:=k3 wifi fw
  DEPENDS:=@TARGET_bcm53xx
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/k3wifi/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_BIN) ./files/brcmfmac4366c-pcie.bin $(1)/lib/firmware/brcm/
endef

$(eval $(call BuildPackage,k3wifi))
