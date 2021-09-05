include $(TOPDIR)/rules.mk

PKG_NAME:=schroot
PKG_VERSION:=1.6.10
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.xz
PKG_SOURCE_URL:=http://deb.debian.org/debian/pool/main/s/schroot
PKG_HASH:=3ce8dfd9cb97b099e4b6d4ccec421d6cc8c9ef84574681e928a12badb5643d0b

PKG_MAINTAINER:=Javier Marcet <javier@marcet.info>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/package.mk

define Package/schroot
  SECTION:=admin
  CATEGORY:=Administration
  TITLE:=Securely enter a chroot and run a command or login shell.
  DEPENDS:=$(ICONV_DEPENDS) \
	+boost +boost-filesystem +boost-iostreams +boost-program_options +boost-regex \
	+SCHROOT_BTRFS:btrfs-progs \
	+SCHROOT_LOOPBACK:losetup \
	+SCHROOT_LVM:lvm2 \
	+SCHROOT_UUID:libuuid \
	$(INTL_DEPENDS)
  URL:=https://salsa.debian.org/debian/schroot
endef

define Package/schroot/description
  Securely enter a chroot and run a command or login shell.
endef

define Package/schroot/conffiles
/etc/schroot/
endef

define Package/schroot/config
	source "$(SOURCE)/Config.in"
endef

PKG_CONFIG_DEPENDS := \
	CONFIG_SCHROOT_BTRFS \
	CONFIG_SCHROOT_LOOPBACK \
	CONFIG_SCHROOT_LVM \
	CONFIG_SCHROOT_UUID

CONFIGURE_ARGS += \
	$(if $(CONFIG_SCHROOT_BTRFS),--enable,--disable)-btrfs-snapshot \
	$(if $(CONFIG_SCHROOT_LOOPBACK),--enable,--disable)-loopback \
	$(if $(CONFIG_SCHROOT_LVM),--enable,--disable)-lvm-snapshot \
	$(if $(CONFIG_SCHROOT_UUID),--enable,--disable)-uuid \
	--enable-block-device \
	--enable-union \
	--disable-dchroot \
	--disable-dchroot-dsa \
	--disable-csbuild \
	--disable-debug \
	--disable-pam \
	--disable-doxygen \
	--disable-rpath

define Package/schroot/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_SUID) $(PKG_INSTALL_DIR)/usr/bin/schroot $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/schroot
	$(CP) -R $(PKG_INSTALL_DIR)/etc/schroot/* $(1)/etc/schroot
	$(INSTALL_DIR) $(1)/usr/lib/schroot
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libsbuild.* $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/schroot/* $(1)/usr/lib/schroot/
	$(INSTALL_DIR) $(1)/usr/share/schroot
	$(CP) -R $(PKG_INSTALL_DIR)/usr/share/schroot/* $(1)/usr/share/schroot/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/schroot.init $(1)/etc/init.d/schroot
endef

$(eval $(call BuildPackage,schroot))
