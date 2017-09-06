#
# Copyright (C) 2006-2012 OpenWrt.org
# Copyright (c) 2016 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uclibc++
PKG_VERSION:=0.2.4
PKG_RELEASE:=3

PKG_SOURCE:=uClibc++-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://cxx.uclibc.org/src/
PKG_HASH:=be16fc6a6eb2e59df420ee41e7042f38e27ebaf5c2762d90e75803364a7b00db

PKG_BUILD_DIR:=$(BUILD_DIR)/uClibc++-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0
PKG_LICENSE:=LGPL-2.1+

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/uclibcxx
  NAME:=uclibc++
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=C++ library for embedded systems
  URL:=http://cxx.uclibc.org/src/
endef

UCLIBC_TARGET_ARCH:=$(shell echo $(ARCH) | sed -e s'/-.*//' \
	-e 's/i.86/i386/' \
	-e 's/sparc.*/sparc/' \
	-e 's/m68k.*/m68k/' \
	-e 's/ppc/powerpc/g' \
	-e 's/v850.*/v850/g' \
	-e 's/sh64/sh/' \
	-e 's/sh[234].*/sh/' \
	-e 's/mips.*/mips/' \
	-e 's/mipsel.*/mips/' \
)

TARGET_CFLAGS += $(FPIC)

ifneq ($(CONFIG_CCACHE),)
TARGET_CXX=$(TARGET_CXX_NOCACHE)
endif

ifeq ($(CONFIG_USE_MUSL),y)
SSP_LIB=-lssp_nonshared
endif

MAKE_FLAGS:= \
	TOPDIR="$(PKG_BUILD_DIR)/" \
	$(TARGET_CONFIGURE_OPTS) \
	CPU_CFLAGS="$(TARGET_CFLAGS)" \
	CROSS="$(TARGET_CROSS)" \
	LDFLAGS="-Wl,--warn-common -Wl,--warn-once -Wl,-z,combreloc -Wl,-z,defs $(TARGET_LDFLAGS)" \
	CP="$(CP)" \
	GEN_LIBS="-lc $(LIBGCC_S) $(SSP_LIB)" \
	GEN_CFLAGS="-std=gnu++98 -nostdinc++" \
	check_as_needed=

# check_as_needed overrides dependency on libgcc_s

define Build/Prepare
	$(PKG_UNPACK)
	$(SED) 's/\r$$$$//' $(PKG_BUILD_DIR)/include/unwind-cxx.h
	$(Build/Patch)
endef

define Build/Configure
	if [ -f ./files/config.$(UCLIBC_TARGET_ARCH) ]; then \
		cp ./files/config.$(UCLIBC_TARGET_ARCH) $(PKG_BUILD_DIR)/.config; \
	else \
		cp ./files/config.default $(PKG_BUILD_DIR)/.config; \
	fi
endef

define Build/InstallDev
	$(INSTALL_DIR) $(2)/bin $(1)/usr/include/uClibc++ $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/g++-uc $(TOOLCHAIN_DIR)/bin/
	$(CP) $(PKG_INSTALL_DIR)/include/* $(1)/usr/include/uClibc++/
	$(CP) $(PKG_INSTALL_DIR)/lib/libuClibc++*.{a,so}* $(1)/usr/lib/
	$(SED) 's!\(^\|[[:space:]]\)-[IL]$(TOOLCHAIN_DIR)/[^[:space:]]*!!g' $(TOOLCHAIN_DIR)/bin/g++-uc
	$(SED) 's|-I/include/|-I$$$${STAGING_DIR:-$(STAGING_DIR)}/usr/include/uClibc++/|g' $(TOOLCHAIN_DIR)/bin/g++-uc
	$(SED) 's|-L/lib/|-L$$$${STAGING_DIR:-$(STAGING_DIR)}/lib/|g' $(TOOLCHAIN_DIR)/bin/g++-uc
# add another wrapper which links against both uClibc++ and libstdc++
	$(INSTALL_BIN) $(TOOLCHAIN_DIR)/bin/g++-uc $(TOOLCHAIN_DIR)/bin/g++-uc+std
	$(SED) 's|^WRAPPER_INCLUDEDIR=.*||g' $(TOOLCHAIN_DIR)/bin/g++-uc+std
	$(SED) 's|-luClibc++|-Wl,-Bdynamic,-luClibc++,-Bstatic,-lstdc++,-Bdynamic|g' $(TOOLCHAIN_DIR)/bin/g++-uc+std
	$(SED) 's|-nostdinc++||g' $(TOOLCHAIN_DIR)/bin/g++-uc+std
endef

define Package/uclibcxx/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/libuClibc++.so.* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/lib/libuClibc++-*.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,uclibcxx))
