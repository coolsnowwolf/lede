#
# Copyright (C) 2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=open-vm-tools
PKG_VERSION:=11.2.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-17337674.tar.gz
PKG_SOURCE_URL:=https://github.com/vmware/open-vm-tools/releases/download/stable-$(PKG_VERSION)
PKG_HASH:=d01c9e036536b569ee561e33302e9dad1c2ac27c04f762ebdc3f81791cb44566
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-17337674

PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=LICENSE

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=glib2/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/open-vm-tools/Default
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=https://github.com/vmware/open-vm-tools
endef

define Package/open-vm-tools
$(call Package/open-vm-tools/Default)
  DEPENDS:=@TARGET_x86 +glib2 +libpthread +libtirpc
  TITLE:=open-vm-tools
  URL:=https://github.com/vmware/open-vm-tools
  MAINTAINER:=Yuhei OKAWA <tochiro.srchack@gmail.com>
endef

define Package/open-vm-tools/description
	Open Virtual Machine Tools for VMware guest OS
endef

define Package/open-vm-tools-fuse
$(call Package/open-vm-tools/Default)
  DEPENDS:=+open-vm-tools +libfuse
  TITLE:=open-vm-tools-fuse
  MAINTAINER:=Kagurazaka Kotori <kagurazakakotori@gmail.com>
endef

define Package/open-vm-tools-fuse/description
	Shared Folders user mode FUSE client support for VMware guest OS
endef

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
	--without-xerces \
	--enable-resolutionkms=no

TARGET_LDFLAGS += -liconv

define Build/Prepare
	$(call Build/Prepare/Default)
ifeq ($(QUILT),)
	mv $(PKG_BUILD_DIR)/lib/include/poll.h $(PKG_BUILD_DIR)/lib/include/vm_poll.h
endif
endef

define Package/open-vm-tools/install
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/vmtoolsd.init $(1)/etc/init.d/vmtoolsd

	$(INSTALL_DIR) $(1)/etc/vmware-tools/
	$(INSTALL_DATA) ./files/tools.conf $(1)/etc/vmware-tools/
	$(CP) $(PKG_INSTALL_DIR)/etc/vmware-tools $(1)/etc/

	$(INSTALL_DIR) $(1)/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vmtoolsd $(1)/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vmware-checkvm $(1)/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vmware-hgfsclient $(1)/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vmware-namespace-cmd $(1)/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vmware-rpctool $(1)/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vmware-toolbox-cmd $(1)/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vmware-xferlogs $(1)/bin/

	$(INSTALL_DIR) $(1)/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/mount.vmhgfs $(1)/sbin/
	$(INSTALL_BIN) ./files/shutdown $(1)/sbin/

	$(INSTALL_DIR) $(1)/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libguestlib.so* $(1)/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libhgfs.so* $(1)/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libvmtools.so* $(1)/lib/

	$(INSTALL_DIR) $(1)/usr/lib/open-vm-tools/plugins/common/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/open-vm-tools/plugins/common/libhgfsServer.so $(1)/usr/lib/open-vm-tools/plugins/common/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/open-vm-tools/plugins/common/libvix.so $(1)/usr/lib/open-vm-tools/plugins/common/

	$(INSTALL_DIR) $(1)/usr/lib/open-vm-tools/plugins/vmsvc/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/open-vm-tools/plugins/vmsvc/libguestInfo.so $(1)/usr/lib/open-vm-tools/plugins/vmsvc/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/open-vm-tools/plugins/vmsvc/libpowerOps.so $(1)/usr/lib/open-vm-tools/plugins/vmsvc/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/open-vm-tools/plugins/vmsvc/libtimeSync.so $(1)/usr/lib/open-vm-tools/plugins/vmsvc/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/open-vm-tools/plugins/vmsvc/libvmbackup.so $(1)/usr/lib/open-vm-tools/plugins/vmsvc/

	$(INSTALL_DIR) $(1)/etc/hotplug.d/block/
	$(INSTALL_BIN) ./files/vmware-scsi.hotplug $(1)/etc/hotplug.d/block/80-vmware-scsi

	$(INSTALL_DIR) $(1)/usr/share/open-vm-tools/messages/de/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/open-vm-tools/messages/de/toolboxcmd.vmsg $(1)/usr/share/open-vm-tools/messages/de/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/open-vm-tools/messages/de/vmtoolsd.vmsg $(1)/usr/share/open-vm-tools/messages/de/
	$(INSTALL_DIR) $(1)/usr/share/open-vm-tools/messages/ko/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/open-vm-tools/messages/ko/toolboxcmd.vmsg $(1)/usr/share/open-vm-tools/messages/ko/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/open-vm-tools/messages/ko/vmtoolsd.vmsg $(1)/usr/share/open-vm-tools/messages/ko/
	$(INSTALL_DIR) $(1)/usr/share/open-vm-tools/messages/zh_CN/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/open-vm-tools/messages/zh_CN/toolboxcmd.vmsg $(1)/usr/share/open-vm-tools/messages/zh_CN/
	$(INSTALL_DIR) $(1)/usr/share/open-vm-tools/messages/ja/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/open-vm-tools/messages/ja/toolboxcmd.vmsg $(1)/usr/share/open-vm-tools/messages/ja/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/open-vm-tools/messages/ja/vmtoolsd.vmsg $(1)/usr/share/open-vm-tools/messages/ja/
endef

define Package/open-vm-tools-fuse/install
	$(INSTALL_DIR) $(1)/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vmhgfs-fuse $(1)/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/vmware-vmblock-fuse $(1)/bin/
endef

$(eval $(call BuildPackage,open-vm-tools))
$(eval $(call BuildPackage,open-vm-tools-fuse))
