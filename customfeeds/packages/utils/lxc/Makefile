#
# Copyright (C) 2013-2015 OpenWrt.org
# Copyright (C) 2020 Sartura
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lxc
PKG_VERSION:=4.0.10
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://linuxcontainers.org/downloads/lxc/
PKG_HASH:=a10c518056d2e497bd600e4ffac2853bdeab1e835bf455a6d87cc8df128bfae9

PKG_MAINTAINER:=Marko Ratkaj <marko.ratkaj@sartura.hr>
PKG_LICENSE:=LGPL-2.1-or-later BSD-2-Clause GPL-2.0

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

LXC_APPLETS_BIN += \
	attach autostart cgroup copy config console create destroy device \
	execute freeze info monitor snapshot start stop unfreeze unshare \
	usernsexec wait top ls

LXC_APPLETS_LIB += \
	monitord user-nic

LXC_SCRIPTS += \
	checkconfig

DEPENDS_APPLETS = +libpthread +libcap +liblxc

DEPENDS_create = +lxc-configs +lxc-hooks +lxc-templates +flock

define Package/lxc/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=LXC userspace tools
  URL:=https://linuxcontainers.org/
  DEPENDS:=lxc
endef

define Package/lxc
  $(call Package/lxc/Default)
  DEPENDS:=@!arc
  MENU:=1
endef

define Package/lxc-auto
  $(call Package/lxc/Default)
  TITLE:= (initscript)
  DEPENDS+=+lxc-start +lxc-stop
endef

define Package/lxc-auto/description
 LXC is the userspace control package for Linux Containers, a lightweight
 virtual system mechanism sometimes described as "chroot on steroids".
 This package adds an initscript for starting and stopping the containers
 on boot and shutdown.
endef

define Package/lxc-auto/conffiles
/etc/config/lxc-auto
endef

define Package/lxc-unprivileged
  $(call Package/lxc/Default)
  TITLE:=Helper script for unprivileged containers support
  DEPENDS+=+shadow-utils +shadow-newuidmap +shadow-newgidmap
endef

define Package/lxc-unprivileged/description
 Support for unprivileged containers requires newuidmap and newguidmap.
 This package makes sure they are available & have correct permissions.
endef

define Package/lxc-unprivileged/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/lxc-unprivileged.defaults $(1)/etc/uci-defaults/lxc-unprivileged
endef

define Package/lxc/config
  source "$(SOURCE)/Config.in"
endef

define Package/lxc/description
 LXC is the userspace control package for Linux Containers, a lightweight
 virtual system mechanism sometimes described as "chroot on steroids".
endef

define Package/lxc-common
  $(call Package/lxc/Default)
  TITLE:=LXC common files
endef

define Package/lxc-hooks
  $(call Package/lxc/Default)
  TITLE:=LXC virtual machine hooks
endef

define Package/lxc-templates
  $(call Package/lxc/Default)
  TITLE:=LXC virtual machine templates
endef

define Package/lxc-configs
  $(call Package/lxc/Default)
  TITLE:=LXC virtual machine common config files
endef

