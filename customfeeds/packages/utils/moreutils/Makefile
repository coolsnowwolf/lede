#
# Copyright (C) 2016 Nikil Mehta <nikil.mehta@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=moreutils
PKG_VERSION:=0.65
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://git.kitenet.net/index.cgi/moreutils.git/snapshot
PKG_HASH:=60c9f6b55204e64cfcd12fd66e75cf7a061b6761f3d5b7797f2452cb17598881

PKG_MAINTAINER:=Nikil Mehta <nikil.mehta@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=0

include $(INCLUDE_DIR)/package.mk

define Package/moreutils
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=additional Unix utilities
  URL:=https://joeyh.name/code/moreutils/
  DEPENDS:=+perl +perlbase-file +perlbase-getopt +perlbase-io +perlbase-ipc +perlbase-posix
endef

define Package/moreutils/description
	This is a growing collection of the Unix tools that nobody thought
	to write long ago, when Unix was young.
	.
	So far, it includes the following utilities:
	- chronic: runs a command quietly unless it fails
	- combine: combine the lines in two files using boolean operations
	- errno: look up errno names and descriptions
	- ifdata: get network interface info without parsing ifconfig output
	- ifne: run a program if the standard input is not empty
	- isutf8: check if a file or standard input is utf-8
	- lckdo: execute a program with a lock held
	- mispipe: pipe two commands, returning the exit status of the first
	- parallel: run multiple jobs at once
	- pee: tee standard input to pipes
	- sponge: soak up standard input and write to a file
	- ts: timestamp standard input
	- vidir: edit a directory in your text editor
	- vipe: insert a text editor into a pipe
	- zrun: automatically uncompress arguments to command
endef

define Package/moreutils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,moreutils))
