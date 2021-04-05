# Based partially on (wongsyrone/hbl0307106015) versions
include $(TOPDIR)/rules.mk

PKG_NAME:=samba
PKG_VERSION:=4.11.6
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:= \
		https://ftp.gwdg.de/pub/samba/stable/ \
		https://ftp.heanet.ie/mirrors/ftp.samba.org/stable/ \
		https://ftp.riken.jp/net/samba/samba/stable/ \
		http://www.nic.funet.fi/index/samba/pub/samba/stable/ \
		http://samba.mirror.bit.nl/samba/ftp/stable/ \
		https://download.samba.org/pub/samba/stable/
PKG_HASH:=91438f4d7b71f673421435fa7f26b03b613f214139636ce50af35bc2ff09ef38

PKG_MAINTAINER:=Andy Walsh <andy.walsh44+github@gmail.com>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:samba:samba

# samba4=(asn1_compile,compile_et) rpcsvc-proto=(rpcgen)
HOST_BUILD_DEPENDS:=python3/host perl/host
PKG_BUILD_DEPENDS:=samba4/host libtasn1/host rpcsvc-proto/host

PKG_CONFIG_DEPENDS:= \
	CONFIG_SAMBA4_SERVER_NETBIOS \
	CONFIG_SAMBA4_SERVER_AVAHI \
	CONFIG_SAMBA4_SERVER_VFS \
	CONFIG_SAMBA4_SERVER_VFSX \
	CONFIG_SAMBA4_SERVER_AD_DC \
	CONFIG_PACKAGE_kmod-fs-btrfs \
	CONFIG_PACKAGE_kmod-fs-xfs

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/version.mk
include $(TOPDIR)/feeds/packages/lang/python/python3-host.mk
#include $(TOPDIR)/feeds/packages/lang/python/python-package.mk
include $(TOPDIR)/feeds/packages/lang/python/python3-package.mk

define Package/samba4/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Samba $(PKG_VERSION)
  URL:=https://www.samba.org/
endef

define Package/samba4/Default/description
  The Samba software suite is a collection of programs that implements the
  SMB/CIFS protocol for UNIX systems, allowing you to serve files and printers.

  Samba 4 implements up-to protocol version SMB v3.1.1 (Win10), supports mDNS via AVAHI and a AD-DC setup via krb5.
  NOTE: No cluster and printer support.
endef

define Package/samba4-libs
  $(call Package/samba4/Default)
  TITLE+= libs
  DEPENDS:= +libtirpc +libreadline +libpopt +libcap +zlib +libgnutls +libtasn1 +libuuid +libopenssl +USE_GLIBC:libpthread \
	+libpam \
	+SAMBA4_SERVER_VFS:attr \
	+SAMBA4_SERVER_VFSX:libaio \
	+SAMBA4_SERVER_AVAHI:libavahi-client \
	+SAMBA4_SERVER_AD_DC:python3-cryptodome +SAMBA4_SERVER_AD_DC:libopenldap +SAMBA4_SERVER_AD_DC:jansson +SAMBA4_SERVER_AD_DC:libarchive +SAMBA4_SERVER_AD_DC:acl +SAMBA4_SERVER_AD_DC:attr
endef

define Package/samba4-server
  $(call Package/samba4/Default)
  TITLE+= server
  DEPENDS:= +samba4-libs
  CONFLICTS:=samba36-server
endef

define Package/samba4-server/description
  installs: smbd (nmbd) smbpasswd pdbedit testparm (nmblookup) (smbcacls sharesec)
            (samba samba-tool ntlm_auth samba-gpupdate samba_dnsupdate samba_kcc samba_spnupdate samba_upgradedns samba_downgrade_db)

  This provides the basic fileserver service and is the minimum needed to serve file shares.
  HINT: https://fitzcarraldoblog.wordpress.com/2016/10/17/a-correct-method-of-configuring-samba-for-browsing-smb-shares-in-a-home-network/
endef

define Package/samba4-server/config
  select PACKAGE_wsdd2
  source "$(SOURCE)/Config.in"
