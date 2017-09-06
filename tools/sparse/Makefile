#
# Copyright (C) 2014 Qualcomm-Atheros Inc.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sparse

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://git.kernel.org/pub/scm/devel/sparse/sparse.git
PKG_SOURCE_DATE:=2017-03-31
PKG_SOURCE_VERSION:=14964df5373292af78b29529d4fc7e1a26b67a97
PKG_MIRROR_HASH:=90110e8a4d7db099ea79dd3b99a12135e24580d81b8662bd9a86e9f81bce9df7

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
       $(INSTALL_BIN) $(HOST_BUILD_DIR)/sparse $(STAGING_DIR_HOST)/bin
endef

$(eval $(call HostBuild))
