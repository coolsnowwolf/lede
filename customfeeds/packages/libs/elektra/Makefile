#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# No copyright by Harald Geyer as filling in a template doesn't constitute
# an original work in the sense of copyright law.

include $(TOPDIR)/rules.mk

PKG_MAINTAINER:=Harald Geyer <harald@ccbib.org>

PKG_NAME:=elektra
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE.md
PKG_VERSION:=0.9.2
PKG_RELEASE:=3

# Use this for official releasees
PKG_HASH:=6f2fcf8aaed8863e1cc323265ca2617751ca50dac974b43a0811bcfd4a511f2e
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://ftp.libelektra.org/ftp/elektra/releases

# Use this to test versions still under development
#PKG_SOURCE_PROTO:=git
#PKG_SOURCE_URL:=https://github.com/ElektraInitiative/libelektra.git
#PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
#PKG_SOURCE_VERSION:=e97efb29a94f3a49cb952d06552fcf53708ea8c7
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.xz

PKG_BUILD_DEPENDS:= lua

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk
include $(INCLUDE_DIR)/nls.mk

define Package/libelektra/Default
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Elektra
  URL:=http://www.libelektra.org/
  SUBMENU:=LibElektra
endef

define Package/libelektra/Default-description
Elektra provides an universal and secure framework to store
configuration parameters in a global, hierarchical key database.
endef

define Package/libelektra-core
  $(call Package/libelektra/Default)
  TITLE:=Elektra core library
  DEPENDS:=+libpthread
endef

define Package/libelektra-core/description
$(call Package/libelektra/Default-description)

This package contains the core libraries, that all other parts of
elektra and all programs using elektra depend on, and an almost
minimal set of plugins to be actually useful.
endef

define Package/elektra-kdb
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Elektra kdb tool
  URL:=http://www.libelektra.org/
  DEPENDS:=+libelektra-core +libstdcpp
endef

define Package/elektra-kdb/description
$(call Package/libelektra/Default-description)

This package contains the kdb tool, which allows to access and change
the content of the key database from the shell.
endef

define Package/libelektra-resolvers
  $(call Package/libelektra/Default)
  TITLE:=Elektra additional resolvers
  DEPENDS:=+libelektra-core +libpthread
endef

define Package/libelektra-resolvers/description
$(call Package/libelektra/Default-description)

This package contains additional resolvers not included in the
libelektra-core package. Usually there is no reason to install this.
endef

define Package/libelektra-plugins
  $(call Package/libelektra/Default)
  TITLE:=Useful elektra plugins
  DEPENDS:=+libelektra-core $(ICONV_DEPENDS)
endef

define CONTENT_ELEKTRA_PLUGINS_TEXT
base64 conditionals csvstorage date file filecheck glob hexcode
hexnumber hidden hosts iconv ipaddr keytometa line lineendings list
mathcheck macaddr mini network null path profile quickdump
range reference rgbcolor shell syslog type uname unit validation
endef

CONTENT_ELEKTRA_PLUGINS = $(strip $(CONTENT_ELEKTRA_PLUGINS_TEXT))

define Package/libelektra-plugins/description
$(call Package/libelektra/Default-description)

This package contains plugins for various common configuration file
format and optional features without heavy dependencies. Currently
these plugins are included:
$(CONTENT_ELEKTRA_PLUGINS_TEXT)
endef

define Package/libelektra-cpp
  $(call Package/libelektra/Default)
  TITLE:=Elektra plugins depending on libstdcpp
  DEPENDS:=+libelektra-core +libstdcpp
endef

CONTENT_ELEKTRA_CPP=ccode directoryvalue dump

define Package/libelektra-cpp/description
$(call Package/libelektra/Default-description)

This package contains plugins written in C++.
Currently these are:
$(CONTENT_ELEKTRA_CPP)
endef

define Package/libelektra-boost
  $(call Package/libelektra/Default)
  TITLE:=Elektra plugins depending on boost
  DEPENDS:=+libelektra-core +libstdcpp +boost
endef

define Package/libelektra-boost/description
$(call Package/libelektra/Default-description)

This package contains plugins written in C++ with an dependency on
boost. Currently this is a plugin for a tcl like format.
endef

define Package/libelektra-crypto
  $(call Package/libelektra/Default)
  TITLE:=Elektra crypto plugin
  DEPENDS:=+libelektra-core +libgcrypt
endef

define Package/libelektra-crypto/description
$(call Package/libelektra/Default-description)

