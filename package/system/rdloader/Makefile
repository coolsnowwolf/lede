include $(TOPDIR)/rules.mk

PKG_NAME:=rdloader
PKG_VERSION:=1.0.1
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_BUILD_DEPENDS:=util-linux e2fsprogs

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/rdloader
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=kmod loader for ramdisk enviroment
  DEPENDS:=@TARGET_ramips +e2fsprogs +libblkid
endef

define Package/rdloader/description
  Lightweight kmod loader for ramdisk enviroment,
  support uuid-based rootfs mount
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE=Release

define Build/InstallDev
	rm -fR $(STAGING_DIR)/rdloader
	mkdir -p $(STAGING_DIR)/rdloader/{bin,lib,etc}

	$(CP) $(PKG_BUILD_DIR)/rdloader $(STAGING_DIR)/rdloader/bin/
	$(CP) $(STAGING_DIR_ROOT)/usr/sbin/e2fsck $(STAGING_DIR)/rdloader/bin/
	$(CP) $(STAGING_DIR_ROOT)/etc/e2fsck.conf $(STAGING_DIR)/rdloader/etc/

	( \
		export \
			READELF=$(TARGET_CROSS)readelf \
			OBJCOPY=$(TARGET_CROSS)objcopy \
			XARGS="$(XARGS)"; \
		find $(STAGING_DIR)/rdloader/bin/ -type f -a -exec \
		$(SCRIPT_DIR)/gen-rddependencies.sh {} \; ; \
	) | while read FILE; do \
		cp $(STAGING_DIR_ROOT)/lib/$$$$FILE $(STAGING_DIR)/rdloader/lib/ || \
		cp $(STAGING_DIR_ROOT)/usr/lib/$$$$FILE $(STAGING_DIR)/rdloader/lib/ \
		; \
	done;
endef

$(eval $(call BuildPackage,rdloader))
