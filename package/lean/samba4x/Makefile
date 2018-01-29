#
# Copyright (C) 2007-2015 wongsyrone
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=samba4x
PKG_VERSION:=4.4.2
PKG_RELEASE:=1

PKG_SOURCE_URL:= \
	http://download.samba.org/pub/samba/stable \
	http://ftp.samba.org/pub/samba \
	http://ftp.samba.org/pub/samba/old-versions

PKG_SOURCE:=samba-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/samba-$(PKG_VERSION)
PKG_MD5SUM:=03a65a3adf08ceb1636ad59d234d7f9d

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_DEPENDS:=heimdal

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
# for "uname release type" check
include $(INCLUDE_DIR)/kernel.mk


define Package/samba4x-server
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Samba $(PKG_VERSION) SMB/CIFS server
  URL:=http://www.samba.org/
  DEPENDS:=+zlib +libpthread +libpopt +libcap +python-light
#DEPENDS+= +libreadline +libuuid +libtirpc libtirpc
endef

define Package/samba4x-server/description
 The Samba software suite is a collection of programs that implements the
 SMB protocol for UNIX systems, allowing you to serve files and printers to
 Windows, NT, OS/2 and DOS clients. This protocol is sometimes also referred
 to as the LanManager or Netbios protocol.
endef


# fix include "dbwrap/dbwrap_open.h" not found in auth/credentials/credentials_secrets.c -I$(PKG_BUILD_DIR)/source3/lib
EXTRA_CFLAGS += \
	-ffunction-sections -fdata-sections \
	-Wno-deprecated-declarations
EXTRA_LDFLAGS += -Wl,--gc-sections

# Use host compiled python2 as interpreter
# Use target python2-config to retrive cflags, ldflags, libs, etc.
# Must have python2 patched to make python2-config works

#	_python_sysroot="$(STAGING_DIR)" \
	_python_prefix="/usr" \
	_python_exec_prefix="/usr/bin" \
	PYTHON="$(STAGING_DIR_HOST)/bin/python2"
	
CONFIGURE_VARS += \
	PYTHON_CONFIG="$(STAGING_DIR)/usr/bin/python2-config" \
	python_LDFLAGS="" \
	python_LIBDIR=""

# XXX: Be sure to take care of commas(,) or you will get 'out of range' error when waf configuring settings.
SAMBA4_IDMAP_MODULES_SHARED := idmap_ad,idmap_rid,idmap_adex,idmap_hash,idmap_tdb2,
SAMBA4_IDMAP_MODULES_STATIC := 
SAMBA4_PDB_MODULES_SHARED := pdb_wbc_sam,pdb_samba4,
SAMBA4_PDB_MODULES_STATIC := pdb_smbpasswd,pdb_tdbsam,
SAMBA4_AUTH_MODULES_SHARED := auth_wbc,auth_script,auth_samba4
SAMBA4_AUTH_MODULES_STATIC := auth_sam,auth_unix,
SAMBA4_VFS_MODULES_STATIC := vfs_default,vfs_aio_pthread


SAMBA4_MODULES_SHARED = "${SAMBA4_IDMAP_MODULES_SHARED}${SAMBA4_PDB_MODULES_SHARED}${SAMBA4_AUTH_MODULES_SHARED}"
SAMBA4_MODULES_STATIC = "${SAMBA4_IDMAP_MODULES_STATIC}${SAMBA4_PDB_MODULES_STATIC}${SAMBA4_AUTH_MODULES_STATIC}${SAMBA4_VFS_MODULES_STATIC}"

# Taken from package-defaults.mk
SAMBA_CONFIGURE_ARGS += \
		--target=$(GNU_TARGET_NAME) \
		--program-prefix="" \
		--prefix=$(CONFIGURE_PREFIX) \
		--exec-prefix=$(CONFIGURE_PREFIX) \
		--bindir=$(CONFIGURE_PREFIX)/bin \
		--sbindir=$(CONFIGURE_PREFIX)/sbin \
		--libexecdir=$(CONFIGURE_PREFIX)/lib \
		--sysconfdir=/etc \
		--datadir=$(CONFIGURE_PREFIX)/share \
		--localstatedir=/var \
		--mandir=$(CONFIGURE_PREFIX)/man \
		--infodir=$(CONFIGURE_PREFIX)/info

