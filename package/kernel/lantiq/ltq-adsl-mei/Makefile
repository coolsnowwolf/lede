# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ltq-adsl-mei
PKG_RELEASE:=1

PKG_MAINTAINER:=John Crispin <john@phrozen.org>
PKG_CHECK_FORMAT_SECURITY:=0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ltq-adsl-mei-template
  SECTION:=sys
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=mei driver for $(1)
  URL:=http://www.lantiq.com/
  VARIANT:=$(1)
  DEPENDS:=@$(2)
  FILES:=$(PKG_BUILD_DIR)/drv_mei_cpe.ko
  AUTOLOAD:=$(call AutoLoad,50,drv_mei_cpe)
endef

KernelPackage/ltq-adsl-danube-mei=$(call KernelPackage/ltq-adsl-mei-template,danube,(TARGET_lantiq_xway||TARGET_lantiq_xway_legacy))
KernelPackage/ltq-adsl-ar9-mei=$(call KernelPackage/ltq-adsl-mei-template,ar9,TARGET_lantiq_xway)
KernelPackage/ltq-adsl-ase-mei=$(call KernelPackage/ltq-adsl-mei-template,ase,TARGET_lantiq_ase)

define Build/Configure
endef

define Build/Compile
	cd $(LINUX_DIR); \
		ARCH=mips CROSS_COMPILE="$(KERNEL_CROSS)" \
		$(MAKE) BUILD_VARIANT=$(BUILD_VARIANT) M=$(PKG_BUILD_DIR)/ V=1 modules
endef

$(eval $(call KernelPackage,ltq-adsl-danube-mei))
$(eval $(call KernelPackage,ltq-adsl-ase-mei))
$(eval $(call KernelPackage,ltq-adsl-ar9-mei))
