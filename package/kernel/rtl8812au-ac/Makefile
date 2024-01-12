include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8812au-ac
PKG_RELEASE:=2

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

PKG_SOURCE_URL:=https://github.com/aircrack-ng/rtl8812au.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2023-01-23
PKG_SOURCE_VERSION:=e72dacdc4af7cf421484ce9567ef2cc995a16f3a
PKG_MIRROR_HASH:=4e2168712f62a6d736fbc3422f651c6738a37a40d226617f7d8248ad8abd2433

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE
# PKG_MAINTAINER:=

PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8812au-ac
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek RTL8812AU/21AU wireless drivers
  DEPENDS:=+kmod-cfg80211 +kmod-usb-core +@DRIVER_11AC_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/rtl8812au.ko
  AUTOLOAD:=$(call AutoProbe,rtl8812au)
  CONFLICTS:=kmod-rtl8812au-ct
endef

NOSTDINC_FLAGS:= \
	$(KERNEL_NOSTDINC_FLAGS) \
	-I$(PKG_BUILD_DIR) \
	-I$(PKG_BUILD_DIR)/include \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-include backport/autoconf.h \
	-include backport/backport.h

NOSTDINC_FLAGS+= \
	-DRTW_SINGLE_WIPHY \
	-DRTW_USE_CFG80211_STA_EVENT \
	-DCONFIG_IOCTL_CFG80211 \
	-DBUILD_OPENWRT

ifeq ($(CONFIG_BIG_ENDIAN),y)
NOSTDINC_FLAGS+= -DCONFIG_BIG_ENDIAN
else
NOSTDINC_FLAGS+= -DCONFIG_LITTLE_ENDIAN
endif

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		CONFIG_88XXAU=m \
		USER_MODULE_NAME=rtl8812au \
		modules
endef

$(eval $(call KernelPackage,rtl8812au-ac))
