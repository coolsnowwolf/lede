include $(TOPDIR)/rules.mk

PKG_NAME:=r8127
PKG_VERSION:=11.014.00
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/openwrt/rtl8127/releases/download/$(PKG_VERSION)
PKG_HASH:=f496bc16c32d2e8f9482c57d006604c70d9e8d55b4f1f999b88c602de9104094

PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPLv2
PKG_MAINTAINER:=Alvaro Fernandez Rojas <noltari@gmail.com>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/r8127
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL8127 PCI 10 Gigabit Ethernet driver
  DEPENDS:=@PCI_SUPPORT +kmod-libphy
  FILES:=$(PKG_BUILD_DIR)/src/r8127.ko
  AUTOLOAD:=$(call AutoProbe,r8127)
  PROVIDES:=kmod-r8169
  VARIANT:=regular
endef

define KernelPackage/r8127-rss
$(call KernelPackage/r8127)
  CONFLICTS:=kmod-r8127
  TITLE+= (RSS)
  VARIANT:=rss
endef

ifeq ($(BUILD_VARIANT),rss)
  PKG_MAKE_FLAGS += ENABLE_RSS_SUPPORT=y
endif

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		$(PKG_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)/src" \
		modules
endef

$(eval $(call KernelPackage,r8127))
$(eval $(call KernelPackage,r8127-rss))
