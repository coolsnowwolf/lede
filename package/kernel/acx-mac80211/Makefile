#
# Copyright (C) 2007-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=acx-mac80211
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://git.code.sf.net/p/acx100/acx-mac80211
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2014-02-16
PKG_SOURCE_VERSION:=b6fc31491020cb01d2cd1acc170cfa03ced7e726
PKG_MIRROR_HASH:=58590245715f0e5fb4b57aab6d91071dfb6a97d3273f5aee0b97b1edee030ed0

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_MAC80211_DEBUGFS \
	CONFIG_PACKAGE_MAC80211_MESH \

include $(INCLUDE_DIR)/package.mk

define KernelPackage/acx-mac80211
  SUBMENU:=Wireless Drivers
  TITLE:=ACX1xx mac80211 driver
  DEPENDS:=@(PCI_SUPPORT||TARGET_ar7) @mipsel +kmod-mac80211
  FILES:=$(PKG_BUILD_DIR)/acx-mac80211.ko
  AUTOLOAD:=$(call AutoProbe,acx-mac80211)
  MAINTAINER:=Florian Fainelli <florian@openwrt.org>
  MENU:=1
endef

define KernelPackage/acx-mac80211/config
	menu "Configuration"
		depends on PACKAGE_kmod-acx-mac80211

	config ACX_ID_0D
		bool "ACX1xx Radio ID 0D firmware"
		help
		  Download and install firmware for:
		    ACX1xx cards with Radio ID 0D into /lib/firmware.

	config ACX_ID_11
		bool "ACX1xx Radio ID 11 firmware"
		help
		  Download and install firmware for:
		    ACX1xx cards with Radio ID 11 into /lib/firmware.

	config ACX_ID_15
		bool "ACX1xx Radio ID 15 firmware"
		help
		  Download and install firmware for:
		    ACX1xx cards with Radio ID 15 into /lib/firmware.

	config ACX_ID_16
		bool "ACX1xx Radio ID 16 firmware"
		default y
		help
		  Download and install firmware for:
		    ACX1xx cards with Radio ID 16 into /lib/firmware.

	choice
		prompt "ACX111 firmware version"
		depends on ACX_ID_16
		default ACX_DEFAULT if !TARGET_adm5120
		default ACX_1_2_1_34 if TARGET_adm5120
		help
		  This option allows you to select the version of the acx firmware.

	config ACX_DEFAULT
		bool "Default"
		help
		  Default firmware for ACX111 devices.

		  If unsure, select this.

	config ACX_1_2_1_34
		bool "1.2.1_34"
		help
		  1.2.1_34 firmware for ACX111 devices. Works with Zyxel P-334WT.

		  If unsure, select the "default" firmware.

	endchoice

	config ACX_ID_17
		bool "ACX1xx Radio ID 17 firmware"
		help
		  Download and install firmware for:
		    ACX1xx cards with Radio ID 17 into /lib/firmware.

	config ACX_ID_19
		bool "ACX1xx Radio ID 19 firmware"
		default y
		help
		  Download and install firmware for:
		    ACX1xx cards with Radio ID 19 into /lib/firmware.

	config ACX_ID_1B
		bool "ACX1xx Radio ID 1B firmware"
		help
		  Download and install firmware for:
		    ACX1xx cards with Radio ID 1b into /lib/firmware.

	endmenu
endef

define KernelPackage/acx-mac80211/description
	Driver for acx111 cards (mac80211 version)
endef

define Download/tiacx100
	FILE:=tiacx100
	URL:=http://acx100.erley.org/fw/acx100_1.9.8.b/
	HASH:=4f05913c940c2455b267545b12d93ad81fa5eebb0cbee22a2c7588c50525b4f0
endef

define Download/tiacx100r0d
	FILE:=tiacx100r0D
	URL:=http://acx100.erley.org/fw/acx100_1.9.8.b/
	HASH:=6a4a7fbb24a328a88261bc2a507b2a0bf63c91e831e3f1a8caa4f6599b2215e6
endef

define Download/tiacx100r11
	FILE:=tiacx100r11
	URL:=http://acx100.erley.org/fw/acx100_1.9.8.b/
	HASH:=e005a93a0b463e01edba2b79038b54c29a7932efee61c851a2ac644b8a4e5dd4
endef

define Download/tiacx100r15
	FILE:=tiacx100r15
	URL:=http://acx100.erley.org/fw/acx100_1.9.8.b/
	HASH:=c6f40bead5ef45720e2d72bbe4d998367c2c7857eb7716234aedeb2ad98bcdde
endef

define Download/tiacx111c16
	FILE:=tiacx111c16
	URL:=http://acx100.erley.org/fw/acx111_2.3.1.31/
	HASH:=cc6108d577ebc55b924ff6bab44eeee3456d284c63819277cb5460338b2f1bd7
endef

define Download/tiacx111c16_1
	FILE:=tiacx111c16_1.2.1_34
	URL:=http://sites.google.com/site/atorkhov/files/
	HASH:=672ed9d02565ab44da450c52f0ced3be99a3a3901f73454455da8e1f98ada220
endef

define Download/tiacx111c17
	FILE:=tiacx111c17
	URL:=http://acx100.erley.org/fw/acx111_2.3.1.31/
	HASH:=2bb900a5886dbea2d3504623d9f3ac8abbb2e9fdfcf0fe233e77951dff748a40
