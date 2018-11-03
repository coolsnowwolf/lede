#
# Copyright (C) 2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=open-vm-tools
PKG_VERSION:=10.3.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/vmware/open-vm-tools.git
PKG_SOURCE_VERSION:=stable-$(PKG_VERSION)

PKG_FIXUP:=autoreconf
PKG_LICENSE:=LGPL-2.1

include $(INCLUDE_DIR)/package.mk

define Package/open-vm-tools
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=@TARGET_x86 +glib2 +libpthread +libtirpc
  TITLE:=open-vm-tools
  URL:=https://github.com/vmware/open-vm-tools
endef

define Package/open-vm-tools-vm-tools/description
	Open Virtual Machine Tools for VMware guest OS
endef


CONFIGURE_PATH = open-vm-tools
MAKE_PATH = open-vm-tools

CONFIGURE_ARGS+= \
	--without-icu \
	--disable-multimon \
	--disable-docs \
	--disable-tests \
	--without-gtkmm \
	--without-gtkmm3 \
	--without-xerces \
	--without-pam \
	--disable-grabbitmqproxy \
	--disable-vgauth \
	--disable-deploypkg \
	--without-root-privileges \
	--without-kernel-modules \
	--without-dnet \
	--with-tirpc \
	--without-x \
	--without-gtk2 \
	--without-gtk3 \
	--without-xerces


define Package/open-vm-tools/install
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/vmtoolsd.init $(1)/etc/init.d/vmtoolsd

	$(INSTALL_DIR) $(1)/etc/vmware-tools/
	$(INSTALL_DATA) ./files/tools.conf $(1)/etc/vmware-tools/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-vm-tools/scripts/poweroff-vm-default $(1)/etc/vmware-tools/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-vm-tools/scripts/poweron-vm-default $(1)/etc/vmware-tools/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-vm-tools/scripts/resume-vm-default $(1)/etc/vmware-tools/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-vm-tools/scripts/suspend-vm-default $(1)/etc/vmware-tools/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/scripts/common/statechange.subr $(1)/etc/vmware-tools/

	$(INSTALL_DIR) $(1)/etc/vmware-tools/scripts/vmware/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-vm-tools/scripts/linux/network $(1)/etc/vmware-tools/scripts/vmware/

	$(INSTALL_DIR) $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-vm-tools/checkvm/.libs/vmware-checkvm $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-vm-tools/namespacetool/.libs/vmware-namespace-cmd $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-vm-tools/xferlogs/.libs/vmware-xferlogs $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-vm-tools/hgfsclient/.libs/vmware-hgfsclient $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-vm-tools/services/vmtoolsd/.libs/vmtoolsd $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-vm-tools/rpctool/vmware-rpctool $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-vm-tools/toolbox/.libs/vmware-toolbox-cmd $(1)/bin/

	$(INSTALL_DIR) $(1)/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-vm-tools/hgfsmounter/mount.vmhgfs $(1)/sbin/
	$(INSTALL_BIN) ./files/shutdown $(1)/sbin/

	$(INSTALL_DIR) $(1)/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/libhgfs/.libs/libhgfs.so.0.0.0 $(1)/lib/
	$(LN) libhgfs.so.0.0.0 $(1)/lib/libhgfs.so.0
	$(LN) libhgfs.so.0.0.0 $(1)/lib/libhgfs.so
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/libguestlib/.libs/libguestlib.so.0.0.0 $(1)/lib/
	$(LN) libguestlib.so.0.0.0 $(1)/lib/libguestlib.so.0
	$(LN) libguestlib.so.0.0.0 $(1)/lib/libguestlib.so
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/libvmtools/.libs/libvmtools.so.0.0.0 $(1)/lib/
	$(LN) libvmtools.so.0.0.0 $(1)/lib/libvmtools.so.0
	$(LN) libvmtools.so.0.0.0 $(1)/lib/libvmtools.so

	$(INSTALL_DIR) $(1)/usr/lib/open-vm-tools/plugins/common/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/services/plugins/vix/.libs/libvix.so $(1)/usr/lib/open-vm-tools/plugins/common/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/services/plugins/hgfsServer/.libs/libhgfsServer.so $(1)/usr/lib/open-vm-tools/plugins/common/

	$(INSTALL_DIR) $(1)/usr/lib/open-vm-tools/plugins/vmsvc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/services/plugins/resolutionKMS/.libs/libresolutionKMS.so $(1)/usr/lib/open-vm-tools/plugins/vmsvc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/services/plugins/powerOps/.libs/libpowerOps.so $(1)/usr/lib/open-vm-tools/plugins/vmsvc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/services/plugins/timeSync/.libs/libtimeSync.so $(1)/usr/lib/open-vm-tools/plugins/vmsvc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/services/plugins/guestInfo/.libs/libguestInfo.so $(1)/usr/lib/open-vm-tools/plugins/vmsvc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/services/plugins/vmbackup/.libs/libvmbackup.so $(1)/usr/lib/open-vm-tools/plugins/vmsvc/

	$(INSTALL_DIR) $(1)/lib/udev/rules.d/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/udev/99-vmware-scsi-udev.rules $(1)/lib/udev/rules.d/

	$(INSTALL_DIR) $(1)/share/open-vm-tools/messages/ko/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/services/vmtoolsd/l10n/ko.vmsg $(1)/share/open-vm-tools/messages/ko/vmtoolsd.vmsg
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/toolbox/l10n/ko.vmsg $(1)/share/open-vm-tools/messages/ko/toolboxcmd.vmsg
	$(INSTALL_DIR) $(1)/share/open-vm-tools/messages/de/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/services/vmtoolsd/l10n/de.vmsg $(1)/share/open-vm-tools/messages/de/vmtoolsd.vmsg
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/toolbox/l10n/de.vmsg $(1)/share/open-vm-tools/messages/de/toolboxcmd.vmsg
	$(INSTALL_DIR) $(1)/share/open-vm-tools/messages/ja/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/services/vmtoolsd/l10n/ja.vmsg $(1)/share/open-vm-tools/messages/ja/vmtoolsd.vmsg
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/toolbox/l10n/ja.vmsg $(1)/share/open-vm-tools/messages/ja/toolboxcmd.vmsg
	$(INSTALL_DIR) $(1)/share/open-vm-tools/messages/zh_CN/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-vm-tools/toolbox/l10n/zh_CN.vmsg $(1)/share/open-vm-tools/messages/zh_CN/toolboxcmd.vmsg
endef

$(eval $(call BuildPackage,open-vm-tools))
