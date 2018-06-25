include $(TOPDIR)/rules.mk

PKG_NAME:=samba
PKG_VERSION:=4.8.1
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://download.samba.org/pub/samba/stable/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=8ef7367507f16b7a5e2f6aed5bcdbd1143feca79aa2a07c9b21292b17d7f789d
PKG_LICENSE:=GPL-3.0

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=perl/host python/host qemu-userspace/host SAMBA4_SERVER_AD_DC:python-crypto

PKG_CONFIG_DEPENDS:= \
	CONFIG_SAMBA4_SERVER \
	CONFIG_SAMBA4_CLIENT \
	CONFIG_SAMBA4_NET \
	CONFIG_SAMBA4_SERVER_NETBIOS \
	CONFIG_SAMBA4_SERVER_AVAHI \
	CONFIG_SAMBA4_SERVER_VFS \
	CONFIG_SAMBA4_SERVER_VFSX \
	CONFIG_SAMBA4_SERVER_QUOTAS \
	CONFIG_SAMBA4_SERVER_ACL \
	CONFIG_SAMBA4_SERVER_AD_DC \
	CONFIG_SAMBA4_SERVER_WINBIND \
	CONFIG_PACKAGE_kmod-fs-btrfs \
	CONFIG_PACKAGE_kmod-fs-xfs

include $(INCLUDE_DIR)/package.mk

define Package/samba4
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Samba $(PKG_VERSION) SMB/CIFS (server/client/tools)
  URL:=https://www.samba.org/
  MAINTAINER:=Andy Walsh <andy.walsh44+github@gmail.com>
  DEPENDS:=+zlib +libpthread +libpopt +libcap +libtirpc +libnettle +krb5-libs \
	+SAMBA4_SERVER_ACL:acl \
	+SAMBA4_SERVER_AVAHI:libavahi-client +SAMBA4_SERVER_AVAHI:avahi-dbus-daemon \
	+SAMBA4_SERVER_AD_DC:python-base +SAMBA4_SERVER_AD_DC:krb5-server +SAMBA4_SERVER_AD_DC:libopenssl +SAMBA4_SERVER_AD_DC:libgnutls +SAMBA4_SERVER_AD_DC:libopenldap
  # buildroot bug? Cant add target deps via '+SAMBA4_SERVER_AD_DC:python-crypto' (as work-around we select via config.in)
endef

define Package/samba4/config
	source "$(SOURCE)/Config.in" 
endef

define Package/samba4/description
  The Samba software suite is a collection of programs that implements the
  SMB/CIFS protocol for UNIX systems, allowing you to serve files and printers.

  Samba4 implements to the latest SMB v3.1.1 version, supports mDNS via AVAHI and a AD-DC setup via krb5.
  NOTE: Without Cluster and printer support.
endef

CONFIGURE_VARS += \
	CPP="$(TARGET_CROSS)cpp"

# WAF configure script does not recognize these options
CONFIGURE_ARGS:=$(filter-out	\
	--host=%		\
	--build=%		\
	--program-suffix=%	\
	--disable-nls		\
	--disable-ipv6		\
	, $(CONFIGURE_ARGS))

CONFIGURE_ARGS += \
		--hostcc="$(HOSTCC)" \
		--cross-compile \
		--cross-execute="qemu-$(ARCH) -L $(STAGING_DIR_ROOT)" \
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
		--without-gpgme \
		--enable-auto-reconfigure

CONFIGURE_ARGS += \
		--with-lockdir=/var/lock \
		--with-logfilebase=/var/log \
		--with-piddir=/var/run \
		--with-privatedir=/etc/samba

ifeq ($(CONFIG_SAMBA4_SERVER_AVAHI),y)
	CONFIGURE_ARGS += --enable-avahi
else
	CONFIGURE_ARGS += --disable-avahi
endif
ifeq ($(CONFIG_SAMBA4_SERVER_QUOTAS),y)
	CONFIGURE_ARGS += --with-quotas
else
	CONFIGURE_ARGS += --without-quotas
endif
ifeq ($(CONFIG_SAMBA4_SERVER_ACL),y)
	CONFIGURE_ARGS += --with-acl-support
