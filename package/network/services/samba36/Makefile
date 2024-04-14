#
# Copyright (C) 2007-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=samba
PKG_VERSION:=4.0.26
PKG_RELEASE:=9

PKG_SOURCE_URL:=https://ftp.samba.org/pub/samba/stable/
PKG_SOURCE:=samba-$(PKG_VERSION).tar.gz
PKG_HASH:=ce2441992c6d18950d752edb2d1274b3f7e056b2e2e8516fc42e408e4a25894a

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1

PKG_BUILD_DIR := $(BUILD_DIR)/samba-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

MAKE_PATH:=source3
CONFIGURE_PATH:=source3

PKG_BUILD_BIN:=$(PKG_BUILD_DIR)/$(MAKE_PATH)/bin

define Package/samba36-server
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Samba $(PKG_VERSION) SMB/CIFS server
  URL:=http://www.samba.org/
  DEPENDS:=+libaio +libpthread +zlib +libpopt
endef

define Package/samba36-server/config
	config PACKAGE_SAMBA_MAX_DEBUG_LEVEL
		int "Maximum level of compiled-in debug messages"
		depends on PACKAGE_samba36-server
		default -1

endef

define Package/samba36-server/description
 The Samba software suite is a collection of programs that implements the
 SMB protocol for UNIX systems, allowing you to serve files and printers to
 Windows, NT, OS/2 and DOS clients. This protocol is sometimes also referred
 to as the LanManager or Netbios protocol.
endef


TARGET_CFLAGS += -DMAX_DEBUG_LEVEL=$(CONFIG_PACKAGE_SAMBA_MAX_DEBUG_LEVEL) -D__location__=\\\"\\\" -ffunction-sections -fdata-sections

TARGET_LDFLAGS += -Wl,--gc-sections

CONFIGURE_VARS += \
	ac_cv_lib_attr_getxattr=no \
	ac_cv_search_getxattr=no \
	ac_cv_file__proc_sys_kernel_core_pattern=yes \
	libreplace_cv_HAVE_C99_VSNPRINTF=yes \
	libreplace_cv_HAVE_GETADDRINFO=yes \
	libreplace_cv_HAVE_IFACE_IFCONF=yes \
	libreplace_cv_HAVE_MREMAP=yes \
	libreplace_cv_HAVE_MMAP=yes \
	libreplace_cv_HAVE_OPEN_O_DIRECT=yes \
	libreplace_cv_REPLACE_INET_NTOA=no \
	LINUX_LFS_SUPPORT=yes \
	samba_cv_TIME_T_MAX=no \
	samba_cv_have_longlong=yes \
	samba_cv_CC_NEGATIVE_ENUM_VALUES=yes \
	samba_cv_HAVE_GETTIMEOFDAY_TZ=yes \
	samba_cv_HAVE_IFACE_IFCONF=yes \
	samba_cv_HAVE_SECURE_MKSTEMP=yes \
	samba_cv_HAVE_WRFILE_KEYTAB=no \
	samba_cv_USE_SETREUID=yes \
	samba_cv_USE_SETRESUID=yes \
	samba_cv_have_setreuid=yes \
	samba_cv_have_setresuid=yes \
	ac_cv_header_zlib_h=no \
	samba_cv_zlib_1_2_3=yes \
	ac_cv_type_long_long=yes \
	samba_cv_USE_SETEUID=yes \
	samba_cv_HAVE_BROKEN_GETGROUPS=no \
	samba_cv_HAVE_BROKEN_READDIR_NAME=no \
	samba_cv_HAVE_BROKEN_LINUX_SENDFILE=no \
	samba_cv_HAVE_SENDFILE=yes \
	samba_cv_HAVE_FCNTL_LOCK=yes \
	samba_cv_HAVE_KERNEL_OPLOCKS_LINUX=no \
	samba_cv_HAVE_KERNEL_SHARE_MODES=yes \
	samba_cv_SIZEOF_BLKCNT_T_8=yes
	
CONFIGURE_ARGS += \
	--exec-prefix=/ \
	--prefix=/ \
	--disable-avahi \
	--disable-cups \
	--disable-external-libtalloc \
	--disable-pie \
	--disable-relro \
	--disable-static \
	--disable-swat \
	--disable-shared-libs \
	--with-codepagedir=/etc/samba \
	--with-configdir=/etc/samba \
	--with-included-iniparser \
	--with-included-popt=no \
	--with-lockdir=/var/lock \
	--with-logfilebase=/var/log \
	--with-nmbdsocketdir=/var/nmbd \
	--with-piddir=/var/run \
	--with-privatedir=/etc/samba \
	--with-sendfile-support \
	--without-acl-support \
	--with-aio-support \
	--without-cluster-support \
	--without-ads \
	--with-krb5=no \
	--without-krb5 \
	--without-ldap \
	--without-pam \
	--without-winbind \
	--without-libtdb \
	--without-libtalloc \
	--without-libnetapi \
	--without-libsmbclient \
	--without-libsmbsharemodes \
	--without-libtevent \
	--without-libaddns \
	--enable-cups=no --enable-iprint=no \
	--enable-largefile \
	--disable-dmalloc \
	--disable-debug \
	--without-utmp \
	--without-quotas \
	--with-libiconv=$(ICONV_PREFIX) \
	--with-shared-modules=pdb_tdbsam,pdb_wbc_sam,idmap_nss,nss_info_template,auth_winbind,auth_wbc,auth_domain

MAKE_FLAGS += DYNEXP= PICFLAG= MODULES=

# special CONFIGURE_ARGS
ifneq ($(CONFIG_mipsel)$(CONFIG_arm)$(CONFIG_x86),)
#Little Endian
CONFIGURE_VARS += samba_cv_big_endian=no  samba_cv_little_endian=yes 
TARGET_CFLAGS += -O3 -fno-inline-functions -fno-inline-small-functions -fno-inline-functions-called-once
else
#Big Endian
CONFIGURE_VARS += samba_cv_big_endian=yes  samba_cv_little_endian=no 
endif



define Package/samba36-server/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/samba
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	
	$(INSTALL_DATA) ./files/samba.config $(1)/etc/config/samba	
	$(INSTALL_DATA) ./files/smb.conf.template $(1)/etc/samba
	
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/codepages/lowcase.dat $(1)/etc/samba
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/codepages/upcase.dat $(1)/etc/samba
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/codepages/valid.dat $(1)/etc/samba

	$(INSTALL_BIN) ./files/samba.init $(1)/etc/init.d/samba
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/uci_defaults_samba $(1)/etc/uci-defaults/99_uci_defaults_samba

	$(INSTALL_DIR)	$(1)/etc/hotplug.d/block
	$(INSTALL_DATA) ./files/smb.auto $(1)/etc/hotplug.d/block/20-smb
	
	$(INSTALL_BIN) $(PKG_BUILD_BIN)/samba_multicall $(1)/usr/sbin
	ln -sf samba_multicall $(1)/usr/sbin/smbd
	ln -sf samba_multicall $(1)/usr/sbin/nmbd
	ln -sf samba_multicall $(1)/usr/sbin/smbpasswd
endef


$(eval $(call BuildPackage,samba36-server))
