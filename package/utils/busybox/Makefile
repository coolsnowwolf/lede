#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=busybox
PKG_VERSION:=1.27.2
PKG_RELEASE:=1
PKG_FLAGS:=essential

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://www.busybox.net/downloads \
		http://sources.buildroot.net
PKG_HASH:=9d4be516b61e6480f156b11eb42577a13529f75d3383850bb75c50c285de63df

PKG_BUILD_DEPENDS:=BUSYBOX_USE_LIBRPC:librpc BUSYBOX_CONFIG_PAM:libpam
PKG_BUILD_PARALLEL:=1
PKG_CHECK_FORMAT_SECURITY:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE archival/libarchive/bz/LICENSE

include $(INCLUDE_DIR)/package.mk

ifeq ($(DUMP),)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell grep '^CONFIG_BUSYBOX_' $(TOPDIR)/.config | mkhash md5)
endif

ifneq ($(findstring c,$(OPENWRT_VERBOSE)),)
  BB_MAKE_VERBOSE := V=1
else
  BB_MAKE_VERBOSE :=
endif

BUSYBOX_SYM=$(if $(CONFIG_BUSYBOX_CUSTOM),CONFIG,DEFAULT)
BUSYBOX_IF_ENABLED=$(if $(CONFIG_BUSYBOX_$(BUSYBOX_SYM)_$(1)),$(2))

define Package/busybox
  SECTION:=base
  CATEGORY:=Base system
  MAINTAINER:=Felix Fietkau <nbd@nbd.name>
  TITLE:=Core utilities for embedded Linux
  URL:=http://busybox.net/
  DEPENDS:=+BUSYBOX_USE_LIBRPC:librpc +BUSYBOX_CONFIG_PAM:libpam +BUSYBOX_CONFIG_NTPD:jsonfilter
  MENU:=1
  ALTERNATIVES:=\
    $(call BUSYBOX_IF_ENABLED,KILL,	100:/bin/kill:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,PS,	100:/bin/ps:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,WATCH,	100:/bin/watch:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,IP,	100:/sbin/ip:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,FREE,	100:/usr/bin/free:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,PGREP,	100:/usr/bin/pgrep:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,PKILL,	100:/usr/bin/pkill:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,PMAP,	100:/usr/bin/pmap:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,PWDX,	100:/usr/bin/pwdx:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,TOP,	100:/usr/bin/top:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,UPTIME,	100:/usr/bin/uptime:/bin/busybox) \

endef

define Package/busybox/description
 The Swiss Army Knife of embedded Linux.
 It slices, it dices, it makes Julian Fries.
endef

define Package/busybox/config
	source "$(SOURCE)/Config.in"
endef

# don't create a version string containing the actual timestamp
export KCONFIG_NOTIMESTAMP=1

define Build/Configure
	rm -f $(PKG_BUILD_DIR)/.configured*
	grep 'CONFIG_BUSYBOX_$(BUSYBOX_SYM)' $(TOPDIR)/.config | sed -e "s,\\(# \)\\?CONFIG_BUSYBOX_$(BUSYBOX_SYM)_\\(.*\\),\\1CONFIG_\\2,g" > $(PKG_BUILD_DIR)/.config
	yes 'n' | $(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		KBUILD_HAVE_NLS=no \
		ARCH="$(ARCH)" \
		$(BB_MAKE_VERBOSE) \
		oldconfig
endef

LDLIBS:=m crypt
ifdef CONFIG_BUSYBOX_USE_LIBRPC
  TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
  export LDFLAGS=$(TARGET_LDFLAGS)
  LDLIBS += rpc
endif

ifdef CONFIG_BUSYBOX_CONFIG_PAM
  TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
  export LDFLAGS=$(TARGET_LDFLAGS)
  LDLIBS += pam pam_misc pthread
endif

ifdef CONFIG_BUSYBOX_DEFAULT_NSLOOKUP_LEDE
  ifeq ($(CONFIG_USE_GLIBC),y)
	LDLIBS += resolv
  endif
endif

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		KBUILD_HAVE_NLS=no \
		EXTRA_CFLAGS="$(TARGET_CFLAGS)" \
		ARCH="$(ARCH)" \
		SKIP_STRIP=y \
		LDLIBS="$(LDLIBS)" \
		$(BB_MAKE_VERBOSE) \
		all
	rm -rf $(PKG_INSTALL_DIR)
	$(FIND) $(PKG_BUILD_DIR) -lname "*busybox" -exec rm \{\} \;
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		EXTRA_CFLAGS="$(TARGET_CFLAGS)" \
		ARCH="$(ARCH)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		LDLIBS="$(LDLIBS)" \
		$(BB_MAKE_VERBOSE) \
		install
endef

define Package/busybox/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
ifneq ($(CONFIG_BUSYBOX_$(BUSYBOX_SYM)_CROND),)
	$(INSTALL_BIN) ./files/cron $(1)/etc/init.d/cron
endif
ifneq ($(CONFIG_BUSYBOX_$(BUSYBOX_SYM)_NTPD),)
	$(INSTALL_BIN) ./files/sysntpd $(1)/etc/init.d/sysntpd
	$(INSTALL_BIN) ./files/ntpd-hotplug $(1)/usr/sbin/ntpd-hotplug
endif
	-rm -rf $(1)/lib64
endef

$(eval $(call BuildPackage,busybox))
