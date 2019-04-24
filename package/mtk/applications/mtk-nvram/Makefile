#
# hua.shao@mediatek.com
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtk-nvram
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:=NVRAM_MTD_NAME

PKG_MAINTAINER:=Hua Shao <nossiac@163.com>

include $(INCLUDE_DIR)/package.mk

define Package/nvram
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Applications
  TITLE:=MTK APSoC nvram tool
  DEPENDS:=+libnvram
  MENU:=1
endef

define Package/nvram/config
	source "$(SOURCE)/config.in"
endef

define Package/flash
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Applications
  TITLE:=MTK APSoC flash R/W tool
  DEPENDS:=+libnvram
endef

define Package/libnvram
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Libraries
  TITLE:=MTK APSoC nvram library
  DEPENDS:=
endef

define Package/nvram/description
  MTK nvram tool
endef

define Package/flash/description
  flash r/w tool
endef

define Package/libnvram/description
  MTK nvram library
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(call Build/Prepare/Default)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP)   ./src/nvram.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP)	$(PKG_BUILD_DIR)/lib*.so $(1)/usr/lib/
endef


MAKE_FLAGS += \
	CONFIG_NVRAM_MTD_NAME=$(CONFIG_NVRAM_MTD_NAME)


define Package/nvram/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nvram $(1)/usr/bin
endef

define Package/flash/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/flash $(1)/usr/bin
endef

define Package/libnvram/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.so $(1)/usr/lib
endef


$(eval $(call BuildPackage,libnvram))
$(eval $(call BuildPackage,nvram))
$(eval $(call BuildPackage,flash))

