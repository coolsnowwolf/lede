#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=taskwarrior
PKG_VERSION:=2.5.3
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=task-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.taskwarrior.org/download
PKG_HASH:=7243d75e0911d9e2c9119ad94a61a87f041e4053e197f7280c42410aa1ee963b
PKG_BUILD_DIR:=$(BUILD_DIR)/task-$(PKG_VERSION)

PKG_MAINTAINER:=
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/taskwarrior
  TITLE:=taskwarrior
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libstdcpp +libuuid +libgnutls
  URL:=https://taskwarrior.org/
endef

define Package/taskwarrior/description
 taskwarrior is a command-line todo list manager
endef

define Package/taskwarrior/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/task $(1)/usr/bin/
endef

$(eval $(call BuildPackage,taskwarrior))
