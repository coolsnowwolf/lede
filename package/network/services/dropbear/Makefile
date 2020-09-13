#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dropbear
PKG_VERSION:=2020.80
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	http://matt.ucc.asn.au/dropbear/releases/ \
	https://dropbear.nl/mirror/releases/
PKG_HASH:=d927941b91f2da150b2033f1a88b6a47999bf0afb1493a73e9216cffdb5d7949

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE libtomcrypt/LICENSE libtommath/LICENSE
PKG_CPE_ID:=cpe:/a:matt_johnston:dropbear_ssh_server

PKG_BUILD_PARALLEL:=1
PKG_ASLR_PIE_REGULAR:=1
PKG_USE_MIPS16:=0
PKG_FIXUP:=autoreconf

PKG_CONFIG_DEPENDS:= \
	CONFIG_TARGET_INIT_PATH CONFIG_DROPBEAR_ECC CONFIG_DROPBEAR_ECC_FULL \
	CONFIG_DROPBEAR_CURVE25519 CONFIG_DROPBEAR_ZLIB \
	CONFIG_DROPBEAR_ED25519 CONFIG_DROPBEAR_CHACHA20POLY1305 \
	CONFIG_DROPBEAR_UTMP CONFIG_DROPBEAR_PUTUTLINE \
	CONFIG_DROPBEAR_DBCLIENT

include $(INCLUDE_DIR)/package.mk

ifneq ($(DUMP),1)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell echo $(CONFIG_TARGET_INIT_PATH) | mkhash md5)
endif

define Package/dropbear/Default
  URL:=http://matt.ucc.asn.au/dropbear/
endef

define Package/dropbear/config
	source "$(SOURCE)/Config.in"
endef

define Package/dropbear
  $(call Package/dropbear/Default)
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=Small SSH2 client/server
  DEPENDS:= +DROPBEAR_ZLIB:zlib
  ALTERNATIVES:=\
	  100:/usr/bin/ssh:/usr/sbin/dropbear \
	  100:/usr/bin/scp:/usr/sbin/dropbear \

endef

define Package/dropbear/description
 A small SSH2 server/client designed for small memory environments.
endef

define Package/dropbear/conffiles
$(if $(CONFIG_DROPBEAR_ED25519),/etc/dropbear/dropbear_ed25519_host_key)
$(if $(CONFIG_DROPBEAR_ECC),/etc/dropbear/dropbear_ecdsa_host_key)
/etc/dropbear/dropbear_rsa_host_key
/etc/config/dropbear
endef

define Package/dropbearconvert
  $(call Package/dropbear/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Utility for converting SSH keys
  DEPENDS:= +DROPBEAR_ZLIB:zlib
endef

CONFIGURE_ARGS += \
	--disable-pam \
	--enable-openpty \
	--enable-syslog \
	--disable-lastlog \
	--disable-utmpx \
	$(if $(CONFIG_DROPBEAR_UTMP),,--disable-utmp) \
	--disable-wtmp \
	--disable-wtmpx \
	--disable-loginfunc \
	$(if $(CONFIG_DROPBEAR_PUTUTLINE),,--disable-pututline) \
	--disable-pututxline \
	$(if $(CONFIG_DROPBEAR_ZLIB),,--disable-zlib) \
	--enable-bundled-libtom

TARGET_CFLAGS += -DARGTYPE=3 -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections -flto=jobserver

define Build/Configure
	: > $(PKG_BUILD_DIR)/localoptions.h

	$(Build/Configure/Default)

	echo '#define DEFAULT_PATH "$(TARGET_INIT_PATH)"' >> \
		$(PKG_BUILD_DIR)/localoptions.h

	echo '#define DROPBEAR_CURVE25519 $(if $(CONFIG_DROPBEAR_CURVE25519),1,0)' >> \
		$(PKG_BUILD_DIR)/localoptions.h

	for OPTION in DROPBEAR_ECDSA DROPBEAR_ECDH; do \
		echo "#define $$$$OPTION $(if $(CONFIG_DROPBEAR_ECC),1,0)" >> \
			$(PKG_BUILD_DIR)/localoptions.h; \
	done

	echo '#define DROPBEAR_ED25519 $(if $(CONFIG_DROPBEAR_ED25519),1,0)' >> \
		$(PKG_BUILD_DIR)/localoptions.h

	echo '#define DROPBEAR_CHACHA20POLY1305 $(if $(CONFIG_DROPBEAR_CHACHA20POLY1305),1,0)' >> \
		$(PKG_BUILD_DIR)/localoptions.h

	# remove protocol idented software version number
	$(ESED) 's,^(#define LOCAL_IDENT) .*$$$$,\1 "SSH-2.0-dropbear",g' \
		$(PKG_BUILD_DIR)/sysoptions.h

	# disable legacy/unsafe methods and unused functionality
	for OPTION in INETD_MODE DROPBEAR_CLI_NETCAT \
	DROPBEAR_DSS DROPBEAR_USE_PASSWORD_ENV DO_MOTD ; do \
		echo "#define $$$$OPTION 0" >> \
			$(PKG_BUILD_DIR)/localoptions.h; \
	done

	# enable nistp384 and nistp521 only if full ECC support was requested
	for OPTION in DROPBEAR_ECC_384 DROPBEAR_ECC_521; do \
		$(ESED) 's,^(#define '$$$$OPTION') .*$$$$,\1 $(if $(CONFIG_DROPBEAR_ECC_FULL),1,0),g' \
		$(PKG_BUILD_DIR)/sysoptions.h; \
	done

	# Enforce rebuild of svr-chansession.c
	rm -f $(PKG_BUILD_DIR)/svr-chansession.o

	# Rebuild them on config change
	+$(MAKE) -C $(PKG_BUILD_DIR)/libtomcrypt clean
	+$(MAKE) -C $(PKG_BUILD_DIR)/libtommath clean
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		PROGRAMS="dropbear $(if $(CONFIG_DROPBEAR_DBCLIENT),dbclient,) dropbearkey scp" \
		MULTI=1 SCPPROGRESS=1
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		PROGRAMS="dropbearconvert"
endef

define Package/dropbear/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dropbearmulti $(1)/usr/sbin/dropbear
	$(INSTALL_DIR) $(1)/usr/bin
	$(if $(CONFIG_DROPBEAR_DBCLIENT),$(LN) ../sbin/dropbear $(1)/usr/bin/dbclient,)
	$(LN) ../sbin/dropbear $(1)/usr/bin/dropbearkey
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/dropbear.config $(1)/etc/config/dropbear
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/dropbear.init $(1)/etc/init.d/dropbear
	$(INSTALL_DIR) $(1)/usr/lib/opkg/info
	$(INSTALL_DIR) $(1)/etc/dropbear
	$(INSTALL_DIR) $(1)/lib/preinit
	$(INSTALL_DATA) ./files/dropbear.failsafe $(1)/lib/preinit/99_10_failsafe_dropbear
	$(if $(CONFIG_DROPBEAR_ED25519),touch $(1)/etc/dropbear/dropbear_ed25519_host_key)
	$(if $(CONFIG_DROPBEAR_ECC),touch $(1)/etc/dropbear/dropbear_ecdsa_host_key)
	touch $(1)/etc/dropbear/dropbear_rsa_host_key
endef

define Package/dropbearconvert/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dropbearconvert $(1)/usr/bin/dropbearconvert
endef

$(eval $(call BuildPackage,dropbear))
$(eval $(call BuildPackage,dropbearconvert))
