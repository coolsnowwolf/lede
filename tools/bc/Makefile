# 
# Copyright (C) 2013-2022 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=bc
PKG_VERSION:=1.07.1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/bc
PKG_HASH:=62adfca89b0a1c0164c2cdca59ca210c1d44c3ffc46daf9931cf4942664cb02a

PKG_FIXUP := autoreconf
PKG_CPE_ID:=cpe:/a:gnu:bc

include $(INCLUDE_DIR)/host-build.mk

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
