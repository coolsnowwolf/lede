#
# Copyright (C) 2022 Chion Tang <tech@chionlab.moe>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fullconenat
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2023-01-01
PKG_SOURCE_URL:=https://github.com/llccd/netfilter-full-cone-nat
PKG_SOURCE_VERSION:=74c5e6f3c7faaf33ece451697537c81781781c20
PKG_MIRROR_HASH:=3c254f1edba28eafdccac9cf95eb550fd2b05eeaaec8a02c73e1dcd2f98f9d93

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Chion Tang <tech@chionlab.moe>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/ipt-fullconenat
  SUBMENU:=Netfilter Extensions
  TITLE:=FULLCONENAT netfilter module
  DEPENDS:=+kmod-nf-ipt +kmod-nf-nat
  KCONFIG:=  \
	CONFIG_NF_CONNTRACK_EVENTS=y \
	CONFIG_NF_CONNTRACK_CHAIN_EVENTS=y
  FILES:=$(PKG_BUILD_DIR)/xt_FULLCONENAT.ko
  AUTOLOAD:=$(call AutoProbe,xt_FULLCONENAT)
endef

define Package/iptables-mod-fullconenat
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE:=FULLCONENAT iptables extension
  DEPENDS:=+libxtables +kmod-ipt-fullconenat
endef

define Package/iptables-mod-fullconenat/install
	$(INSTALL_DIR) $(1)/usr/lib/iptables
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libipt_FULLCONENAT.so $(1)/usr/lib/iptables
endef

define Build/Compile
	+$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" modules
	$(call Build/Compile/Default)
endef

$(eval $(call KernelPackage,ipt-fullconenat))
$(eval $(call BuildPackage,iptables-mod-fullconenat))
