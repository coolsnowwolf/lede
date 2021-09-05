# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

PKG_NAME:=apparmor
PKG_VERSION:=3.0.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://gitlab.com/apparmor/apparmor.git
PKG_SOURCE_VERSION:=0325ba06da6eeb5acf3e568063a08136fd0913e0
PKG_MIRROR_HASH:=303ceca041ad8023fa44cdda366448d60b6299790266834b4078d30b70ad27f9

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
BUILDONLY:=1

PKG_BUILD_DEPENDS:=swig/host
HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../../../packages/lang/python/python3-package.mk

define Package/apparmor/Default
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=AppArmor
  URL:=https://apparmor.net
endef

define Package/libapparmor
  TITLE:=AppArmor library
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=https://apparmor.net
endef

define Package/python3-apparmor
  TITLE:=AppArmor Python bindings
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  URL:=https://apparmor.net
  DEPENDS:=+libapparmor +python3
endef

define Package/apparmor-utils
  $(call Package/apparmor/Default)
  TITLE:=AppArmor utils
  DEPENDS:=$(INTL_DEPENDS) +libapparmor +python3-apparmor +python3 +python3-readline +python3-psutil +ss +findutils-xargs
endef

define Package/apparmor-profiles
  $(call Package/apparmor/Default)
  TITLE:=AppArmor default profiles
endef

define Package/libapparmor/description
  Library to support AppArmor userspace utilities
endef

define Package/apparmor-utils/description
  AppArmor application security system init script and
  userspace utils to assist with AppArmor profile management
endef

define Package/apparmor-profiles/description
  A collection of profiles for the AppArmor application security system
endef

CONFIGURE_PATH=libraries/libapparmor

TARGET_LDFLAGS += \
		-L$(PYTHON3_LIB_DIR)

CONFIGURE_VARS += \
		SHELL=$(bash) \
		PYTHON_VERSION=$(PYTHON3_VERSION) \
		PYTHON_VERSIONS=$(PYTHON3) \
		PYTHON=$(PYTHON3) \
		PYTHON_CONFIG=$(STAGING_DIR_ROOT)/host/bin/python$(PYTHON3_VERSION)-config \
		PYTHON_LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(PYTHON3_LIB_DIR)" \
		PYTHON_CPPFLAGS="-I$(STAGING_DIR)/usr/include/python$(PYTHON3_VERSION)" \
		PYTHON_LDFLAGS="-I$(PYTHON3_INC_DIR) -L$(STAGING_DIR)/usr/lib -L$(PYTHON3_LIB_DIR)" \
		PYTHON_EXTRA_LDFLAGS="-L$(STAGING_DIR)/usr/lib -L$(PYTHON3_LIB_DIR)/config-$(PYTHON3_VERSION) -lpthread -ldl -lm -lz -lpython$(PYTHON3_VERSION)" \
		ac_cv_path_PYTHON_CONFIG="$(STAGING_DIR)/host/bin/python$(PYTHON3_VERSION)-config"

CONFIGURE_ARGS += \
		--with-python \
		--without-perl \
		--without-ruby \
		--disable-man-pages

ifeq ($(CONFIG_BUILD_NLS),y)
	MAKE_VARS += WITH_LIBINTL=1
	MAKE_FLAGS += WITH_LIBINTL=1
endif

APPARMOR_CFLAGS = -I$(PKG_BUILD_DIR)/libraries/libapparmor/include
APPARMOR_LDFLAGS = -L$(PKG_BUILD_DIR)/libraries/libapparmor/src/.libs

define Build/Configure
	$(MAKE) -C $(PKG_BUILD_DIR)/libraries/libapparmor configure
	$(RM) $(PKG_BUILD_DIR)/libraries/libapparmor/Makefile
	$(SED) 's#ac_cv_path_PYTHON_CONFIG=#ac_cv_path_X_PYTHON_CONFIG=#g' $(PKG_BUILD_DIR)/libraries/libapparmor/configure
	$(call Build/Configure/Default)
