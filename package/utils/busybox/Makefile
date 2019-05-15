#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=busybox
PKG_VERSION:=1.30.0
PKG_RELEASE:=1
PKG_FLAGS:=essential

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://www.busybox.net/downloads \
		http://sources.buildroot.net
PKG_HASH:=9553da068c0a30b1b8b72479908c1ba58672e2be7b535363a88de5e0f7bc04ce

PKG_BUILD_DEPENDS:=BUSYBOX_USE_LIBRPC:librpc BUSYBOX_CONFIG_PAM:libpam
PKG_BUILD_PARALLEL:=1
PKG_CHECK_FORMAT_SECURITY:=0

#Busybox use it's own PIE config flag and LDFLAGS are used with ld, not gcc.
PKG_ASLR_PIE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE archival/libarchive/bz/LICENSE
PKG_CPE_ID:=cpe:/a:busybox:busybox

include $(INCLUDE_DIR)/package.mk

ifeq ($(DUMP),)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell grep '^CONFIG_BUSYBOX_' $(TOPDIR)/.config | mkhash md5)
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
    $(call BUSYBOX_IF_ENABLED,FLOCK,	100:/usr/bin/flock:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,FREE,	100:/usr/bin/free:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,IP,	100:/sbin/ip:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,KILL,	100:/bin/kill:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,PGREP,	100:/usr/bin/pgrep:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,PKILL,	100:/usr/bin/pkill:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,PMAP,	100:/usr/bin/pmap:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,PS,	100:/bin/ps:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,PWDX,	100:/usr/bin/pwdx:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,TOP,	100:/usr/bin/top:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,UPTIME,	100:/usr/bin/uptime:/bin/busybox) \
    $(call BUSYBOX_IF_ENABLED,WATCH,	100:/bin/watch:/bin/busybox) \

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


ifndef CONFIG_USE_MUSL
LDLIBS:=m crypt
endif

LDLIBS += $(call BUSYBOX_IF_ENABLED,PAM,pam pam_misc pthread)
ifdef CONFIG_BUSYBOX_USE_LIBRPC
  LDLIBS += rpc
endif
ifeq ($(CONFIG_USE_GLIBC),y)
  LDLIBS += $(call BUSYBOX_IF_ENABLED,NSLOOKUP_OPENWRT,resolv)
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
	grep 'CONFIG_BUSYBOX_$(BUSYBOX_SYM)' $(TOPDIR)/.config | sed -e "s,\\(# \)\\?CONFIG_BUSYBOX_$(BUSYBOX_SYM)_\\(.*\\),\\1CONFIG_\\2,g" > $(PKG_BUILD_DIR)/.config
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
