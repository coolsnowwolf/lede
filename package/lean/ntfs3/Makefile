#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ntfs3
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/LGA1150/ntfs3-oot.git
PKG_SOURCE_DATE:=2021-04-30
PKG_SOURCE_VERSION:=4e50c89890d510fe22096bd6be3b640abf295e59
PKG_MIRROR_HASH:=311ee651f55ec46daa5c3e0029108e9a5c511553c4baf0daecd25efb4235b991

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define KernelPackage/fs-ntfs3
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Filesystems
  TITLE:=Fully functional NTFS Read-Write driver
  KCONFIG:=CONFIG_NLS_DEFAULT="utf8"
  FILES:=$(PKG_BUILD_DIR)/ntfs3.ko
  AUTOLOAD:=$(call AutoProbe,ntfs3)
  DEPENDS:=+kmod-nls-utf8
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
