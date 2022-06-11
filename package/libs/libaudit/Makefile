#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libaudit
PKG_VERSION:=2.8.5
PKG_RELEASE:=1

PKG_SOURCE_NAME:=audit
PKG_SOURCE:=$(PKG_SOURCE_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://people.redhat.com/sgrubb/audit
PKG_HASH:=0e5d4103646e00f8d1981e1cd2faea7a2ae28e854c31a803e907a383c5e2ecb7
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_NAME)-$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_SOURCE_NAME)-$(PKG_VERSION)
PKG_MAINTAINER:=Thomas Petazzoni <thomas.petazzoni@bootlin.com>
PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:linux_audit_project:linux_audit

PKG_FIXUP:=autoreconf

PKG_USE_MIPS16:=0
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/libaudit
  CATEGORY:=Libraries
  TITLE:=Linux Auditing Framework (shared library)
  URL:=http://people.redhat.com/sgrubb/audit/
endef

define Package/libaudit/description
	This package contains the audit shared library.
endef

CONFIGURE_VARS += \
	LDFLAGS_FOR_BUILD="$(HOST_LDFLAGS)" \
	CPPFLAGS_FOR_BUILD="$(HOST_CPPFLAGS)" \
	CFLAGS_FOR_BUILD="$(HOST_CFLAGS)" \
	CC_FOR_BUILD="$(HOSTCC)"

CONFIGURE_ARGS += \
	--without-libcap-ng \
	--disable-systemd \
	--without-python \
	--without-python3 \
	--disable-zos-remote

ifeq ($(ARCH),aarch64)
CONFIGURE_ARGS += --with-aarch64
else ifeq ($(ARCH),arm)
CONFIGURE_ARGS += --with-arm
endif

HOST_CONFIGURE_ARGS += \
	--without-libcap-ng \
	--disable-systemd \
	--without-python \
	--without-python3 \
	--disable-zos-remote

MAKE_PATH:=lib

# Host/Compile/default doesn't include $(MAKE_PATH), override to use,
# so we avoid building and installing unnecessary parts on the host.
define Host/Compile
	+$(HOST_MAKE_VARS) $(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR)/$(MAKE_PATH) $(HOST_MAKE_FLAGS) all
endef

define Host/Install
	+$(HOST_MAKE_VARS) $(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR)/lib $(HOST_MAKE_FLAGS) install
	+$(HOST_MAKE_VARS) $(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR)/init.d $(HOST_MAKE_FLAGS) install
endef

# We can't use the default, as the default passes $(MAKE_ARGS), which
# overrides CC, CFLAGS, etc. and defeats the *_FOR_BUILD definitions
# passed in CONFIGURE_VARS
define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/$(MAKE_PATH)
endef

define Build/Install
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/lib $(MAKE_INSTALL_FLAGS) install
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/init.d $(MAKE_INSTALL_FLAGS) install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/libaudit/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc
	$(CP) $(PKG_INSTALL_DIR)/etc/libaudit.conf $(1)/etc/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,libaudit))
