#
# Copyright (C) 2016-2017 Mauro Mozzarelli
#
# This is free software, licensed under the GNU General Public License
# See /LICENSE for more information.
#
# AUTHOR: Mauro Mozzarelli <mauro@ezplanet.org>
#
include $(TOPDIR)/rules.mk

PKG_NAME:=ipvsadm
PKG_VERSION:=1.31
PKG_MAINTAINER:=Mauro Mozzarelli <mauro@ezplanet.org>, \
		Florian Eckert <fe@dev.tdt.de>
PKG_LICENSE:=GPL-2.0-or-later
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/kernel/ipvsadm/
PKG_HASH:=1a0a5e25b5a1226435d2fb76341656f83a710183aebb0d204db39c0ec3bedfdb

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ipvsadm
  SECTION:=net
  CATEGORY:=Network
  TITLE:=IP Virtual Server Configuration Manager
  URL:=http://www.linuxvirtualserver.org
  DEPENDS:= +kmod-nf-ipvs +libnl-genl +libpopt
endef

define Package/ipvsadm/description
  IPVS (IP Virtual Server) implements transport-layer load balancing
  inside the Linux kernel, so called Layer-4 switching. The command line tool
  ipvsadm is used to set up, maintain or inspect the virtual server table
  in the Linux kernel. The Linux Virtual Server can be used to build scalable
  network services based on a cluster of two or more nodes.
endef

define Package/ipvsadm/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ipvsadm $(1)/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ipvsadm-save $(1)/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ipvsadm-restore $(1)/sbin/
endef

$(eval $(call BuildPackage,ipvsadm))
