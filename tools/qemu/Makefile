#
# Copyright (C) 2010-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=qemu
PKG_VERSION:=0.14.1
PKG_RELEASE:=1

PKG_SOURCE_URL:=@SAVANNAH/qemu
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=33ceae3fbe516f2cbb151dc98d16c8ccfec74b1056674ad715e75a2f7fed45c3

include $(INCLUDE_DIR)/host-build.mk

HOST_CFLAGS += \
	-I$(STAGING_DIR_HOST)/include/e2fsprogs \
	-DAES_cbc_encrypt=QEMU_AES_cbc_encrypt \
	-DAES_decrypt=QEMU_AES_decrypt \
	-DAES_encrypt=QEMU_AES_encrypt \
	-DAES_set_decrypt_key=QEMU_AES_set_decrypt_key \
	-DAES_set_encrypt_key=QEMU_AES_set_encrypt_key

HOST_CONFIGURE_VARS := \
	CFLAGS="$(HOST_CFLAGS)" \
	LDFLAGS="$(HOST_LDFLAGS)"

HOST_CONFIGURE_ARGS := \
	--extra-cflags="$(HOST_CFLAGS)" \
	--enable-uuid

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) qemu-img
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/qemu-img $(STAGING_DIR_HOST)/bin
endef

$(eval $(call HostBuild))
