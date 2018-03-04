#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iproute2
PKG_VERSION:=4.14.1
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/net/iproute2
PKG_HASH:=d43ac068afcc350a448f4581b6e292331ef7e4e7aa746e34981582d5fdb10067
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=iptables
PKG_LICENSE:=GPL-2.0
PKG_CPE_ID:=cpe:/a:iproute2_project:iproute2

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/iproute2/Default
  SECTION:=net
  CATEGORY:=Network
  URL:=http://www.linuxfoundation.org/collaborate/workgroups/networking/iproute2
  SUBMENU:=Routing and Redirection
  MAINTAINER:=Russell Senior <russell@personaltelco.net>
endef

define Package/ip-tiny
$(call Package/iproute2/Default)
 TITLE:=Routing control utility (Minimal)
 VARIANT:=tiny
 PROVIDES:=ip
 ALTERNATIVES:=200:/sbin/ip:/sbin/ip-tiny
 DEPENDS:=+libnl-tiny +(PACKAGE_devlink||PACKAGE_rdma):libmnl
endef

define Package/ip-full
$(call Package/iproute2/Default)
 TITLE:=Routing control utility (Full)
 VARIANT:=full
 PROVIDES:=ip
 ALTERNATIVES:=300:/sbin/ip:/sbin/ip-full
 DEPENDS:=+libnl-tiny +(PACKAGE_devlink||PACKAGE_rdma):libmnl
endef

define Package/tc
$(call Package/iproute2/Default)
  TITLE:=Traffic control utility
  DEPENDS:=+kmod-sched-core +(PACKAGE_devlink||PACKAGE_rdma):libmnl
endef

define Package/genl
$(call Package/iproute2/Default)
  TITLE:=General netlink utility frontend
  DEPENDS:=+libnl-tiny +(PACKAGE_devlink||PACKAGE_rdma):libmnl
endef

define Package/ip-bridge
$(call Package/iproute2/Default)
  TITLE:=Bridge configuration utility from iproute2
  DEPENDS:=+libnl-tiny +(PACKAGE_devlink||PACKAGE_rdma):libmnl
endef

define Package/ss
$(call Package/iproute2/Default)
  TITLE:=Socket statistics utility
  DEPENDS:=+libnl-tiny +(PACKAGE_devlink||PACKAGE_rdma):libmnl
endef

define Package/nstat
$(call Package/iproute2/Default)
  TITLE:=Network statistics utility
  DEPENDS:=+libnl-tiny +(PACKAGE_devlink||PACKAGE_rdma):libmnl
endef

define Package/devlink
$(call Package/iproute2/Default)
  TITLE:=Network devlink utility
  DEPENDS:=+libmnl
endef

define Package/rdma
$(call Package/iproute2/Default)
  TITLE:=Network rdma utility
  DEPENDS:=+libmnl
endef

ifeq ($(BUILD_VARIANT),tiny)
  IP_CONFIG_TINY:=y
endif

ifdef CONFIG_PACKAGE_devlink
  HAVE_MNL:=y
endif

ifdef CONFIG_PACKAGE_rdma
  HAVE_MNL:=y
endif

define Build/Configure
	$(SED) "s,-I/usr/include/db3,," $(PKG_BUILD_DIR)/Makefile
	$(SED) "s,^KERNEL_INCLUDE.*,KERNEL_INCLUDE=$(LINUX_DIR)/include," \
		$(PKG_BUILD_DIR)/Makefile
	$(SED) "s,^LIBC_INCLUDE.*,LIBC_INCLUDE=$(STAGING_DIR)/include," \
		$(PKG_BUILD_DIR)/Makefile
	echo "static const char SNAPSHOT[] = \"$(PKG_VERSION)-$(PKG_RELEASE)-openwrt\";" \
		> $(PKG_BUILD_DIR)/include/SNAPSHOT.h
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections

MAKE_FLAGS += \
	EXTRA_CCOPTS="-I$(STAGING_DIR)/usr/include/libnl-tiny" \
	KERNEL_INCLUDE="$(LINUX_DIR)/include" \
	SHARED_LIBS="" \
	LDFLAGS="$(TARGET_LDFLAGS) -Wl,--gc-sections" \
	IP_CONFIG_TINY=$(IP_CONFIG_TINY) \
	HAVE_MNL=$(HAVE_MNL) \
	IPT_LIB_DIR=/usr/lib/iptables \
	FPIC="$(FPIC)"

define Build/Compile
	+$(MAKE_VARS) $(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) $(MAKE_FLAGS)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/{libgenl,libnetlink,ll_map}.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib/libnetlink.a $(1)/usr/lib/
endef

define Package/ip-tiny/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ip/ip $(1)/sbin/ip-tiny
endef

define Package/ip-full/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ip/ip $(1)/sbin/ip-full
endef

define Package/tc/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tc/tc $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/15-teql $(1)/etc/hotplug.d/iface/
endef

define Package/genl/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/genl/genl $(1)/usr/sbin/
endef

define Package/ip-bridge/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bridge/bridge $(1)/usr/sbin/
endef

define Package/ss/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/misc/ss $(1)/usr/sbin/
endef

define Package/nstat/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/misc/nstat $(1)/usr/sbin/
endef

define Package/devlink/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/devlink/devlink $(1)/usr/sbin/
endef

define Package/rdma/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rdma/rdma $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,ip-tiny))
$(eval $(call BuildPackage,ip-full))
$(eval $(call BuildPackage,tc))
$(eval $(call BuildPackage,genl))
$(eval $(call BuildPackage,ip-bridge))
$(eval $(call BuildPackage,ss))
$(eval $(call BuildPackage,nstat))
$(eval $(call BuildPackage,devlink))
$(eval $(call BuildPackage,rdma))