endef

define Package/samba4-client
  $(call Package/samba4/Default)
  TITLE+= client
  DEPENDS:= +samba4-libs
endef

define Package/samba4-client/description
  installs: cifsdd smbclient smbget

  The smbclient program implements a simple ftp-like client for accessing SMB shares
endef

define Package/samba4-admin
  $(call Package/samba4/Default)
  TITLE+= admin tools
  DEPENDS:= +samba4-libs
endef

define Package/samba4-admin/description
  installs: net smbcontrol profiles rpcclient dbwrap_tool eventlogadm
            ldbadd ldbdel ldbedit ldbmodify ldbrename ldbsearch
            tdbbackup tdbdump tdbrestore tdbtool

  Administration tools collection
endef

define Package/samba4-utils
  $(call Package/samba4/Default)
  TITLE+= utils
  DEPENDS:= +samba4-libs
endef

define Package/samba4-utils/description
  installs: smbstatus smbtree mvxattr smbtar smbcquotas

  Utilities collection
endef

TARGET_CFLAGS += $(FPIC) -ffunction-sections -fdata-sections -I$(STAGING_DIR)/usr/include/tirpc
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed
# dont mess with sambas private rpath!
RSTRIP:=:

CONFIGURE_VARS += \
	CPP="$(TARGET_CROSS)cpp"

CONFIGURE_CMD = ./buildtools/bin/waf
HOST_CONFIGURE_CMD = ./buildtools/bin/waf

# Strip options that WAF configure script does not recognize
CONFIGURE_ARGS:=$(filter-out	\
	--target=%			\
	--host=%			\
	--build=%			\
	--program-prefix=%	\
	--program-suffix=% 	\
	--disable-nls		\
	--disable-ipv6		\
	, $(CONFIGURE_ARGS))

HOST_CONFIGURE_ARGS:=$(filter-out	\
	--target=%			\
	--host=%			\
	--build=%			\
	--program-prefix=%	\
	--program-suffix=% 	\
	--disable-nls		\
	--disable-ipv6		\
	, $(HOST_CONFIGURE_ARGS))

# Waf needs the "configure" argument
CONFIGURE_ARGS:=configure $(CONFIGURE_ARGS)
HOST_CONFIGURE_ARGS:=configure $(HOST_CONFIGURE_ARGS)

CONFIGURE_ARGS += \
		--hostcc="$(HOSTCC)" \
		--cross-compile \
		--cross-answers=cross-answers.txt \
		--disable-cups \
		--disable-iprint \
		--disable-cephfs \
		--disable-fault-handling \
		--disable-glusterfs \
		--enable-fhs \
		--without-automount \
		--without-iconv \
		--without-lttng \
		--without-ntvfs-fileserver \
		--without-pam \
		--without-systemd \
		--without-utmp \
		--without-dmapi \
		--without-fam \
		--without-gettext \
		--without-regedit \
		--without-gpgme

HOST_CONFIGURE_ARGS += \
		--hostcc="$(HOSTCC)" \
		--disable-cups \
		--disable-iprint \
		--disable-cephfs \
		--disable-fault-handling \
		--disable-glusterfs \
		--disable-rpath \
		--disable-rpath-install \
		--disable-rpath-private-install \
		--enable-fhs \
		--without-automount \
		--without-iconv \
		--without-lttng \
		--without-ntvfs-fileserver \
		--without-pam \
		--without-systemd \
		--without-utmp \
		--without-dmapi \
		--without-fam \
		--without-gettext \
		--without-regedit \
		--without-gpgme

HOST_CONFIGURE_ARGS += --disable-avahi --without-quotas --without-acl-support --without-winbind \
	--without-ad-dc --without-json --without-libarchive --disable-python --nopyc --nopyo \
	--without-dnsupdate --without-ads --without-ldap --without-ldb-lmdb
HOST_CONFIGURE_VARS += python_LDFLAGS="" python_LIBDIR=""

