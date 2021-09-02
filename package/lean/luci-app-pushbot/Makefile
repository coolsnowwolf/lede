include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-pushbot
PKG_VERSION:=3.12
PKG_RELEASE:=10

PKG_MAINTAINER:=tty228  zzsj0928

LUCI_TITLE:=LuCI support for Pushbot
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+iputils-arping +curl

define Package/$(PKG_NAME)/conffiles
/etc/config/pushbot
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
