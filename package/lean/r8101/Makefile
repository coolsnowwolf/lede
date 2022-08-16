#
# Download realtek r8101 linux driver from official site:
# [https://github.com/ghostrider-reborn/realtek-r8101-linux-driver]
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=r8101
PKG_VERSION:=1.035.03
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/umarizulkifli/openwrt-r8101.git
#PKG_SOURCE_VERSION:=ddfaceacd1b7ed2857fb995642a8ffb1fc37e989
#PKG_MIRROR_HASH:=5428f60dc33e9503c6cfdf690c00077149dce24cbb0591129d905b9f1aad9202

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/r8101
  TITLE:=Driver for Realtek r8101 chipsets
  SUBMENU:=Network Devices
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
  FILES:= $(PKG_BUILD_DIR)/src/r8101.ko
  AUTOLOAD:=$(call AutoProbe,r8101)
endef

define Package/r8101/description
  This package contains a driver for Realtek r8101 chipsets.
endef

define Build/Compile
	+$(KERNEL_MAKE) M=$(PKG_BUILD_DIR)/src modules
endef

$(eval $(call KernelPackage,r8101))
