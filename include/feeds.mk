# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2014 OpenWrt.org
# Copyright (C) 2016 LEDE Project

-include $(TMP_DIR)/.packageauxvars

FEEDS_INSTALLED:=$(notdir $(wildcard $(TOPDIR)/package/feeds/*))
FEEDS_AVAILABLE:=$(sort $(FEEDS_INSTALLED) $(shell $(SCRIPT_DIR)/feeds list -n 2>/dev/null))

PACKAGE_SUBDIRS=$(PACKAGE_DIR)
ifneq ($(CONFIG_PER_FEED_REPO),)
  PACKAGE_SUBDIRS += $(OUTPUT_DIR)/packages/$(ARCH_PACKAGES)/base
  PACKAGE_SUBDIRS += $(foreach FEED,$(FEEDS_AVAILABLE),$(OUTPUT_DIR)/packages/$(ARCH_PACKAGES)/$(FEED))
endif

opkg_package_files = $(wildcard \
	$(foreach dir,$(PACKAGE_SUBDIRS), \
	  $(foreach pkg,$(1), $(dir)/$(pkg)_*.ipk)))

# 1: package name
define FeedPackageDir
$(strip $(if $(CONFIG_PER_FEED_REPO), \
  $(if $(Package/$(1)/subdir), \
    $(abspath $(OUTPUT_DIR)/packages/$(ARCH_PACKAGES)/$(Package/$(1)/subdir)), \
    $(PACKAGE_DIR)), \
  $(PACKAGE_DIR)))
endef

# 1: destination file
define FeedSourcesAppend
( \
  echo 'src/gz %d_core %U/targets/%S/packages'; \
  $(strip $(if $(CONFIG_PER_FEED_REPO), \
	echo 'src/gz %d_base %U/packages/%A/base'; \
	$(if $(filter %SNAPSHOT-y,$(VERSION_NUMBER)-$(CONFIG_BUILDBOT)), \
		echo 'src/gz %d_kmods %U/targets/%S/kmods/$(LINUX_VERSION)-$(LINUX_RELEASE)-$(LINUX_VERMAGIC)';) \
	$(foreach feed,$(FEEDS_AVAILABLE), \
		$(if $(CONFIG_FEED_$(feed)), \
			echo '$(if $(filter m,$(CONFIG_FEED_$(feed))),# )src/gz %d_$(feed) %U/packages/%A/$(feed)';)))) \
) >> $(1)
endef

# 1: package name
define GetABISuffix
$(if $(ABIV_$(1)),$(ABIV_$(1)),$(call FormatABISuffix,$(1),$(foreach v,$(wildcard $(STAGING_DIR)/pkginfo/$(1).version),$(shell cat $(v)))))
endef

# 1: package name
# 2: abi version
define FormatABISuffix
$(if $(filter-out kmod-%,$(1)),$(if $(2),$(if $(filter %0 %1 %2 %3 %4 %5 %6 %7 %8 %9,$(1)),-)$(2)))
endef
