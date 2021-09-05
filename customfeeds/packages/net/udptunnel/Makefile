include $(TOPDIR)/rules.mk

PKG_NAME:=udptunnel
PKG_VERSION:=1.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.cs.columbia.edu/~lennox/udptunnel
PKG_HASH:=45c0e12045735bc55734076ebbdc7622c746d1fe4e6f7267fa122e2421754670


PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYRIGHT

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/udptunnel
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Tunnel UDP packets over a TCP connection
  URL:=http://www.cs.columbia.edu/~lennox/udptunnel/
endef

define Package/udptunnel/description 
  UDPTunnel is a small program which can tunnel UDP packets bi-directionally
  over a TCP connection. Its primary purpose (and original motivation) is to
  allow multi-media conferences to traverse a firewall which allows only outgoing
  TCP connections.
endef

define Package/udptunnel/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/udptunnel $(1)/usr/bin/
endef

$(eval $(call BuildPackage,udptunnel))
