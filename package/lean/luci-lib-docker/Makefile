include $(TOPDIR)/rules.mk

PKG_NAME:=luci-lib-docker
PKG_LICENSE:=AGPL-3.0
PKG_VERSION:=v0.3.4
PKG_MAINTAINER:=lisaac <lisaac.cn@gmail.com> \
	Florian Eckert <fe@dev.tdt.de>

LUCI_TYPE:=col

LUCI_TITLE:=LuCI library for docker
LUCI_DESCRIPTION:=Docker Engine API for LuCI

LUCI_DEPENDS:=@(aarch64||arm||x86_64) +luci-lib-jsonc
LUCI_PKGARCH:=all

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