endef

define Build/Compile
	# Building libapparmor
	+$(MAKE_VARS) \
		CFLAGS="$(TARGET_CFLAGS)" CPPFLAGS="$(TARGET_CPPFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)" $(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/libraries/libapparmor \
		$(MAKE_FLAGS)
	# Building parser
	+$(MAKE_VARS) PYTHON=$(HOST_PYTHON) \
		CFLAGS="$(TARGET_CFLAGS) $(APPARMOR_CFLAGS)" CPPFLAGS="$(TARGET_CPPFLAGS) $(APPARMOR_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS) $(APPARMOR_LDFLAGS) -lgcc_s" USE_SYSTEM=0 $(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/parser \
		$(MAKE_FLAGS) apparmor_parser
	# Building binutils
	+$(MAKE_VARS) PYTHON=$(HOST_PYTHON) \
		CFLAGS="$(TARGET_CFLAGS) $(APPARMOR_CFLAGS)" CPPFLAGS="$(TARGET_CPPFLAGS) $(APPARMOR_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS) $(APPARMOR_LDFLAGS)" USE_SYSTEM=0 $(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/binutils \
		$(MAKE_FLAGS)
	# Building utils
	+$(MAKE_VARS) PYTHON=$(HOST_PYTHON) \
		CFLAGS="$(TARGET_CFLAGS) $(APPARMOR_CFLAGS)" CPPFLAGS="$(TARGET_CPPFLAGS) $(APPARMOR_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS) $(APPARMOR_LDFLAGS)" USE_SYSTEM=0 $(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/utils \
		$(MAKE_FLAGS)
	# Building profiles
	+$(MAKE_VARS) PYTHON=$(HOST_PYTHON) \
		CFLAGS="$(TARGET_CFLAGS) $(APPARMOR_CFLAGS)" CPPFLAGS="$(TARGET_CPPFLAGS) $(APPARMOR_CFLAGS")" LDFLAGS="$(TARGET_LDFLAGS) $(APPARMOR_LDFLAGS)" USE_SYSTEM=0 $(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/profiles \
		$(MAKE_FLAGS)
endef

define Build/Install
	# Make sure we have python's setup tools installed
	$(if $(PYTHON3_PKG_HOST_PIP_INSTALL_ARGS), \
		$(call HostPython3/PipInstall,$(PYTHON3_PKG_HOST_PIP_INSTALL_ARGS)) \
	)
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)-libapparmor $(PKG_INSTALL_DIR)-utils $(PKG_INSTALL_DIR)-profiles
	# Installing libapparmor
	+$(MAKE_VARS) PYTHON=$(HOST_PYTHON) VERSION=$(PYTHON3_VERSION) \
		CFLAGS="$(TARGET_CFLAGS)" CPPFLAGS="$(TARGET_CPPFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)" \
		$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/libraries/libapparmor \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)-libapparmor" install
	# Installing parser
	+$(MAKE_VARS) PYTHON=$(HOST_PYTHON) VERSION=$(PYTHON3_VERSION) \
		CFLAGS="$(TARGET_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)" USE_SYSTEM=1 $(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/parser \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)-utils" install
	# Installing binutils
	+$(MAKE_VARS) PYTHON=$(HOST_PYTHON) VERSION=$(PYTHON3_VERSION) \
		CFLAGS="$(TARGET_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)" USE_SYSTEM=1 $(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/binutils \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)-utils" install
	# Installing utils
	+$(MAKE_VARS) PYTHON=$(HOST_PYTHON) VERSION=$(PYTHON3_VERSION) \
		CFLAGS="$(TARGET_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)" USE_SYSTEM=1 $(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/utils \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)-utils" install
	# Installing profiles
	+$(MAKE_VARS) PYTHON=$(HOST_PYTHON) VERSION=$(PYTHON3_VERSION) \
		CFLAGS="$(TARGET_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)" USE_SYSTEM=1 $(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/profiles \
		$(MAKE_FLAGS) DESTDIR="$(PKG_INSTALL_DIR)-profiles" install
endef

define Package/libapparmor/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)-libapparmor/usr/lib/libapparmor.so.1 $(1)/usr/lib/
	$(LN) libapparmor.so.1 $(1)/usr/lib/libapparmor.so
endef

define Package/python3-apparmor/install
	$(INSTALL_DIR) \
		$(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages \
		$(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages/LibAppArmor
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)-libapparmor/usr/lib/python$(PYTHON3_VERSION)/site-packages/*.egg-info \
		$(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)-libapparmor/usr/lib/python$(PYTHON3_VERSION)/site-packages/LibAppArmor/*.py \
		$(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages/LibAppArmor
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)-libapparmor/usr/lib/python$(PYTHON3_VERSION)/site-packages/LibAppArmor/*.so \
		$(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages/LibAppArmor/
	$(LN) -s _LibAppArmor.cpython-$(PYTHON3_VERSION_MAJOR)$(PYTHON3_VERSION_MINOR).so \
		$(1)/usr/lib/python$(PYTHON3_VERSION)/site-packages/LibAppArmor/_LibAppArmor.so
endef

define Package/apparmor-utils/install
	$(INSTALL_DIR) $(1)/etc/apparmor $(1)/usr/sbin $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)-utils/sbin/apparmor_parser $(1)/sbin/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)-utils/etc/apparmor/*.conf $(1)/etc/apparmor/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)-utils/etc/apparmor/severity.db $(1)/etc/apparmor/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)-utils/sbin/apparmor_parser $(1)/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)-utils/usr/bin/{aa-exec,aa-easyprof,aa-enabled,aa-features-abi} $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)-utils/usr/sbin/{aa-audit,aa-autodep,aa-cleanprof,aa-complain,aa-decode,aa-disable,aa-enforce,aa-genprof,aa-logprof,aa-mergeprof,aa-notify,aa-remove-unknown,aa-status,aa-unconfined} $(1)/usr/sbin/
	$(LN) aa-status $(1)/usr/sbin/apparmor_status
	$(INSTALL_DIR) $(1)/usr/share/apparmor/easyprof/templates $(1)/usr/share/apparmor/easyprof/policygroups
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)-utils/usr/share/apparmor/easyprof/templates/* $(1)/usr/share/apparmor/easyprof/templates/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)-utils/usr/share/apparmor/easyprof/policygroups/* $(1)/usr/share/apparmor/easyprof/policygroups/
	$(INSTALL_DIR) $(1)/usr/lib/python3.9/site-packages $(1)/usr/lib/python3.9/site-packages/apparmor $(1)/usr/lib/python3.9/site-packages/apparmor/rule
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)-utils/usr/lib/python3.9/site-packages/*.egg-info \
		$(1)/usr/lib/python3.9/site-packages/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)-utils/usr/lib/python3.9/site-packages/apparmor/*.py \
		$(1)/usr/lib/python3.9/site-packages/apparmor/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)-utils/usr/lib/python3.9/site-packages/apparmor/rule/*.py \
		$(1)/usr/lib/python3.9/site-packages/apparmor/rule
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/lib/functions
	$(INSTALL_BIN) ./files/apparmor.sh $(1)/lib/functions/
	$(INSTALL_BIN) ./files/apparmor.init $(1)/etc/init.d/apparmor
endef

define Package/apparmor-profiles/install
	$(INSTALL_DIR) $(1)/etc/apparmor.d $(1)/usr/share/apparmor/extra-profiles
	$(CP) -aR $(PKG_INSTALL_DIR)-profiles/etc/apparmor.d/** $(1)/etc/apparmor.d/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)-profiles/usr/share/apparmor/extra-profiles/** $(1)/usr/share/apparmor/extra-profiles/
endef

$(eval $(call BuildPackage,libapparmor))
$(eval $(call BuildPackage,python3-apparmor))
$(eval $(call BuildPackage,apparmor-utils))
$(eval $(call BuildPackage,apparmor-profiles))
