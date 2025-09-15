include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=nat46

PKG_SOURCE_DATE:=2025-04-23
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ayourtch/nat46.git
PKG_SOURCE_VERSION:=04923c51039e8ca270c6f1dde3f04f3b36958089
PKG_MIRROR_HASH:=27c0b42c7705935f70fda4f6b400e4758bf4727f0462bfecf7758384ecc3f0d8

PKG_MAINTAINER:=Hans Dedecker <dedeckeh@gmail.com>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/nat46
  DEPENDS:=@IPV6 +kmod-nf-conntrack6
  TITLE:=Stateless NAT46 translation kernel module
  SECTION:=kernel
  SUBMENU:=Network Support
  FILES:=$(PKG_BUILD_DIR)/nat46/modules/nat46.ko
  AUTOLOAD:=$(call AutoLoad,33,nat46)
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)/nat46/modules" \
		MODFLAGS="-DMODULE -mlong-calls" \
		EXTRA_CFLAGS="-DNAT46_VERSION=\\\"$(PKG_SOURCE_VERSION)\\\"" \
		modules
endef

$(eval $(call KernelPackage,nat46))
