include $(TOPDIR)/rules.mk

PKG_NAME:=r8126
PKG_VERSION:=10.016.00
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/openwrt/rtl8126/releases/download/$(PKG_VERSION)
PKG_HASH:=50c8d3d49592d2e8f372bd7ece8e7df9b50a71b055c077d42eacc42302914440

PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPLv2
PKG_MAINTAINER:=Alvaro Fernandez Rojas <noltari@gmail.com>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/r8126
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL8126 PCI 5 Gigabit Ethernet driver
  DEPENDS:=@PCI_SUPPORT +kmod-libphy
  FILES:=$(PKG_BUILD_DIR)/src/r8126.ko
  AUTOLOAD:=$(call AutoProbe,r8126,1)
  PROVIDES:=kmod-r8169
  VARIANT:=regular
endef

define KernelPackage/r8126-rss
$(call KernelPackage/r8126)
  CONFLICTS:=kmod-r8126
  TITLE+= (RSS)
  VARIANT:=rss
endef

ifeq ($(BUILD_VARIANT),rss)
  PKG_MAKE_FLAGS += \
    ENABLE_RSS_SUPPORT=y \
    ENABLE_MULTIPLE_TX_QUEUE=y
endif

PKG_MAKE_FLAGS += CONFIG_ASPM=n

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		$(PKG_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)/src" \
		modules
endef

$(eval $(call KernelPackage,r8126))
$(eval $(call KernelPackage,r8126-rss))
