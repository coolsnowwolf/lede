# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

PKG_NAME:=bcm4908img
PKG_RELEASE:=1

PKG_FLAGS:=nonshared

PKG_BUILD_DEPENDS := bcm4908img/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/bcm4908img
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Utility handling BCM4908 images
  MAINTAINER:=Rafał Miłecki <rafal@milecki.pl>
  DEPENDS:=@TARGET_bcm4908
endef

define Package/bcm4908img/description
  CFE bootloader for BCM4908 uses custom image format. It consists of:
  1. Optional cferom image
  2. bootfs JFFS2 partition (cferam, kernel, DTB and optional helper files)
  3. padding
  4. rootfs simply appended to the bootfs + padding
  5. tail with checksum and basic device info

  This util allows creating, modifying and extracting from BCM4908 images.
endef

define Host/Prepare
  $(CP) ./src/* $(HOST_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall"
endef

define Package/bcm4908img/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bcm4908img $(1)/usr/bin/
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bcm4908img $(STAGING_DIR_HOST)/bin/
endef

$(eval $(call BuildPackage,bcm4908img))
$(eval $(call HostBuild))
