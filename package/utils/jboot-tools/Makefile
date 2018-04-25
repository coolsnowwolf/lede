include $(TOPDIR)/rules.mk

PKG_NAME:=jboot-tools
PKG_RELEASE:=1
CMAKE_INSTALL:=1
PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/jboot-tools
  SECTION:=firmware
  CATEGORY:=Firmware
  DEPENDS:=@TARGET_ramips
  TITLE:=Utilites for accessing JBOOT based D-Link devices Calibration data
endef

define Package/jboot-tools/description
 This package contains:
 jboot_config_read.c: partially read the config partition of JBOOT based D-Link devices.
endef

define Package/jboot-tools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/jboot_config_read $(1)/usr/bin/
endef

$(eval $(call BuildPackage,jboot-tools))
