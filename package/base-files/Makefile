#
# Copyright (C) 2007-2016 OpenWrt.org
# Copyright (C) 2010 Vertical Communications
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/version.mk
include $(INCLUDE_DIR)/feeds.mk

PKG_NAME:=base-files
PKG_RELEASE:=199
PKG_FLAGS:=nonshared

PKG_FILE_DEPENDS:=$(PLATFORM_DIR)/ $(GENERIC_PLATFORM_DIR)/base-files/
PKG_BUILD_DEPENDS:=usign/host ucert/host
PKG_LICENSE:=GPL-2.0

# Extend depends from version.mk
PKG_CONFIG_DEPENDS += \
	CONFIG_SIGNED_PACKAGES CONFIG_TARGET_INIT_PATH CONFIG_TARGET_PREINIT_DISABLE_FAILSAFE \
	CONFIG_NAND_SUPPORT \
	CONFIG_CLEAN_IPKG \
	CONFIG_PER_FEED_REPO \
	$(foreach feed,$(FEEDS_AVAILABLE),CONFIG_FEED_$(feed))

include $(INCLUDE_DIR)/package.mk

ifneq ($(DUMP),1)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell echo $(CONFIG_TARGET_INIT_PATH) | mkhash md5)
  TARGET:=-$(BOARD)
endif

define Package/base-files
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+netifd +libc +procd +jsonfilter +SIGNED_PACKAGES:usign +SIGNED_PACKAGES:openwrt-keyring +NAND_SUPPORT:ubi-utils +fstools +fwtool
  TITLE:=Base filesystem for OpenWrt
  URL:=http://openwrt.org/
  VERSION:=$(PKG_RELEASE)-$(REVISION)
endef

define Package/base-files/conffiles
/etc/config/
/etc/config/network
/etc/config/system
/etc/crontabs/
/etc/dropbear/
/etc/ethers
/etc/group
/etc/hosts
/etc/inittab
/etc/iproute2/rt_protos
/etc/iproute2/rt_tables
/etc/passwd
/etc/profile
/etc/profile.d
/etc/protocols
/etc/rc.local
/etc/services
/etc/shadow
/etc/shells
/etc/sysctl.conf
/etc/sysupgrade.conf
$(call $(TARGET)/conffiles)
endef

define Package/base-files/description
 This package contains a base filesystem and system scripts for OpenWrt.
endef

define ImageConfigOptions
	mkdir -p $(1)/lib/preinit
	echo 'pi_suppress_stderr="$(CONFIG_TARGET_PREINIT_SUPPRESS_STDERR)"' >$(1)/lib/preinit/00_preinit.conf
	echo 'fs_failsafe_wait_timeout=$(if $(CONFIG_TARGET_PREINIT_TIMEOUT),$(CONFIG_TARGET_PREINIT_TIMEOUT),2)' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_init_path="$(TARGET_INIT_PATH)"' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_init_env=$(if $(CONFIG_TARGET_INIT_ENV),$(CONFIG_TARGET_INIT_ENV),"")' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_init_cmd=$(if $(CONFIG_TARGET_INIT_CMD),$(CONFIG_TARGET_INIT_CMD),"/sbin/init")' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_init_suppress_stderr="$(CONFIG_TARGET_INIT_SUPPRESS_STDERR)"' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_ifname=$(if $(CONFIG_TARGET_PREINIT_IFNAME),$(CONFIG_TARGET_PREINIT_IFNAME),"")' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_ip=$(if $(CONFIG_TARGET_PREINIT_IP),$(CONFIG_TARGET_PREINIT_IP),"192.168.1.1")' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_netmask=$(if $(CONFIG_TARGET_PREINIT_NETMASK),$(CONFIG_TARGET_PREINIT_NETMASK),"255.255.255.0")' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_broadcast=$(if $(CONFIG_TARGET_PREINIT_BROADCAST),$(CONFIG_TARGET_PREINIT_BROADCAST),"192.168.1.255")' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_preinit_net_messages="$(CONFIG_TARGET_PREINIT_SHOW_NETMSG)"' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_preinit_no_failsafe_netmsg="$(CONFIG_TARGET_PREINIT_SUPPRESS_FAILSAFE_NETMSG)"' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_preinit_no_failsafe="$(CONFIG_TARGET_PREINIT_DISABLE_FAILSAFE)"' >>$(1)/lib/preinit/00_preinit.conf
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile/Default

endef
Build/Compile = $(Build/Compile/Default)

