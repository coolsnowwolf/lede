include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-jd-dailybonus
PKG_VERSION:=1.0.5
PKG_RELEASE:=20210316

define Package/$(PKG_NAME)/conffiles
/etc/config/jd-dailybonus
endef

LUCI_TITLE:=Luci for JD dailybonus Script
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+node +wget-ssl +lua +curl +libuci-lua

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
