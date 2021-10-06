include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-jd-dailybonus
PKG_VERSION:=1.0.5
PKG_RELEASE:=20210316

PKG_MAINTAINER:=jerrykuku <jerrykuku@qq.com>

LUCI_TITLE:=Luci for JD dailybonus Script
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+node +uclient-fetch +lua +libuci-lua

define Package/$(PKG_NAME)/conffiles
/etc/config/jd-dailybonus
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
