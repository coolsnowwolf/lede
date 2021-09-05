include $(TOPDIR)/rules.mk

PKG_NAME:=uvol
PKG_VERSION:=0.2
PKG_RELEASE:=$(AUTORELEASE)

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=GPL-2.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/autopart
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Disc
  TITLE:=Automatically initialize LVM partition
  DEPENDS:=+lvm2 +partx-utils +sfdisk
  PKGARCH=all
endef

define Package/autopart/description
 Automatically allocate the GPT partition for LVM and initialize it
 on first boot.
endef

define Package/uvol
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Disc
  TITLE:=OpenWrt UBI/LVM volume abstraction
  PKGARCH=all
endef

define Package/uvol/description
  'uvol' is tool to automate storage volume handling on embedded
  devices in a generic way.
  Also install the 'autopart' package to easily make use of 'uvol' on
  block-storage based devices.

  Examples:
  uvol create example_volume_1 256MiB rw
  uvol up example_volume_1
  uvol device example_volume_1

  uvol create example_volume_2 9812733 ro
  cat example_volume_2.squashfs | uvol write example_volume_2 9812733
  uvol up example_volume_2
  uvol device example_volume_2
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/autopart/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/autopart.defaults $(1)/etc/uci-defaults/30-autopart
endef

define Package/uvol/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/usr/libexec/uvol $(1)/usr/sbin
	$(INSTALL_BIN) ./files/uvol.init $(1)/etc/init.d/uvol
	$(INSTALL_BIN) ./files/ubi.sh $(1)/usr/libexec/uvol/20-ubi.sh
	$(INSTALL_BIN) ./files/lvm.sh $(1)/usr/libexec/uvol/50-lvm.sh
	$(INSTALL_BIN) ./files/uvol $(1)/usr/sbin
endef

$(eval $(call BuildPackage,autopart))
$(eval $(call BuildPackage,uvol))
