#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bash
PKG_VERSION:=5.1
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/bash
PKG_HASH:=cc012bc860406dcf42f64431bcd3d2fa7560c02915a601aba9cd597a39329baa

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:gnu:bash

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/bash
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Shells
  TITLE:=The GNU Bourne Again SHell
  DEPENDS:=+libncurses +libreadline
  URL:=http://www.gnu.org/software/bash/
endef

define Package/bash/description
  Bash is an sh-compatible command language interpreter that executes
  commands read from the standard input or from a file. Bash also
  incorporates useful features from the Korn and C shells (ksh and csh).
endef

define Package/bash/conffiles
/etc/bash.bashrc
/etc/bash.bash_logout
endef

# Bash detects and enables certain features by runtest simple piece of code
# which is not viable when doing cross compilation and default to no in many
# cases.  Grep for 'cross.compil' in aclocal.m4 and config-bot.h for details
#
# bash_cv_job_control_missing: Job control is needed for bash to be used as the
# default shell
#
# bash_cv_getcwd_malloc: The replacement by bash does not work well with
# overlayfs.  See "bash getcwd errors when running under OverlayFS"
# http://permalink.gmane.org/gmane.linux.embedded.yocto.general/25204
#
# bash_cv_dev_fd: The test is done on build system but OpenWrt currently does
# not have /dev/fd, it provides /proc/self/fd/ instead
#
# bash_cv_sys_named_pipes: Required for process substituion
CONFIGURE_VARS += \
	ac_cv_rl_prefix="$(STAGING_DIR)/usr" \
	ac_cv_rl_version="8.0" \
	bash_cv_getcwd_malloc=yes \
	bash_cv_job_control_missing=present \
	bash_cv_dev_fd=whacky \
	bash_cv_sys_named_pipes=present \

# The following are some other tests that cannot run when cross-compiling
CONFIGURE_VARS += \
	bash_cv_func_sigsetjmp=present \
	bash_cv_printf_a_format=yes \

CONFIGURE_ARGS+= \
	--with-curses \
	--without-bash-malloc \
	--bindir=/bin \
	--disable-rpath \
	--enable-direxpand-default \
	--enable-job-control \
	--enable-readline

define Package/bash/postinst
#!/bin/sh
grep -q bash "$${IPKG_INSTROOT}/etc/shells" || {
	echo /bin/bash >> "$${IPKG_INSTROOT}/etc/shells"
	echo /bin/rbash >> "$${IPKG_INSTROOT}/etc/shells"
}
endef

define Package/bash/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/bash $(1)/bin/
	$(LN) bash $(1)/bin/rbash
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,bash))
