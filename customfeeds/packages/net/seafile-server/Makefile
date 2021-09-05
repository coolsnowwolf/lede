#
# Copyright (C) 2007-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=seafile-server
PKG_VERSION:=7.1.5
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/haiwen/seafile-server/tar.gz/v$(PKG_VERSION)-server?
PKG_HASH:=407a6243f754e366ce6754a2aa643574aa7742d4e81b7cc350dea348f3063707

PKG_MAINTAINER:=Jeffery To <jeffery.to@gmail.com>
PKG_LICENSE:=AGPL-3.0-only
PKG_LICENSE_FILES:=LICENSE.txt

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-server
PKG_BUILD_DEPENDS:=vala/host libevhtp

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PYTHON3_PKG_BUILD:=0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../../lang/python/python3-package.mk

# Check that the actual Makefile version-relase match the above.
$(eval $(shell awk '/^PKG_VERSION.*=/ { print "SEAHUB_" $$$$0 }' ../seafile-seahub/Makefile))
ifneq ($(PKG_VERSION),$(SEAHUB_PKG_VERSION))
  $(error $(if $(SEAHUB_PKG_VERSION), \
    Version mismatch between seafile-seahub ($(SEAHUB_PKG_VERSION)) and \
    seafile-server ($(PKG_VERSION)), \
    Could not get PKG_VERSION from seafile-seahub Makefile))
endif

define Package/seafile-server
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Seafile server
  URL:=https://seafile.com/
  DEPENDS:= \
    +bash \
    +glib2 \
    +jansson \
    +libarchive \
    +libevent2 \
    +libevent2-openssl \
    +libmariadb \
    +libopenssl \
    +libpthread \
    +libsearpc \
    +libsqlite3 \
    +libuuid \
    +oniguruma \
    +procps-ng \
    +procps-ng-pkill \
    +python3-pymysql \
    +python3-seafile-server \
    +seafile-ccnet \
    +seafile-seahub \
    +sqlite3-cli \
    +zlib \
    $(ICONV_DEPENDS)
endef

define Package/seafile-server/description
  Open source cloud storage with advanced features on privacy
  protection and teamwork.
endef

define Package/seafile-server/config
  select LIBEVHTP_BUILD_DEPENDS
endef

define Package/seafile-server-fuse
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Seafile server - FUSE extension
  URL:=https://seafile.com/
  DEPENDS:= \
    +bash \
    +glib2 \
    +jansson \
    +libevent2 \
    +libevent2-openssl \
    +libfuse \
    +libmariadb \
    +libopenssl \
    +libpthread \
    +libsearpc \
    +libsqlite3 \
    +libuuid \
    +procps-ng \
    +procps-ng-pkill \
    +seafile-ccnet \
    +seafile-server \
    +zlib \
    $(ICONV_DEPENDS)
endef

define Package/seafile-server-fuse/description
$(call Package/seafile-server/description)

  This package contains the FUSE extension (seaf-fuse) for Seafile
  server.
endef

define Package/python3-seafile-server
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Python bindings for Seafile server
  DEPENDS:=+python3-light +python3-logging +python3-searpc +python3-seafile-ccnet
endef

define Package/python3-seafile-server/description
$(call Package/seafile-server/description)

  This package contains Python bindings for Seafile server.
endef

CONFIGURE_ARGS += \
	--disable-console \
	--enable-fuse \
	--enable-python \
	--with-mysql="$(STAGING_DIR)/usr/bin/mysql_config"

CONFIGURE_VARS += \
	PYTHON="$(HOST_PYTHON3_BIN)"

define Package/seafile-server/conffiles
/etc/config/seafile-server
endef

SEAFILE_SERVER_BINARIES:= \
	seaf-fsck \
	seafile-controller \
	seaf-server \
	seaf-server-init \
	seafserv-gc

SEAFILE_SERVER_SHELL_SCRIPTS:= \
	reset-admin.sh \
	seaf-fsck.sh \
	seaf-gc.sh \
	seafile.sh \
	seahub.sh \
	setup-seafile-mysql.sh \
	setup-seafile.sh
	# sqlite2mysql.sh handled differently

SEAFILE_SERVER_PYTHON_SCRIPTS:= \
	check_init_admin.py \
	setup-seafile-mysql.py \
	sqlite2mysql.py

define Package/seafile-server/install
	$(INSTALL_DIR) $(1)/usr/{bin,libexec}
	$(INSTALL_DIR) $(1)/usr/share/seafile/seafile-server
	$(INSTALL_DIR) $(1)/etc/{config,init.d,seafile}
	$(INSTALL_DIR) $(1)/lib/upgrade/keep.d

	for bin in $(SEAFILE_SERVER_BINARIES) ; do \
		$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/$$$$bin $(1)/usr/libexec/ ; \
	done

	for sh in $(SEAFILE_SERVER_SHELL_SCRIPTS) ; do \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/scripts/$$$$sh $(1)/usr/bin/$$$${sh%.sh} ; \
	done
	mv $(1)/usr/bin/reset-admin $(1)/usr/bin/reset-seafile-admin

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scripts/sqlite2mysql.sh $(1)/usr/share/seafile/seafile-server/

	for py in $(SEAFILE_SERVER_PYTHON_SCRIPTS) ; do \
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/scripts/$$$$py $(1)/usr/share/seafile/seafile-server/ ; \
	done

	$(CP) $(PKG_BUILD_DIR)/scripts/{sql,upgrade} $(1)/usr/share/seafile/seafile-server/
	rm -rf $(1)/usr/share/seafile/seafile-server/upgrade/win32

	$(INSTALL_BIN) ./files/create-seafile-admin.sh $(1)/usr/bin/create-seafile-admin

	$(INSTALL_CONF) ./files/seafile-server.config $(1)/etc/config/seafile-server
	$(INSTALL_BIN) ./files/seafile-server.init $(1)/etc/init.d/seafile-server
	$(INSTALL_DATA) ./files/seafile-server.upgrade $(1)/lib/upgrade/keep.d/seafile-server
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/seafile
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/seafile/*.h $(1)/usr/include/seafile/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libseafile.pc $(1)/usr/lib/pkgconfig/
endef

define Package/seafile-server-fuse/install
	$(INSTALL_DIR) $(1)/usr/{bin,libexec}
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/seaf-fuse $(1)/usr/libexec/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scripts/seaf-fuse.sh $(1)/usr/bin/seaf-fuse
endef

Py3Package/python3-seafile-server/install:=:

define Package/seafile-server/postinst
#!/bin/sh
cat << EOF

Please see:

https://github.com/openwrt/packages/blob/master/net/seafile-server/README.md

for OpenWrt-specific installation notes (in particular, if you are
upgrading from a version before 7.1).

EOF

[ -z "$$IPKG_INSTROOT" ] || exit 0

cat << EOF

opkg will attempt to start Seafile server now. This may fail if you have
not set up Seafile server before or if you are upgrading and have not
run the upgrade scripts.

EOF
endef

$(eval $(call BuildPackage,seafile-server))
$(eval $(call BuildPackage,seafile-server-fuse))

$(eval $(call Py3Package,python3-seafile-server))
$(eval $(call BuildPackage,python3-seafile-server))
$(eval $(call BuildPackage,python3-seafile-server-src))