else
	CONFIGURE_ARGS += --without-acl-support
endif
ifeq ($(CONFIG_SAMBA4_SERVER_AD_DC),y)
	CONFIGURE_ARGS += --enable-gnutls --with-dnsupdate --with-ads --with-ldap
	TARGET_CFLAGS := -I$(STAGING_DIR)/usr/include/python2.7 $(TARGET_CFLAGS)
else
	CONFIGURE_ARGS += --without-ad-dc --disable-python --nopyc --nopyo --disable-gnutls --without-dnsupdate --without-ads --without-ldap
	CONFIGURE_VARS += \
		python_LDFLAGS="" \
		python_LIBDIR=""
endif
ifeq ($(CONFIG_SAMBA4_SERVER_WINBIND),y)
	CONFIGURE_ARGS += --with-winbind
else
	CONFIGURE_ARGS += --without-winbind
endif
# lib bundling
ifeq ($(CONFIG_SAMBA4_SERVER_AD_DC),y)
	CONFIGURE_ARGS += \
			--bundled-libraries=talloc,tevent,tdb,ldb,cmocka,pytalloc-util,pyldb-util,NONE \
			--builtin-libraries=talloc,tevent,tdb,ldb,cmocka
else
	CONFIGURE_ARGS += \
			--bundled-libraries=talloc,tevent,tdb,ldb,cmocka,NONE \
			--builtin-libraries=talloc,tevent,tdb,ldb,cmocka
endif

CONFIGURE_ARGS += \
		--with-system-mitkrb5 "$(STAGING_DIR)/usr" \
		--with-system-mitkdc=/usr/sbin/krb5kdc

		## embedded-heimdal
		# --bundled-libraries=talloc,tevent,tdb,ldb,com_err,cmocka,roken,wind,hx509,asn1,heimbase,hcrypto,krb5,gssapi,heimntlm,hdb,kdc,NONE

# idmap_tdb,idmap_passdb,idmap_nss (idmap_tdb2,idmap_script) # shared_only (idmap_ad idmap_rfc2307 idmap_autorid idmap_rid idmap_hash)
SAMBA4_IDMAP_MODULES :=
SAMBA4_IDMAP_MODULES_SHARED :=
# pdb_smbpasswd,pdb_tdbsam (pdb_samba_dsdb) #(pdb_ads,pdb_sam,pdb_samba4,pdb_wbc_sam)
SAMBA4_PDB_MODULES :=pdb_smbpasswd,pdb_tdbsam,
# auth_builtin,auth_sam,auth_winbind,auth_unix (auth_script) #(auth_samba4)
SAMBA4_AUTH_MODULES :=auth_builtin,auth_sam,auth_unix,auth_script,
# vfs_default (vfs_recycle,vfs_audit,vfs_extd_audit,vfs_full_audit,vfs_netatalk,vfs_fake_perms,vfs_default_quota,vfs_readonly,vfs_cap,vfs_expand_msdfs,vfs_posix_eadb,vfs_dfs_samba4
# vfs_shadow_copy,vfs_shadow_copy2,vfs_readahead,vfs_xattr_tdb,vfs_streams_xattr,vfs_streams_depot,vfs_acl_xattr,vfs_acl_tdb,vfs_posixacl,vfs_preopen,vfs_catia,vfs_media_harmony
# vfs_unityed_media,vfs_fruit,vfs_shell_snap,vfs_commit,vfs_worm,vfs_crossrename,vfs_linux_xfs_sgid,vfs_time_audit,vfs_offline,vfs_syncops,vfs_dirsort,vfs_fileid,vfs_aio_fork,vfs_aio_pthread,vfs_btrfs
# vfs_snapper (needs libdbus)
# vfs_virusfilter
SAMBA4_VFS_MODULES :=vfs_default,
ifeq ($(CONFIG_SAMBA4_SERVER_VFS),y)
	SAMBA4_VFS_MODULES :=$(SAMBA4_VFS_MODULES)vfs_fruit,vfs_shadow_copy2,vfs_recycle,vfs_fake_perms,vfs_readonly,vfs_cap,vfs_offline,vfs_crossrename,
