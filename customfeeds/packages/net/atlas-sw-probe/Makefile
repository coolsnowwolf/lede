#
# Copyright (C) 2019-2021 CZ.NIC z.s.p.o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=atlas-sw-probe
PKG_VERSION:=5020
PKG_RELEASE:=3

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/RIPE-NCC/ripe-atlas-software-probe.git
PKG_SOURCE:=ripe-atlas-software-probe-$(PKG_VERSION).tar.gz

PKG_MIRROR_HASH:=846aa20ff4bc938c07526a9893dcae4ac7dfa41982a5b2bcfe2dd53c974ecdc9
PKG_SOURCE_VERSION:=edee49c942b726a1d8865d91c8d7f32843bc8ad1

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=LICENSE

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/atlas-sw-probe
  SECTION:=net
  CATEGORY:=Network
  TITLE:=RIPE Atlas software probe
  URL:=https://atlas.ripe.net/about/probes/
  DEPENDS:=+atlas-probe +PACKAGE_dropbear:dropbearconvert
endef

define Package/atlas-sw-probe/description
  RIPE Atlas SW probe is software variant of RIPE Atlas Probe.
  It contains utilities which helps actively measure
  Internet connectivity through ping, traceroute, DNS, SSL/TLS, NTP, and HTTP.
  Data are then collected, aggregated and published by the RIPE NCC.
endef

define Package/atlas-sw-probe-rpc
  SECTION:=net
  CATEGORY:=Network
  TITLE:=RPC service
  URL:=https://atlas.ripe.net/about/probes/
  DEPENDS:=+atlas-probe +bind-dig +rpcd
endef

define Package/atlas-sw-probe-rpc/description
  Provides ubus calls for probe.
endef

Build/Compile:=:
Build/Install:=:

define Package/atlas-sw-probe-rpc/postinst
#!/bin/sh
[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/rpcd restart
exit 0
endef

define Package/atlas-sw-probe-rpc/postrm
#!/bin/sh
[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/rpcd restart
exit 0
endef

define Package/atlas-sw-probe/conffiles
/etc/atlas/
/etc/config/atlas
/usr/libexec/atlas-probe-scripts/state/config.txt
endef

TMP_BASE_DIR:=/tmp/ripe_atlas_probe
SCRIPTS_DIR:=/usr/libexec/atlas-probe-scripts

define Package/atlas-sw-probe/install
	$(INSTALL_DIR) $(1)/$(SCRIPTS_DIR)
	$(INSTALL_DIR) $(1)/$(SCRIPTS_DIR)/{etc,state,bin/arch,bin/bin}

	# Copy config
	$(CP) $(PKG_BUILD_DIR)/atlas-config/etc/* $(1)/$(SCRIPTS_DIR)/etc/

	# Copy firmware version
	$(CP) $(PKG_BUILD_DIR)/atlas-config/state/FIRMWARE_APPS_VERSION $(1)/$(SCRIPTS_DIR)/state/

	# Set probe mode
	echo "prod" > $(1)/$(SCRIPTS_DIR)/state/mode

	# Copy scripts
	$(CP) $(PKG_BUILD_DIR)/bin/{ATLAS,common-pre.sh,common.sh,reginit.sh,resolvconf} $(1)/$(SCRIPTS_DIR)/bin/
	$(CP) $(PKG_BUILD_DIR)/bin/arch/{linux,openwrt-sw-probe} $(1)/$(SCRIPTS_DIR)/bin/arch/

	# Create config info
	echo "DEVICE_NAME=openwrt-sw-probe" > $(1)/$(SCRIPTS_DIR)/bin/config.sh
	echo "ATLAS_BASE=$(SCRIPTS_DIR)" >> $(1)/$(SCRIPTS_DIR)/bin/config.sh
	echo "ATLAS_STATIC=$(SCRIPTS_DIR)" >> $(1)/$(SCRIPTS_DIR)/bin/config.sh
	echo "SUB_ARCH=openwrt-$(ARCH)-$(PKG_VERSION)-$(PKG_RELEASE)" >> $(1)/$(SCRIPTS_DIR)/bin/bin/config.sh

	# Enable sending interface traffic statistics as Atlas measurement results
	echo "RXTXRPT=yes" > $(1)/$(SCRIPTS_DIR)/state/config.txt

	# Fix permision
	chmod 755 $(1)/$(SCRIPTS_DIR)/bin

	# Add registration instruction
	$(INSTALL_DIR) $(1)/etc/atlas/
	$(CP) ./files/atlas.readme $(1)/etc/atlas/

	# Create softlinks for writable dirs
	$(LN) $(TMP_BASE_DIR)/crons $(1)/$(SCRIPTS_DIR)/crons
	$(LN) $(TMP_BASE_DIR)/data $(1)/$(SCRIPTS_DIR)/data
	$(LN) $(TMP_BASE_DIR)/run $(1)/$(SCRIPTS_DIR)/run
	$(LN) $(TMP_BASE_DIR)/status $(1)/$(SCRIPTS_DIR)/status

	# Copy init and config
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/atlas.init $(1)/etc/init.d/atlas

	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_CONF) ./files/atlas.conf $(1)/etc/config/atlas
endef

define Package/atlas-sw-probe-rpc/install
	$(INSTALL_DIR) $(1)/usr/libexec/rpcd
	$(INSTALL_BIN) ./files/atlas_rpcd.sh $(1)/usr/libexec/rpcd/atlas
endef

$(eval $(call BuildPackage,atlas-sw-probe))
$(eval $(call BuildPackage,atlas-sw-probe-rpc))
