#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uboot-layerscape-32bit
PKG_VERSION:=v1.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_MIRROR_HASH:=a29edbeddde4b3007ac349c2a5ee434719049ac4753e459dbb1bab21e1c585da
PKG_SOURCE_URL:=https://github.com/fsl-jyt/uboot-ls-32b.git
PKG_SOURCE_VERSION:=6476195551ea83193afd06d0c441483c04729c8d

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

PKG_MAINTAINER:=Jiang Yutang <jiangyutang1978@gmail.com>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/uboot-layerscape-32b
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_layerscape_32b
  TITLE:=32-bit U-Boot for Layerscape boards
  DEFAULT:=y
  HIDDEN:=1
endef

define Build/Configure

endef

define Build/Compile

endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/ls1043ardb-uboot.bin $(STAGING_DIR_IMAGE)/ls1043ardb-32b-uboot.bin
	$(CP) $(PKG_BUILD_DIR)/ls1046ardb-uboot.bin $(STAGING_DIR_IMAGE)/ls1046ardb-32b-uboot.bin
	$(CP) $(PKG_BUILD_DIR)/ls1012ardb-uboot.bin.swap $(STAGING_DIR_IMAGE)/ls1012ardb-32b-uboot.bin
endef

$(eval $(call BuildPackage,uboot-layerscape-32b))
