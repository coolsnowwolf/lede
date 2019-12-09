
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=oaf
PKG_RELEASE:=3

include $(INCLUDE_DIR)/package.mk

PKG_AUTOLOAD:=oaf

define KernelPackage/oaf
  SECTION:=Derry Apps
  CATEGORY:=Derry Apps
  TITLE:=open app filter kernel module
  FILES:=$(PKG_BUILD_DIR)/oaf.ko 
  KCONFIG:=
  AUTOLOAD:=$(call AutoLoad,0,$(PKG_AUTOLOAD))
endef

define KernelPackage/oaf/description
  open appfilter kernel module 
endef


MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,oaf))

