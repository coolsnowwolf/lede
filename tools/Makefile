#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# Main makefile for the host tools
#
curdir:=tools

# subdirectories to descend into
tools-y :=

ifeq ($(CONFIG_EXTERNAL_TOOLCHAIN),)
  BUILD_TOOLCHAIN := y
  ifdef CONFIG_GCC_USE_GRAPHITE
    BUILD_ISL = y
  endif
endif
ifneq ($(CONFIG_SDK)$(CONFIG_PACKAGE_kmod-b43)$(CONFIG_BRCMSMAC_USE_FW_FROM_WL),)
  BUILD_B43_TOOLS = y
endif

tools-y += autoconf autoconf-archive automake bc bison cmake cpio dosfstools
tools-y += e2fsprogs expat fakeroot findutils firmware-utils flex gengetopt
tools-y += libressl libtool lzma m4 make-ext4fs meson missing-macros mkimage
tools-y += mklibs mtd-utils mtools ninja padjffs2 patch-image
tools-y += patchelf pkgconf quilt squashfskit4 sstrip zip zlib zstd
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(BUILD_B43_TOOLS),y) += b43-tools
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(BUILD_ISL),y) += isl
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(BUILD_TOOLCHAIN),y) += gmp mpc mpfr
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_TARGET_apm821xx)$(CONFIG_TARGET_gemini),y) += genext2fs
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_TARGET_ath79),y) += lzma-old squashfs
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_TARGET_mxs),y) += elftosb sdimage
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_TARGET_realtek),y) += 7z
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_TARGET_tegra),y) += cbootimage cbootimage-configs
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_USES_MINOR),y) += kernel2minor
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_USE_SPARSE),y) += sparse
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_USE_LLVM_BUILD),y) += llvm-bpf

# builddir dependencies
$(curdir)/autoconf/compile := $(curdir)/m4/compile
$(curdir)/automake/compile := $(curdir)/autoconf/compile $(curdir)/pkgconf/compile $(curdir)/xz/compile
$(curdir)/b43-tools/compile := $(curdir)/bison/compile
$(curdir)/bc/compile := $(curdir)/bison/compile $(curdir)/libtool/compile
$(curdir)/bison/compile := $(curdir)/flex/compile
$(curdir)/cbootimage/compile += $(curdir)/automake/compile
$(curdir)/cmake/compile += $(curdir)/libressl/compile $(curdir)/ninja/compile $(curdir)/expat/compile $(curdir)/xz/compile $(curdir)/zlib/compile $(curdir)/zstd/compile
$(curdir)/dosfstools/compile := $(curdir)/automake/compile
$(curdir)/e2fsprogs/compile := $(curdir)/libtool/compile
$(curdir)/fakeroot/compile := $(curdir)/libtool/compile
$(curdir)/findutils/compile := $(curdir)/bison/compile
$(curdir)/firmware-utils/compile += $(curdir)/cmake/compile
$(curdir)/flex/compile := $(curdir)/libtool/compile
$(curdir)/genext2fs/compile := $(curdir)/libtool/compile
$(curdir)/gengetopt/compile := $(curdir)/libtool/compile
$(curdir)/gmp/compile := $(curdir)/libtool/compile
$(curdir)/isl/compile := $(curdir)/gmp/compile
$(curdir)/libressl/compile := $(curdir)/pkgconf/compile
$(curdir)/libtool/compile := $(curdir)/automake/compile $(curdir)/missing-macros/compile
$(curdir)/lzma-old/compile := $(curdir)/zlib/compile
$(curdir)/llvm-bpf/compile := $(curdir)/cmake/compile
$(curdir)/make-ext4fs/compile := $(curdir)/zlib/compile
$(curdir)/meson/compile := $(curdir)/ninja/compile
$(curdir)/missing-macros/compile := $(curdir)/autoconf/compile
$(curdir)/mkimage/compile += $(curdir)/bison/compile $(curdir)/libressl/compile
$(curdir)/mklibs/compile := $(curdir)/libtool/compile
$(curdir)/mpc/compile := $(curdir)/mpfr/compile $(curdir)/gmp/compile
$(curdir)/mpfr/compile := $(curdir)/gmp/compile
$(curdir)/mtd-utils/compile := $(curdir)/libtool/compile $(curdir)/e2fsprogs/compile $(curdir)/zlib/compile
$(curdir)/padjffs2/compile := $(curdir)/findutils/compile
$(curdir)/patchelf/compile := $(curdir)/libtool/compile
$(curdir)/pkgconf/compile := $(curdir)/meson/compile
$(curdir)/quilt/compile := $(curdir)/autoconf/compile $(curdir)/findutils/compile
$(curdir)/sdcc/compile := $(curdir)/bison/compile
$(curdir)/squashfs/compile := $(curdir)/lzma-old/compile
$(curdir)/squashfskit4/compile := $(curdir)/xz/compile $(curdir)/zlib/compile
$(curdir)/zstd/compile := $(curdir)/meson/compile

