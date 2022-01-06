include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-xlnetacc
PKG_VERSION:=1.0.5
PKG_RELEASE:=2

PKG_MAINTAINER:=Sense <sensec@gmail.com>

LUCI_TITLE:=LuCI Support for XLNetAcc
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+jshn +curl +openssl-util

define Package/$(PKG_NAME)/conffiles
/etc/config/xlnetacc
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
