include $(TOPDIR)/rules.mk

PKG_NAME:=cgroupfs-mount
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/tianon/$(PKG_NAME)
PKG_SOURCE_VERSION:=0549428171605eae3097a3e21bf7664845eac9e8
PKG_SOURCE_DATE:=2020-06-26
PKG_MIRROR_HASH:=ca217ffff5aa938149d2d8adfe15d800903d2fec180acb2400c36d62905988ea

PKG_MAINTAINER:=Gerard Ryan <G.M0N3Y.2503@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/cgroupfs-mount/config
	config CGROUPFS_MOUNT_KERNEL_CGROUPS
		bool "Enable kernel cgroups support"
		depends on PACKAGE_cgroupfs-mount
		default y if ( DOCKER_KERNEL_OPTIONS || LXC_KERNEL_OPTIONS )
		select KERNEL_CGROUPS
endef

define Package/cgroupfs-mount
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=cgroup mount scripts
  DEPENDS:=+mount-utils
  MENU:=1
endef

define Package/cgroupfs-mount/description
Simple scripts to properly mount the cgroupfs hierarchy, especially structured for Debian packaging
endef

Build/Compile=# Nothing to compile, just install the scripts

define Package/cgroupfs-mount/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cgroupfs-mount $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cgroupfs-umount $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/cgroupfs-mount.init $(1)/etc/init.d/cgroupfs-mount
endef

$(eval $(call BuildPackage,cgroupfs-mount))
