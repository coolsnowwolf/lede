include $(TOPDIR)/rules.mk

PKG_NAME:=fibocom-dial
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/fibocom-dial
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Fibocom Dial
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/fibocom-dial/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fibocom-dial $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fibo_qmimsg_server $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/multi-pdn-manager $(1)/usr/bin
endef

$(eval $(call BuildPackage,fibocom-dial))
