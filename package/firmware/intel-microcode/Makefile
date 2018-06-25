#
# Copyright (C) 2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=intel-microcode
PKG_VERSION:=20180312
PKG_RELEASE:=1

PKG_SOURCE:=intel-microcode_3.$(PKG_VERSION).$(PKG_RELEASE).tar.xz
PKG_SOURCE_URL:=http://ftp.debian.org/debian/pool/non-free/i/intel-microcode/
PKG_HASH:=6ccb295d23961c7b96a69280e30fdce939e1d905147b22b8428886b173812d52
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-3.$(PKG_VERSION).$(PKG_RELEASE)

PKG_BUILD_DEPENDS:=iucode-tool/host

ifdef CONFIG_TARGET_x86_64
	MICROCODE:="intel-microcode-64"
else
	MICROCODE:="intel-microcode"
endif

include $(INCLUDE_DIR)/package.mk

define Package/intel-microcode
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=$(PKG_SOURCE_URL)
  DEPENDS:=@TARGET_x86 +iucode-tool
  TITLE:=Intel x86 CPU microcode
endef

define Build/Compile
	IUCODE_TOOL=$(STAGING_DIR)/../host/bin/iucode_tool $(MAKE) -C $(PKG_BUILD_DIR)
	mkdir $(PKG_BUILD_DIR)/intel-ucode
	$(STAGING_DIR)/../host/bin/iucode_tool -q \
		--write-firmware=$(PKG_BUILD_DIR)/intel-ucode $(PKG_BUILD_DIR)/$(MICROCODE).bin
endef

define Package/intel-microcode/install
	$(INSTALL_DIR) $(1)/lib/firmware/intel-ucode
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/intel-ucode/* $(1)/lib/firmware/intel-ucode
endef

$(eval $(call BuildPackage,intel-microcode))
