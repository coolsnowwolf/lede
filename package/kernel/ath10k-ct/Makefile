include $(TOPDIR)/rules.mk

PKG_NAME:=ath10k-ct
PKG_RELEASE=1

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

PKG_SOURCE_URL:=https://github.com/greearb/ath10k-ct.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2018-12-20
PKG_SOURCE_VERSION:=118e16da8e82f2e028654f86e47f4c8e5bddccc8
PKG_MIRROR_HASH:=f84616d5c6c3f6c0aded40b821d9b54fd88b78737f534b37a6ed0c51db16ed06

# Build the 4.19 ath10k-ct driver version.  Other options are "-4.16", or
# leave un-defined for 4.7 kernel.  Probably this should match as closely as
# possible to whatever mac80211 backports version is being used.
CT_KVER="-4.19"

PKG_MAINTAINER:=Ben Greear <greearb@candelatech.com>
PKG_BUILD_PARALLEL:=1
PKG_EXTMOD_SUBDIRS:=ath10k$(CT_KVER)

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/ath10k-ct
  SUBMENU:=Wireless Drivers
  TITLE:=ath10k-ct driver optimized for CT ath10k firmware
  DEPENDS:=+kmod-mac80211 +kmod-ath +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT +@DRIVER_11W_SUPPORT @PCI_SUPPORT +kmod-hwmon-core
  FILES:=\
	$(PKG_BUILD_DIR)/ath10k$(CT_KVER)/ath10k_pci.ko \
	$(PKG_BUILD_DIR)/ath10k$(CT_KVER)/ath10k_core.ko
  AUTOLOAD:=$(call AutoProbe,ath10k_pci)
  PROVIDES:=kmod-ath10k
endef

define KernelPackage/ath10k-ct/config

       config ATH10K-CT_LEDS
               bool "Enable LED support"
               default y
               depends on PACKAGE_kmod-ath10k-ct
endef

NOSTDINC_FLAGS = \
	-I$(PKG_BUILD_DIR) \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-include backport/autoconf.h \
	-include backport/backport.h

ifdef CONFIG_PACKAGE_MAC80211_MESH
  NOSTDINC_FLAGS += -DCONFIG_MAC80211_MESH
endif

CT_MAKEDEFS += CONFIG_ATH10K=m CONFIG_ATH10K_PCI=m CONFIG_ATH10K_CE=y

# This AHB logic is needed for IPQ4019 radios
CT_MAKEDEFS += CONFIG_ATH10K_AHB=m
NOSTDINC_FLAGS += -DCONFIG_ATH10K_AHB

NOSTDINC_FLAGS += -DSTANDALONE_CT

ifdef CONFIG_PACKAGE_MAC80211_DEBUGFS
  CT_MAKEDEFS += CONFIG_ATH10K_DEBUGFS=y CONFIG_MAC80211_DEBUGFS=y
  NOSTDINC_FLAGS += -DCONFIG_MAC80211_DEBUGFS
  NOSTDINC_FLAGS += -DCONFIG_ATH10K_DEBUGFS
endif

ifdef CONFIG_PACKAGE_ATH_DEBUG
  NOSTDINC_FLAGS += -DCONFIG_ATH10K_DEBUG
endif

ifdef CONFIG_PACKAGE_ATH_DFS
  NOSTDINC_FLAGS += -DCONFIG_ATH10K_DFS_CERTIFIED
endif

ifdef CONFIG_PACKAGE_ATH_SPECTRAL
  CT_MAKEDEFS += CONFIG_ATH10K_SPECTRAL=y
  NOSTDINC_FLAGS += -DCONFIG_ATH10K_SPECTRAL
endif

ifeq ($(CONFIG_ATH10K-CT_LEDS),y)
  CT_MAKEDEFS += CONFIG_ATH10K_LEDS=y
  NOSTDINC_FLAGS += -DCONFIG_ATH10K_LEDS
endif

define Build/Configure
	cp $(STAGING_DIR)/usr/include/mac80211/ath/*.h $(PKG_BUILD_DIR)
endef

ifneq ($(findstring c,$(OPENWRT_VERBOSE)),)
  CT_MAKEDEFS += V=1
endif

define Build/Compile
	+$(MAKE) $(CT_MAKEDEFS) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		SUBDIRS="$(PKG_BUILD_DIR)/ath10k$(CT_KVER)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		modules
endef

$(eval $(call KernelPackage,ath10k-ct))
