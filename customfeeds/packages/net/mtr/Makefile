#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtr
PKG_VERSION:=0.94
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.bitwizard.nl/mtr/files
PKG_HASH:=cb5ffc803d136f7136b49b950abbc2a27d2a5ba62195de5b70f8ef9f0fcf2791

PKG_MAINTAINER:=Jonathan McCrohan <jmccrohan@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:matt_kimball_and_roger_wolff:mtr

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/mtr
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libncurses
  TITLE:=Full screen ncurses traceroute tool
  URL:=https://www.bitwizard.nl/mtr/
endef

define Package/mtr/description
	mtr combines the functionality of the 'traceroute' and 'ping' programs
	in a single network diagnostic tool.
	As mtr starts, it investigates the network connection between the host
	mtr runs on and a user-specified destination host. After it
	determines the address of each network hop between the machines,
	it sends a sequence ICMP ECHO requests to each one to determine the
	quality of the link to each machine. As it does this, it prints
	running statistics about each machine.
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

CONFIGURE_ARGS += \
	--without-gtk \
	--without-jansson \
	$(call autoconf_bool,CONFIG_IPV6,ipv6)

CONFIGURE_VARS += ac_cv_lib_cap_cap_set_proc=no

define Package/mtr/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/mtr $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/mtr-packet $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,mtr))
