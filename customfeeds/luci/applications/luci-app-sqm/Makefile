# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI Support for SQM Scripts
LUCI_DESCRIPTION:=Luci interface for the SQM scripts queue management package

PKG_VERSION:=1.4.0
PKG_RELEASE:=9

PKG_MAINTAINER:=Toke Høiland-Jørgensen <toke@toke.dk>

LUCI_DEPENDS:=+sqm-scripts
LUCI_PKGARCH:=all

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature
