#
# Copyright (C) 2008-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# Based on the initial porting done by el1n

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-softethervpn
PKG_VERSION:=4.30-9696
PKG_VERREL:=beta
PKG_VERDATE:=2019.07.08
PKG_RELEASE:=1

PKG_SOURCE:=softether-src-v$(PKG_VERSION)-$(PKG_VERREL).tar.gz
PKG_SOURCE_URL:=http://www.softether-download.com/files/softether/v$(PKG_VERSION)-$(PKG_VERREL)-$(PKG_VERDATE)-tree/Source_Code/
PKG_HASH:=07b6fb3a677ab9eef42fc5a6abee414904552bcdc66baa56ebd3896bf2a8b490

PKG_BUILD_DIR:=$(BUILD_DIR)/v$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/v$(PKG_VERSION)

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_DEPENDS:=$(PKG_NAME)/host

HAMCORE_SE2:=$(STAGING_DIR_HOST)/share/$(PKG_NAME)/hamcore.se2

include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

export USE_MUSL=YES

# Override CC to add fake libreadline to linker search path
HOSTCC += -L./src/readline

# Add defines to turn add_history() and readline() calls into no-ops
HOSTCC += -D'add_history(x)' -D'readline(x)=\"\"'

# Execute in host build directory
HOST_MAKE_FLAGS += -C $(HOST_BUILD_DIR)

# Select 32 or 64 bit Makefile for host build depending on host architecture
HOST_MAKE_FLAGS += -f src/makefiles/linux_$(if $(shell uname -m | grep 64),64,32)bit.mak

# Prevent calling upstream configure
define Host/Configure
endef

define Host/Compile
	# Prepare fake readline headers and library
	mkdir -p $(HOST_BUILD_DIR)/src/readline
	touch $(HOST_BUILD_DIR)/src/readline/readline.h
	touch $(HOST_BUILD_DIR)/src/readline/history.h
	ar rcs $(HOST_BUILD_DIR)/src/readline/libreadline.a

	# Build hamcorebuilder using host compiler and let it generate
	# the hamcore.se2 archive file
	CC="$(HOSTCC)" $(MAKE) $(HOST_MAKE_FLAGS) \
		src/bin/BuiltHamcoreFiles/unix/hamcore.se2
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/share/$(PKG_NAME)
	$(INSTALL_DATA) $(HOST_BUILD_DIR)/src/bin/BuiltHamcoreFiles/unix/hamcore.se2 $(HAMCORE_SE2)
endef


# Tune CFLAGS for target
TARGET_CFLAGS += \
	$(if $(CONFIG_OPENSSL_WITH_SSL3),,-DSSL_OP_NO_SSLv3) \
	$(if $(filter mips mipsel,$(ARCH)),-minterlink-mips16)

# Select 32 or 64 bit Makefile for target build depending on 64bit config symbol
MAKE_FLAGS += \
	-f src/makefiles/linux_$(if $(CONFIG_ARCH_64BIT),64,32)bit.mak

# Map nonstandard CCFLAGS variable to standard TARGET_CFLAGS
MAKE_FLAGS += \
	CC="$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) $(TARGET_LDFLAGS)"

define Build/Configure
	# Fetch prebuilt hamcore.se2 from staging dir
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/src/bin/BuiltHamcoreFiles/unix
	$(CP) $(HAMCORE_SE2) $(PKG_BUILD_DIR)/src/bin/BuiltHamcoreFiles/unix/hamcore.se2

	# Portably set hamcore.se2 modtime to one day in the future
	# to prevent rebuilding it
	perl -e 'utime(time() + 86400, time() + 86400, $$$$ARGV[0])' \
		$(PKG_BUILD_DIR)/src/bin/BuiltHamcoreFiles/unix/hamcore.se2
endef


define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
  DEPENDS:=+libpthread +librt +libreadline +libopenssl +libncurses +kmod-tun +zlib $(ICONV_DEPENDS)
  TITLE:=LuCI Support for softethervpn-server
  URL:=http://www.softether.org/
  MAINTAINER:=Federico Di Marco <fededim@gmail.com>
endef

define Package/$(PKG_NAME)/description
$(PKG_NAME)
endef


define Package/$(PKG_NAME)/conffiles
/usr/libexec/softethervpn/vpn_server.config
/usr/libexec/softethervpn/lang.config
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/libexec/softethervpn
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/vpnserver/vpnserver $(1)/usr/libexec/softethervpn
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/vpnserver/hamcore.se2 $(1)/usr/libexec/softethervpn
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/vpncmd/vpncmd $(1)/usr/libexec/softethervpn
	$(INSTALL_BIN) files/launcher.sh $(1)/usr/libexec/softethervpn

	$(INSTALL_DATA) files/dummy $(1)/usr/libexec/softethervpn/vpn_server.config
	$(INSTALL_DATA) files/dummy $(1)/usr/libexec/softethervpn/lang.config

	$(INSTALL_DIR) $(1)/usr/bin

	$(LN) ../../usr/libexec/softethervpn/launcher.sh $(1)/usr/bin/vpnserver
	$(LN) ../../usr/libexec/softethervpn/launcher.sh $(1)/usr/bin/vpncmd

	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/vpnserver.init $(1)/etc/init.d/softethervpn
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/
	cp -pR ./root/* $(1)/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/softether.po $(1)/usr/lib/lua/luci/i18n/softether.zh-cn.lmo
	
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call HostBuild))
