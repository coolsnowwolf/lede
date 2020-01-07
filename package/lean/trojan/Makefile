#
# Copyright (C) 2018-2019 wongsyrone
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=trojan
PKG_VERSION:=1.14.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/trojan-gfw/trojan.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=18763b7529963efef8dcb01f50389a6f49213ae1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

CMAKE_INSTALL:=1
PKG_BUILD_PARALLEL:=0
PKG_BUILD_DEPENDS:=openssl

PKG_LICENSE:=GPL-3.0
PKG_MAINTAINER:=GreaterFire

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

TARGET_CXXFLAGS += -Wall -Wextra
TARGET_CXXFLAGS += $(FPIC)

# LTO
TARGET_CXXFLAGS += -flto
TARGET_LDFLAGS += -flto

# CXX standard
TARGET_CXXFLAGS += -std=c++11
TARGET_CXXFLAGS := $(filter-out -O%,$(TARGET_CXXFLAGS)) -O3
TARGET_CXXFLAGS += -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections

CMAKE_OPTIONS += \
	-DENABLE_MYSQL=OFF \
	-DENABLE_SSL_KEYLOG=ON \
	-DENABLE_NAT=ON \
	-DFORCE_TCP_FASTOPEN=OFF \
	-DSYSTEMD_SERVICE=OFF \
	-DOPENSSL_USE_STATIC_LIBS=FALSE \
	-DBoost_DEBUG=OFF \
	-DBoost_NO_BOOST_CMAKE=ON

define Package/trojan
	SECTION:=net
	CATEGORY:=Network
	TITLE:=An unidentifiable mechanism that helps you bypass GFW
	URL:=https://github.com/trojan-gfw/trojan
	DEPENDS:=+libpthread +libstdcpp +libopenssl \
		+boost +boost-system +boost-program_options +boost-date_time
endef

define Package/trojan/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/trojan $(1)/usr/sbin/trojan
endef

$(eval $(call BuildPackage,trojan))
