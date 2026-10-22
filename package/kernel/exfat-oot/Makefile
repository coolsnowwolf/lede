#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=linux-exfat-oot
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/namjaejeon/linux-exfat-oot.git
PKG_SOURCE_DATE:=2023-10-03
PKG_SOURCE_VERSION:=3ad149047d1acf158f327b3f0279d3d8b03bc894
PKG_MIRROR_HASH:=7c31b707c23cfaeb495f4aeee728e346248a773a937ecc29abb7f6a30b9d94b6

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_SOURCE_DATE)-$(call version_abbrev,$(PKG_SOURCE_VERSION))
PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define KernelPackage/fs-exfat-oot
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Filesystems
  TITLE:=exFAT kernel module
  URL:=https://github.com/namjaejeon/linux-exfat-oot
  FILES:=$(PKG_BUILD_DIR)/exfat.ko
  AUTOLOAD:=$(call AutoProbe,exfat)
  DEPENDS:=@LINUX_5_4 +kmod-nls-base
endef

define KernelPackage/exfat-oot/description
  This package provides the kernel module for exFAT.
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(PKG_EXTRA_KCONFIG) \
	CONFIG_EXFAT_FS=m \
	modules
endef

$(eval $(call KernelPackage,fs-exfat-oot))
