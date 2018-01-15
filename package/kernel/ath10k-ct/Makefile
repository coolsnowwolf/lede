include $(TOPDIR)/rules.mk

PKG_NAME:=ath10k-ct
PKG_RELEASE=2

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

PKG_SOURCE_URL:=https://github.com/greearb/ath10k-ct.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2017-06-13
PKG_SOURCE_VERSION:=bded1823912549017d819d1796273b3134c3de20
PKG_MIRROR_HASH:=616174650e12a82edb6b6bd18ac186e2c6a48fdad0082df9d2011ab20940814b

PKG_MAINTAINER:=Ben Greear <greearb@candelatech.com>
PKG_BUILD_PARALLEL:=1
PKG_EXTMOD_SUBDIRS:=ath10k

# Build the 4.13 ath10k-ct driver version.  Other options are "-4.9", or
# leave un-defined for 4.7 kernel.  Probably this should match as closely as
# possible to whatever mac80211 backports version is being used.
CT_KVER="-4.13"

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/ath10k-ct
  SUBMENU:=Wireless Drivers
  TITLE:=ath10k-ct driver optimized for CT ath10k firmware
  DEPENDS:=+kmod-mac80211 +kmod-ath +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT @PCI_SUPPORT +kmod-hwmon-core
  FILES:=\
	$(PKG_BUILD_DIR)/ath10k$(CT_KVER)/ath10k_pci.ko \
	$(PKG_BUILD_DIR)/ath10k$(CT_KVER)/ath10k_core.ko
  AUTOLOAD:=$(call AutoProbe,ath10k_pci)
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

CT_MAKEDEFS += CONFIG_ATH10K=m CONFIG_ATH10K_PCI=m
# No AHB support enabled yet.  Could conditionally enable it later.
#CT_MAKEDEFS += CONFIG_ATH10K_AHB=y
#NOSTDINC_FLAGS += -DCONFIG_ATH10K_AHB
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
