#
# Copyright (C) 2019 PandoraBox Team
#
include $(TOPDIR)/rules.mk

PKG_NAME := e8350-firmware-utils

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/kernel.mk

define cc
	$(HOSTCC) $(HOST_CFLAGS) -include endian.h $(HOST_LDFLAGS) -o $(HOST_BUILD_DIR)/bin/$(firstword $(1)) $(foreach src,$(1),src/$(src).c) $(2)
endef

define Host/Compile
	mkdir -p $(HOST_BUILD_DIR)/bin
	$(call cc,addpattern-e8350 code_header,-Wall --std=gnu99)
	$(call cc,buildrom-e8350, -Wall --std=gnu99)
	$(call cc,addfwhdr-e8350 crc, -Wall --std=gnu99)
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bin/* $(STAGING_DIR_HOST)/bin/
endef

$(eval $(call HostBuild))