# Optional AES-NI support - https://lists.samba.org/archive/samba-technical/2017-September/122738.html
# Support for Nettle wasn't comitted
ifdef CONFIG_TARGET_x86_64
	CONFIGURE_ARGS += --accel-aes=intelaesni
else
	CONFIGURE_ARGS += --accel-aes=none
endif

CONFIGURE_ARGS += \
		--with-lockdir=/var/lock \
		--with-logfilebase=/var/log \
		--with-piddir=/var/run \
		--with-privatedir=/etc/samba

# features
ifeq ($(CONFIG_SAMBA4_SERVER_VFS),y)
	CONFIGURE_ARGS += --with-quotas
else
	CONFIGURE_ARGS += --without-quotas
endif
ifeq ($(CONFIG_SAMBA4_SERVER_AVAHI),y)
	CONFIGURE_ARGS += --enable-avahi
else
	CONFIGURE_ARGS += --disable-avahi
endif

# NOTE: We need host python-bin, but target python-config here!
CONFIGURE_VARS += python_LDFLAGS="" python_LIBDIR=""
ifeq ($(CONFIG_SAMBA4_SERVER_AD_DC),y)
	CONFIGURE_ARGS += --without-winbind --without-ldb-lmdb --with-acl-support
	CONFIGURE_VARS += \
		PYTHON="$(HOST_PYTHON3_BIN)" \
		PYTHON_CONFIG="$(STAGING_DIR)/host/bin/$(PYTHON3)-config"
else
	CONFIGURE_ARGS += --without-winbind --without-ads --without-ldap --without-ldb-lmdb --without-ad-dc \
		--without-json --without-libarchive --disable-python --nopyc --nopyo --without-dnsupdate --without-acl-support
endif

SAMBA4_PDB_MODULES :=pdb_smbpasswd,pdb_tdbsam,
SAMBA4_AUTH_MODULES :=auth_builtin,auth_sam,auth_unix,auth_script,
SAMBA4_VFS_MODULES :=vfs_default,
SAMBA4_VFS_MODULES_SHARED :=
ifeq ($(CONFIG_SAMBA4_SERVER_VFS),y)
	SAMBA4_VFS_MODULES_SHARED :=$(SAMBA4_VFS_MODULES_SHARED)vfs_fruit,vfs_shadow_copy2,vfs_recycle,vfs_fake_perms,vfs_readonly,vfs_cap,vfs_offline,vfs_crossrename,vfs_catia,vfs_streams_xattr,vfs_xattr_tdb,vfs_default_quota,
ifeq ($(CONFIG_PACKAGE_kmod-fs-btrfs),y)
	SAMBA4_VFS_MODULES_SHARED :=$(SAMBA4_VFS_MODULES_SHARED)vfs_btrfs,
endif
endif
ifeq ($(CONFIG_SAMBA4_SERVER_VFSX),y)
	SAMBA4_VFS_MODULES_SHARED :=$(SAMBA4_VFS_MODULES_SHARED)vfs_virusfilter,vfs_shell_snap,vfs_commit,vfs_worm,vfs_aio_fork,vfs_aio_pthread,vfs_netatalk,vfs_dirsort,vfs_fileid,
ifeq ($(CONFIG_PACKAGE_kmod-fs-xfs),y)
	SAMBA4_VFS_MODULES_SHARED :=$(SAMBA4_VFS_MODULES_SHARED)vfs_linux_xfs_sgid,
endif
endif
ifeq ($(CONFIG_SAMBA4_SERVER_AD_DC),y)
	SAMBA4_PDB_MODULES :=$(SAMBA4_PDB_MODULES)pdb_samba_dsdb,pdb_ldapsam,
	SAMBA4_AUTH_MODULES :=$(SAMBA4_AUTH_MODULES)auth_samba4,
	SAMBA4_VFS_MODULES :=$(SAMBA4_VFS_MODULES)vfs_posixacl,
	SAMBA4_VFS_MODULES_SHARED :=$(SAMBA4_VFS_MODULES_SHARED)vfs_audit,vfs_extd_audit,vfs_full_audit,vfs_acl_xattr,vfs_acl_tdb,
	# vfs_zfsacl needs https://github.com/zfsonlinux/zfs/tree/master/include/sys/zfs_acl.h
	# vfs_nfs4acl_xattr needs https://github.com/notriddle/libdrpc/blob/master/rpc/xdr.h