define Package/liblxc
  $(call Package/lxc/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=LXC userspace library
  DEPENDS+= +libcap +libpthread +LXC_SECCOMP:libseccomp +libopenssl
endef

define Package/lxc-init
  $(call Package/lxc/Default)
  TITLE:=LXC Lua bindings
  DEPENDS+= +liblxc
endef

CONFIGURE_ARGS += \
	--disable-werror \
	--disable-rpath \
	--disable-doc \
	--disable-api-docs \
	--disable-apparmor \
	--disable-selinux \
	--$(if $(CONFIG_LXC_SECCOMP),en,dis)able-seccomp \
	--enable-capabilities \
	--disable-examples

ifdef CONFIG_USE_MIPS16
  TARGET_CFLAGS += -minterlink-mips16
endif
TARGET_LDFLAGS += -lgcc_eh

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/lxc/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/lxc/* \
		$(1)/usr/include/lxc/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/liblxc.so* \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/lxc.pc \
		$(1)/usr/lib/pkgconfig/
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/lxc.pc
	$(SED) 's,/usr/lib,$$$${prefix}/lib,g' $(1)/usr/lib/pkgconfig/lxc.pc
endef


define Package/lxc/install
	true
endef

define Package/lxc-auto/install
	$(INSTALL_DIR) $(1)/etc/config $(1)/etc/init.d
	$(INSTALL_CONF) ./files/lxc-auto.config $(1)/etc/config/lxc-auto
	$(INSTALL_BIN) ./files/lxc-auto.init $(1)/etc/init.d/lxc-auto
endef

define Package/lxc-common/conffiles
/etc/lxc/default.conf
/etc/lxc/lxc.conf
endef

define Package/lxc-common/install
	$(INSTALL_DIR) $(1)/usr/lib/lxc/rootfs
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/lxc/rootfs/README \
		$(1)/usr/lib/lxc/rootfs/

	$(INSTALL_DIR) $(1)/usr/share/lxc
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/lxc/lxc.functions \
		$(1)/usr/share/lxc/

	$(INSTALL_DIR) $(1)/etc/lxc/
	$(CP) \
		$(PKG_INSTALL_DIR)/etc/lxc/default.conf \
		$(1)/etc/lxc/default.conf

	$(INSTALL_DIR) $(1)/etc/lxc/
	$(CP) \
		./files/lxc.conf \
		$(1)/etc/lxc/lxc.conf

	$(INSTALL_DIR) $(1)/srv/lxc/
endef

define Package/lxc-hooks/install
	$(INSTALL_DIR) $(1)/usr/share/lxc/hooks
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/lxc/hooks/* \
		$(1)/usr/share/lxc/hooks/
endef

define Package/lxc-templates/install
	$(INSTALL_DIR) $(1)/usr/share/lxc/templates/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/lxc/templates/lxc-* \
		$(1)/usr/share/lxc/templates/
endef

define Package/lxc-configs/install
	$(INSTALL_DIR) $(1)/usr/share/lxc/config/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/lxc/config/* \
		$(1)/usr/share/lxc/config/
endef

define Package/liblxc/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/liblxc.so* \
		$(1)/usr/lib/
endef

define Package/lxc-init/install
	$(INSTALL_DIR) $(1)/sbin
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/sbin/init.lxc \
		$(1)/sbin/
endef

define GenPlugin
  define Package/lxc-$(1)
    $(call Package/lxc/Default)
    TITLE:=Utility lxc-$(1) from the LXC userspace tools
    DEPENDS+= +lxc-common $(2) $(DEPENDS_$(1))
  endef

  define Package/lxc-$(1)/install
	$(INSTALL_DIR) $$(1)$(3)
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)$(3)/lxc-$(1) \
		$$(1)$(3)/
  endef

  $$(eval $$(call BuildPackage,lxc-$(1)))
endef


$(eval $(call BuildPackage,lxc))
$(eval $(call BuildPackage,lxc-common))
$(eval $(call BuildPackage,lxc-hooks))
$(eval $(call BuildPackage,lxc-configs))
$(eval $(call BuildPackage,lxc-templates))
$(eval $(call BuildPackage,liblxc))
$(eval $(call BuildPackage,lxc-init))
$(eval $(call BuildPackage,lxc-auto))
$(eval $(call BuildPackage,lxc-unprivileged))
$(foreach u,$(LXC_APPLETS_BIN),$(eval $(call GenPlugin,$(u),$(DEPENDS_APPLETS),"/usr/bin")))
$(foreach u,$(LXC_APPLETS_LIB),$(eval $(call GenPlugin,$(u),$(DEPENDS_APPLETS),"/usr/lib/lxc")))
$(foreach u,$(LXC_SCRIPTS),$(eval $(call GenPlugin,$(u),,"/usr/bin")))
