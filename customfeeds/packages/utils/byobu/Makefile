#
# Copyright (C) 2009 Sharim At Work, Ltd.
# Copyright (C) 2019 Jeffery To
# Author: Xiangfu Liu <xiangfu@sharism.cc>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=byobu
PKG_VERSION:=5.133
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_SOURCE_URL:=https://launchpad.net/byobu/trunk/$(PKG_VERSION)/+download
PKG_HASH:=4d8ea48f8c059e56f7174df89b04a08c32286bae5a21562c5c6f61be6dab7563

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/byobu/Default
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=http://byobu.org/
  PKGARCH:=all
endef

define Package/byobu
$(call Package/byobu/Default)
  TITLE:=Text-based window manager and terminal multiplexer
  DEPENDS:=+python3-light +python3-newt
endef

define Package/byobu-utils
$(call Package/byobu/Default)
  TITLE:=Utilities included with byobu
endef

define Package/byobu/description
  Byobu is a GPLv3 open source text-based window manager and terminal
  multiplexer. It was originally designed to provide elegant enhancements
  to the otherwise functional, plain, practical GNU Screen, for the Ubuntu
  server distribution. Byobu now includes an enhanced profiles, convenient
  keybindings, configuration utilities, and toggle-able system status
  notifications for both the GNU Screen window manager and the more modern
  Tmux terminal multiplexer, and works on most Linux, BSD, and Mac
  distributions.
endef

define Package/byobu-utils/description
$(call Package/byobu/description)

  These are the "bikeshed" utilities bundled with byobu:
  * col1..col9, NF
  * ctail
  * manifest
  * purge-old-kernels
  * vigpg
  * wifi-status
endef

define Package/byobu/install
	$(INSTALL_DIR) $(1)/etc
	$(CP) $(PKG_INSTALL_DIR)/etc/* $(1)/etc/

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/byobu* $(1)/usr/bin/
	$(LN) byobu $(1)/usr/bin/byobu-screen
	$(LN) byobu $(1)/usr/bin/byobu-tmux

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/byobu $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/share/byobu
	for i in keybindings profiles status windows; do \
		$(CP) $(PKG_INSTALL_DIR)/usr/share/byobu/$$$$i $(1)/usr/share/byobu/ ; \
	done

	$(INSTALL_DIR) $(1)/usr/share/doc
	$(CP) $(PKG_INSTALL_DIR)/usr/share/doc/byobu $(1)/usr/share/doc/
endef

define Package/byobu-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	for i in col1 ctail manifest purge-old-kernels vigpg wifi-status; do \
		$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/$$$$i $(1)/usr/bin/ ; \
	done
	for i in col2 col3 col4 col5 col6 col7 col8 col9 NF; do \
		$(LN) col1 $(1)/usr/bin/$$$$i ; \
	done
endef

$(eval $(call BuildPackage,byobu))
$(eval $(call BuildPackage,byobu-utils))
