include $(TOPDIR)/rules.mk

PKG_NAME:=static-neighbor-reports
PKG_VERSION:=1
PKG_RELEASE:=2

PKG_MAINTAINER:=David Bauer <mail@david-bauer.net>
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

Build/Compile=

define Package/static-neighbor-reports/Default
endef

define Package/static-neighbor-reports
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Configure static 802.11k neighbor reports
  PKGARCH:=all
  DEPENDS:=+libuci-lua +libubus-lua +lua
endef

define Package/static-neighbor-reports/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/static-neighbor-report.conf $(1)/etc/config/static-neighbor-report
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/static-neighbor-reports.init $(1)/etc/init.d/static-neighbor-reports
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) ./files/reload-neighbor-reports.lua $(1)/usr/lib/reload-neighbor-reports.lua
endef

define Package/static-neighbor-reports/conffiles
/etc/config/static-neighbor-report
endef

define Package/static-neighbor-reports/description
  Allows to configure static neighbor reports for hostapd interfaces.

  These neighbor reports are avilable to clients using IEEE 802.11k.
endef

$(eval $(call BuildPackage,static-neighbor-reports))
