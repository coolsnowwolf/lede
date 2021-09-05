#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openocd
PKG_SOURCE_VERSION:=v0.11.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://git.code.sf.net/p/openocd/code
PKG_MIRROR_HASH:=ec31ce908d951c6c7e173c7474269f3f105d7a7b3991afbe8f54f8d96646c725

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_MAINTAINER:=Paul Fertser <fercerpav@gmail.com>
PKG_CPE_ID:=cpe:/a:openocd:open_on-chip_debugger

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

PKG_CONFIG_DEPENDS:=CONFIG_PACKAGE_openocd_with_usb

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/openocd
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=OpenOCD Utility
  URL:=http://openocd.sf.net/
  DEPENDS:=+libusb-1.0 \
	+libusb-compat \
	+libftdi1 \
	+hidapi \
	+libgpiod
endef

define Package/openocd/description
OpenOCD provides on-chip programming and debugging support with a
layered architecture of JTAG interface and TAP support including:

- (X)SVF playback to facilitate automated boundary scan and FPGA/CPLD
  programming;
- debug target support (e.g. ARM, MIPS): single-stepping,
  breakpoints/watchpoints, gprof profiling, etc;
- flash chip drivers (e.g. CFI, NAND, internal flash);
- embedded TCL interpreter for easy scripting.

Several network interfaces are available for interacting with OpenOCD:
telnet, TCL, and GDB. The GDB server enables OpenOCD to function as a
"remote target" for source-level debugging of embedded systems using
the GNU GDB program (and the others who talk GDB protocol, e.g. IDA
Pro).
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	-$(RM) $(PKG_BUILD_DIR)/guess-rev.sh
endef

CONFIGURE_ARGS += \
	--prefix="/usr" \
	--disable-werror \
	MAKEINFO=true \
	--enable-dummy \
	--enable-sysfsgpio \
	--enable-linuxgpiod

TARGET_CFLAGS += -DRELSTR=\\\"-$(PKG_VERSION)-$(PKG_RELEASE)-OpenWrt\\\"

define Build/Compile
        +$(MAKE_VARS) \
        $(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/$(MAKE_PATH)
endef

define Package/openocd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/share/openocd
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/openocd $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/openocd/scripts $(1)/usr/share/openocd
endef

$(eval $(call BuildPackage,openocd))
