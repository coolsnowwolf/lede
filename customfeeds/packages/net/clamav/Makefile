#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=clamav
PKG_VERSION:=0.103.1
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.clamav.net/downloads/production/
PKG_HASH:=7308c47b89b268af3b9f36140528927a49ff3e633a9c9c0aac2712d81056e257

PKG_MAINTAINER:=Marko Ratkaj <marko.ratkaj@sartura.hr> \
		Lucian Cristian <lucian.cristian@gmail.com>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING*
PKG_CPE_ID:=cpe:/a:clamav:clamav

PKG_BUILD_DEPENDS:=ncurses

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/clamav/Default
  SECTION:=net
  DEPENDS:=+libstdcpp +libpthread +zlib +libbz2 +libxml2 +libcurl +libjson-c +libmilter-sendmail +libopenssl +libltdl +libpcre2 $(ICONV_DEPENDS)
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=ClamAV
  URL:=https://www.clamav.net/
endef

define Package/clamav
    $(call Package/clamav/Default)
endef

define Package/freshclam
    $(call Package/clamav/Default)
    DEPENDS+= +clamav
    TITLE+=database updater
endef

define Package/clamav/description
 ClamAV is an open source antivirus engine for detecting trojans,
 viruses, malware & other malicious threats.
endef

define Package/freshclam/description
 Database updater for ClamAV
endef

CMAKE_OPTIONS += \
	-DENABLE_MILTER=ON \
	-DHAVE_ATTRIB_PACKED=ON \
	-DHAVE_ATTRIB_ALIGNED=ON \
	-Dtest_run_result=ON \
	-Dtest_run_result__TRYRUN_OUTPUT=ON \
	-DCLAMAV_USER=nobody \
	-DCLAMAV_GROUP=nogroup \
	-DMMAP_FOR_CROSSCOMPILING=ON \
	-DENABLE_CLAMONACC=ON \
	-DENABLE_DOCS=OFF \
	-DENABLE_DOXYGEN=OFF \
	-DENABLE_EXAMPLES=OFF \
	-DENABLE_UNRAR=OFF \
	-DENABLE_SYSTEMD=OFF \
	-DIconv_INCLUDE_DIR=$(ICONV_PREFIX)/include \
	-DIconv_LIBRARY=$(ICONV_PREFIX)/lib/libiconv.$(if $(ICONV_FULL),so,a) \
	-DHAVE_SYSTEM_LFS_FTS=O$(if $(CONFIG_USE_GLIBC),N,FF)

TARGET_CXXFLAGS += -ffunction-sections -fdata-sections -fno-rtti -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

define Package/clamav/conffiles
/etc/config/clamav
/etc/config/clamav-milter
endef

define Package/clamav/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/clamd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/clamav-milter $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/clambc $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/clamconf $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/clamdscan $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/clamscan $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/clamonacc $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/sigtool $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib*/lib*.so.* $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/clamav.config $(1)/etc/config/clamav
	$(INSTALL_CONF) ./files/clamav-milter.config $(1)/etc/config/clamav-milter

	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/clamav.init $(1)/etc/init.d/clamav
	$(INSTALL_BIN) ./files/clamav-milter.init $(1)/etc/init.d/clamav-milter

	$(INSTALL_DIR) $(1)/usr/share/clamav
	$(CP) ./files/bytecode.cvd $(1)/usr/share/clamav/
endef

define Package/freshclam/conffiles
/etc/config/freshclam
endef

define Package/freshclam/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/freshclam $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/freshclam.config $(1)/etc/config/freshclam

	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/freshclam.init $(1)/etc/init.d/freshclam
endef

$(eval $(call BuildPackage,clamav))
$(eval $(call BuildPackage,freshclam))
