#
# Copyright (C) 2016 Nikil Mehta <nikil.mehta@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fping
PKG_VERSION:=5.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://fping.org/dist
PKG_HASH:=ed38c0b9b64686a05d1b3bc1d66066114a492e04e44eef1821d43b1263cd57b8

PKG_MAINTAINER:=Nikil Mehta <nikil.mehta@gmail.com>
PKG_LICENSE:=BSD-4-Clause
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/fping
  SECTION:=net
  CATEGORY:=Network
  TITLE:=sends ICMP ECHO_REQUEST packets to network hosts
  URL:=https://fping.org/
endef

define Package/fping/description
  fping is a ping like program which uses the Internet Control Message Protocol
  (ICMP) echo request to determine if a target host is responding.  fping
  differs from ping in that you can specify any number of targets on the command
  line, or specify a file containing the lists of targets to ping.  Instead of
  sending to one target until it times out or replies, fping will send out a
  ping packet and move on to the next target in a round-robin fashion.
endef

CONFIGURE_ARGS += \
	--enable-ipv4 \
	$(if $(CONFIG_IPV6),en,dis)able-ipv6

define Package/fping/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_SUID) $(PKG_INSTALL_DIR)/usr/sbin/fping $(1)/usr/bin/
endef

$(eval $(call BuildPackage,fping))
