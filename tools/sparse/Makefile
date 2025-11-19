#
# Copyright (C) 2014 Qualcomm-Atheros Inc.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sparse

PKG_VERSION:=0.6.4
PKG_HASH:=8b907c007459a66db110496f0a02fcff1c3c8b67ddff37b959fb102a28424209

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
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
