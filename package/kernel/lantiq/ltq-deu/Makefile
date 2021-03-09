# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ltq-deu
PKG_RELEASE:=1

PKG_MAINTAINER:=John Crispin <john@phrozen.org>
PKG_LICENSE:=GPL-2.0+

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ltq-deu-template
  SECTION:=sys
  CATEGORY:=Kernel modules
  SUBMENU:=Cryptographic API modules
  TITLE:=deu driver for $(1)
  URL:=http://www.lantiq.com/
  VARIANT:=$(1)
  DEPENDS:=@TARGET_lantiq_$(2) +kmod-crypto-manager
  FILES:=$(PKG_BUILD_DIR)/ltq_deu_$(1).ko
  AUTOLOAD:=$(call AutoProbe,ltq_deu_$(1))
endef

KernelPackage/ltq-deu-danube=$(call KernelPackage/ltq-deu-template,danube,xway)
KernelPackage/ltq-deu-ar9=$(call KernelPackage/ltq-deu-template,ar9,xway)
KernelPackage/ltq-deu-vr9=$(call KernelPackage/ltq-deu-template,vr9,xrx200)

define Build/Configure
endef

define Build/Compile
	cd $(LINUX_DIR); \
		ARCH=mips CROSS_COMPILE="$(KERNEL_CROSS)" \
		$(MAKE) BUILD_VARIANT=$(BUILD_VARIANT) M=$(PKG_BUILD_DIR) V=1 modules
endef

$(eval $(call KernelPackage,ltq-deu-danube))
$(eval $(call KernelPackage,ltq-deu-ar9))
$(eval $(call KernelPackage,ltq-deu-vr9))
