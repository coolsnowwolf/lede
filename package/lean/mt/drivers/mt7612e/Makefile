# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt76x2e
P4REV:=8
PKG_VERSION:=3.0.5.0
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/mt76x2

PKG_KCONFIG:= \
	RT_FIRST_CARD  RT_SECOND_IF_MT7612E RT_SECOND_CARD  AP_LLTD AP_LED AP_WDS AP_MBSS AP_MBSS_NEW_MBSSID_MODE  \
	AP_APCLI AP_80211AC_VHT AP_TXBF_SUPPORT  \

PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))

include $(INCLUDE_DIR)/package.mk

TAR_CMD=$(HOST_TAR) -C $(1)/ $(TAR_OPTIONS)

define KernelPackage/mt76x2e
  CATEGORY:=Kernel modules
  TITLE:=MTK MT76x2e wifi AP driver
  DEPENDS:=@TARGET_ramips
  FILES:=$(PKG_BUILD_DIR)/mt76x2_ap/mt76x2_ap.ko
  AUTOLOAD:=$(call AutoLoad,90,mt76x2_ap)
  SUBMENU:=Wireless Drivers
  MENU:=1
endef

define KernelPackage/mt76x2e/config
	source "$(SOURCE)/config.in"
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" V=1 \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		M="$(PKG_BUILD_DIR)/mt76x2_ap" \
		$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_MT76X2E_$c),CONFIG_$(c)=$(CONFIG_MT76X2E_$(c)))) \
		modules
endef

define KernelPackage/mt76x2e/install
	$(INSTALL_DIR) $(1)/etc/wireless/mt7612/
endef

$(eval $(call KernelPackage,mt76x2e))
