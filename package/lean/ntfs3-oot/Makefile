#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ntfs3-oot
PKG_RELEASE:=3

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/LGA1150/ntfs3-oot.git
PKG_SOURCE_DATE:=2021-07-05
PKG_SOURCE_VERSION:=46d199e7d7302879b23ad20097ba53b152257288
PKG_MIRROR_HASH:=f07253ec864887c121177fc5b358c21249af0483e4dab0d5157410db618c0990

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define KernelPackage/fs-ntfs3-oot
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Filesystems
  TITLE:=Fully functional NTFS Read-Write driver
  FILES:=$(PKG_BUILD_DIR)/ntfs3.ko
  AUTOLOAD:=$(call AutoProbe,ntfs3)
  DEPENDS:=@LINUX_5_4 +kmod-nls-utf8
endef

define KernelPackage/fs-ntfs3-oot/description
  This package provides the kernel module for ntfs3.
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(PKG_EXTRA_KCONFIG) \
	CONFIG_NTFS3_FS=m \
	modules
endef

$(eval $(call KernelPackage,fs-ntfs3-oot))
