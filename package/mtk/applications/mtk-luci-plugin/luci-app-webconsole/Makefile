include $(TOPDIR)/rules.mk

PKG_LICENSE:=GPLv2
PKG_MAINTAINER:=Hua Shao <nossiac@163.com>

LUCI_TITLE:=A simple web console.
LUCI_DEPENDS:=

# earlier version of luci put "luci.mk" somewhere else...
LUCI_MK_OLDPATH:=$(shell test -e ../luci.mk && echo "old")
LUCI_MK_NEWPATH:=$(shell test -e ../../luci.mk && echo "new")
ifeq ($(LUCI_MK_OLDPATH),old)
include ../luci.mk
endif
ifeq ($(LUCI_MK_NEWPATH),new)
include ../../luci.mk
endif

# call BuildPackage - OpenWrt buildroot signature