ifdef CONFIG_SIGNED_PACKAGES
  define Build/Configure
	[ -s $(BUILD_KEY) -a -s $(BUILD_KEY).pub ] || \
		$(STAGING_DIR_HOST)/bin/usign -G -s $(BUILD_KEY) -p $(BUILD_KEY).pub -c "Local build key"

	[ -s $(BUILD_KEY).ucert ] || \
		$(STAGING_DIR_HOST)/bin/ucert -I -c $(BUILD_KEY).ucert -p $(BUILD_KEY).pub -s $(BUILD_KEY)

  endef

  define Package/base-files/install-key
	mkdir -p $(1)/etc/opkg/keys
	$(CP) $(BUILD_KEY).pub $(1)/etc/opkg/keys/`$(STAGING_DIR_HOST)/bin/usign -F -p $(BUILD_KEY).pub`

  endef
endif

ifeq ($(CONFIG_NAND_SUPPORT),)
  define Package/base-files/nand-support
	rm -f $(1)/lib/upgrade/nand.sh
  endef
endif

define Package/base-files/install
	$(CP) ./files/* $(1)/
	$(Package/base-files/install-key)
	$(Package/base-files/nand-support)
	if [ -d $(GENERIC_PLATFORM_DIR)/base-files/. ]; then \
		$(CP) $(GENERIC_PLATFORM_DIR)/base-files/* $(1)/; \
	fi
	if [ -d $(PLATFORM_DIR)/base-files/. ]; then \
		$(CP) $(PLATFORM_DIR)/base-files/* $(1)/; \
	fi
	$(if $(filter-out $(PLATFORM_DIR),$(PLATFORM_SUBDIR)), \
		if [ -d $(PLATFORM_SUBDIR)/base-files/. ]; then \
			$(CP) $(PLATFORM_SUBDIR)/base-files/* $(1)/; \
		fi; \
	)

	$(VERSION_SED_SCRIPT) \
		$(1)/etc/banner \
		$(1)/etc/openwrt_version \
		$(1)/usr/lib/os-release

	$(VERSION_SED_SCRIPT) \
		$(1)/etc/openwrt_release \
		$(1)/etc/device_info \
		$(1)/usr/lib/os-release

	$(SED) "s#%PATH%#$(TARGET_INIT_PATH)#g" \
		$(1)/sbin/hotplug-call \
		$(1)/etc/preinit \
		$(1)/etc/profile

	mkdir -p $(1)/CONTROL
	mkdir -p $(1)/dev
	mkdir -p $(1)/etc/config
	mkdir -p $(1)/etc/crontabs
	mkdir -p $(1)/etc/rc.d
	mkdir -p $(1)/overlay
	mkdir -p $(1)/lib/firmware
	$(if $(LIB_SUFFIX),-$(LN) lib $(1)/lib$(LIB_SUFFIX))
	mkdir -p $(1)/mnt
	mkdir -p $(1)/proc
	mkdir -p $(1)/tmp
	mkdir -p $(1)/usr/lib
	$(if $(LIB_SUFFIX),-$(LN) lib $(1)/usr/lib$(LIB_SUFFIX))
	mkdir -p $(1)/usr/bin
	mkdir -p $(1)/sys
	mkdir -p $(1)/www
	mkdir -p $(1)/root
	$(LN) /proc/mounts $(1)/etc/mtab
	rm -f $(1)/var
	$(LN) tmp $(1)/var
	mkdir -p $(1)/etc
	$(LN) /tmp/resolv.conf /tmp/TZ /tmp/localtime $(1)/etc/

	chmod 0600 $(1)/etc/shadow
	chmod 1777 $(1)/tmp

	$(call ImageConfigOptions,$(1))
	$(call Package/base-files/install-target,$(1))
	for conffile in $(1)/etc/config/*; do \
		if [ -f "$$$$conffile" ]; then \
			grep "$$$${conffile##$(1)}" $(1)/CONTROL/conffiles || \
				echo "$$$${conffile##$(1)}" >> $(1)/CONTROL/conffiles; \
		fi \
	done

	$(if $(CONFIG_INCLUDE_CONFIG), \
		echo -e "# Build configuration for board $(BOARD)/$(SUBTARGET)/$(PROFILE)\n" >$(1)/etc/build.config; \
		cat $(BIN_DIR)/config.seed >>$(1)/etc/build.config)

	$(if $(CONFIG_CLEAN_IPKG),, \
		mkdir -p $(1)/etc/opkg; \
		$(call FeedSourcesAppend,$(1)/etc/opkg/distfeeds.conf); \
		$(VERSION_SED_SCRIPT) $(1)/etc/opkg/distfeeds.conf)
endef

ifneq ($(DUMP),1)
  -include $(PLATFORM_DIR)/base-files.mk
  -include $(PLATFORM_SUBDIR)/base-files.mk
endif

$(eval $(call BuildPackage,base-files))
