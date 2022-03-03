#
# Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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

PKG_NAME:=fast-classifier
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define KernelPackage/fast-classifier/Default
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  DEPENDS:=+kmod-ipt-conntrack +kmod-shortcut-fe
  TITLE:=Kernel driver for FAST Classifier
  FILES:=$(PKG_BUILD_DIR)/fast-classifier.ko
  KCONFIG:= \
	   CONFIG_NF_CONNTRACK_EVENTS=y \
	   CONFIG_NF_CONNTRACK_CHAIN_EVENTS=y \
	   CONFIG_NF_CONNTRACK_MARK=y \
	   CONFIG_XFRM=y
  CONFLICTS:=kmod-shortcut-fe-drv kmod-shortcut-fe-cm
endef

define KernelPackage/fast-classifier
  $(call KernelPackage/fast-classifier/Default)
endef

define KernelPackage/fast-classifier-noload
  $(call KernelPackage/fast-classifier/Default)
endef

define KernelPackage/fast-classifier/Default/description
FAST Classifier talks to SFE to make decisions about offloading connections
endef

define KernelPackage/fast-classifier/description
$(call KernelPackage/fast-classifier/Default/description)
endef

define KernelPackage/fast-classifier-noload/description
$(call KernelPackage/fast-classifier/Default/description)

This package does not load fast-classifier at boot by default
endef

define Package/fast-classifier-example
  TITLE:=Example user space program for fast-classifier
  DEPENDS:=+libnl +kmod-fast-classifier
endef

define Package/fast-classifier-example/description
Example user space program that communicates with fast
classifier kernel module
endef

HAVE_ECM:=$(CONFIG_PACKAGE_kmod-qca-nss-ecm-premium)$(CONFIG_PACKAGE_kmod-qca-nss-ecm-noload)$(CONFIG_PACKAGE_kmod-qca-nss-ecm-premium-noload)$(CONFIG_PACKAGE_kmod-qca-nss-ecm-standard)

define Build/Compile/kmod
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		$(PKG_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		CONFIG_FAST_CLASSIFIER=m \
		EXTRA_CFLAGS+="-DSFE_SUPPORT_IPV6" \
		$(if $(HAVE_ECM),EXTRA_CFLAGS+="-DCONFIG_SFE_ECM" CONFIG_SFE_ECM=y,) \
		modules
endef

define Build/Compile/example
	$(TARGET_CC) -o $(PKG_BUILD_DIR)/userspace_fast_classifier \
		-I $(PKG_BUILD_DIR) \
		-I$(STAGING_DIR)/usr/include/libnl \
		-I$(STAGING_DIR)/usr/include/libnl3 \
		-lnl-genl-3 -lnl-3 \
		$(PKG_BUILD_DIR)/nl_classifier_test.c
endef

define Build/Compile
	$(Build/Compile/kmod)
	$(if $(CONFIG_PACKAGE_fast-classifier-example),$(Build/Compile/example))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/fast-classifier.h $(1)/usr/include/
endef

define Package/fast-classifier-example/install
	$(INSTALL_DIR) $(1)/sbin
	$(CP) $(PKG_BUILD_DIR)/userspace_fast_classifier $(1)/sbin/
endef

$(eval $(call KernelPackage,fast-classifier))
#$(eval $(call KernelPackage,fast-classifier-noload))
#$(eval $(call BuildPackage,fast-classifier-example))
