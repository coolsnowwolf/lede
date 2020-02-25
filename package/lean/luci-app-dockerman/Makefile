include $(TOPDIR)/rules.mk

LUCI_TITLE:=Docker Manager interface for LuCI
LUCI_DEPENDS:=+luci-lib-docker +docker-ce +e2fsprogs +fdisk
PKG_NAME:=luci-app-dockerman
PKG_VERSION:=v0.3.0
PKG_RELEASE:=beta
PKG_MAINTAINER:=lisaac <https://github.com/lisaac/luci-app-dockerman>
PKG_LICENSE:=AGPL-3.0

include $(TOPDIR)/feeds/luci/luci.mk

define Package/$(PKG_NAME)/postinst
#!/bin/sh
rm -rf /tmp/luci-indexcache /tmp/luci-modulecache
endef

# call BuildPackage - OpenWrt buildroot signature
