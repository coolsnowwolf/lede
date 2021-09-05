# Copyright 2020 by Christian Dreihsig and Steffen Möller
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=boinc
PKG_VERSION:=7.16.16
PKG_VERSION_SHORT:=$(shell echo $(PKG_VERSION)| cut -f1,2 -d.)
PKG_RELEASE:=1

PKG_SOURCE_DATE:=2020-02-25
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/BOINC/boinc/tar.gz/client_release/$(PKG_VERSION_SHORT)/$(PKG_VERSION)?
PKG_HASH:=0d5656a9f8ed1048936a5764270848b892d63f27bdb863d0ace447f1eaae6002
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-client_release-$(PKG_VERSION_SHORT)-$(PKG_VERSION)

PKG_MAINTAINER:=Christian Dreihsig <christian.dreihsig@t-online.de>, Steffen Moeller <moeller@debian.org>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:boinc_project:boinc

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=0
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/target.mk

define Package/boinc
  SECTION:=net
  CATEGORY:=Network
  TITLE:=BOINC client
  DEPENDS:=+curl +bzip2 +libstdcpp +libopenssl +zlib
  USERID:=boinc:boinc
  URL:=https://github.com/BOINC/boinc/
endef

define Package/boinc/description
  The Berkeley Open Infrastructure for Network Computing (BOINC) is a
  software platform for distributed computing: several initiatives of
  various scientific disciplines all compete for the idle time of
  desktop computers. The developers' web site at the University of
  Berkeley serves as a common portal to the otherwise independently run
  projects.
 
  This package provides the BOINC core client program that is
  required to participate in any project that uses BOINC to control what
  projects to join and to determine constraints for the computation
  like the percentage of CPU time. OpenWrt does not
  provide the graphical BOINC Manager, but you can connect to this
  machine from the BOINC Manager of your desktop computer.
endef

CONFIGURE_ARGS += \
	--disable-server --disable-manager --enable-client --enable-libraries \
	--disable-boinczip --enable-install-headers --enable-dynamic-client-linkage \
	--with-boinc-platform=$(REAL_GNU_TARGET_NAME) \
	--with-boinc-alt-platform=$(ARCH)-$(BOARD)-$(DEVICE_TYPE)-openwrt-$(TARGET_SUFFIX)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/boinc
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/boinc/* $(1)/usr/include/boinc/
	$(CP) $(PKG_BUILD_DIR)/*.h $(1)/usr/include/boinc/ # project_specific_defines.h, config.h, version.h, svn_version.h
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.a $(1)/usr/lib/
endef

define Package/boinc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.{la,so}* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/boinc-client.init $(1)/etc/init.d/boinc-client
	$(INSTALL_DIR) $(1)/opt/boinc
	$(INSTALL_DIR) $(1)/usr/share/boinc
	$(CP) ./files/global_prefs_override.xml $(1)/usr/share/boinc/global_prefs_override.xml
	$(CP) ./files/remote_hosts.cfg $(1)/usr/share/boinc/remote_hosts.cfg
endef

$(eval $(call BuildPackage,boinc))
