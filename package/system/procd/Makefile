#
# Copyright (C) 2014-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=procd
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/procd.git
PKG_SOURCE_DATE:=2020-05-28
PKG_SOURCE_VERSION:=b9b39e2061d7035a9d84eecbb4a4613deaf6d03f
PKG_MIRROR_HASH:=0d6a96a2fb38f72c72b457a2a8638bee22f91009f9686152fcf4aee97846fc84
CMAKE_INSTALL:=1

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=John Crispin <john@phrozen.org>

PKG_ASLR_PIE_REGULAR:=1
PKG_CONFIG_DEPENDS:= \
	CONFIG_TARGET_INIT_PATH CONFIG_KERNEL_SECCOMP \
	CONFIG_PROCD_SHOW_BOOT CONFIG_PROCD_ZRAM_TMPFS \
	CONFIG_KERNEL_NAMESPACES CONFIG_PACKAGE_procd-ujail CONFIG_PACKAGE_procd-seccomp

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

ifeq ($(DUMP),)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell echo $(CONFIG_TARGET_INIT_PATH) | mkhash md5)
endif

CMAKE_OPTIONS += -DEARLY_PATH="$(TARGET_INIT_PATH)"
TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lrt)

TARGET_CFLAGS += -flto
TARGET_LDFLAGS += -flto

define Package/procd
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+ubusd +ubus +libjson-script +ubox +USE_GLIBC:librt +libubox +libubus +libblobmsg-json +libjson-c
  TITLE:=OpenWrt system process manager
  USERID:=:dialout=20 :audio=29
endef

define Package/procd-ujail
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=@KERNEL_NAMESPACES +@KERNEL_UTS_NS +@KERNEL_IPC_NS +@KERNEL_PID_NS +libubox +libubus +libblobmsg-json
  TITLE:=OpenWrt process jail helper
endef

define Package/procd-ujail-console
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+procd-ujail +libubus +libubox
  TITLE:=OpenWrt process jail console
endef

define Package/procd-seccomp
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=@(arm||armeb||mips||mipsel||i386||powerpc||x86_64) @!TARGET_uml @KERNEL_SECCOMP +libubox +libblobmsg-json
  TITLE:=OpenWrt process seccomp helper + utrace
endef

define Package/procd/config
menu "Configuration"
	depends on PACKAGE_procd

config PROCD_SHOW_BOOT
	bool
	default n
	prompt "Print the shutdown to the console as well as logging it to syslog"

config PROCD_ZRAM_TMPFS
	bool
	default n
	prompt "Mount /tmp using zram."
endmenu
endef


ifeq ($(CONFIG_PROCD_SHOW_BOOT),y)
  CMAKE_OPTIONS += -DSHOW_BOOT_ON_CONSOLE=1
endif

ifeq ($(CONFIG_PROCD_ZRAM_TMPFS),y)
  CMAKE_OPTIONS += -DZRAM_TMPFS=1
endif

ifdef CONFIG_PACKAGE_procd-ujail
  CMAKE_OPTIONS += -DJAIL_SUPPORT=1
endif

SECCOMP=$(if $(CONFIG_PACKAGE_procd-seccomp),1,0)
CMAKE_OPTIONS += -DSECCOMP_SUPPORT=$(SECCOMP) -DUTRACE_SUPPORT=$(SECCOMP)

define Package/procd/install
	$(INSTALL_DIR) $(1)/sbin $(1)/etc $(1)/lib/functions

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/{init,procd,askfirst,udevtrigger,upgraded} $(1)/sbin/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libsetlbf.so $(1)/lib
	$(INSTALL_BIN) ./files/reload_config $(1)/sbin/
	$(INSTALL_CONF) ./files/hotplug*.json $(1)/etc/
	$(INSTALL_DATA) ./files/procd.sh $(1)/lib/functions/
endef

define Package/procd-ujail/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/ujail $(1)/sbin/
endef

define Package/procd-ujail-console/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/ujail-console $(1)/sbin/
endef

define Package/procd-seccomp/install
	$(INSTALL_DIR) $(1)/sbin $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libpreload-seccomp.so $(1)/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/utrace $(1)/sbin/
	$(LN) utrace $(1)/sbin/seccomp-trace
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libpreload-trace.so $(1)/lib
endef

$(eval $(call BuildPackage,procd))
$(eval $(call BuildPackage,procd-ujail))
$(eval $(call BuildPackage,procd-ujail-console))
$(eval $(call BuildPackage,procd-seccomp))