ifneq ($(HOST_OS),Linux)
  $(curdir)/squashfskit4/compile += $(curdir)/coreutils/compile
  tools-y += coreutils
endif
ifeq ($(HOST_OS),Darwin)
  tools-y += bash
else
  $(curdir)/dwarves/compile += $(curdir)/elfutils/compile
  $(curdir)/elfutils/compile := $(curdir)/m4/compile $(curdir)/zlib/compile
  tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_DWARVES),y) += dwarves
  tools-y += elfutils
endif

ifneq ($(CONFIG_CCACHE)$(CONFIG_SDK),)
$(foreach tool, $(filter-out zstd zlib xz pkgconf patch ninja meson libressl expat cmake,$(tools-y)), $(eval $(curdir)/$(tool)/compile += $(curdir)/ccache/compile))
tools-y += ccache
$(curdir)/ccache/compile := $(curdir)/cmake/compile
endif

# in case there is no patch tool on the host we need to make patch tool a
# dependency for tools which have patches directory
$(foreach tool, $(tools-y), $(if $(wildcard $(curdir)/$(tool)/patches),$(eval $(curdir)/$(tool)/compile += $(curdir)/patch/compile)))

$(foreach tool, $(filter-out xz,$(tools-y)), $(eval $(curdir)/$(tool)/compile += $(curdir)/xz/compile))

# make any tool depend on tar, xz and patch to ensure that archives can be unpacked and patched properly
tools-core := tar xz patch

$(foreach tool, $(tools-y), $(eval $(curdir)/$(tool)/compile += $(patsubst %,$(curdir)/%/compile,$(tools-core))))
tools-y += $(tools-core)

# make core tools depend on sed and flock
$(foreach tool, $(filter-out xz,$(tools-core)), $(eval $(curdir)/$(tool)/compile += $(curdir)/sed/compile))
$(curdir)/xz/compile += $(curdir)/flock/compile

$(curdir)/sed/compile := $(curdir)/flock/compile $(curdir)/xz/compile
tools-y += flock sed

$(curdir)/autoremove := 1
$(curdir)/builddirs := $(tools-y) $(tools-dep) $(tools-)
$(curdir)/builddirs-default := $(tools-y)

ifdef CHECK_ALL
$(curdir)/builddirs-check:=$($(curdir)/builddirs)
$(curdir)/builddirs-download:=$($(curdir)/builddirs)
endif

ifndef DUMP_TARGET_DB
define PrepareStaging
	@for dir in $(1); do ( \
		$(if $(QUIET),,set -x;) \
		mkdir -p "$$dir"; \
		cd "$$dir"; \
		mkdir -p bin lib stamp usr/include usr/lib; \
	); done
endef

$(BIN_DIR):
	mkdir -p $@

# preparatory work
$(STAGING_DIR)/.prepared: $(TMP_DIR)/.build
	$(call PrepareStaging,$(STAGING_DIR))
	mkdir -p $(BUILD_DIR)/stamp
	touch $@

$(STAGING_DIR_HOST)/.prepared: $(TMP_DIR)/.build
	$(call PrepareStaging,$(STAGING_DIR_HOST))
	mkdir -p $(BUILD_DIR_HOST)/stamp $(STAGING_DIR_HOST)/include/sys
	$(INSTALL_DATA) $(TOPDIR)/tools/include/*.h $(STAGING_DIR_HOST)/include/
	$(INSTALL_DATA) $(TOPDIR)/tools/include/sys/*.h $(STAGING_DIR_HOST)/include/sys/
ifneq ($(HOST_OS),Linux)
	mkdir -p $(STAGING_DIR_HOST)/include/asm
	$(INSTALL_DATA) $(TOPDIR)/tools/include/asm/*.h $(STAGING_DIR_HOST)/include/asm/
endif
	ln -snf lib $(STAGING_DIR_HOST)/lib64
	touch $@

endif

$(curdir)//prepare = $(STAGING_DIR)/.prepared $(STAGING_DIR_HOST)/.prepared $(BIN_DIR)
$(curdir)//compile = $(STAGING_DIR)/.prepared $(STAGING_DIR_HOST)/.prepared $(BIN_DIR)

# prerequisites for the individual targets
$(curdir)/ := .config prereq

$(curdir)/install: $(curdir)/compile

tools_enabled = $(foreach tool,$(sort $(tools-y) $(tools-)),$(if $(filter $(tool),$(tools-y)),y,n))
$(eval $(call stampfile,$(curdir),tools,compile,,_$(subst $(space),,$(tools_enabled)),$(STAGING_DIR_HOST)))
$(eval $(call stampfile,$(curdir),tools,check,$(TMP_DIR)/.build,,$(STAGING_DIR_HOST)))
$(eval $(call subdir,$(curdir)))
