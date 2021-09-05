# Copyright (C) 2019-2020 Jerônimo Cordoni Pellegrini <j_p@aleph0.info>
#
# This file is free software, licensed under the GNU General Public License v3
# or later.
# See /LICENSE for details
#

include $(TOPDIR)/rules.mk

PKG_NAME:=chicken-scheme
PKG_VERSION:=5.2.0
PKG_RELEASE:=2

PKG_BUILD_DIR:=$(BUILD_DIR)/chicken-$(PKG_VERSION)
PKG_SOURCE:=chicken-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://code.call-cc.org/releases/$(PKG_VERSION)/
PKG_HASH:=819149c8ce7303a9b381d3fdc1d5765c5f9ac4dee6f627d1652f47966a8780fa
PKG_MAINTAINER:=Jeronimo Pellegrini <j_p@aleph0.info>

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk




define Package/chicken-scheme/Default
  SECTION:=lang
  CATEGORY:=Languages
  URL:=https://call-cc.org
  ABI_VERSION:=11
endef


##
## chicken-scheme-interpreter
##

define Package/chicken-scheme-interpreter
$(call Package/chicken-scheme/Default)
  TITLE:=Chicken Scheme -- interpreter only
  SECTION:=lang
  CATEGORY:=Languages
endef

define Package/chicken-scheme-interpreter/description
  Chicken is an implementation of the Scheme language. It is portable, efficient, and supports 
  the R5RS and R7RS (work in progress) standards, and many extensions.
  Chicken can be used as a scripting language to automate tasks.
  This package contains the interpreter, 'csi', only --
  the compiler and the package installer are not included because they depend on a C compiler.
  For more information, please refer to the Chicken Scheme website at https://call-cc.org.
endef

MAKE_FLAGS += PLATFORM=linux C_COMPILER=$(TARGET_CC) LINKER=$(TARGET_CC) PREFIX=/usr C_COMPILER_OPTIMIZATION_OPTIONS="$(TARGET_CFLAGS)"

# not installed:
# - csc and chicken, the compiler
# - the include dir (only useful with the compiler)
# - install, uninstall, status, and chicken-do, which deal with modules
#   (installation of more modules depends on the compiler)
# - profiler
# - feathers, the debugger
# - libchicken.a, the static library
define Package/chicken-scheme-interpreter/install
	$(INSTALL_DIR)  $(1)/usr/bin
	$(INSTALL_DIR)  $(1)/usr/lib/chicken/$(ABI_VERSION)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/csi                 $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/libchicken.so.$(ABI_VERSION) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libchicken.so                $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/*.import.so                  $(1)/usr/lib/chicken/$(ABI_VERSION)/
	$(CP) $(PKG_BUILD_DIR)/types.db                     $(1)/usr/lib/chicken/$(ABI_VERSION)/
endef

$(eval $(call BuildPackage,chicken-scheme-interpreter))



##
## chicken-scheme-full
##

# mips-openwrt-linux-musl-gcc dowsn't seem to link with libchicken.so.
# already tried -lchicken and passing /usr/lib/libchicken.so on the command
# line.
# also tried -Wl,-R/usr/lib,-R/usr/lib/chicken/$(ABI_VERSION)
#
# current solution: we do not strip binaries in chicken-scheme-full
# we do this by unsetting STRIP and RSTRIP, *and* reloading rules.mk
#
STRIP:=:
RSTRIP:=:
include $(TOPDIR)/rules.mk

define Package/chicken-scheme-full
$(call Package/chicken-scheme/Default)
  TITLE:=Chicken Scheme -- full package
  SECTION:=lang
  CATEGORY:=Languages
  # csc depends on gcc; chicken-install uses the 'install' command from coreutils
  EXTRA_DEPENDS:= gcc, coreutils-install
endef

define Package/chicken-scheme-full/description
  Chicken is an implementation of the Scheme language. It is portable, efficient, and supports 
  the R5RS and R7RS (work in progress) standards, and many extensions.
  Chicken can be used as a scripting language to automate tasks.
  This package contains the interpreter, 'csi'; the compiler, 'csc';
  the tools for installing and removing eggs (modules); the profiler and
  the debugger.
  Note that this package depends on gcc, which is quite large (more than 100Mb).
  For more information, please refer to the Chicken Scheme website at https://call-cc.org.
endef


# not installed:
# - libchicken.a, the static library
define Package/chicken-scheme-full/install
	$(INSTALL_DIR)  $(1)/usr/bin
	$(INSTALL_DIR)  $(1)/usr/lib/chicken/$(ABI_VERSION)
	$(INSTALL_DIR)  $(1)/usr/include/chicken
	$(INSTALL_DIR)  $(1)/usr/share/chicken
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/csi                 $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/libchicken.so.$(ABI_VERSION) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libchicken.so                $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/*.import.so                  $(1)/usr/lib/chicken/$(ABI_VERSION)/
	$(CP) $(PKG_BUILD_DIR)/types.db                     $(1)/usr/lib/chicken/$(ABI_VERSION)/
	# csc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/csc     		$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/chicken 		$(1)/usr/bin/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/chicken.h		$(1)/usr/include/chicken/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/chicken-config.h	$(1)/usr/include/chicken/
	# chicken-install
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/chicken-install		$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/chicken-uninstall	$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/chicken-status		$(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/chicken-do		$(1)/usr/bin/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/setup.defaults		$(1)/usr/share/chicken/
	# profile
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/chicken-profile		$(1)/usr/bin/
	# feathers
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/feathers.tcl		$(1)/usr/share/chicken/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/feathers		$(1)/usr/bin/
endef

$(eval $(call BuildPackage,chicken-scheme-full))
