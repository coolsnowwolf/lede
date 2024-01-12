#
# Copyright (C) 2022 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=libdeflate
PKG_VERSION:=1.17
PKG_RELEASE:=2

PKG_SOURCE_URL:=https://github.com/ebiggers/libdeflate.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_MIRROR_HASH:=ee5790cf3140aa6a2e0f0c400d4b32539f13cb270e9357135c51927ba3784dc7

include $(INCLUDE_DIR)/host-build.mk

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
