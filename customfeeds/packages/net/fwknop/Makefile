#
# Copyright (C) 2011-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fwknop
PKG_VERSION:=2.6.10
PKG_RELEASE:=7

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://www.cipherdyne.org/fwknop/download
PKG_HASH:=f6c09bec97ed8e474a98ae14f9f53e1bcdda33393f20667b6af3fb6bb894ca77

PKG_MAINTAINER:=Jonathan Bennett <JBennett@incomsystems.biz>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:cipherdyne:fwknop

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/fwknop/Default
  TITLE:=FireWall KNock OPerator
  URL:=https://www.cipherdyne.org/fwknop/
endef

define Package/fwknop/Default/description
  Fwknop implements an authorization scheme known as Single Packet Authorization
  (SPA) for Linux systems running iptables.  This mechanism requires only a
  single encrypted and non-replayed packet to communicate various pieces of
  information including desired access through an iptables policy. The main
  application of this program is to use iptables in a default-drop stance to
  protect services such as SSH with an additional layer of security in order to
  make the exploitation of vulnerabilities (both 0-day and unpatched code) much
  more difficult.
endef

define Package/fwknopd
  $(call Package/fwknop/Default)
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE+= Daemon
  DEPENDS:=+iptables +libfko +!FWKNOPD_NFQ_CAPTURE:libpcap +FWKNOPD_NFQ_CAPTURE:iptables-mod-nfqueue \
	+FWKNOPD_NFQ_CAPTURE:libnetfilter-queue +FWKNOPD_NFQ_CAPTURE:libnfnetlink \
	+FWKNOP_GPG:gnupg +FWKNOP_GPG:libgpgme
endef

define Package/fwknopd/description
  $(call Package/fwknop/Default/description)
  This package contains the fwknop daemon.
endef

define Package/fwknopd/conffiles
/etc/fwknop/access.conf
/etc/fwknop/fwknopd.conf
/etc/config/fwknopd
endef

define Package/fwknopd/config
	source "$(SOURCE)/Config.in"
endef

define Package/fwknop
  $(call Package/fwknop/Default)
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE+= Client
  DEPENDS:=+libfko +FWKNOPD_GPG:gnupg +FWKNOPD_GPG:libgpgme
endef

define Package/fwknop/description
  $(call Package/fwknop/Default/description)
  This package contains the fwknop client.
endef

define Package/libfko
  $(call Package/fwknop/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=Firewall
  TITLE+= Library
  DEPENDS:=+FWKNOPD_GPG:gnupg +FWKNOPD_GPG:libgpgme
endef

define Package/libfko/description
  $(call Package/fwknop/Default/description)
  This package contains the libfko shared library.
endef


CONFIGURE_ARGS += \
	--$(if $(CONFIG_FWKNOPD_NFQ_CAPTURE),en,dis)able-nfq-capture \
	--with$(if $(CONFIG_FWKNOPD_GPG),,out)-gpgme \
	--with-iptables=/usr/sbin/iptables \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/fko.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfko.{a,la,so*} $(1)/usr/lib/
endef

define Package/fwknopd/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/fwknopd $(1)/etc/config/fwknopd
	$(INSTALL_DIR) $(1)/etc/fwknop
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/fwknop/{access,fwknopd}.conf \
		$(1)/etc/fwknop/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/fwknopd.init $(1)/etc/init.d/fwknopd
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/fwknopd $(1)/usr/sbin/
endef

define Package/fwknop/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/fwknop $(1)/usr/bin/
endef

define Package/libfko/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfko.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,fwknopd))
$(eval $(call BuildPackage,fwknop))
$(eval $(call BuildPackage,libfko))
