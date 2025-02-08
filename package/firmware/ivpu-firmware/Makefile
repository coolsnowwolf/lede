#
# Copyright (C) 2022 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ivpu-firmware
PKG_VERSION:=1.10.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL_FILE:=v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/intel/linux-npu-driver/archive/refs/tags
PKG_HASH:=a756de38cde7bd5a0402854d08429d271a33784835b80db3be672ade80570900
PKG_BUILD_DIR:=$(BUILD_DIR)/linux-npu-driver-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/ivpu-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Intel VPU firmware
  URL:=$(PKG_SOURCE_URL)
  DEPENDS:=
endef

define Build/Compile
	true
endef

define Package/ivpu-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/intel/vpu
	$(INSTALL_DATA) \
	        $(PKG_BUILD_DIR)/firmware/bin/*.bin \
	        $(1)/lib/firmware/intel/vpu
endef

$(eval $(call BuildPackage,ivpu-firmware))
