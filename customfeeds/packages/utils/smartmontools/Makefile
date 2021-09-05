#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=smartmontools
PKG_VERSION:=7.2
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/smartmontools
PKG_HASH:=5cd98a27e6393168bc6aaea070d9e1cd551b0f898c52f66b2ff2e5d274118cd6

PKG_MAINTAINER:=Maxim Storchak <m.storchak@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/smartmontools/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=S.M.A.R.T Monitoring
  URL:=https://www.smartmontools.org/
endef

define Package/smartmontools
  $(call Package/smartmontools/Default)
  DEPENDS:=+libstdcpp
  TITLE+= Tool
endef

define Package/smartmontools/description
  smartmontools contains utility programs (smartctl) to
  control/monitor storage systems using the Self-Monitoring, Analysis
  and Reporting Technology System (S.M.A.R.T.) built into most modern
  ATA and SCSI disks. It is derived from smartsuite.
endef

define Package/smartd
  $(call Package/smartmontools/Default)
  DEPENDS:=+libstdcpp
  TITLE+= Daemon
endef

define Package/smartd/description
  smartmontools contains utility programs (smartd) to
  control/monitor storage systems using the Self-Monitoring, Analysis
  and Reporting Technology System (S.M.A.R.T.) built into most modern
  ATA and SCSI disks. It is derived from smartsuite.
endef

define Package/smartmontools-drivedb
  $(call Package/smartmontools/Default)
  TITLE+= Drive database
  PKGARCH=all
endef

define Package/smartmontools-drivedb/description
  Database of known drives and USB bridges for smartctl and smartd.
endef

define Package/smartd-mail
  $(call Package/smartmontools/Default)
  TITLE+= Email notifications
  PKGARCH=all
  DEPENDS+= +smartd +nail +msmtp-mta
endef

define Package/smartd-mail/description
  A script for mailing about SMART issues
endef

TARGET_LDFLAGS += -flto

TARGET_CXXFLAGS +=-fno-rtti -flto
MAKE_FLAGS +=BUILD_INFO='"(localbuild)"'

CONFIGURE_ARGS += \
	--disable-fast-lebe \
	--without-gnupg \
	--without-libcap-ng \
	--without-libsystemd

#lower file size vs. CONFIGURE_ARGS
CONFIGURE_VARS += \
	with_cxx11_regex=yes \
	with_smartdplugindir=no \
	with_systemdenvfile=no \
	with_systemdsystemunitdir=no \
	with_update_smart_drivedb=no

define Package/smartmontools/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/smartctl $(1)/usr/sbin/
endef

define Package/smartd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/smartd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/smartd.conf $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/smartd.init $(1)/etc/init.d/smartd
endef

define Package/smartmontools-drivedb/install
	$(INSTALL_DIR) $(1)/usr/share/smartmontools/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/smartmontools/drivedb.h $(1)/usr/share/smartmontools/
endef

define Package/smartd-mail/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/etc/smartd_warning.sh $(1)/etc
endef

define Package/smartd/conffiles
/etc/smartd.conf
endef

$(eval $(call BuildPackage,smartmontools))
$(eval $(call BuildPackage,smartd))
$(eval $(call BuildPackage,smartmontools-drivedb))
$(eval $(call BuildPackage,smartd-mail))
