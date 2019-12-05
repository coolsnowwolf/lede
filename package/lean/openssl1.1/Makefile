#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

### Modified by wongsyrone to fit need of trojan-gfw/trojan

include $(TOPDIR)/rules.mk

PKG_NAME:=openssl1.1

PKG_BASE:=1.1.1
PKG_BUGFIX:=d
PKG_VERSION:=$(PKG_BASE)$(PKG_BUGFIX)
PKG_HASH:=1e3a91bc1f9dfce01af26026f856e064eab4c8ee0a8f457b5ae30b40b8b711f2
ENGINES_DIR=engines-1.1


PKG_RELEASE:=1
PKG_USE_MIPS16:=0
PATCH_DIR=./patches/$(PKG_BASE)

PKG_BUILD_PARALLEL:=0
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/openssl-$(PKG_VERSION)

PKG_SOURCE:=openssl-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:= \
	http://www.openssl.org/source/ \
	http://www.openssl.org/source/old/$(PKG_BASE)/

PKG_LICENSE:=OpenSSL
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:openssl:openssl

include $(INCLUDE_DIR)/package.mk

ifneq ($(CONFIG_CCACHE),)
HOSTCC=$(HOSTCC_NOCACHE)
HOSTCXX=$(HOSTCXX_NOCACHE)
endif

define Package/$(PKG_NAME)/Default
  TITLE:=Open source SSL toolkit
  URL:=http://www.openssl.org/
  SECTION:=libs
  CATEGORY:=Libraries
endef

define Package/openssl1.1/Default/description
The OpenSSL Project is a collaborative effort to develop a robust,
commercial-grade, full-featured, and Open Source toolkit implementing the
Transport Layer Security (TLS) and Secure Sockets Layer (SSL) protocols as well
as a full-strength general-purpose cryptography library.
endef

define Package/libopenssl1.1
$(call Package/openssl1.1/Default)
  SUBMENU:=SSL
  TITLE+= (libraries)
  ABI_VERSION:=$(PKG_VERSION)
  MENU:=1
endef

define Package/libopenssl1.1/description
$(call Package/openssl/Default/description)
This package contains the OpenSSL shared libraries, needed by other programs.
endef


define Package/libopenssl1.1/conffiles
/etc/ssl/openssl.cnf
endef

# do NOT interfere original openssl staging dir
MY_PKG_STAGING_DIR:=$(BUILD_DIR)/openssl1.1_staging_dir

OPENSSL_OPTIONS:= no-shared no-ssl3-method

# https://github.com/openssl/openssl/issues/1607
# it seems musl-libc doesn't support this
OPENSSL_OPTIONS += no-async

OPENSSL_OPTIONS += no-sm2 no-sm3 no-sm4

OPENSSL_OPTIONS += no-idea

OPENSSL_OPTIONS += no-seed

OPENSSL_OPTIONS += no-whirlpool

OPENSSL_OPTIONS += no-deprecated

TARGET_CFLAGS := $(filter-out -O%,$(TARGET_CFLAGS)) -O3



OPENSSL_TARGET:=linux-$(call qstrip,$(CONFIG_ARCH))-openwrt


STAMP_CONFIGURED := $(STAMP_CONFIGURED)_$(shell echo $(OPENSSL_OPTIONS) | mkhash md5)

define Build/Configure
	[ -f $(STAMP_CONFIGURED) ] || { \
		rm -f $(PKG_BUILD_DIR)/*.so.* $(PKG_BUILD_DIR)/*.a; \
		find $(PKG_BUILD_DIR) -name \*.o | xargs rm -f; \
		rm -rf $(MY_PKG_STAGING_DIR); \
	}
	(cd $(PKG_BUILD_DIR); \
		./Configure $(OPENSSL_TARGET) \
			--prefix=/usr \
			--openssldir=/etc/ssl \
			--libdir=lib \
			$(TARGET_CPPFLAGS) \
			$(TARGET_LDFLAGS) \
			$(OPENSSL_OPTIONS) && \
		{ [ -f $(STAMP_CONFIGURED) ] || make clean; } \
	)

endef

TARGET_CFLAGS += $(FPIC) -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		CC="$(TARGET_CC)" \
		SOURCE_DATE_EPOCH=$(SOURCE_DATE_EPOCH) \
		OPENWRT_OPTIMIZATION_FLAGS="$(TARGET_CFLAGS)" \
		$(OPENSSL_MAKEFLAGS) \
		all
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		CC="$(TARGET_CC)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		$(OPENSSL_MAKEFLAGS) \
		install_sw install_ssldirs
endef

define Build/InstallDev
	$(INSTALL_DIR) $(MY_PKG_STAGING_DIR)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/openssl $(MY_PKG_STAGING_DIR)/usr/include/
	$(INSTALL_DIR) $(MY_PKG_STAGING_DIR)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{crypto,ssl}.a $(MY_PKG_STAGING_DIR)/usr/lib/
endef

define Build/Clean
	rm -rf $(MY_PKG_STAGING_DIR)
	$(call Build/Clean/Default)
endef

define Package/libopenssl1.1/install
true
endef

$(eval $(call BuildPackage,libopenssl1.1))
