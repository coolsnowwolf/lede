# 
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=bc
PKG_VERSION:=1.06.95

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://alpha.gnu.org/gnu/bc \
	http://gnualpha.uib.no/bc/ \
	http://mirrors.fe.up.pt/pub/gnu-alpha/bc/ \
	http://www.nic.funet.fi/pub/gnu/alpha/gnu/bc/
PKG_HASH:=7ee4abbcfac03d8a6e1a8a3440558a3d239d6b858585063e745c760957725ecc

PKG_FIXUP := autoreconf

include $(INCLUDE_DIR)/host-build.mk

$(eval $(call HostBuild))
