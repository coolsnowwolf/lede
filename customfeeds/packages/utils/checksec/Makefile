#
# Copyright (C) 2020 CZ.NIC z.s.p.o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=checksec.sh
PKG_VERSION:=2.4.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/slimm609/checksec.sh/archive/$(PKG_VERSION)
PKG_HASH:=05bb28e22a916ff5f43d60ddf7b00f233618bbf8f283a059f8e0ceb695bc4ac0

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE_FILES:=LICENSE.txt

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/checksec/default
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=https://github.com/slimm609/checksec.sh
endef

define Package/checksec
  $(call Package/checksec/default)
  TITLE:=Utility to check PIE, RELRO, ASLR etc.
  DEPENDS:=+bash +file +binutils +procps-ng +procps-ng-sysctl +openssl-util +coreutils +coreutils-stat
endef

define Package/checksec_automator
  $(call Package/checksec/default)
  TITLE:=Utility to use checksec for dirs
  DEPENDS:=+checksec +coreutils-tee +findutils-find +grep
endef

define Package/checksec/description
  Checksec is a bash script to check the properties
  of executables (like PIE, RELRO, PaX, Canaries, ASLR, Fortify Source).
endef

define Package/checksec_automator/description
  Script for checksec directory scan.
endef

Build/Compile:=:
Build/Install:=:

define Package/checksec/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/checksec $(1)/usr/bin/
endef

define Package/checksec_automator/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/checksec_automator.sh $(1)/usr/bin/
endef

$(eval $(call BuildPackage,checksec))
$(eval $(call BuildPackage,checksec_automator))
