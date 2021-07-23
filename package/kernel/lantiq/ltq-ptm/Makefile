# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ltq-ptm
PKG_RELEASE:=3

PKG_MAINTAINER:=John Crispin <john@phrozen.org>
PKG_LICENSE:=GPL-2.0+

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ltq-ptm-template
  SECTION:=sys
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=ptm driver for $(1)
  URL:=http://www.lantiq.com/
  VARIANT:=$(1)
  DEPENDS:=@TARGET_lantiq_$(2)
ifeq ($(1),vr9)
  DEPENDS+= +kmod-ltq-vdsl-vr9-mei
else
  DEPENDS+= +kmod-ltq-adsl-$(1)-mei
endif
ifeq ($(1),ase)
  DEPENDS+=@BROKEN
endif
  FILES:=$(PKG_BUILD_DIR)/ltq_ptm_$(1).ko 
endef

KernelPackage/ltq-ptm-danube=$(call KernelPackage/ltq-ptm-template,danube,xway)
KernelPackage/ltq-ptm-ar9=$(call KernelPackage/ltq-ptm-template,ar9,xway)
KernelPackage/ltq-ptm-ase=$(call KernelPackage/ltq-ptm-template,ase,ase)
KernelPackage/ltq-ptm-vr9=$(call KernelPackage/ltq-ptm-template,vr9,xrx200)

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
