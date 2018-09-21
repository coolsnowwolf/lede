include $(TOPDIR)/rules.mk

PKG_NAME:=dclient
PKG_VERSION=1.1
PKG_RELEASE:=170919
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=dclient -- Zte 802.1X Auth Client
	DEPENDS:=+libpthread +libstdcpp +libpcap +libcurl +libjpeg
	MAINTAINER:=www.zeyes.org
endef

define Package/$(PKG_NAME)/description
	ZTE 802.1X Auth Client For CCDGUT.
endef

define Build/Prepare
	echo "Here is Package/Prepare"
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	echo "Here is Package/install"
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/bin/
endef

$(eval $(call BuildPackage,dclient))
