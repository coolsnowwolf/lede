#
# Copyright (C) 2016-2020 eGloo, Incorporated
#
# This is free software, licensed under the GNU General Public License v2.

include $(TOPDIR)/rules.mk

PKG_NAME:=netifyd
PKG_RELEASE:=2
PKG_MAINTAINER:=Darryl Sokoloski <darryl@egloo.ca>
PKG_LICENSE:=GPL-3.0-or-later

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://gitlab.com/netify.ai/public/netify-agent.git
PKG_SOURCE_DATE:=2021-05-19
PKG_SOURCE_VERSION:=v3.07
#PKG_SOURCE_VERSION:=a22c66b9d916347b34f6d26de2a95c94f446401b
PKG_MIRROR_HASH:=de6c4ce7bb00ec72478a7eeaa44b1a30d2ef64202f8524a000bce6015441a5ca

include $(INCLUDE_DIR)/package.mk

define Package/netifyd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Netify Agent
  URL:=http://www.netify.ai/
  DEPENDS:=+ca-bundle +libcurl +libmnl +libnetfilter-conntrack +libpcap +zlib +libpthread @!USE_UCLIBC
  # Explicitly depend on libstdcpp rather than $(CXX_DEPENDS).  At the moment
  # std::unordered_map is only available via libstdcpp which is required for
  # performance reasons.
  DEPENDS+=+libstdcpp
endef

define Package/netifyd/description
The Netify Agent is a deep-packet inspection server which detects network
protocols and applications.  These detections can be saved locally, served over
a UNIX or TCP socket, and/or "pushed" (via HTTP POSTs) to a remote third-party
server.  Flow metadata, network statistics, and detection classifications are
JSON encoded for easy consumption by third-party applications.
endef

define Package/netifyd/conffiles
/etc/netifyd.conf
/etc/config/netifyd
/etc/netify.d/agent.uuid
/etc/netify.d/serial.uuid
/etc/netify.d/site.uuid
endef

TARGET_CFLAGS+=-ffunction-sections -fdata-sections -Wno-psabi
TARGET_CXXFLAGS+=-ffunction-sections -fdata-sections -Wno-psabi
TARGET_LDFLAGS+=-Wl,--gc-sections

CONFIGURE_ARGS+= \
	--sharedstatedir=/var/run \
	--enable-lean-and-mean \
	--disable-libtcmalloc \
	--without-systemdsystemunitdir \
	--without-tmpfilesdir

# Disable configuration file-watch support (deprecated feature).
# Not to be confused with kernel/system-level inotify support.
CONFIGURE_ARGS+= \
	--disable-inotify

ifneq ($(CONFIG_LIBCURL_ZLIB),y)
CONFIGURE_ARGS+= \
	--without-libcurl-zlib
endif

define Build/Configure
	(cd $(PKG_BUILD_DIR); ./autogen.sh)
	$(call Build/Configure/Default,$(CONFIGURE_ARGS))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/netifyd
	$(CP) $(PKG_INSTALL_DIR)/usr/include/netifyd/*.h $(1)/usr/include/netifyd
	$(INSTALL_DIR) $(1)/usr/include/netifyd/pcap-compat
	$(CP) $(PKG_INSTALL_DIR)/usr/include/netifyd/pcap-compat/*.h $(1)/usr/include/netifyd/pcap-compat
	$(INSTALL_DIR) $(1)/usr/include/netifyd/nlohmann
	$(CP) $(PKG_INSTALL_DIR)/usr/include/netifyd/nlohmann/*.hpp $(1)/usr/include/netifyd/nlohmann
	$(INSTALL_DIR) $(1)/usr/include/libndpi-2.9.0
	$(INSTALL_DIR) $(1)/usr/include/libndpi-2.9.0/libndpi
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libndpi-2.9.0/libndpi/*.h $(1)/usr/include/libndpi-2.9.0/libndpi
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnetifyd.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libnetifyd.pc $(1)/usr/lib/pkgconfig/
endef

define Package/netifyd/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/deploy/netifyd.conf $(1)/etc
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/netifyd.config $(1)/etc/config/netifyd
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/netifyd.init $(1)/etc/init.d/netifyd
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/netifyd $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnetifyd.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc/netify.d
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/deploy/netify-sink.conf $(1)/etc/netify.d/netify-sink.conf
	$(INSTALL_DIR) $(1)/usr/share/netifyd
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/deploy/functions.sh $(1)/usr/share/netifyd
endef

$(eval $(call BuildPackage,netifyd))
