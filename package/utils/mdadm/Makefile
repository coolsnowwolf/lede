#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mdadm
PKG_VERSION:=4.0
PKG_RELEASE:=4

PKG_SOURCE_URL:=@KERNEL/linux/utils/raid/mdadm
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=1d6ae7f24ced3a0fa7b5613b32f4a589bb4881e3946a5a2c3724056254ada3a9

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/mdadm
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Disc
  TITLE:=A tool for managing Soft RAID under Linux
  URL:=http://www.kernel.org/pub/linux/utils/raid/mdadm/
  DEPENDS:=+libpthread +kmod-md-mod +kmod-md-raid0 +kmod-md-raid10 +kmod-md-raid1
endef

define Package/mdadm/description
 A tool for managing Linux Software RAID arrays.
 RAID 0, 1 and 10 support included.
 If you need RAID 4,5 or 6 functionality please
 install kmod-md-raid456 .
endef

define Package/mdadm/conffiles
/etc/config/mdadm
endef

TARGET_CFLAGS += \
	-ffunction-sections -fdata-sections \
	-DHAVE_STDINT_H -DNO_COROSYNC -DNO_DLM -DUSE_PTHREADS \
	-DCONFFILE='\"/var/etc/mdadm.conf\"' \
	-DMAP_DIR='\"/var/run/mdadm\"' \
	-DMDMON_DIR='\"/var/run/mdadm\"' \
	-DFAILED_SLOTS_DIR='\"/var/run/mdadm/failed-slots\"'

TARGET_LDFLAGS += -Wl,--gc-sections

MAKE_VARS += CHECK_RUN_DIR=0

define Build/Compile
	$(call Build/Compile/Default,mdadm)
endef

define Package/mdadm/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mdadm $(1)/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/mdadm.init $(1)/etc/init.d/mdadm
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/mdadm.config $(1)/etc/config/mdadm
endef

$(eval $(call BuildPackage,mdadm))
