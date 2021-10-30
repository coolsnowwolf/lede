include $(TOPDIR)/rules.mk

PKG_NAME:=mtk_apcli
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/mtk_apcli
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Wireless
  TITLE:=apcli for MTK
  DEPENDS:=@TARGET_ramips +libiwinfo
endef

define Package/mtk_apcli/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/apcli $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/apcli_2g $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/apcli_5g $(1)/usr/bin/
endef

$(eval $(call BuildPackage,mtk_apcli))
