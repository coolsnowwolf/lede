#
# Copyright (C) 2022 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=libdeflate
PKG_VERSION:=1.22
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/ebiggers/libdeflate/releases/download/v$(PKG_VERSION)
PKG_HASH:=7834d9adbc9a809e0fb0d7b486060a9ae5f7819eb7f55bb8c22b10d7b3bed8da

include $(INCLUDE_DIR)/host-build.mk

DECOMPRESS_CMD:=$(STAGING_DIR_HOST)/bin/gzip -dc $(DL_DIR)/$(PKG_SOURCE) |

define Host/Compile
	$(HOSTCC_NOCACHE) $(HOST_CFLAGS) $(HOST_LDFLAGS) \
	$(HOST_BUILD_DIR)/lib/*{,/*}.c \
	$(HOST_BUILD_DIR)/programs/{gzip,prog_util,tgetopt}.c \
	-o $(HOST_BUILD_DIR)/libdeflate-gzip
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/libdeflate-gzip $(STAGING_DIR_HOST)/bin/
	$(LN) libdeflate-gzip $(STAGING_DIR_HOST)/bin/libdeflate-gunzip
endef

define Host/Uninstall
	$(RM) $(STAGING_DIR_HOST)/bin/libdeflate-gzip
	$(RM) $(STAGING_DIR_HOST)/bin/libdeflate-gunzip
	$(call Host/Uninstall/Default)
endef

$(eval $(call HostBuild))
