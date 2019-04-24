# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=wifi_forward
PKG_REVISION:=01fc537c
PKG_SOURCE:=$(PKG_NAME)-$(PKG_REVISION).tar.bz2
P4REV:=
PKG_VERSION:=
PKG_SOURCE_URL:=
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:= \
	MT_WIFI_PKT_FWD \
	MT_WIFI_PKT_FWD_V1

PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))

include $(INCLUDE_DIR)/package.mk

TAR_CMD=$(HOST_TAR) -C $(1)/.. $(TAR_OPTIONS)

define KernelPackage/wifi_forward
  CATEGORY:=MTK Properties
  TITLE:=MTK wifi forward driver
ifeq ($(CONFIG_MT_WIFI_PKT_FWD_V1),y)
  FILES:=$(PKG_BUILD_DIR)/wifi_fwd_v1/wifi_forward.ko
else
  FILES:=$(PKG_BUILD_DIR)/wifi_fwd/wifi_forward.ko
endif
  AUTOLOAD:=$(call AutoProbe,wifi_forward)
  SUBMENU:=Drivers
  MENU:=1
endef

define KernelPackage/wifi_forward/config
	source "$(SOURCE)/config.in"
endef
	

ifeq ($(CONFIG_MT_WIFI_PKT_FWD_V1),y)
define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" V=1 \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)/wifi_fwd_v1" \
		$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=$(CONFIG_$(c)))) \
		modules
endef
else
define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" V=1 \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)/wifi_fwd" \
		$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=$(CONFIG_$(c)))) \
		modules
endef
endif

TARGET_CFLAGS += \
	$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),-DCONFIG_$(c)=$(CONFIG_$c)))

MAKE_FLAGS += \
	$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=$(CONFIG_$c))) 

TARGET_CFLAGS += -DCONFIG_SUPPORT_OPENWRT
MAKE_FLAGS += CONFIG_SUPPORT_OPENWRT=y

define KernelPackage/wifi_forward/install
	# $(INSTALL_DIR) $(1)/lib/modules/ralink/
	# if [ "$$(CONFIG_MT_WIFI_PKT_FWD_V1)" = "y" ]; then \
		# $(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi_fwd_v1/wifi_forward.ko $(1)/lib/modules/ralink/; \
	# else \
		# $(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi_fwd/wifi_forward.ko $(1)/lib/modules/ralink/; \
	# fi
endef

$(eval $(call KernelPackage,wifi_forward))


