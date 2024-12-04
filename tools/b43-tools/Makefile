#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=b43-tools
PKG_DATE:=2022-07-05

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/mbuesch/b43-tools.git
PKG_SOURCE_VERSION:=2fe10ea6690df9a068cb21cde537236bae784a14
PKG_MIRROR_HASH:=4009d6c1f9ede43102897d6ebf4bd954331f3c4e2a94d4e61c0e5f303f929914

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
	+$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR)/fwcutter \
		CFLAGS="$(HOST_CFLAGS) -include endian.h" \
		$(HOST_MAKE_FLAGS) \
		$(1) QUIET_SPARSE=:
	+$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR)/assembler \
		CFLAGS="$(HOST_CFLAGS) -include endian.h -Wno-error=int-conversion" \
		$(HOST_MAKE_FLAGS) \
		LDFLAGS= \
		$(1) QUIET_SPARSE=:
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/fwcutter/b43-fwcutter $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/assembler/b43-asm $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/assembler/b43-asm.bin $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) ./files/b43-fwsquash.py $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/b43-fwcutter
	rm -f $(STAGING_DIR_HOST)/bin/b43-asm
	rm -f $(STAGING_DIR_HOST)/bin/b43-asm.bin
	rm -f $(STAGING_DIR_HOST)/bin/b43-fwsquash.py
endef

$(eval $(call HostBuild))
