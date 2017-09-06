#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ifeq ($(DUMP),)
  define BuildTarget/bin
    ifeq ($(if $(VARIANT),$(BUILD_VARIANT)),$(VARIANT))
    ifdef Package/$(1)/install
      ifneq ($(CONFIG_PACKAGE_$(1))$(DEVELOPER),)
        $(_pkg_target)compile: $(PKG_BUILD_DIR)/.pkgdir/$(1).installed
        compile: install-bin-$(1)
      else
        compile: $(1)-disabled
        $(1)-disabled:
		@echo "WARNING: skipping $(1) -- package not selected" >&2
      endif
    endif
    endif

    $(PKG_BUILD_DIR)/.pkgdir/$(1).installed: $(STAMP_BUILT)
		rm -rf $(PKG_BUILD_DIR)/.pkgdir/$(1) $$@
		mkdir -p $(PKG_BUILD_DIR)/.pkgdir/$(1)
		$(call Package/$(1)/install,$(PKG_BUILD_DIR)/.pkgdir/$(1))
		touch $$@

    install-bin-$(1): $(PKG_BUILD_DIR)/.pkgdir/$(1).installed
	rm -rf $(BIN_DIR)/$(1)
	-rmdir $(PKG_BUILD_DIR)/.pkgdir/$(1) >/dev/null 2>/dev/null
	if [ -d $(PKG_BUILD_DIR)/.pkgdir/$(1) ]; then \
		$(INSTALL_DIR) $(BIN_DIR)/$(1) && \
		$(CP) $(PKG_BUILD_DIR)/.pkgdir/$(1)/. $(BIN_DIR)/$(1)/; \
	fi

    clean-$(1):
	  rm -rf $(BIN_DIR)/$(1)

    clean: clean-$(1)
    .PHONY: install-bin-$(1)
  endef
endif
