include $(TOPDIR)/rules.mk

PKG_NAME:=qos-gargoyle
PKG_VERSION:=1.0.1
PKG_RELEASE:=1
PKG_LICENSE:=GPL-3.0+
PKG_MAINTAINER:=Xingwang Liao <kuoruan@gmail.com>

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/qos-gargoyle
	SECTION:=net
	CATEGORY:=Gargoyle
	SUBMENU:=Network
	TITLE:=A set of QoS scripts
	DEPENDS:=+tc +ip +kmod-sched +iptables-mod-filter +iptables-mod-ipopt +iptables-mod-imq
	MAINTAINER:=Xingwang Liao <kuoruan@gmail.com>
	PKGARCH:=all
endef

define Package/qos-gargoyle/description
	A set of QoS scripts from Gargoyle
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(call Build/Prepare/Default)
endef

define Build/Configure
endef

define Build/Compile
	-$(MAKE) -C $(PKG_BUILD_DIR) clean
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -I $(STAGING_DIR)/usr/include" \
		LDFLAGS="$(TARGET_LDFLAGS) -L $(STAGING_DIR)/usr/lib" \
		BUILD_DIR="$(BUILD_DIR)" \
		all
endef

define Package/qos-gargoyle/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DIR) $(1)/etc/config

	$(INSTALL_BIN) ./files/qos_gargoyle.init $(1)/etc/init.d/qos_gargoyle
	$(INSTALL_BIN) ./files/qos_gargoyle.hotplug $(1)/etc/hotplug.d/iface/23-qos_gargoyle
	$(INSTALL_DATA) ./files/qos_gargoyle.conf $(1)/etc/config/qos_gargoyle

	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qosmon $(1)/usr/sbin/qosmon
endef

define Package/qos-gargoyle/prerm
	#!/bin/sh
	$${IPKG_INSTROOT}/etc/init.d/qos_gargoyle stop 2>/dev/null
	$${IPKG_INSTROOT}/etc/init.d/qos_gargoyle disable 2>/dev/null
	exit 0
endef

$(eval $(call BuildPackage,qos-gargoyle))
