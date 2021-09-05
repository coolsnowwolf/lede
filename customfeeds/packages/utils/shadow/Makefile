#
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=shadow
PKG_VERSION:=4.8.1
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/shadow-maint/shadow/releases/download/$(PKG_VERSION)
PKG_HASH:=a3ad4630bdc41372f02a647278a8c3514844295d36eefe68ece6c3a641c1ae62

PKG_MAINTAINER:=
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:debian:shadow

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=libxcrypt

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

SHADOW_APPLETS := \
	chage chfn chgpasswd chpasswd chsh expiry faillog gpasswd \
	groupadd groupdel groupmems groupmod groups grpck grpconv grpunconv \
	lastlog login logoutd newgidmap newgrp newuidmap newusers nologin \
	passwd pwck pwconv pwunconv su \
	useradd userdel usermod vipw

CONFIGURE_ARGS += \
	--without-audit \
	--without-libpam \
	--without-selinux \
	--without-acl \
	--without-attr \
	--without-tcb \
	--without-nscd \
	--with-bcrypt

CONFIGURE_VARS += \
	ac_cv_func_ruserok=no

define Package/shadow/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=The PLD Linux shadow utilities
  URL:=https://github.com/shadow-maint/shadow
endef

define Package/shadow
  $(call Package/shadow/Default)
  TITLE+= (all)
  DEPENDS:=$(foreach u,$(SHADOW_APPLETS),+shadow-$(u))
  HIDDEN:=1
endef

define Package/shadow/description
  Full versions of standard shadow utilities. Normally, you would not
  use this package, since the functionality in BusyBox is more than
  sufficient and much smaller.
endef

define Package/shadow/install
	true
endef

define Package/shadow-utils
  $(call Package/shadow/Default)
endef

define Package/shadow-utils/config
    menu "Select shadow utilities"
      depends on PACKAGE_shadow-utils

    config shadow-all
      bool "Include all PLD shadow utilities"
      select PACKAGE_shadow
      default y

    comment "Utilities"

  $(foreach u,$(SHADOW_APPLETS), \
    config PACKAGE_shadow-$(u)
      prompt "Full PLD version of $(u) utility"

   )

  endmenu

endef

Package/shadow-utils/description = $(Package/shadow/description)

define Package/shadow-common
  $(call Package/shadow/Default)
  TITLE:=Shared definitions for the PLD Linux shadow utilities
  DEPENDS:=$(ICONV_DEPENDS) $(INTL_DEPENDS)
  HIDDEN:=1
endef

define GenPlugin
  define Package/shadow-$(1)
    $(call Package/shadow/Default)
    TITLE:=Utility $(1) from the PLD Linux shadow utilities
    DEPENDS:=+shadow-common
    HIDDEN:=1
  endef

  define Package/shadow-$(1)/description
    Full version of standard $(1) utility. Normally, you would not use this
    package, since the functionality in BusyBox is more than sufficient.
  endef
endef

$(foreach u,$(SHADOW_APPLETS),$(eval $(call GenPlugin,$(u))))

define Package/shadow-common/conffiles
/etc/login.defs
endef

#hack to get libxcrypt working
TARGET_LDFLAGS:=-L$(STAGING_DIR)/usr/lib/libxcrypt $(TARGET_LDFLAGS)

define Package/shadow-common/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/login.defs $(1)/etc/
	$(SED) 's,SU_NAME,#SU_NAME,g' $(1)/etc/login.defs
	$(SED) 's,#ENCRYPT_METHOD DES,ENCRYPT_METHOD BCRYPT,g' $(1)/etc/login.defs
endef

define Package/shadow-utils/install
	true
endef

define BuildPlugin
  define Package/shadow-$(1)/install
	[ -x "$(PKG_INSTALL_DIR)/usr/sbin/$(1)" ] && { \
		$(INSTALL_DIR) $$(1)/usr/sbin; \
		$(CP) $(PKG_INSTALL_DIR)/usr/sbin/$(1) $$(1)/usr/sbin/; \
	} || { \
		$(INSTALL_DIR) $$(1)/usr/bin; \
		$(CP) $(PKG_INSTALL_DIR)/usr/bin/$(1) $$(1)/usr/bin/; \
	}
  endef

  $$(eval $$(call BuildPackage,shadow-$(1)))
endef

$(foreach u,$(SHADOW_APPLETS),$(eval $(call BuildPlugin,$(u))))

$(eval $(call BuildPackage,shadow-utils))
$(eval $(call BuildPackage,shadow-common))
$(eval $(call BuildPackage,shadow))
