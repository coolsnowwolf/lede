#
# Copyright (C) 2017 Val Kulkov <val.kulkov@gmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sendmail
PKG_VERSION:=8.16.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME).$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=ftp://ftp.sendmail.org/pub/sendmail
PKG_HASH:=7886d5dc4b436b86175f32b5b9c7305c80787749847e2909bf99123ecc4e64ba
PKG_MAINTAINER:=Val Kulkov <val.kulkov@gmail.com>

PKG_LICENSE:=Sendmail
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:sendmail:sendmail

PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=openssl

include $(INCLUDE_DIR)/package.mk

TARGET_OS:=OpenWrt
SITECONFIG:=site.$(TARGET_OS).m4

#
# Uncomment the following sections that are currently commented out
# to enable building and packaging Sendmail executables and default
# configuration files into the "sendmail" package.
#
# Note that in this edition of the "sendmail" package:
#   1. No init script is provided to start the Sendmail daemon.
#   2. The initial configuration files in /etc/mail are likely inadequate
#      for running the Sendmail daemon on your LEDE/OpenWrt device.
#   3. You may have to "opkg install m4" to build Sendmail configuration files
#      from customized mc files on your LEDE/OpenWrt device using m4, or
#      alternatively you may use a Linux computer to build configuration
#      files from customized mc files.
#   4. Compiling and building the "sendmail" package will likely fail at
#      the packaging stage if you have already selected and built the "ssmtp"
#      package. "ssmtp" installs /usr/sbin/sendmail in the form of a symlink
#      to /usr/sbin/ssmtp. Where /usr/sbin/sendmail is already present,
#      copying the sendmail executable into /usr/sbin/sendmail results in
#      an error:
#        cp: not writing through dangling symlink
#        ...
#        lede/source/include/toplevel.mk:206: recipe for target 'package/feeds/packages/libmilter/compile' failed
#
# ANYONE WHO IS READING THIS MAKEFILE AND WHO HAS THE KNOWLEDGE AND DESIRE
# TO COMPLETE PORTING OF SENDMAIL TO LEDE/OPENWRT PLATFORM IS ENCOURAGED
# TO DO SO AND TAKE OVER THE MAINTAINERSHIP OF THIS PACKAGE FROM VAL KULKOV.
#
#define Package/sendmail
#  SECTION:=mail
#  CATEGORY:=Mail
#  DEPENDS:=+libopenssl
#  TITLE:=A general purpose internetwork mail routing facility
#  URL:=http://sendmail.org/
#endef
#
#define Package/sendmail/description
#   Sendmail is a general purpose internetwork email routing facility that
#   supports many kinds of mail-transfer and delivery methods, including
#   the Simple Mail Transfer Protocol (SMTP) used for email transport over
#   the Internet.
#endef
#
#define Package/sendmail/conffiles
#/etc/mail/helpfile
#/etc/mail/sendmail.cf
#/etc/mail/submit.cf
#/etc/mail/statistics
#endef

define Package/libmilter-sendmail
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libpthread
  TITLE:=The sendmail Mail Filter API (Milter) library
  URL:=http://sendmail.org/
endef

define Package/libmilter-sendmail/description
  The sendmail Mail Filter API (Milter) is designed to allow third-party
programs access to mail messages as they are being processed in order to
filter meta-information and content.
endef

TARGET_CFLAGS += $(FPIC)

define Build/Prepare
	$(Build/Prepare/Default)
	$(CP) files/lm_getver.c $(PKG_BUILD_DIR)/libmilter/
	$(CP) files/$(SITECONFIG) $(PKG_BUILD_DIR)/devtools/Site/
	$(CP) files/$(TARGET_OS) $(PKG_BUILD_DIR)/devtools/OS/
	$(CP) files/sharedlibrary.m4 $(PKG_BUILD_DIR)/devtools/M4/UNIX/
	$(CP) $(PKG_BUILD_DIR)/cf/cf/generic-linux.mc $(PKG_BUILD_DIR)/cf/cf/sendmail.mc
	$(SED) 's@TARGET_CC@$(TARGET_CC)@g' $(PKG_BUILD_DIR)/devtools/Site/$(SITECONFIG)
	$(SED) 's@TARGET_CFLAGS@$(TARGET_CFLAGS)@g' $(PKG_BUILD_DIR)/devtools/Site/$(SITECONFIG)
	$(SED) 's@STAGING_DIR@$(STAGING_DIR)@g' $(PKG_BUILD_DIR)/devtools/OS/$(TARGET_OS)
endef

define Build/Compile
	$(call Build/Compile/Default,)
	$(MAKE_VARS) $(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/libmilter $(MAKE_FLAGS) all
	$(MAKE_VARS) $(MAKE) -C $(PKG_BUILD_DIR)/libmilter $(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)" install
	$(MAKE_VARS) $(MAKE) -C $(PKG_BUILD_DIR)/cf/cf $(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)" install-cf
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/libmilter $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libmilter/* $(1)/usr/include/libmilter/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmilter.so* $(1)/usr/lib/
endef

define Package/libmilter-sendmail/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmilter.so* $(1)/usr/lib/
endef

#define Package/sendmail/install
#	$(INSTALL_DIR) $(1)/etc/mail $(1)/usr/bin $(1)/usr/sbin
#	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/mail/* $(1)/etc/mail/
#	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
#	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/* $(1)/usr/sbin/
#endef

$(eval $(call BuildPackage,libmilter-sendmail))
#$(eval $(call BuildPackage,sendmail))

