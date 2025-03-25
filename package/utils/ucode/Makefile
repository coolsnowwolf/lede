#
# Copyright (C) 2020-2021 Jo-Philipp Wich <jo@mein.io>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ucode
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/jow-/ucode.git
PKG_SOURCE_DATE:=2025-03-24
PKG_SOURCE_VERSION:=b27d70c977ab4381f4094a0b1208e2a13fc5123f
PKG_MIRROR_HASH:=c2285ab8e473a12a1795864323af15779ef42a72dfe8a77c740cf99c316a2ac7
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=ISC

PKG_ABI_VERSION:=20230711
PKG_BUILD_DEPENDS:=libmd
HOST_BUILD_DEPENDS:=libjson-c/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += \
	-DSOVERSION=$(PKG_ABI_VERSION)

CMAKE_HOST_OPTIONS += \
	-DCMAKE_SKIP_RPATH=FALSE \
	-DCMAKE_INSTALL_RPATH="${STAGING_DIR_HOSTPKG}/lib"

ifeq ($(HOST_OS),Darwin)
  CMAKE_HOST_OPTIONS += \
	-DCMAKE_MACOSX_RPATH=1
else
  CMAKE_HOST_OPTIONS += \
	-DUSE_RPATH="${STAGING_DIR_HOSTPKG}/lib"
endif

CMAKE_HOST_OPTIONS += \
	-DFS_SUPPORT=ON \
	-DMATH_SUPPORT=ON \
	-DNL80211_SUPPORT=OFF \
	-DRESOLV_SUPPORT=OFF \
	-DRTNL_SUPPORT=OFF \
	-DSTRUCT_SUPPORT=ON \
	-DUBUS_SUPPORT=OFF \
	-DUCI_SUPPORT=OFF \
	-DULOOP_SUPPORT=OFF \
	-DDEBUG_SUPPORT=ON \
	-DLOG_SUPPORT=OFF \
	-DDIGEST_SUPPORT=OFF


define Package/ucode/default
  SUBMENU:=ucode
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Tiny scripting and templating language
endef

define Package/ucode
  $(Package/ucode/default)
  DEPENDS:=+libucode
endef

define Package/ucode/description
 ucode is a tiny script interpreter featuring an ECMAScript oriented
 script language and Jinja-inspired templating.
endef


define Package/libucode
  $(Package/ucode/default)
  SUBMENU:=
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= (library)
  ABI_VERSION:=$(PKG_ABI_VERSION)
  DEPENDS:=+libjson-c
endef

define Package/libucode/description
 The libucode package provides the shared runtime library for the ucode interpreter.
endef

# 1: name
# 2: cmake symbol
# 3: depends
# 4: description
define UcodeModule
  UCODE_MODULES += ucode-mod-$(strip $(1))
  CMAKE_OPTIONS += -D$(strip $(2))=$(if $(CONFIG_PACKAGE_ucode-mod-$(strip $(1))),ON,OFF)
  PKG_CONFIG_DEPENDS += CONFIG_PACKAGE_ucode-mod-$(strip $(1))

 define Package/ucode-mod-$(strip $(1))
  $(Package/ucode/default)
  TITLE+= ($(strip $(1)) module)
  DEPENDS:=+ucode $(3)
 endef

 define Package/ucode-mod-$(strip $(1))/description
 $(strip $(4))
 endef

 define Package/ucode-mod-$(strip $(1))/install
	$(INSTALL_DIR) $$(1)/usr/lib/ucode
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/ucode/$(strip $(1)).so $$(1)/usr/lib/ucode/
 endef
endef


define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/ucode
	$(CP) $(PKG_INSTALL_DIR)/usr/include/ucode/*.h $(1)/usr/include/ucode/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libucode.so* $(1)/usr/lib/
endef


define Package/ucode/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/u* $(1)/usr/bin/
endef

define Package/libucode/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libucode.so.* $(1)/usr/lib/
endef


$(eval $(call UcodeModule, \
	debug, DEBUG_SUPPORT, +libubox +libucode, \
	The debug plugin module provides runtime debugging and introspection facilities.))

$(eval $(call UcodeModule, \
	fs, FS_SUPPORT, , \
	The filesystem plugin module allows interaction with the local file system.))

$(eval $(call UcodeModule, \
	log, LOG_SUPPORT, +libubox, \
	The log plugin module provides access to the syslog and libubox ulog APIs.))

$(eval $(call UcodeModule, \
	math, MATH_SUPPORT, , \
	The math plugin provides access to various <math.h> procedures.))

$(eval $(call UcodeModule, \
	nl80211, NL80211_SUPPORT, +libnl-tiny +libubox, \
	The nl80211 plugin provides access to the Linux wireless 802.11 netlink API.))

$(eval $(call UcodeModule, \
	resolv, RESOLV_SUPPORT, , \
	The resolv plugin implements simple DNS resolving.))

$(eval $(call UcodeModule, \
	rtnl, RTNL_SUPPORT, +libnl-tiny +libubox, \
	The rtnl plugin provides access to the Linux routing netlink API.))

$(eval $(call UcodeModule, \
	socket, SOCKET_SUPPORT, , \
	The socket plugin provides access to IPv4, IPv6, Unix and packet socket APIs.))

$(eval $(call UcodeModule, \
	struct, STRUCT_SUPPORT, , \
	The struct plugin implements Python 3 compatible struct.pack/unpack functionality.))

$(eval $(call UcodeModule, \
	ubus, UBUS_SUPPORT, +libubus +libblobmsg-json, \
	The ubus module allows ucode template scripts to enumerate and invoke ubus procedures.))

$(eval $(call UcodeModule, \
	uci, UCI_SUPPORT, +libuci, \
	The uci module allows templates to read and modify uci configuration.))

$(eval $(call UcodeModule, \
	uloop, ULOOP_SUPPORT, +libubox, \
	The uloop module allows ucode scripts to interact with OpenWrt uloop event loop implementation.))

$(eval $(call UcodeModule, \
	digest, DIGEST_SUPPORT, , \
	The digest module allows ucode scripts to use libmd digests.))

$(eval $(call BuildPackage,libucode))
$(eval $(call BuildPackage,ucode))

$(foreach mod,$(UCODE_MODULES), \
	$(eval $(call BuildPackage,$(mod))))

$(eval $(call HostBuild))
