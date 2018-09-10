#
# Copyright (C) 2013-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=2018.03
PKG_RELEASE:=1

PKG_HASH:=7e7477534409d5368eb1371ffde6820f0f79780a1a1f676161c48442cb303dfd

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=imx6
  UBOOT_IMAGE:=u-boot.imx
endef

define U-Boot/mx6cuboxi
   NAME:=SolidRun Cubox-i boards
   UBOOT_IMAGE:=SPL u-boot.img
   UBOOT_MAKE_FLAGS:=SPL u-boot.img
   BUILD_DEVICES:=cubox-i
   HIDDEN:=1
endef

define U-Boot/mx6sabresd
  NAME:=SABRE i.MX6Quad board
endef

define U-Boot/nitrogen6dl
  NAME:=Nitrogen6x i.MX6Dual-Lite 1GB board
endef

define U-Boot/nitrogen6dl2g
  NAME:=Nitrogen6x i.MX6Dual-Lite 2GB board
endef

define U-Boot/nitrogen6q
  NAME:=Nitrogen6x/SABRE Lite (MX6Q/1GB)
endef

define U-Boot/nitrogen6q2g
  NAME:=Nitrogen6x i.MX6Quad 2GB board
endef

define U-Boot/nitrogen6s
  NAME:=Nitrogen6x i.MX6Solo 512MB board
endef

define U-Boot/nitrogen6s1g
  NAME:=Nitrogen6x i.MX6Solo 1GB board
endef

define U-Boot/wandboard
  NAME:=Wandboard Dual Lite/Quad/Solo
  BUILD_DEVICES:=wandboard
endef

UBOOT_TARGETS := \
	mx6cuboxi \
	mx6sabresd \
	nitrogen6dl \
	nitrogen6dl2g \
	nitrogen6q \
	nitrogen6q2g \
	nitrogen6s \
	nitrogen6s1g \
	wandboard

UBOOT_MAKE_FLAGS += u-boot.imx

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(foreach img,$(UBOOT_IMAGE), \
		$(CP) $(PKG_BUILD_DIR)/$(img) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-$(img); \
	)
endef

$(eval $(call BuildPackage/U-Boot))
