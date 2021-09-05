#
# Copyright (C) 2016-2017 Dan Luedtke <mail@danrl.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=jool
PKG_VERSION:=4.1.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/NICMx/Jool/releases/download/v$(PKG_VERSION)
PKG_HASH:=ec130a3659acff5450e74924d5f9da52540539fc213b5842fb3a8fd7d919ccf9

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=!USE_GLIBC:argp-standalone

include $(INCLUDE_DIR)/package.mk

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		KERNEL_DIR="$(LINUX_DIR)" \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		M="$(PKG_BUILD_DIR)/src/mod/common" \
		V="$(V)" \
		modules
	$(MAKE) -C "$(LINUX_DIR)" \
		KERNEL_DIR="$(LINUX_DIR)" \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		M="$(PKG_BUILD_DIR)/src/mod/nat64" \
		V="$(V)" \
		modules
	$(MAKE) -C "$(LINUX_DIR)" \
		KERNEL_DIR="$(LINUX_DIR)" \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		M="$(PKG_BUILD_DIR)/src/mod/siit" \
		V="$(V)" \
		modules
	$(call Build/Compile/Default)
endef

define Package/jool/Default
  SECTION:=net
  CATEGORY:=Network
  URL:=https://www.jool.mx
endef

define Package/jool/Default/description
  Jool is an Open Source SIIT and NAT64 for Linux.
endef

define Package/jool
  $(call Package/jool/Default)
  TITLE:=Jool meta-package
  DEPENDS:=+kmod-jool +jool-tools
  BUILDONLY:=1
endef

define Package/jool/description
  $(call Package/jool/Default/description)
endef

define KernelPackage/jool
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  TITLE:=Jool kernel module
  DEPENDS:= \
    @IPV6 \
    +kmod-crypto-md5 \
    +kmod-nf-conntrack \
    +kmod-nf-conntrack6 \
    +kmod-nf-ipt
  FILES:= \
    $(PKG_BUILD_DIR)/src/mod/common/jool_common.$(LINUX_KMOD_SUFFIX) \
    $(PKG_BUILD_DIR)/src/mod/nat64/jool.$(LINUX_KMOD_SUFFIX) \
    $(PKG_BUILD_DIR)/src/mod/siit/jool_siit.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/jool/description
  $(call Package/jool/Default/description)

  This package provides the kernel module for Jool.
endef

define Package/jool-tools
  $(call Package/jool/Default)
  TITLE:=Jool userspace control programs
  DEPENDS:=+libnl +libxtables
endef

define Package/jool-tools/description
  $(call Package/jool/Default/description)

  This package provides the userspace control programs for Jool.
endef

CONFIGURE_ARGS += \
	--disable-shared \
	--without-bash-completion-dir

CONFIGURE_VARS += \
	ac_cv_func_memset=yes \
	ac_cv_func_strcasecmp=yes

define Package/jool-tools/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/jool		$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/joold		$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/jool_siit	$(1)/usr/bin/

	$(INSTALL_DIR) $(1)/usr/lib/iptables
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/iptables/libxt_JOOL_SIIT.so \
		$(1)/usr/lib/iptables
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/iptables/libxt_JOOL.so \
		$(1)/usr/lib/iptables
endef

$(eval $(call BuildPackage,jool))
$(eval $(call KernelPackage,jool))
$(eval $(call BuildPackage,jool-tools))
