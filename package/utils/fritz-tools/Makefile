include $(TOPDIR)/rules.mk

PKG_NAME:=fritz-tools
PKG_RELEASE:=1
CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/fritz-tools/Default
  SECTION:=utils
  CATEGORY:=Utilities
endef

define Package/fritz-tffs
  $(call Package/fritz-tools/Default)
  TITLE:=Utility to partially read the TFFS filesystems
endef

define Package/fritz-tffs/description
 Utility to partially read the TFFS filesystems.
endef

define Package/fritz-tffs-nand
  $(call Package/fritz-tools/Default)
  TITLE:=Utility to partially read the TFFS filesystems on NAND flash
endef

define Package/fritz-tffs-nand/description
 Utility to partially read the TFFS filesystems on NAND flash.
endef

define Package/fritz-caldata
  $(call Package/fritz-tools/Default)
  DEPENDS:=+zlib
  TITLE:=Utility to extract WLAN calibration data
endef

define Package/fritz-caldata/description
 Utility to extract the zlib compress calibration data from flash.
endef

define Package/fritz-tffs/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fritz_tffs_read $(1)/usr/bin/fritz_tffs
endef

define Package/fritz-tffs-nand/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fritz_tffs_nand_read $(1)/usr/bin/fritz_tffs_nand
endef

define Package/fritz-caldata/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fritz_cal_extract $(1)/usr/bin/
endef

$(eval $(call BuildPackage,fritz-tffs))
$(eval $(call BuildPackage,fritz-tffs-nand))
$(eval $(call BuildPackage,fritz-caldata))
