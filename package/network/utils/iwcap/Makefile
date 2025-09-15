#
# Copyright (C) 2012 Jo-Philipp Wich <jo@mein.io>
#
# This is free software, licensed under the Apache 2 license.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iwcap
PKG_RELEASE:=1
PKG_LICENSE:=Apache-2.0

include $(INCLUDE_DIR)/package.mk


define Package/iwcap
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Simple radiotap capture utility
  MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
endef

define Package/iwcap/description
  The iwcap utility receives radiotap packet data from wifi monitor interfaces
  and outputs it to pcap format. It gathers recived packets in a fixed ring
  buffer to dump them on demand which is useful for background monitoring.
  Alternatively the utility can stream the data to stdout to act as remote
  capture drone for Wireshark or similar programs.
endef


define Build/Configure
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) \
		-o $(PKG_BUILD_DIR)/iwcap $(PKG_BUILD_DIR)/iwcap.c
endef


define Package/iwcap/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iwcap $(1)/usr/sbin/iwcap
endef

$(eval $(call BuildPackage,iwcap))
