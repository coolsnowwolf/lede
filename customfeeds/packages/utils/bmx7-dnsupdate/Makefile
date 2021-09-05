include $(TOPDIR)/rules.mk

PKG_NAME:=bmx7-dnsupdate
PKG_VERSION:=0.1
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/bmx7-dnsupdate
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=bmx7-dnsupdate
  MAINTAINER:=Paul Spooren <spooren@informatik.uni-leipzig.de>
  DEPENDS:=+bmx7 +bmx7-json inotifywait jshn
  PKGARCH:=all
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/bmx7-dnsupdate/install
	$(CP) files/* $(1)/
endef

$(eval $(call BuildPackage,bmx7-dnsupdate))