endif

SAMBA4_MODULES :=${SAMBA4_VFS_MODULES}${SAMBA4_AUTH_MODULES}${SAMBA4_PDB_MODULES}
SAMBA4_MODULES_SHARDED :=${SAMBA4_VFS_MODULES_SHARED}

CONFIGURE_ARGS += \
		--with-static-modules=$(SAMBA4_MODULES)!DEFAULT,!FORCED \
		--with-shared-modules=$(SAMBA4_MODULES_SHARDED)!DEFAULT,!FORCED

HOST_CONFIGURE_ARGS += \
		--with-static-modules=!DEFAULT,!FORCED \
		--with-shared-modules=!DEFAULT,!FORCED

# lib bundling
PY_VER:=$(PYTHON3_VERSION_MAJOR)$(PYTHON3_VERSION_MINOR)
# NOTE: bundle + make private, we want to avoid version configuration (build, link) conflicts
HOST_CONFIGURE_ARGS += --builtin-libraries=replace --nonshared-binary=asn1_compile,compile_et
SYSTEM_BUNDLED_LIBS:=talloc,tevent,tevent-util,texpect,tdb,ldb,tdr,cmocka,replace,com_err
PYTHON_BUNDLED_LIBS:=pytalloc-util.cpython-$(PY_VER),pyldb-util.cpython-$(PY_VER)
# CONFIGURE_ARGS += --builtin-libraries=talloc,tevent,tevent-util,texpect,tdb,ldb,tdr,cmocka,com_err
ifeq ($(CONFIG_SAMBA4_SERVER_AD_DC),y)
	CONFIGURE_ARGS += --bundled-libraries=NONE,$(SYSTEM_BUNDLED_LIBS),$(PYTHON_BUNDLED_LIBS)
else
	CONFIGURE_ARGS += --bundled-libraries=NONE,$(SYSTEM_BUNDLED_LIBS)
endif
CONFIGURE_ARGS += --private-libraries=$(SYSTEM_BUNDLED_LIBS)

export COMPILE_ET=$(STAGING_DIR_HOSTPKG)/bin/compile_et_samba
export ASN1_COMPILE=$(STAGING_DIR_HOSTPKG)/bin/asn1_compile_samba

# we dont need GnuTLS for the host helpers
define Host/Prepare
	$(call Host/Prepare/Default)
	$(SED) 's,mandatory=True,mandatory=False,g' $(HOST_BUILD_DIR)/wscript_configure_system_gnutls
endef
define Host/Compile
	(cd $(HOST_BUILD_DIR); \
		./buildtools/bin/waf build \
		--targets=asn1_compile,compile_et \
	)
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOSTPKG)/bin/
	# add host tools suffix, prevent conflicts with krb5
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bin/asn1_compile $(STAGING_DIR_HOSTPKG)/bin/asn1_compile_samba
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bin/compile_et $(STAGING_DIR_HOSTPKG)/bin/compile_et_samba
endef

define Build/Prepare
	$(Build/Prepare/Default)
ifeq ($(CONFIG_SAMBA4_SERVER_AD_DC),)
	# un-bundle dnspython
	$(SED) '/"dns.resolver":/d' $(PKG_BUILD_DIR)/third_party/wscript
	# unbundle iso8601
	$(SED) '/"iso8601":/d' $(PKG_BUILD_DIR)/third_party/wscript
endif
endef