This package contains support for encrypting values before storage.
endef

define Package/libelektra-curlget
  $(call Package/libelektra/Default)
  TITLE:=Elektra curlget plugin
  DEPENDS:=+libelektra-core +libcurl +libopenssl
endef

define Package/libelektra-curlget/description
$(call Package/libelektra/Default-description)

This plugin can get configuration data from remote URLs before access.
endef

define Package/libelektra-dbus
  $(call Package/libelektra/Default)
  TITLE:=Elektra dbus plugin
  DEPENDS:=+libelektra-core +libdbus
endef

define Package/libelektra-dbus/description
$(call Package/libelektra/Default-description)

This package contains support for dbus notification on configuration
changes.
endef

define Package/libelektra-xerces
  $(call Package/libelektra/Default)
  TITLE:=Elektra xerces based xml plugin
  DEPENDS:=+libelektra-core +libstdcpp +libxerces-c
endef

define Package/libelektra-xerces/description
$(call Package/libelektra/Default-description)

The xerces plugin supplants the xmltool plugin
and allows us to use XML files not following a specific schemata.
Attributes are mapped to Elektra's metadata, multiple keys with the
same names are mapped to arrays.
endef

define Package/libelektra-xml
  $(call Package/libelektra/Default)
  TITLE:=Elektra xmltool plugin
  DEPENDS:=+libelektra-core +libxml2
endef

define Package/libelektra-xml/description
$(call Package/libelektra/Default-description)

This package contains a plugin for storing data with xml syntax.
endef

define Package/libelektra-yajl
  $(call Package/libelektra/Default)
  TITLE:=Elektra yajl (json) plugin
  DEPENDS:=+libelektra-core +yajl
endef

define Package/libelektra-yajl/description
$(call Package/libelektra/Default-description)

This package contains support for storing the key database as json files.
endef

define Package/libelektra-yamlcpp
  $(call Package/libelektra/Default)
  TITLE:=Elektra yaml plugin
  DEPENDS:=+libelektra-core +libyaml-cpp
endef

define Package/libelektra-yamlcpp/description
$(call Package/libelektra/Default-description)

This package contains support for storing the key database as yaml files.
endef

define Package/libelektra-zmq
  $(call Package/libelektra/Default)
  TITLE:=Elektra ZeroMQ transport plugins
  DEPENDS:=+libelektra-core +libzmq
endef

define Package/libelektra-python3
  $(call Package/libelektra/Default)
  TITLE:=Elektra python3 plugin
  DEPENDS:=+libelektra-core +python3-light +libstdcpp
endef

define Package/libelektra-python3/description
$(call Package/libelektra/Default-description)

This package adds python3 support to elektra.
endef

define Package/libelektra-lua
  $(call Package/libelektra/Default)
  TITLE:=Elektra lua plugin
  DEPENDS:=+libelektra-core +lua5.3 +libstdcpp
endef

define Package/libelektra-lua/description
$(call Package/libelektra/Default-description)

This package adds lua support to elektra.
endef

define Package/libelektra-extra
  $(call Package/libelektra/Default)
  TITLE:=Additional elektra plugins
  DEPENDS:=+libelektra-core +libstdcpp
endef

define CONTENT_EXTRA_PLUGINS_TEXT
blockresolver c constants counter desktop dpkg error fcrypt
fstab logchange mozprefs passwd process rename
timeofday tracer yamlsmith
endef

CONTENT_ELEKTRA_EXTRA:=$(strip $(CONTENT_EXTRA_PLUGINS_TEXT))

define Package/libelektra-extra/description
$(call Package/libelektra/Default-description)

This package contains extra plugins that are only useful for debugging
or as an example of what can be done. Also most experimental plugins
are included in this package. Currently this includes:
$(CONTENT_EXTRA_PLUGINS_TEXT)
endef

CMAKE_BINARY_SUBDIR=build

