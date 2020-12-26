#
# Copyright (C) 2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=amd64-microcode
PKG_VERSION:=20191218
PKG_RELEASE:=1

PKG_SOURCE:=amd64-microcode_3.$(PKG_VERSION).$(PKG_RELEASE).tar.xz
PKG_SOURCE_URL:=http://ftp.debian.org/debian/pool/non-free/a/amd64-microcode/
PKG_HASH:=f469b79348097c5f04641b67a39d0ee5a2a1916c9556281626c04f2275d4132d
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-3.$(PKG_VERSION).$(PKG_RELEASE)

PKG_LICENSE_FILE:=LICENSE.amd-ucode

include $(INCLUDE_DIR)/package.mk

define Package/amd64-microcode
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=$(PKG_SOURCE_URL)
  DEPENDS:=@TARGET_x86
  TITLE:=AMD64 CPU microcode
endef

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	$(TAR) -C $(BUILD_DIR) -xJf $(DL_DIR)/$(PKG_SOURCE)
endef

define Build/Compile
endef

define Package/amd64-microcode/install
	$(INSTALL_DIR) $(1)/lib/firmware/amd-ucode
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/*.bin $(1)/lib/firmware/amd-ucode
endef

$(eval $(call BuildPackage,amd64-microcode))
