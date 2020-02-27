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

tools-$(BUILD_TOOLCHAIN) += gmp mpfr mpc libelf expat
tools-y += m4 libtool autoconf automake flex bison pkg-config mklibs zlib
tools-y += sstrip make-ext4fs e2fsprogs mtd-utils mkimage
tools-y += firmware-utils patch-image quilt padjffs2
tools-y += mm-macros missing-macros cmake bc findutils gengetopt patchelf
tools-y += mtools dosfstools libressl
tools-$(CONFIG_TARGET_orion_generic) += wrt350nv2-builder upslug2
tools-y += ucl upx
tools-$(CONFIG_TARGET_x86) += qemu
tools-$(CONFIG_EFI_IMAGES) += gptfdisk popt
tools-$(CONFIG_TARGET_mxs) += elftosb sdimage
tools-$(CONFIG_TARGET_ar71xx) += lzma-old
tools-$(CONFIG_TARGET_ar71xx)$(CONFIG_TARGET_ath79) += squashfs
tools-$(CONFIG_USES_MINOR) += kernel2minor
tools-y += lzma squashfskit4 zip
tools-$(BUILD_B43_TOOLS) += b43-tools
tools-$(BUILD_ISL) += isl
tools-$(CONFIG_USE_SPARSE) += sparse
tools-$(CONFIG_TARGET_apm821xx)$(CONFIG_TARGET_gemini) += genext2fs
tools-$(CONFIG_TARGET_tegra) += cbootimage cbootimage-configs

# builddir dependencies
$(curdir)/upx/compile := $(curdir)/ucl/compile
$(curdir)/bison/compile := $(curdir)/flex/compile
$(curdir)/flex/compile := $(curdir)/libtool/compile
$(curdir)/libtool/compile := $(curdir)/m4/compile $(curdir)/autoconf/compile $(curdir)/automake/compile $(curdir)/missing-macros/compile
$(curdir)/squashfs/compile := $(curdir)/lzma-old/compile
$(curdir)/squashfskit4/compile := $(curdir)/xz/compile $(curdir)/zlib/compile
$(curdir)/quilt/compile := $(curdir)/autoconf/compile $(curdir)/findutils/compile
$(curdir)/autoconf/compile := $(curdir)/m4/compile
$(curdir)/automake/compile := $(curdir)/m4/compile $(curdir)/autoconf/compile $(curdir)/pkg-config/compile $(curdir)/xz/compile
$(curdir)/gmp/compile := $(curdir)/libtool/compile
$(curdir)/mpc/compile := $(curdir)/mpfr/compile $(curdir)/gmp/compile
$(curdir)/mpfr/compile := $(curdir)/gmp/compile
$(curdir)/mtd-utils/compile := $(curdir)/libtool/compile $(curdir)/e2fsprogs/compile $(curdir)/zlib/compile
$(curdir)/mklibs/compile := $(curdir)/libtool/compile
$(curdir)/qemu/compile := $(curdir)/e2fsprogs/compile $(curdir)/zlib/compile
$(curdir)/upslug2/compile := $(curdir)/libtool/compile
$(curdir)/mm-macros/compile := $(curdir)/libtool/compile
$(curdir)/missing-macros/compile := $(curdir)/autoconf/compile
$(curdir)/e2fsprogs/compile := $(curdir)/libtool/compile
$(curdir)/libelf/compile := $(curdir)/libtool/compile
$(curdir)/sdcc/compile := $(curdir)/bison/compile
$(curdir)/b43-tools/compile := $(curdir)/bison/compile
$(curdir)/padjffs2/compile := $(curdir)/findutils/compile
$(curdir)/isl/compile := $(curdir)/gmp/compile
$(curdir)/bc/compile := $(curdir)/bison/compile
$(curdir)/findutils/compile := $(curdir)/bison/compile
$(curdir)/gengetopt/compile := $(curdir)/libtool/compile
$(curdir)/patchelf/compile := $(curdir)/libtool/compile
$(curdir)/dosfstools/compile := $(curdir)/autoconf/compile $(curdir)/automake/compile
$(curdir)/libressl/compile := $(curdir)/pkg-config/compile
$(curdir)/mkimage/compile += $(curdir)/libressl/compile
$(curdir)/firmware-utils/compile += $(curdir)/libressl/compile $(curdir)/zlib/compile
$(curdir)/cmake/compile += $(curdir)/libressl/compile
$(curdir)/zlib/compile := $(curdir)/cmake/compile
$(curdir)/wrt350nv2-builder/compile := $(curdir)/zlib/compile
$(curdir)/lzma-old/compile := $(curdir)/zlib/compile
$(curdir)/make-ext4fs/compile := $(curdir)/zlib/compile
$(curdir)/cbootimage/compile += $(curdir)/automake/compile
$(curdir)/gptfdisk/compile += $(curdir)/e2fsprogs/compile $(curdir)/popt/compile

ifneq ($(HOST_OS),Linux)
  $(curdir)/squashfskit4/compile += $(curdir)/coreutils/compile
  tools-y += coreutils
endif

ifneq ($(CONFIG_CCACHE)$(CONFIG_SDK),)
$(foreach tool, $(filter-out xz patch,$(tools-y)), $(eval $(curdir)/$(tool)/compile += $(curdir)/ccache/compile))
tools-y += ccache
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
		mkdir -p bin lib include stamp; \
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
	ln -sf lib $(STAGING_DIR_HOST)/lib64
	touch $@

endif

$(curdir)//prepare = $(STAGING_DIR)/.prepared $(STAGING_DIR_HOST)/.prepared
$(curdir)//compile = $(STAGING_DIR)/.prepared $(STAGING_DIR_HOST)/.prepared

# prerequisites for the individual targets
$(curdir)/ := .config prereq

$(curdir)/install: $(curdir)/compile

tools_enabled = $(foreach tool,$(sort $(tools-y) $(tools-)),$(if $(filter $(tool),$(tools-y)),y,n))
$(eval $(call stampfile,$(curdir),tools,compile,,_$(subst $(space),,$(tools_enabled))))
$(eval $(call stampfile,$(curdir),tools,check,$(TMP_DIR)/.build))
$(eval $(call subdir,$(curdir)))
