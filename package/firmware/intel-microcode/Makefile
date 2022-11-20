#
# Copyright (C) 2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=intel-microcode
PKG_VERSION:=20220809
PKG_RELEASE:=1

PKG_SOURCE:=intel-microcode_3.$(PKG_VERSION).1.tar.xz
PKG_SOURCE_URL:=@DEBIAN/pool/non-free/i/intel-microcode/
PKG_HASH:=4cf6c3638bb52d9d45c1916af866fd0929628a6f459daac3edfd369149e9c665
PKG_BUILD_DIR:=$(BUILD_DIR)/intel-microcode-3.$(PKG_VERSION).1

PKG_BUILD_DEPENDS:=iucode-tool/host

ifdef CONFIG_TARGET_x86_64
	MICROCODE:="intel-microcode-64"
else
	MICROCODE:="intel-microcode"
endif

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/intel-microcode
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=$(PKG_SOURCE_URL)
  DEPENDS:=@TARGET_x86
  TITLE:=Intel x86 CPU microcode
endef

define Build/Compile
	IUCODE_TOOL=$(STAGING_DIR)/../host/bin/iucode_tool $(MAKE) -C $(PKG_BUILD_DIR)
	mkdir $(PKG_BUILD_DIR)/intel-ucode-ipkg
	$(STAGING_DIR)/../host/bin/iucode_tool -q \
		--write-firmware=$(PKG_BUILD_DIR)/intel-ucode-ipkg $(PKG_BUILD_DIR)/$(MICROCODE).bin
endef

define Package/intel-microcode/install
	$(INSTALL_DIR) $(1)/lib/firmware/intel-ucode
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/intel-ucode-ipkg/* $(1)/lib/firmware/intel-ucode
endef

$(eval $(call BuildPackage,intel-microcode))