# Should have a look before compiling.
SAMBA43_CACHE := " \
	\nChecking simple C program: OK \
	\nrpath library support: OK \
	\n-Wl,--version-script support: OK \
	\nChecking getconf LFS_CFLAGS: NO \
	\nChecking for large file support without additional flags: OK \
	\nChecking for -D_LARGE_FILES: OK \
	\nChecking correct behavior of strtoll: NO \
	\nChecking for working strptime: OK \
	\nChecking for C99 vsnprintf: OK \
	\nChecking for HAVE_SHARED_MMAP: OK \
	\nChecking for HAVE_MREMAP: OK \
	\nChecking for HAVE_INCOHERENT_MMAP: NO \
	\nChecking for HAVE_SECURE_MKSTEMP: OK \
	\nChecking for HAVE_IFACE_GETIFADDRS: OK \
	\nChecking for kernel change notify support: OK \
	\nChecking for Linux kernel oplocks: OK \
	\nChecking for kernel share modes: OK \
	\nChecking if can we convert from CP850 to UCS-2LE: OK \
	\nChecking if can we convert from UTF-8 to UCS-2LE: OK \
	\nChecking whether we can use Linux thread-specific credentials with 32-bit system calls: OK \
	\nChecking whether we can use Linux thread-specific credentials: OK \
	\nChecking whether setreuid is available: OK \
	\nChecking whether setresuid is available: OK \
	\nChecking whether seteuid is available: OK \
	\nChecking whether fcntl locking is available: OK \
	\nChecking whether the realpath function allows a NULL argument: OK \
	\nChecking whether POSIX capabilities are available: OK \
	\nChecking for the maximum value of the 'time_t' type: NO \
	\nChecking for ftruncate extend: OK \
	\nvfs_fileid checking for statfs() and struct statfs.f_fsid: OK \
	\ngetcwd takes a NULL argument: OK \
	\nChecking value of NSIG: \"65\" \
	\nChecking value of _NSIG: \"65\" \
	\nChecking value of SIGRTMAX: \"64\" \
	\nChecking value of SIGRTMIN: \"34\" \
	\n"
# 	\nChecking for the maximum value of the 'time_t' type: OK  <<-- check it, not for sure. It seems musl-libc don't have max time_t

# do the renaming trick
define Build/Prepare
	$(call Build/Prepare/Default)
	$(SED) 's#__OPENWRT_SAMBA_MULTICALL__#$(PKG_NAME)_multicall#g' $(PKG_BUILD_DIR)/source3/wscript_build
	$(CP) ./files/python2-config "$(STAGING_DIR)/usr/bin/python2-config"
endef

define Build/Configure
	(cd $(PKG_BUILD_DIR); \
		echo -e >cache.txt $(SAMBA43_CACHE) " \
			\nChecking uname release type: \"$(LINUX_VERSION)\" \
			\nChecking uname machine type: \"$(ARCH)\" \
			\nChecking uname version type: \"#1 Tue Nov 1 12:34:56 UTC 2013\" \
			\nChecking uname sysname type: \"Linux\" \
			\n" ; \
		$(CONFIGURE_VARS) \
		./buildtools/bin/waf configure -j$(shell nproc) \
			--cross-compile \
			--cross-answers=$(PKG_BUILD_DIR)/cache.txt \
			--hostcc="$(HOSTCC)" \
			$(SAMBA_CONFIGURE_ARGS) \
			--with-lockdir=/var/lock \
			--with-logfilebase=/var/log \
			--with-piddir=/var/run \
			--with-privatedir=/etc/samba \
			--enable-fhs \
			--without-dmapi \
			--disable-glusterfs \
			--disable-rpath \
			--disable-rpath-install \
			--disable-avahi \
			--disable-cups \
			--disable-iprint \
			--disable-gnutls \
			--with-relro \
			--with-sendfile-support \
			--without-acl-support \
			--without-ads \
			--without-ad-dc \
			--without-automount \
			--without-cluster-support \
			--without-dnsupdate \
			--without-fam \
			--without-gettext \
			--without-iconv \
			--without-ldap \
			--without-pam \
			--without-pie \
			--without-regedit \
			--without-systemd \
			--without-utmp \
			--without-winbind \
			--enable-auto-reconfigure \
			--nonshared-binary=$(PKG_NAME)_multicall \
			--with-static-modules=${SAMBA4_MODULES_STATIC} \
			--with-shared-modules=${SAMBA4_MODULES_SHARED} \
			--bundled-libraries='heimdal,!asn1_compile,!compile_et,!zlib,popt,talloc,pytalloc,pytalloc-util' \
			--without-quotas \
			--with-syslog \
			--nopyc \
			--nopyo \
	)
