#
# Copyright (C) Ian Cooper <iancooper@hotmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
#
#
include $(TOPDIR)/rules.mk

PKG_NAME:=openssl-qat
PKG_VERSION:=0.6.12
PKG_RELEASE:=1

PKG_SOURCE:=v${PKG_VERSION}.tar.gz
PKG_SOURCE_URL:=https://github.com/intel/QAT_Engine/archive/refs/tags/
PKG_HASH:=skip

PKG_BUILD_DIR:=$(BUILD_DIR)/QAT_Engine-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=openssl
PKG_FIXUP:=autoreconf
PKG_REMOVE_FILES:=autogen.sh

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/openssl-qat
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=SSL
  TITLE:=Intel Quick Assist OpenSSL Engine
  URL:=https://github.com/intel/QAT_Engine.git
  DEPENDS:= \
	@TARGET_x86_64 \
	+libopenssl \
	+(TARGET_x86_64&&QAT_C2XXX):quickassist-c2xxx \
	+(TARGET_x86_64&&QAT_C3XXX):quickassist-c3xxx
endef

define Package/openssl-qat/description
 Intel Quick Assist hardware acceleration engine for OpenSSL
endef

define Package/openssl-qat/config
 source "$(SOURCE)/Config.in"
endef

#CONFIGURE_ARGS += --with-openssl_install_dir=$(PKG_INSTALL_DIR)/usr 
CONFIGURE_ARGS += --prefix=$(PKG_INSTALL_DIR) 

CONFIGURE_ARGS += $(if $(CONFIG_QAT_C3XXX)                            ,--with-qat_dir=$(KERNEL_BUILD_DIR)/quickassist-c3xxx-1.7)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_C2XXX)                            ,--with-qat_dir=$(KERNEL_BUILD_DIR)/quickassist-c2xxx-1.5)

CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_DISABLE_RSA)               ,--disable-qat_rsa)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_DISABLE_DSA)               ,--disable-qat_dsa)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_DISABLE_DH)                ,--disable-qat_dh)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_DISABLE_ECDSA)             ,--disable-qat_ecdsa)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_DISABLE_CIPHERS)           ,--disable-qat_ciphers)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_DISABLE_PRF)               ,--disable-qat_prf)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_DISABLE_HKDF)              ,--disable-qat_hkdf)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_ENABLE_GCM_CIPHERS)        ,--enable-qat_gcm)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_ENABLE_SMALL_PACKET)       ,--enable-qat_small_pkt_offload)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_ENABLE_WARNINGS)           ,--enable-qat_warnings)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_ENABLE_DEBUG)              ,--enable-qat_debug)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_ENABLE_QAT_MEM_WARNINGS)   ,--enable-qat_mem_warnings)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_ENABLE_MEM_DEBUG)          ,--enable-qat_mem_debug)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_ENABLE_MULTITHREAD)        ,--enable-multi_thread)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_DISABLE_LENSTRA)           ,--disable-qat_lenstra_protection)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_ENGINE_DISABLE_AUTOINIT_ONFORK)   ,--disable-qat_auto_engine_init_on_fork)
CONFIGURE_ARGS += $(if $(CONFIG_QAT_C2XXX)                            ,--enable-qat16_driver)

MAKE_VARS += \
	CROSS_COMPILE="$(KERNEL_CROSS)" \
	KERNEL_SOURCE_ROOT="$(LINUX_DIR)" \
	PATH_AUTOCNF="include/generated/autoconf.h" \
	WITH_ICP_TARGET="1" 

define Package/openssl-qat/install
	$(INSTALL_DIR) $(1)/usr/lib/engines-1.1
	$(INSTALL_DIR) $(1)/usr/share/quickassist/QAT_Engine/config
	$(CP) $(PKG_BUILD_DIR)/.libs/qatengine.so $(1)/usr/lib/engines-1.1
	$(CP) $(PKG_BUILD_DIR)/qatengine.la $(1)/usr/lib/engines-1.1
	$(CP) $(PKG_BUILD_DIR)/qat/config/c3xxx $(1)/usr/share/quickassist/QAT_Engine/config
endef

$(eval $(call BuildPackage,openssl-qat))
