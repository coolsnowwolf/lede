# Copyright (c) 2016, prpl Foundation
#
# Permission to use, copy, modify, and/or distribute this software for any purpose with or without
# fee is hereby granted, provided that the above copyright notice and this permission notice appear
# in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
# FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# Author: Nils Koenig <openwrt@newk.it> 

include $(TOPDIR)/rules.mk

PKG_NAME:=wifischedule
PKG_VERSION:=1
PKG_RELEASE:=3
PKG_LICENSE:=PRPL

PKG_MAINTAINER:=Nils Koenig <openwrt@newk.it> 

include $(INCLUDE_DIR)/package.mk

define Package/wifischedule
  SUBMENU:=Wireless
  TITLE:=Turns WiFi on and off according to a schedule
  SECTION:=net
  CATEGORY:=Network
  PKGARCH:=all
endef

define Package/wifischedule/description
Turns WiFi on and off according to a schedule defined in UCI.
endef

define Package/wifischedule/conffiles
/etc/config/wifi_schedule
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/wifischedule/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./net/usr/bin/wifi_schedule.sh $(1)/usr/bin/wifi_schedule.sh
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./net/etc/config/wifi_schedule $(1)/etc/config/wifi_schedule
endef

$(eval $(call BuildPackage,wifischedule))
