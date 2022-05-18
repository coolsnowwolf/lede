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
ifneq ($(CONFIG_SDK)$(CONFIG_PACKAGE_kmod-b43)$(CONFIG_PACKAGE_b43legacy-firmware)$(CONFIG_BRCMSMAC_USE_FW_FROM_WL),)
  BUILD_B43_TOOLS = y
endif

tools-y += autoconf autoconf-archive automake bc bison cmake cpio dosfstools
tools-y += e2fsprogs fakeroot findutils firmware-utils flex gengetopt
tools-y += libressl libtool lzma m4 make-ext4fs meson missing-macros mkimage
tools-y += mklibs mm-macros mtd-utils mtools ninja padjffs2 patch-image
tools-y += patchelf pkgconf quilt squashfskit4 sstrip ucl upx xxd zip zlib zstd
tools-$(BUILD_B43_TOOLS) += b43-tools
tools-$(BUILD_ISL) += isl
tools-$(BUILD_TOOLCHAIN) += expat gmp mpc mpfr
tools-$(CONFIG_TARGET_apm821xx)$(CONFIG_TARGET_gemini) += genext2fs
tools-$(CONFIG_TARGET_ath79) += lzma-old squashfs
tools-$(CONFIG_TARGET_mxs) += elftosb sdimage
tools-$(CONFIG_TARGET_realtek) += 7z
tools-$(CONFIG_TARGET_tegra) += cbootimage cbootimage-configs
tools-$(CONFIG_USES_MINOR) += kernel2minor
tools-$(CONFIG_USE_SPARSE) += sparse
tools-$(CONFIG_USE_LLVM_BUILD) += llvm-bpf

# builddir dependencies
$(curdir)/autoconf/compile := $(curdir)/m4/compile
$(curdir)/automake/compile := $(curdir)/m4/compile $(curdir)/autoconf/compile $(curdir)/pkgconf/compile $(curdir)/xz/compile
$(curdir)/b43-tools/compile := $(curdir)/bison/compile
$(curdir)/bc/compile := $(curdir)/bison/compile $(curdir)/libtool/compile
$(curdir)/upx/compile := $(curdir)/ucl/compile
$(curdir)/bison/compile := $(curdir)/flex/compile
$(curdir)/cbootimage/compile += $(curdir)/automake/compile
$(curdir)/cmake/compile += $(curdir)/libressl/compile $(curdir)/ninja/compile
$(curdir)/dosfstools/compile := $(curdir)/autoconf/compile $(curdir)/automake/compile
$(curdir)/e2fsprogs/compile := $(curdir)/libtool/compile
$(curdir)/fakeroot/compile := $(curdir)/libtool/compile
$(curdir)/findutils/compile := $(curdir)/bison/compile
$(curdir)/firmware-utils/compile += $(curdir)/libressl/compile $(curdir)/zlib/compile
$(curdir)/flex/compile := $(curdir)/libtool/compile
$(curdir)/gengetopt/compile := $(curdir)/libtool/compile
$(curdir)/gmp/compile := $(curdir)/libtool/compile
$(curdir)/isl/compile := $(curdir)/gmp/compile
$(curdir)/libressl/compile := $(curdir)/pkgconf/compile
$(curdir)/libtool/compile := $(curdir)/m4/compile $(curdir)/autoconf/compile $(curdir)/automake/compile $(curdir)/missing-macros/compile
$(curdir)/lzma-old/compile := $(curdir)/zlib/compile
$(curdir)/llvm-bpf/compile := $(curdir)/cmake/compile
$(curdir)/meson/compile := $(curdir)/ninja/compile
$(curdir)/make-ext4fs/compile := $(curdir)/zlib/compile
$(curdir)/missing-macros/compile := $(curdir)/autoconf/compile
$(curdir)/mkimage/compile += $(curdir)/libressl/compile
$(curdir)/mklibs/compile := $(curdir)/libtool/compile
$(curdir)/mm-macros/compile := $(curdir)/libtool/compile
$(curdir)/mpc/compile := $(curdir)/mpfr/compile $(curdir)/gmp/compile
$(curdir)/mpfr/compile := $(curdir)/gmp/compile
$(curdir)/mtd-utils/compile := $(curdir)/libtool/compile $(curdir)/e2fsprogs/compile $(curdir)/zlib/compile
$(curdir)/padjffs2/compile := $(curdir)/findutils/compile
$(curdir)/patchelf/compile := $(curdir)/libtool/compile
$(curdir)/quilt/compile := $(curdir)/autoconf/compile $(curdir)/findutils/compile
$(curdir)/sdcc/compile := $(curdir)/bison/compile
$(curdir)/squashfs/compile := $(curdir)/lzma-old/compile
$(curdir)/squashfskit4/compile := $(curdir)/xz/compile $(curdir)/zlib/compile
$(curdir)/zlib/compile := $(curdir)/cmake/compile
$(curdir)/zstd/compile := $(curdir)/cmake/compile

ifneq ($(HOST_OS),Linux)
  $(curdir)/squashfskit4/compile += $(curdir)/coreutils/compile
  tools-y += coreutils
endif
ifeq ($(HOST_OS),Darwin)
  tools-y += bash
else
  $(curdir)/dwarves/compile += $(curdir)/elfutils/compile
  $(curdir)/elfutils/compile := $(curdir)/m4/compile $(curdir)/zlib/compile
  tools-$(CONFIG_DWARVES) += dwarves
  tools-y += elfutils
endif

ifneq ($(CONFIG_CCACHE)$(CONFIG_SDK),)
$(foreach tool, $(filter-out xz zstd patch pkgconf libressl ninja cmake,$(tools-y)), $(eval $(curdir)/$(tool)/compile += $(curdir)/ccache/compile))
tools-y += ccache
$(curdir)/ccache/compile := $(curdir)/zstd/compile
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

$(curdir)//prepare = $(STAGING_DIR)/.prepared $(STAGING_DIR_HOST)/.prepared
$(curdir)//compile = $(STAGING_DIR)/.prepared $(STAGING_DIR_HOST)/.prepared

# prerequisites for the individual targets
$(curdir)/ := .config prereq

$(curdir)/install: $(curdir)/compile

tools_enabled = $(foreach tool,$(sort $(tools-y) $(tools-)),$(if $(filter $(tool),$(tools-y)),y,n))
$(eval $(call stampfile,$(curdir),tools,compile,,_$(subst $(space),,$(tools_enabled)),$(STAGING_DIR_HOST)))
$(eval $(call stampfile,$(curdir),tools,check,$(TMP_DIR)/.build,,$(STAGING_DIR_HOST)))
$(eval $(call subdir,$(curdir)))
