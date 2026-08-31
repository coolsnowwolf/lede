# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ltq-ptm
PKG_RELEASE:=6

PKG_MAINTAINER:=John Crispin <john@phrozen.org>
PKG_LICENSE:=GPL-2.0+

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ltq-ptm-template
  SECTION:=sys
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=ptm driver for $(1)
  VARIANT:=$(1)
  DEPENDS:=@$(2)
ifeq ($(1),vr9)
  DEPENDS+= +PACKAGE_kmod-ltq-ptm-$(1):kmod-ltq-vdsl-vr9-mei
else
  DEPENDS+= +PACKAGE_kmod-ltq-ptm-$(1):kmod-ltq-adsl-$(1)-mei
endif
ifeq ($(1),ase)
  DEPENDS+=@BROKEN
endif
  FILES:=$(PKG_BUILD_DIR)/ltq_ptm_$(1).ko 
endef

KernelPackage/ltq-ptm-danube=$(call KernelPackage/ltq-ptm-template,danube,TARGET_lantiq_xway)
KernelPackage/ltq-ptm-ar9=$(call KernelPackage/ltq-ptm-template,ar9,TARGET_lantiq_xway)
KernelPackage/ltq-ptm-ase=$(call KernelPackage/ltq-ptm-template,ase,TARGET_lantiq_ase)
KernelPackage/ltq-ptm-vr9=$(call KernelPackage/ltq-ptm-template,vr9,(TARGET_lantiq_xrx200||TARGET_lantiq_xrx200_legacy))

define Build/Configure
endef

define Build/Compile
	cd $(LINUX_DIR); \
		ARCH=mips CROSS_COMPILE="$(KERNEL_CROSS)" \
		$(MAKE) $(KERNEL_MAKE_FLAGS) BUILD_VARIANT=$(BUILD_VARIANT) M=$(PKG_BUILD_DIR) V=1 modules
endef

$(eval $(call KernelPackage,ltq-ptm-danube))
$(eval $(call KernelPackage,ltq-ptm-ase))
$(eval $(call KernelPackage,ltq-ptm-ar9))
$(eval $(call KernelPackage,ltq-ptm-vr9))
