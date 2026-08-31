#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=kexec-tools
PKG_VERSION:=2.0.32
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/kernel/kexec
PKG_HASH:=8f81422a5fd2362cf6cb001b511e535565ed0f32c2f4451fb5eb68fed6710a5d

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:kernel:kexec-tools

PKG_CONFIG_DEPENDS := CONFIG_KEXEC_ZLIB CONFIG_KEXEC_LZMA CONFIG_KEXEC_ZSTD

PKG_BUILD_FLAGS:=gc-sections

include $(INCLUDE_DIR)/package.mk

define Package/kexec-tools/Default
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=https://github.com/horms/kexec-tools
endef

define Package/kexec-tools
  $(call Package/kexec-tools/Default)
  TITLE:=kexec-tools transition meta package
  DEPENDS:=+kexec
endef

define Package/kexec-tools/description
 kexec is a set of system calls that allows you to load
 another kernel from the currently executing Linux kernel.
 The kexec utility allows to load and boot another kernel.
endef

define Package/kexec
  $(call Package/kexec-tools/Default)
  TITLE:=Kernel boots kernel
  DEPENDS:=\
	@(armeb||arm||aarch64||i386||x86_64||powerpc64||mipsel||mips) \
	+KEXEC_ZLIB:zlib +KEXEC_LZMA:liblzma +KEXEC_ZSTD:libzstd @KERNEL_KEXEC
endef

define Package/kexec/description
 The kexec utility allows to load and boot another kernel.
endef

define Package/kdump
  $(call Package/kexec-tools/Default)
  TITLE:=Kernel crash analysis
  DEPENDS:=+kexec @(i386||x86_64||arm||aarch64) @KERNEL_CRASH_DUMP
endef

define Package/kdump/description
 The kdump package allows to automatically boot into a
 special kernel for analyzing kernel crashes using kdump.
endef

define Package/kexec/config
	source "$(SOURCE)/Config.in"
endef

KEXEC_TARGET_NAME:=$(ARCH)-linux-$(TARGET_SUFFIX)

CONFIGURE_ARGS = \
		--target=$(KEXEC_TARGET_NAME) \
		--host=$(REAL_GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--program-prefix="" \
		--program-suffix="" \
		--prefix=/usr \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--libexecdir=/usr/lib \
		--sysconfdir=/etc \
		$(if $(CONFIG_KEXEC_ZLIB),--with,--without)-zlib \
		$(if $(CONFIG_KEXEC_LZMA),--with,--without)-lzma \
		$(if $(CONFIG_KEXEC_ZSTD),--with,--without)-zstd \
		TARGET_LD="$(TARGET_CROSS)ld"

CONFIGURE_VARS += \
	BUILD_CC="$(HOSTCC)" \
	TARGET_CC="$(TARGET_CC)"

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) DESTDIR="$(PKG_INSTALL_DIR)" all install
endef

define Package/kexec-tools/install
	:
endef

define Package/kexec/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kexec $(1)/usr/sbin

# make a link for compatability with other distros
	$(INSTALL_DIR) $(1)/sbin
	$(LN) ../usr/sbin/kexec $(1)/sbin/kexec
endef

define Package/kdump/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d $(1)/etc/config $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/vmcore-dmesg $(1)/usr/sbin
	$(INSTALL_BIN) ./files/kdump.init $(1)/etc/init.d/kdump
	$(INSTALL_BIN) ./files/kdump.defaults $(1)/etc/uci-defaults/kdump
	$(INSTALL_CONF) ./files/kdump.config $(1)/etc/config/kdump
endef

define Package/kdump/prerm
#!/bin/sh

case $$(uname -m) in
	i?86|x86_64)
		if grep -q " crashkernel=" /boot/grub/grub.cfg; then
			mount /boot -o remount,rw
			sed -i 's/ crashkernel=[^ ]*//' /boot/grub/grub.cfg
			mount /boot -o remount,ro
		fi
		;;
esac
endef

$(eval $(call BuildPackage,kexec-tools))
$(eval $(call BuildPackage,kexec))
$(eval $(call BuildPackage,kdump))
