include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=antfs
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/Sembedded/antfs.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2018-08-14
PKG_SOURCE_VERSION:=002407a1b4f1bc9ec19fb07a4c01fc5348d216a6
PKG_MIRROR_HASH:=d1c44b2be92531c2caa4fa00b620591f95104436920640e94861f8207d6e9082

PKG_LICENSE:=GPL-2.0-only
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
		modules
endef

$(eval $(call KernelPackage,fs-antfs))