ifeq ($(CONFIG_PACKAGE_kmod-fs-btrfs),y)
	SAMBA4_VFS_MODULES :=$(SAMBA4_VFS_MODULES)vfs_btrfs,
endif
endif
ifeq ($(CONFIG_SAMBA4_SERVER_VFSX),y)
	SAMBA4_VFS_MODULES :=$(SAMBA4_VFS_MODULES)vfs_virusfilter,vfs_shell_snap,vfs_commit,vfs_worm,vfs_xattr_tdb,vfs_streams_xattr,vfs_aio_fork,vfs_aio_pthread,vfs_netatalk,vfs_dirsort,vfs_fileid,
ifeq ($(CONFIG_PACKAGE_kmod-fs-xfs),y)
	SAMBA4_VFS_MODULES :=$(SAMBA4_VFS_MODULES)vfs_linux_xfs_sgid,
endif
endif
ifeq ($(CONFIG_SAMBA4_SERVER_QUOTAS),y)
	SAMBA4_VFS_MODULES :=$(SAMBA4_VFS_MODULES)vfs_default_quota,
endif
ifeq ($(CONFIG_SAMBA4_SERVER_ACL),y)
	SAMBA4_VFS_MODULES :=$(SAMBA4_VFS_MODULES)vfs_posixacl,vfs_acl_xattr,vfs_acl_tdb,
	# vfs_zfsacl 		needs https://github.com/zfsonlinux/zfs/tree/master/include/sys/zfs_acl.h
	# vfs_nfs4acl_xattr needs https://github.com/notriddle/libdrpc/blob/master/rpc/xdr.h
endif
ifeq ($(CONFIG_SAMBA4_SERVER_AD_DC),y)
	SAMBA4_PDB_MODULES :=$(SAMBA4_PDB_MODULES)pdb_samba_dsdb,
	SAMBA4_AUTH_MODULES :=$(SAMBA4_AUTH_MODULES)auth_samba4,
endif
ifeq ($(CONFIG_SAMBA4_SERVER_WINBIND),y)
	SAMBA4_IDMAP_MODULES :=$(SAMBA4_IDMAP_MODULES)idmap_passdb,idmap_nss,idmap_tdb,idmap_tdb2,idmap_script,nss_info_template,
	SAMBA4_IDMAP_MODULES_SHARED :=$(SAMBA4_IDMAP_MODULES_SHARED)idmap_autorid,idmap_rid,idmap_ad,idmap_rfc2307,
	# idmap_ad needs --with-ads
	# idmap_rfc2307 needs ldap headers
	SAMBA4_AUTH_MODULES :=$(SAMBA4_AUTH_MODULES)auth_winbind,
endif

SAMBA4_MODULES :=${SAMBA4_AUTH_MODULES}${SAMBA4_PDB_MODULES}${SAMBA4_IDMAP_MODULES}${SAMBA4_VFS_MODULES}
SAMBA4_MODULES_SHARDED :=${SAMBA4_IDMAP_MODULES_SHARED}

CONFIGURE_ARGS += \
		--with-static-modules=$(SAMBA4_MODULES)!DEFAULT,!FORCED \
		--with-shared-modules=$(SAMBA4_MODULES_SHARDED)!DEFAULT,!FORCED

TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections

# setup build/install targets
BUILD_TARGETS :=smbstatus,smbtree
ifeq ($(CONFIG_SAMBA4_SERVER),y)
	BUILD_TARGETS :=$(BUILD_TARGETS),smbd/smbd,smbpasswd,pdbedit,testparm
endif
ifeq ($(CONFIG_SAMBA4_SERVER_QUOTAS),y)
	BUILD_TARGETS :=$(BUILD_TARGETS),smbcquotas
endif
ifeq ($(CONFIG_SAMBA4_SERVER_ACL),y)
	BUILD_TARGETS :=$(BUILD_TARGETS),mvxattr,sharesec,smbcacls
endif
ifeq ($(CONFIG_SAMBA4_SERVER_NETBIOS),y)
	BUILD_TARGETS :=$(BUILD_TARGETS),nmbd,nmblookup
