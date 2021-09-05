#
# Copyright (C) 2016 Joseph C. Lehner <joseph.c.lehner@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mtd-rw
PKG_VERSION:=git-20160214
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MIRROR_HASH:=c44db17c3e05079116a1704f277642c9ce6f5ca4fa380c60f7e6d44509dc16be
PKG_SOURCE_URL:=https://github.com/jclehner/mtd-rw.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=7e8562067d6a366c8cbaa8084396c33b7e12986b

PKG_MAINTAINER:=Joseph C. Lehner <joseph.c.lehner@gmail.com>
PKG_LICENSE=GPL-2.0
PKG_LICENSE_FILES=LICENSE

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mtd-rw
	SUBMENU:=Other modules
	TITLE:=Write-enabler for MTD partitions
	FILES:=$(PKG_BUILD_DIR)/mtd-rw.ko
	DEPENDS:=@!(TARGET_x86||TARGET_bcm27xx||TARGET_octeontx)
endef

define KernelPackage/mtd-rw/description
	A kernel module that temporarily makes all MTD partitions writeable.
endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	M="$(PKG_BUILD_DIR)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
			$(MAKE_OPTS) \
			CONFIG_MTD_RW=m \
			modules
endef

$(eval $(call KernelPackage,mtd-rw))
