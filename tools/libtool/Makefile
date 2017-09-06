# 
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=libtool
PKG_VERSION:=2.4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=afcce660d3dc54c63a0a5ba3cf05272239dc3c54bbeba20f6bad250f9dc007ae

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_VARS += \
	lt_cv_sys_dlsearch_path=""

define Host/Prepare
	$(call Host/Prepare/Default)
	(cd $(STAGING_DIR_HOST)/share/aclocal/ && rm -f libtool.m4 ltdl.m4 lt~obsolete.m4 ltoptions.m4 ltsugar.m4 ltversion.m4)
	(cd $(HOST_BUILD_DIR); $(AM_TOOL_PATHS) ./bootstrap)
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) install
	$(SED) 's,\(hardcode_into_libs\)=yes,\1=no,g' $(STAGING_DIR_HOST)/bin/libtool
	$(CP) $(STAGING_DIR_HOST)/bin/libtool $(STAGING_DIR_HOST)/bin/libtool-ucxx
	$(SED) 's,-lstdc++,-luClibc++,g' $(STAGING_DIR_HOST)/bin/libtool-ucxx
endef

define Host/Clean
	-$(MAKE) -C $(HOST_BUILD_DIR) uninstall
	$(call Host/Clean/Default)
endef

$(eval $(call HostBuild))
