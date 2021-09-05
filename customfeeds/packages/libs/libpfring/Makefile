#
# Copyright (C) 2017 Banglang Huang
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=libpfring
PKG_VERSION:=7.8.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/ntop/PF_RING/tar.gz/$(PKG_VERSION)?
PKG_HASH:=3c7a563ee3ff58c76525c4f66ef711ecd66b080eadfcef99aa1d08df12f65ec0
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/PF_RING-$(PKG_VERSION)

PKG_MAINTAINER:=Banglang Huang <banglang.huang@foxmail.com>

PKG_FIXUP:=patch-libtool
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

CONFIGURE_PATH:=userland
MAKE_PATH:=userland/lib

define Package/libpfring
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Library for PR_RING (package process framework)
  URL:=https://github.com/ntop/pf_ring
  DEPENDS:=+kmod-pf-ring +libpcap +libpthread
  LICENSE:=LGPL-2.1-or-later
  LICENSE_FILES:=LICENSE
endef

define Package/libpfring/description
  PF_RING is a high speed packet capture library that turns a commodity PC into an efficient and cheap
  network measurement box suitable for both packet and active traffic analysis and manipulation.
  Moreover, PF_RING opens totally new markets as it enables the creation of efficient application such as
  traffic balancers or packet filters in a matter of lines of codes.
endef

define KernelPackage/pf-ring
  SUBMENU:=Network Support
  TITLE:=PF_RING Kernel driver
  FILES:=$(PKG_BUILD_DIR)/kernel/pf_ring.ko
  AUTOLOAD:=$(call AutoLoad,90,pf_ring,1)
  LICENSE:=GPL-2.0-or-later
endef

define KernelPackage/pf-ring/description
  Kernel module for libpf-ring package
endef

CONFIGURE_VARS += \
	MACHINE="$(ARCH)" \
	ac_cv_lib_nl_3_nl_socket_alloc=no

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		KERNEL_DIR="$(LINUX_DIR)" \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		M="$(PKG_BUILD_DIR)/kernel" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS) -I$(PKG_BUILD_DIR)/kernel" \
		modules
	$(call Build/Compile/Default)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpfring.so* $(1)/usr/lib/
endef

define Package/libpfring/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpfring.so* $(1)/usr/lib/
	$(LN) libpfring.so $(1)/usr/lib/libpfring.so.1
endef

$(eval $(call BuildPackage,libpfring))
$(eval $(call KernelPackage,pf-ring))
