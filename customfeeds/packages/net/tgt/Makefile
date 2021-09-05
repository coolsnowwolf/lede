# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=tgt
PKG_VERSION:=1.0.80
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/fujita/tgt/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=e2255482f1a2797c115a6d545c37428b10a4ba6276a2af787da5378fa4e34e75

PKG_MAINTAINER:=Maxim Storchak <m.storchak@gmail.com>
PKG_LICENSE:=GPL-2.0-only
PKG_CPE_ID:=cpe:/a:zaal:tgt

PKG_USE_MIPS16:=0
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/tgt
	SECTION:=net
	CATEGORY:=Network
	URL:=http://stgt.sourceforge.net/
	TITLE:=userspace iSCSI target
	DEPENDS:=+libpthread +libaio
endef

define Package/tgt/description
The Linux target framework (tgt) is a user space SCSI target framework
that supports the iSCSI and iSER transport protocols and that also
supports multiple methods for accessing block storage. Tgt consists of
a user-space daemon and user-space tools.
endef

define Build/Compile
	$(call Build/Compile/Default,programs)
endef

define Package/tgt/conffiles
/etc/config/tgt
endef

define Package/tgt/install
	$(INSTALL_DIR) $(1)/etc/config $(1)/etc/init.d $(1)/usr/sbin
	$(INSTALL_DATA) ./files/tgt.config $(1)/etc/config/tgt
	$(INSTALL_BIN) ./files/tgt.init $(1)/etc/init.d/tgt
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/tgt{d,adm} $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,tgt))
