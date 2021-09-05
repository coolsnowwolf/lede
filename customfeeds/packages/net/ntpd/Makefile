#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ntp
PKG_VERSION:=4.2.8p15
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.eecis.udel.edu/~ntp/ntp_spool/ntp4/ntp-4.2/
PKG_HASH:=f65840deab68614d5d7ceb2d0bb9304ff70dcdedd09abb79754a87536b849c19

PKG_LICENSE:=Unique
PKG_LICENSE_FILES:=COPYRIGHT html/copyright.html
PKG_CPE_ID:=cpe:/a:ntp:ntp

PKG_FIXUP:=autoreconf
PKG_LIBTOOL_PATHS:=. sntp
PKG_CHECK_FORMAT_SECURITY:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ntpd/Default
  SUBMENU:=Time Synchronization
  SECTION:=net
  CATEGORY:=Network
  TITLE:=ISC ntp
  MAINTAINER:=Peter Wagner <tripolar@gmx.at>
  URL:=http://www.ntp.org/
  DEPENDS:=+libopenssl +libpthread +libcap
endef

define Package/ntpd/Default/description
 The ISC ntp suite is a collection of tools used to synchronize the
 system clock with remote NTP time servers and run/monitor local NTP
 servers.
endef

define Package/ntpd
$(call Package/ntpd/Default)
  DEPENDS+= +ntp-utils
  TITLE+= server
  USERID:=ntp=123:ntp=123
endef

define Package/ntpd/description
$(call Package/ntpd/Default/description)
 .
 This package contains the ntpd server.
endef

define Package/ntpdate
$(call Package/ntpd/Default)
  TITLE+=date
endef

define Package/ntpdate/description
$(call Package/ntpd/Default/description)
 .
 This package contains ntpdate.
endef

define Package/ntp-utils
$(call Package/ntpd/Default)
  TITLE+= utilities
  DEPENDS+= +libevent2-pthreads
endef

define Package/ntp-utils/description
$(call Package/ntpd/Default/description)
 .
 This package contains ntpdc, ntpq, ntptime and sntp.
endef

define Package/ntp-keygen
$(call Package/ntpd/Default)
  TITLE+=keygen
  DEPENDS+= +libevent2-core
endef

define Package/ntp-keygen/description
$(call Package/ntpd/Default/description)
 .
 This package contains the ntp-keygen.
endef

CONFIGURE_VARS += \
	ac_cv_header_md5_h=no \
	ac_cv_lib_rt_sched_setscheduler=no \
	ac_cv_header_dns_sd_h=no \
	hw_cv_func_snprintf_c99=yes \
	hw_cv_func_vsnprintf_c99=yes \

CONFIGURE_ARGS += \
	--disable-all-clocks \
	--disable-parse-clocks \
	--without-ntpsnmpd \
	--without-lineeditlibs \
	--enable-NMEA \
	--enable-LOCAL-CLOCK \
	--enable-SHM \
	--enable-ATOM \
	--enable-linuxcaps \
	--with-yielding-select=yes \
	--with-crypto \
	$(if $(CONFIG_PACKAGE_ntp-utils),--with-sntp,--without-sntp) \
	--with-openssl-incdir="$(STAGING_DIR)/usr/include" \
	--with-openssl-libdir="$(STAGING_DIR)/usr/lib"

define Package/ntpd/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ntpd/ntpd $(1)/sbin/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/ntpd.hotplug-helper $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ntpd.init $(1)/etc/init.d/ntpd
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/ntpd.hotplug $(1)/etc/hotplug.d/iface/20-ntpd
endef

define Package/ntpd/postinst
#!/bin/sh
[ -L "$${IPKG_INSTROOT}/usr/sbin/ntpd" ] && rm -f "$${IPKG_INSTROOT}/usr/sbin/ntpd"
exit 0
endef

define Package/ntpd/postrm
#!/bin/sh
/bin/busybox ntpd -h 2>&1 | grep -q BusyBox && ln -sf ../../bin/busybox /usr/sbin/ntpd
exit 0
endef

define Package/ntpdate/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ntpdate/ntpdate $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ntpdate.init $(1)/etc/init.d/ntpdate
endef

define Package/ntp-utils/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ntpdc/ntpdc $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ntpq/ntpq $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/util/ntptime $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sntp/sntp $(1)/usr/sbin/
endef

define Package/ntp-keygen/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/util/ntp-keygen $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,ntpd))
$(eval $(call BuildPackage,ntpdate))
$(eval $(call BuildPackage,ntp-utils))
$(eval $(call BuildPackage,ntp-keygen))
