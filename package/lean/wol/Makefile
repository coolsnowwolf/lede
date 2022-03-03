# 
# Copyright (C) 2007-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wol
PKG_VERSION:=0.7.1
PKG_RELEASE:=3

PKG_SOURCE_URL:=@SF/ahh
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MD5SUM:=c2fa9d7e771134ac8c89d56b8197d4ca

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/wol
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Wake On LAN client
  URL:=http://ahh.sourceforge.net/wol/
endef

define Package/wol/description
 wol implements Wake On LAN functionality in a small program.
 It wakes up hardware that is Magic Packet compliant.
endef

CONFIGURE_VARS += \
	ac_cv_func_malloc_0_nonnull=yes \
	ac_cv_func_realloc_0_nonnull=yes \
	ac_cv_func_mmap_fixed_mapped=yes \
	jm_cv_func_working_malloc=yes \
	ac_cv_func_alloca_works=yes \

define Package/wol/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/$(PKG_NAME) $(1)/usr/bin/
endef

define Package/wol/conffiles
/etc/config/wol
endef

$(eval $(call BuildPackage,wol))