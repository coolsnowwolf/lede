# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=airoha-pon
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0-only
PKG_MAINTAINER:=coolsnowwolf <coolsnowwolf@gmail.com>

include $(INCLUDE_DIR)/package.mk

define KernelPackage/airoha-pon/Default
  SUBMENU:=Network Devices
  DEPENDS:=+kmod-i2c-core
  KCONFIG:=
endef

define KernelPackage/airoha-gpon-en757x
  $(call KernelPackage/airoha-pon/Default)
  DEPENDS+=@TARGET_airoha_en7523
  TITLE:=Airoha EN757x GPON driver (v1)
  VARIANT:=v1
  FILES:= \
	$(PKG_BUILD_DIR)/gpon-en757x/xpon_phy/phy.ko \
	$(PKG_BUILD_DIR)/gpon-en757x/xpon/xpon.ko
  AUTOLOAD:=$(call AutoProbe,phy xpon)
endef

define KernelPackage/airoha-gpon-en757x/description
 Kernel modules for the v1 Airoha EN7570 and EN7571 GPON MAC and PHY.
endef

define KernelPackage/airoha-xpon-en757x
  $(call KernelPackage/airoha-pon/Default)
  DEPENDS+=@(TARGET_airoha_an7581||TARGET_airoha_an7583)
  TITLE:=Airoha EN757x xPON driver (v2)
  VARIANT:=v2
  FILES:= \
	$(PON_BSP_DIR)/airoha_ecnt_hook.ko \
	$(PON_BSP_DIR)/airoha_ecnt_scu.ko \
	$(PON_BSP_DIR)/airoha_ecnt_pon_phy.ko \
	$(PON_BSP_DIR)/airoha_ecnt_olt_phy.ko \
	$(PON_BSP_DIR)/airoha_arht_board.ko \
	$(PON_BSP_DIR)/airoha_arht_serdes.ko \
	$(PON_BSP_DIR)/airoha_an7583_combo_phy.ko \
	$(PKG_BUILD_DIR)/xpon-en757x/xpon_phy_10g/phy_10g.ko \
	$(PKG_BUILD_DIR)/xpon-en757x/xpon_10g/xpon_10g.ko
  AUTOLOAD:=$(call AutoProbe,airoha_ecnt_hook airoha_ecnt_scu airoha_ecnt_pon_phy airoha_ecnt_olt_phy airoha_arht_board airoha_arht_serdes airoha_an7583_combo_phy phy_10g xpon_10g)
endef

define KernelPackage/airoha-xpon-en757x/description
 Kernel modules for the v2 Airoha EN7581 and AN7583 GPON and EPON MAC and PHY.
endef

PON_BSP_DIR:=$(PKG_BUILD_DIR)/bsp
PON_INCLUDE_FLAGS:= \
	-I$(PON_BSP_DIR)/include \
	-I$(PON_BSP_DIR)/include/global_inc \
	-I$(PON_BSP_DIR)/include/global_inc/version \
	-I$(PON_BSP_DIR)/an7583/pon_pma/inc \
	-I$(PON_BSP_DIR)/qdma
PON_V2_CPU_CFLAGS:= \
	-D__IMEM= \
	-DTCSUPPORT_CPU_ARMV8=1 \
	-DTCSUPPORT_CPU_ARMV8_64=1 \
	-DTCSUPPORT_CPU_EN7581=1 \
	-DTCSUPPORT_BOARD_SELECT=1 \
	-DCONFIG_USE_MT7520_ASIC=1 \
	$(if $(CONFIG_TARGET_airoha_an7583),-DTCSUPPORT_CPU_AN7583=1)
PON_V2_CPU_MAKE:=TCSUPPORT_CPU_EN7581=1 $(if $(CONFIG_TARGET_airoha_an7583),TCSUPPORT_CPU_AN7583=1)

ifeq ($(BUILD_VARIANT),v1)
define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) M="$(PKG_BUILD_DIR)/gpon-en757x/xpon_phy" \
		KCFLAGS="$(PON_INCLUDE_FLAGS) -DTCSUPPORT_CPU_EN7521=1" \
		MODULES_PHY_DIR="$(PKG_BUILD_DIR)/gpon-en757x/xpon_phy" \
		MODULES_QDMA_7512_DIR="$(PON_BSP_DIR)/qdma" \
		TCSUPPORT_CPU_EN7521=1 \
		modules
	+$(KERNEL_MAKE) $(PKG_JOBS) M="$(PKG_BUILD_DIR)/gpon-en757x/xpon" \
		KCFLAGS="$(PON_INCLUDE_FLAGS) -DTCSUPPORT_CPU_EN7521=1" \
		MODULES_XPON_WAN_DIR="$(PKG_BUILD_DIR)/gpon-en757x/xpon" \
		MODULES_QDMA_7512_DIR="$(PON_BSP_DIR)/qdma" \
		MODULES_FE_7512_DIR="$(PKG_BUILD_DIR)/gpon-en757x/raeth" \
		GLOBAL_INC_DIR="$(PON_BSP_DIR)/include/global_inc" \
		TCSUPPORT_WAN_GPON=1 \
		TCSUPPORT_CPU_EN7521=1 \
		modules
endef
endif

ifeq ($(BUILD_VARIANT),v2)
define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) M="$(PON_BSP_DIR)" \
		AIROHA_PON_CFLAGS="$(PON_INCLUDE_FLAGS) $(PON_V2_CPU_CFLAGS)" \
		$(PON_V2_CPU_MAKE) \
		modules
	+$(KERNEL_MAKE) $(PKG_JOBS) M="$(PKG_BUILD_DIR)/xpon-en757x/xpon_phy_10g" \
		KCFLAGS="$(PON_INCLUDE_FLAGS) $(PON_V2_CPU_CFLAGS)" \
		KBUILD_EXTRA_SYMBOLS="$(PON_BSP_DIR)/Module.symvers" \
		MODULES_PHY_10G_DIR="$(PKG_BUILD_DIR)/xpon-en757x/xpon_phy_10g" \
		MODULES_QDMA_DIR="$(PON_BSP_DIR)/qdma" \
		$(PON_V2_CPU_MAKE) \
		modules
	+$(KERNEL_MAKE) $(PKG_JOBS) M="$(PKG_BUILD_DIR)/xpon-en757x/xpon_10g" \
		KCFLAGS="$(PON_INCLUDE_FLAGS) $(PON_V2_CPU_CFLAGS) -DTCSUPPORT_WAN_GPON=1 -DTCSUPPORT_WAN_EPON=1" \
		KBUILD_MODPOST_WARN=1 \
		KBUILD_EXTRA_SYMBOLS="$(PON_BSP_DIR)/Module.symvers $(PKG_BUILD_DIR)/xpon-en757x/xpon_phy_10g/Module.symvers" \
		XPON_BSP_MODULES_XPON_10G_DRIVER="$(PKG_BUILD_DIR)/xpon-en757x/xpon_10g" \
		XPON_BSP_GLOBAL_INC_PRIVATE="$(PON_BSP_DIR)/include/global_inc" \
		XPON_BSP_GLOBAL_INC_PUBLIC="$(PON_BSP_DIR)/include/global_inc" \
		XPON_BSP_ECNT_HOOK_DIR="$(PON_BSP_DIR)" \
		TCSUPPORT_WAN_GPON=1 \
		TCSUPPORT_WAN_EPON=1 \
		$(PON_V2_CPU_MAKE) \
		modules
endef
endif

$(eval $(call KernelPackage,airoha-gpon-en757x))
$(eval $(call KernelPackage,airoha-xpon-en757x))
