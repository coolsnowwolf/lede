# SPDX-License-Identifier: GPL-2.0-only
include $(TOPDIR)/rules.mk

PKG_NAME:=cpio
PKG_CPE_ID:=cpe:/a:gnu:cpio
PKG_VERSION:=2.15

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@GNU/cpio
PKG_HASH:=937610b97c329a1ec9268553fb780037bcfff0dcffe9725ebc4fd9c1aa9075db

include $(INCLUDE_DIR)/host-build.mk

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