define Build/Configure
	$(CP) ./waf-cross-answers/$(ARCH).txt $(PKG_BUILD_DIR)/cross-answers.txt
	echo 'Checking uname sysname type: "$(VERSION_DIST)"' >> $(PKG_BUILD_DIR)/cross-answers.txt
	echo 'Checking uname machine type: "$(ARCH)"' >> $(PKG_BUILD_DIR)/cross-answers.txt
	echo 'Checking uname release type: "$(LINUX_VERSION)"' >> $(PKG_BUILD_DIR)/cross-answers.txt
	echo 'Checking uname version type: "$(VERSION_DIST) Linux-$(LINUX_VERSION) $(shell date +%Y-%m-%d)"' >> $(PKG_BUILD_DIR)/cross-answers.txt
	# NOTE: special answers for freeBSD/CircleCI
	echo 'Checking whether POSIX capabilities are available: OK' >> $(PKG_BUILD_DIR)/cross-answers.txt
	echo 'checking for clnt_create(): OK' >> $(PKG_BUILD_DIR)/cross-answers.txt
	$(call Build/Configure/Default)
endef

# Build via "waf install", avoid the make wrapper. (Samba logic is 'waf install' = build + install)
define Build/Compile
	(cd $(PKG_BUILD_DIR); \
		./buildtools/bin/waf install \
		--jobs=$(shell nproc) \
		--destdir="$(PKG_INSTALL_DIR)" \
	)
endef

# No default install see above
define Build/Install
endef

define Package/samba4-libs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	# rpath-install
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/samba $(1)/usr/lib/
endef

define Package/samba4-client/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{cifsdd,smbclient,smbget} $(1)/usr/bin/
endef

define Package/samba4-admin/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{net,smbcontrol,profiles,rpcclient,dbwrap_tool} $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{ldbadd,ldbdel,ldbedit,ldbmodify,ldbrename,ldbsearch} $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{tdbbackup,tdbdump,tdbrestore,tdbtool} $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/eventlogadm $(1)/usr/sbin/
endef

define Package/samba4-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{smbstatus,smbtree,mvxattr,smbtar} $(1)/usr/bin/
ifeq ($(CONFIG_SAMBA4_SERVER_VFS),y)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/smbcquotas $(1)/usr/bin/
endif
endef

define Package/samba4-server/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{smbpasswd,pdbedit,testparm} $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/smbd $(1)/usr/sbin/
ifeq ($(CONFIG_SAMBA4_SERVER_NETBIOS),y)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/nmbd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/nmblookup $(1)/usr/bin/
endif
ifeq ($(CONFIG_SAMBA4_SERVER_AD_DC),y)
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/$(PYTHON3) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/share/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/samba $(1)/usr/share/
	# fix wrong hardcoded python3 location
	$(SED) '1s,^#!/.*python3.*,#!/usr/bin/python3,' $(PKG_INSTALL_DIR)/usr/bin/samba-tool
	$(SED) '1s,^#!/.*python3.*,#!/usr/bin/python3,' $(PKG_INSTALL_DIR)/usr/sbin/{samba-gpupdate,samba_dnsupdate,samba_kcc,samba_spnupdate,samba_upgradedns,samba_downgrade_db}
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{samba-tool,ntlm_auth,oLschema2ldif} $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/{sharesec,smbcacls} $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/{samba,samba-gpupdate,samba_dnsupdate,samba_kcc,samba_spnupdate,samba_upgradedns,samba_downgrade_db} $(1)/usr/sbin/
endif
	$(INSTALL_DIR) $(1)/etc/config $(1)/etc/samba $(1)/etc/init.d
	$(INSTALL_CONF) ./files/samba.config $(1)/etc/config/samba4
	$(INSTALL_DATA) ./files/smb.conf.template $(1)/etc/samba
	$(INSTALL_BIN) ./files/samba.init $(1)/etc/init.d/samba4
endef

define Package/samba4-server/conffiles
/etc/config/samba4
/etc/samba/smb.conf.template
/etc/samba/smb.conf
/etc/samba/smbpasswd
/etc/samba/secrets.tdb
/etc/samba/passdb.tdb
/etc/samba/lmhosts
/etc/nsswitch.conf
/etc/krb5.conf
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,samba4-libs))
$(eval $(call BuildPackage,samba4-server))
$(eval $(call BuildPackage,samba4-client))
$(eval $(call BuildPackage,samba4-admin))
$(eval $(call BuildPackage,samba4-utils))