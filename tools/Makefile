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
ifneq ($(CONFIG_SDK)$(CONFIG_TARGET_INITRAMFS_COMPRESSION_BZIP2),)
  BUILD_BZIP2_TOOLS = y
endif
ifneq ($(CONFIG_SDK)$(CONFIG_TARGET_INITRAMFS_COMPRESSION_LZ4),)
  BUILD_LZ4_TOOLS = y
endif
ifneq ($(CONFIG_SDK)$(CONFIG_TARGET_INITRAMFS_COMPRESSION_LZO),)
  BUILD_LZO_TOOLS = y
endif

tools-y += autoconf
tools-y += autoconf-archive
tools-y += automake
tools-y += bc
tools-y += bison
tools-y += cmake
tools-y += cpio
tools-y += dosfstools
tools-y += e2fsprogs
tools-y += elfutils
tools-y += expat
tools-y += fakeroot
tools-y += findutils
tools-y += firmware-utils
tools-y += flex
tools-y += gengetopt
tools-y += gnulib
tools-y += libressl
tools-y += libtool
tools-y += lzma
tools-y += m4
tools-y += make-ext4fs
tools-y += meson
tools-y += missing-macros
tools-y += mkimage
tools-y += mklibs
tools-y += mtd-utils
tools-y += mtools
tools-y += ninja
tools-y += padjffs2
tools-y += patch-image
tools-y += patchelf
tools-y += pkgconf
tools-y += quilt
tools-y += squashfs4
tools-y += sstrip
tools-y += util-linux
tools-y += xz
tools-y += zip
tools-y += zlib
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS),y) += liblzo
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(BUILD_B43_TOOLS),y) += b43-tools
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(BUILD_BZIP2_TOOLS),y) += bzip2
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(BUILD_ISL),y) += isl
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(BUILD_LZ4_TOOLS),y) += lz4
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(BUILD_LZO_TOOLS),y) += lzop
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(BUILD_TOOLCHAIN),y) += gmp mpc mpfr
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_TARGET_apm821xx)$(CONFIG_TARGET_gemini),y) += genext2fs
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_TARGET_ath79),y) += lzma-old squashfs3-lzma
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_TARGET_mxs),y) += elftosb sdimage
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_TARGET_realtek),y) += 7z
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_TARGET_tegra),y) += cbootimage cbootimage-configs
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_USES_MINOR),y) += yafut
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_USE_SPARSE),y) += sparse
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_USE_LLVM_BUILD),y) += llvm-bpf
tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_USE_MOLD),y) += mold

