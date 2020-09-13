#
# Copyright 2017 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ls-dpl
PKG_VERSION:=LSDK-20.04
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://source.codeaurora.org/external/qoriq/qoriq-components/mc-utils
PKG_SOURCE_VERSION:=8672a5f5abcd3a354dcab07e03f2a8a69b2e962d
PKG_MIRROR_HASH:=4b8ad3148aee1e0c034206543472aebb435655fd03a661c4c1be545dcac7ddf0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/layerscape-dpl
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=NXP DPL firmware
  DEPENDS:=@TARGET_layerscape
endef

MAKE_PATH:=config
MAKE_VARS+= \
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH)

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/config/ls1088a/RDB/dpl-eth.0x1D_0x0D.dtb \
		$(STAGING_DIR_IMAGE)/fsl_ls1088a-rdb-dpl.dtb
	$(CP) $(PKG_BUILD_DIR)/config/ls1088a/RDB/dpc.0x1D-0x0D.dtb \
		$(STAGING_DIR_IMAGE)/fsl_ls1088a-rdb-dpc.dtb
	$(CP) $(PKG_BUILD_DIR)/config/ls2088a/RDB/dpl-eth.0x2A_0x41.dtb \
		$(STAGING_DIR_IMAGE)/fsl_ls2088a-rdb-dpl.dtb
	$(CP) $(PKG_BUILD_DIR)/config/ls2088a/RDB/dpc.0x2A_0x41.dtb \
		$(STAGING_DIR_IMAGE)/fsl_ls2088a-rdb-dpc.dtb
endef

$(eval $(call BuildPackage,layerscape-dpl))
