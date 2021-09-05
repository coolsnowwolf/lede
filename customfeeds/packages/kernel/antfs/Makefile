include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=antfs
PKG_RELEASE:=2

PKG_SOURCE_URL:=https://github.com/klukonin/antfs.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2020-02-10
PKG_SOURCE_VERSION:=b41ba529f6b64b429527e09a06ce0326d5456c05
PKG_MIRROR_HASH:=dae039c0fe5bf1a2c8c1cca4211d607a4d6f56fc41b38444e2234b40d710d9db

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define KernelPackage/fs-antfs
	SUBMENU:=Filesystems
	TITLE:=AVM NTFS Read/Write Driver
	FILES:=$(PKG_BUILD_DIR)/antfs.ko
	AUTOLOAD:=$(call AutoLoad,30,antfs,1)
	DEPENDS:=+kmod-nls-base
endef

define KernelPackage/fs-antfs/description
	Kernel module for NTFS Filesytem
endef

MAKE_OPTS:= \
        ARCH="$(LINUX_KARCH)" \
        CROSS_COMPILE="$(TARGET_CROSS)" \
        M="$(PKG_BUILD_DIR)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		CONFIG_ANTFS_FS=m \
		CONFIG_ANTFS_SYMLINKS=y \
		ANTFS_VERSION=07.19-$(call version_abbrev,$(PKG_SOURCE_VERSION)) \
		modules
endef

$(eval $(call KernelPackage,fs-antfs))
