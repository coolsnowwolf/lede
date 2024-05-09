#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_VERSION:=20240315
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=default
PKG_SOURCE:=ophub-uboot-prebuilt-git-$(PKG_VERSION).tar.gz
PKG_SOURCE_VERSION:=abe491ab386607f9ab0d66728e5766bc5d7e8a20
PKG_SOURCE_URL_FILE:=$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/ophub/u-boot/archive/
PKG_HASH:=3f98f5728f48d13f33cf4fd21fb6032a625ad3c61aa2e8073dda821f71f067ec

PKG_MAINTAINER:=ophub

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

TAR_OPTIONS:=--strip-components 1 $(TAR_OPTIONS)
TAR_CMD=$(HOST_TAR) -C $(1) $(TAR_OPTIONS)

define U-Boot/Default
  BUILD_TARGET:=amlogic
  UENV:=default
  HIDDEN:=1
  DEFAULT:=y
endef

define U-Boot/phicomm-n1
  NAME:=Phicomm N1
  OVERLAY:=u-boot-n1.bin
  BUILD_SUBTARGET:=mesongx
  BUILD_DEVICES:=phicomm_n1
endef

UBOOT_TARGETS := phicomm-n1

define Build/Configure
	true
endef

define Build/Compile
	true
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/u-boot/amlogic/overload/$(OVERLAY) $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot-overlay.bin
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
