#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtw88-usb
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2021-07-06
PKG_SOURCE_URL:=https://github.com/ulli-kroll/rtw88-usb.git
PKG_SOURCE_VERSION:=dd05eec8e5ac934b3d8e67e8008228f842b4ce1c
PKG_MIRROR_HASH:=9932f69573a3469d1eba403c7964a9472530fcfa1c29356babeecdad8d9c6991

PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtw88-default
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek rtw88 family usb driver
  DEPENDS:=+kmod-mac80211 +kmod-usb-core \
	+@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT
endef

define KernelPackage/rtw88-usb
  $(KernelPackage/rtw88-default)
  HIDDEN:=1
  FILES:= \
	$(PKG_BUILD_DIR)/rtw88_usb.ko \
	$(PKG_BUILD_DIR)/rtw88_core.ko
endef

define KernelPackage/rtl8723du
  $(KernelPackage/rtw88-default)
  TITLE:=Realtek RTL8723DU support
  DEPENDS+=+kmod-rtw88-usb +rtl8723du-firmware
  FILES:= \
	$(PKG_BUILD_DIR)/rtw88_8723d.ko \
	$(PKG_BUILD_DIR)/rtw88_8723du.ko
  AUTOLOAD:=$(call AutoProbe,rtw88_8723du)
endef

define KernelPackage/rtl8821cu
  $(KernelPackage/rtw88-default)
  TITLE:=Realtek RTL8821CU support
  DEPENDS+=+kmod-rtw88-usb +rtl8821ce-firmware
  FILES:= \
	$(PKG_BUILD_DIR)/rtw88_8821c.ko \
	$(PKG_BUILD_DIR)/rtw88_8821cu.ko
  AUTOLOAD:=$(call AutoProbe,rtw88_8821cu)
endef

define KernelPackage/rtl8822bu
  $(KernelPackage/rtw88-default)
  TITLE:=Realtek RTL8822BU support
  DEPENDS+=+kmod-rtw88-usb +rtl8822be-firmware
  FILES:= \
	$(PKG_BUILD_DIR)/rtw88_8822b.ko \
	$(PKG_BUILD_DIR)/rtw88_8822bu.ko
  AUTOLOAD:=$(call AutoProbe,rtw88_8822bu)
endef

define KernelPackage/rtl8822cu
  $(KernelPackage/rtw88-default)
  TITLE:=Realtek RTL8822CU support
  DEPENDS+=+kmod-rtw88-usb +rtl8822ce-firmware
  FILES:= \
	$(PKG_BUILD_DIR)/rtw88_8822c.ko \
	$(PKG_BUILD_DIR)/rtw88_8822cu.ko
  AUTOLOAD:=$(call AutoProbe,rtw88_8822cu)
endef

NOSTDINC_FLAGS := \
	$(KERNEL_NOSTDINC_FLAGS) \
	-I$(PKG_BUILD_DIR) \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-include backport/autoconf.h \
	-include backport/backport.h

ifdef CONFIG_PACKAGE_kmod-rtw88-usb
PKG_MAKE_FLAGS += \
    CONFIG_RTW88_USB=m \
    CONFIG_RTW88_CORE=m
endif
ifdef CONFIG_PACKAGE_kmod-rtl8723du
  PKG_MAKE_FLAGS += \
    CONFIG_RTW88_8723D=m \
    CONFIG_RTW88_8723DU=m
endif
ifdef CONFIG_PACKAGE_kmod-rtl8821cu
  PKG_MAKE_FLAGS += \
    CONFIG_RTW88_8821C=m \
    CONFIG_RTW88_8821CU=m
endif
ifdef CONFIG_PACKAGE_kmod-rtl8822bu
  PKG_MAKE_FLAGS += \
    CONFIG_RTW88_8822B=m \
    CONFIG_RTW88_8822BU=m
endif
ifdef CONFIG_PACKAGE_kmod-rtl8822cu
  PKG_MAKE_FLAGS += \
    CONFIG_RTW88_8822C=m \
    CONFIG_RTW88_8822CU=m
endif

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		$(PKG_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		modules
endef

$(eval $(call KernelPackage,rtw88-usb))
$(eval $(call KernelPackage,rtl8723du))
$(eval $(call KernelPackage,rtl8821cu))
$(eval $(call KernelPackage,rtl8822bu))
$(eval $(call KernelPackage,rtl8822cu))
