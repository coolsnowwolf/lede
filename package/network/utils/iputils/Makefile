#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iputils
PKG_VERSION:=20101006
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-s$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://www.skbuff.net/iputils
PKG_HASH:=fd3af46c80ebb99607c2ca1f2a3608b6fe828e25bbec6e54f2afd25f6ddb6ee7

PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=GPL-2.0+
PKG_CPE_ID:=cpe:/a:skbuff:iputils

PKG_BUILD_DEPENDS:=sysfsutils
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-s$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk


define Package/iputils/Default
  SECTION:=net
  CATEGORY:=Network
  URL:=http://www.skbuff.net/iputils
endef


define Package/iputils-arping
$(call Package/iputils/Default)
  TITLE:=iputils - arping
  DEPENDS+= +libsysfs
endef

define Package/iputils-arping/description
  Program arping from iputils.
  Sends ARP REQUEST to a neighbour host.
endef


define Package/iputils-clockdiff
$(call Package/iputils/Default)
  TITLE:=iputils - clockdiff
endef

define Package/iputils-clockdiff/description
  Program clockdiff from iputils.
  Measures clock difference between hosts.
endef


define Package/iputils-ping
$(call Package/iputils/Default)
  TITLE:=iputils - ping
endef

define Package/iputils-ping/description
  Program ping from iputils.
  Sends ICMP ECHO_REQUEST to network hosts (IPv4).
endef


define Package/iputils-ping6
$(call Package/iputils/Default)
  TITLE:=iputils - ping6
  DEPENDS+= @IPV6 +USE_GLIBC:libopenssl
endef

define Package/iputils-ping6/description
  Program ping6 from iputils.
  Sends ICMP ECHO_REQUEST to network hosts (IPv6).
endef


define Package/iputils-tftpd
$(call Package/iputils/Default)
  TITLE:=iputils - tftpd
endef

define Package/iputils-tftpd/description
  Program tftpd from iputils
  Trivial File Transfer Protocol server.
endef


define Package/iputils-tracepath
$(call Package/iputils/Default)
  TITLE:=iputils - tracepath
endef

define Package/iputils-tracepath/description
  Program tracepath from iputils.
  Traces path to a network host discovering MTU along this path (IPv4).
endef


define Package/iputils-tracepath6
$(call Package/iputils/Default)
  TITLE:=iputils - tracepath6
  DEPENDS+= @IPV6
endef

define Package/iputils-tracepath6/description
  Program tracepath6 from iputils.
  Traces path to a network host discovering MTU along this path (IPv6).
endef


define Package/iputils-traceroute6
$(call Package/iputils/Default)
  TITLE:=iputils - traceroute6
  DEPENDS+= @IPV6
endef

define Package/iputils-traceroute6/description
  Program traceroute6 from iputils.
  Traces path to a network host (IPv6).
endef

ifdef CONFIG_USE_MUSL
  TARGET_CFLAGS += -D__UCLIBC__
endif

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS) $(TARGET_LDFLAGS)" \
	CONFIG_IPV6="$(CONFIG_IPV6)" \
	CONFIG_USE_UCLIBC="$(CONFIG_USE_UCLIBC)$(CONFIG_USE_MUSL)" \

define Package/iputils-arping/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/arping $(1)/usr/bin/
endef

define Package/iputils-clockdiff/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/clockdiff $(1)/usr/bin/
endef

define Package/iputils-ping/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ping $(1)/usr/bin/
endef

define Package/iputils-ping6/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ping6 $(1)/usr/bin/
endef

define Package/iputils-tftpd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tftpd $(1)/usr/sbin/
endef

define Package/iputils-tracepath/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tracepath $(1)/usr/bin/
endef

define Package/iputils-tracepath6/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tracepath6 $(1)/usr/bin/
endef

define Package/iputils-traceroute6/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/traceroute6 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,iputils-arping))
$(eval $(call BuildPackage,iputils-clockdiff))
$(eval $(call BuildPackage,iputils-ping))
$(eval $(call BuildPackage,iputils-tftpd))
$(eval $(call BuildPackage,iputils-tracepath))
$(eval $(call BuildPackage,iputils-ping6))
$(eval $(call BuildPackage,iputils-tracepath6))
$(eval $(call BuildPackage,iputils-traceroute6))
