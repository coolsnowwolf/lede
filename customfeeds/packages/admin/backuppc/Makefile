# Copyright (C) 2018 OpenWrt
#
# Ideas used from the backuppc packaging in Debian GNU/Linux
#

include $(TOPDIR)/rules.mk

PKG_NAME:=backuppc
PKG_VERSION:=3.3.2
PKG_RELEASE:=3

PKG_SOURCE:=BackupPC-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/backuppc
PKG_HASH:=fbade2c8d8039297e826a75d2c39d5ac9a6f66e0c84c0cf8c4cef0bcf64d2152
PKG_BUILD_DIR:=$(BUILD_DIR)/BackupPC-$(PKG_VERSION)

PKG_MAINTAINER:=Carsten Wolff <carsten@wolffcarsten.de>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/backuppc
  SECTION:=admin
  CATEGORY:=Administration
  TITLE:=high-performance, enterprise-grade system for backing up PCs
  URL:=https://sourceforge.net/projects/backuppc/
  DEPENDS:=+perl +perl-www +perl-cgi +perlbase-digest +perlbase-compress +perlbase-archive +perlbase-data +perlbase-storable +perlbase-getopt +perl-file-rsyncp +openssh-client +tar +bzip2 +samba4-client +rsync +iputils-ping
endef

define Package/backuppc/description
  BackupPC is a disk based backup system featuring a clever pooling scheme and
  compression to minimize disk storage and disk I/O. It can use SMB, rsync or tar
  to access the clients without any additional client software and offers a
  powerful http/cgi user interface.
endef

define Build/Configure
	true
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) $(TARGET_LDFLAGS) -Wall -o $(PKG_BUILD_DIR)/BackupPC_Admin ./files/setuidwrapper.c
endef

define Package/backuppc/install
	$(INSTALL_DIR) $(strip $(1))/etc/init.d
	$(INSTALL_BIN) ./files/backuppc.init $(strip $(1))/etc/init.d/backuppc
	cd $(PKG_BUILD_DIR) && ./configure.pl --batch --html-dir-url=/backuppc/ \
	    --html-dir=/www/backuppc/ --cgi-dir=/www/cgi-bin/ --no-fhs --uid-ignore \
	    --dest-dir=$(strip $(1)) --install-dir /usr/share/backuppc \
	    --hostname=XXXXXX --data-dir=/data/backuppc --log-dir=/data/backuppc/log \
	    --bin-path perl=/usr/bin/perl --bin-path tar=/bin/tar \
	    --bin-path smbclient=/usr/sbin/smbclient --bin-path nmblookup=/usr/sbin/nmblookup \
	    --bin-path rsync=/usr/bin/rsync --bin-path ping=/usr/bin/ping --bin-path df=/bin/df \
	    --bin-path ssh=/usr/bin/ssh --bin-path gzip=/bin/gzip \
	    --bin-path sendmail=/usr/sbin/sendmail --bin-path bzip2=/usr/bin/bzip2 \
	    --bin-path par2=
	$(INSTALL_DIR) $(strip $(1))/usr/share/backuppc/conf
	chmod 755 $(strip $(1))/data
	mv $(strip $(1))/data/backuppc/conf/config.pl $(strip $(1))/usr/share/backuppc/conf/config.pl
	patch --no-backup-if-mismatch $(strip $(1))/usr/share/backuppc/conf/config.pl ./files/fixup-config-pl.patch
	mv $(strip $(1))/www/cgi-bin/BackupPC_Admin $(strip $(1))/usr/share/backuppc/bin/BackupPC_Admin_real
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/BackupPC_Admin $(strip $(1))/www/cgi-bin/BackupPC_Admin
	$(INSTALL_DIR) $(strip $(1))/lib/upgrade/keep.d
	$(INSTALL_DATA) files/backuppc.upgrade  $(strip $(1))/lib/upgrade/keep.d/backuppc
endef

$(eval $(call BuildPackage,backuppc))
