#
# Copyright (C) 2022 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bridger
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/nbd168/bridger
PKG_SOURCE_DATE:=2023-05-12
PKG_SOURCE_VERSION:=d0f79a16c749ad310d79e1c31f593860619f99eb
PKG_MIRROR_HASH:=bee35594767cbcd13764f5c95e4837a4fc73171a91fcdae73aaefe00f4e8f8fa

PKG_LICENSE:=GPL-2.0
PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

PKG_BUILD_DEPENDS:=bpf-headers

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk
include $(INCLUDE_DIR)/bpf.mk
include $(INCLUDE_DIR)/nls.mk

define Package/bridger
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Bridge forwarding accelerator
  DEPENDS:=+libbpf +libubox +libubus +libnl-tiny +kmod-sched-core +kmod-sched-flower +kmod-sched-bpf +kmod-sched-act-vlan $(BPF_DEPENDS)
endef

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(STAGING_DIR)/usr/include

CMAKE_OPTIONS += \
	-DLIBNL_LIBS=-lnl-tiny

define Build/Compile
	$(call CompileBPF,$(PKG_BUILD_DIR)/bridger-bpf.c)
	$(Build/Compile/Default)
endef

define Package/bridger/install
	$(INSTALL_DIR) \
		$(1)/etc/config \
		$(1)/etc/init.d \
		$(1)/lib/bpf \
		$(1)/usr/sbin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/bridger-bpf.o $(1)/lib/bpf
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/bridger \
		$(1)/usr/sbin/
	$(INSTALL_DATA) ./files/bridger.conf $(1)/etc/config/bridger
	$(INSTALL_BIN) ./files/bridger.init $(1)/etc/init.d/bridger
endef

$(eval $(call BuildPackage,bridger))
