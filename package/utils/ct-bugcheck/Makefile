#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ct-bugcheck
PKG_RELEASE:=2016-07-21

include $(INCLUDE_DIR)/package.mk

define Package/ct-bugcheck
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Bug checking and reporting utility
  VERSION:=$(PKG_RELEASE)
  MAINTAINER:=Ben Greear <greearb@candelatech.com>
endef

define Package/ct-bugcheck/description
  Scripts to check for bugs (like firmware crashes) and package them for reporting.
  Currently this script only checks for ath10k firmware crashes.
  Once installed, you can enable this tool by creating a file called
  /etc/config/bugcheck with the following contents:
 DO_BUGCHECK=1
 export DO_BUGCHECK

endef

define Build/Prepare
	$(CP) src/bugcheck.sh $(PKG_BUILD_DIR)/
	$(CP) src/bugchecker.sh $(PKG_BUILD_DIR)/
	$(CP) src/bugcheck.initd $(PKG_BUILD_DIR)/
endef

define Build/Compile
	true
endef

define Package/ct-bugcheck/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bugcheck.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bugchecker.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bugcheck.initd $(1)/etc/init.d/bugcheck
endef

$(eval $(call BuildPackage,ct-bugcheck))
