# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=dsa-rtl837x
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/airjinkela/rtl837x-dsa-driver.git
PKG_SOURCE_DATE:=2026-07-30
PKG_SOURCE_VERSION:=e306c16520196631b45965e94777c2fabe796dcf
PKG_MIRROR_HASH:=5b2094d3931db06649296a6b958c4d3ff4bd0e8be7206e4111768dd083b858e0

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=StarField Xu <air_jinkela@163.com>

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/dsa-rtl837x
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL837x switch DSA support
  KCONFIG:= \
	CONFIG_NET_DSA_TAG_RTL8_4=y
  FILES:=$(PKG_BUILD_DIR)/src/rtl8372n_dsa.ko
  DEPENDS:=@TARGET_mediatek_filogic
  AUTOLOAD:=$(call AutoLoad,42,rtl8372n_dsa,1)
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/src" \
		CONFIG_RTL8372N_DSA=m \
		CONFIG_RTL837X_PHY_PATCH=y \
		CONFIG_RTL8372N_DSA_DEBUG=n \
		modules
endef

$(eval $(call KernelPackage,dsa-rtl837x))
