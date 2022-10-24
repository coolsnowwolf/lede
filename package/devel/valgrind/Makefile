#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=valgrind
PKG_VERSION:=3.19.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://sourceware.org/pub/valgrind/
PKG_HASH:=dd5e34486f1a483ff7be7300cc16b4d6b24690987877c3278d797534d6738f02

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=GPL-2.0+
PKG_CPE_ID:=cpe:/a:valgrind:valgrind

PKG_FIXUP = autoreconf
PKG_INSTALL := 1
PKG_BUILD_PARALLEL := 1
PKG_USE_MIPS16:=0
PKG_SSP:=0

STRIP:=:

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/valgrind
  SECTION:=devel
  CATEGORY:=Development
  DEPENDS:=@mips||mipsel||mips64||mips64el||i386||x86_64||powerpc||arm_v7||aarch64 +libpthread +librt
  TITLE:=debugging and profiling tools for Linux
  URL:=http://www.valgrind.org
endef

define Package/valgrind/default
  $(Package/valgrind)
  DEPENDS := valgrind
endef

define Package/valgrind-cachegrind
  $(Package/valgrind/default)
  TITLE += (cache profiling)
endef

define Package/valgrind-callgrind
  $(Package/valgrind/default)
  TITLE += (callgraph profiling)
endef

define Package/valgrind-drd
  $(Package/valgrind/default)
  TITLE += (thread error detection)
endef

define Package/valgrind-massif
  $(Package/valgrind/default)
  TITLE += (heap profiling)
endef

define Package/valgrind-helgrind
  $(Package/valgrind/default)
  TITLE += (thread debugging)
endef

define Package/valgrind-vgdb
  $(Package/valgrind/default)
  TITLE += (GDB interface)
endef

define Package/valgrind/description
	Valgrind is an award-winning suite of tools for debugging and
	profiling Linux programs. With the tools that come with Valgrind,
	you can automatically detect many memory management and threading
	bugs, avoiding hours of frustrating bug-hunting, making your
	programs more stable. You can also perform detailed profiling,
	to speed up and reduce memory use of your programs.
endef

CPU := $(patsubst x86_64,amd64,$(patsubst x86,i386,$(patsubst um,$(ARCH),$(LINUX_KARCH))))

CONFIGURE_VARS += \
	UNAME_R=$(LINUX_VERSION)

ifeq ($(CONFIG_ARCH_64BIT),y)
	CONFIGURE_ARGS += \
		--enable-only64bit
	BITS := 64bit
else
	CONFIGURE_ARGS += \
		--enable-only32bit
	BITS := 32bit
endif

CONFIGURE_ARGS += \
	--enable-lto \
	--enable-tls \
	--without-x \
	--without-mpicc \
	--without-uiout \
	--disable-valgrindmi \
	--disable-tui \
	--disable-valgrindtk \
	--without-included-gettext \
	--with-pagesize=4 \

define Package/valgrind/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/valgrind* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/valgrind
	$(CP) \
		./files/default.supp \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/none-* \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/vgpreload_core*.so \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/$(BITS)-core*.xml \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/$(BITS)-linux*.xml \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/memcheck-* \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/vgpreload_memcheck*.so \
		$(1)/usr/lib/valgrind/

ifneq ($(ARCH),aarch64)
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/$(CPU)-*.xml \
		$(1)/usr/lib/valgrind/
endif
	$(patsubst STRIP=%,STRIP="$(TARGET_CROSS)strip",$(RSTRIP)) $(PKG_INSTALL_DIR)/usr/bin
	$(patsubst STRIP=%,STRIP="$(TARGET_CROSS)strip -g",$(RSTRIP)) \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/none-* \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/memcheck-*
endef

define Package/valgrind-cachegrind/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/cg_* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/valgrind
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/cachegrind-* \
		$(1)/usr/lib/valgrind/
	$(patsubst STRIP=%,STRIP="$(TARGET_CROSS)strip",$(RSTRIP)) $(PKG_INSTALL_DIR)/usr/bin
	$(patsubst STRIP=%,STRIP="$(TARGET_CROSS)strip -g",$(RSTRIP)) $(PKG_INSTALL_DIR)/usr/lib/valgrind/cachegrind-*
endef

define Package/valgrind-callgrind/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/callgrind* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/valgrind
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/callgrind-* \
		$(1)/usr/lib/valgrind/
	$(patsubst STRIP=%,STRIP="$(TARGET_CROSS)strip",$(RSTRIP)) $(PKG_INSTALL_DIR)/usr/bin
	$(patsubst STRIP=%,STRIP="$(TARGET_CROSS)strip -g",$(RSTRIP)) $(PKG_INSTALL_DIR)/usr/lib/valgrind/callgrind-*
endef

define Package/valgrind-drd/install
	$(INSTALL_DIR) $(1)/usr/lib/valgrind
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/drd-* \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/vgpreload_drd*.so \
		$(1)/usr/lib/valgrind/
	$(patsubst STRIP=%,STRIP="$(TARGET_CROSS)strip",$(RSTRIP)) $(PKG_INSTALL_DIR)/usr/bin
	$(patsubst STRIP=%,STRIP="$(TARGET_CROSS)strip -g",$(RSTRIP)) $(PKG_INSTALL_DIR)/usr/lib/valgrind/drd-*
endef

define Package/valgrind-massif/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ms_print $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/valgrind
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/massif-* \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/vgpreload_massif*.so \
		$(1)/usr/lib/valgrind/
	$(patsubst STRIP=%,STRIP="$(TARGET_CROSS)strip",$(RSTRIP)) $(PKG_INSTALL_DIR)/usr/bin
	$(patsubst STRIP=%,STRIP="$(TARGET_CROSS)strip -g",$(RSTRIP)) $(PKG_INSTALL_DIR)/usr/lib/valgrind/massif-*
endef

define Package/valgrind-helgrind/install
	$(INSTALL_DIR) $(1)/usr/lib/valgrind
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/helgrind-* \
		$(PKG_INSTALL_DIR)/usr/lib/valgrind/vgpreload_helgrind*.so \
		$(1)/usr/lib/valgrind/
	$(patsubst STRIP=%,STRIP="$(TARGET_CROSS)strip",$(RSTRIP)) $(PKG_INSTALL_DIR)/usr/bin
	$(patsubst STRIP=%,STRIP="$(TARGET_CROSS)strip -g",$(RSTRIP)) $(PKG_INSTALL_DIR)/usr/lib/valgrind/helgrind-*
endef

define Package/valgrind-vgdb/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vgdb $(1)/usr/bin/
	$(patsubst STRIP=%,STRIP="$(TARGET_CROSS)strip",$(RSTRIP)) $(PKG_INSTALL_DIR)/usr/bin
endef

$(eval $(call BuildPackage,valgrind))
$(eval $(call BuildPackage,valgrind-cachegrind))
$(eval $(call BuildPackage,valgrind-callgrind))
$(eval $(call BuildPackage,valgrind-drd))
$(eval $(call BuildPackage,valgrind-massif))
$(eval $(call BuildPackage,valgrind-helgrind))
$(eval $(call BuildPackage,valgrind-vgdb))
