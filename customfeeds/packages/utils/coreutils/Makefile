#
# Copyright (C) 2008-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=coreutils
PKG_VERSION:=8.32
PKG_RELEASE:=6

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/coreutils
PKG_HASH:=4458d8de7849df44ccab15e16b1548b285224dbba5f08fac070c1c0e0bcc4cfa

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:gnu:coreutils

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

COREUTILS_APPLETS := \
	base32 base64 basename basenc b2sum cat chcon chgrp chmod chown chroot	\
	cksum comm cp csplit cut date dd df dir dircolors dirname du echo env	\
	expand expr factor false fmt fold groups head hostid id install join	\
	kill link ln logname ls md5sum mkdir mkfifo mknod mktemp mv nice nl	\
	nohup nproc numfmt od paste pathchk pinky pr printenv printf ptx pwd	\
	readlink realpath rm rmdir runcon seq sha1sum sha224sum sha256sum	\
	sha384sum sha512sum shred shuf sleep sort split stat stdbuf stty sum	\
	sync tac tail tee test timeout touch tr true truncate tsort tty uname	\
	unexpand uniq unlink uptime users vdir wc who whoami yes

DIR_BIN := \
	base64 cat chgrp chmod chown cp date dd df echo false kill link ln ls	\
	mkdir mknod mktemp mv nice printenv pwd rm rmdir sleep stat stty sync	\
	touch true uname

DIR_USR_BIN := \
	basename chcon cksum comm cut dirname du env expand expr factor fold	\
	groups head hostid id install logname md5sum mkfifo nl nohup nproc od	\
	paste printf readlink realpath runcon seq sha1sum sha256sum sha512sum	\
	shred shuf sort split sum tac tail tee test timeout tr truncate tty	\
	unexpand uniq unlink uptime users wc who whoami yes

DIR_USR_SBIN := \
	chroot

# BusyBox does not provide these yet
DIR_OTHERS := \
	base32 b2sum basenc csplit dir dircolors fmt join numfmt pathchk pinky	\
	pr ptx sha224sum sha384sum stdbuf tsort vdir

$(eval $(foreach a,$(DIR_BIN),ALTS_$(a):=300:/bin/$(a):/usr/libexec/$(a)-coreutils$(newline)))
$(eval $(foreach a,$(DIR_USR_BIN),ALTS_$(a):=300:/usr/bin/$(a):/usr/libexec/$(a)-coreutils$(newline)))
$(eval $(foreach a,$(DIR_USR_SBIN),ALTS_$(a):=300:/usr/sbin/$(a):/usr/libexec/$(a)-coreutils$(newline)))

DEPENDS_sort = +libpthread
DEPENDS_timeout = +librt
DEPENDS_expr = +libgmp
DEPENDS_factor = +libgmp
DEPENDS_cp = +libacl
DEPENDS_dir = +libacl +libcap
DEPENDS_install = +libacl
DEPENDS_ls = +libacl +libcap
DEPENDS_mv = +libacl
DEPENDS_vdir = +libacl +libcap

FILES_stdbuf := usr/lib/coreutils/libstdbuf.so

define Package/coreutils/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=The GNU core utilities
  URL:=http://www.gnu.org/software/coreutils/
endef

define Package/coreutils
  $(call Package/coreutils/Default)
  TITLE:=The GNU core utilities
  MENU:=1
endef

define Package/coreutils/description
 Full versions of standard GNU utilities. If an equivalent Busybox applet is
 available, you should consider compiling that instead as Busybox applets are
 usually smaller, at the expense of reduced functionality.
endef

define GenPlugin
 define Package/$(1)
   $(call Package/coreutils/Default)
   DEPENDS:=coreutils $(DEPENDS_$(2))
   TITLE:=Utility $(2) from the GNU core utilities
   ALTERNATIVES:=$(ALTS_$(2))
 endef

 define Package/$(1)/description
  Full version of standard GNU $(2) utility.
 endef
endef

$(foreach a,$(COREUTILS_APPLETS),$(eval $(call GenPlugin,coreutils-$(a),$(a))))

CONFIGURE_VARS += \
	gl_cv_func_mbrtowc_incomplete_state=yes \
	gl_cv_func_mbrtowc_retval=yes \
	gl_cv_func_wcrtomb_retval=yes \
	ac_cv_header_selinux_context_h=no \
	ac_cv_header_selinux_flash_h=no \
	ac_cv_header_selinux_selinux_h=no \
	ac_cv_search_setfilecon=no

CONFIGURE_ARGS += \
	--disable-xattr \
	--enable-install-program=su \
	--enable-threads=posix \
	--enable-acl \
	--disable-assert \
	--disable-rpath \
	--disable-libsmack \
	--enable-libcap \
	--without-linux-crypto \
	--without-openssl \
	--$(if $(CONFIG_USE_MUSL),with,without)-included-regex \
	--without-selinux \
	--with-gmp

define Package/coreutils/install
	true
endef

define BuildPlugin
  define Package/$(1)/install
	$(INSTALL_DIR) $$(1)/usr/$(if $(ALTS_$(2)),libexec,bin)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/$(2) $$(1)/usr/$(if $(ALTS_$(2)),libexec/$(2)-coreutils,bin/$(2))
	$(foreach f,$(FILES_$(2)),
		$(INSTALL_DIR) $$(1)/$(dir $(f))
		$(INSTALL_BIN) $(PKG_INSTALL_DIR)/$(f) $$(1)/$(f)
	)
  endef

  $$(eval $$(call BuildPackage,$(1)))
endef

$(eval $(call BuildPackage,coreutils))

$(foreach a,$(COREUTILS_APPLETS),$(eval $(call BuildPlugin,coreutils-$(a),$(a))))
