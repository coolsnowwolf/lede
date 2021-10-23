include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-jd-dailybonus
PKG_VERSION:=1.0.6
PKG_RELEASE:=20211016

LUCI_TITLE:=Luci for JD dailybonus Script
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+node +wget +lua +libuci-lua

define Package/$(PKG_NAME)/conffiles
/etc/config/jd-dailybonus
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