endef
# leftovers of samba 4.0: 			--with-shared-modules=pdb_tdbsam,pdb_wbc_sam,idmap_nss,nss_info_template,auth_wbc
# simple file server: --bundled-libraries=NONE --builtin-libraries=NONE
# --bundled-libraries=ALL ---> do not find libs from system, use integrated libs
# '--nonshared-binary' works like '--disable-shared-libs' in samba 4.0.26
# or use '--builtin-libraries' to specify which library to build into.
# --nonshared-binary=smbd/smbd,nmbd/nmbd,smbpasswd,smbcontrol,smbtree,smbstatus,smbcquotas,smbcacls,nmblookup,testparm,$(PKG_NAME)_multicall

# 'builtin-libraries'
# smbd_base samba-util tdb talloc param pdb
# seems don't need: krb5samba

#  Some important options:
#
#   --with-static-modules=STATIC_MODULES
#            Comma-separated list of names of modules to statically link in
#   --with-shared-modules=SHARED_MODULES
#            Comma-separated list of names of modules to build shared
#    --private-libraries=PRIVATE_LIBS
#            comma separated list of normally public libraries to build instead as private libraries. May include !LIBNAME to disable making a library private. Can be 'NONE' or 'ALL' [auto]
#    --builtin-libraries=BUILTIN_LIBRARIES
#            command separated list of libraries to build directly into binaries [NONE]
#    --nonshared-binary=NONSHARED_BINARIES
#            Disable use of shared libs for the listed binaries
#

# Disable some libs
# 	--without-libtdb \   bld.SAMBA_LIBRARY('tdb',
#	--without-libtalloc \
#	--without-libnetapi \  bld.SAMBA3_LIBRARY('netapi',
#	--without-libsmbclient \
#	--without-libsmbsharemodes \
#

# speed up
# $(shell nproc)
define Build/Compile
	(cd $(PKG_BUILD_DIR); \
		./buildtools/bin/waf build \
		--targets=$(PKG_NAME)_multicall \
		-j$(shell nproc) \
	)
endef

# Do NOT run 'make install' since we only need the multicall binary
define Build/Install
	$(RSTRIP) $(PKG_BUILD_DIR)/bin/default/source3/$(PKG_NAME)_multicall
endef

# XXX: Make sure the name of multicall binary matches name in 111-multicall-wscript-build.patch
define Package/samba4x-server/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/$(PKG_NAME).config $(1)/etc/config/samba4
	$(INSTALL_DIR) $(1)/etc/samba
	$(INSTALL_DATA) ./files/smb.conf.template $(1)/etc/samba
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/samba4
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/default/source3/$(PKG_NAME)_multicall $(1)/usr/sbin
	$(LN) $(PKG_NAME)_multicall $(1)/usr/sbin/smbd
	$(LN) $(PKG_NAME)_multicall $(1)/usr/sbin/nmbd
	$(LN) $(PKG_NAME)_multicall $(1)/usr/sbin/smbpasswd
	$(LN) $(PKG_NAME)_multicall $(1)/usr/sbin/pdbedit
	$(INSTALL_DIR) $(1)/usr/bin
	$(LN) ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbcontrol
	$(LN) ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbtree
	$(LN) ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbstatus
	$(LN) ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbcquotas
	$(LN) ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/smbcacls
	$(LN) ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/nmblookup
	$(LN) ../sbin/$(PKG_NAME)_multicall $(1)/usr/bin/testparm
endef

define Package/samba4x-server/conffiles
/etc/config/samba4
/etc/samba/smb.conf.template
/etc/samba/smbpasswd
endef


$(eval $(call BuildPackage,$(PKG_NAME)-server))