CMAKE_OPTIONS += \
	-DTARGET_PLUGIN_FOLDER="" \
	-DCARGO_EXECUTABLE=OFF \
	-DBUILD_FULL=OFF \
	-DBUILD_STATIC=OFF \
	-DBUILD_DOCUMENTATION=OFF \
	-DFORCE_IN_SOURCE_BUILD=ON \
	-DBUILD_TESTING=OFF \
	-DKDB_DEFAULT_RESOLVER=resolver_fm_pb_b \
	-DKDB_DEFAULT_STORAGE=ini \
	-DENABLE_OPTIMIZATIONS=OFF \
	-DPLUGINS="ALL;-gpgme;-internalnotification;-multifile;-simpleini" \
	-DIconv_INCLUDE_DIR="$(ICONV_PREFIX)/include" \
	-DIconv_LIBRARY="$(ICONV_PREFIX)/lib/libiconv.$(if $(CONFIG_BUILD_NLS),so,a)" \
	-DBINDINGS="MAINTAINED;-intercept_env;-intercept_fs;-io_uv;-io_ev;-io_glib"

define Package/libelektra-core/install
	$(INSTALL_DIR) $(1)/etc/kdb/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-core.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-ease.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-globbing.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-highlevel.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-invoke.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-io.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-kdb.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-merge.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-meta.so* $(1)/usr/lib/
	#The next is excluded because of an upstream bug
	#$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-notification.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-opts.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-plugin.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-pluginprocess.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-proposal.so* $(1)/usr/lib/
	#The next is only supported with glibc, so skip it.
	#$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektraintercept-* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-resolver_fm_pb_b.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-utility.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-cache.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-mmapstorage.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-ni.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-ini.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-sync.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-storage.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-resolver.so $(1)/usr/lib/
endef

define Package/elektra-kdb/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kdb $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib/elektra/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektratools* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/elektra/tool_exec/ $(1)/usr/lib/elektra/
endef

define Package/libelektra-resolvers/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-resolver_* $(1)/usr/lib/
	rm -f $(1)/usr/lib/libelektra-resolver_fm_pb_b.so
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-wresolver.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-noresolver.so $(1)/usr/lib/
endef

define Package/libelektra-plugins/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(foreach plugin,$(CONTENT_ELEKTRA_PLUGINS),$(PKG_INSTALL_DIR)/usr/lib/libelektra-$(plugin).so) $(1)/usr/lib/
endef

define Package/libelektra-boost/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-tcl.so $(1)/usr/lib/
endef

define Package/libelektra-cpp/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(foreach plugin,$(CONTENT_ELEKTRA_CPP),$(PKG_INSTALL_DIR)/usr/lib/libelektra-$(plugin).so) $(1)/usr/lib/
endef

define Package/libelektra-crypto/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-crypto.so $(1)/usr/lib/
endef

define Package/libelektra-curlget/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-curlget.so $(1)/usr/lib/
endef

define Package/libelektra-dbus/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-dbus.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-dbusrecv.so $(1)/usr/lib/
endef

define Package/libelektra-xerces/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-xerces.so $(1)/usr/lib/
endef

define Package/libelektra-xml/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-xmltool.so $(1)/usr/lib/
endef

define Package/libelektra-yajl/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-yajl.so $(1)/usr/lib/
endef

define Package/libelektra-yamlcpp/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-yamlcpp.so $(1)/usr/lib/
endef

define Package/libelektra-zmq/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-zeromqsend.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-zeromqrecv.so $(1)/usr/lib/
endef

define Package/libelektra-python3/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-python.so $(1)/usr/lib/
endef

define Package/libelektra-lua/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libelektra-lua.so $(1)/usr/lib/
endef

define Package/libelektra-extra/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(foreach plugin,$(CONTENT_ELEKTRA_EXTRA),$(PKG_INSTALL_DIR)/usr/lib/libelektra-$(plugin).so) $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/elektra/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/elektra/* $(1)/usr/include/elektra/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libelektra-core))
$(eval $(call BuildPackage,elektra-kdb))
$(eval $(call BuildPackage,libelektra-resolvers))
$(eval $(call BuildPackage,libelektra-plugins))
$(eval $(call BuildPackage,libelektra-boost))
$(eval $(call BuildPackage,libelektra-cpp))
$(eval $(call BuildPackage,libelektra-curlget))
$(eval $(call BuildPackage,libelektra-crypto))
$(eval $(call BuildPackage,libelektra-dbus))
$(eval $(call BuildPackage,libelektra-xerces))
$(eval $(call BuildPackage,libelektra-yamlcpp))
$(eval $(call BuildPackage,libelektra-xml))
$(eval $(call BuildPackage,libelektra-yajl))
$(eval $(call BuildPackage,libelektra-python3))
$(eval $(call BuildPackage,libelektra-lua))
$(eval $(call BuildPackage,libelektra-zmq))
$(eval $(call BuildPackage,libelektra-extra))
