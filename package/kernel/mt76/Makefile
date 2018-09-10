include $(TOPDIR)/rules.mk

PKG_NAME:=mt76
PKG_RELEASE=1

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

PKG_SOURCE_URL:=https://github.com/openwrt/mt76
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2018-08-24
PKG_SOURCE_VERSION:=30b8371ad7233c04fa825f318954791d3796c2bf
PKG_MIRROR_HASH:=388c95dbc7e6a971b3372654825c5641d9f92c8c2144768a87b00c02086ac7c8

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/mt76-default
  SUBMENU:=Wireless Drivers
  DEPENDS:= \
	+kmod-mac80211 @PCI_SUPPORT @!LINUX_3_18 \
	+@DRIVER_11AC_SUPPORT +@DRIVER_11N_SUPPORT +@DRIVER_11W_SUPPORT
endef

define KernelPackage/mt76
  SUBMENU:=Wireless Drivers
  TITLE:=MediaTek MT76x2/MT7603 wireless driver (metapackage)
  DEPENDS:= \
	+kmod-mt76-core +kmod-mt76x2 +kmod-mt7603
endef

define KernelPackage/mt76-core
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT76xx wireless driver
  FILES:=\
	$(PKG_BUILD_DIR)/mt76.ko
endef

define KernelPackage/mt76x2
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT76x2 wireless driver
  DEPENDS+=+kmod-mt76-core
  FILES:=\
	$(PKG_BUILD_DIR)/mt76x2e.ko
  AUTOLOAD:=$(call AutoProbe,mt76x2e)
endef

define KernelPackage/mt7603
  $(KernelPackage/mt76-default)
  TITLE:=MediaTek MT7603 wireless driver
  DEPENDS+=+kmod-mt76-core
  FILES:=\
	$(PKG_BUILD_DIR)/mt7603e.ko
  AUTOLOAD:=$(call AutoProbe,mt7603e)
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

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		modules
endef

define Package/kmod-mt76/install
	true
endef

define KernelPackage/mt76x2/install
	$(INSTALL_DIR) $(1)/lib/firmware
	cp \
		$(PKG_BUILD_DIR)/firmware/mt7662_rom_patch.bin \
		$(PKG_BUILD_DIR)/firmware/mt7662.bin \
		$(1)/lib/firmware
endef

define KernelPackage/mt7603/install
	$(INSTALL_DIR) $(1)/lib/firmware
	cp $(if $(CONFIG_TARGET_ramips_mt76x8), \
		$(PKG_BUILD_DIR)/firmware/mt7628_e1.bin \
		$(PKG_BUILD_DIR)/firmware/mt7628_e2.bin \
		,\
		$(PKG_BUILD_DIR)/firmware/mt7603_e1.bin \
		$(PKG_BUILD_DIR)/firmware/mt7603_e2.bin \
		) \
		$(1)/lib/firmware
endef

$(eval $(call KernelPackage,mt76-core))
$(eval $(call KernelPackage,mt76x2))
$(eval $(call KernelPackage,mt7603))
$(eval $(call KernelPackage,mt76))
