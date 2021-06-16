#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ntfs3
PKG_VERSION:=5.1.1
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/LGA1150/ntfs3-oot.git
PKG_SOURCE_DATE:=2021-04-30
PKG_SOURCE_VERSION:=4e50c89890d510fe22096bd6be3b640abf295e59
PKG_MIRROR_HASH:=979d9befeeb4a38ac1481ddca05b81ecda08ae127a63889f22633022bca5f00c

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define KernelPackage/fs-ntfs3
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Filesystems
  TITLE:=Kernel module for NTFS Read-Write file system support
  FILES:=$(PKG_BUILD_DIR)/ntfs3.ko
  AUTOLOAD:=$(call AutoProbe,ntfs3)
  DEPENDS:=+kmod-nls-base
endef

define KernelPackage/ntfs3/description
  This package provides the kernel module for ntfs3.
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(PKG_EXTRA_KCONFIG) \
	CONFIG_NTFS3_FS=m \
	modules
endef

$(eval $(call KernelPackage,fs-ntfs3))