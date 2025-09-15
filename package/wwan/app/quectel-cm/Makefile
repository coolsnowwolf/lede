include $(TOPDIR)/rules.mk

PKG_NAME:=quectel-cm
PKG_VERSION:=1.6.7
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/quectel-open-source/Quectel_QConnectManager_Linux
PKG_SOURCE_VERSION:=4dd7951dd9cbd078fbe50221d93ed098f7e38289
PKG_MIRROR_HASH:=9c864c75b7ff0e3e60c5d515ab4521194b0c6642eb636e88f6e0ace630b9c54b

include $(INCLUDE_DIR)/package.mk

define Package/quectel-cm
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Quectel dial program
  DEPENDS:=+libpthread
endef

define Package/quectel-cm/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/quectel-CM $(1)/usr/bin
endef

$(eval $(call BuildPackage,quectel-cm))
