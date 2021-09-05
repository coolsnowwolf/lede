#
# Copyright (C) 2009 David Cooper <dave@kupesoft.com>
# Copyright (C) 2009-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=msmtp
PKG_VERSION:=1.8.15
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://marlam.de/msmtp/releases
PKG_HASH:=2265dc639ebf2edf3069fffe0a3bd76749f8b58f4001d5cdeae19873949099ce

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:marlam:msmtp

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/msmtp/Default
  SECTION:=mail
  CATEGORY:=Mail
  DEPENDS:=$(INTL_DEPENDS)
  TITLE:=Simple sendmail SMTP forwarding
  URL:=https://marlam.de/msmtp
endef

define Package/msmtp/Default/description
 msmtp is an SMTP client. In the default mode, it transmits a mail to
 an SMTP server (for example at a free mail provider) which does the
 delivery. To use this program with your mail user agent (MUA), create
 a configuration file with your mail account(s) and tell your MUA to
 call msmtp instead of /usr/sbin/sendmail.
endef

define Package/msmtp
$(call Package/msmtp/Default)
  DEPENDS+= +libgnutls +ca-bundle
  TITLE+= (with SSL support)
  VARIANT:=ssl
  DEFAULT_VARIANT:=1
endef

define Package/msmtp/conffiles
/etc/msmtprc
endef

define Package/msmtp/description
$(call Package/msmtp/Default/description)
 This package is built with SSL support.
endef

define Package/msmtp-nossl
$(call Package/msmtp/Default)
  TITLE+= (without SSL support)
  VARIANT:=nossl
  PROVIDES:=msmtp
endef

define Package/msmtp-nossl/description
$(call Package/msmtp/Default/description)
 This package is built without SSL support.
endef

define Package/msmtp-mta
$(call Package/msmtp/Default)
  TITLE+= (as MTA)
  DEPENDS+=+msmtp
  ALTERNATIVES:=\
	100:/usr/sbin/sendmail:/usr/bin/msmtp \
	100:/usr/lib/sendmail:/usr/bin/msmtp
endef

define Package/msmtp-mta/description
$(call Package/msmtp/Default/description)
 This package add a link from sendmail to msmtp
 and is built with SSL support.
endef

define Package/msmtp-queue
$(call Package/msmtp/Default)
  DEPENDS+= +bash +msmtp
  TITLE+= (queue scripts)
endef

define Package/msmtp-queue/description
$(call Package/msmtp/Default/description)
 This package contains the msmtp queue scripts.
endef

CONFIGURE_ARGS += \
	--disable-rpath \
	--without-libintl-prefix \
	--without-libgsasl \
	--without-libidn \
	--without-libsecret \
	--without-macosx-keyring \
	--without-msmtpd

ifeq ($(BUILD_VARIANT),ssl)
	CONFIGURE_ARGS += --with-tls=gnutls
else
	CONFIGURE_ARGS += --without-tls
endif

define Package/msmtp/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/doc/msmtprc-system.example \
		$(1)/etc/msmtprc
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/msmtp $(1)/usr/bin/
endef

define Package/msmtp-mta/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/usr/lib
endef

Package/msmtp-nossl/conffiles = $(Package/msmtp/conffiles)
Package/msmtp-nossl/install = $(Package/msmtp/install)

define Package/msmtp-queue/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scripts/msmtpq/msmtp{q,-queue} $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scripts/msmtpqueue/msmtp-{en,list,run}queue.sh $(1)/usr/bin/
endef

$(eval $(call BuildPackage,msmtp))
$(eval $(call BuildPackage,msmtp-nossl))
$(eval $(call BuildPackage,msmtp-queue))
$(eval $(call BuildPackage,msmtp-mta))
