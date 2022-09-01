# SPDX-License-Identifier: GPL-2.0-or-later

include $(TOPDIR)/rules.mk

PKG_NAME:=xxd
PKG_VERSION:=1.10
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://grail.cba.csuohio.edu/~somos/
PKG_HASH:=9bf05c19b9084973e3cc877696a7f9881a5c87fa5a9fa438d9962519726559f9
PKG_CPE_ID:=cpe:/a:vim:vim

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/xxd $(STAGING_DIR_HOST)/bin/
endef

$(eval $(call HostBuild))
