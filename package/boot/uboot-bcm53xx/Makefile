include $(TOPDIR)/rules.mk

PKG_VERSION:=2025.10
PKG_HASH:=b4f032848e56cc8f213ad59f9132c084dbbb632bc29176d024e58220e0efdf4a
PKG_RELEASE:=1

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=bcm53xx
  BUILD_SUBTARGET:=generic
  UBOOT_CONFIG:=bcmns
  UBOOT_BOARD:=$(1)
endef

define U-Boot/dir-885l
  NAME:=D-Link DIR-885L
  BUILD_DEVICES:=dlink_dir-885l
endef

define U-Boot/dir-890l
  NAME:=D-Link DIR-890L
  BUILD_DEVICES:=dlink_dir-890l
endef

UBOOT_TARGETS := dir-885l dir-890l

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(STAGING_DIR_IMAGE)/$(BUILD_DEVICES)-u-boot.bin
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
