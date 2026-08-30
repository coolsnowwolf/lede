include $(TOPDIR)/rules.mk

PKG_NAME:=libtraceevent
PKG_VERSION:=1.9.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://git.kernel.org/pub/scm/libs/libtrace/libtraceevent.git/snapshot/
PKG_HASH:=e6d82cb2392d04866203387fa092692e4282ccc36c8235dc129afd1ceba12684

PKG_MAINTAINER:=Nick Hainke <vincent@systemli.org>

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

define Package/libtraceevent
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Linux kernel trace event library
  URL:=https://git.kernel.org/pub/scm/libs/libtrace/libtraceevent.git
  ABI_VERSION:=0
endef

define Package/libtraceevent/description
The libtraceevent library provides APIs to access kernel tracepoint events, located in
the tracefs file system under the events directory.
endef

define Package/libtraceevent-extra
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Extra plugins for libtraceevent
  DEPENDS:=+libtraceevent
endef

MESON_ARGS += \
        -Ddoc=false

PLUGINS_DIR := $(PKG_INSTALL_DIR)/usr/lib/traceevent/plugins
PLUGINS_MAIN := function hrtimer mac80211 sched_switch

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/traceevent $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtraceevent.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libtraceevent.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libtraceevent/install
	$(INSTALL_DIR) $(1)/usr/lib/traceevent/plugins
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtraceevent.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(patsubst %,$(PLUGINS_DIR)/plugin_%.so,$(PLUGINS_MAIN)) \
		$(1)/usr/lib/traceevent/plugins
endef

define Package/libtraceevent-extra/install
	$(INSTALL_DIR) $(1)/usr/lib/traceevent/plugins
	$(CP) \
		$$(patsubst %,$(PLUGINS_DIR)/plugin_%.so, \
			$$(filter-out $(PLUGINS_MAIN), \
				$$(patsubst $(PLUGINS_DIR)/plugin_%.so,%, \
					$$(wildcard $(PLUGINS_DIR)/plugin_*.so)))) \
		$(1)/usr/lib/traceevent/plugins
endef

$(eval $(call BuildPackage,libtraceevent))
$(eval $(call BuildPackage,libtraceevent-extra))
