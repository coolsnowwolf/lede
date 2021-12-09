#
# Copyright (c) 2013-2018, 2020 The Linux Foundation. All rights reserved.
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
# OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=shortcut-fe
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define KernelPackage/shortcut-fe
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  DEPENDS:=@IPV6 +kmod-nf-conntrack
  TITLE:=Kernel driver for SFE
  FILES:= \
	   $(PKG_BUILD_DIR)/shortcut-fe.ko \
	   $(PKG_BUILD_DIR)/shortcut-fe-ipv6.ko
  KCONFIG:= \
	   CONFIG_NF_CONNTRACK_EVENTS=y \
	   CONFIG_NF_CONNTRACK_TIMEOUT=y \
	   CONFIG_SHORTCUT_FE=y \
	   CONFIG_XFRM=y
  PROVIDES:=$(PKG_NAME)
  AUTOLOAD:=$(call AutoLoad,09,shortcut-fe shortcut-fe-ipv6)
endef

define KernelPackage/shortcut-fe/Description
Shortcut is an in-Linux-kernel IP packet forwarding engine.
endef

define KernelPackage/shortcut-fe/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/shortcut-fe $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/usr/bin/sfe_dump $(1)/usr/bin
endef

HAVE_ECM:=$(CONFIG_PACKAGE_kmod-qca-nss-ecm-premium)$(CONFIG_PACKAGE_kmod-qca-nss-ecm-noload)$(CONFIG_PACKAGE_kmod-qca-nss-ecm-premium-noload)$(CONFIG_PACKAGE_kmod-qca-nss-ecm-standard)

define KernelPackage/shortcut-fe-cm
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  DEPENDS:=+kmod-ipt-conntrack +kmod-shortcut-fe
  TITLE:=Kernel driver for SFE
  FILES:=$(PKG_BUILD_DIR)/shortcut-fe-cm.ko
  KCONFIG:= \
	   CONFIG_NF_CONNTRACK_CHAIN_EVENTS=y \
	   CONFIG_NF_CONNTRACK_EVENTS=y \
	   CONFIG_XFRM=y
  CONFLICTS:=kmod-shortcut-fe-drv
endef

define KernelPackage/shortcut-fe-cm/Description
Simple connection manager for the Shortcut forwarding engine.
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		$(PKG_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS+="-DSFE_SUPPORT_IPV6" SFE_SUPPORT_IPV6=y \
		$(if $(HAVE_ECM),EXTRA_CFLAGS+="-DCONFIG_SFE_ECM" CONFIG_SFE_ECM=y,) \
		modules
endef

ifneq ($(CONFIG_PACKAGE_kmod-shortcut-fe)$(CONFIG_PACKAGE_kmod-shortcut-fe-cm),)
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/shortcut-fe
	$(CP) -rf $(PKG_BUILD_DIR)/sfe.h $(1)/usr/include/shortcut-fe
endef
endif

$(eval $(call KernelPackage,shortcut-fe))
$(eval $(call KernelPackage,shortcut-fe-cm))
