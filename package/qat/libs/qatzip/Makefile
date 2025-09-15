#
# Copyright (C) DL <revelstone@yahoo.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
# 
#
include $(TOPDIR)/rules.mk

PKG_NAME:=qatzip
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/intel/QATzip
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2020-05-15
PKG_SOURCE_VERSION:=5abaaf546a479350ada023115c6e3d499db2b363
PKG_MIRROR_HASH:=17ff33161e7a6b513f90050fb747e0a6f0a74ea3e4e3b1dc10b18e5540adbe9a

PKG_BUILD_DEPENDS:=openssl
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/libqatzip
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=Compression
  TITLE:=Intel Quick Assist QATzip Compression Library
  URL:=https://github.com/intel/QATzip
  DEPENDS:= \
	@TARGET_x86_64 \
	+zlib \
	+openssl-qat
endef

define Package/libqatzip/description
 Intel Quick Assist userspace compression library
endef

define Package/qzip
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Compression
  TITLE:=Intel Quick Assist qzip compression utility
  URL:=https://github.com/intel/QATzip
  DEPENDS:= \
	@TARGET_x86_64 \
	+libqatzip
endef

define Package/qzip/description
 Intel Quick Assist file compression utility
endef

PKG_BUILD_DEPENDS += $(if $(CONFIG_QAT_C2XXX),quickassist-c2xxx,quickassist-c3xxx)
QAT_DIR=$(KERNEL_BUILD_DIR)/$(if $(CONFIG_QAT_C2XXX),quickassist-c2xxx-1.5,quickassist-c3xxx-1.7)/quickassist

TARGET_CFLAGS += \
	-I$(QAT_DIR)/include \
	-I$(QAT_DIR)/include/dc \
	-I$(QAT_DIR)/lookaside/access_layer/include \
	-I$(QAT_DIR)/utilities/libusdm_drv \
	-I$(PKG_BUILD_DIR)/include \
	-I$(PKG_BUILD_DIR)/src \
	-DADF_PCI_API \
	-fPIC

TARGET_LDFLAGS += \
	-L$(QAT_DIR)/build 

MAKE_VARS += \
	QATZIP_LIB_STATIC="libqatzip.a" \
	QATZIP_LIB_SHARED="libqatzip.so" \
	QATZIP_LIB_D="$(PKG_BUILD_DIR)/src" \
	LIBADD="-lqat_s -lusdm_drv_s -lz -lpthread -ludev -lssl" \
	VER="1.0.1" \
	VER_M="1" \
	LN_S="ln -s"

define Build/Configure
endef

define Build/Compile
	$(call Build/Compile/Default, -C src libqatzip.so)
	$(call Build/Compile/Default, -C src libqatzip.a)
	$(call Build/Compile/Default, -C utils qzip)
endef

define Package/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/src/lib*.{a,so*} $(1)/usr/lib/	
endef

define Package/libqatzip/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/lib*.so* $(1)/usr/lib/	
endef

define Package/qzip/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/utils/qzip $(1)/usr/bin/qzip
endef

$(eval $(call BuildPackage,libqatzip))
$(eval $(call BuildPackage,qzip))
