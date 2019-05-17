#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=trelay
PKG_VERSION:=0.1
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define KernelPackage/trelay
  SUBMENU:=Network Support
  TITLE:=Trivial Ethernet Relay
  FILES:=$(PKG_BUILD_DIR)/trelay.ko
  AUTOLOAD:=$(call AutoLoad,50,trelay)
endef

define KernelPackage/trelay/description
trelay relays ethernet packets between two devices (similar to a bridge), but
without any MAC address checks. This makes it possible to bridge client mode
or ad-hoc mode wifi devices to ethernet VLANs, assuming the remote end uses
the same source MAC address as the device that packets are supposed to exit
from.
endef

include $(INCLUDE_DIR)/kernel-defaults.mk

define Build/Compile
	$(KERNEL_MAKE) SUBDIRS="$(PKG_BUILD_DIR)" modules
endef

define KernelPackage/trelay/conffiles
/etc/config/trelay
endef

define KernelPackage/trelay/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/net $(1)/etc/init.d $(1)/etc/config
	$(INSTALL_CONF) ./files/trelay.hotplug $(1)/etc/hotplug.d/net/50-trelay
	$(INSTALL_BIN) ./files/trelay.init $(1)/etc/init.d/trelay
	$(INSTALL_CONF) ./files/trelay.config $(1)/etc/config/trelay
endef

$(eval $(call KernelPackage,trelay))
