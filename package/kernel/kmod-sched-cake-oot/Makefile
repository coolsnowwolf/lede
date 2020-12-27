#
# Copyright (C) 2016 LEDE
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=sched-cake-oot
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/dtaht/sch_cake.git
PKG_SOURCE_DATE:=2020-07-24
PKG_SOURCE_VERSION:=48979385757f3408c3427b3ebbf5963efdada5aa
PKG_MIRROR_HASH:=0e8ed53f55e28ad0c30f20293c16988876bae8d6f70f5629a43cf2ce1e49fc51
PKG_MAINTAINER:=Kevin Darbyshire-Bryant <ldir@darbyshire-bryant.me.uk>

include $(INCLUDE_DIR)/package.mk

define KernelPackage/sched-cake-oot
  SUBMENU:=Network Support
  TITLE:=OOT Cake fq_codel/blue derived shaper
  URL:=https://github.com/dtaht/sch_cake
  FILES:=$(PKG_BUILD_DIR)/sch_cake.ko
  AUTOLOAD:=$(call AutoLoad,75,sch_cake)
  DEPENDS:=@LINUX_4_14 +kmod-sched-core +kmod-ipt-conntrack
endef

define KernelPackage/sched-cake-oot/description
  O(ut) O(f) T(ree) Common Applications Kept Enhanced fq_codel/blue derived shaper
endef

define KernelPackage/sched-cake-virtual
  SUBMENU:=Network Support
  TITLE:=Virtual package for sched-cake
  URL:=https://github.com/dtaht/sch_cake
  DEPENDS:=+!LINUX_4_14:kmod-sched-cake +LINUX_4_14:kmod-sched-cake-oot
endef

define KernelPackage/sched-cake-virtual/description
  Virtual package for resolving sch_cake dependencies
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" modules
endef

$(eval $(call KernelPackage,sched-cake-oot))
$(eval $(call KernelPackage,sched-cake-virtual))
