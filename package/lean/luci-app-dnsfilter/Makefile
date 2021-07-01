include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-dnsfilter
include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for DNSFilter
LUCI_DEPENDS:=+uclient-fetch +ipset +dnsmasq-full
LUCI_PKGARCH:=all
PKG_NAME:=luci-app-dnsfilter
PKG_VERSION:=1
PKG_RELEASE:=3

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
