#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=elftosb
PKG_VERSION:=10.12.01

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://repository.timesys.com/buildsources/e/elftosb/elftosb-10.12.01/
PKG_HASH:=77bb6981620f7575b87d136d94c7daa88dd09195959cc75fc18b138369ecd42b

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) LDFLAGS="$(HOST_LDFLAGS)"
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bld/linux/elftosb $(STAGING_DIR_HOST)/bin/elftosb
endef

$(eval $(call HostBuild))
