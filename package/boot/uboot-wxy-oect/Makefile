#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=uboot-wxy-oect
PKG_RELEASE:=1

PKG_MAINTAINER:=lean
PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/uboot-wxy-oect
  SECTION:=boot
  CATEGORY:=Boot Loaders
  TITLE:=Prebuilt U-Boot for WXY OEC / OECT
  DEPENDS:=@TARGET_rockchip_armv8
  HIDDEN:=1
  DEFAULT:=y if TARGET_rockchip_armv8_DEVICE_wxy_oec-turbo-4g
endef

define Build/Prepare
	true
endef

define Build/Compile
	true
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) ./bootloader/* $(STAGING_DIR_IMAGE)/
endef

define Package/uboot-wxy-oect/install
	true
endef

$(eval $(call BuildPackage,uboot-wxy-oect))
