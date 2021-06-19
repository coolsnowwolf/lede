#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libpcap
PKG_VERSION:=1.10.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.us.tcpdump.org/release/ \
        http://www.tcpdump.org/release/
PKG_HASH:=ed285f4accaf05344f90975757b3dbfe772ba41d1c401c2648b7fa45b711bdd4

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

PKG_ASLR_PIE_REGULAR:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libpcap
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Low-level packet capture library
  URL:=http://www.tcpdump.org/
  MENU:=1
  ABI_VERSION:=1
endef

define Package/libpcap/description
This package contains a system-independent library for user-level network packet
capture.
endef

define Package/libpcap/config
	source "$(SOURCE)/Config.in"
endef

define Package/rpcapd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Capture daemon to be controlled by a remote libpcap application
  URL:=http://www.tcpdump.org/
  DEPENDS+= +libpcap
endef

ifdef CONFIG_PACKAGE_rpcapd
	CMAKE_OPTIONS += \
		-DENABLE_REMOTE=ON
endif

CMAKE_OPTIONS += \
	-DBUILD_SHARED_LIBS=ON \
	-DBUILD_WITH_LIBNL=OFF \
	-DINET6=O$(if $(CONFIG_IPV6),N,FF) \
	-DPCAP_SUPPORT_NETFILTER=O$(if $(CONFIG_PCAP_HAS_NETFILTER),N,FF)

# grep 'option(DISABLE_' CMakeLists.txt | cut -f2 -d'(' | cut -f1 -d' ' | sort --unique
CMAKE_OPTIONS += \
	-DDISABLE_BLUETOOTH=O$(if $(CONFIG_PCAP_HAS_BT),FF,N) \
	-DDISABLE_DAG=ON \
	-DDISABLE_DBUS=ON \
	-DDISABLE_DPDK=ON \
	-DDISABLE_LINUX_USBMON=O$(if $(CONFIG_PCAP_HAS_USB),FF,N) \
	-DDISABLE_NETMAP=ON \
	-DDISABLE_RDMA=ON \
	-DDISABLE_SEPTEL=ON \
	-DDISABLE_SNF=ON \
	-DDISABLE_TC=ON \

# Debugging options
CMAKE_OPTIONS += \
	-DBDEBUG=OFF \
	-DYYDEBUG=OFF \

define Build/InstallDev
	$(call Build/InstallDev/cmake,$(1))
	$(SED) \
		's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' \
		$(1)/usr/bin/pcap-config
	$(INSTALL_DIR) $(2)/bin
	$(LN) ../../usr/bin/pcap-config $(2)/bin/pcap-config
endef

define Package/libpcap/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpcap.so.* $(1)/usr/lib/
endef

define Package/rpcapd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/rpcapd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,libpcap))
$(eval $(call BuildPackage,rpcapd))