endef

define Download/tiacx111c19
	FILE:=tiacx111c19
	URL:=http://acx100.erley.org/fw/acx111_2.3.1.31/
	HASH:=383d86a8cfddf92400d661b4e43a9b855350fa656edd4f75b4aff7fab2d00e90
endef

define Download/tiacx111usbc1b
	FILE:=tiacx111usbc1B
	URL:=http://acx100.erley.org/fw/acx111_2.4.0.70-USB/
	HASH:=f3c9e574de7073014ab6eef9a0f6412c53ae521b67723360af753c41401ed4d5
endef

PKG_EXTRA_KCONFIG:= \
	CONFIG_ACX_MAC80211=m \
	CONFIG_ACX_MAC80211_PCI=m \

PKG_EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(PKG_EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(PKG_EXTRA_KCONFIG)))) \
	$(if $(CONFIG_LEDS_TRIGGERS), -DCONFIG_MAC80211_LEDS -DCONFIG_LEDS_TRIGGERS) \
	$(if $(CONFIG_PACKAGE_MAC80211_DEBUGFS), -DCONFIG_CFG80211_DEBUGFS -DCONFIG_MAC80211_DEBUGFS) \
	$(if $(CONFIG_PACKAGE_MAC80211_MESH), -DCONFIG_MAC80211_MESH) \
	-DBACKPORTED_KERNEL_NAME=\\\"$(PKG_SOURCE)\\\" \
	-DBACKPORTED_KERNEL_VERSION=\\\"$(PKG_SOURCE_VERSION)\\\" \
	-DBACKPORTS_VERSION=\\\"unknown\\\" \

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		$(PKG_EXTRA_KCONFIG) \
		EXTRA_CFLAGS="$(PKG_EXTRA_CFLAGS) -DCONFIG_ACX_MAC80211_VERSION=\"KERNEL_VERSION(4,2,0)\"" \
		LINUXINCLUDE="-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi -I$(STAGING_DIR)/usr/include/mac80211-backport \
			-I$(STAGING_DIR)/usr/include/mac80211/uapi -I$(STAGING_DIR)/usr/include/mac80211 \
			-I$(LINUX_DIR)/include -I$(LINUX_DIR)/include/$(LINUX_UAPI_DIR) \
			-I$(LINUX_DIR)/include/generated/uapi/ -Iarch/$(LINUX_KARCH)/include \
			-Iarch/$(LINUX_KARCH)/include/$(LINUX_UAPI_DIR) \
			-Iarch/$(LINUX_KARCH)/include/generated \
			-Iarch/$(LINUX_KARCH)/include/generated/$(LINUX_UAPI_DIR) \
			-include generated/autoconf.h \
			-include backport/backport.h " \
		V="$(V)" \
		modules
endef

define Build/Configure
endef

define KernelPackage/acx-mac80211/install
	$(INSTALL_DIR) $(1)/lib/firmware

ifneq ($(CONFIG_ACX_ID_0D)$(CONFIG_ACX_ID_11)$(CONFIG_ACX_ID_15),)
	$(INSTALL_DATA) $(DL_DIR)/tiacx100 $(1)/lib/firmware/
endif

ifneq ($(CONFIG_ACX_ID_0D),)
	$(INSTALL_DATA) $(DL_DIR)/tiacx100r0D $(1)/lib/firmware/
endif

ifneq ($(CONFIG_ACX_ID_11),)
	$(INSTALL_DATA) $(DL_DIR)/tiacx100r11 $(1)/lib/firmware/
endif

ifneq ($(CONFIG_ACX_ID_15),)
	$(INSTALL_DATA) $(DL_DIR)/tiacx100r15 $(1)/lib/firmware/
endif

ifneq ($(CONFIG_ACX_DEFAULT),)
	$(INSTALL_DATA) $(DL_DIR)/tiacx111c16 $(1)/lib/firmware/
endif

ifneq ($(CONFIG_ACX_1_2_1_34),)
	$(INSTALL_DATA)	$(DL_DIR)/tiacx111c16_1.2.1_34 $(1)/lib/firmware/tiacx111c16
endif

ifneq ($(CONFIG_ACX_ID_17),)
	$(INSTALL_DATA) $(DL_DIR)/tiacx111c17 $(1)/lib/firmware/
endif

ifneq ($(CONFIG_ACX_ID_19),)
	$(INSTALL_DATA) $(DL_DIR)/tiacx111c19 $(1)/lib/firmware/
endif

ifneq ($(CONFIG_ACX_ID_1B),)
	$(INSTALL_DATA) $(DL_DIR)/tiacx111usbc1B $(1)/lib/firmware/
endif

endef

$(eval $(call KernelPackage,acx-mac80211))
$(eval $(call Download,tiacx100))
$(eval $(call Download,tiacx100r0d))
$(eval $(call Download,tiacx100r11))
$(eval $(call Download,tiacx100r15))
$(eval $(call Download,tiacx111c16))
$(eval $(call Download,tiacx111c16_1))
$(eval $(call Download,tiacx111c17))
$(eval $(call Download,tiacx111c19))
$(eval $(call Download,tiacx111usbc1b))