endif
ifeq ($(CONFIG_SAMBA4_SERVER_AD_DC),y)
	BUILD_TARGETS :=$(BUILD_TARGETS),samba,nsstest,ntlm_auth
endif
ifeq ($(CONFIG_SAMBA4_SERVER_WINBIND),y)
	BUILD_TARGETS :=$(BUILD_TARGETS),winbindd,wbinfo,winbind_krb5_locator
endif
ifeq ($(CONFIG_SAMBA4_CLIENT),y)
	BUILD_TARGETS :=$(BUILD_TARGETS),client/smbclient
endif
ifeq ($(CONFIG_SAMBA4_NET),y)
	BUILD_TARGETS :=$(BUILD_TARGETS),net,smbget,smbcontrol,profiles,rpcclient
endif

# BUG: make install $(BUILD_TARGETS) is not supported, by the buggy waf/make wrapper atm and will instead trigger a full recompile (without -j4)!
# set targets to build/install (is used via makefile waf wrapper) NOTE: shared modules need to-be added to the targets list explicitly!
MAKE_VARS += WAF_TARGETS=--targets=$(SAMBA4_MODULES)$(SAMBA4_MODULES_SHARDED)$(BUILD_TARGETS)

ifeq ($(CONFIG_SAMBA4_SERVER_AD_DC),)
define Build/Prepare
	$(Build/Prepare/Default)
	# un-bundle dnspython
	$(SED) '/"dns.resolver":/d' $(PKG_BUILD_DIR)/third_party/wscript
	# unbundle iso8601
	$(SED) '/"iso8601":/d' $(PKG_BUILD_DIR)/third_party/wscript
endef
endif

define Package/samba4/install
	# install libs
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/samba/*.so* $(1)/usr/lib
	# install shared modules
	$(INSTALL_DIR) $(1)/usr/lib/samba
	if [ -d $(PKG_INSTALL_DIR)/usr/lib/samba/idmap ]; then \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/samba/idmap $(1)/usr/lib/samba/; \
	fi
	if [ -d $(PKG_INSTALL_DIR)/usr/lib/samba/auth ]; then \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/samba/auth $(1)/usr/lib/samba/; \
	fi
	if [ -d $(PKG_INSTALL_DIR)/usr/lib/samba/vfs ]; then \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/samba/vfs $(1)/usr/lib/samba/; \
	fi
	if [ -d $(PKG_INSTALL_DIR)/usr/lib/samba/pdb ]; then \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/samba/pdb $(1)/usr/lib/samba/; \
	fi
ifeq ($(CONFIG_SAMBA4_SERVER_AD_DC),y)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/python2.7 $(1)/usr/lib/
endif
	# install build targets
	$(INSTALL_DIR) $(1)/usr
	if [ -d $(PKG_INSTALL_DIR)/usr/bin ]; then \
		$(CP) $(PKG_INSTALL_DIR)/usr/bin $(1)/usr/; \
	fi
	if [ -d $(PKG_INSTALL_DIR)/usr/sbin ]; then \
		$(CP) $(PKG_INSTALL_DIR)/usr/sbin $(1)/usr/; \
	fi	
	# install init, config
ifeq ($(CONFIG_SAMBA4_SERVER),y)
	$(INSTALL_DIR) $(1)/etc/config $(1)/etc/samba $(1)/etc/init.d
	$(INSTALL_DATA) ./files/samba.config $(1)/etc/config/samba4
	$(INSTALL_DATA) ./files/smb.conf.template $(1)/etc/samba
	$(INSTALL_BIN) ./files/samba.init $(1)/etc/init.d/samba4
endif
endef

define Package/samba4/conffiles
/etc/config/samba4
/etc/samba/smb.conf.template
/etc/samba/smb.conf
/etc/samba/smbpasswd
/etc/samba/secrets.tdb
/etc/samba/passdb.tdb
/etc/samba/idmap.ldb
/etc/samba/lmhosts
/etc/nsswitch.conf
endef

$(eval $(call BuildPackage,samba4))
