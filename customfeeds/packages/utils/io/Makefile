#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=io
PKG_RELEASE:=4

include $(INCLUDE_DIR)/package.mk

PKG_MAINTAINER:=Hauke Mehrtens <hauke@hauke-m.de>

define Package/io
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Raw memory i/o utility
  URL:=http://www.sleepie.demon.co.uk/linuxvme/io.c
endef

define Package/io/description
	This tool can be used to access physical memory addresses from userspace.
	It can be useful to access hardware for which no device driver exists!
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -Wall \
		$(PKG_BUILD_DIR)/io.c -o $(PKG_BUILD_DIR)/$(PKG_NAME)
endef

define Package/io/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/io $(1)/usr/bin/
endef

$(eval $(call BuildPackage,io))
