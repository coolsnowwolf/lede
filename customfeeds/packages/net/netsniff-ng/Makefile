include $(TOPDIR)/rules.mk

PKG_NAME:=netsniff-ng
PKG_VERSION:=0.6.7
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/netsniff-ng/netsniff-ng/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=030b0554206b43c5883e48a63575fb65d7835e275333372f55f7d615d419da05

PKG_MAINTAINER:=Toni Uhlig <matzeton@googlemail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/netsniff-ng
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libpthread +libpcap +libncurses +zlib +liburcu +libsodium +libnetfilter-conntrack
  TITLE:=netsniff-ng
  URL:=https://github.com/netsniff-ng/netsniff-ng
endef

define Package/netsniff-ng/description
	netsniff-ng is a free, performant Linux network analyzer and
	networking toolkit. If you will, the Swiss army knife for network
	packets.
endef

define Build/Configure
	( cd $(PKG_BUILD_DIR); \
	  CC="$(TARGET_CC)" \
	  LD="$(TARGET_LD)" \
	  CFLAGS="$(TARGET_CFLAGS)" \
	  LDFLAGS="$(TARGET_LDFLAGS)" \
	  CROSS_COMPILE="$(REAL_GNU_TARGET_NAME)-" \
	  ./configure --prefix=/usr --disable-geoip --disable-libnl )
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) Q= \
	  CC="$(TARGET_CC)" \
	  LD="$(TARGET_LD)" \
	  CFLAGS="$(TARGET_CFLAGS)" \
	  LDFLAGS="$(TARGET_LDFLAGS)" \
	  CROSS_COMPILE="$(REAL_GNU_TARGET_NAME)-"
endef

define Package/netsniff-ng/install
	$(INSTALL_DIR) $(1)/etc/netsniff-ng
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/ether.conf $(1)/etc/netsniff-ng
	$(CP) $(PKG_BUILD_DIR)/tcp.conf $(1)/etc/netsniff-ng
	$(CP) $(PKG_BUILD_DIR)/udp.conf $(1)/etc/netsniff-ng
	$(CP) $(PKG_BUILD_DIR)/oui.conf $(1)/etc/netsniff-ng
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/netsniff-ng/netsniff-ng $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/trafgen/trafgen $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/astraceroute/astraceroute $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/flowtop/flowtop $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ifpps/ifpps $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bpfc/bpfc $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/curvetun/curvetun $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,netsniff-ng))
