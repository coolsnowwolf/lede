#
# Copyright (C) 2007-2011 OpenWrt.org
# Copyright (c) 2009-2014 Thorsten Glaser <tg@mirbsd.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mksh
PKG_VERSION:=59c
PKG_RELEASE:=1

PKG_MAINTAINER:=Thorsten Glaser <tg@mirbsd.org>, \
		Alif M. Ahmad <alive4ever@live.com>
PKG_LICENSE:=MirOS

PKG_SOURCE:=$(PKG_NAME)-R$(PKG_VERSION).tgz
PKG_SOURCE_URL:=http://www.mirbsd.org/MirOS/dist/mir/mksh \
		http://pub.allbsd.org/MirOS/dist/mir/mksh
PKG_HASH:=77ae1665a337f1c48c61d6b961db3e52119b38e58884d1c89684af31f87bc506

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/mksh
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Shells
  TITLE:=MirBSD Korn Shell
  DEPENDS:=$(DEP)
  URL:=http://mirbsd.de/mksh
endef

define Package/mksh/description
mksh is the MirBSD enhanced version of the Public Domain Korn
shell (pdksh), a Bourne-compatible shell which is largely si-
milar to the original AT&T Korn shell; mksh is the only pdksh
derivate currently being actively developed.  It includes bug
fixes and feature improvements, in order to produce a modern,
robust shell good for interactive and especially script use.
mksh has UTF-8 support (in substring operations and the Emacs
editing mode) and - while R59 corresponds to OpenBSD 5.7-cur-
rent ksh (without GNU bash-like PS1 and fancy character clas-
ses) - adheres to SUSv4 and is much more robust. The code has
been cleaned up and simplified, bugs fixed, standards compli-
ance added, and several enhancements (for extended compatibi-
lity to other modern shells - as well as a couple of its own)
are available. It has sensible defaults as usual with BSD.
endef

define Build/Compile
	# -DMKSH_SMALL=1 ⇒ reduce functionality quite a lot
	# -DMKSH_ASSUME_UTF8=0 ⇒ never automatically enable
	#	UTF-8 mode, neither use setlocale/nl_langinfo
	#	nor look at $LC_* and $LANG (not recommended)
	# -DMKSH_BINSHPOSIX ⇒ enable POSIX mode if called as sh
	#XXX maybe change to -DMKSH_ASSUME_UTF8=1 now (which
	#XXX is always assume UTF-8 mode)
	# HAVE_CAN_FSTACKPROTECTORALL=0 ⇒ nuke libssp dependency
	# HAVE_CAN_FSTACKPROTECTORSTRONG=0 ⇒ same, for gcc 4.9+
	cd $(PKG_BUILD_DIR); \
		CC="$(TARGET_CC)" \
		TARGET_OS="$(shell uname -s)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -DMKSH_SMALL=1 -DMKSH_ASSUME_UTF8=0 -DMKSH_BINSHPOSIX -DMKSHRC_PATH=\\\"/etc/mkshrc\\\"" \
		HAVE_CAN_FSTACKPROTECTORALL=0 \
		HAVE_CAN_FSTACKPROTECTORSTRONG=0 \
		LDFLAGS="$(TARGET_LDFLAGS)" \
			$(BASH) Build.sh -Q -r
endef

define Package/mksh/postinst
#!/bin/sh
grep mksh $${IPKG_INSTROOT}/etc/shells || \
	echo "/bin/mksh" >> $${IPKG_INSTROOT}/etc/shells
endef

define Package/mksh/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/dot.mkshrc $(1)/etc/mkshrc
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mksh $(1)/bin/
endef

define Package/mksh/conffiles
/etc/mkshrc
endef

$(eval $(call BuildPackage,mksh))
