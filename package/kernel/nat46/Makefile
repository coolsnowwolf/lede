include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=nat46

PKG_MIRROR_HASH:=a74a9a84605bac6186adf37025b3880eb9cf5931791bb61d47e69b8898cac411
PKG_SOURCE_URL:=https://github.com/ayourtch/nat46.git
PKG_SOURCE_DATE:=2021-06-24
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=9b3a8197421d987d77e1b9df4159cfe0b9820ee9

PKG_MAINTAINER:=Hans Dedecker <dedeckeh@gmail.com>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/nat46
  DEPENDS:=@IPV6
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
