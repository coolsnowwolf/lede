
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=oaf
PKG_VERSION:=5.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

PKG_AUTOLOAD:=oaf
RSTRIP:=:

define KernelPackage/oaf
  SECTION:=Derry Apps
  CATEGORY:=Derry Apps
  TITLE:=open app filter kernel module
  FILES:=$(PKG_BUILD_DIR)/oaf.ko 
  DEPENDS:=+kmod-ipt-conntrack
  KCONFIG:=
  AUTOLOAD:=$(call AutoLoad,0,$(PKG_AUTOLOAD))
endef

define KernelPackage/oaf/description
  open appfilter kernel module 
endef


MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,oaf))

