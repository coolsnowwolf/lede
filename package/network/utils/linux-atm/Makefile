#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=linux-atm
PKG_VERSION:=2.5.2
PKG_RELEASE:=5

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/$(PKG_NAME)
PKG_HASH:=9645481a2b16476b59220aa2d6bc5bc41043f291326c9b37581018fbd16dd53a

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0+
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

ATM_DEBUG_BINS:=aread awrite atmdiag atmdump atmswitch saaldump \
		sonetdiag svc_recv svc_send ttcp_atm
ATM_DEBUG_SBINS:=atmaddr atmloop atmtcp esi atmsigd bus \
		 ilmid ilmidiag lecs les mpcd zeppelin
ATM_DEBUG_TOOLS:=$(ATM_DEBUG_BINS) $(ATM_DEBUG_SBINS)

define Package/linux-atm
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Linux ATM library
  URL:=http://linux-atm.sourceforge.net/
endef

define Package/linux-atm/description
  This package contains a library for accessing the Linux ATM subsystem.
endef

define Package/linux-atm/Default
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+linux-atm
  URL:=http://linux-atm.sourceforge.net/
  SUBMENU:=Linux ATM tools
endef

define Package/atm-tools
  $(call Package/linux-atm/Default)
  TITLE:=Linux ATM tools
endef

define Package/atm-tools/description
  This package contains the Linux ATM tools.
endef

define Package/atm-diagnostics
  $(call Package/linux-atm/Default)
  TITLE:=Linux ATM Diagnostics
endef

define Package/atm-diagnostics/description
  This package contains the Linux ATM diagnostics.
endef

define Package/atm-debug-tools
  $(call Package/linux-atm/Default)
  TITLE:=Linux ATM debugging tools
endef

define Package/atm-debug-tools/description
  This package contains the Linux ATM debugging tools.
endef

define Package/br2684ctl
  $(call Package/linux-atm/Default)
  TITLE:=ATM Ethernet bridging configuration utility
endef

define Package/br2684ctl/description
  Support for AAL5 encapsulation (RFC-1483/RFC-2684) over ATM.
endef

define GenAtmPlugin
  define Package/$(1)
     $(call Package/linux-atm/Default)
     TITLE:=Linux ATM tool $(2)
  endef

  define Package/$(1)/description
     Linux ATM tool $(2).
  endef
endef

$(foreach t,$(ATM_DEBUG_TOOLS),$(eval $(call GenAtmPlugin,atm-$(t),$(t))))

define Build/Configure
	$(call Build/Configure/Default)
	# prevent autoheader invocation
	touch $(PKG_BUILD_DIR)/stamp-h.in
endef

unexport PREFIX

define Build/Compile
	# src/qgen is built with HOSTCC, which does not really like our LDFLAGS
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/src/qgen \
		LDFLAGS="" \
		all
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) OBJCOPY=$(TARGET_CROSS)objcopy all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include \
		$(PKG_INSTALL_DIR)/usr/lib \
		$(1)/usr/
endef

define Package/linux-atm/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libatm.so* $(1)/usr/lib/
endef

define Package/atm-tools/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/atmarp{,d} $(1)/usr/sbin/
endef


define BuildAtmPlugin
  define Package/$(1)/install
	$(INSTALL_DIR) $$(1)/usr/$(3)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/$(3)/$(2) $$(1)/usr/$(3)
  endef

  $$(eval $$(call BuildPackage,$(1)))
endef

define Package/atm-debug-tools/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/atmaddr $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/atmloop $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/atmtcp     $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/esi $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/aread $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/awrite $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/atmdiag $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/atmdump $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/atmsigd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/bus $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/ilmid $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/ilmidiag $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/lecs $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/les $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/mpcd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/zeppelin $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/atmswitch $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/saaldump $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/sonetdiag $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/svc_recv $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/svc_send $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ttcp_atm $(1)/usr/bin/
endef

define Package/atm-diagnostics/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/aread $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/awrite $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/atmdiag $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/atmdump $(1)/usr/bin/
endef

define Package/br2684ctl/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/etc/hotplug.d/atm $(1)/usr/sbin $(1)/lib/netifd
	$(INSTALL_BIN) ./files/br2684-up $(1)/lib/netifd/br2684-up
	$(INSTALL_BIN) ./files/br2684ctl $(1)/etc/init.d/
	$(INSTALL_DATA) ./files/atm.hotplug $(1)/etc/hotplug.d/atm/00-trigger
	$(INSTALL_BIN) \
		./files/br2684ctl_wrap \
		$(PKG_INSTALL_DIR)/usr/sbin/br2684ctl \
		$(1)/usr/sbin/
endef

$(eval $(call BuildPackage,linux-atm))
$(eval $(call BuildPackage,atm-tools))
$(eval $(call BuildPackage,atm-debug-tools))
$(eval $(call BuildPackage,atm-diagnostics))
$(eval $(call BuildPackage,br2684ctl))
$(foreach t,$(ATM_DEBUG_BINS),$(eval $(call BuildAtmPlugin,atm-$(t),$(t),bin)))
$(foreach t,$(ATM_DEBUG_SBINS),$(eval $(call BuildAtmPlugin,atm-$(t),$(t),sbin)))
