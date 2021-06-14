# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2006-2021 OpenWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=busybox
PKG_VERSION:=1.33.1
PKG_RELEASE:=$(AUTORELEASE)
PKG_FLAGS:=essential

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://www.busybox.net/downloads \
		http://sources.buildroot.net
PKG_HASH:=12cec6bd2b16d8a9446dd16130f2b92982f1819f6e1c5f5887b6db03f5660d28

PKG_BUILD_DEPENDS:=BUSYBOX_CONFIG_PAM:libpam
PKG_BUILD_PARALLEL:=1
PKG_CHECK_FORMAT_SECURITY:=0

#Busybox use it's own PIE config flag and LDFLAGS are used with ld, not gcc.
PKG_ASLR_PIE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE archival/libarchive/bz/LICENSE
PKG_CPE_ID:=cpe:/a:busybox:busybox

BUSYBOX_SYM=$(if $(CONFIG_BUSYBOX_CUSTOM),CONFIG,DEFAULT)
BUSYBOX_IF_ENABLED=$(if $(CONFIG_BUSYBOX_$(BUSYBOX_SYM)_$(1)),$(2))

ifneq ($(CONFIG_BUSYBOX_$(BUSYBOX_SYM)_FEATURE_SUID),)
  PKG_FILE_MODES:=/bin/busybox:root:root:4755
endif

include $(INCLUDE_DIR)/package.mk

ifeq ($(DUMP),)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell grep '^CONFIG_BUSYBOX_' $(TOPDIR)/.config | $(MKHASH) md5)
endif

# All files provided by busybox will serve as fallback alternatives by opkg.
# There should be no need to enumerate ALTERNATIVES entries here
define Package/busybox/Default
  SECTION:=base
  CATEGORY:=Base system
  MAINTAINER:=Felix Fietkau <nbd@nbd.name>
  TITLE:=Core utilities for embedded Linux
  URL:=http://busybox.net/
  DEPENDS:=+BUSYBOX_CONFIG_PAM:libpam +BUSYBOX_CONFIG_NTPD:jsonfilter
  USERID:=ntp=123:ntp=123
endef

define Package/busybox
  $(call Package/busybox/Default)
  CONFLICTS:=busybox-selinux
  VARIANT:=default
endef

define Package/busybox-selinux
  $(call Package/busybox/Default)
  TITLE += with SELinux support
  DEPENDS += +libselinux
  VARIANT:=selinux
  PROVIDES:=busybox
endef

define Package/busybox/description
 The Swiss Army Knife of embedded Linux.
 It slices, it dices, it makes Julian Fries.
endef

define Package/busybox/config
	source "$(SOURCE)/Config.in"
endef

ifneq ($(CONFIG_BUSYBOX_$(BUSYBOX_SYM)_FEATURE_SYSLOG)$(CONFIG_BUSYBOX_$(BUSYBOX_SYM)_FEATURE_SYSLOGD_CFG),)
define Package/busybox/conffiles/syslog
/etc/syslog.conf
endef
endif

ifneq ($(CONFIG_BUSYBOX_$(BUSYBOX_SYM)_CROND),)
define Package/busybox/conffiles/crond
/etc/crontabs/
endef
endif

define Package/busybox/conffiles
$(Package/busybox/conffiles/syslog)
$(Package/busybox/conffiles/crond)
endef

Package/busybox-selinux/conffiles = $(Package/busybox/conffiles)

ifndef CONFIG_USE_MUSL
LDLIBS:=m crypt
endif

LDLIBS += $(call BUSYBOX_IF_ENABLED,PAM,pam pam_misc pthread)

ifeq ($(CONFIG_USE_GLIBC),y)
  LDLIBS += $(call BUSYBOX_IF_ENABLED,NSLOOKUP,resolv)
endif

ifeq ($(BUILD_VARIANT),selinux)
  LDLIBS += selinux sepol
endif

TARGET_CFLAGS += -flto
TARGET_LDFLAGS += -flto=jobserver -fuse-linker-plugin

MAKE_VARS :=
MAKE_FLAGS += \
	EXTRA_CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
	EXTRA_LDFLAGS="$(TARGET_LDFLAGS)" \
	LDLIBS="$(LDLIBS)" \
	LD="$(TARGET_CC)" \
	SKIP_STRIP=y
ifneq ($(findstring c,$(OPENWRT_VERBOSE)),)
  MAKE_FLAGS += V=1
endif

define Build/Configure
	rm -f $(PKG_BUILD_DIR)/.config
	touch $(PKG_BUILD_DIR)/.config
ifeq ($(DEVICE_TYPE),nas)
	echo "CONFIG_HDPARM=y" >> $(PKG_BUILD_DIR)/.config
endif
ifeq ($(BUILD_VARIANT),selinux)
	cat $(TOPDIR)/$(SOURCE)/selinux.config >> $(PKG_BUILD_DIR)/.config
endif
	grep 'CONFIG_BUSYBOX_$(BUSYBOX_SYM)' $(TOPDIR)/.config | sed -e "s,\\(# \)\\?CONFIG_BUSYBOX_$(BUSYBOX_SYM)_\\(.*\\),\\1CONFIG_\\2,g" >> $(PKG_BUILD_DIR)/.config
	yes 'n' | $(MAKE) -C $(PKG_BUILD_DIR) $(MAKE_FLAGS) oldconfig
endef

define Build/Compile
	$(call Build/Compile/Default, \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all install \
	)
endef

define Package/busybox/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
ifneq ($(CONFIG_BUSYBOX_$(BUSYBOX_SYM)_FEATURE_SYSLOG)$(CONFIG_BUSYBOX_$(BUSYBOX_SYM)_FEATURE_SYSLOGD_CFG),)
	touch $(1)/etc/syslog.conf
endif
ifneq ($(CONFIG_BUSYBOX_$(BUSYBOX_SYM)_CROND),)
	$(INSTALL_BIN) ./files/cron $(1)/etc/init.d/cron
	$(INSTALL_DIR) $(1)/etc/crontabs
endif
ifneq ($(CONFIG_BUSYBOX_$(BUSYBOX_SYM)_NTPD),)
	$(INSTALL_BIN) ./files/sysntpd $(1)/etc/init.d/sysntpd
	$(INSTALL_BIN) ./files/ntpd-hotplug $(1)/usr/sbin/ntpd-hotplug
	$(INSTALL_DIR) $(1)/etc/capabilities/
	$(INSTALL_DATA) ./files/ntpd.capabilities $(1)/etc/capabilities/ntpd.json
endif
	-rm -rf $(1)/lib64
endef

Package/busybox-selinux/install = $(Package/busybox/install)

$(eval $(call BuildPackage,busybox))
$(eval $(call BuildPackage,busybox-selinux))
