include $(TOPDIR)/rules.mk

PKG_NAME:=r8125
PKG_VERSION:=9.014.01
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/openwrt/rtl8125/releases/download/$(PKG_VERSION)
PKG_HASH:=f006aa95501738ca55c522812c9d1b473ac781675f3ad88ce341a09316b8aa13

PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPLv2
PKG_MAINTAINER:=Alvaro Fernandez Rojas <noltari@gmail.com>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/r8125
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL8125 PCI 2.5 Gigabit Ethernet driver
  DEPENDS:=@PCI_SUPPORT +kmod-libphy
  FILES:=$(PKG_BUILD_DIR)/src/r8125.ko
  AUTOLOAD:=$(call AutoProbe,r8125)
  PROVIDES:=kmod-r8169
  VARIANT:=regular
endef

define KernelPackage/r8125-rss
$(call KernelPackage/r8125)
  CONFLICTS:=kmod-r8125
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

$(eval $(call KernelPackage,r8125))
$(eval $(call KernelPackage,r8125-rss))
