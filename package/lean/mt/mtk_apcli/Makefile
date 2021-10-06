
include $(TOPDIR)/rules.mk

PKG_NAME:=mtk_apcli
PKG_RELEASE:=2

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/mtk_apcli
	SECTION:=net
  CATEGORY:=Network
	TITLE:=apcli for MTK
	DEPENDS:=+libiwinfo
endef

define Package/mtk_apcli/description
	apcli for MTK
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/mtk_apcli/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/apcli $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/apcli_2g $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/apcli_5g $(1)/usr/bin/
endef

$(eval $(call BuildPackage,mtk_apcli))
