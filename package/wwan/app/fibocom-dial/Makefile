include $(TOPDIR)/rules.mk

PKG_NAME:=fibocom-dial
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/fibocom-dial
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Fibocom Dial
endef

define Package/fibocom-dial/description
	Fibocom Dial (Prints a snarky message)
endef

define Package/fibocom-dial/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fibocom-dial $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fibo_qmimsg_server $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/multi-pdn-manager $(1)/bin/
endef

$(eval $(call BuildPackage,fibocom-dial))
