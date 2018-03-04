# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ltq-atm
PKG_RELEASE:=2
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/ltq-atm-$(BUILD_VARIANT)

PKG_MAINTAINER:=John Crispin <john@phrozen.org>
PKG_LICENSE:=GPL-2.0+

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ltq-atm-template
  SECTION:=sys
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=atm driver for $(1)
  URL:=http://www.lantiq.com/
  VARIANT:=$(1)
  DEPENDS:=@$(2) +kmod-atm +br2684ctl
  FILES:=$(PKG_BUILD_DIR)/ltq_atm_$(1).ko
endef

KernelPackage/ltq-atm-danube=$(call KernelPackage/ltq-atm-template,danube,(TARGET_lantiq_xway||TARGET_lantiq_xway_legacy) +kmod-ltq-adsl-danube-mei)
KernelPackage/ltq-atm-ar9=$(call KernelPackage/ltq-atm-template,ar9,TARGET_lantiq_xway +kmod-ltq-adsl-ar9-mei)
KernelPackage/ltq-atm-ase=$(call KernelPackage/ltq-atm-template,ase,TARGET_lantiq_ase +kmod-ltq-adsl-ase-mei)
KernelPackage/ltq-atm-vr9=$(call KernelPackage/ltq-atm-template,vr9,TARGET_lantiq_xrx200 +kmod-ltq-vdsl-vr9-mei)

define Build/Configure
endef

define Build/Compile
	cd $(LINUX_DIR); \
		ARCH=mips CROSS_COMPILE="$(KERNEL_CROSS)" \
		$(MAKE) BUILD_VARIANT=$(BUILD_VARIANT) M=$(PKG_BUILD_DIR) V=1 modules
endef

$(eval $(call KernelPackage,ltq-atm-danube))
$(eval $(call KernelPackage,ltq-atm-ase))
$(eval $(call KernelPackage,ltq-atm-ar9))
$(eval $(call KernelPackage,ltq-atm-vr9))
