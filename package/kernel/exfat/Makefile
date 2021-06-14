#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=exfat
PKG_VERSION:=5.11.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/namjaejeon/linux-exfat-oot/tar.gz/$(PKG_VERSION)?
PKG_HASH:=85324f1edb1c03f65700164ba4f5750fe5cc7a8b956add321626be69149790f5
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/linux-exfat-oot-$(PKG_VERSION)

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define KernelPackage/fs-exfat0
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Filesystems
  TITLE:=exFAT kernel module
  URL:=https://github.com/namjaejeon/linux-exfat-oot
  FILES:=$(PKG_BUILD_DIR)/exfat.ko
  AUTOLOAD:=$(call AutoProbe,exfat)
  DEPENDS:=+kmod-nls-base
  CONFLICTS:=kmod-fs-exfat
endef

define KernelPackage/exfat0/description
  This package provides the kernel module for exfat.
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(PKG_EXTRA_KCONFIG) \
	CONFIG_EXFAT_FS=m \
	modules
endef

$(eval $(call KernelPackage,fs-exfat0))
