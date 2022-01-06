include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-diskman
PKG_VERSION:=v0.2.11
PKG_RELEASE:=2

PKG_MAINTAINER:=lisaac <lisaac.cn@gmail.com>
PKG_LICENSE:=AGPL-3.0

LUCI_TITLE:=Disk Manager interface for LuCI
LUCI_DEPENDS:=+blkid +e2fsprogs +parted +smartmontools \
	+PACKAGE_$(PKG_NAME)_INCLUDE_btrfs_progs:btrfs-progs \
	+PACKAGE_$(PKG_NAME)_INCLUDE_lsblk:lsblk \
	+PACKAGE_$(PKG_NAME)_INCLUDE_mdadm:mdadm \
	+PACKAGE_$(PKG_NAME)_INCLUDE_kmod_md_raid456:mdadm \
	+PACKAGE_$(PKG_NAME)_INCLUDE_kmod_md_raid456:kmod-md-raid456 \
	+PACKAGE_$(PKG_NAME)_INCLUDE_kmod_md_linears:mdadm \
	+PACKAGE_$(PKG_NAME)_INCLUDE_kmod_md_linears:kmod-md-linear

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_btrfs_progs
	bool "Include btrfs-progs"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_lsblk
	bool "Include lsblk"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_mdadm
	bool "Include mdadm"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_kmod_md_raid456
	depends on PACKAGE_$(PKG_NAME)_INCLUDE_mdadm
	bool "Include kmod-md-raid456"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_kmod_md_linear
	depends on PACKAGE_$(PKG_NAME)_INCLUDE_mdadm
	bool "Include kmod-md-linear"
	default n
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
rm -fr /tmp/luci-indexcache /tmp/luci-modulecache
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
