#
# Copyright (C) 2007-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iw
PKG_VERSION:=5.9-8fab0c9e
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://git.kernel.org/pub/scm/linux/kernel/git/jberg/iw.git
PKG_SOURCE_VERSION:=8fab0c9ee9db217587a58efcc37421c86edcb638
PKG_MIRROR_HASH:=797b322bc03952f3127ae0a7da476c14ada1bbe9a9ae234a56dd6f864c568e16

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/iw
  SECTION:=net
  CATEGORY:=Network
  TITLE:=cfg80211 interface configuration utility
  URL:=http://wireless.kernel.org/en/users/Documentation/iw
  DEPENDS:= +libnl-tiny
  VARIANT:=tiny
endef

define Package/iw-full
  $(Package/iw)
  TITLE += (full version)
  VARIANT:=full
  PROVIDES:=iw
endef

define Build/Configure
	echo "const char iw_version[] = \"$(PKG_VERSION)\";" > $(PKG_BUILD_DIR)/version.c
	echo "#!/bin/sh" > $(PKG_BUILD_DIR)/version.sh
	chmod +x $(PKG_BUILD_DIR)/version.sh
endef

TARGET_CPPFLAGS:= \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	$(TARGET_CPPFLAGS) \
	-DCONFIG_LIBNL20 \
	-D_GNU_SOURCE \
	-flto

ifeq ($(BUILD_VARIANT),full)
  TARGET_CPPFLAGS += -DIW_FULL
  MAKE_FLAGS += IW_FULL=1
endif

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS) -ffunction-sections -fdata-sections" \
	LDFLAGS="$(TARGET_LDFLAGS) -Wl,--gc-sections -flto" \
	NL1FOUND="" NL2FOUND=Y \
	NLLIBNAME="libnl-tiny" \
	LIBS="-lm -lnl-tiny" \
	V=1

define Package/iw/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iw $(1)/usr/sbin/
endef

Package/iw-full/install=$(Package/iw/install)

$(eval $(call BuildPackage,iw))
$(eval $(call BuildPackage,iw-full))
