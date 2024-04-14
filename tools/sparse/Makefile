#
# Copyright (C) 2014 Qualcomm-Atheros Inc.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sparse

PKG_VERSION:=0.6.4
PKG_HASH:=6ab28b4991bc6aedbd73550291360aa6ab3df41f59206a9bde9690208a6e387c
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/software/devel/sparse/dist/

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
       $(INSTALL_BIN) $(HOST_BUILD_DIR)/sparse $(STAGING_DIR_HOST)/bin
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/sparse
endef

$(eval $(call HostBuild))