# builddir dependencies
$(curdir)/autoconf/compile := $(curdir)/m4/compile
$(curdir)/automake/compile := $(curdir)/autoconf/compile $(curdir)/pkgconf/compile $(curdir)/xz/compile
$(curdir)/b43-tools/compile := $(curdir)/bison/compile
$(curdir)/bc/compile := $(curdir)/bison/compile $(curdir)/libtool/compile
$(curdir)/bison/compile := $(curdir)/flex/compile
$(curdir)/cbootimage/compile += $(curdir)/automake/compile
$(curdir)/cmake/compile += $(curdir)/libressl/compile $(curdir)/ninja/compile $(curdir)/expat/compile $(curdir)/xz/compile $(curdir)/zlib/compile $(curdir)/zstd/compile
$(curdir)/dosfstools/compile := $(curdir)/automake/compile
$(curdir)/e2fsprogs/compile := $(curdir)/libtool/compile $(curdir)/util-linux/compile
$(curdir)/elfutils/compile := $(curdir)/m4/compile $(curdir)/zlib/compile $(curdir)/gnulib/compile $(curdir)/libtool/compile
$(curdir)/fakeroot/compile := $(curdir)/libtool/compile
$(curdir)/findutils/compile := $(curdir)/bison/compile
$(curdir)/firmware-utils/compile += $(curdir)/cmake/compile
$(curdir)/flex/compile := $(curdir)/libtool/compile
$(curdir)/genext2fs/compile := $(curdir)/libtool/compile
$(curdir)/gengetopt/compile := $(curdir)/libtool/compile
$(curdir)/gmp/compile := $(curdir)/libtool/compile
$(curdir)/isl/compile := $(curdir)/gmp/compile
$(curdir)/liblzo/compile := $(curdir)/cmake/compile
$(curdir)/libressl/compile := $(curdir)/pkgconf/compile
$(curdir)/libtool/compile := $(curdir)/automake/compile $(curdir)/gnulib/compile $(curdir)/missing-macros/compile
$(curdir)/lz4/compile := $(curdir)/meson/compile
$(curdir)/lzma-old/compile := $(curdir)/zlib/compile
$(curdir)/lzop/compile := $(curdir)/cmake/compile $(curdir)/liblzo/compile
$(curdir)/llvm-bpf/compile := $(curdir)/cmake/compile
$(curdir)/make-ext4fs/compile := $(curdir)/zlib/compile
$(curdir)/meson/compile := $(curdir)/ninja/compile
$(curdir)/missing-macros/compile := $(curdir)/autoconf/compile
$(curdir)/mkimage/compile += $(curdir)/bison/compile $(curdir)/libressl/compile
$(curdir)/mklibs/compile := $(curdir)/libtool/compile
$(curdir)/mold/compile := $(curdir)/cmake/compile $(curdir)/zlib/compile $(curdir)/zstd/compile
$(curdir)/mpc/compile := $(curdir)/mpfr/compile $(curdir)/gmp/compile
$(curdir)/mpfr/compile := $(curdir)/gmp/compile
$(curdir)/mtd-utils/compile := $(curdir)/libtool/compile $(curdir)/e2fsprogs/compile $(curdir)/zlib/compile
$(curdir)/padjffs2/compile := $(curdir)/findutils/compile
$(curdir)/patchelf/compile := $(curdir)/libtool/compile
$(curdir)/pkgconf/compile := $(curdir)/meson/compile
$(curdir)/quilt/compile := $(curdir)/autoconf/compile $(curdir)/findutils/compile
$(curdir)/sdcc/compile := $(curdir)/bison/compile
$(curdir)/squashfs3-lzma/compile := $(curdir)/lzma-old/compile
$(curdir)/squashfs4/compile := $(curdir)/xz/compile $(curdir)/zlib/compile
$(curdir)/util-linux/compile := $(curdir)/bison/compile $(curdir)/automake/compile
$(curdir)/yafut/compile := $(curdir)/cmake/compile

ifneq ($(HOST_OS),Linux)
  $(curdir)/coreutils/compile += $(curdir)/automake/compile $(curdir)/bison/compile $(curdir)/gnulib/compile
  $(curdir)/squashfs4/compile += $(curdir)/coreutils/compile
  tools-y += coreutils
endif
ifeq ($(HOST_OS),Darwin)
  tools-y += bash
else
  $(curdir)/dwarves/compile += $(curdir)/elfutils/compile
  tools-$(if $(CONFIG_BUILD_ALL_HOST_TOOLS)$(CONFIG_DWARVES),y) += dwarves
endif

ifneq ($(CONFIG_CCACHE)$(CONFIG_SDK),)
$(foreach tool, $(filter-out zstd zlib xz pkgconf patch ninja meson libressl expat cmake,$(tools-y)), $(eval $(curdir)/$(tool)/compile += $(curdir)/ccache/compile))
tools-y += ccache xxhash
$(curdir)/xxhash/compile := $(curdir)/cmake/compile
$(curdir)/ccache/compile := $(curdir)/xxhash/compile
endif

# in case there is no patch tool on the host we need to make patch tool a
# dependency for tools which have patches directory
$(foreach tool, $(tools-y), $(if $(wildcard $(curdir)/$(tool)/patches),$(eval $(curdir)/$(tool)/compile += $(curdir)/patch/compile)))

$(foreach tool, $(filter-out zstd,$(tools-y)), $(eval $(curdir)/$(tool)/compile += $(curdir)/zstd/compile))

# make any tool depend on the following to ensure that archives can be unpacked and patched properly
tools-core += libdeflate
tools-core += patch
tools-core += tar
tools-core += zstd

$(foreach tool, $(tools-y), $(eval $(curdir)/$(tool)/compile += $(patsubst %,$(curdir)/%/compile,$(tools-core))))
tools-y += $(tools-core)

# make some core tools depend on sed and flock
$(curdir)/patch/compile += $(curdir)/sed/compile
$(curdir)/tar/compile += $(curdir)/sed/compile
$(curdir)/zstd/compile += $(curdir)/libdeflate/compile

$(curdir)/sed/compile := $(curdir)/flock/compile $(curdir)/zstd/compile
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
