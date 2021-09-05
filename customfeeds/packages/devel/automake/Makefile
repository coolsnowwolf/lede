#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=automake
PKG_VERSION:=1.16.3
PKG_RELEASE:=1

PKG_SOURCE_URL:=@GNU/automake
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=ff2bf7656c4d1c6fdda3b8bebb21f09153a736bcba169aaf65eab25fa113bf3a

PKG_MAINTAINER:=Heinrich Schuchardt <xypron.glpk@gmx.de>
PKG_LICENSE:=GPL-3.0-or-later
PKG_CPE_ID:=cpe:/a:gnu:automake

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/automake
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=automake
  URL:=https://www.gnu.org/software/automake/
  DEPENDS:=+autoconf +perlbase-thread +perlbase-attributes
endef

define Package/automake/description
  Automake is a tool for automatically generating Makefile.in files compliant
  with the GNU Coding Standards.
endef

FIX_PATHS = $(SED) '1c \#!/usr/bin/perl' -e 's| /[^ ]*/bin/perl| /usr/bin/perl|g'

AM_VERSION:=$(word 1,$(subst ., ,$(PKG_VERSION))).$(word 2,$(subst ., ,$(PKG_VERSION)))

define Package/automake/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/automake-$(AM_VERSION) \
	  $(1)/usr/bin/automake-$(AM_VERSION)
	$(LN) automake-$(AM_VERSION) $(1)/usr/bin/automake
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/aclocal-$(AM_VERSION) \
	  $(1)/usr/bin/aclocal-$(AM_VERSION)
	$(LN) aclocal-$(AM_VERSION) $(1)/usr/bin/aclocal
	$(FIX_PATHS) $(1)/usr/bin/automake-$(AM_VERSION)
	$(FIX_PATHS) $(1)/usr/bin/aclocal-$(AM_VERSION)
	$(INSTALL_DIR) $(1)/usr/share/automake-$(AM_VERSION)

	for dir in \
	  automake-$(AM_VERSION) automake-$(AM_VERSION)/Automake \
	  automake-$(AM_VERSION)/am aclocal \
	  aclocal-$(AM_VERSION) aclocal-$(AM_VERSION)/internal \
	; do \
		$(INSTALL_DIR) $(1)/usr/share/$$$$dir; \
		for file in $$$$(cd $(PKG_INSTALL_DIR) && \
		  find usr/share/$$$$dir -maxdepth 1 -type f); do \
			$(INSTALL_DATA) $$(PKG_INSTALL_DIR)/$$$$file \
			$(1)/$$$$file; \
		done; \
	done
endef

$(eval $(call BuildPackage,automake))
